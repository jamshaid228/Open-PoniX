/*
 * Copyright © 2006,2008,2011 Intel Corporation
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Wang Zhenyu <zhenyu.z.wang@sna.com>
 *    Eric Anholt <eric@anholt.net>
 *    Carl Worth <cworth@redhat.com>
 *    Keith Packard <keithp@keithp.com>
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include "sna_reg.h"
#include "sna_render.h"
#include "sna_render_inline.h"
#include "sna_video.h"

#include "brw/brw.h"
#include "gen6_render.h"

#define NO_COMPOSITE 0
#define NO_COMPOSITE_SPANS 0
#define NO_COPY 0
#define NO_COPY_BOXES 0
#define NO_FILL 0
#define NO_FILL_BOXES 0
#define NO_FILL_ONE 0
#define NO_FILL_CLEAR 0

#define NO_RING_SWITCH 0
#define PREFER_RENDER 0

#define USE_8_PIXEL_DISPATCH 1
#define USE_16_PIXEL_DISPATCH 1
#define USE_32_PIXEL_DISPATCH 0

#if !USE_8_PIXEL_DISPATCH && !USE_16_PIXEL_DISPATCH && !USE_32_PIXEL_DISPATCH
#error "Must select at least 8, 16 or 32 pixel dispatch"
#endif

#define GEN6_MAX_SIZE 8192

struct gt_info {
	int max_vs_threads;
	int max_gs_threads;
	int max_wm_threads;
	struct {
		int size;
		int max_vs_entries;
		int max_gs_entries;
	} urb;
};

static const struct gt_info gt1_info = {
	.max_vs_threads = 24,
	.max_gs_threads = 21,
	.max_wm_threads = 40,
	.urb = { 32, 256, 256 },
};

static const struct gt_info gt2_info = {
	.max_vs_threads = 60,
	.max_gs_threads = 60,
	.max_wm_threads = 80,
	.urb = { 64, 256, 256 },
};

static const uint32_t ps_kernel_packed[][4] = {
#include "exa_wm_src_affine.g6b"
#include "exa_wm_src_sample_argb.g6b"
#include "exa_wm_yuv_rgb.g6b"
#include "exa_wm_write.g6b"
};

static const uint32_t ps_kernel_planar[][4] = {
#include "exa_wm_src_affine.g6b"
#include "exa_wm_src_sample_planar.g6b"
#include "exa_wm_yuv_rgb.g6b"
#include "exa_wm_write.g6b"
};

#define NOKERNEL(kernel_enum, func, ns) \
    [GEN6_WM_KERNEL_##kernel_enum] = {#kernel_enum, func, 0, ns}
#define KERNEL(kernel_enum, kernel, ns) \
    [GEN6_WM_KERNEL_##kernel_enum] = {#kernel_enum, kernel, sizeof(kernel), ns}

static const struct wm_kernel_info {
	const char *name;
	const void *data;
	unsigned int size;
	unsigned int num_surfaces;
} wm_kernels[] = {
	NOKERNEL(NOMASK, brw_wm_kernel__affine, 2),
	NOKERNEL(NOMASK_P, brw_wm_kernel__projective, 2),

	NOKERNEL(MASK, brw_wm_kernel__affine_mask, 3),
	NOKERNEL(MASK_P, brw_wm_kernel__projective_mask, 3),

	NOKERNEL(MASKCA, brw_wm_kernel__affine_mask_ca, 3),
	NOKERNEL(MASKCA_P, brw_wm_kernel__projective_mask_ca, 3),

	NOKERNEL(MASKSA, brw_wm_kernel__affine_mask_sa, 3),
	NOKERNEL(MASKSA_P, brw_wm_kernel__projective_mask_sa, 3),

	NOKERNEL(OPACITY, brw_wm_kernel__affine_opacity, 2),
	NOKERNEL(OPACITY_P, brw_wm_kernel__projective_opacity, 2),

	KERNEL(VIDEO_PLANAR, ps_kernel_planar, 7),
	KERNEL(VIDEO_PACKED, ps_kernel_packed, 2),
};
#undef KERNEL

static const struct blendinfo {
	bool src_alpha;
	uint32_t src_blend;
	uint32_t dst_blend;
} gen6_blend_op[] = {
	/* Clear */	{0, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_ZERO},
	/* Src */	{0, GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_ZERO},
	/* Dst */	{0, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_ONE},
	/* Over */	{1, GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* OverReverse */ {0, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_ONE},
	/* In */	{0, GEN6_BLENDFACTOR_DST_ALPHA, GEN6_BLENDFACTOR_ZERO},
	/* InReverse */	{1, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_SRC_ALPHA},
	/* Out */	{0, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_ZERO},
	/* OutReverse */ {1, GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* Atop */	{1, GEN6_BLENDFACTOR_DST_ALPHA, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* AtopReverse */ {1, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_SRC_ALPHA},
	/* Xor */	{1, GEN6_BLENDFACTOR_INV_DST_ALPHA, GEN6_BLENDFACTOR_INV_SRC_ALPHA},
	/* Add */	{0, GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_ONE},
};

/**
 * Highest-valued BLENDFACTOR used in gen6_blend_op.
 *
 * This leaves out GEN6_BLENDFACTOR_INV_DST_COLOR,
 * GEN6_BLENDFACTOR_INV_CONST_{COLOR,ALPHA},
 * GEN6_BLENDFACTOR_INV_SRC1_{COLOR,ALPHA}
 */
#define GEN6_BLENDFACTOR_COUNT (GEN6_BLENDFACTOR_INV_DST_ALPHA + 1)

#define GEN6_BLEND_STATE_PADDED_SIZE	ALIGN(sizeof(struct gen6_blend_state), 64)

#define BLEND_OFFSET(s, d) \
	(((s) * GEN6_BLENDFACTOR_COUNT + (d)) * GEN6_BLEND_STATE_PADDED_SIZE)

#define NO_BLEND BLEND_OFFSET(GEN6_BLENDFACTOR_ONE, GEN6_BLENDFACTOR_ZERO)
#define CLEAR BLEND_OFFSET(GEN6_BLENDFACTOR_ZERO, GEN6_BLENDFACTOR_ZERO)

#define SAMPLER_OFFSET(sf, se, mf, me) \
	(((((sf) * EXTEND_COUNT + (se)) * FILTER_COUNT + (mf)) * EXTEND_COUNT + (me) + 2) * 2 * sizeof(struct gen6_sampler_state))

#define VERTEX_2s2s 0

#define COPY_SAMPLER 0
#define COPY_VERTEX VERTEX_2s2s
#define COPY_FLAGS(a) GEN6_SET_FLAGS(COPY_SAMPLER, (a) == GXcopy ? NO_BLEND : CLEAR, GEN6_WM_KERNEL_NOMASK, COPY_VERTEX)

#define FILL_SAMPLER (2 * sizeof(struct gen6_sampler_state))
#define FILL_VERTEX VERTEX_2s2s
#define FILL_FLAGS(op, format) GEN6_SET_FLAGS(FILL_SAMPLER, gen6_get_blend((op), false, (format)), GEN6_WM_KERNEL_NOMASK, FILL_VERTEX)
#define FILL_FLAGS_NOBLEND GEN6_SET_FLAGS(FILL_SAMPLER, NO_BLEND, GEN6_WM_KERNEL_NOMASK, FILL_VERTEX)

#define VIDEO_SAMPLER \
	SAMPLER_OFFSET(SAMPLER_FILTER_BILINEAR, SAMPLER_EXTEND_PAD, \
		       SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE)

#define GEN6_SAMPLER(f) (((f) >> 16) & 0xfff0)
#define GEN6_BLEND(f) (((f) >> 0) & 0xfff0)
#define GEN6_KERNEL(f) (((f) >> 16) & 0xf)
#define GEN6_VERTEX(f) (((f) >> 0) & 0xf)
#define GEN6_SET_FLAGS(S, B, K, V)  (((S) | (K)) << 16 | ((B) | (V)))

#define OUT_BATCH(v) batch_emit(sna, v)
#define OUT_VERTEX(x,y) vertex_emit_2s(sna, x,y)
#define OUT_VERTEX_F(v) vertex_emit(sna, v)

static inline bool too_large(int width, int height)
{
	return width > GEN6_MAX_SIZE || height > GEN6_MAX_SIZE;
}

static uint32_t gen6_get_blend(int op,
			       bool has_component_alpha,
			       uint32_t dst_format)
{
	uint32_t src, dst;

	src = gen6_blend_op[op].src_blend;
	dst = gen6_blend_op[op].dst_blend;

	/* If there's no dst alpha channel, adjust the blend op so that
	 * we'll treat it always as 1.
	 */
	if (PICT_FORMAT_A(dst_format) == 0) {
		if (src == GEN6_BLENDFACTOR_DST_ALPHA)
			src = GEN6_BLENDFACTOR_ONE;
		else if (src == GEN6_BLENDFACTOR_INV_DST_ALPHA)
			src = GEN6_BLENDFACTOR_ZERO;
	}

	/* If the source alpha is being used, then we should only be in a
	 * case where the source blend factor is 0, and the source blend
	 * value is the mask channels multiplied by the source picture's alpha.
	 */
	if (has_component_alpha && gen6_blend_op[op].src_alpha) {
		if (dst == GEN6_BLENDFACTOR_SRC_ALPHA)
			dst = GEN6_BLENDFACTOR_SRC_COLOR;
		else if (dst == GEN6_BLENDFACTOR_INV_SRC_ALPHA)
			dst = GEN6_BLENDFACTOR_INV_SRC_COLOR;
	}

	DBG(("blend op=%d, dst=%x [A=%d] => src=%d, dst=%d => offset=%x\n",
	     op, dst_format, PICT_FORMAT_A(dst_format),
	     src, dst, (int)BLEND_OFFSET(src, dst)));
	return BLEND_OFFSET(src, dst);
}

static uint32_t gen6_get_card_format(PictFormat format)
{
	switch (format) {
	default:
		return -1;
	case PICT_a8r8g8b8:
		return GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;
	case PICT_x8r8g8b8:
		return GEN6_SURFACEFORMAT_B8G8R8X8_UNORM;
	case PICT_a8b8g8r8:
		return GEN6_SURFACEFORMAT_R8G8B8A8_UNORM;
	case PICT_x8b8g8r8:
		return GEN6_SURFACEFORMAT_R8G8B8X8_UNORM;
	case PICT_a2r10g10b10:
		return GEN6_SURFACEFORMAT_B10G10R10A2_UNORM;
	case PICT_x2r10g10b10:
		return GEN6_SURFACEFORMAT_B10G10R10X2_UNORM;
	case PICT_r8g8b8:
		return GEN6_SURFACEFORMAT_R8G8B8_UNORM;
	case PICT_r5g6b5:
		return GEN6_SURFACEFORMAT_B5G6R5_UNORM;
	case PICT_a1r5g5b5:
		return GEN6_SURFACEFORMAT_B5G5R5A1_UNORM;
	case PICT_a8:
		return GEN6_SURFACEFORMAT_A8_UNORM;
	case PICT_a4r4g4b4:
		return GEN6_SURFACEFORMAT_B4G4R4A4_UNORM;
	}
}

static uint32_t gen6_get_dest_format(PictFormat format)
{
	switch (format) {
	default:
		return -1;
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
		return GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;
	case PICT_a8b8g8r8:
	case PICT_x8b8g8r8:
		return GEN6_SURFACEFORMAT_R8G8B8A8_UNORM;
	case PICT_a2r10g10b10:
	case PICT_x2r10g10b10:
		return GEN6_SURFACEFORMAT_B10G10R10A2_UNORM;
	case PICT_r5g6b5:
		return GEN6_SURFACEFORMAT_B5G6R5_UNORM;
	case PICT_x1r5g5b5:
	case PICT_a1r5g5b5:
		return GEN6_SURFACEFORMAT_B5G5R5A1_UNORM;
	case PICT_a8:
		return GEN6_SURFACEFORMAT_A8_UNORM;
	case PICT_a4r4g4b4:
	case PICT_x4r4g4b4:
		return GEN6_SURFACEFORMAT_B4G4R4A4_UNORM;
	}
}

static bool gen6_check_dst_format(PictFormat format)
{
	if (gen6_get_dest_format(format) != -1)
		return true;

	DBG(("%s: unhandled format: %x\n", __FUNCTION__, (int)format));
	return false;
}

static bool gen6_check_format(uint32_t format)
{
	if (gen6_get_card_format(format) != -1)
		return true;

	DBG(("%s: unhandled format: %x\n", __FUNCTION__, (int)format));
	return false;
}

static uint32_t gen6_filter(uint32_t filter)
{
	switch (filter) {
	default:
		assert(0);
	case PictFilterNearest:
		return SAMPLER_FILTER_NEAREST;
	case PictFilterBilinear:
		return SAMPLER_FILTER_BILINEAR;
	}
}

static uint32_t gen6_check_filter(PicturePtr picture)
{
	switch (picture->filter) {
	case PictFilterNearest:
	case PictFilterBilinear:
		return true;
	default:
		return false;
	}
}

static uint32_t gen6_repeat(uint32_t repeat)
{
	switch (repeat) {
	default:
		assert(0);
	case RepeatNone:
		return SAMPLER_EXTEND_NONE;
	case RepeatNormal:
		return SAMPLER_EXTEND_REPEAT;
	case RepeatPad:
		return SAMPLER_EXTEND_PAD;
	case RepeatReflect:
		return SAMPLER_EXTEND_REFLECT;
	}
}

static bool gen6_check_repeat(PicturePtr picture)
{
	if (!picture->repeat)
		return true;

	switch (picture->repeatType) {
	case RepeatNone:
	case RepeatNormal:
	case RepeatPad:
	case RepeatReflect:
		return true;
	default:
		return false;
	}
}

static int
gen6_choose_composite_kernel(int op, bool has_mask, bool is_ca, bool is_affine)
{
	int base;

	if (has_mask) {
		if (is_ca) {
			if (gen6_blend_op[op].src_alpha)
				base = GEN6_WM_KERNEL_MASKSA;
			else
				base = GEN6_WM_KERNEL_MASKCA;
		} else
			base = GEN6_WM_KERNEL_MASK;
	} else
		base = GEN6_WM_KERNEL_NOMASK;

	return base + !is_affine;
}

static void
gen6_emit_urb(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_URB | (3 - 2));
	OUT_BATCH(((1 - 1) << GEN6_3DSTATE_URB_VS_SIZE_SHIFT) |
		  (sna->render_state.gen6.info->urb.max_vs_entries << GEN6_3DSTATE_URB_VS_ENTRIES_SHIFT)); /* at least 24 on GEN6 */
	OUT_BATCH((0 << GEN6_3DSTATE_URB_GS_SIZE_SHIFT) |
		  (0 << GEN6_3DSTATE_URB_GS_ENTRIES_SHIFT)); /* no GS thread */
}

static void
gen6_emit_state_base_address(struct sna *sna)
{
	OUT_BATCH(GEN6_STATE_BASE_ADDRESS | (10 - 2));
	OUT_BATCH(0); /* general */
	OUT_BATCH(kgem_add_reloc(&sna->kgem, /* surface */
				 sna->kgem.nbatch,
				 NULL,
				 I915_GEM_DOMAIN_INSTRUCTION << 16,
				 BASE_ADDRESS_MODIFY));
	OUT_BATCH(kgem_add_reloc(&sna->kgem, /* instruction */
				 sna->kgem.nbatch,
				 sna->render_state.gen6.general_bo,
				 I915_GEM_DOMAIN_INSTRUCTION << 16,
				 BASE_ADDRESS_MODIFY));
	OUT_BATCH(0); /* indirect */
	OUT_BATCH(kgem_add_reloc(&sna->kgem,
				 sna->kgem.nbatch,
				 sna->render_state.gen6.general_bo,
				 I915_GEM_DOMAIN_INSTRUCTION << 16,
				 BASE_ADDRESS_MODIFY));

	/* upper bounds, disable */
	OUT_BATCH(0);
	OUT_BATCH(BASE_ADDRESS_MODIFY);
	OUT_BATCH(0);
	OUT_BATCH(BASE_ADDRESS_MODIFY);
}

static void
gen6_emit_viewports(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_VIEWPORT_STATE_POINTERS |
		  GEN6_3DSTATE_VIEWPORT_STATE_MODIFY_CC |
		  (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(sna->render_state.gen6.cc_vp);
}

static void
gen6_emit_vs(struct sna *sna)
{
	/* disable VS constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_VS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_VS | (6 - 2));
	OUT_BATCH(0); /* no VS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
}

static void
gen6_emit_gs(struct sna *sna)
{
	/* disable GS constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_GS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_GS | (7 - 2));
	OUT_BATCH(0); /* no GS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
}

static void
gen6_emit_clip(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_CLIP | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
	OUT_BATCH(0);
}

static void
gen6_emit_wm_constants(struct sna *sna)
{
	/* disable WM constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_PS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen6_emit_null_depth_buffer(struct sna *sna)
{
	OUT_BATCH(GEN6_3DSTATE_DEPTH_BUFFER | (7 - 2));
	OUT_BATCH(GEN6_SURFACE_NULL << GEN6_3DSTATE_DEPTH_BUFFER_TYPE_SHIFT |
		  GEN6_DEPTHFORMAT_D32_FLOAT << GEN6_3DSTATE_DEPTH_BUFFER_FORMAT_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_CLEAR_PARAMS | (2 - 2));
	OUT_BATCH(0);
}

static void
gen6_emit_invariant(struct sna *sna)
{
	OUT_BATCH(GEN6_PIPELINE_SELECT | PIPELINE_SELECT_3D);

	OUT_BATCH(GEN6_3DSTATE_MULTISAMPLE | (3 - 2));
	OUT_BATCH(GEN6_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
		  GEN6_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_SAMPLE_MASK | (2 - 2));
	OUT_BATCH(1);

	gen6_emit_urb(sna);

	gen6_emit_state_base_address(sna);

	gen6_emit_viewports(sna);
	gen6_emit_vs(sna);
	gen6_emit_gs(sna);
	gen6_emit_clip(sna);
	gen6_emit_wm_constants(sna);
	gen6_emit_null_depth_buffer(sna);

	sna->render_state.gen6.needs_invariant = false;
}

static bool
gen6_emit_cc(struct sna *sna, int blend)
{
	struct gen6_render_state *render = &sna->render_state.gen6;

	if (render->blend == blend)
		return blend != NO_BLEND;

	DBG(("%s: blend = %x\n", __FUNCTION__, blend));

	OUT_BATCH(GEN6_3DSTATE_CC_STATE_POINTERS | (4 - 2));
	OUT_BATCH((render->cc_blend + blend) | 1);
	if (render->blend == (unsigned)-1) {
		OUT_BATCH(1);
		OUT_BATCH(1);
	} else {
		OUT_BATCH(0);
		OUT_BATCH(0);
	}

	render->blend = blend;
	return blend != NO_BLEND;
}

static void
gen6_emit_sampler(struct sna *sna, uint32_t state)
{
	if (sna->render_state.gen6.samplers == state)
		return;

	sna->render_state.gen6.samplers = state;

	DBG(("%s: sampler = %x\n", __FUNCTION__, state));

	OUT_BATCH(GEN6_3DSTATE_SAMPLER_STATE_POINTERS |
		  GEN6_3DSTATE_SAMPLER_STATE_MODIFY_PS |
		  (4 - 2));
	OUT_BATCH(0); /* VS */
	OUT_BATCH(0); /* GS */
	OUT_BATCH(sna->render_state.gen6.wm_state + state);
}

static void
gen6_emit_sf(struct sna *sna, bool has_mask)
{
	int num_sf_outputs = has_mask ? 2 : 1;

	if (sna->render_state.gen6.num_sf_outputs == num_sf_outputs)
		return;

	DBG(("%s: num_sf_outputs=%d, read_length=%d, read_offset=%d\n",
	     __FUNCTION__, num_sf_outputs, 1, 0));

	sna->render_state.gen6.num_sf_outputs = num_sf_outputs;

	OUT_BATCH(GEN6_3DSTATE_SF | (20 - 2));
	OUT_BATCH(num_sf_outputs << GEN6_3DSTATE_SF_NUM_OUTPUTS_SHIFT |
		  1 << GEN6_3DSTATE_SF_URB_ENTRY_READ_LENGTH_SHIFT |
		  1 << GEN6_3DSTATE_SF_URB_ENTRY_READ_OFFSET_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(GEN6_3DSTATE_SF_CULL_NONE);
	OUT_BATCH(2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT); /* DW4 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW9 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW14 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW19 */
}

static void
gen6_emit_wm(struct sna *sna, unsigned int kernel, bool has_mask)
{
	const uint32_t *kernels;

	if (sna->render_state.gen6.kernel == kernel)
		return;

	sna->render_state.gen6.kernel = kernel;
	kernels = sna->render_state.gen6.wm_kernel[kernel];

	DBG(("%s: switching to %s, num_surfaces=%d (8-pixel? %d, 16-pixel? %d,32-pixel? %d)\n",
	     __FUNCTION__,
	     wm_kernels[kernel].name, wm_kernels[kernel].num_surfaces,
	    kernels[0], kernels[1], kernels[2]));

	OUT_BATCH(GEN6_3DSTATE_WM | (9 - 2));
	OUT_BATCH(kernels[0] ?: kernels[1] ?: kernels[2]);
	OUT_BATCH(1 << GEN6_3DSTATE_WM_SAMPLER_COUNT_SHIFT |
		  wm_kernels[kernel].num_surfaces << GEN6_3DSTATE_WM_BINDING_TABLE_ENTRY_COUNT_SHIFT);
	OUT_BATCH(0); /* scratch space */
	OUT_BATCH((kernels[0] ? 4 : kernels[1] ? 6 : 8) << GEN6_3DSTATE_WM_DISPATCH_0_START_GRF_SHIFT |
		  8 << GEN6_3DSTATE_WM_DISPATCH_1_START_GRF_SHIFT |
		  6 << GEN6_3DSTATE_WM_DISPATCH_2_START_GRF_SHIFT);
	OUT_BATCH((sna->render_state.gen6.info->max_wm_threads - 1) << GEN6_3DSTATE_WM_MAX_THREADS_SHIFT |
		  (kernels[0] ? GEN6_3DSTATE_WM_8_DISPATCH_ENABLE : 0) |
		  (kernels[1] ? GEN6_3DSTATE_WM_16_DISPATCH_ENABLE : 0) |
		  (kernels[2] ? GEN6_3DSTATE_WM_32_DISPATCH_ENABLE : 0) |
		  GEN6_3DSTATE_WM_DISPATCH_ENABLE);
	OUT_BATCH((1 + has_mask) << GEN6_3DSTATE_WM_NUM_SF_OUTPUTS_SHIFT |
		  GEN6_3DSTATE_WM_PERSPECTIVE_PIXEL_BARYCENTRIC);
	OUT_BATCH(kernels[2]);
	OUT_BATCH(kernels[1]);
}

static bool
gen6_emit_binding_table(struct sna *sna, uint16_t offset)
{
	if (sna->render_state.gen6.surface_table == offset)
		return false;

	/* Binding table pointers */
	OUT_BATCH(GEN6_3DSTATE_BINDING_TABLE_POINTERS |
		  GEN6_3DSTATE_BINDING_TABLE_MODIFY_PS |
		  (4 - 2));
	OUT_BATCH(0);		/* vs */
	OUT_BATCH(0);		/* gs */
	/* Only the PS uses the binding table */
	OUT_BATCH(offset*4);

	sna->render_state.gen6.surface_table = offset;
	return true;
}

static bool
gen6_emit_drawing_rectangle(struct sna *sna,
			    const struct sna_composite_op *op)
{
	uint32_t limit = (op->dst.height - 1) << 16 | (op->dst.width - 1);
	uint32_t offset = (uint16_t)op->dst.y << 16 | (uint16_t)op->dst.x;

	assert(!too_large(op->dst.x, op->dst.y));
	assert(!too_large(op->dst.width, op->dst.height));

	if (sna->render_state.gen6.drawrect_limit  == limit &&
	    sna->render_state.gen6.drawrect_offset == offset)
		return false;

	/* [DevSNB-C+{W/A}] Before any depth stall flush (including those
	 * produced by non-pipelined state commands), software needs to first
	 * send a PIPE_CONTROL with no bits set except Post-Sync Operation !=
	 * 0.
	 *
	 * [Dev-SNB{W/A}]: Pipe-control with CS-stall bit set must be sent
	 * BEFORE the pipe-control with a post-sync op and no write-cache
	 * flushes.
	 */
	if (!sna->render_state.gen6.first_state_packet) {
		OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
		OUT_BATCH(GEN6_PIPE_CONTROL_CS_STALL |
			  GEN6_PIPE_CONTROL_STALL_AT_SCOREBOARD);
		OUT_BATCH(0);
		OUT_BATCH(0);
	}

	OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN6_PIPE_CONTROL_WRITE_TIME);
	OUT_BATCH(kgem_add_reloc(&sna->kgem, sna->kgem.nbatch,
				 sna->render_state.gen6.general_bo,
				 I915_GEM_DOMAIN_INSTRUCTION << 16 |
				 I915_GEM_DOMAIN_INSTRUCTION,
				 64));
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_DRAWING_RECTANGLE | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(limit);
	OUT_BATCH(offset);

	sna->render_state.gen6.drawrect_offset = offset;
	sna->render_state.gen6.drawrect_limit = limit;
	return true;
}

static void
gen6_emit_vertex_elements(struct sna *sna,
			  const struct sna_composite_op *op)
{
	/*
	 * vertex data in vertex buffer
	 *    position: (x, y)
	 *    texture coordinate 0: (u0, v0) if (is_affine is true) else (u0, v0, w0)
	 *    texture coordinate 1 if (has_mask is true): same as above
	 */
	struct gen6_render_state *render = &sna->render_state.gen6;
	uint32_t src_format, dw, offset;
	int id = GEN6_VERTEX(op->u.gen6.flags);
	bool has_mask;

	DBG(("%s: setup id=%d\n", __FUNCTION__, id));

	if (render->ve_id == id)
		return;
	render->ve_id = id;

	if (id == VERTEX_2s2s) {
		DBG(("%s: setup COPY\n", __FUNCTION__));

		OUT_BATCH(GEN6_3DSTATE_VERTEX_ELEMENTS |
			  ((2 * (1 + 2)) + 1 - 2));

		OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
			  GEN6_SURFACEFORMAT_R32G32B32A32_FLOAT << VE0_FORMAT_SHIFT |
			  0 << VE0_OFFSET_SHIFT);
		OUT_BATCH(GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_0_SHIFT |
			  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_1_SHIFT |
			  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT |
			  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_3_SHIFT);

		/* x,y */
		OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
			  GEN6_SURFACEFORMAT_R16G16_SSCALED << VE0_FORMAT_SHIFT |
			  0 << VE0_OFFSET_SHIFT);
		OUT_BATCH(GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT |
			  GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT |
			  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT |
			  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT);

		/* u0, v0, w0 */
		OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
			  GEN6_SURFACEFORMAT_R16G16_SSCALED << VE0_FORMAT_SHIFT |
			  4 << VE0_OFFSET_SHIFT);
		OUT_BATCH(GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT |
			  GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT |
			  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT |
			  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT);
		return;
	}

	/* The VUE layout
	 *    dword 0-3: pad (0.0, 0.0, 0.0. 0.0)
	 *    dword 4-7: position (x, y, 1.0, 1.0),
	 *    dword 8-11: texture coordinate 0 (u0, v0, w0, 1.0)
	 *    dword 12-15: texture coordinate 1 (u1, v1, w1, 1.0)
	 *
	 * dword 4-15 are fetched from vertex buffer
	 */
	has_mask = (id >> 2) != 0;
	OUT_BATCH(GEN6_3DSTATE_VERTEX_ELEMENTS |
		((2 * (3 + has_mask)) + 1 - 2));

	OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
		  GEN6_SURFACEFORMAT_R32G32B32A32_FLOAT << VE0_FORMAT_SHIFT |
		  0 << VE0_OFFSET_SHIFT);
	OUT_BATCH(GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_0_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_1_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_3_SHIFT);

	/* x,y */
	OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
		  GEN6_SURFACEFORMAT_R16G16_SSCALED << VE0_FORMAT_SHIFT |
		  0 << VE0_OFFSET_SHIFT);
	OUT_BATCH(GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT |
		  GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT |
		  GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT |
		  GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT);
	offset = 4;

	/* u0, v0, w0 */
	DBG(("%s: first channel %d floats, offset=%d\n", __FUNCTION__, id & 3, offset));
	dw = GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT;
	switch (id & 3) {
	case 1:
		src_format = GEN6_SURFACEFORMAT_R32_FLOAT;
		dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT;
		break;
	default:
		assert(0);
	case 2:
		src_format = GEN6_SURFACEFORMAT_R32G32_FLOAT;
		dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT;
		break;
	case 3:
		src_format = GEN6_SURFACEFORMAT_R32G32B32_FLOAT;
		dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT;
		dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT;
		dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_2_SHIFT;
		break;
	}
	OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
		  src_format << VE0_FORMAT_SHIFT |
		  offset << VE0_OFFSET_SHIFT);
	OUT_BATCH(dw);
	offset += (id & 3) * sizeof(float);

	/* u1, v1, w1 */
	if (has_mask) {
		DBG(("%s: second channel %d floats, offset=%d\n", __FUNCTION__, (id >> 2) & 3, offset));
		dw = GEN6_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT;
		switch ((id >> 2) & 3) {
		case 1:
			src_format = GEN6_SURFACEFORMAT_R32_FLOAT;
			dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT;
			dw |= GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_1_SHIFT;
			dw |= GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT;
			break;
		default:
			assert(0);
		case 2:
			src_format = GEN6_SURFACEFORMAT_R32G32_FLOAT;
			dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT;
			dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT;
			dw |= GEN6_VFCOMPONENT_STORE_0 << VE1_VFCOMPONENT_2_SHIFT;
			break;
		case 3:
			src_format = GEN6_SURFACEFORMAT_R32G32B32_FLOAT;
			dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT;
			dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT;
			dw |= GEN6_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_2_SHIFT;
			break;
		}
		OUT_BATCH(id << VE0_VERTEX_BUFFER_INDEX_SHIFT | VE0_VALID |
			  src_format << VE0_FORMAT_SHIFT |
			  offset << VE0_OFFSET_SHIFT);
		OUT_BATCH(dw);
	}
}

