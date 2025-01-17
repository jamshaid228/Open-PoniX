/*
 * Copyright © 2011 Intel Corporation.
 *             2012 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including
 * the next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xf86.h>
#define GLAMOR_FOR_XORG  1
#include <glamor.h>

#include "radeon.h"
#include "radeon_bo_helper.h"

#if HAS_DEVPRIVATEKEYREC
DevPrivateKeyRec glamor_pixmap_index;
#else
int glamor_pixmap_index;
#endif

void
radeon_glamor_exchange_buffers(PixmapPtr src,
			       PixmapPtr dst)
{
	RADEONInfoPtr info = RADEONPTR(xf86ScreenToScrn(dst->drawable.pScreen));

	if (!info->use_glamor)
		return;
	glamor_egl_exchange_buffers(src, dst);
}

Bool
radeon_glamor_create_screen_resources(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	RADEONInfoPtr info = RADEONPTR(scrn);

	if (!info->use_glamor)
		return TRUE;

	if (!glamor_glyphs_init(screen))
		return FALSE;

	if (!glamor_egl_create_textured_screen_ext(screen,
						   info->front_bo->handle,
						   scrn->displayWidth *
						   info->pixel_bytes,
						   NULL))
		return FALSE;

	return TRUE;
}


Bool
radeon_glamor_pre_init(ScrnInfoPtr scrn)
{
	RADEONInfoPtr info = RADEONPTR(scrn);
	pointer glamor_module;
	CARD32 version;
	const char *s;

	if (!info->dri2.available)
		return FALSE;

	s = xf86GetOptValString(info->Options, OPTION_ACCELMETHOD);
	if (s == NULL)
		return FALSE;

	if (strcasecmp(s, "glamor") != 0)
		return FALSE;

	/* Load glamor module */
	if ((glamor_module = xf86LoadSubModule(scrn, GLAMOR_EGL_MODULE_NAME))) {
		version = xf86GetModuleVersion(glamor_module);
		if (version < MODULE_VERSION_NUMERIC(0,3,1)) {
			xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			"Incompatible glamor version, required >= 0.3.0.\n");
			return FALSE;
		} else {
			if (glamor_egl_init(scrn, info->dri2.drm_fd)) {
				xf86DrvMsg(scrn->scrnIndex, X_INFO,
					   "glamor detected, initialising EGL layer.\n");
			} else {
				xf86DrvMsg(scrn->scrnIndex, X_ERROR,
					   "glamor detected, failed to initialize EGL.\n");
				return FALSE;
			}
		}
	} else {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR, "glamor not available\n");
		return FALSE;
	}

	info->use_glamor = TRUE;

	return TRUE;
}

Bool
radeon_glamor_create_textured_pixmap(PixmapPtr pixmap)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(pixmap->drawable.pScreen);
	RADEONInfoPtr info = RADEONPTR(scrn);
	struct radeon_pixmap *priv;

	if ((info->use_glamor) == 0)
		return TRUE;

	priv = radeon_get_pixmap_private(pixmap);
	if (glamor_egl_create_textured_pixmap(pixmap, priv->bo->handle,
					      priv->stride))
		return TRUE;
	else
		return FALSE;
}

Bool radeon_glamor_pixmap_is_offscreen(PixmapPtr pixmap)
{
	struct radeon_pixmap *priv = radeon_get_pixmap_private(pixmap);
	return priv && priv->bo;
}

Bool radeon_glamor_prepare_access(PixmapPtr pixmap, glamor_access_t access)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(pixmap->drawable.pScreen);
	RADEONInfoPtr info = RADEONPTR(scrn);
	struct radeon_bo *bo;
	int ret;

	if (access == GLAMOR_GPU_ACCESS_RW || access == GLAMOR_GPU_ACCESS_RO) {
		if (info->ChipFamily < CHIP_FAMILY_TAHITI)
			return TRUE;

		return info->accel_state->force;
	}

	bo = radeon_get_pixmap_bo(pixmap);
	if (bo) {
		/* When falling back to swrast, flush all pending operations */
		if (info->ChipFamily < CHIP_FAMILY_TAHITI)
			radeon_glamor_flush(scrn);

		ret = radeon_bo_map(bo, 1);
		if (ret) {
			xf86DrvMsg(scrn->scrnIndex, X_WARNING,
				   "%s: bo map (tiling_flags %d, access %d) failed: %s\n",
				   __FUNCTION__,
				   radeon_get_pixmap_private(pixmap)->tiling_flags,
				   access,
				   strerror(-ret));
			return FALSE;
		}

		pixmap->devPrivate.ptr = bo->ptr;
	}

	return TRUE;
}

void
radeon_glamor_finish_access(PixmapPtr pixmap, glamor_access_t access)
{
	struct radeon_bo *bo;

	switch(access) {
	case GLAMOR_GPU_ACCESS_RW:
	case GLAMOR_GPU_ACCESS_RO:
		break;
	case GLAMOR_CPU_ACCESS_RO:
	case GLAMOR_CPU_ACCESS_RW:
		bo = radeon_get_pixmap_bo(pixmap);
		if (bo) {
			radeon_bo_unmap(bo);
			pixmap->devPrivate.ptr = NULL;
		}
		break;
	default:
		ErrorF("Invalid access mode %d\n", access);
	}

	return;
}