static void
gen6_emit_flush(struct sna *sna)
{
	OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(GEN6_PIPE_CONTROL_WC_FLUSH |
		  GEN6_PIPE_CONTROL_TC_FLUSH |
		  GEN6_PIPE_CONTROL_CS_STALL);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

static void
gen6_emit_state(struct sna *sna,
		const struct sna_composite_op *op,
		uint16_t wm_binding_table)
{
	bool need_stall = wm_binding_table & 1;

	if (gen6_emit_cc(sna, GEN6_BLEND(op->u.gen6.flags)))
		need_stall = false;
	gen6_emit_sampler(sna, GEN6_SAMPLER(op->u.gen6.flags));
	gen6_emit_sf(sna, GEN6_VERTEX(op->u.gen6.flags) >> 2);
	gen6_emit_wm(sna, GEN6_KERNEL(op->u.gen6.flags), GEN6_VERTEX(op->u.gen6.flags) >> 2);
	gen6_emit_vertex_elements(sna, op);

	need_stall |= gen6_emit_binding_table(sna, wm_binding_table & ~1);
	if (gen6_emit_drawing_rectangle(sna, op))
		need_stall = false;
	if (kgem_bo_is_dirty(op->src.bo) || kgem_bo_is_dirty(op->mask.bo)) {
		gen6_emit_flush(sna);
		kgem_clear_dirty(&sna->kgem);
		if (op->dst.bo->exec)
			kgem_bo_mark_dirty(op->dst.bo);
		need_stall = false;
	}
	if (need_stall) {
		OUT_BATCH(GEN6_PIPE_CONTROL | (4 - 2));
		OUT_BATCH(GEN6_PIPE_CONTROL_CS_STALL |
			  GEN6_PIPE_CONTROL_STALL_AT_SCOREBOARD);
		OUT_BATCH(0);
		OUT_BATCH(0);
	}
	sna->render_state.gen6.first_state_packet = false;
}

static void gen6_magic_ca_pass(struct sna *sna,
			       const struct sna_composite_op *op)
{
	struct gen6_render_state *state = &sna->render_state.gen6;

	if (!op->need_magic_ca_pass)
		return;

	DBG(("%s: CA fixup (%d -> %d)\n", __FUNCTION__,
	     sna->render.vertex_start, sna->render.vertex_index));

	gen6_emit_flush(sna);

	gen6_emit_cc(sna, gen6_get_blend(PictOpAdd, true, op->dst.format));
	gen6_emit_wm(sna,
		     gen6_choose_composite_kernel(PictOpAdd,
						  true, true,
						  op->is_affine),
		     true);

	OUT_BATCH(GEN6_3DPRIMITIVE |
		  GEN6_3DPRIMITIVE_VERTEX_SEQUENTIAL |
		  _3DPRIM_RECTLIST << GEN6_3DPRIMITIVE_TOPOLOGY_SHIFT |
		  0 << 9 |
		  4);
	OUT_BATCH(sna->render.vertex_index - sna->render.vertex_start);
	OUT_BATCH(sna->render.vertex_start);
	OUT_BATCH(1);	/* single instance */
	OUT_BATCH(0);	/* start instance location */
	OUT_BATCH(0);	/* index buffer offset, ignored */

	state->last_primitive = sna->kgem.nbatch;
}

static void gen6_vertex_flush(struct sna *sna)
{
	assert(sna->render_state.gen6.vertex_offset);

	DBG(("%s[%x] = %d\n", __FUNCTION__,
	     4*sna->render_state.gen6.vertex_offset,
	     sna->render.vertex_index - sna->render.vertex_start));
	sna->kgem.batch[sna->render_state.gen6.vertex_offset] =
		sna->render.vertex_index - sna->render.vertex_start;
	sna->render_state.gen6.vertex_offset = 0;
}

static int gen6_vertex_finish(struct sna *sna)
{
	struct kgem_bo *bo;
	unsigned int i;

	DBG(("%s: used=%d / %d\n", __FUNCTION__,
	     sna->render.vertex_used, sna->render.vertex_size));
	assert(sna->render.vertex_used);
	assert(sna->render.nvertex_reloc);

	/* Note: we only need dword alignment (currently) */

	bo = sna->render.vbo;
	if (bo) {
		if (sna->render_state.gen6.vertex_offset)
			gen6_vertex_flush(sna);

		for (i = 0; i < sna->render.nvertex_reloc; i++) {
			DBG(("%s: reloc[%d] = %d\n", __FUNCTION__,
			     i, sna->render.vertex_reloc[i]));

			sna->kgem.batch[sna->render.vertex_reloc[i]] =
				kgem_add_reloc(&sna->kgem,
					       sna->render.vertex_reloc[i], bo,
					       I915_GEM_DOMAIN_VERTEX << 16,
					       0);
		}

		sna->render.nvertex_reloc = 0;
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
		sna->render.vbo = NULL;
		sna->render_state.gen6.vb_id = 0;

		kgem_bo_destroy(&sna->kgem, bo);
	}

	sna->render.vertices = NULL;
	sna->render.vbo = kgem_create_linear(&sna->kgem,
					     256*1024, CREATE_GTT_MAP);
	if (sna->render.vbo)
		sna->render.vertices = kgem_bo_map(&sna->kgem, sna->render.vbo);
	if (sna->render.vertices == NULL) {
		if (sna->render.vbo)
			kgem_bo_destroy(&sna->kgem, sna->render.vbo);
		sna->render.vbo = NULL;
		return 0;
	}

	DBG(("%s: create vbo handle=%d\n", __FUNCTION__, sna->render.vbo->handle));

	kgem_bo_sync__cpu(&sna->kgem, sna->render.vbo);
	if (sna->render.vertex_used) {
		DBG(("%s: copying initial buffer x %d to handle=%d\n",
		     __FUNCTION__,
		     sna->render.vertex_used,
		     sna->render.vbo->handle));
		memcpy(sna->render.vertices,
		       sna->render.vertex_data,
		       sizeof(float)*sna->render.vertex_used);
	}
	sna->render.vertex_size = 64 * 1024 - 1;
	return sna->render.vertex_size - sna->render.vertex_used;
}

static void gen6_vertex_close(struct sna *sna)
{
	struct kgem_bo *bo, *free_bo = NULL;
	unsigned int i, delta = 0;

	assert(sna->render_state.gen6.vertex_offset == 0);

	if (!sna->render_state.gen6.vb_id)
		return;

	DBG(("%s: used=%d, vbo active? %d\n",
	     __FUNCTION__, sna->render.vertex_used, sna->render.vbo ? sna->render.vbo->handle : 0));

	bo = sna->render.vbo;
	if (bo) {
		if (sna->render.vertex_size - sna->render.vertex_used < 64) {
			DBG(("%s: discarding vbo (full), handle=%d\n", __FUNCTION__, sna->render.vbo->handle));
			sna->render.vbo = NULL;
			sna->render.vertices = sna->render.vertex_data;
			sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
			free_bo = bo;
		}
	} else {
		if (sna->kgem.nbatch + sna->render.vertex_used <= sna->kgem.surface) {
			DBG(("%s: copy to batch: %d @ %d\n", __FUNCTION__,
			     sna->render.vertex_used, sna->kgem.nbatch));
			memcpy(sna->kgem.batch + sna->kgem.nbatch,
			       sna->render.vertex_data,
			       sna->render.vertex_used * 4);
			delta = sna->kgem.nbatch * 4;
			bo = NULL;
			sna->kgem.nbatch += sna->render.vertex_used;
		} else {
			bo = kgem_create_linear(&sna->kgem,
						4*sna->render.vertex_used, 0);
			if (bo && !kgem_bo_write(&sna->kgem, bo,
						 sna->render.vertex_data,
						 4*sna->render.vertex_used)) {
				kgem_bo_destroy(&sna->kgem, bo);
				bo = NULL;
			}
			DBG(("%s: new vbo: %d\n", __FUNCTION__,
			     sna->render.vertex_used));
			free_bo = bo;
		}
	}

	assert(sna->render.nvertex_reloc);
	for (i = 0; i < sna->render.nvertex_reloc; i++) {
		DBG(("%s: reloc[%d] = %d\n", __FUNCTION__,
		     i, sna->render.vertex_reloc[i]));

		sna->kgem.batch[sna->render.vertex_reloc[i]] =
			kgem_add_reloc(&sna->kgem,
				       sna->render.vertex_reloc[i], bo,
				       I915_GEM_DOMAIN_VERTEX << 16,
				       delta);
	}
	sna->render.nvertex_reloc = 0;

	if (sna->render.vbo == NULL) {
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
		assert(sna->render.vertices == sna->render.vertex_data);
		assert(sna->render.vertex_size == ARRAY_SIZE(sna->render.vertex_data));
	}

	if (free_bo)
		kgem_bo_destroy(&sna->kgem, free_bo);
}

typedef struct gen6_surface_state_padded {
	struct gen6_surface_state state;
	char pad[32 - sizeof(struct gen6_surface_state)];
} gen6_surface_state_padded;

static void null_create(struct sna_static_stream *stream)
{
	/* A bunch of zeros useful for legacy border color and depth-stencil */
	sna_static_stream_map(stream, 64, 64);
}

static void scratch_create(struct sna_static_stream *stream)
{
	/* 64 bytes of scratch space for random writes, such as
	 * the pipe-control w/a.
	 */
	sna_static_stream_map(stream, 64, 64);
}

static void
sampler_state_init(struct gen6_sampler_state *sampler_state,
		   sampler_filter_t filter,
		   sampler_extend_t extend)
{
	sampler_state->ss0.lod_preclamp = 1;	/* GL mode */

	/* We use the legacy mode to get the semantics specified by
	 * the Render extension. */
	sampler_state->ss0.border_color_mode = GEN6_BORDER_COLOR_MODE_LEGACY;

	switch (filter) {
	default:
	case SAMPLER_FILTER_NEAREST:
		sampler_state->ss0.min_filter = GEN6_MAPFILTER_NEAREST;
		sampler_state->ss0.mag_filter = GEN6_MAPFILTER_NEAREST;
		break;
	case SAMPLER_FILTER_BILINEAR:
		sampler_state->ss0.min_filter = GEN6_MAPFILTER_LINEAR;
		sampler_state->ss0.mag_filter = GEN6_MAPFILTER_LINEAR;
		break;
	}

	switch (extend) {
	default:
	case SAMPLER_EXTEND_NONE:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_CLAMP_BORDER;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_CLAMP_BORDER;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_CLAMP_BORDER;
		break;
	case SAMPLER_EXTEND_REPEAT:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_WRAP;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_WRAP;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_WRAP;
		break;
	case SAMPLER_EXTEND_PAD:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_CLAMP;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_CLAMP;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_CLAMP;
		break;
	case SAMPLER_EXTEND_REFLECT:
		sampler_state->ss1.r_wrap_mode = GEN6_TEXCOORDMODE_MIRROR;
		sampler_state->ss1.s_wrap_mode = GEN6_TEXCOORDMODE_MIRROR;
		sampler_state->ss1.t_wrap_mode = GEN6_TEXCOORDMODE_MIRROR;
		break;
	}
}

static void
sampler_copy_init(struct gen6_sampler_state *ss)
{
	sampler_state_init(ss, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE);
	ss->ss3.non_normalized_coord = 1;

	sampler_state_init(ss+1, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE);
}

static void
sampler_fill_init(struct gen6_sampler_state *ss)
{
	sampler_state_init(ss, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_REPEAT);
	ss->ss3.non_normalized_coord = 1;

	sampler_state_init(ss+1, SAMPLER_FILTER_NEAREST, SAMPLER_EXTEND_NONE);
}

static uint32_t gen6_create_cc_viewport(struct sna_static_stream *stream)
{
	struct gen6_cc_viewport vp;

	vp.min_depth = -1.e35;
	vp.max_depth = 1.e35;

	return sna_static_stream_add(stream, &vp, sizeof(vp), 32);
}

static uint32_t
gen6_tiling_bits(uint32_t tiling)
{
	switch (tiling) {
	default: assert(0);
	case I915_TILING_NONE: return 0;
	case I915_TILING_X: return GEN6_SURFACE_TILED;
	case I915_TILING_Y: return GEN6_SURFACE_TILED | GEN6_SURFACE_TILED_Y;
	}
}

/**
 * Sets up the common fields for a surface state buffer for the given
 * picture in the given surface state buffer.
 */
static int
gen6_bind_bo(struct sna *sna,
	     struct kgem_bo *bo,
	     uint32_t width,
	     uint32_t height,
	     uint32_t format,
	     bool is_dst)
{
	uint32_t *ss;
	uint32_t domains;
	uint16_t offset;
	uint32_t is_scanout = is_dst && bo->scanout;

	/* After the first bind, we manage the cache domains within the batch */
	offset = kgem_bo_get_binding(bo, format | is_scanout << 31);
	if (offset) {
		DBG(("[%x]  bo(handle=%d), format=%d, reuse %s binding\n",
		     offset, bo->handle, format,
		     is_dst ? "render" : "sampler"));
		if (is_dst)
			kgem_bo_mark_dirty(bo);
		return offset * sizeof(uint32_t);
	}

	offset = sna->kgem.surface -=
		sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
	ss = sna->kgem.batch + offset;
	ss[0] = (GEN6_SURFACE_2D << GEN6_SURFACE_TYPE_SHIFT |
		 GEN6_SURFACE_BLEND_ENABLED |
		 format << GEN6_SURFACE_FORMAT_SHIFT);
	if (is_dst)
		domains = I915_GEM_DOMAIN_RENDER << 16 |I915_GEM_DOMAIN_RENDER;
	else
		domains = I915_GEM_DOMAIN_SAMPLER << 16;
	ss[1] = kgem_add_reloc(&sna->kgem, offset + 1, bo, domains, 0);
	ss[2] = ((width - 1)  << GEN6_SURFACE_WIDTH_SHIFT |
		 (height - 1) << GEN6_SURFACE_HEIGHT_SHIFT);
	assert(bo->pitch <= (1 << 18));
	ss[3] = (gen6_tiling_bits(bo->tiling) |
		 (bo->pitch - 1) << GEN6_SURFACE_PITCH_SHIFT);
	ss[4] = 0;
	ss[5] = is_scanout ? 0 : 3 << 16;

	kgem_bo_set_binding(bo, format, offset | is_scanout << 31);

	DBG(("[%x] bind bo(handle=%d, addr=%d), format=%d, width=%d, height=%d, pitch=%d, tiling=%d -> %s\n",
	     offset, bo->handle, ss[1],
	     format, width, height, bo->pitch, bo->tiling,
	     domains & 0xffff ? "render" : "sampler"));

	return offset * sizeof(uint32_t);
}

fastcall static void
gen6_emit_composite_primitive_solid(struct sna *sna,
				    const struct sna_composite_op *op,
				    const struct sna_composite_rectangles *r)
{
	float *v;
	union {
		struct sna_coordinate p;
		float f;
	} dst;

	DBG(("%s: [%d+9] = (%d, %d)x(%d, %d)\n", __FUNCTION__,
	     sna->render.vertex_used, r->dst.x, r->dst.y, r->width, r->height));

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;
	assert(sna->render.vertex_used <= sna->render.vertex_size);
	assert(!too_large(op->dst.x + r->dst.x + r->width,
			  op->dst.y + r->dst.y + r->height));

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	dst.p.x = r->dst.x;
	v[3] = dst.f;
	dst.p.y = r->dst.y;
	v[6] = dst.f;

	v[5] = v[2] = v[1] = 1.;
	v[8] = v[7] = v[4] = 0.;
}

fastcall static void
gen6_emit_composite_primitive_identity_source(struct sna *sna,
					      const struct sna_composite_op *op,
					      const struct sna_composite_rectangles *r)
{
	union {
		struct sna_coordinate p;
		float f;
	} dst;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	dst.p.x = r->dst.x;
	v[3] = dst.f;
	dst.p.y = r->dst.y;
	v[6] = dst.f;

	v[7] = v[4] = (r->src.x + op->src.offset[0]) * op->src.scale[0];
	v[1] = v[4] + r->width * op->src.scale[0];

	v[8] = (r->src.y + op->src.offset[1]) * op->src.scale[1];
	v[5] = v[2] = v[8] + r->height * op->src.scale[1];
}

fastcall static void
gen6_emit_composite_primitive_simple_source(struct sna *sna,
					    const struct sna_composite_op *op,
					    const struct sna_composite_rectangles *r)
{
	float *v;
	union {
		struct sna_coordinate p;
		float f;
	} dst;

	float xx = op->src.transform->matrix[0][0];
	float x0 = op->src.transform->matrix[0][2];
	float yy = op->src.transform->matrix[1][1];
	float y0 = op->src.transform->matrix[1][2];
	float sx = op->src.scale[0];
	float sy = op->src.scale[1];
	int16_t tx = op->src.offset[0];
	int16_t ty = op->src.offset[1];

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 3*3;

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	v[1] = ((r->src.x + r->width + tx) * xx + x0) * sx;
	v[5] = v[2] = ((r->src.y + r->height + ty) * yy + y0) * sy;

	dst.p.x = r->dst.x;
	v[3] = dst.f;
	v[7] = v[4] = ((r->src.x + tx) * xx + x0) * sx;

	dst.p.y = r->dst.y;
	v[6] = dst.f;
	v[8] = ((r->src.y + ty) * yy + y0) * sy;
}

fastcall static void
gen6_emit_composite_primitive_affine_source(struct sna *sna,
					    const struct sna_composite_op *op,
					    const struct sna_composite_rectangles *r)
{
	union {
		struct sna_coordinate p;
		float f;
	} dst;
	float *v;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 9;

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	_sna_get_transformed_coordinates(op->src.offset[0] + r->src.x + r->width,
					 op->src.offset[1] + r->src.y + r->height,
					 op->src.transform,
					 &v[1], &v[2]);
	v[1] *= op->src.scale[0];
	v[2] *= op->src.scale[1];

	dst.p.x = r->dst.x;
	v[3] = dst.f;
	_sna_get_transformed_coordinates(op->src.offset[0] + r->src.x,
					 op->src.offset[1] + r->src.y + r->height,
					 op->src.transform,
					 &v[4], &v[5]);
	v[4] *= op->src.scale[0];
	v[5] *= op->src.scale[1];

	dst.p.y = r->dst.y;
	v[6] = dst.f;
	_sna_get_transformed_coordinates(op->src.offset[0] + r->src.x,
					 op->src.offset[1] + r->src.y,
					 op->src.transform,
					 &v[7], &v[8]);
	v[7] *= op->src.scale[0];
	v[8] *= op->src.scale[1];
}

fastcall static void
gen6_emit_composite_primitive_identity_mask(struct sna *sna,
					    const struct sna_composite_op *op,
					    const struct sna_composite_rectangles *r)
{
	union {
		struct sna_coordinate p;
		float f;
	} dst;
	float msk_x, msk_y;
	float w, h;
	float *v;

	msk_x = r->mask.x + op->mask.offset[0];
	msk_y = r->mask.y + op->mask.offset[1];
	w = r->width;
	h = r->height;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	v[3] = (msk_x + w) * op->mask.scale[0];
	v[9] = v[4] = (msk_y + h) * op->mask.scale[1];

	dst.p.x = r->dst.x;
	v[5] = dst.f;
	v[13] = v[8] = msk_x * op->mask.scale[0];

	dst.p.y = r->dst.y;
	v[10] = dst.f;
	v[14] = msk_y * op->mask.scale[1];

	v[7] = v[2] = v[1] = 1;
	v[12] = v[11] = v[6] = 0;
}

fastcall static void
gen6_emit_composite_primitive_identity_source_mask(struct sna *sna,
						   const struct sna_composite_op *op,
						   const struct sna_composite_rectangles *r)
{
	union {
		struct sna_coordinate p;
		float f;
	} dst;
	float src_x, src_y;
	float msk_x, msk_y;
	float w, h;
	float *v;

	src_x = r->src.x + op->src.offset[0];
	src_y = r->src.y + op->src.offset[1];
	msk_x = r->mask.x + op->mask.offset[0];
	msk_y = r->mask.y + op->mask.offset[1];
	w = r->width;
	h = r->height;

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 15;

	dst.p.x = r->dst.x + r->width;
	dst.p.y = r->dst.y + r->height;
	v[0] = dst.f;
	v[1] = (src_x + w) * op->src.scale[0];
	v[2] = (src_y + h) * op->src.scale[1];
	v[3] = (msk_x + w) * op->mask.scale[0];
	v[4] = (msk_y + h) * op->mask.scale[1];

	dst.p.x = r->dst.x;
	v[5] = dst.f;
	v[6] = src_x * op->src.scale[0];
	v[7] = v[2];
	v[8] = msk_x * op->mask.scale[0];
	v[9] = v[4];

	dst.p.y = r->dst.y;
	v[10] = dst.f;
	v[11] = v[6];
	v[12] = src_y * op->src.scale[1];
	v[13] = v[8];
	v[14] = msk_y * op->mask.scale[1];
}

inline static void
gen6_emit_composite_texcoord(struct sna *sna,
			     const struct sna_composite_channel *channel,
			     int16_t x, int16_t y)
{
	x += channel->offset[0];
	y += channel->offset[1];

	if (channel->is_affine) {
		float s, t;

		sna_get_transformed_coordinates(x, y,
						channel->transform,
						&s, &t);
		OUT_VERTEX_F(s * channel->scale[0]);
		OUT_VERTEX_F(t * channel->scale[1]);
	} else {
		float s, t, w;

		sna_get_transformed_coordinates_3d(x, y,
						   channel->transform,
						   &s, &t, &w);
		OUT_VERTEX_F(s * channel->scale[0]);
		OUT_VERTEX_F(t * channel->scale[1]);
		OUT_VERTEX_F(w);
	}
}

static void
gen6_emit_composite_vertex(struct sna *sna,
			   const struct sna_composite_op *op,
			   int16_t srcX, int16_t srcY,
			   int16_t mskX, int16_t mskY,
			   int16_t dstX, int16_t dstY)
{
	OUT_VERTEX(dstX, dstY);
	gen6_emit_composite_texcoord(sna, &op->src, srcX, srcY);
	gen6_emit_composite_texcoord(sna, &op->mask, mskX, mskY);
}

fastcall static void
gen6_emit_composite_primitive(struct sna *sna,
			      const struct sna_composite_op *op,
			      const struct sna_composite_rectangles *r)
{
	gen6_emit_composite_vertex(sna, op,
				   r->src.x + r->width,  r->src.y + r->height,
				   r->mask.x + r->width, r->mask.y + r->height,
				   r->dst.x + r->width, r->dst.y + r->height);
	gen6_emit_composite_vertex(sna, op,
				   r->src.x,  r->src.y + r->height,
				   r->mask.x, r->mask.y + r->height,
				   r->dst.x,  r->dst.y + r->height);
	gen6_emit_composite_vertex(sna, op,
				   r->src.x,  r->src.y,
				   r->mask.x, r->mask.y,
				   r->dst.x,  r->dst.y);
}

static void gen6_emit_vertex_buffer(struct sna *sna,
				    const struct sna_composite_op *op)
{
	int id = GEN6_VERTEX(op->u.gen6.flags);

	OUT_BATCH(GEN6_3DSTATE_VERTEX_BUFFERS | 3);
	OUT_BATCH(id << VB0_BUFFER_INDEX_SHIFT | VB0_VERTEXDATA |
		  4*op->floats_per_vertex << VB0_BUFFER_PITCH_SHIFT);
	sna->render.vertex_reloc[sna->render.nvertex_reloc++] = sna->kgem.nbatch;
	OUT_BATCH(0);
	OUT_BATCH(~0); /* max address: disabled */
	OUT_BATCH(0);

	sna->render_state.gen6.vb_id |= 1 << id;
}

static void gen6_emit_primitive(struct sna *sna)
{
	if (sna->kgem.nbatch == sna->render_state.gen6.last_primitive) {
		DBG(("%s: continuing previous primitive, start=%d, index=%d\n",
		     __FUNCTION__,
		     sna->render.vertex_start,
		     sna->render.vertex_index));
		sna->render_state.gen6.vertex_offset = sna->kgem.nbatch - 5;
		return;
	}

	OUT_BATCH(GEN6_3DPRIMITIVE |
		  GEN6_3DPRIMITIVE_VERTEX_SEQUENTIAL |
		  _3DPRIM_RECTLIST << GEN6_3DPRIMITIVE_TOPOLOGY_SHIFT |
		  0 << 9 |
		  4);
	sna->render_state.gen6.vertex_offset = sna->kgem.nbatch;
	OUT_BATCH(0);	/* vertex count, to be filled in later */
	OUT_BATCH(sna->render.vertex_index);
	OUT_BATCH(1);	/* single instance */
	OUT_BATCH(0);	/* start instance location */
	OUT_BATCH(0);	/* index buffer offset, ignored */
	sna->render.vertex_start = sna->render.vertex_index;
	DBG(("%s: started new primitive: index=%d\n",
	     __FUNCTION__, sna->render.vertex_start));

	sna->render_state.gen6.last_primitive = sna->kgem.nbatch;
}

static bool gen6_rectangle_begin(struct sna *sna,
				 const struct sna_composite_op *op)
{
	int id = 1 << GEN6_VERTEX(op->u.gen6.flags);
	int ndwords;

	ndwords = op->need_magic_ca_pass ? 60 : 6;
	if ((sna->render_state.gen6.vb_id & id) == 0)
		ndwords += 5;
	if (!kgem_check_batch(&sna->kgem, ndwords))
		return false;

	if ((sna->render_state.gen6.vb_id & id) == 0)
		gen6_emit_vertex_buffer(sna, op);

	gen6_emit_primitive(sna);
	return true;
}

static int gen6_get_rectangles__flush(struct sna *sna,
				      const struct sna_composite_op *op)
{
	if (!kgem_check_batch(&sna->kgem, op->need_magic_ca_pass ? 65 : 5))
		return 0;
	if (!kgem_check_exec(&sna->kgem, 1))
		return 0;
	if (!kgem_check_reloc(&sna->kgem, 2))
		return 0;

	if (op->need_magic_ca_pass && sna->render.vbo)
		return 0;

	return gen6_vertex_finish(sna);
}

inline static int gen6_get_rectangles(struct sna *sna,
				      const struct sna_composite_op *op,
				      int want,
				      void (*emit_state)(struct sna *, const struct sna_composite_op *op))
{
	int rem;

start:
	rem = vertex_space(sna);
	if (rem < op->floats_per_rect) {
		DBG(("flushing vbo for %s: %d < %d\n",
		     __FUNCTION__, rem, op->floats_per_rect));
		rem = gen6_get_rectangles__flush(sna, op);
		if (unlikely(rem == 0))
			goto flush;
	}

	if (unlikely(sna->render_state.gen6.vertex_offset == 0 &&
		     !gen6_rectangle_begin(sna, op)))
		goto flush;

	if (want > 1 && want * op->floats_per_rect > rem)
		want = rem / op->floats_per_rect;

	assert(want > 0);
	sna->render.vertex_index += 3*want;
	return want;

flush:
	if (sna->render_state.gen6.vertex_offset) {
		gen6_vertex_flush(sna);
		gen6_magic_ca_pass(sna, op);
	}
	_kgem_submit(&sna->kgem);
	emit_state(sna, op);
	goto start;
}

inline static uint32_t *gen6_composite_get_binding_table(struct sna *sna,
							 uint16_t *offset)
{
	uint32_t *table;

	sna->kgem.surface -=
		sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
	/* Clear all surplus entries to zero in case of prefetch */
	table = memset(sna->kgem.batch + sna->kgem.surface,
		       0, sizeof(struct gen6_surface_state_padded));

	DBG(("%s(%x)\n", __FUNCTION__, 4*sna->kgem.surface));

	*offset = sna->kgem.surface;
	return table;
}

static uint32_t
gen6_choose_composite_vertex_buffer(const struct sna_composite_op *op)
{
	int id = 2 + !op->is_affine;
	if (op->mask.bo)
		id |= id << 2;
	assert(id > 0 && id < 16);
	return id;
}

static bool
gen6_get_batch(struct sna *sna, const struct sna_composite_op *op)
{
	kgem_set_mode(&sna->kgem, KGEM_RENDER, op->dst.bo);

	if (!kgem_check_batch_with_surfaces(&sna->kgem, 150, 4)) {
		DBG(("%s: flushing batch: %d < %d+%d\n",
		     __FUNCTION__, sna->kgem.surface - sna->kgem.nbatch,
		     150, 4*8));
		kgem_submit(&sna->kgem);
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	if (sna->render_state.gen6.needs_invariant)
		gen6_emit_invariant(sna);

	return kgem_bo_is_dirty(op->dst.bo);
}

static void gen6_emit_composite_state(struct sna *sna,
				      const struct sna_composite_op *op)
{
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;

	dirty = gen6_get_batch(sna, op);

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
			    op->dst.bo, op->dst.width, op->dst.height,
			    gen6_get_dest_format(op->dst.format),
			    true);
	binding_table[1] =
		gen6_bind_bo(sna,
			     op->src.bo, op->src.width, op->src.height,
			     op->src.card_format,
			     false);
	if (op->mask.bo) {
		binding_table[2] =
			gen6_bind_bo(sna,
				     op->mask.bo,
				     op->mask.width,
				     op->mask.height,
				     op->mask.card_format,
				     false);
	}

	if (sna->kgem.surface == offset &&
	    *(uint64_t *)(sna->kgem.batch + sna->render_state.gen6.surface_table) == *(uint64_t*)binding_table &&
	    (op->mask.bo == NULL ||
	     sna->kgem.batch[sna->render_state.gen6.surface_table+2] == binding_table[2])) {
		sna->kgem.surface += sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
		offset = sna->render_state.gen6.surface_table;
	}

	gen6_emit_state(sna, op, offset | dirty);
}

static void
gen6_align_vertex(struct sna *sna, const struct sna_composite_op *op)
{
	assert (sna->render_state.gen6.vertex_offset == 0);
	if (op->floats_per_vertex != sna->render_state.gen6.floats_per_vertex) {
		if (sna->render.vertex_size - sna->render.vertex_used < 2*op->floats_per_rect)
			/* XXX propagate failure */
			gen6_vertex_finish(sna);

		DBG(("aligning vertex: was %d, now %d floats per vertex, %d->%d\n",
		     sna->render_state.gen6.floats_per_vertex,
		     op->floats_per_vertex,
		     sna->render.vertex_index,
		     (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex));
		sna->render.vertex_index = (sna->render.vertex_used + op->floats_per_vertex - 1) / op->floats_per_vertex;
		sna->render.vertex_used = sna->render.vertex_index * op->floats_per_vertex;
		sna->render_state.gen6.floats_per_vertex = op->floats_per_vertex;
	}
}

fastcall static void
gen6_render_composite_blt(struct sna *sna,
			  const struct sna_composite_op *op,
			  const struct sna_composite_rectangles *r)
{
	gen6_get_rectangles(sna, op, 1, gen6_emit_composite_state);
	op->prim_emit(sna, op, r);
}

fastcall static void
gen6_render_composite_box(struct sna *sna,
			  const struct sna_composite_op *op,
			  const BoxRec *box)
{
	struct sna_composite_rectangles r;

	gen6_get_rectangles(sna, op, 1, gen6_emit_composite_state);

	DBG(("  %s: (%d, %d), (%d, %d)\n",
	     __FUNCTION__,
	     box->x1, box->y1, box->x2, box->y2));

	r.dst.x = box->x1;
	r.dst.y = box->y1;
	r.width  = box->x2 - box->x1;
	r.height = box->y2 - box->y1;
	r.src = r.mask = r.dst;

	op->prim_emit(sna, op, &r);
}

static void
gen6_render_composite_boxes(struct sna *sna,
			    const struct sna_composite_op *op,
			    const BoxRec *box, int nbox)
{
	DBG(("composite_boxes(%d)\n", nbox));

	do {
		int nbox_this_time;

		nbox_this_time = gen6_get_rectangles(sna, op, nbox,
						     gen6_emit_composite_state);
		nbox -= nbox_this_time;

		do {
			struct sna_composite_rectangles r;

			DBG(("  %s: (%d, %d), (%d, %d)\n",
			     __FUNCTION__,
			     box->x1, box->y1, box->x2, box->y2));

			r.dst.x = box->x1;
			r.dst.y = box->y1;
			r.width  = box->x2 - box->x1;
			r.height = box->y2 - box->y1;
			r.src = r.mask = r.dst;

			op->prim_emit(sna, op, &r);
			box++;
		} while (--nbox_this_time);
	} while (nbox);
}

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

static uint32_t
gen6_composite_create_blend_state(struct sna_static_stream *stream)
{
	char *base, *ptr;
	int src, dst;

	base = sna_static_stream_map(stream,
				     GEN6_BLENDFACTOR_COUNT * GEN6_BLENDFACTOR_COUNT * GEN6_BLEND_STATE_PADDED_SIZE,
				     64);

	ptr = base;
	for (src = 0; src < GEN6_BLENDFACTOR_COUNT; src++) {
		for (dst= 0; dst < GEN6_BLENDFACTOR_COUNT; dst++) {
			struct gen6_blend_state *blend =
				(struct gen6_blend_state *)ptr;

			blend->blend0.dest_blend_factor = dst;
			blend->blend0.source_blend_factor = src;
			blend->blend0.blend_func = GEN6_BLENDFUNCTION_ADD;
			blend->blend0.blend_enable =
				!(dst == GEN6_BLENDFACTOR_ZERO && src == GEN6_BLENDFACTOR_ONE);

			blend->blend1.post_blend_clamp_enable = 1;
			blend->blend1.pre_blend_clamp_enable = 1;

			ptr += GEN6_BLEND_STATE_PADDED_SIZE;
		}
	}

	return sna_static_stream_offsetof(stream, base);
}

static uint32_t gen6_bind_video_source(struct sna *sna,
				       struct kgem_bo *src_bo,
				       uint32_t src_offset,
				       int src_width,
				       int src_height,
				       int src_pitch,
				       uint32_t src_surf_format)
{
	struct gen6_surface_state *ss;

	sna->kgem.surface -= sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);

	ss = memset(sna->kgem.batch + sna->kgem.surface, 0, sizeof(*ss));
	ss->ss0.surface_type = GEN6_SURFACE_2D;
	ss->ss0.surface_format = src_surf_format;

	ss->ss1.base_addr =
		kgem_add_reloc(&sna->kgem,
			       sna->kgem.surface + 1,
			       src_bo,
			       I915_GEM_DOMAIN_SAMPLER << 16,
			       src_offset);

	ss->ss2.width  = src_width - 1;
	ss->ss2.height = src_height - 1;
	ss->ss3.pitch  = src_pitch - 1;

	return sna->kgem.surface * sizeof(uint32_t);
}

static void gen6_emit_video_state(struct sna *sna,
				  const struct sna_composite_op *op)
{
	struct sna_video_frame *frame = op->priv;
	uint32_t src_surf_format;
	uint32_t src_surf_base[6];
	int src_width[6];
	int src_height[6];
	int src_pitch[6];
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;
	int n_src, n;

	dirty = gen6_get_batch(sna, op);

	src_surf_base[0] = 0;
	src_surf_base[1] = 0;
	src_surf_base[2] = frame->VBufOffset;
	src_surf_base[3] = frame->VBufOffset;
	src_surf_base[4] = frame->UBufOffset;
	src_surf_base[5] = frame->UBufOffset;

	if (is_planar_fourcc(frame->id)) {
		src_surf_format = GEN6_SURFACEFORMAT_R8_UNORM;
		src_width[1]  = src_width[0]  = frame->width;
		src_height[1] = src_height[0] = frame->height;
		src_pitch[1]  = src_pitch[0]  = frame->pitch[1];
		src_width[4]  = src_width[5]  = src_width[2]  = src_width[3] =
			frame->width / 2;
		src_height[4] = src_height[5] = src_height[2] = src_height[3] =
			frame->height / 2;
		src_pitch[4]  = src_pitch[5]  = src_pitch[2]  = src_pitch[3] =
			frame->pitch[0];
		n_src = 6;
	} else {
		if (frame->id == FOURCC_UYVY)
			src_surf_format = GEN6_SURFACEFORMAT_YCRCB_SWAPY;
		else
			src_surf_format = GEN6_SURFACEFORMAT_YCRCB_NORMAL;

		src_width[0]  = frame->width;
		src_height[0] = frame->height;
		src_pitch[0]  = frame->pitch[0];
		n_src = 1;
	}

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
			     op->dst.bo, op->dst.width, op->dst.height,
			     gen6_get_dest_format(op->dst.format),
			     true);
	for (n = 0; n < n_src; n++) {
		binding_table[1+n] =
			gen6_bind_video_source(sna,
					       frame->bo,
					       src_surf_base[n],
					       src_width[n],
					       src_height[n],
					       src_pitch[n],
					       src_surf_format);
	}

	gen6_emit_state(sna, op, offset | dirty);
}

static bool
gen6_render_video(struct sna *sna,
		  struct sna_video *video,
		  struct sna_video_frame *frame,
		  RegionPtr dstRegion,
		  short src_w, short src_h,
		  short drw_w, short drw_h,
		  PixmapPtr pixmap)
{
	struct sna_composite_op tmp;
	int nbox, dxo, dyo, pix_xoff, pix_yoff;
	float src_scale_x, src_scale_y;
	struct sna_pixmap *priv;
	BoxPtr box;

	DBG(("%s: src=(%d, %d), dst=(%d, %d), %dx[(%d, %d), (%d, %d)...]\n",
	     __FUNCTION__, src_w, src_h, drw_w, drw_h,
	     REGION_NUM_RECTS(dstRegion),
	     REGION_EXTENTS(NULL, dstRegion)->x1,
	     REGION_EXTENTS(NULL, dstRegion)->y1,
	     REGION_EXTENTS(NULL, dstRegion)->x2,
	     REGION_EXTENTS(NULL, dstRegion)->y2));

	priv = sna_pixmap_force_to_gpu(pixmap, MOVE_READ | MOVE_WRITE);
	if (priv == NULL)
		return false;

	memset(&tmp, 0, sizeof(tmp));

	tmp.dst.pixmap = pixmap;
	tmp.dst.width  = pixmap->drawable.width;
	tmp.dst.height = pixmap->drawable.height;
	tmp.dst.format = sna_render_format_for_depth(pixmap->drawable.depth);
	tmp.dst.bo = priv->gpu_bo;

	tmp.src.bo = frame->bo;
	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 3;
	tmp.floats_per_rect = 9;

	tmp.u.gen6.flags =
		GEN6_SET_FLAGS(VIDEO_SAMPLER, NO_BLEND,
			       is_planar_fourcc(frame->id) ?
			       GEN6_WM_KERNEL_VIDEO_PLANAR :
			       GEN6_WM_KERNEL_VIDEO_PACKED,
			       2);
	tmp.priv = frame;

	kgem_set_mode(&sna->kgem, KGEM_RENDER, tmp.dst.bo);
	if (!kgem_check_bo(&sna->kgem, tmp.dst.bo, frame->bo, NULL)) {
		kgem_submit(&sna->kgem);
		assert(kgem_check_bo(&sna->kgem, tmp.dst.bo, frame->bo, NULL));
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen6_emit_video_state(sna, &tmp);
	gen6_align_vertex(sna, &tmp);

	/* Set up the offset for translating from the given region (in screen
	 * coordinates) to the backing pixmap.
	 */
#ifdef COMPOSITE
	pix_xoff = -pixmap->screen_x + pixmap->drawable.x;
	pix_yoff = -pixmap->screen_y + pixmap->drawable.y;
#else
	pix_xoff = 0;
	pix_yoff = 0;
#endif

	dxo = dstRegion->extents.x1;
	dyo = dstRegion->extents.y1;

	/* Use normalized texture coordinates */
	src_scale_x = ((float)src_w / frame->width) / (float)drw_w;
	src_scale_y = ((float)src_h / frame->height) / (float)drw_h;

	box = REGION_RECTS(dstRegion);
	nbox = REGION_NUM_RECTS(dstRegion);
	while (nbox--) {
		BoxRec r;

		r.x1 = box->x1 + pix_xoff;
		r.x2 = box->x2 + pix_xoff;
		r.y1 = box->y1 + pix_yoff;
		r.y2 = box->y2 + pix_yoff;

		gen6_get_rectangles(sna, &tmp, 1, gen6_emit_video_state);

		OUT_VERTEX(r.x2, r.y2);
		OUT_VERTEX_F((box->x2 - dxo) * src_scale_x);
		OUT_VERTEX_F((box->y2 - dyo) * src_scale_y);

		OUT_VERTEX(r.x1, r.y2);
		OUT_VERTEX_F((box->x1 - dxo) * src_scale_x);
		OUT_VERTEX_F((box->y2 - dyo) * src_scale_y);

		OUT_VERTEX(r.x1, r.y1);
		OUT_VERTEX_F((box->x1 - dxo) * src_scale_x);
		OUT_VERTEX_F((box->y1 - dyo) * src_scale_y);

		if (!DAMAGE_IS_ALL(priv->gpu_damage)) {
			sna_damage_add_box(&priv->gpu_damage, &r);
			sna_damage_subtract_box(&priv->cpu_damage, &r);
		}
		box++;
	}
	priv->clear = false;

	gen6_vertex_flush(sna);
	return true;
}

static bool
gen6_composite_solid_init(struct sna *sna,
			  struct sna_composite_channel *channel,
			  uint32_t color)
{
	DBG(("%s: color=%x\n", __FUNCTION__, color));

	channel->filter = PictFilterNearest;
	channel->repeat = RepeatNormal;
	channel->is_affine = true;
	channel->is_solid  = true;
	channel->is_opaque = (color >> 24) == 0xff;
	channel->transform = NULL;
	channel->width  = 1;
	channel->height = 1;
	channel->card_format = GEN6_SURFACEFORMAT_B8G8R8A8_UNORM;

	channel->bo = sna_render_get_solid(sna, color);

	channel->scale[0]  = channel->scale[1]  = 1;
	channel->offset[0] = channel->offset[1] = 0;
	return channel->bo != NULL;
}

static bool
gen6_composite_linear_init(struct sna *sna,
			   PicturePtr picture,
			   struct sna_composite_channel *channel,
			   int x, int y,
			   int w, int h,
			   int dst_x, int dst_y)
{
	PictLinearGradient *linear =
		(PictLinearGradient *)picture->pSourcePict;
	pixman_fixed_t tx, ty;
	float x0, y0, sf;
	float dx, dy;

	DBG(("%s: p1=(%f, %f), p2=(%f, %f), src=(%d, %d), dst=(%d, %d), size=(%d, %d)\n",
	     __FUNCTION__,
	     pixman_fixed_to_double(linear->p1.x), pixman_fixed_to_double(linear->p1.y),
	     pixman_fixed_to_double(linear->p2.x), pixman_fixed_to_double(linear->p2.y),
	     x, y, dst_x, dst_y, w, h));

	if (linear->p2.x == linear->p1.x && linear->p2.y == linear->p1.y)
		return 0;

	if (!sna_transform_is_affine(picture->transform)) {
		DBG(("%s: fallback due to projective transform\n",
		     __FUNCTION__));
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);
	}

	channel->bo = sna_render_get_gradient(sna, (PictGradient *)linear);
	if (!channel->bo)
		return 0;

	channel->filter = PictFilterNearest;
	channel->repeat = picture->repeat ? picture->repeatType : RepeatNone;
	channel->width  = channel->bo->pitch / 4;
	channel->height = 1;
	channel->pict_format = PICT_a8r8g8b8;

	channel->scale[0]  = channel->scale[1]  = 1;
	channel->offset[0] = channel->offset[1] = 0;

	if (sna_transform_is_translation(picture->transform, &tx, &ty)) {
		dx = pixman_fixed_to_double(linear->p2.x - linear->p1.x);
		dy = pixman_fixed_to_double(linear->p2.y - linear->p1.y);

		x0 = pixman_fixed_to_double(linear->p1.x);
		y0 = pixman_fixed_to_double(linear->p1.y);

		if (tx | ty) {
			x0 -= pixman_fixed_to_double(tx);
			y0 -= pixman_fixed_to_double(ty);
		}
	} else {
		struct pixman_f_vector p1, p2;
		struct pixman_f_transform m, inv;

		pixman_f_transform_from_pixman_transform(&m, picture->transform);
		DBG(("%s: transform = [%f %f %f, %f %f %f, %f %f %f]\n",
		     __FUNCTION__,
		     m.m[0][0], m.m[0][1], m.m[0][2],
		     m.m[1][0], m.m[1][1], m.m[1][2],
		     m.m[2][0], m.m[2][1], m.m[2][2]));
		if (!pixman_f_transform_invert(&inv, &m))
			return 0;

		p1.v[0] = pixman_fixed_to_double(linear->p1.x);
		p1.v[1] = pixman_fixed_to_double(linear->p1.y);
		p1.v[2] = 1.;
		pixman_f_transform_point(&inv, &p1);

		p2.v[0] = pixman_fixed_to_double(linear->p2.x);
		p2.v[1] = pixman_fixed_to_double(linear->p2.y);
		p2.v[2] = 1.;
		pixman_f_transform_point(&inv, &p2);

		DBG(("%s: untransformed: p1=(%f, %f, %f), p2=(%f, %f, %f)\n",
		     __FUNCTION__,
		     p1.v[0], p1.v[1], p1.v[2],
		     p2.v[0], p2.v[1], p2.v[2]));

		dx = p2.v[0] - p1.v[0];
		dy = p2.v[1] - p1.v[1];

		x0 = p1.v[0];
		y0 = p1.v[1];
	}

	sf = dx*dx + dy*dy;
	dx /= sf;
	dy /= sf;

	channel->embedded_transform.matrix[0][0] = pixman_double_to_fixed(dx);
	channel->embedded_transform.matrix[0][1] = pixman_double_to_fixed(dy);
	channel->embedded_transform.matrix[0][2] = -pixman_double_to_fixed(dx*(x0+dst_x-x) + dy*(y0+dst_y-y));

	channel->embedded_transform.matrix[1][0] = 0;
	channel->embedded_transform.matrix[1][1] = 0;
	channel->embedded_transform.matrix[1][2] = pixman_double_to_fixed(.5);

	channel->embedded_transform.matrix[2][0] = 0;
	channel->embedded_transform.matrix[2][1] = 0;
	channel->embedded_transform.matrix[2][2] = pixman_fixed_1;

	channel->transform = &channel->embedded_transform;
	channel->is_affine = 1;

	DBG(("%s: dx=%f, dy=%f, offset=%f\n",
	     __FUNCTION__, dx, dy, -dx*(x0-x+dst_x) + -dy*(y0-y+dst_y)));

	return channel->bo != NULL;
}