static PixmapPtr
radeon_glamor_create_pixmap(ScreenPtr screen, int w, int h, int depth,
			unsigned usage)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	RADEONInfoPtr info = RADEONPTR(scrn);
	struct radeon_pixmap *priv;
	PixmapPtr pixmap, new_pixmap = NULL;

	if (!(usage & RADEON_CREATE_PIXMAP_DRI2)) {
		if (info->ChipFamily < CHIP_FAMILY_TAHITI) {
			pixmap = glamor_create_pixmap(screen, w, h, depth, usage);
			if (pixmap)
				return pixmap;
		} else
			return fbCreatePixmap(screen, w, h, depth, usage);
	}

	if (w > 32767 || h > 32767)
		return NullPixmap;

	if (depth == 1)
		return fbCreatePixmap(screen, w, h, depth, usage);

	if (usage == CREATE_PIXMAP_USAGE_GLYPH_PICTURE && w <= 32 && h <= 32)
		return fbCreatePixmap(screen, w, h, depth, usage);

	pixmap = fbCreatePixmap(screen, 0, 0, depth, usage);
	if (pixmap == NullPixmap)
		return pixmap;

	if (w && h) {
		priv = calloc(1, sizeof (struct radeon_pixmap));
		if (priv == NULL)
			goto fallback_pixmap;

		priv->bo = radeon_alloc_pixmap_bo(scrn, w, h, depth, usage,
						  pixmap->drawable.bitsPerPixel,
						  &priv->stride,
						  &priv->surface,
						  &priv->tiling_flags);
		if (!priv->bo)
			goto fallback_priv;

		radeon_set_pixmap_private(pixmap, priv);

		screen->ModifyPixmapHeader(pixmap, w, h, 0, 0, priv->stride, NULL);

		if (!radeon_glamor_create_textured_pixmap(pixmap))
			goto fallback_glamor;
	}

	return pixmap;

fallback_glamor:
	if (usage & RADEON_CREATE_PIXMAP_DRI2) {
	/* XXX need further work to handle the DRI2 failure case.
	 * Glamor don't know how to handle a BO only pixmap. Put
	 * a warning indicator here.
	 */
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "Failed to create textured DRI2 pixmap.");
		return pixmap;
	}
	/* Create textured pixmap failed means glamor failed to
	 * create a texture from current BO for some reasons. We turn
	 * to create a new glamor pixmap and clean up current one.
	 * One thing need to be noted, this new pixmap doesn't
	 * has a priv and bo attached to it. It's glamor's responsbility
	 * to take care of it. Glamor will mark this new pixmap as a
	 * texture only pixmap and will never fallback to DDX layer
	 * afterwards.
	 */
	new_pixmap = glamor_create_pixmap(screen, w, h,	depth, usage);
	radeon_bo_unref(priv->bo);
fallback_priv:
	free(priv);
fallback_pixmap:
	fbDestroyPixmap(pixmap);
	if (new_pixmap)
		return new_pixmap;
	else
		return fbCreatePixmap(screen, w, h, depth, usage);
}

static Bool radeon_glamor_destroy_pixmap(PixmapPtr pixmap)
{
	if (pixmap->refcnt == 1) {
		glamor_egl_destroy_textured_pixmap(pixmap);
		radeon_set_pixmap_bo(pixmap, NULL);
	}
	fbDestroyPixmap(pixmap);
	return TRUE;
}

Bool
radeon_glamor_init(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	RADEONInfoPtr info = RADEONPTR(scrn);
	unsigned int glamor_init_flags = GLAMOR_INVERTED_Y_AXIS | GLAMOR_USE_EGL_SCREEN;

	if (info->ChipFamily < CHIP_FAMILY_TAHITI)
		glamor_init_flags |= GLAMOR_USE_SCREEN | GLAMOR_USE_PICTURE_SCREEN;

	if (!glamor_init(screen, glamor_init_flags)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Failed to initialize glamor.\n");
		return FALSE;
	}

	if (!glamor_egl_init_textured_pixmap(screen)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Failed to initialize textured pixmap of screen for glamor.\n");
		return FALSE;
	}

#if HAS_DIXREGISTERPRIVATEKEY
	if (!dixRegisterPrivateKey(&glamor_pixmap_index, PRIVATE_PIXMAP, 0))
#else
	if (!dixRequestPrivate(&glamor_pixmap_index, 0))
#endif
		return FALSE;

	if (!(glamor_init_flags & GLAMOR_USE_SCREEN) &&
	    !glamor_screen_init(screen)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "GLAMOR initialization failed\n");
		return FALSE;
	}

	screen->CreatePixmap = radeon_glamor_create_pixmap;
	screen->DestroyPixmap = radeon_glamor_destroy_pixmap;

	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "Use GLAMOR acceleration.\n");
	return TRUE;
}

void
radeon_glamor_flush(ScrnInfoPtr pScrn)
{
	RADEONInfoPtr info = RADEONPTR(pScrn);

	if (info->use_glamor)
		glamor_block_handler(pScrn->pScreen);
}