static int
gen6_composite_picture(struct sna *sna,
		       PicturePtr picture,
		       struct sna_composite_channel *channel,
		       int x, int y,
		       int w, int h,
		       int dst_x, int dst_y,
		       bool precise)
{
	PixmapPtr pixmap;
	uint32_t color;
	int16_t dx, dy;

	DBG(("%s: (%d, %d)x(%d, %d), dst=(%d, %d)\n",
	     __FUNCTION__, x, y, w, h, dst_x, dst_y));

	channel->is_solid = false;
	channel->card_format = -1;

	if (sna_picture_is_solid(picture, &color))
		return gen6_composite_solid_init(sna, channel, color);

	if (picture->pDrawable == NULL) {
		int ret;

		if (picture->pSourcePict->type == SourcePictTypeLinear)
			return gen6_composite_linear_init(sna, picture, channel,
							  x, y,
							  w, h,
							  dst_x, dst_y);

		DBG(("%s -- fixup, gradient\n", __FUNCTION__));
		ret = -1;
		if (!precise)
			ret = sna_render_picture_approximate_gradient(sna, picture, channel,
								      x, y, w, h, dst_x, dst_y);
		if (ret == -1)
			ret = sna_render_picture_fixup(sna, picture, channel,
						       x, y, w, h, dst_x, dst_y);
		return ret;
	}

	if (picture->alphaMap) {
		DBG(("%s -- fixup, alphamap\n", __FUNCTION__));
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);
	}

	if (!gen6_check_repeat(picture))
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);

	if (!gen6_check_filter(picture))
		return sna_render_picture_fixup(sna, picture, channel,
						x, y, w, h, dst_x, dst_y);

	channel->repeat = picture->repeat ? picture->repeatType : RepeatNone;
	channel->filter = picture->filter;

	pixmap = get_drawable_pixmap(picture->pDrawable);
	get_drawable_deltas(picture->pDrawable, pixmap, &dx, &dy);

	x += dx + picture->pDrawable->x;
	y += dy + picture->pDrawable->y;

	channel->is_affine = sna_transform_is_affine(picture->transform);
	if (sna_transform_is_integer_translation(picture->transform, &dx, &dy)) {
		DBG(("%s: integer translation (%d, %d), removing\n",
		     __FUNCTION__, dx, dy));
		x += dx;
		y += dy;
		channel->transform = NULL;
		channel->filter = PictFilterNearest;
	} else
		channel->transform = picture->transform;

	channel->pict_format = picture->format;
	channel->card_format = gen6_get_card_format(picture->format);
	if (channel->card_format == (unsigned)-1)
		return sna_render_picture_convert(sna, picture, channel, pixmap,
						  x, y, w, h, dst_x, dst_y,
						  false);

	if (too_large(pixmap->drawable.width, pixmap->drawable.height)) {
		DBG(("%s: extracting from pixmap %dx%d\n", __FUNCTION__,
		     pixmap->drawable.width, pixmap->drawable.height));
		return sna_render_picture_extract(sna, picture, channel,
						  x, y, w, h, dst_x, dst_y);
	}

	return sna_render_pixmap_bo(sna, channel, pixmap,
				    x, y, w, h, dst_x, dst_y);
}

inline static void gen6_composite_channel_convert(struct sna_composite_channel *channel)
{
	channel->repeat = gen6_repeat(channel->repeat);
	channel->filter = gen6_filter(channel->filter);
	if (channel->card_format == (unsigned)-1)
		channel->card_format = gen6_get_card_format(channel->pict_format);
	assert(channel->card_format != (unsigned)-1);
}

static void gen6_render_composite_done(struct sna *sna,
				       const struct sna_composite_op *op)
{
	DBG(("%s\n", __FUNCTION__));

	if (sna->render_state.gen6.vertex_offset) {
		gen6_vertex_flush(sna);
		gen6_magic_ca_pass(sna, op);
	}

	if (op->mask.bo)
		kgem_bo_destroy(&sna->kgem, op->mask.bo);
	if (op->src.bo)
		kgem_bo_destroy(&sna->kgem, op->src.bo);

	sna_render_composite_redirect_done(sna, op);
}

static bool
gen6_composite_set_target(struct sna *sna,
			  struct sna_composite_op *op,
			  PicturePtr dst,
			  int x, int y, int w, int h)
{
	BoxRec box;

	op->dst.pixmap = get_drawable_pixmap(dst->pDrawable);
	op->dst.format = dst->format;
	op->dst.width = op->dst.pixmap->drawable.width;
	op->dst.height = op->dst.pixmap->drawable.height;

	if (w && h) {
		box.x1 = x;
		box.y1 = y;
		box.x2 = x + w;
		box.y2 = y + h;
	} else
		sna_render_picture_extents(dst, &box);

	op->dst.bo = sna_drawable_use_bo (dst->pDrawable,
					  PREFER_GPU | FORCE_GPU | RENDER_GPU,
					  &box, &op->damage);
	if (op->dst.bo == NULL)
		return false;

	get_drawable_deltas(dst->pDrawable, op->dst.pixmap,
			    &op->dst.x, &op->dst.y);

	DBG(("%s: pixmap=%p, format=%08x, size=%dx%d, pitch=%d, delta=(%d,%d),damage=%p\n",
	     __FUNCTION__,
	     op->dst.pixmap, (int)op->dst.format,
	     op->dst.width, op->dst.height,
	     op->dst.bo->pitch,
	     op->dst.x, op->dst.y,
	     op->damage ? *op->damage : (void *)-1));

	assert(op->dst.bo->proxy == NULL);

	if (too_large(op->dst.width, op->dst.height) &&
	    !sna_render_composite_redirect(sna, op, x, y, w, h))
		return false;

	return true;
}

static bool prefer_blt_ring(struct sna *sna)
{
	if (PREFER_RENDER)
		return PREFER_RENDER < 0;

	return sna->kgem.ring != KGEM_RENDER;
}

static bool can_switch_to_blt(struct sna *sna)
{
	if (sna->kgem.ring == KGEM_BLT)
		return true;

	if (NO_RING_SWITCH)
		return false;

	if (!sna->kgem.has_semaphores)
		return false;

	return (sna->kgem.mode == KGEM_NONE ||
		kgem_ring_is_idle(&sna->kgem, KGEM_BLT));
}

static inline bool untiled_tlb_miss(struct kgem_bo *bo)
{
	return bo->tiling == I915_TILING_NONE && bo->pitch >= 4096;
}

static bool prefer_blt_bo(struct sna *sna, struct kgem_bo *bo)
{
	return untiled_tlb_miss(bo) && bo->pitch < MAXSHORT;
}

static bool
try_blt(struct sna *sna,
	PicturePtr dst, PicturePtr src,
	int width, int height)
{
	if (prefer_blt_ring(sna)) {
		DBG(("%s: already performing BLT\n", __FUNCTION__));
		return true;
	}

	if (too_large(width, height)) {
		DBG(("%s: operation too large for 3D pipe (%d, %d)\n",
		     __FUNCTION__, width, height));
		return true;
	}

	if (can_switch_to_blt(sna) && sna_picture_is_solid(src, NULL))
		return true;

	return false;
}

static bool
check_gradient(PicturePtr picture)
{
	if (picture->pDrawable)
		return false;

	switch (picture->pSourcePict->type) {
	case SourcePictTypeSolidFill:
	case SourcePictTypeLinear:
		return false;
	default:
		return true;
	}
}

static bool
has_alphamap(PicturePtr p)
{
	return p->alphaMap != NULL;
}

static bool
untransformed(PicturePtr p)
{
	return !p->transform || pixman_transform_is_int_translate(p->transform);
}

static bool
need_upload(PicturePtr p)
{
	return p->pDrawable && unattached(p->pDrawable) && untransformed(p);
}

static bool
source_is_busy(PixmapPtr pixmap)
{
	struct sna_pixmap *priv = sna_pixmap(pixmap);
	if (priv == NULL || priv->clear)
		return false;

	if (priv->gpu_bo && kgem_bo_is_busy(priv->gpu_bo))
		return true;

	if (priv->cpu_bo && kgem_bo_is_busy(priv->cpu_bo))
		return true;

	return priv->gpu_damage && !priv->cpu_damage;
}

static bool
source_fallback(PicturePtr p, PixmapPtr pixmap)
{
	if (sna_picture_is_solid(p, NULL))
		return false;

	if (p->pSourcePict)
		return check_gradient(p);

	if (!gen6_check_repeat(p) || !gen6_check_format(p->format))
		return true;

	if (pixmap && source_is_busy(pixmap))
		return false;

	return has_alphamap(p) || !gen6_check_filter(p) || need_upload(p);
}

static bool
gen6_composite_fallback(struct sna *sna,
			PicturePtr src,
			PicturePtr mask,
			PicturePtr dst)
{
	PixmapPtr src_pixmap;
	PixmapPtr mask_pixmap;
	PixmapPtr dst_pixmap;
	bool src_fallback, mask_fallback;

	if (!gen6_check_dst_format(dst->format)) {
		DBG(("%s: unknown destination format: %d\n",
		     __FUNCTION__, dst->format));
		return true;
	}

	dst_pixmap = get_drawable_pixmap(dst->pDrawable);

	src_pixmap = src->pDrawable ? get_drawable_pixmap(src->pDrawable) : NULL;
	src_fallback = source_fallback(src, src_pixmap);

	if (mask) {
		mask_pixmap = mask->pDrawable ? get_drawable_pixmap(mask->pDrawable) : NULL;
		mask_fallback = source_fallback(mask, mask_pixmap);
	} else {
		mask_pixmap = NULL;
		mask_fallback = false;
	}

	/* If we are using the destination as a source and need to
	 * readback in order to upload the source, do it all
	 * on the cpu.
	 */
	if (src_pixmap == dst_pixmap && src_fallback) {
		DBG(("%s: src is dst and will fallback\n",__FUNCTION__));
		return true;
	}
	if (mask_pixmap == dst_pixmap && mask_fallback) {
		DBG(("%s: mask is dst and will fallback\n",__FUNCTION__));
		return true;
	}

	/* If anything is on the GPU, push everything out to the GPU */
	if (dst_use_gpu(dst_pixmap)) {
		DBG(("%s: dst is already on the GPU, try to use GPU\n",
		     __FUNCTION__));
		return false;
	}

	if (src_pixmap && !src_fallback) {
		DBG(("%s: src is already on the GPU, try to use GPU\n",
		     __FUNCTION__));
		return false;
	}
	if (mask_pixmap && !mask_fallback) {
		DBG(("%s: mask is already on the GPU, try to use GPU\n",
		     __FUNCTION__));
		return false;
	}

	/* However if the dst is not on the GPU and we need to
	 * render one of the sources using the CPU, we may
	 * as well do the entire operation in place onthe CPU.
	 */
	if (src_fallback) {
		DBG(("%s: dst is on the CPU and src will fallback\n",
		     __FUNCTION__));
		return true;
	}

	if (mask && mask_fallback) {
		DBG(("%s: dst is on the CPU and mask will fallback\n",
		     __FUNCTION__));
		return true;
	}

	if (too_large(dst_pixmap->drawable.width,
		      dst_pixmap->drawable.height) &&
	    dst_is_cpu(dst_pixmap)) {
		DBG(("%s: dst is on the CPU and too large\n", __FUNCTION__));
		return true;
	}

	DBG(("%s: dst is not on the GPU and the operation should not fallback\n",
	     __FUNCTION__));
	return false;
}

static int
reuse_source(struct sna *sna,
	     PicturePtr src, struct sna_composite_channel *sc, int src_x, int src_y,
	     PicturePtr mask, struct sna_composite_channel *mc, int msk_x, int msk_y)
{
	uint32_t color;

	if (src_x != msk_x || src_y != msk_y)
		return false;

	if (src == mask) {
		DBG(("%s: mask is source\n", __FUNCTION__));
		*mc = *sc;
		mc->bo = kgem_bo_reference(mc->bo);
		return true;
	}

	if (sna_picture_is_solid(mask, &color))
		return gen6_composite_solid_init(sna, mc, color);

	if (sc->is_solid)
		return false;

	if (src->pDrawable == NULL || mask->pDrawable != src->pDrawable)
		return false;

	DBG(("%s: mask reuses source drawable\n", __FUNCTION__));

	if (!sna_transform_equal(src->transform, mask->transform))
		return false;

	if (!sna_picture_alphamap_equal(src, mask))
		return false;

	if (!gen6_check_repeat(mask))
		return false;

	if (!gen6_check_filter(mask))
		return false;

	if (!gen6_check_format(mask->format))
		return false;

	DBG(("%s: reusing source channel for mask with a twist\n",
	     __FUNCTION__));

	*mc = *sc;
	mc->repeat = gen6_repeat(mask->repeat ? mask->repeatType : RepeatNone);
	mc->filter = gen6_filter(mask->filter);
	mc->pict_format = mask->format;
	mc->card_format = gen6_get_card_format(mask->format);
	mc->bo = kgem_bo_reference(mc->bo);
	return true;
}

static bool
prefer_blt_composite(struct sna *sna, struct sna_composite_op *tmp)
{
	if (sna->kgem.ring == KGEM_BLT)
		return true;

	if (!prefer_blt_ring(sna))
		return false;

	return (prefer_blt_bo(sna, tmp->dst.bo) ||
		prefer_blt_bo(sna, tmp->src.bo));
}

static bool
gen6_render_composite(struct sna *sna,
		      uint8_t op,
		      PicturePtr src,
		      PicturePtr mask,
		      PicturePtr dst,
		      int16_t src_x, int16_t src_y,
		      int16_t msk_x, int16_t msk_y,
		      int16_t dst_x, int16_t dst_y,
		      int16_t width, int16_t height,
		      struct sna_composite_op *tmp)
{
	if (op >= ARRAY_SIZE(gen6_blend_op))
		return false;

	DBG(("%s: %dx%d, current mode=%d\n", __FUNCTION__,
	     width, height, sna->kgem.ring));

	if (mask == NULL &&
	    try_blt(sna, dst, src, width, height) &&
	    sna_blt_composite(sna, op,
			      src, dst,
			      src_x, src_y,
			      dst_x, dst_y,
			      width, height,
			      tmp, false))
		return true;

	if (gen6_composite_fallback(sna, src, mask, dst))
		return false;

	if (need_tiling(sna, width, height))
		return sna_tiling_composite(op, src, mask, dst,
					    src_x, src_y,
					    msk_x, msk_y,
					    dst_x, dst_y,
					    width, height,
					    tmp);

	if (op == PictOpClear)
		op = PictOpSrc;
	tmp->op = op;
	if (!gen6_composite_set_target(sna, tmp, dst,
				       dst_x, dst_y, width, height))
		return false;

	switch (gen6_composite_picture(sna, src, &tmp->src,
				       src_x, src_y,
				       width, height,
				       dst_x, dst_y,
				       dst->polyMode == PolyModePrecise)) {
	case -1:
		goto cleanup_dst;
	case 0:
		if (!gen6_composite_solid_init(sna, &tmp->src, 0))
			goto cleanup_dst;
		/* fall through to fixup */
	case 1:
		/* Did we just switch rings to prepare the source? */
		if (mask == NULL &&
		    prefer_blt_composite(sna, tmp) &&
		    sna_blt_composite__convert(sna,
					       dst_x, dst_y, width, height,
					       tmp))
			return true;

		gen6_composite_channel_convert(&tmp->src);
		break;
	}

	tmp->is_affine = tmp->src.is_affine;
	tmp->has_component_alpha = false;
	tmp->need_magic_ca_pass = false;

	tmp->mask.bo = NULL;
	tmp->mask.filter = SAMPLER_FILTER_NEAREST;
	tmp->mask.repeat = SAMPLER_EXTEND_NONE;

	tmp->prim_emit = gen6_emit_composite_primitive;
	if (mask) {
		if (mask->componentAlpha && PICT_FORMAT_RGB(mask->format)) {
			tmp->has_component_alpha = true;

			/* Check if it's component alpha that relies on a source alpha and on
			 * the source value.  We can only get one of those into the single
			 * source value that we get to blend with.
			 */
			if (gen6_blend_op[op].src_alpha &&
			    (gen6_blend_op[op].src_blend != GEN6_BLENDFACTOR_ZERO)) {
				if (op != PictOpOver)
					goto cleanup_src;

				tmp->need_magic_ca_pass = true;
				tmp->op = PictOpOutReverse;
			}
		}

		if (!reuse_source(sna,
				  src, &tmp->src, src_x, src_y,
				  mask, &tmp->mask, msk_x, msk_y)) {
			switch (gen6_composite_picture(sna, mask, &tmp->mask,
						       msk_x, msk_y,
						       width, height,
						       dst_x, dst_y,
						       dst->polyMode == PolyModePrecise)) {
			case -1:
				goto cleanup_src;
			case 0:
				if (!gen6_composite_solid_init(sna, &tmp->mask, 0))
					goto cleanup_src;
				/* fall through to fixup */
			case 1:
				gen6_composite_channel_convert(&tmp->mask);
				break;
			}
		}

		tmp->is_affine &= tmp->mask.is_affine;

		if (tmp->src.transform == NULL && tmp->mask.transform == NULL) {
			if (tmp->src.is_solid)
				tmp->prim_emit = gen6_emit_composite_primitive_identity_mask;
			else
				tmp->prim_emit = gen6_emit_composite_primitive_identity_source_mask;
		}

		tmp->floats_per_vertex = 5 + 2 * !tmp->is_affine;
	} else {
		if (tmp->src.is_solid) {
			DBG(("%s: choosing gen6_emit_composite_primitive_solid\n",
			     __FUNCTION__));
			tmp->prim_emit = gen6_emit_composite_primitive_solid;
			if (tmp->src.is_opaque && op == PictOpOver)
				tmp->op = PictOpSrc;
		} else if (tmp->src.transform == NULL) {
			DBG(("%s: choosing gen6_emit_composite_primitive_identity_source\n",
			     __FUNCTION__));
			tmp->prim_emit = gen6_emit_composite_primitive_identity_source;
		} else if (tmp->src.is_affine) {
			if (tmp->src.transform->matrix[0][1] == 0 &&
			    tmp->src.transform->matrix[1][0] == 0) {
				tmp->src.scale[0] /= tmp->src.transform->matrix[2][2];
				tmp->src.scale[1] /= tmp->src.transform->matrix[2][2];
				DBG(("%s: choosing gen6_emit_composite_primitive_simple_source\n",
				     __FUNCTION__));
				tmp->prim_emit = gen6_emit_composite_primitive_simple_source;
			} else {
				DBG(("%s: choosing gen6_emit_composite_primitive_affine_source\n",
				     __FUNCTION__));
				tmp->prim_emit = gen6_emit_composite_primitive_affine_source;
			}
		}

		tmp->floats_per_vertex = 3 + !tmp->is_affine;
	}
	tmp->floats_per_rect = 3 * tmp->floats_per_vertex;

	tmp->u.gen6.flags =
		GEN6_SET_FLAGS(SAMPLER_OFFSET(tmp->src.filter,
					      tmp->src.repeat,
					      tmp->mask.filter,
					      tmp->mask.repeat),
			       gen6_get_blend(tmp->op,
					      tmp->has_component_alpha,
					      tmp->dst.format),
			       gen6_choose_composite_kernel(tmp->op,
							    tmp->mask.bo != NULL,
							    tmp->has_component_alpha,
							    tmp->is_affine),
			       gen6_choose_composite_vertex_buffer(tmp));

	tmp->blt   = gen6_render_composite_blt;
	tmp->box   = gen6_render_composite_box;
	tmp->boxes = gen6_render_composite_boxes;
	tmp->done  = gen6_render_composite_done;

	kgem_set_mode(&sna->kgem, KGEM_RENDER, tmp->dst.bo);
	if (!kgem_check_bo(&sna->kgem,
			   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem,
				   tmp->dst.bo, tmp->src.bo, tmp->mask.bo,
				   NULL))
			goto cleanup_mask;
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen6_emit_composite_state(sna, tmp);
	gen6_align_vertex(sna, tmp);
	return true;

cleanup_mask:
	if (tmp->mask.bo)
		kgem_bo_destroy(&sna->kgem, tmp->mask.bo);
cleanup_src:
	if (tmp->src.bo)
		kgem_bo_destroy(&sna->kgem, tmp->src.bo);
cleanup_dst:
	if (tmp->redirect.real_bo)
		kgem_bo_destroy(&sna->kgem, tmp->dst.bo);
	return false;
}

#if !NO_COMPOSITE_SPANS
inline static void
gen6_emit_composite_texcoord_affine(struct sna *sna,
				    const struct sna_composite_channel *channel,
				    int16_t x, int16_t y)
{
	float t[2];

	sna_get_transformed_coordinates(x + channel->offset[0],
					y + channel->offset[1],
					channel->transform,
					&t[0], &t[1]);
	OUT_VERTEX_F(t[0] * channel->scale[0]);
	OUT_VERTEX_F(t[1] * channel->scale[1]);
}

inline static void
gen6_emit_composite_spans_vertex(struct sna *sna,
				 const struct sna_composite_spans_op *op,
				 int16_t x, int16_t y)
{
	OUT_VERTEX(x, y);
	gen6_emit_composite_texcoord(sna, &op->base.src, x, y);
}

fastcall static void
gen6_emit_composite_spans_primitive(struct sna *sna,
				    const struct sna_composite_spans_op *op,
				    const BoxRec *box,
				    float opacity)
{
	gen6_emit_composite_spans_vertex(sna, op, box->x2, box->y2);
	OUT_VERTEX_F(opacity);

	gen6_emit_composite_spans_vertex(sna, op, box->x1, box->y2);
	OUT_VERTEX_F(opacity);

	gen6_emit_composite_spans_vertex(sna, op, box->x1, box->y1);
	OUT_VERTEX_F(opacity);
}

fastcall static void
gen6_emit_composite_spans_solid(struct sna *sna,
				const struct sna_composite_spans_op *op,
				const BoxRec *box,
				float opacity)
{
	OUT_VERTEX(box->x2, box->y2);
	OUT_VERTEX_F(1); OUT_VERTEX_F(1);
	OUT_VERTEX_F(opacity);

	OUT_VERTEX(box->x1, box->y2);
	OUT_VERTEX_F(0); OUT_VERTEX_F(1);
	OUT_VERTEX_F(opacity);

	OUT_VERTEX(box->x1, box->y1);
	OUT_VERTEX_F(0); OUT_VERTEX_F(0);
	OUT_VERTEX_F(opacity);
}

fastcall static void
gen6_emit_composite_spans_identity(struct sna *sna,
				   const struct sna_composite_spans_op *op,
				   const BoxRec *box,
				   float opacity)
{
	float *v;
	union {
		struct sna_coordinate p;
		float f;
	} dst;

	float sx = op->base.src.scale[0];
	float sy = op->base.src.scale[1];
	int16_t tx = op->base.src.offset[0];
	int16_t ty = op->base.src.offset[1];

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 3*4;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	dst.p.x = box->x2;
	dst.p.y = box->y2;
	v[0] = dst.f;
	v[1] = (box->x2 + tx) * sx;
	v[6] = v[2] = (box->y2 + ty) * sy;

	dst.p.x = box->x1;
	v[4] = dst.f;
	v[9] = v[5] = (box->x1 + tx) * sx;

	dst.p.y = box->y1;
	v[8] = dst.f;
	v[10] = (box->y1 + ty) * sy;

	v[11] = v[7] = v[3] = opacity;
}

fastcall static void
gen6_emit_composite_spans_simple(struct sna *sna,
				 const struct sna_composite_spans_op *op,
				 const BoxRec *box,
				 float opacity)
{
	float *v;
	union {
		struct sna_coordinate p;
		float f;
	} dst;

	float xx = op->base.src.transform->matrix[0][0];
	float x0 = op->base.src.transform->matrix[0][2];
	float yy = op->base.src.transform->matrix[1][1];
	float y0 = op->base.src.transform->matrix[1][2];
	float sx = op->base.src.scale[0];
	float sy = op->base.src.scale[1];
	int16_t tx = op->base.src.offset[0];
	int16_t ty = op->base.src.offset[1];

	v = sna->render.vertices + sna->render.vertex_used;
	sna->render.vertex_used += 3*4;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	dst.p.x = box->x2;
	dst.p.y = box->y2;
	v[0] = dst.f;
	v[1] = ((box->x2 + tx) * xx + x0) * sx;
	v[6] = v[2] = ((box->y2 + ty) * yy + y0) * sy;

	dst.p.x = box->x1;
	v[4] = dst.f;
	v[9] = v[5] = ((box->x1 + tx) * xx + x0) * sx;

	dst.p.y = box->y1;
	v[8] = dst.f;
	v[10] = ((box->y1 + ty) * yy + y0) * sy;

	v[11] = v[7] = v[3] = opacity;
}

fastcall static void
gen6_emit_composite_spans_affine(struct sna *sna,
				 const struct sna_composite_spans_op *op,
				 const BoxRec *box,
				 float opacity)
{
	OUT_VERTEX(box->x2, box->y2);
	gen6_emit_composite_texcoord_affine(sna, &op->base.src,
					    box->x2, box->y2);
	OUT_VERTEX_F(opacity);

	OUT_VERTEX(box->x1, box->y2);
	gen6_emit_composite_texcoord_affine(sna, &op->base.src,
					    box->x1, box->y2);
	OUT_VERTEX_F(opacity);

	OUT_VERTEX(box->x1, box->y1);
	gen6_emit_composite_texcoord_affine(sna, &op->base.src,
					    box->x1, box->y1);
	OUT_VERTEX_F(opacity);
}

fastcall static void
gen6_render_composite_spans_box(struct sna *sna,
				const struct sna_composite_spans_op *op,
				const BoxRec *box, float opacity)
{
	DBG(("%s: src=+(%d, %d), opacity=%f, dst=+(%d, %d), box=(%d, %d) x (%d, %d)\n",
	     __FUNCTION__,
	     op->base.src.offset[0], op->base.src.offset[1],
	     opacity,
	     op->base.dst.x, op->base.dst.y,
	     box->x1, box->y1,
	     box->x2 - box->x1,
	     box->y2 - box->y1));

	gen6_get_rectangles(sna, &op->base, 1, gen6_emit_composite_state);
	op->prim_emit(sna, op, box, opacity);
}

static void
gen6_render_composite_spans_boxes(struct sna *sna,
				  const struct sna_composite_spans_op *op,
				  const BoxRec *box, int nbox,
				  float opacity)
{
	DBG(("%s: nbox=%d, src=+(%d, %d), opacity=%f, dst=+(%d, %d)\n",
	     __FUNCTION__, nbox,
	     op->base.src.offset[0], op->base.src.offset[1],
	     opacity,
	     op->base.dst.x, op->base.dst.y));

	do {
		int nbox_this_time;

		nbox_this_time = gen6_get_rectangles(sna, &op->base, nbox,
						     gen6_emit_composite_state);
		nbox -= nbox_this_time;

		do {
			DBG(("  %s: (%d, %d) x (%d, %d)\n", __FUNCTION__,
			     box->x1, box->y1,
			     box->x2 - box->x1,
			     box->y2 - box->y1));

			op->prim_emit(sna, op, box++, opacity);
		} while (--nbox_this_time);
	} while (nbox);
}

fastcall static void
gen6_render_composite_spans_done(struct sna *sna,
				 const struct sna_composite_spans_op *op)
{
	DBG(("%s()\n", __FUNCTION__));

	if (sna->render_state.gen6.vertex_offset)
		gen6_vertex_flush(sna);

	if (op->base.src.bo)
		kgem_bo_destroy(&sna->kgem, op->base.src.bo);

	sna_render_composite_redirect_done(sna, &op->base);
}

static bool
gen6_check_composite_spans(struct sna *sna,
			   uint8_t op, PicturePtr src, PicturePtr dst,
			   int16_t width, int16_t height,
			   unsigned flags)
{
	DBG(("%s: op=%d, width=%d, height=%d, flags=%x\n",
	     __FUNCTION__, op, width, height, flags));

	if (op >= ARRAY_SIZE(gen6_blend_op))
		return false;

	if (gen6_composite_fallback(sna, src, NULL, dst)) {
		DBG(("%s: operation would fallback\n", __FUNCTION__));
		return false;
	}

	if (need_tiling(sna, width, height) && !is_gpu(dst->pDrawable)) {
		DBG(("%s: fallback, tiled operation not on GPU\n",
		     __FUNCTION__));
		return false;
	}

	if ((flags & COMPOSITE_SPANS_RECTILINEAR) == 0) {
		if ((flags & COMPOSITE_SPANS_INPLACE_HINT) == 0) {
			struct sna_pixmap *priv = sna_pixmap_from_drawable(dst->pDrawable);
			assert(priv);

			if ((priv->cpu_bo && kgem_bo_is_busy(priv->cpu_bo)) ||
			    (priv->gpu_bo && kgem_bo_is_busy(priv->gpu_bo))) {
				return true;
			}
		}

		DBG(("%s: fallback, non-rectilinear spans to idle bo\n",
		     __FUNCTION__));
		return false;
	}

	return true;
}

static bool
gen6_render_composite_spans(struct sna *sna,
			    uint8_t op,
			    PicturePtr src,
			    PicturePtr dst,
			    int16_t src_x,  int16_t src_y,
			    int16_t dst_x,  int16_t dst_y,
			    int16_t width,  int16_t height,
			    unsigned flags,
			    struct sna_composite_spans_op *tmp)
{
	DBG(("%s: %dx%d with flags=%x, current mode=%d\n", __FUNCTION__,
	     width, height, flags, sna->kgem.ring));

	assert(gen6_check_composite_spans(sna, op, src, dst, width, height, flags));

	if (need_tiling(sna, width, height)) {
		DBG(("%s: tiling, operation (%dx%d) too wide for pipeline\n",
		     __FUNCTION__, width, height));
		return sna_tiling_composite_spans(op, src, dst,
						  src_x, src_y, dst_x, dst_y,
						  width, height, flags, tmp);
	}

	tmp->base.op = op;
	if (!gen6_composite_set_target(sna, &tmp->base, dst,
				       dst_x, dst_y, width, height))
		return false;

	switch (gen6_composite_picture(sna, src, &tmp->base.src,
				       src_x, src_y,
				       width, height,
				       dst_x, dst_y,
				       dst->polyMode == PolyModePrecise)) {
	case -1:
		goto cleanup_dst;
	case 0:
		if (!gen6_composite_solid_init(sna, &tmp->base.src, 0))
			goto cleanup_dst;
		/* fall through to fixup */
	case 1:
		gen6_composite_channel_convert(&tmp->base.src);
		break;
	}
	tmp->base.mask.bo = NULL;

	tmp->base.is_affine = tmp->base.src.is_affine;
	tmp->base.need_magic_ca_pass = false;

	tmp->prim_emit = gen6_emit_composite_spans_primitive;
	if (tmp->base.src.is_solid) {
		tmp->prim_emit = gen6_emit_composite_spans_solid;
	} else if (tmp->base.src.transform == NULL) {
		tmp->prim_emit = gen6_emit_composite_spans_identity;
	} else if (tmp->base.is_affine) {
		if (tmp->base.src.transform->matrix[0][1] == 0 &&
		    tmp->base.src.transform->matrix[1][0] == 0) {
			tmp->base.src.scale[0] /= tmp->base.src.transform->matrix[2][2];
			tmp->base.src.scale[1] /= tmp->base.src.transform->matrix[2][2];
			tmp->prim_emit = gen6_emit_composite_spans_simple;
		} else
			tmp->prim_emit = gen6_emit_composite_spans_affine;
	}
	tmp->base.floats_per_vertex = 4 + !tmp->base.is_affine;
	tmp->base.floats_per_rect = 3 * tmp->base.floats_per_vertex;

	tmp->base.u.gen6.flags =
		GEN6_SET_FLAGS(SAMPLER_OFFSET(tmp->base.src.filter,
					      tmp->base.src.repeat,
					      SAMPLER_FILTER_NEAREST,
					      SAMPLER_EXTEND_PAD),
			       gen6_get_blend(tmp->base.op, false, tmp->base.dst.format),
			       GEN6_WM_KERNEL_OPACITY | !tmp->base.is_affine,
			       1 << 2 | (2+!tmp->base.is_affine));

	tmp->box   = gen6_render_composite_spans_box;
	tmp->boxes = gen6_render_composite_spans_boxes;
	tmp->done  = gen6_render_composite_spans_done;

	kgem_set_mode(&sna->kgem, KGEM_RENDER, tmp->base.dst.bo);
	if (!kgem_check_bo(&sna->kgem,
			   tmp->base.dst.bo, tmp->base.src.bo,
			   NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem,
				   tmp->base.dst.bo, tmp->base.src.bo,
				   NULL))
			goto cleanup_src;
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen6_emit_composite_state(sna, &tmp->base);
	gen6_align_vertex(sna, &tmp->base);
	return true;

cleanup_src:
	if (tmp->base.src.bo)
		kgem_bo_destroy(&sna->kgem, tmp->base.src.bo);
cleanup_dst:
	if (tmp->base.redirect.real_bo)
		kgem_bo_destroy(&sna->kgem, tmp->base.dst.bo);
	return false;
}
#endif

static void
gen6_emit_copy_state(struct sna *sna,
		     const struct sna_composite_op *op)
{
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;

	dirty = gen6_get_batch(sna, op);

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
			     op->dst.bo, op->dst.width, op->dst.height,
			     gen6_get_dest_format(op->dst.format),
			     true);
	binding_table[1] =
		gen6_bind_bo(sna,
			     op->src.bo, op->src.width, op->src.height,
			     op->src.card_format,
			     false);

	if (sna->kgem.surface == offset &&
	    *(uint64_t *)(sna->kgem.batch + sna->render_state.gen6.surface_table) == *(uint64_t*)binding_table) {
		sna->kgem.surface += sizeof(struct gen6_surface_state_padded) / sizeof(uint32_t);
		offset = sna->render_state.gen6.surface_table;
	}

	gen6_emit_state(sna, op, offset | dirty);
}

static inline bool prefer_blt_copy(struct sna *sna,
				   struct kgem_bo *src_bo,
				   struct kgem_bo *dst_bo,
				   unsigned flags)
{
	if (PREFER_RENDER)
		return PREFER_RENDER > 0;

	if (sna->kgem.ring == KGEM_BLT)
		return true;

	if (src_bo == dst_bo && can_switch_to_blt(sna))
		return true;

	return ((flags & COPY_LAST && sna->kgem.ring != KGEM_RENDER) ||
		prefer_blt_bo(sna, src_bo) ||
		prefer_blt_bo(sna, dst_bo));
}

inline static void boxes_extents(const BoxRec *box, int n, BoxRec *extents)
{
	*extents = box[0];
	while (--n) {
		box++;

		if (box->x1 < extents->x1)
			extents->x1 = box->x1;
		if (box->x2 > extents->x2)
			extents->x2 = box->x2;

		if (box->y1 < extents->y1)
			extents->y1 = box->y1;
		if (box->y2 > extents->y2)
			extents->y2 = box->y2;
	}
}

static inline bool
overlaps(struct sna *sna,
	 struct kgem_bo *src_bo, int16_t src_dx, int16_t src_dy,
	 struct kgem_bo *dst_bo, int16_t dst_dx, int16_t dst_dy,
	 const BoxRec *box, int n, BoxRec *extents)
{
	if (src_bo != dst_bo)
		return false;

	boxes_extents(box, n, extents);
	return (extents->x2 + src_dx > extents->x1 + dst_dx &&
		extents->x1 + src_dx < extents->x2 + dst_dx &&
		extents->y2 + src_dy > extents->y1 + dst_dy &&
		extents->y1 + src_dy < extents->y2 + dst_dy);
}

static bool
gen6_render_copy_boxes(struct sna *sna, uint8_t alu,
		       PixmapPtr src, struct kgem_bo *src_bo, int16_t src_dx, int16_t src_dy,
		       PixmapPtr dst, struct kgem_bo *dst_bo, int16_t dst_dx, int16_t dst_dy,
		       const BoxRec *box, int n, unsigned flags)
{
	struct sna_composite_op tmp;
	BoxRec extents;

	DBG(("%s (%d, %d)->(%d, %d) x %d, alu=%x, self-copy=%d, overlaps? %d\n",
	     __FUNCTION__, src_dx, src_dy, dst_dx, dst_dy, n, alu,
	     src_bo == dst_bo,
	     overlaps(sna,
		      src_bo, src_dx, src_dy,
		      dst_bo, dst_dx, dst_dy,
		      box, n, &extents)));

	if (prefer_blt_copy(sna, src_bo, dst_bo, flags) &&
	    sna_blt_compare_depth(&src->drawable, &dst->drawable) &&
	    sna_blt_copy_boxes(sna, alu,
			       src_bo, src_dx, src_dy,
			       dst_bo, dst_dx, dst_dy,
			       dst->drawable.bitsPerPixel,
			       box, n))
		return true;

	if (!(alu == GXcopy || alu == GXclear)) {
fallback_blt:
		if (!sna_blt_compare_depth(&src->drawable, &dst->drawable))
			return false;

		return sna_blt_copy_boxes_fallback(sna, alu,
						   src, src_bo, src_dx, src_dy,
						   dst, dst_bo, dst_dx, dst_dy,
						   box, n);
	}

	if (overlaps(sna,
		     src_bo, src_dx, src_dy,
		     dst_bo, dst_dx, dst_dy,
		     box, n, &extents)) {
		if (too_large(extents.x2-extents.x1, extents.y2-extents.y1))
			goto fallback_blt;

		if ((flags & COPY_LAST || can_switch_to_blt(sna)) &&
		    sna_blt_compare_depth(&src->drawable, &dst->drawable) &&
		    sna_blt_copy_boxes(sna, alu,
				       src_bo, src_dx, src_dy,
				       dst_bo, dst_dx, dst_dy,
				       dst->drawable.bitsPerPixel,
				       box, n))
			return true;

		return sna_render_copy_boxes__overlap(sna, alu,
						      src, src_bo, src_dx, src_dy,
						      dst, dst_bo, dst_dx, dst_dy,
						      box, n, &extents);
	}

	if (dst->drawable.depth == src->drawable.depth) {
		tmp.dst.format = sna_render_format_for_depth(dst->drawable.depth);
		tmp.src.pict_format = tmp.dst.format;
	} else {
		tmp.dst.format = sna_format_for_depth(dst->drawable.depth);
		tmp.src.pict_format = sna_format_for_depth(src->drawable.depth);
	}
	if (!gen6_check_format(tmp.src.pict_format))
		goto fallback_blt;

	tmp.dst.pixmap = dst;
	tmp.dst.width  = dst->drawable.width;
	tmp.dst.height = dst->drawable.height;
	tmp.dst.bo = dst_bo;
	tmp.dst.x = tmp.dst.y = 0;
	tmp.damage = NULL;

	sna_render_composite_redirect_init(&tmp);
	if (too_large(tmp.dst.width, tmp.dst.height)) {
		int i;

		extents = box[0];
		for (i = 1; i < n; i++) {
			if (box[i].x1 < extents.x1)
				extents.x1 = box[i].x1;
			if (box[i].y1 < extents.y1)
				extents.y1 = box[i].y1;

			if (box[i].x2 > extents.x2)
				extents.x2 = box[i].x2;
			if (box[i].y2 > extents.y2)
				extents.y2 = box[i].y2;
		}

		if (!sna_render_composite_redirect(sna, &tmp,
						   extents.x1 + dst_dx,
						   extents.y1 + dst_dy,
						   extents.x2 - extents.x1,
						   extents.y2 - extents.y1))
			goto fallback_tiled;

		dst_dx += tmp.dst.x;
		dst_dy += tmp.dst.y;

		tmp.dst.x = tmp.dst.y = 0;
	}

	tmp.src.card_format = gen6_get_card_format(tmp.src.pict_format);
	if (too_large(src->drawable.width, src->drawable.height)) {
		int i;

		extents = box[0];
		for (i = 1; i < n; i++) {
			if (extents.x1 < box[i].x1)
				extents.x1 = box[i].x1;
			if (extents.y1 < box[i].y1)
				extents.y1 = box[i].y1;

			if (extents.x2 > box[i].x2)
				extents.x2 = box[i].x2;
			if (extents.y2 > box[i].y2)
				extents.y2 = box[i].y2;
		}

		if (!sna_render_pixmap_partial(sna, src, src_bo, &tmp.src,
					       extents.x1 + src_dx,
					       extents.y1 + src_dy,
					       extents.x2 - extents.x1,
					       extents.y2 - extents.y1)) {
			DBG(("%s: unable to extract partial pixmap\n", __FUNCTION__));
			goto fallback_tiled_dst;
		}

		src_dx += tmp.src.offset[0];
		src_dy += tmp.src.offset[1];
	} else {
		tmp.src.bo = src_bo;
		tmp.src.width  = src->drawable.width;
		tmp.src.height = src->drawable.height;
	}

	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 2;
	tmp.floats_per_rect = 6;
	tmp.need_magic_ca_pass = 0;

	tmp.u.gen6.flags = COPY_FLAGS(alu);
	assert(GEN6_KERNEL(tmp.u.gen6.flags) == GEN6_WM_KERNEL_NOMASK);
	assert(GEN6_SAMPLER(tmp.u.gen6.flags) == COPY_SAMPLER);
	assert(GEN6_VERTEX(tmp.u.gen6.flags) == COPY_VERTEX);

	kgem_set_mode(&sna->kgem, KGEM_RENDER, tmp.dst.bo);
	if (!kgem_check_bo(&sna->kgem, tmp.dst.bo, tmp.src.bo, NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem, tmp.dst.bo, tmp.src.bo, NULL)) {
			DBG(("%s: too large for a single operation\n",
			     __FUNCTION__));
			goto fallback_tiled_src;
		}
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen6_emit_copy_state(sna, &tmp);
	gen6_align_vertex(sna, &tmp);

	do {
		int16_t *v;
		int n_this_time;

		n_this_time = gen6_get_rectangles(sna, &tmp, n,
						  gen6_emit_copy_state);
		n -= n_this_time;

		v = (int16_t *)(sna->render.vertices + sna->render.vertex_used);
		sna->render.vertex_used += 6 * n_this_time;
		assert(sna->render.vertex_used <= sna->render.vertex_size);
		do {

			DBG(("	(%d, %d) -> (%d, %d) + (%d, %d)\n",
			     box->x1 + src_dx, box->y1 + src_dy,
			     box->x1 + dst_dx, box->y1 + dst_dy,
			     box->x2 - box->x1, box->y2 - box->y1));
			v[0] = box->x2 + dst_dx;
			v[2] = box->x2 + src_dx;
			v[1]  = v[5] = box->y2 + dst_dy;
			v[3]  = v[7] = box->y2 + src_dy;
			v[8]  = v[4] = box->x1 + dst_dx;
			v[10] = v[6] = box->x1 + src_dx;
			v[9]  = box->y1 + dst_dy;
			v[11] = box->y1 + src_dy;
			v += 12; box++;
		} while (--n_this_time);
	} while (n);

	gen6_vertex_flush(sna);
	sna_render_composite_redirect_done(sna, &tmp);
	if (tmp.src.bo != src_bo)
		kgem_bo_destroy(&sna->kgem, tmp.src.bo);
	return true;

fallback_tiled_src:
	if (tmp.src.bo != src_bo)
		kgem_bo_destroy(&sna->kgem, tmp.src.bo);
fallback_tiled_dst:
	if (tmp.redirect.real_bo)
		kgem_bo_destroy(&sna->kgem, tmp.dst.bo);
fallback_tiled:
	return sna_tiling_copy_boxes(sna, alu,
				     src, src_bo, src_dx, src_dy,
				     dst, dst_bo, dst_dx, dst_dy,
				     box, n);
}

static void
gen6_render_copy_blt(struct sna *sna,
		     const struct sna_copy_op *op,
		     int16_t sx, int16_t sy,
		     int16_t w,  int16_t h,
		     int16_t dx, int16_t dy)
{
	int16_t *v;

	gen6_get_rectangles(sna, &op->base, 1, gen6_emit_copy_state);

	v = (int16_t *)&sna->render.vertices[sna->render.vertex_used];
	sna->render.vertex_used += 6;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	v[0]  = dx+w; v[1]  = dy+h;
	v[2]  = sx+w; v[3]  = sy+h;
	v[4]  = dx;   v[5]  = dy+h;
	v[6]  = sx;   v[7]  = sy+h;
	v[8]  = dx;   v[9]  = dy;
	v[10] = sx;   v[11] = sy;
}

static void
gen6_render_copy_done(struct sna *sna, const struct sna_copy_op *op)
{
	DBG(("%s()\n", __FUNCTION__));

	if (sna->render_state.gen6.vertex_offset)
		gen6_vertex_flush(sna);
}

static bool
gen6_render_copy(struct sna *sna, uint8_t alu,
		 PixmapPtr src, struct kgem_bo *src_bo,
		 PixmapPtr dst, struct kgem_bo *dst_bo,
		 struct sna_copy_op *op)
{
	DBG(("%s (alu=%d, src=(%dx%d), dst=(%dx%d))\n",
	     __FUNCTION__, alu,
	     src->drawable.width, src->drawable.height,
	     dst->drawable.width, dst->drawable.height));

	if (prefer_blt_copy(sna, src_bo, dst_bo, 0) &&
	    sna_blt_compare_depth(&src->drawable, &dst->drawable) &&
	    sna_blt_copy(sna, alu,
			 src_bo, dst_bo,
			 dst->drawable.bitsPerPixel,
			 op))
		return true;

	if (!(alu == GXcopy || alu == GXclear) || src_bo == dst_bo ||
	    too_large(src->drawable.width, src->drawable.height) ||
	    too_large(dst->drawable.width, dst->drawable.height)) {
fallback:
		if (!sna_blt_compare_depth(&src->drawable, &dst->drawable))
			return false;

		return sna_blt_copy(sna, alu, src_bo, dst_bo,
				    dst->drawable.bitsPerPixel,
				    op);
	}

	if (dst->drawable.depth == src->drawable.depth) {
		op->base.dst.format = sna_render_format_for_depth(dst->drawable.depth);
		op->base.src.pict_format = op->base.dst.format;
	} else {
		op->base.dst.format = sna_format_for_depth(dst->drawable.depth);
		op->base.src.pict_format = sna_format_for_depth(src->drawable.depth);
	}
	if (!gen6_check_format(op->base.src.pict_format))
		goto fallback;

	op->base.dst.pixmap = dst;
	op->base.dst.width  = dst->drawable.width;
	op->base.dst.height = dst->drawable.height;
	op->base.dst.bo = dst_bo;

	op->base.src.bo = src_bo;
	op->base.src.card_format =
		gen6_get_card_format(op->base.src.pict_format);
	op->base.src.width  = src->drawable.width;
	op->base.src.height = src->drawable.height;

	op->base.mask.bo = NULL;

	op->base.floats_per_vertex = 2;
	op->base.floats_per_rect = 6;

	op->base.u.gen6.flags = COPY_FLAGS(alu);
	assert(GEN6_KERNEL(op->base.u.gen6.flags) == GEN6_WM_KERNEL_NOMASK);
	assert(GEN6_SAMPLER(op->base.u.gen6.flags) == COPY_SAMPLER);
	assert(GEN6_VERTEX(op->base.u.gen6.flags) == COPY_VERTEX);

	kgem_set_mode(&sna->kgem, KGEM_RENDER, dst_bo);
	if (!kgem_check_bo(&sna->kgem, dst_bo, src_bo, NULL)) {
		kgem_submit(&sna->kgem);
		if (!kgem_check_bo(&sna->kgem, dst_bo, src_bo, NULL))
			goto fallback;
		_kgem_set_mode(&sna->kgem, KGEM_RENDER);
	}

	gen6_emit_copy_state(sna, &op->base);
	gen6_align_vertex(sna, &op->base);

	op->blt  = gen6_render_copy_blt;
	op->done = gen6_render_copy_done;
	return true;
}

static void
gen6_emit_fill_state(struct sna *sna, const struct sna_composite_op *op)
{
	uint32_t *binding_table;
	uint16_t offset;
	bool dirty;

	dirty = gen6_get_batch(sna, op);

	binding_table = gen6_composite_get_binding_table(sna, &offset);

	binding_table[0] =
		gen6_bind_bo(sna,
			     op->dst.bo, op->dst.width, op->dst.height,
			     gen6_get_dest_format(op->dst.format),
			     true);
	binding_table[1] =
		gen6_bind_bo(sna,
			     op->src.bo, 1, 1,
			     GEN6_SURFACEFORMAT_B8G8R8A8_UNORM,
			     false);

	if (sna->kgem.surface == offset &&
	    *(uint64_t *)(sna->kgem.batch + sna->render_state.gen6.surface_table) == *(uint64_t*)binding_table) {
		sna->kgem.surface +=
			sizeof(struct gen6_surface_state_padded)/sizeof(uint32_t);
		offset = sna->render_state.gen6.surface_table;
	}

	gen6_emit_state(sna, op, offset | dirty);
}

static inline bool prefer_blt_fill(struct sna *sna,
				   struct kgem_bo *bo)
{
	if (PREFER_RENDER)
		return PREFER_RENDER < 0;

	return (can_switch_to_blt(sna) ||
		prefer_blt_ring(sna) ||
		untiled_tlb_miss(bo));
}

static bool
gen6_render_fill_boxes(struct sna *sna,
		       CARD8 op,
		       PictFormat format,
		       const xRenderColor *color,
		       PixmapPtr dst, struct kgem_bo *dst_bo,
		       const BoxRec *box, int n)
{
	struct sna_composite_op tmp;
	uint32_t pixel;

	DBG(("%s (op=%d, color=(%04x, %04x, %04x, %04x) [%08x])\n",
	     __FUNCTION__, op,
	     color->red, color->green, color->blue, color->alpha, (int)format));

	if (op >= ARRAY_SIZE(gen6_blend_op)) {
		DBG(("%s: fallback due to unhandled blend op: %d\n",
		     __FUNCTION__, op));
		return false;
	}

	if (prefer_blt_fill(sna, dst_bo) || !gen6_check_dst_format(format)) {
		uint8_t alu = GXinvalid;

		if (op <= PictOpSrc) {
			pixel = 0;
			if (op == PictOpClear)
				alu = GXclear;
			else if (sna_get_pixel_from_rgba(&pixel,
							 color->red,
							 color->green,
							 color->blue,
							 color->alpha,
							 format))
				alu = GXcopy;
		}

		if (alu != GXinvalid &&
		    sna_blt_fill_boxes(sna, alu,
				       dst_bo, dst->drawable.bitsPerPixel,
				       pixel, box, n))
			return true;

		if (!gen6_check_dst_format(format))
			return false;
	}

	if (op == PictOpClear) {
		pixel = 0;
		op = PictOpSrc;
	} else if (!sna_get_pixel_from_rgba(&pixel,
					    color->red,
					    color->green,
					    color->blue,
					    color->alpha,
					    PICT_a8r8g8b8))
		return false;

	DBG(("%s(%08x x %d [(%d, %d), (%d, %d) ...])\n",
	     __FUNCTION__, pixel, n,
	     box[0].x1, box[0].y1, box[0].x2, box[0].y2));

	tmp.dst.pixmap = dst;
	tmp.dst.width  = dst->drawable.width;
	tmp.dst.height = dst->drawable.height;
	tmp.dst.format = format;
	tmp.dst.bo = dst_bo;
	tmp.dst.x = tmp.dst.y = 0;
	tmp.damage = NULL;

	sna_render_composite_redirect_init(&tmp);
	if (too_large(dst->drawable.width, dst->drawable.height)) {
		BoxRec extents;

		boxes_extents(box, n, &extents);
		if (!sna_render_composite_redirect(sna, &tmp,
						   extents.x1, extents.y1,
						   extents.x2 - extents.x1,
						   extents.y2 - extents.y1))
			return sna_tiling_fill_boxes(sna, op, format, color,
						     dst, dst_bo, box, n);
	}

	tmp.src.bo = sna_render_get_solid(sna, pixel);
	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 2;
	tmp.floats_per_rect = 6;
	tmp.need_magic_ca_pass = false;

	tmp.u.gen6.flags = FILL_FLAGS(op, format);
	assert(GEN6_KERNEL(tmp.u.gen6.flags) == GEN6_WM_KERNEL_NOMASK);
	assert(GEN6_SAMPLER(tmp.u.gen6.flags) == FILL_SAMPLER);
	assert(GEN6_VERTEX(tmp.u.gen6.flags) == FILL_VERTEX);

	if (!kgem_check_bo(&sna->kgem, dst_bo, NULL)) {
		kgem_submit(&sna->kgem);
		assert(kgem_check_bo(&sna->kgem, dst_bo, NULL));
	}

	gen6_emit_fill_state(sna, &tmp);
	gen6_align_vertex(sna, &tmp);

	do {
		int n_this_time;
		int16_t *v;

		n_this_time = gen6_get_rectangles(sna, &tmp, n,
						  gen6_emit_fill_state);
		n -= n_this_time;

		v = (int16_t *)(sna->render.vertices + sna->render.vertex_used);
		sna->render.vertex_used += 6 * n_this_time;
		assert(sna->render.vertex_used <= sna->render.vertex_size);
		do {
			DBG(("	(%d, %d), (%d, %d)\n",
			     box->x1, box->y1, box->x2, box->y2));

			v[0] = box->x2;
			v[5] = v[1] = box->y2;
			v[8] = v[4] = box->x1;
			v[9] = box->y1;
			v[2] = v[3]  = v[7]  = 1;
			v[6] = v[10] = v[11] = 0;
			v += 12; box++;
		} while (--n_this_time);
	} while (n);

	gen6_vertex_flush(sna);
	kgem_bo_destroy(&sna->kgem, tmp.src.bo);
	sna_render_composite_redirect_done(sna, &tmp);
	return true;
}

static void
gen6_render_op_fill_blt(struct sna *sna,
			const struct sna_fill_op *op,
			int16_t x, int16_t y, int16_t w, int16_t h)
{
	int16_t *v;

	DBG(("%s: (%d, %d)x(%d, %d)\n", __FUNCTION__, x, y, w, h));

	gen6_get_rectangles(sna, &op->base, 1, gen6_emit_fill_state);

	v = (int16_t *)&sna->render.vertices[sna->render.vertex_used];
	sna->render.vertex_used += 6;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	v[0] = x+w;
	v[4] = v[8] = x;
	v[1] = v[5] = y+h;
	v[9] = y;

	v[2] = v[3]  = v[7]  = 1;
	v[6] = v[10] = v[11] = 0;
}

fastcall static void
gen6_render_op_fill_box(struct sna *sna,
			const struct sna_fill_op *op,
			const BoxRec *box)
{
	int16_t *v;

	DBG(("%s: (%d, %d),(%d, %d)\n", __FUNCTION__,
	     box->x1, box->y1, box->x2, box->y2));

	gen6_get_rectangles(sna, &op->base, 1, gen6_emit_fill_state);

	v = (int16_t *)&sna->render.vertices[sna->render.vertex_used];
	sna->render.vertex_used += 6;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	v[0] = box->x2;
	v[8] = v[4] = box->x1;
	v[5] = v[1] = box->y2;
	v[9] = box->y1;

	v[7] = v[2]  = v[3]  = 1;
	v[6] = v[10] = v[11] = 0;
}

fastcall static void
gen6_render_op_fill_boxes(struct sna *sna,
			  const struct sna_fill_op *op,
			  const BoxRec *box,
			  int nbox)
{
	DBG(("%s: (%d, %d),(%d, %d)... x %d\n", __FUNCTION__,
	     box->x1, box->y1, box->x2, box->y2, nbox));

	do {
		int nbox_this_time;
		int16_t *v;

		nbox_this_time = gen6_get_rectangles(sna, &op->base, nbox,
						     gen6_emit_fill_state);
		nbox -= nbox_this_time;

		v = (int16_t *)&sna->render.vertices[sna->render.vertex_used];
		sna->render.vertex_used += 6 * nbox_this_time;
		assert(sna->render.vertex_used <= sna->render.vertex_size);

		do {
			v[0] = box->x2;
			v[8] = v[4] = box->x1;
			v[5] = v[1] = box->y2;
			v[9] = box->y1;
			v[7] = v[2]  = v[3]  = 1;
			v[6] = v[10] = v[11] = 0;
			box++; v += 12;
		} while (--nbox_this_time);
	} while (nbox);
}

static void
gen6_render_op_fill_done(struct sna *sna, const struct sna_fill_op *op)
{
	DBG(("%s()\n", __FUNCTION__));

	if (sna->render_state.gen6.vertex_offset)
		gen6_vertex_flush(sna);
	kgem_bo_destroy(&sna->kgem, op->base.src.bo);
}

static bool
gen6_render_fill(struct sna *sna, uint8_t alu,
		 PixmapPtr dst, struct kgem_bo *dst_bo,
		 uint32_t color,
		 struct sna_fill_op *op)
{
	DBG(("%s: (alu=%d, color=%x)\n", __FUNCTION__, alu, color));

	if (prefer_blt_fill(sna, dst_bo) &&
	    sna_blt_fill(sna, alu,
			 dst_bo, dst->drawable.bitsPerPixel,
			 color,
			 op))
		return true;

	if (!(alu == GXcopy || alu == GXclear) ||
	    too_large(dst->drawable.width, dst->drawable.height))
		return sna_blt_fill(sna, alu,
				    dst_bo, dst->drawable.bitsPerPixel,
				    color,
				    op);

	if (alu == GXclear)
		color = 0;

	op->base.dst.pixmap = dst;
	op->base.dst.width  = dst->drawable.width;
	op->base.dst.height = dst->drawable.height;
	op->base.dst.format = sna_format_for_depth(dst->drawable.depth);
	op->base.dst.bo = dst_bo;
	op->base.dst.x = op->base.dst.y = 0;

	op->base.src.bo =
		sna_render_get_solid(sna,
				     sna_rgba_for_color(color,
							dst->drawable.depth));
	op->base.mask.bo = NULL;

	op->base.need_magic_ca_pass = false;
	op->base.floats_per_vertex = 2;
	op->base.floats_per_rect = 6;

	op->base.u.gen6.flags = FILL_FLAGS_NOBLEND;
	assert(GEN6_KERNEL(op->base.u.gen6.flags) == GEN6_WM_KERNEL_NOMASK);
	assert(GEN6_SAMPLER(op->base.u.gen6.flags) == FILL_SAMPLER);
	assert(GEN6_VERTEX(op->base.u.gen6.flags) == FILL_VERTEX);

	if (!kgem_check_bo(&sna->kgem, dst_bo, NULL)) {
		kgem_submit(&sna->kgem);
		assert(kgem_check_bo(&sna->kgem, dst_bo, NULL));
	}

	gen6_emit_fill_state(sna, &op->base);
	gen6_align_vertex(sna, &op->base);

	op->blt  = gen6_render_op_fill_blt;
	op->box  = gen6_render_op_fill_box;
	op->boxes = gen6_render_op_fill_boxes;
	op->done = gen6_render_op_fill_done;
	return true;
}

static bool
gen6_render_fill_one_try_blt(struct sna *sna, PixmapPtr dst, struct kgem_bo *bo,
			     uint32_t color,
			     int16_t x1, int16_t y1, int16_t x2, int16_t y2,
			     uint8_t alu)
{
	BoxRec box;

	box.x1 = x1;
	box.y1 = y1;
	box.x2 = x2;
	box.y2 = y2;

	return sna_blt_fill_boxes(sna, alu,
				  bo, dst->drawable.bitsPerPixel,
				  color, &box, 1);
}

static bool
gen6_render_fill_one(struct sna *sna, PixmapPtr dst, struct kgem_bo *bo,
		     uint32_t color,
		     int16_t x1, int16_t y1,
		     int16_t x2, int16_t y2,
		     uint8_t alu)
{
	struct sna_composite_op tmp;
	int16_t *v;

	/* Prefer to use the BLT if already engaged */
	if (prefer_blt_fill(sna, bo) &&
	    gen6_render_fill_one_try_blt(sna, dst, bo, color,
					 x1, y1, x2, y2, alu))
		return true;

	/* Must use the BLT if we can't RENDER... */
	if (!(alu == GXcopy || alu == GXclear) ||
	    too_large(dst->drawable.width, dst->drawable.height))
		return gen6_render_fill_one_try_blt(sna, dst, bo, color,
						    x1, y1, x2, y2, alu);

	if (alu == GXclear)
		color = 0;

	tmp.dst.pixmap = dst;
	tmp.dst.width  = dst->drawable.width;
	tmp.dst.height = dst->drawable.height;
	tmp.dst.format = sna_format_for_depth(dst->drawable.depth);
	tmp.dst.bo = bo;
	tmp.dst.x = tmp.dst.y = 0;

	tmp.src.bo =
		sna_render_get_solid(sna,
				     sna_rgba_for_color(color,
							dst->drawable.depth));
	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 2;
	tmp.floats_per_rect = 6;
	tmp.need_magic_ca_pass = false;

	tmp.u.gen6.flags = FILL_FLAGS_NOBLEND;
	assert(GEN6_KERNEL(tmp.u.gen6.flags) == GEN6_WM_KERNEL_NOMASK);
	assert(GEN6_SAMPLER(tmp.u.gen6.flags) == FILL_SAMPLER);
	assert(GEN6_VERTEX(tmp.u.gen6.flags) == FILL_VERTEX);

	if (!kgem_check_bo(&sna->kgem, bo, NULL)) {
		_kgem_submit(&sna->kgem);
		assert(kgem_check_bo(&sna->kgem, bo, NULL));
	}

	gen6_emit_fill_state(sna, &tmp);
	gen6_align_vertex(sna, &tmp);

	gen6_get_rectangles(sna, &tmp, 1, gen6_emit_fill_state);

	DBG(("	(%d, %d), (%d, %d)\n", x1, y1, x2, y2));

	v = (int16_t *)&sna->render.vertices[sna->render.vertex_used];
	sna->render.vertex_used += 6;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	v[0] = x2;
	v[8] = v[4] = x1;
	v[5] = v[1] = y2;
	v[9] = y1;
	v[7] = v[2]  = v[3]  = 1;
	v[6] = v[10] = v[11] = 0;

	gen6_vertex_flush(sna);
	kgem_bo_destroy(&sna->kgem, tmp.src.bo);

	return true;
}

static bool
gen6_render_clear_try_blt(struct sna *sna, PixmapPtr dst, struct kgem_bo *bo)
{
	BoxRec box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = dst->drawable.width;
	box.y2 = dst->drawable.height;

	return sna_blt_fill_boxes(sna, GXclear,
				  bo, dst->drawable.bitsPerPixel,
				  0, &box, 1);
}

static bool
gen6_render_clear(struct sna *sna, PixmapPtr dst, struct kgem_bo *bo)
{
	struct sna_composite_op tmp;
	int16_t *v;

	DBG(("%s: %dx%d\n",
	     __FUNCTION__,
	     dst->drawable.width,
	     dst->drawable.height));

	/* Prefer to use the BLT if, and only if, already engaged */
	if (sna->kgem.ring == KGEM_BLT &&
	    gen6_render_clear_try_blt(sna, dst, bo))
		return true;

	/* Must use the BLT if we can't RENDER... */
	if (too_large(dst->drawable.width, dst->drawable.height))
		return gen6_render_clear_try_blt(sna, dst, bo);

	tmp.dst.pixmap = dst;
	tmp.dst.width  = dst->drawable.width;
	tmp.dst.height = dst->drawable.height;
	tmp.dst.format = sna_format_for_depth(dst->drawable.depth);
	tmp.dst.bo = bo;
	tmp.dst.x = tmp.dst.y = 0;

	tmp.src.bo = sna_render_get_solid(sna, 0);
	tmp.mask.bo = NULL;

	tmp.floats_per_vertex = 2;
	tmp.floats_per_rect = 6;
	tmp.need_magic_ca_pass = false;

	tmp.u.gen6.flags = FILL_FLAGS_NOBLEND;
	assert(GEN6_KERNEL(tmp.u.gen6.flags) == GEN6_WM_KERNEL_NOMASK);
	assert(GEN6_SAMPLER(tmp.u.gen6.flags) == FILL_SAMPLER);
	assert(GEN6_VERTEX(tmp.u.gen6.flags) == FILL_VERTEX);

	if (!kgem_check_bo(&sna->kgem, bo, NULL)) {
		_kgem_submit(&sna->kgem);
		assert(kgem_check_bo(&sna->kgem, bo, NULL));
	}

	gen6_emit_fill_state(sna, &tmp);
	gen6_align_vertex(sna, &tmp);

	gen6_get_rectangles(sna, &tmp, 1, gen6_emit_fill_state);

	v = (int16_t *)&sna->render.vertices[sna->render.vertex_used];
	sna->render.vertex_used += 6;
	assert(sna->render.vertex_used <= sna->render.vertex_size);

	v[0] = dst->drawable.width;
	v[5] = v[1] = dst->drawable.height;
	v[8] = v[4] = 0;
	v[9] = 0;

	v[7] = v[2]  = v[3]  = 1;
	v[6] = v[10] = v[11] = 0;

	gen6_vertex_flush(sna);
	kgem_bo_destroy(&sna->kgem, tmp.src.bo);

	return true;
}

static void gen6_render_flush(struct sna *sna)
{
	gen6_vertex_close(sna);
}

static void
gen6_render_context_switch(struct kgem *kgem,
			   int new_mode)
{
	if (!new_mode)
		return;

	 DBG(("%s: from %d to %d\n", __FUNCTION__, kgem->mode, new_mode));

	if (kgem->mode)
		kgem_submit(kgem);

	kgem->ring = new_mode;
}

static void
gen6_render_retire(struct kgem *kgem)
{
	struct sna *sna;

	if (kgem->ring && (kgem->has_semaphores || !kgem->need_retire))
		kgem->ring = kgem->mode;

	sna = container_of(kgem, struct sna, kgem);
	if (kgem->nbatch == 0 && sna->render.vbo && !kgem_bo_is_busy(sna->render.vbo)) {
		DBG(("%s: resetting idle vbo handle=%d\n", __FUNCTION__, sna->render.vbo->handle));
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
	}
}

static void
gen6_render_expire(struct kgem *kgem)
{
	struct sna *sna;

	sna = container_of(kgem, struct sna, kgem);
	if (sna->render.vbo && !sna->render.vertex_used) {
		DBG(("%s: discarding vbo handle=%d\n", __FUNCTION__, sna->render.vbo->handle));
		kgem_bo_destroy(kgem, sna->render.vbo);
		sna->render.vbo = NULL;
		sna->render.vertices = sna->render.vertex_data;
		sna->render.vertex_size = ARRAY_SIZE(sna->render.vertex_data);
		sna->render.vertex_used = 0;
		sna->render.vertex_index = 0;
	}
}

static void gen6_render_reset(struct sna *sna)
{
	sna->render_state.gen6.needs_invariant = true;
	sna->render_state.gen6.first_state_packet = true;
	sna->render_state.gen6.vb_id = 0;
	sna->render_state.gen6.ve_id = 3 << 2;
	sna->render_state.gen6.last_primitive = -1;

	sna->render_state.gen6.num_sf_outputs = 0;
	sna->render_state.gen6.samplers = -1;
	sna->render_state.gen6.blend = -1;
	sna->render_state.gen6.kernel = -1;
	sna->render_state.gen6.drawrect_offset = -1;
	sna->render_state.gen6.drawrect_limit = -1;
	sna->render_state.gen6.surface_table = -1;
}

static void gen6_render_fini(struct sna *sna)
{
	kgem_bo_destroy(&sna->kgem, sna->render_state.gen6.general_bo);
}

static bool gen6_render_setup(struct sna *sna)
{
	struct gen6_render_state *state = &sna->render_state.gen6;
	struct sna_static_stream general;
	struct gen6_sampler_state *ss;
	int i, j, k, l, m;

	state->info = &gt1_info;
	if (DEVICE_ID(sna->PciInfo) & 0x20)
		state->info = &gt2_info; /* XXX requires GT_MODE WiZ disabled */

	sna_static_stream_init(&general);

	/* Zero pad the start. If you see an offset of 0x0 in the batchbuffer
	 * dumps, you know it points to zero.
	 */
	null_create(&general);
	scratch_create(&general);

	for (m = 0; m < GEN6_KERNEL_COUNT; m++) {
		if (wm_kernels[m].size) {
			state->wm_kernel[m][1] =
				sna_static_stream_add(&general,
						      wm_kernels[m].data,
						      wm_kernels[m].size,
						      64);
		} else {
			if (USE_8_PIXEL_DISPATCH) {
				state->wm_kernel[m][0] =
					sna_static_stream_compile_wm(sna, &general,
								     wm_kernels[m].data, 8);
			}

			if (USE_16_PIXEL_DISPATCH) {
				state->wm_kernel[m][1] =
					sna_static_stream_compile_wm(sna, &general,
								     wm_kernels[m].data, 16);
			}

			if (USE_32_PIXEL_DISPATCH) {
				state->wm_kernel[m][2] =
					sna_static_stream_compile_wm(sna, &general,
								     wm_kernels[m].data, 32);
			}
		}
		if ((state->wm_kernel[m][0]|state->wm_kernel[m][1]|state->wm_kernel[m][2]) == 0) {
			state->wm_kernel[m][1] =
				sna_static_stream_compile_wm(sna, &general,
							     wm_kernels[m].data, 16);
		}
	}

	ss = sna_static_stream_map(&general,
				   2 * sizeof(*ss) *
				   (2 +
				    FILTER_COUNT * EXTEND_COUNT *
				    FILTER_COUNT * EXTEND_COUNT),
				   32);
	state->wm_state = sna_static_stream_offsetof(&general, ss);
	sampler_copy_init(ss); ss += 2;
	sampler_fill_init(ss); ss += 2;
	for (i = 0; i < FILTER_COUNT; i++) {
		for (j = 0; j < EXTEND_COUNT; j++) {
			for (k = 0; k < FILTER_COUNT; k++) {
				for (l = 0; l < EXTEND_COUNT; l++) {
					sampler_state_init(ss++, i, j);
					sampler_state_init(ss++, k, l);
				}
			}
		}
	}

	state->cc_vp = gen6_create_cc_viewport(&general);
	state->cc_blend = gen6_composite_create_blend_state(&general);

	state->general_bo = sna_static_stream_fini(sna, &general);
	return state->general_bo != NULL;
}

bool gen6_render_init(struct sna *sna)
{
	if (!gen6_render_setup(sna))
		return false;

	sna->kgem.context_switch = gen6_render_context_switch;
	sna->kgem.retire = gen6_render_retire;
	sna->kgem.expire = gen6_render_expire;

#if !NO_COMPOSITE
	sna->render.composite = gen6_render_composite;
#endif
#if !NO_COMPOSITE_SPANS
	sna->render.check_composite_spans = gen6_check_composite_spans;
	sna->render.composite_spans = gen6_render_composite_spans;
#endif
	sna->render.video = gen6_render_video;

#if !NO_COPY_BOXES
	sna->render.copy_boxes = gen6_render_copy_boxes;
#endif
#if !NO_COPY
	sna->render.copy = gen6_render_copy;
#endif

#if !NO_FILL_BOXES
	sna->render.fill_boxes = gen6_render_fill_boxes;
#endif
#if !NO_FILL
	sna->render.fill = gen6_render_fill;
#endif
#if !NO_FILL_ONE
	sna->render.fill_one = gen6_render_fill_one;
#endif
#if !NO_FILL_CLEAR
	sna->render.clear = gen6_render_clear;
#endif

	sna->render.flush = gen6_render_flush;
	sna->render.reset = gen6_render_reset;
	sna->render.fini = gen6_render_fini;

	sna->render.max_3d_size = GEN6_MAX_SIZE;
	sna->render.max_3d_pitch = 1 << 18;
	return true;
}
