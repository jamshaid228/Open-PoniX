/*--------------------------------*-C-*---------------------------------*
 * File:	command.c
 *----------------------------------------------------------------------*
 * $Id: command.c,v 1.264 2003/03/26 05:59:49 gcw Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 * 				- extensive modifications
 * Copyright (c) 1995      Garrett D'Amore <garrett@netcom.com>
 *				- vt100 printing
 * Copyright (c) 1995      Steven Hirsch <hirsch@emba.uvm.edu>
 *				- X11 mouse report mode and support for
 *				  DEC "private mode" save/restore functions.
 * Copyright (c) 1995      Jakub Jelinek <jj@gnu.ai.mit.edu>
 *				- key-related changes to handle Shift+function
 *				  keys properly.
 * Copyright (c) 1997      MJ Olesen <olesen@me.queensu.ca>
 *				- extensive modifications
 * Copyright (c) 1997      Raul Garcia Garcia <rgg@tid.es>
 *				- modification and cleanups for Solaris 2.x
 *				  and Linux 1.2.x
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 * 				- extensive modifications
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 * Copyright (c) 2001      Marius Gedminas
 *				- Ctrl/Mod4+Tab works like Meta+Tab (options)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

/*{{{ includes: */
#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "version.h"
#include "command.h"

/*----------------------------------------------------------------------*/

/*{{{ Convert the keypress event into a string */
/* INTPROTO */
void
rxvt_lookup_key(rxvt_t *r, XKeyEvent *ev)
{
    int             ctrl, meta, shft, len;
    unsigned int    newlen;
    KeySym          keysym;
#ifdef DEBUG_CMD
    static int      debug_key = 1;	/* accessible by a debugger only */
#endif
#ifdef USE_XIM
    int             valid_keysym;
#endif
    unsigned char  *kbuf = r->h->kbuf;

/*
 * use Num_Lock to toggle Keypad on/off.  If Num_Lock is off, allow an
 * escape sequence to toggle the Keypad.
 *
 * Always permit `shift' to override the current setting
 */
    shft = (ev->state & ShiftMask);
    ctrl = (ev->state & ControlMask);
    meta = (ev->state & r->h->ModMetaMask);
    if (r->numlock_state || (ev->state & r->h->ModNumLockMask)) {
	r->numlock_state = (ev->state & r->h->ModNumLockMask);
	PrivMode((!r->numlock_state), PrivMode_aplKP);
    }
#ifdef USE_XIM
    if (r->h->Input_Context != NULL) {
	Status          status_return;

	kbuf[0] = '\0';
	len = XmbLookupString(r->h->Input_Context, ev, (char *)kbuf,
			      KBUFSZ, &keysym, &status_return);
	valid_keysym = ((status_return == XLookupKeySym)
			|| (status_return == XLookupBoth));
    } else {
	len = XLookupString(ev, (char *)kbuf, KBUFSZ, &keysym,
			    &r->h->compose);
	valid_keysym = 1;
    }
#else				/* USE_XIM */
    len = XLookupString(ev, (char *)kbuf, KBUFSZ, &keysym,
			&r->h->compose);
/*
 * map unmapped Latin[2-4]/Katakana/Arabic/Cyrillic/Greek entries -> Latin1
 * good for installations with correct fonts, but without XLOCALE
 */
    if (!len) {
	if ((keysym >= 0x0100) && (keysym < 0x0800)) {
	    kbuf[0] = (keysym & 0xFF);
	    kbuf[1] = '\0';
	    len = 1;
	} else
	    kbuf[0] = '\0';
    }
#endif				/* USE_XIM */

#ifdef USE_XIM
    if (valid_keysym)
#endif
    {
/* for some backwards compatibility */
#if defined(HOTKEY_CTRL) || defined(HOTKEY_META)
# ifdef HOTKEY_CTRL
	if (ctrl) {
# else
	if (meta) {
# endif
	    if (keysym == r->h->ks_bigfont) {
		rxvt_change_font(r, 0, FONT_UP);
		return;
	    } else if (keysym == r->h->ks_smallfont) {
		rxvt_change_font(r, 0, FONT_DN);
		return;
	    }
	}
#endif

	if (r->TermWin.saveLines) {
#ifdef UNSHIFTED_SCROLLKEYS
	    if (!ctrl && !meta) {
#else
	    if (IS_SCROLL_MOD) {
#endif
		int             lnsppg;

#ifdef PAGING_CONTEXT_LINES
		lnsppg = r->TermWin.nrow - PAGING_CONTEXT_LINES;
#else
		lnsppg = r->TermWin.nrow * 4 / 5;
#endif
		if (keysym == XK_Prior) {
		    rxvt_scr_page(r, UP, lnsppg);
		    return;
		} else if (keysym == XK_Next) {
		    rxvt_scr_page(r, DN, lnsppg);
		    return;
		}
	    }
#ifdef SCROLL_ON_UPDOWN_KEYS
	    if (IS_SCROLL_MOD) {
		if (keysym == XK_Up) {
		    rxvt_scr_page(r, UP, 1);
		    return;
		} else if (keysym == XK_Down) {
		    rxvt_scr_page(r, DN, 1);
		    return;
		}
	    }
#endif
#ifdef SCROLL_ON_HOMEEND_KEYS
	    if (IS_SCROLL_MOD) {
		if (keysym == XK_Home) {
		    rxvt_scr_move_to(r, 0, 1);
		    return;
		} else if (keysym == XK_End) {
		    rxvt_scr_move_to(r, 1, 0);
		    return;
		}
	    }
#endif
	}

	if (shft) {
	/* Shift + F1 - F10 generates F11 - F20 */
	    if (keysym >= XK_F1 && keysym <= XK_F10) {
		keysym += (XK_F11 - XK_F1);
		shft = 0;	/* turn off Shift */
	    } else if (!ctrl && !meta && (r->h->PrivateModes & PrivMode_ShiftKeys)) {
		switch (keysym) {
		/* normal XTerm key bindings */
		case XK_Insert:	/* Shift+Insert = paste mouse selection */
		    rxvt_selection_request(r, ev->time, 0, 0);
		    return;
		/* rxvt extras */
		case XK_KP_Add:	/* Shift+KP_Add = bigger font */
		    rxvt_change_font(r, 0, FONT_UP);
		    return;
		case XK_KP_Subtract:	/* Shift+KP_Subtract = smaller font */
		    rxvt_change_font(r, 0, FONT_DN);
		    return;
		}
	    }
	}
#ifdef PRINTPIPE
	if (keysym == XK_Print) {
	    rxvt_scr_printscreen(r, ctrl | shft);
	    return;
	}
#endif
#ifdef GREEK_SUPPORT
	if (keysym == r->h->ks_greekmodeswith) {
	    r->h->greek_mode = !r->h->greek_mode;
	    if (r->h->greek_mode) {
		rxvt_xterm_seq(r, XTerm_title,
		               (greek_getmode() == GREEK_ELOT928
				? "[Greek: iso]" : "[Greek: ibm]"), CHAR_ST);
		greek_reset();
	    } else
		rxvt_xterm_seq(r, XTerm_title, APL_NAME "-" VERSION, CHAR_ST);
	    return;
	}
#endif

	if (keysym >= 0xFF00 && keysym <= 0xFFFF) {
#ifdef KEYSYM_RESOURCE
	    if (!(shft | ctrl) && r->h->Keysym_map[keysym & 0xFF] != NULL) {
		unsigned int    l;
		const unsigned char *kbuf0;
		const unsigned char ch = C0_ESC;

		kbuf0 = (r->h->Keysym_map[keysym & 0xFF]);
		l = (unsigned int)*kbuf0++;

	    /* escape prefix */
		if (meta)
# ifdef META8_OPTION
		    if (r->h->meta_char == C0_ESC)
# endif
			rxvt_tt_write(r, &ch, 1);
		rxvt_tt_write(r, kbuf0, l);
		return;
	    } else
#endif
	    {
		newlen = 1;
		switch (keysym) {
#ifndef NO_BACKSPACE_KEY
		case XK_BackSpace:
		    if (r->h->PrivateModes & PrivMode_HaveBackSpace) {
			kbuf[0] = (!!(r->h->PrivateModes & PrivMode_BackSpace)
				   ^ !!ctrl) ? '\b' : '\177';
			kbuf[1] = '\0';
		    } else
			STRCPY(kbuf, r->h->key_backspace);
# ifdef MULTICHAR_SET
		    if ((r->Options & Opt_mc_hack) && r->screen.cur.col > 0) {
			int             col, row;

			newlen = STRLEN(kbuf);
			col = r->screen.cur.col - 1;
			row = r->screen.cur.row + r->TermWin.saveLines;
			if (IS_MULTI2(r->screen.rend[row][col]))
			    MEMMOVE(kbuf + newlen, kbuf, newlen + 1);
		    }
# endif
		    break;
#endif
#ifndef NO_DELETE_KEY
		case XK_Delete:
		    STRCPY(kbuf, r->h->key_delete);
# ifdef MULTICHAR_SET
		    if (r->Options & Opt_mc_hack) {
			int             col, row;
 
			newlen = STRLEN(kbuf);
			col = r->screen.cur.col;
			row = r->screen.cur.row + r->TermWin.saveLines;
			if (IS_MULTI1(r->screen.rend[row][col]))
			    MEMMOVE(kbuf + newlen, kbuf, newlen + 1);
		    }
# endif
		    break;
#endif
		case XK_Tab:
		    if (shft)
			STRCPY(kbuf, "\033[Z");
		    else {
#ifdef CTRL_TAB_MAKES_META
			if (ctrl)
			    meta = 1;
#endif
#ifdef MOD4_TAB_MAKES_META
			if (ev->state & Mod4Mask)
			    meta = 1;
#endif
			newlen = 0;
		    }
		    break;


#ifdef XK_KP_Left
		case XK_KP_Up:		/* \033Ox or standard */
		case XK_KP_Down:	/* \033Or or standard */
		case XK_KP_Right:	/* \033Ov or standard */
		case XK_KP_Left:	/* \033Ot or standard */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033OZ");
			kbuf[2] = ("txvr"[keysym - XK_KP_Left]);
			break;
		    } else
		    /* translate to std. cursor key */
			keysym = XK_Left + (keysym - XK_KP_Left);
		/* FALLTHROUGH */
#endif
		case XK_Up:	/* "\033[A" */
		case XK_Down:	/* "\033[B" */
		case XK_Right:	/* "\033[C" */
		case XK_Left:	/* "\033[D" */
		    STRCPY(kbuf, "\033[Z");
		    kbuf[2] = ("DACB"[keysym - XK_Left]);
		/* do Shift first */
		    if (shft)
			kbuf[2] = ("dacb"[keysym - XK_Left]);
		    else if (ctrl) {
			kbuf[1] = 'O';
			kbuf[2] = ("dacb"[keysym - XK_Left]);
		    } else if (r->h->PrivateModes & PrivMode_aplCUR)
			kbuf[1] = 'O';
#ifdef MULTICHAR_SET
		    if (r->Options & Opt_mc_hack) {
			int             col, row, m;

			col = r->screen.cur.col;
			row = r->screen.cur.row + r->TermWin.saveLines;
			m = 0;
			if (keysym == XK_Right
			    && IS_MULTI1(r->screen.rend[row][col]))
			    m = 1;
			else if (keysym == XK_Left) {
			    if (col > 0) {
				if (IS_MULTI2(r->screen.rend[row][col - 1]))
				    m = 1;
			    } else if (r->screen.cur.row > 0) {
				col = r->screen.tlen[--row];
				if (col == -1)
				    col = r->TermWin.ncol - 1;
				else
				    col--;
				if (col > 0
				    && IS_MULTI2(r->screen.rend[row][col]))
				    m = 1;
			    }
			}
			if (m)
			    MEMMOVE(kbuf + 3, kbuf, 3 + 1);
		    }
#endif
		    break;

#ifndef UNSHIFTED_SCROLLKEYS
# ifdef XK_KP_Prior
		case XK_KP_Prior:
		/* allow shift to override */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Oy");
			break;
		    }
		/* FALLTHROUGH */
# endif
		case XK_Prior:
		    STRCPY(kbuf, "\033[5~");
		    break;
# ifdef XK_KP_Next
		case XK_KP_Next:
		/* allow shift to override */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Os");
			break;
		    }
		/* FALLTHROUGH */
# endif
		case XK_Next:
		    STRCPY(kbuf, "\033[6~");
		    break;
#endif
		case XK_KP_Enter:
		/* allow shift to override */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033OM");
		    } else {
			kbuf[0] = '\r';
			kbuf[1] = '\0';
		    }
		    break;

#ifdef XK_KP_Begin
		case XK_KP_Begin:
		    STRCPY(kbuf, "\033Ou");
		    break;

		case XK_KP_Insert:
		    STRCPY(kbuf, "\033Op");
		    break;

		case XK_KP_Delete:
		    STRCPY(kbuf, "\033On");
		    break;
#endif
		case XK_KP_F1:	/* "\033OP" */
		case XK_KP_F2:	/* "\033OQ" */
		case XK_KP_F3:	/* "\033OR" */
		case XK_KP_F4:	/* "\033OS" */
		    STRCPY(kbuf, "\033OP");
		    kbuf[2] += (keysym - XK_KP_F1);
		    break;

		case XK_KP_Multiply:	/* "\033Oj" : "*" */
		case XK_KP_Add:		/* "\033Ok" : "+" */
		case XK_KP_Separator:	/* "\033Ol" : "," */
		case XK_KP_Subtract:	/* "\033Om" : "-" */
		case XK_KP_Decimal:	/* "\033On" : "." */
		case XK_KP_Divide:	/* "\033Oo" : "/" */
		case XK_KP_0:		/* "\033Op" : "0" */
		case XK_KP_1:		/* "\033Oq" : "1" */
		case XK_KP_2:		/* "\033Or" : "2" */
		case XK_KP_3:		/* "\033Os" : "3" */
		case XK_KP_4:		/* "\033Ot" : "4" */
		case XK_KP_5:		/* "\033Ou" : "5" */
		case XK_KP_6:		/* "\033Ov" : "6" */
		case XK_KP_7:		/* "\033Ow" : "7" */
		case XK_KP_8:		/* "\033Ox" : "8" */
		case XK_KP_9:		/* "\033Oy" : "9" */
		/* allow shift to override */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Oj");
			kbuf[2] += (keysym - XK_KP_Multiply);
		    } else {
			kbuf[0] = ('*' + (keysym - XK_KP_Multiply));
			kbuf[1] = '\0';
		    }
		    break;

		case XK_Find:
		    STRCPY(kbuf, "\033[1~");
		    break;
		case XK_Insert:
		    STRCPY(kbuf, "\033[2~");
		    break;
#ifdef DXK_Remove		/* support for DEC remove like key */
		case DXK_Remove:
		/* FALLTHROUGH */
#endif
		case XK_Execute:
		    STRCPY(kbuf, "\033[3~");
		    break;
		case XK_Select:
		    STRCPY(kbuf, "\033[4~");
		    break;
#ifdef XK_KP_End
		case XK_KP_End:
		/* allow shift to override */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Oq");
			break;
		    }
		/* FALLTHROUGH */
#endif
		case XK_End:
		    STRCPY(kbuf, KS_END);
		    break;
#ifdef XK_KP_Home
		case XK_KP_Home:
		/* allow shift to override */
		    if ((r->h->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Ow");
			break;
		    }
		/* FALLTHROUGH */
#endif
		case XK_Home:
		    STRCPY(kbuf, KS_HOME);
		    break;

#define FKEY(n, fkey)							\
    sprintf((char *)kbuf,"\033[%2d~", (int)((n) + (keysym - fkey)))

		case XK_F1:	/* "\033[11~" */
		case XK_F2:	/* "\033[12~" */
		case XK_F3:	/* "\033[13~" */
		case XK_F4:	/* "\033[14~" */
		case XK_F5:	/* "\033[15~" */
		    FKEY(11, XK_F1);
		    break;
		case XK_F6:	/* "\033[17~" */
		case XK_F7:	/* "\033[18~" */
		case XK_F8:	/* "\033[19~" */
		case XK_F9:	/* "\033[20~" */
		case XK_F10:	/* "\033[21~" */
		    FKEY(17, XK_F6);
		    break;
		case XK_F11:	/* "\033[23~" */
		case XK_F12:	/* "\033[24~" */
		case XK_F13:	/* "\033[25~" */
		case XK_F14:	/* "\033[26~" */
		    FKEY(23, XK_F11);
		    break;
		case XK_F15:	/* "\033[28~" */
		case XK_F16:	/* "\033[29~" */
		    FKEY(28, XK_F15);
		    break;
		case XK_Help:	/* "\033[28~" */
		    FKEY(28, XK_Help);
		    break;
		case XK_Menu:	/* "\033[29~" */
		    FKEY(29, XK_Menu);
		    break;
		case XK_F17:	/* "\033[31~" */
		case XK_F18:	/* "\033[32~" */
		case XK_F19:	/* "\033[33~" */
		case XK_F20:	/* "\033[34~" */
		case XK_F21:	/* "\033[35~" */
		case XK_F22:	/* "\033[36~" */
		case XK_F23:	/* "\033[37~" */
		case XK_F24:	/* "\033[38~" */
		case XK_F25:	/* "\033[39~" */
		case XK_F26:	/* "\033[40~" */
		case XK_F27:	/* "\033[41~" */
		case XK_F28:	/* "\033[42~" */
		case XK_F29:	/* "\033[43~" */
		case XK_F30:	/* "\033[44~" */
		case XK_F31:	/* "\033[45~" */
		case XK_F32:	/* "\033[46~" */
		case XK_F33:	/* "\033[47~" */
		case XK_F34:	/* "\033[48~" */
		case XK_F35:	/* "\033[49~" */
		    FKEY(31, XK_F17);
		    break;
#undef FKEY
		default:
		    newlen = 0;
		    break;
		}
		if (newlen)
		    len = STRLEN(kbuf);
	    }
	/*
	 * Pass meta for all function keys, if 'meta' option set
	 */
#ifdef META8_OPTION
	    if (meta && (r->h->meta_char == 0x80) && len > 0)
		kbuf[len - 1] |= 0x80;
#endif
	} else if (ctrl && keysym == XK_minus) {
	    len = 1;
	    kbuf[0] = '\037';	/* Ctrl-Minus generates ^_ (31) */
	} else {
#ifdef META8_OPTION
	/* set 8-bit on */
	    if (meta && (r->h->meta_char == 0x80)) {
		unsigned char  *ch;

		for (ch = kbuf; ch < kbuf + len; ch++)
		    *ch |= 0x80;
		meta = 0;
	    }
#endif
#ifdef GREEK_SUPPORT
	    if (r->h->greek_mode)
		len = greek_xlat(kbuf, len);
#endif
	/* nil */ ;
	}
    }

    if (len <= 0)
	return;			/* not mapped */

    if (r->Options & Opt_scrollTtyKeypress)
	if (r->TermWin.view_start) {
	    r->TermWin.view_start = 0;
	    r->h->want_refresh = 1;
	}

/*
 * these modifications only affect the static keybuffer
 * pass Shift/Control indicators for function keys ending with `~'
 *
 * eg,
 *   Prior = "ESC[5~"
 *   Shift+Prior = "ESC[5$"
 *   Ctrl+Prior = "ESC[5^"
 *   Ctrl+Shift+Prior = "ESC[5@"
 * Meta adds an Escape prefix (with META8_OPTION, if meta == <escape>).
 */
    if (kbuf[0] == C0_ESC && kbuf[1] == '[' && kbuf[len - 1] == '~')
	kbuf[len - 1] = (shft ? (ctrl ? '@' : '$') : (ctrl ? '^' : '~'));

/* escape prefix */
    if (meta
#ifdef META8_OPTION
	&& (r->h->meta_char == C0_ESC)
#endif
	) {
	const unsigned char ch = C0_ESC;

	rxvt_tt_write(r, &ch, 1);
    }
#ifdef DEBUG_CMD
    if (debug_key) {		/* Display keyboard buffer contents */
	char           *p;
	int             i;

	fprintf(stderr, "key 0x%04X [%d]: `", (unsigned int)keysym, len);
	for (i = 0, p = kbuf; i < len; i++, p++)
	    fprintf(stderr, (*p >= ' ' && *p < '\177' ? "%c" : "\\%03o"), *p);
	fprintf(stderr, "'\n");
    }
#endif				/* DEBUG_CMD */
    rxvt_tt_write(r, kbuf, (unsigned int)len);
}
/*}}} */

#if (MENUBAR_MAX)
/*{{{ rxvt_cmd_write(), rxvt_cmd_getc() */
/* attempt to `write' count to the input buffer */
/* EXTPROTO */
unsigned int
rxvt_cmd_write(rxvt_t *r, const unsigned char *str, unsigned int count)
{
    unsigned int    n, s;
    unsigned char  *cmdbuf_base = r->h->cmdbuf_base,
                   *cmdbuf_endp = r->h->cmdbuf_endp,
                   *cmdbuf_ptr = r->h->cmdbuf_ptr;

    n = cmdbuf_ptr - cmdbuf_base;
    s = cmdbuf_base + BUFSIZ - 1 - cmdbuf_endp;
    if (n > 0 && s < count) {
	MEMMOVE(cmdbuf_base, cmdbuf_ptr,
		(unsigned int)(cmdbuf_endp - cmdbuf_ptr));
	cmdbuf_ptr = cmdbuf_base;
	cmdbuf_endp -= n;
	s += n;
    }
    if (count > s) {
	rxvt_print_error("data loss: cmd_write too large");
	count = s;
    }
    for (; count--;)
	*cmdbuf_endp++ = *str++;
    r->h->cmdbuf_ptr = cmdbuf_ptr;
    r->h->cmdbuf_endp = cmdbuf_endp;
    return 0;
}
#endif				/* MENUBAR_MAX */

/* rxvt_cmd_getc() - Return next input character */
/*
 * Return the next input character after first passing any keyboard input
 * to the command.
 */
/* INTPROTO */
unsigned char
rxvt_cmd_getc(rxvt_t *r)
{
#define TIMEOUT_USEC	5000
    fd_set          readfds;
    int             quick_timeout, select_res;
    struct timeval  value;
    struct rxvt_hidden *h = r->h;

    if (h->cmdbuf_ptr < h->cmdbuf_endp)	/* characters already read in */
        return *h->cmdbuf_ptr++;

    for (;;) {
    /* loop until we can return something */

	if (h->v_bufstr < h->v_bufptr)	/* output any pending chars */
	    rxvt_tt_write(r, NULL, 0);

	while (XPending(r->Xdisplay)) {	/* process pending X events */
	    XEvent          xev;

	    XNextEvent(r->Xdisplay, &xev);
#ifdef USE_XIM
	    if (!XFilterEvent(&xev, xev.xany.window))
		rxvt_process_x_event(r, &xev);
	    h->event_type = xev.type;
#else
	    rxvt_process_x_event(r, &xev);
#endif
	/* in case button actions pushed chars to cmdbuf */
	    if (h->cmdbuf_ptr < h->cmdbuf_endp)
		return *h->cmdbuf_ptr++;
	}

/*
 * the command input buffer is empty and we have no pending X events
 */
	quick_timeout = 0;

#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
	if (h->mouse_slip_wheel_speed) {
	    quick_timeout = 1;
	    if (!h->mouse_slip_wheel_delay--
		&& rxvt_scr_page(r, h->mouse_slip_wheel_speed > 0 ? UP : DN,
				 abs(h->mouse_slip_wheel_speed))) {
		h->mouse_slip_wheel_delay = SCROLLBAR_CONTINUOUS_DELAY;
		h->refresh_type |= SMOOTH_REFRESH;
		h->want_refresh = 1;
	    }
	}
#endif /* MOUSE_WHEEL && MOUSE_SLIP_WHEELING */
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	if (scrollbar_isUp() || scrollbar_isDn()) {
	    quick_timeout = 1;
	    if (!h->scroll_arrow_delay--
		&& rxvt_scr_page(r, scrollbar_isUp() ? UP : DN, 1)) {
		h->scroll_arrow_delay = SCROLLBAR_CONTINUOUS_DELAY;
		h->refresh_type |= SMOOTH_REFRESH;
		h->want_refresh = 1;
	    }
	}
#endif				/* NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING */

	FD_ZERO(&readfds);
	FD_SET(r->cmd_fd, &readfds);
	FD_SET(r->Xfd, &readfds);
	value.tv_usec = TIMEOUT_USEC;
	value.tv_sec = 0;

	if (!r->TermWin.mapped)
	    quick_timeout = 0;
	else {
	    quick_timeout |= h->want_refresh;
#ifdef TRANSPARENT
	    quick_timeout |= h->want_full_refresh;
#endif
	}
	if ((select_res = select(r->num_fds, &readfds, NULL, NULL,
				 (quick_timeout ? &value : NULL))) == 0) {
	/* select statement timed out - we're not hard and fast scrolling */
	    h->refresh_limit = 1;
	}

    /* See if we can read new data from the application */
	if (select_res > 0 && FD_ISSET(r->cmd_fd, &readfds)) {
	    int             n;
	    unsigned int    count;

	    h->cmdbuf_ptr = h->cmdbuf_endp = h->cmdbuf_base;
	    for (count = BUFSIZ; count; count -= n, h->cmdbuf_endp += n)
		if ((n = read(r->cmd_fd, h->cmdbuf_endp, count)) > 0)
		    continue;
		else if (n == 0 || (n < 0 && errno == EAGAIN))
		    break;
		else {
		    rxvt_clean_exit();
		    exit(EXIT_FAILURE);	/* bad order of events? */
		}
	    if (count != BUFSIZ)	/* some characters read in */
		return *h->cmdbuf_ptr++;
	}
#ifdef TRANSPARENT
	if (h->want_full_refresh) {
	    h->want_full_refresh = 0;
	    rxvt_scr_clear(r);
	    rxvt_scr_touch(r, False);
	    h->want_refresh = 1;
	}
#endif
	if (h->want_refresh) {
	    rxvt_scr_refresh(r, h->refresh_type);
	    rxvt_scrollbar_show(r, 1);
#ifdef USE_XIM
	    rxvt_IMSendSpot(r);
#endif
	}
    }
/* NOTREACHED */
}
/*}}} */

/* INTPROTO */
void
rxvt_mouse_report(rxvt_t *r, const XButtonEvent *ev)
{
    int             button_number, key_state = 0;
    int             x, y;

    x = ev->x;
    y = ev->y;
    rxvt_pixel_position(r, &x, &y);

    if (r->h->MEvent.button == AnyButton) {
	button_number = 3;
    } else {
	button_number = r->h->MEvent.button - Button1;
	/* add 0x3D for wheel events, like xterm does */
	if (button_number >= 3)
	    button_number += (64 - 3);
    }

    if (r->h->PrivateModes & PrivMode_MouseX10) {
    /*
     * do not report ButtonRelease
     * no state info allowed
     */
	key_state = 0;
	if (button_number == 3)
	    return;
    } else {
    /* XTerm mouse reporting needs these values:
     *   4 = Shift
     *   8 = Meta
     *  16 = Control
     * plus will add in our own Double-Click reporting
     *  32 = Double Click
     */
	key_state = ((r->h->MEvent.state & ShiftMask) ? 4 : 0)
		     + ((r->h->MEvent.state & r->h->ModMetaMask) ? 8 : 0)
		     + ((r->h->MEvent.state & ControlMask) ? 16 : 0);
#ifdef MOUSE_REPORT_DOUBLECLICK
	key_state += ((r->h->MEvent.clicks > 1) ? 32 : 0);
#endif
    }

#ifdef DEBUG_MOUSEREPORT
    fprintf(stderr, "Mouse [");
    if (key_state & 16)
	fputc('C', stderr);
    if (key_state & 4)
	fputc('S', stderr);
    if (key_state & 8)
	fputc('A', stderr);
    if (key_state & 32)
	fputc('2', stderr);
    fprintf(stderr, "]: <%d>, %d/%d\n",
	    button_number,
	    x + 1,
	    y + 1);
#else
    rxvt_tt_printf(r, "\033[M%c%c%c",
	      (32 + button_number + key_state),
	      (32 + x + 1),
	      (32 + y + 1));
#endif
}

#ifdef USING_W11LIB
/* EXTPROTO */
void
rxvt_W11_process_x_event(XEvent *ev)
{
    rxvt_t         *r = rxvt_get_r();

    rxvt_process_x_event(r, ev);
}
#endif

/*{{{ process an X event */
/* INTPROTO */
void
rxvt_process_x_event(rxvt_t *r, XEvent *ev)
{
    int             i, want_time = 0;
    Window          unused_root, unused_child;
    int             unused_root_x, unused_root_y;
    unsigned int    unused_mask;
    struct timeval  tp;
    struct rxvt_hidden *h = r->h;
#ifdef DEBUG_X
    const char *const eventnames[] =
    {				/* mason - this matches my system */
	"",
	"",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify"
    };
    struct tm      *ltt;
#endif

#ifdef DEBUG_X
    want_time = 1;
#else
    /*
     * check if we need to get the time for any timeouts
     */

    for (i = NUM_TIMEOUTS; i--; )
	if (h->timeout[i].tv_sec) {
	    want_time = 1;
	    break;
	}
#endif

    if (want_time)
	(void)gettimeofday(&tp, NULL);

#ifdef DEBUG_X
    ltt = localtime(&(tp.tv_sec));
    D_X((stderr, "Event: %-16s %-7s %08lx (%4d-%02d-%02d %02d:%02d:%02d.%.6ld) %s %lu", eventnames[ev->type], (ev->xany.window == r->TermWin.parent[0] ? "parent" : (ev->xany.window == r->TermWin.vt ? "vt" : (ev->xany.window == r->scrollBar.win ? "scroll" : (ev->xany.window == r->menuBar.win ? "menubar" : "UNKNOWN")))), (ev->xany.window == r->TermWin.parent[0] ? r->TermWin.parent[0] : (ev->xany.window == r->TermWin.vt ? r->TermWin.vt : (ev->xany.window == r->scrollBar.win ? r->scrollBar.win : (ev->xany.window == r->menuBar.win ? r->menuBar.win : 0)))), ltt->tm_year + 1900, ltt->tm_mon + 1, ltt->tm_mday, ltt->tm_hour, ltt->tm_min, ltt->tm_sec, tp.tv_usec, ev->xany.send_event ? "S" : " ", ev->xany.serial));
#endif

    /* X event timeouts */
    if (want_time)
	for (i = NUM_TIMEOUTS; i--; ) {
	    if (h->timeout[i].tv_sec == 0)
		continue;
	    if ((tp.tv_sec < h->timeout[i].tv_sec)
		|| (tp.tv_sec == h->timeout[i].tv_sec
		    && tp.tv_usec < h->timeout[i].tv_usec))
		continue;
	    h->timeout[i].tv_sec = 0;
	    switch(i) {
	    case TIMEOUT_INCR:
		rxvt_print_error("data loss: timeout on INCR selection paste");
		h->selection_wait = Sel_none;
		break;
	    default:
		break;
	    }
	}

    switch (ev->type) {
    case KeyPress:
	rxvt_lookup_key(r, (XKeyEvent *)ev);
	break;

#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
    case KeyRelease:
	{
	    if (!(ev->xkey.state & ControlMask))
		h->mouse_slip_wheel_speed = 0;
	    else {
		KeySym          ks;
		
		ks = XKeycodeToKeysym(r->Xdisplay, ev->xkey.keycode, 0);
		if (ks == XK_Control_L || ks == XK_Control_R)
		    h->mouse_slip_wheel_speed = 0;
	    }
	    break;
	}
#endif

    case ButtonPress:
	rxvt_button_press(r, (XButtonEvent *)ev);
	break;

    case ButtonRelease:
	rxvt_button_release(r, (XButtonEvent *)ev);
	break;

    case ClientMessage:
	if (ev->xclient.format == 32
	    && (Atom)ev->xclient.data.l[0] == h->xa[XA_WMDELETEWINDOW])
	    exit(EXIT_SUCCESS);
#ifdef OFFIX_DND
    /* OffiX Dnd (drag 'n' drop) protocol */
	if (ev->xclient.message_type == h->xa[XA_DNDPROTOCOL]
	    && (ev->xclient.data.l[0] == DndFile
		|| ev->xclient.data.l[0] == DndDir
		|| ev->xclient.data.l[0] == DndLink)) {
	/* Get Dnd data */
	    Atom            ActualType;
	    int             ActualFormat;
	    unsigned char  *data;
	    unsigned long   Size, RemainingBytes;

	    XGetWindowProperty(r->Xdisplay, Xroot,
			       r->h->xa[XA_DNDSELECTION],
			       0L, 1000000L,
			       False, AnyPropertyType,
			       &ActualType, &ActualFormat,
			       &Size, &RemainingBytes,
			       &data);
	    XChangeProperty(r->Xdisplay, Xroot,
			    XA_CUT_BUFFER0, XA_STRING,
			    8, PropModeReplace,
			    data, STRLEN(data));
	    rxvt_selection_paste(r, Xroot, XA_CUT_BUFFER0, True);
	    XSetInputFocus(r->Xdisplay, Xroot, RevertToNone, CurrentTime);
	}
#endif				/* OFFIX_DND */
	break;

    case MappingNotify:
	XRefreshKeyboardMapping(&(ev->xmapping));
	break;

    /*
     * XXX: this is not the _current_ arrangement
     * Here's my conclusion:
     * If the window is completely unobscured, use bitblt's
     * to scroll. Even then, they're only used when doing partial
     * screen scrolling. When partially obscured, we have to fill
     * in the GraphicsExpose parts, which means that after each refresh,
     * we need to wait for the graphics expose or Noexpose events,
     * which ought to make things real slow!
     */
    case VisibilityNotify:
	switch (ev->xvisibility.state) {
	case VisibilityUnobscured:
	    h->refresh_type = FAST_REFRESH;
	    break;
	case VisibilityPartiallyObscured:
	    h->refresh_type = SLOW_REFRESH;
	    break;
	default:
	    h->refresh_type = NO_REFRESH;
	    break;
	}
	break;

    case FocusIn:
	if (!r->TermWin.focus) {
	    r->TermWin.focus = 1;
	    h->want_refresh = 1;
#ifdef USE_XIM
	    if (h->Input_Context != NULL)
		XSetICFocus(h->Input_Context);
#endif
	}
	break;

    case FocusOut:
	if (r->TermWin.focus) {
	    r->TermWin.focus = 0;
	    h->want_refresh = 1;
#ifdef USE_XIM
	    if (h->Input_Context != NULL)
		XUnsetICFocus(h->Input_Context);
#endif
	}
	break;

    case ConfigureNotify:
	if (ev->xconfigure.window == r->TermWin.parent[0]) {
	    int             height, width;

	    do {	/* Wrap lots of configures into one */
		width = ev->xconfigure.width;
		height = ev->xconfigure.height;
	    } while (XCheckTypedWindowEvent(r->Xdisplay, ev->xconfigure.window,
					    ConfigureNotify, ev));
	    if (r->szHint.width != width || r->szHint.height != height)
		rxvt_resize_all_windows(r, (unsigned int)width,
					(unsigned int)height, 1);
#ifdef TRANSPARENT		/* XXX: maybe not needed - leave in for now */
	    if (r->Options & Opt_transparent) {
		rxvt_check_our_parents(r);
		if (h->am_transparent)
		    h->want_full_refresh = 1;
	    }
#endif
	}
	break;

    case SelectionClear:
	rxvt_selection_clear(r);
	break;

    case SelectionNotify:
	if (h->selection_wait == Sel_normal)
	    rxvt_selection_paste(r, ev->xselection.requestor,
				 ev->xselection.property, True);
	break;

    case SelectionRequest:
	rxvt_selection_send(r, &(ev->xselectionrequest));
	break;

    case UnmapNotify:
	r->TermWin.mapped = 0;
	break;

    case MapNotify:
	r->TermWin.mapped = 1;
	break;

    case PropertyNotify:
	if (ev->xproperty.atom == h->xa[XA_VT_SELECTION]) {
	    if (ev->xproperty.state == PropertyNewValue)
		rxvt_selection_property(r, ev->xproperty.window,
					ev->xproperty.atom);
	    break;
	}
#ifdef TRANSPARENT
    /*
     * if user used some Esetroot compatible prog to set the root bg,
     * use the property to determine the pixmap.  We use it later on.
     */
	if (h->xa[XA_XROOTPMAPID] == 0)
	    h->xa[XA_XROOTPMAPID] = XInternAtom(r->Xdisplay,
					       "_XROOTPMAP_ID", False);
	if (ev->xproperty.atom != h->xa[XA_XROOTPMAPID])
	    break;
    /* FALLTHROUGH */
    case ReparentNotify:
	if ((r->Options & Opt_transparent) && rxvt_check_our_parents(r)) {
	    if (h->am_transparent)
		h->want_full_refresh = 1;
	}
#endif				/* TRANSPARENT */
	break;

    case GraphicsExpose:
    case Expose:
	if (ev->xany.window == r->TermWin.vt) {
#ifdef NO_SLOW_LINK_SUPPORT
	    rxvt_scr_expose(r, ev->xexpose.x, ev->xexpose.y,
			    ev->xexpose.width, ev->xexpose.height, False);
#else
	    rxvt_scr_expose(r, ev->xexpose.x, 0,
			    ev->xexpose.width, r->TermWin.height, False);
#endif
	    h->want_refresh = 1;
	} else {
	    XEvent          unused_xevent;

	    while (XCheckTypedWindowEvent(r->Xdisplay, ev->xany.window,
					  Expose,
					  &unused_xevent)) ;
	    while (XCheckTypedWindowEvent(r->Xdisplay, ev->xany.window,
					  GraphicsExpose,
					  &unused_xevent)) ;
	    if (isScrollbarWindow(ev->xany.window)) {
		scrollbar_setIdle();
		rxvt_scrollbar_show(r, 0);
	    }
	    if (menubar_visible(r) && isMenuBarWindow(ev->xany.window))
		rxvt_menubar_expose(r);
	    rxvt_Gr_expose(r, ev->xany.window);
	}
	break;

    case MotionNotify:
	if (isMenuBarWindow(ev->xany.window)) {
	    rxvt_menubar_control(r, &(ev->xbutton));
	    break;
	}
	if ((h->PrivateModes & PrivMode_mouse_report) && !(h->bypass_keystate))
	    break;

	if (ev->xany.window == r->TermWin.vt) {
	    if ((ev->xbutton.state & (Button1Mask | Button3Mask))) {
		while (XCheckTypedWindowEvent(r->Xdisplay, r->TermWin.vt,
					      MotionNotify, ev)) ;
		XQueryPointer(r->Xdisplay, r->TermWin.vt,
			      &unused_root, &unused_child,
			      &unused_root_x, &unused_root_y,
			      &(ev->xbutton.x), &(ev->xbutton.y),
			      &unused_mask);
#ifdef MOUSE_THRESHOLD
	    /* deal with a `jumpy' mouse */
		if ((ev->xmotion.time - h->MEvent.time) > MOUSE_THRESHOLD)
#endif
		    rxvt_selection_extend(r, (ev->xbutton.x), (ev->xbutton.y),
				  (ev->xbutton.state & Button3Mask) ? 2 : 0);
	    }
	} else if (isScrollbarWindow(ev->xany.window) && scrollbar_isMotion()) {
	    while (XCheckTypedWindowEvent(r->Xdisplay, r->scrollBar.win,
					  MotionNotify, ev)) ;
	    XQueryPointer(r->Xdisplay, r->scrollBar.win,
			  &unused_root, &unused_child,
			  &unused_root_x, &unused_root_y,
			  &(ev->xbutton.x), &(ev->xbutton.y),
			  &unused_mask);
	    rxvt_scr_move_to(r, scrollbar_position(ev->xbutton.y) - h->csrO,
			     scrollbar_size());
	    rxvt_scr_refresh(r, h->refresh_type);
	    h->refresh_limit = 0;
	    rxvt_scrollbar_show(r, 1);
	}
	break;
    }
}

/* INTPROTO */
void
rxvt_button_press(rxvt_t *r, XButtonEvent *ev)
{
    int             reportmode = 0, clickintime;
    struct rxvt_hidden *h = r->h;

    h->bypass_keystate = ev->state & (h->ModMetaMask | ShiftMask);
    if (!h->bypass_keystate)
	reportmode = !!(h->PrivateModes & PrivMode_mouse_report);
/*
 * VT window processing of button press
 */
    if (ev->window == r->TermWin.vt) {
	if (ev->subwindow != None)
	    rxvt_Gr_ButtonPress(ev->x, ev->y);
	else {
	    clickintime = ev->time - h->MEvent.time < MULTICLICK_TIME;
	    if (reportmode) {
		/* mouse report from vt window */
		/* save the xbutton state (for ButtonRelease) */
		h->MEvent.state = ev->state;
#ifdef MOUSE_REPORT_DOUBLECLICK
		if (ev->button == h->MEvent.button && clickintime) {
		    /* same button, within alloted time */
		    h->MEvent.clicks++;
		    if (h->MEvent.clicks > 1) {
			/* only report double clicks */
			h->MEvent.clicks = 2;
			rxvt_mouse_report(r, ev);

			/* don't report the release */
			h->MEvent.clicks = 0;
			h->MEvent.button = AnyButton;
		    }
		} else {
		    /* different button, or time expired */
		    h->MEvent.clicks = 1;
		    h->MEvent.button = ev->button;
		    rxvt_mouse_report(r, ev);
		}
#else
		h->MEvent.button = ev->button;
		rxvt_mouse_report(r, ev);
#endif				/* MOUSE_REPORT_DOUBLECLICK */
	    } else {
		if (ev->button != h->MEvent.button)
		    h->MEvent.clicks = 0;
		switch (ev->button) {
		case Button1:
		    if (h->MEvent.button == Button1 && clickintime)
			h->MEvent.clicks++;
		    else
			h->MEvent.clicks = 1;
		    rxvt_selection_click(r, h->MEvent.clicks, ev->x, ev->y);
		    h->MEvent.button = Button1;
		    break;

		case Button3:
		    if (h->MEvent.button == Button3 && clickintime)
			rxvt_selection_rotate(r, ev->x, ev->y);
		    else
			rxvt_selection_extend(r, ev->x, ev->y, 1);
		    h->MEvent.button = Button3;
		    break;
		}
	    }
	    h->MEvent.time = ev->time;
	    return;
	}
    }

/*
 * Scrollbar window processing of button press
 */
    if (isScrollbarWindow(ev->window)) {
	scrollbar_setIdle();
	/*
	 * Rxvt-style scrollbar:
	 * move up if mouse is above slider
	 * move dn if mouse is below slider
	 *
	 * XTerm-style scrollbar:
	 * Move display proportional to pointer location
	 * pointer near top -> scroll one line
	 * pointer near bot -> scroll full page
	 */
#ifndef NO_SCROLLBAR_REPORT
	if (reportmode) {
	    /*
	     * Mouse report disabled scrollbar:
	     * arrow buttons - send up/down
	     * click on scrollbar - send pageup/down
	     */
	    if ((r->scrollBar.style == R_SB_NEXT
		 && scrollbarnext_upButton(ev->y))
		|| (r->scrollBar.style == R_SB_RXVT
		    && scrollbarrxvt_upButton(ev->y)))
		rxvt_tt_printf(r, "\033[A");
	    else if ((r->scrollBar.style == R_SB_NEXT
		      && scrollbarnext_dnButton(ev->y))
		     || (r->scrollBar.style == R_SB_RXVT
			 && scrollbarrxvt_dnButton(ev->y)))
		rxvt_tt_printf(r, "\033[B");
	    else
		switch (ev->button) {
		case Button2:
		    rxvt_tt_printf(r, "\014");
		    break;
		case Button1:
		    rxvt_tt_printf(r, "\033[6~");
		    break;
		case Button3:
		    rxvt_tt_printf(r, "\033[5~");
		    break;
		}
	} else
#endif				/* NO_SCROLLBAR_REPORT */
	{
	    char            upordown = 0;

	    if (r->scrollBar.style == R_SB_NEXT) {
		if (scrollbarnext_upButton(ev->y))
		    upordown = -1;	/* up */
		else if (scrollbarnext_dnButton(ev->y))
		    upordown = 1;	/* down */
	    } else if (r->scrollBar.style == R_SB_RXVT) {
		if (scrollbarrxvt_upButton(ev->y))
		    upordown = -1;	/* up */
		else if (scrollbarrxvt_dnButton(ev->y))
		    upordown = 1;	/* down */
	    }
	    if (upordown) { 
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
		h->scroll_arrow_delay = SCROLLBAR_INITIAL_DELAY;
#endif
		if (rxvt_scr_page(r, upordown < 0 ? UP : DN, 1)) {
		    if (upordown < 0)
			scrollbar_setUp();
		    else
			scrollbar_setDn();
		}
	    } else
		switch (ev->button) {
		case Button2:
		    switch (h->scrollbar_align) {
		    case R_SB_ALIGN_TOP:
			h->csrO = 0;
			break;
		    case R_SB_ALIGN_CENTRE:
			h->csrO = (r->scrollBar.bot - r->scrollBar.top) / 2;
			break;
		    case R_SB_ALIGN_BOTTOM:
			h->csrO = r->scrollBar.bot - r->scrollBar.top;
			break;
		    }
		    if (r->scrollBar.style == R_SB_XTERM
			|| scrollbar_above_slider(ev->y)
			|| scrollbar_below_slider(ev->y))
			rxvt_scr_move_to(r,
					 scrollbar_position(ev->y) - h->csrO,
					 scrollbar_size());
		    scrollbar_setMotion();
		    break;

		case Button1:
		    if (h->scrollbar_align == R_SB_ALIGN_CENTRE)
			h->csrO = ev->y - r->scrollBar.top;
		    /* FALLTHROUGH */

		case Button3:
		    if (r->scrollBar.style != R_SB_XTERM) {
			if (scrollbar_above_slider(ev->y))
# ifdef RXVT_SCROLL_FULL
			    rxvt_scr_page(r, UP, r->TermWin.nrow - 1);
# else
			    rxvt_scr_page(r, UP, r->TermWin.nrow / 4);
# endif
			else if (scrollbar_below_slider(ev->y))
# ifdef RXVT_SCROLL_FULL
			    rxvt_scr_page(r, DN, r->TermWin.nrow - 1);
# else
			    rxvt_scr_page(r, DN, r->TermWin.nrow / 4);
# endif
			else
			    scrollbar_setMotion();
		    } else {
			rxvt_scr_page(r, (ev->button == Button1 ? DN : UP),
				      (r->TermWin.nrow
				       * scrollbar_position(ev->y)
				       / scrollbar_size()));
		    }
		    break;
		}
	}
	return;
    }
/*
 * Menubar window processing of button press
 */
    if (isMenuBarWindow(ev->window))
	rxvt_menubar_control(r, ev);
}

/* INTPROTO */
void
rxvt_button_release(rxvt_t *r, XButtonEvent *ev)
{
    int             reportmode = 0;

    r->h->csrO = 0;		/* reset csr Offset */
    if (!r->h->bypass_keystate)
	reportmode = !!(r->h->PrivateModes & PrivMode_mouse_report);

    if (scrollbar_isUpDn()) {
	scrollbar_setIdle();
	rxvt_scrollbar_show(r, 0);
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	r->h->refresh_type &= ~SMOOTH_REFRESH;
#endif
    }
    if (ev->window == r->TermWin.vt) {
	if (ev->subwindow != None)
	    rxvt_Gr_ButtonRelease(ev->x, ev->y);
	else {
	    if (reportmode) {
		/* mouse report from vt window */
		/* don't report release of wheel "buttons" */
		if (ev->button >= 4)
		    return;
#ifdef MOUSE_REPORT_DOUBLECLICK
		/* only report the release of 'slow' single clicks */
		if (r->h->MEvent.button != AnyButton
		    && (ev->button != r->h->MEvent.button
			|| (ev->time - r->h->MEvent.time
			    > MULTICLICK_TIME / 2))) {
		    r->h->MEvent.clicks = 0;
		    r->h->MEvent.button = AnyButton;
		    rxvt_mouse_report(r, ev);
		}
#else				/* MOUSE_REPORT_DOUBLECLICK */
		r->h->MEvent.button = AnyButton;
		rxvt_mouse_report(r, ev);
#endif				/* MOUSE_REPORT_DOUBLECLICK */
		return;
	    }
	    /*
	     * dumb hack to compensate for the failure of click-and-drag
	     * when overriding mouse reporting
	     */
	    if (r->h->PrivateModes & PrivMode_mouse_report
		&& r->h->bypass_keystate
		&& ev->button == Button1 && r->h->MEvent.clicks <= 1)
		rxvt_selection_extend(r, ev->x, ev->y, 0);

	    switch (ev->button) {
	    case Button1:
	    case Button3:
		rxvt_selection_make(r, ev->time);
		break;
	    case Button2:
		rxvt_selection_request(r, ev->time, ev->x, ev->y);
		break;
#ifdef MOUSE_WHEEL
	    case Button4:
	    case Button5:
		{
		    int             i, v;

		    v = (ev->button == Button4) ? UP : DN;
		    if (ev->state & ShiftMask)
			i = 1;
		    else if ((r->Options & Opt_mouseWheelScrollPage))
			i = r->TermWin.nrow - 1;
		    else
			i = 5;
# ifdef MOUSE_SLIP_WHEELING
                    if (ev->state & ControlMask) {
			r->h->mouse_slip_wheel_speed += (v ? -1 : 1);
			r->h->mouse_slip_wheel_delay = SCROLLBAR_CONTINUOUS_DELAY;
		    }
# endif
# ifdef JUMP_MOUSE_WHEEL
		    rxvt_scr_page(r, v, i);
		    rxvt_scr_refresh(r, SMOOTH_REFRESH);
		    rxvt_scrollbar_show(r, 1);
# else
		    for (; i--;) {
			rxvt_scr_page(r, v, 1);
			rxvt_scr_refresh(r, SMOOTH_REFRESH);
			rxvt_scrollbar_show(r, 1);
		    }
# endif
		}
		break;
#endif
	    }
	}
    } else if (isMenuBarWindow(ev->window))
	rxvt_menubar_control(r, ev);
}


#ifdef TRANSPARENT
/*
 * Check our parents are still who we think they are.
 * Do transparency updates if required
 */
/* EXTPROTO */
int
rxvt_check_our_parents(rxvt_t *r)
{
    int             i, pchanged, aformat, have_pixmap, rootdepth;
    unsigned long   nitems, bytes_after;
    Atom            atype;
    unsigned char   *prop = NULL;
    Window          root, oldp, *list;
    Pixmap          rootpixmap = None;
    XWindowAttributes wattr, wrootattr;

    pchanged = 0;

    if (!(r->Options & Opt_transparent))
	return pchanged;	/* Don't try any more */

    XGetWindowAttributes(r->Xdisplay, Xroot, &wrootattr);
    rootdepth = wrootattr.depth;

    XGetWindowAttributes(r->Xdisplay, r->TermWin.parent[0], &wattr);
    if (rootdepth != wattr.depth) {
	if (r->h->am_transparent) {
	    pchanged = 1;
	    XSetWindowBackground(r->Xdisplay, r->TermWin.vt,
				 r->PixColors[Color_bg]);
	    r->h->am_transparent = r->h->am_pixmap_trans = 0;
	}
	return pchanged;	/* Don't try any more */
    }

/* Get all X ops out of the queue so that our information is up-to-date. */
    XSync(r->Xdisplay, False);

/*
 * Make the frame window set by the window manager have
 * the root background. Some window managers put multiple nested frame
 * windows for each client, so we have to take care about that.
 */
    i = (r->h->xa[XA_XROOTPMAPID] != 0
	 && (XGetWindowProperty(r->Xdisplay, Xroot, r->h->xa[XA_XROOTPMAPID],
				0L, 1L, False, XA_PIXMAP, &atype, &aformat,
				&nitems, &bytes_after, &prop) == Success));
    if (!i || prop == NULL)
	have_pixmap = 0;
    else {
	have_pixmap = 1;
	rootpixmap = *((Pixmap *)prop);
	XFree(prop);
    }
    if (have_pixmap) {
/*
 * Copy Xroot pixmap transparency
 */
	int             sx, sy, nx, ny;
	unsigned int    nw, nh;
	Window          cr;
	XImage         *image;
	GC              gc;
	XGCValues       gcvalue;

	XTranslateCoordinates(r->Xdisplay, r->TermWin.parent[0], Xroot,
			      0, 0, &sx, &sy, &cr);
	nw = (unsigned int)r->szHint.width;
	nh = (unsigned int)r->szHint.height;
	nx = ny = 0;
	if (sx < 0) {
	    nw += sx;
	    nx = -sx;
	    sx = 0;
	}
	if (sy < 0) {
	    nh += sy;
	    ny = -sy;
	    sy = 0;
	}
	MIN_IT(nw, (unsigned int)(wrootattr.width - sx));
	MIN_IT(nh, (unsigned int)(wrootattr.height - sy));
	r->h->allowedxerror = -1;
	image = XGetImage(r->Xdisplay, rootpixmap, sx, sy, nw, nh, AllPlanes,
			  ZPixmap);
	/* XXX: handle BadMatch - usually because we're outside the pixmap */
	/* XXX: may need a delay here? */
	r->h->allowedxerror = 0;
	if (image == NULL) {
	    if (r->h->am_transparent && r->h->am_pixmap_trans) {
		pchanged = 1;
		if (r->TermWin.pixmap != None) {
		    XFreePixmap(r->Xdisplay, r->TermWin.pixmap);
		    r->TermWin.pixmap = None;
		}
	    }
	    r->h->am_pixmap_trans = 0;
	} else {
	    if (r->TermWin.pixmap != None)
		XFreePixmap(r->Xdisplay, r->TermWin.pixmap);
	    r->TermWin.pixmap = XCreatePixmap(r->Xdisplay, r->TermWin.vt,
					      (unsigned int)r->szHint.width,
					      (unsigned int)r->szHint.height,
					      (unsigned int)image->depth);
	    gc = XCreateGC(r->Xdisplay, r->TermWin.vt, 0UL, &gcvalue);
	    XPutImage(r->Xdisplay, r->TermWin.pixmap, gc, image, 0, 0,
		      nx, ny, (unsigned int)image->width,
		      (unsigned int)image->height);
	    XFreeGC(r->Xdisplay, gc);
	    XDestroyImage(image);
	    XSetWindowBackgroundPixmap(r->Xdisplay, r->TermWin.vt,
				       r->TermWin.pixmap);
	    if (!r->h->am_transparent || !r->h->am_pixmap_trans)
		pchanged = 1;
	    r->h->am_transparent = r->h->am_pixmap_trans = 1;
	}
    }
    if (!r->h->am_pixmap_trans) {
	unsigned int    n;
/*
 * InheritPixmap transparency
 */
	D_X((stderr, "InheritPixmap Seeking to  %08lx", Xroot));
	for (i = 1; i < (int)(sizeof(r->TermWin.parent) / sizeof(Window));
	     i++) {
	    oldp = r->TermWin.parent[i];
	    XQueryTree(r->Xdisplay, r->TermWin.parent[i - 1], &root,
		       &r->TermWin.parent[i], &list, &n);
	    XFree(list);
	    D_X((stderr, "InheritPixmap Parent[%d] = %08lx", i, r->TermWin.parent[i]));
	    if (r->TermWin.parent[i] == Xroot) {
		if (oldp != None)
		    pchanged = 1;
		break;
	    }
	    if (oldp != r->TermWin.parent[i])
		pchanged = 1;
	}
	n = 0;
	if (pchanged) {
	    for (; n < (unsigned int)i; n++) {
		XGetWindowAttributes(r->Xdisplay, r->TermWin.parent[n], &wattr);
		D_X((stderr, "InheritPixmap Checking Parent[%d]: %s", n, (wattr.depth == rootdepth && wattr.class != InputOnly) ? "OK" : "FAIL"));
		if (wattr.depth != rootdepth || wattr.class == InputOnly) {
		    n = (int)(sizeof(r->TermWin.parent) / sizeof(Window)) + 1;
		    break;
		}
	    }
	}
	if (n > (int)(sizeof(r->TermWin.parent)
		      / sizeof(r->TermWin.parent[0]))) {
	    D_X((stderr, "InheritPixmap Turning off"));
	    XSetWindowBackground(r->Xdisplay, r->TermWin.parent[0],
				 r->PixColors[Color_fg]);
	    XSetWindowBackground(r->Xdisplay, r->TermWin.vt,
				 r->PixColors[Color_bg]);
	    r->h->am_transparent = 0;
	    /* XXX: also turn off Opt_transparent? */
	} else {
	    /* wait (an arbitrary period) for the WM to do its thing
	     * needed for fvwm2.2.2 (and before?) */
# ifdef HAVE_NANOSLEEP
	    struct timespec rqt;

	    rqt.tv_sec = 1;
	    rqt.tv_nsec = 0;
	    nanosleep(&rqt, NULL);
# else
	    sleep(1);	
# endif
	    D_X((stderr, "InheritPixmap Turning on (%d parents)", i - 1));
	    for (n = 0; n < (unsigned int)i; n++)
		XSetWindowBackgroundPixmap(r->Xdisplay, r->TermWin.parent[n],
					   ParentRelative);
	    XSetWindowBackgroundPixmap(r->Xdisplay, r->TermWin.vt,
				       ParentRelative);
	    r->h->am_transparent = 1;
	}
	for (; i < (int)(sizeof(r->TermWin.parent) / sizeof(Window)); i++)
	    r->TermWin.parent[i] = None;
    }
    return pchanged;
}
#endif

/*}}} */

/*{{{ print pipe */
/*----------------------------------------------------------------------*/
#ifdef PRINTPIPE
/* EXTPROTO */
FILE           *
rxvt_popen_printer(rxvt_t *r)
{
    FILE           *stream = popen(r->h->rs[Rs_print_pipe], "w");

    if (stream == NULL)
	rxvt_print_error("can't open printer pipe");
    return stream;
}

/* EXTPROTO */
int
rxvt_pclose_printer(FILE *stream)
{
    fflush(stream);
/* pclose() reported not to work on SunOS 4.1.3 */
# if defined (__sun__)		/* TODO: RESOLVE THIS */
/* pclose works provided SIGCHLD handler uses waitpid */
    return pclose(stream);	/* return fclose (stream); */
# else
    return pclose(stream);
# endif
}

/*
 * simulate attached vt100 printer
 */
/* INTPROTO */
void
rxvt_process_print_pipe(rxvt_t *r)
{
    int             done;
    FILE           *fd;

    if ((fd = rxvt_popen_printer(r)) == NULL)
	return;

/*
 * Send all input to the printer until either ESC[4i or ESC[?4i
 * is received.
 */
    for (done = 0; !done;) {
	unsigned char   buf[8];
	unsigned char   ch;
	unsigned int    i, len;

	if ((ch = rxvt_cmd_getc(r)) != C0_ESC) {
	    if (putc(ch, fd) == EOF)
		break;		/* done = 1 */
	} else {
	    len = 0;
	    buf[len++] = ch;

	    if ((buf[len++] = rxvt_cmd_getc(r)) == '[') {
		if ((ch = rxvt_cmd_getc(r)) == '?') {
		    buf[len++] = '?';
		    ch = rxvt_cmd_getc(r);
		}
		if ((buf[len++] = ch) == '4') {
		    if ((buf[len++] = rxvt_cmd_getc(r)) == 'i')
			break;	/* done = 1 */
		}
	    }
	    for (i = 0; i < len; i++)
		if (putc(buf[i], fd) == EOF) {
		    done = 1;
		    break;
		}
	}
    }
    rxvt_pclose_printer(fd);
}
#endif				/* PRINTPIPE */
/*}}} */

/* *INDENT-OFF* */
enum {
    C1_40 = 0x40,
	    C1_41 , C1_BPH, C1_NBH, C1_44 , C1_NEL, C1_SSA, C1_ESA,
    C1_HTS, C1_HTJ, C1_VTS, C1_PLD, C1_PLU, C1_RI , C1_SS2, C1_SS3,
    C1_DCS, C1_PU1, C1_PU2, C1_STS, C1_CCH, C1_MW , C1_SPA, C1_EPA,
    C1_SOS, C1_59 , C1_SCI, C1_CSI, CS_ST , C1_OSC, C1_PM , C1_APC
};
/* *INDENT-ON* */

/*{{{ process non-printing single characters */
/* INTPROTO */
void
rxvt_process_nonprinting(rxvt_t *r, unsigned char ch)
{
    switch (ch) {
    case C0_ENQ:	/* terminal Status */
	if (r->h->rs[Rs_answerbackstring])
	    rxvt_tt_write(r,
		(const unsigned char *)r->h->rs[Rs_answerbackstring],
		(unsigned int)STRLEN(r->h->rs[Rs_answerbackstring]));
	else
	    rxvt_tt_write(r, (unsigned char *)VT100_ANS,
			  (unsigned int)STRLEN(VT100_ANS));
	break;
    case C0_BEL:	/* bell */
	rxvt_scr_bell(r);
	break;
    case C0_BS:		/* backspace */
	rxvt_scr_backspace(r);
	break;
    case C0_HT:		/* tab */
	rxvt_scr_tab(r, 1);
	break;
    case C0_CR:		/* carriage return */
	rxvt_scr_gotorc(r, 0, 0, R_RELATIVE);
	break;
    case C0_VT:		/* vertical tab, form feed */
    case C0_FF:
    case C0_LF:		/* line feed */
	rxvt_scr_index(r, UP);
	break;
    case C0_SO:		/* shift out - acs */
	rxvt_scr_charset_choose(r, 1);
	break;
    case C0_SI:		/* shift in - acs */
	rxvt_scr_charset_choose(r, 0);
	break;
    }
}
/*}}} */


/*{{{ process VT52 escape sequences */
/* INTPROTO */
void
rxvt_process_escape_vt52(rxvt_t *r, unsigned char ch)
{
    int row, col;
    
    switch (ch) {
    case 'A':		/* cursor up */
	rxvt_scr_gotorc(r, -1, 0, R_RELATIVE | C_RELATIVE);	
	break;
    case 'B':		/* cursor down */
	rxvt_scr_gotorc(r, 1, 0, R_RELATIVE | C_RELATIVE);	
	break;
    case 'C':		/* cursor right */
	rxvt_scr_gotorc(r, 0, 1, R_RELATIVE | C_RELATIVE);	
	break;
    case 'D':		/* cursor left */
	rxvt_scr_gotorc(r, 0, -1, R_RELATIVE | C_RELATIVE);	
	break;
    case 'H':		/* cursor home */
	rxvt_scr_gotorc(r, 0, 0, 0);	
	break;
    case 'I':		/* cursor up and scroll down if needed */
	rxvt_scr_index(r, DN);
	break;
    case 'J':		/* erase to end of screen */
	rxvt_scr_erase_screen(r, 0);
	break;
    case 'K':		/* erase to end of line */
	rxvt_scr_erase_line(r, 0);
	break;
    case 'Y':         	/* move to specified row and col */
	  /* full command is 'ESC Y row col' where row and col
	   * are encoded by adding 32 and sending the ascii
	   * character.  eg. SPACE = 0, '+' = 13, '0' = 18,
	   * etc. */
	row = rxvt_cmd_getc(r) - ' ';
	col = rxvt_cmd_getc(r) - ' ';
	rxvt_scr_gotorc(r, row, col, 0);
	break;
    case 'Z':		/* identify the terminal type */
	rxvt_tt_printf(r, "\033/Z");	/* I am a VT100 emulating a VT52 */
        break;
    case '<':		/* turn off VT52 mode */
        PrivMode(0, PrivMode_vt52);
	break;
    case 'F':     	/* use special graphics character set */
    case 'G':           /* use regular character set */
	  /* unimplemented */
	break;
    case '=':     	/* use alternate keypad mode */
    case '>':           /* use regular keypad mode */
	  /* unimplemented */
	break;
    }
}
/*}}} */


/*{{{ process escape sequences */
/* INTPROTO */
void
rxvt_process_escape_seq(rxvt_t *r)
{
    unsigned char   ch = rxvt_cmd_getc(r);

    if (r->h->PrivateModes & PrivMode_vt52) {
	rxvt_process_escape_vt52(r, ch);
	return;
    }
    
    switch (ch) {
    /* case 1:        do_tek_mode (); break; */
    case '#':
	if (rxvt_cmd_getc(r) == '8')
	    rxvt_scr_E(r);
	break;
    case '(':
	rxvt_scr_charset_set(r, 0, (unsigned int)rxvt_cmd_getc(r));
	break;
    case ')':
	rxvt_scr_charset_set(r, 1, (unsigned int)rxvt_cmd_getc(r));
	break;
    case '*':
	rxvt_scr_charset_set(r, 2, (unsigned int)rxvt_cmd_getc(r));
	break;
    case '+':
	rxvt_scr_charset_set(r, 3, (unsigned int)rxvt_cmd_getc(r));
	break;
#ifdef MULTICHAR_SET
    case '$':
	rxvt_scr_charset_set(r, -2, (unsigned int)rxvt_cmd_getc(r));
	break;
#endif
#ifndef NO_FRILLS
    case '6':
	rxvt_scr_backindex(r);
	break;
#endif
    case '7':
	rxvt_scr_cursor(r, SAVE);
	break;
    case '8':
	rxvt_scr_cursor(r, RESTORE);
	break;
#ifndef NO_FRILLS
    case '9':
	rxvt_scr_forwardindex(r);
	break;
#endif
    case '=':
    case '>':
	PrivMode((ch == '='), PrivMode_aplKP);
	break;

    case C1_40:
	rxvt_cmd_getc(r);
	break;
    case C1_44:
	rxvt_scr_index(r, UP);
	break;

    /* 8.3.87: NEXT LINE */
    case C1_NEL:		/* ESC E */
	rxvt_scr_add_lines(r, (const unsigned char *)"\n\r", 1, 2);
	break;

    /* kidnapped escape sequence: Should be 8.3.48 */
    case C1_ESA:		/* ESC G */
	rxvt_process_graphics(r);
	break;

    /* 8.3.63: CHARACTER TABULATION SET */
    case C1_HTS:		/* ESC H */
	rxvt_scr_set_tab(r, 1);
	break;

    /* 8.3.105: REVERSE LINE FEED */
    case C1_RI:			/* ESC M */
	rxvt_scr_index(r, DN);
	break;

    /* 8.3.142: SINGLE-SHIFT TWO */
    /*case C1_SS2: scr_single_shift (2);   break; */

    /* 8.3.143: SINGLE-SHIFT THREE */
    /*case C1_SS3: scr_single_shift (3);   break; */

    /* 8.3.27: DEVICE CONTROL STRING */
    case C1_DCS:		/* ESC P */
	rxvt_process_dcs_seq(r);
	break;

    /* 8.3.110: SINGLE CHARACTER INTRODUCER */
    case C1_SCI:		/* ESC Z */
	rxvt_tt_write(r, (const unsigned char *)ESCZ_ANSWER,
		      (unsigned int)(sizeof(ESCZ_ANSWER) - 1));
	break;			/* steal obsolete ESC [ c */

    /* 8.3.16: CONTROL SEQUENCE INTRODUCER */
    case C1_CSI:		/* ESC [ */
	rxvt_process_csi_seq(r);
	break;

    /* 8.3.90: OPERATING SYSTEM COMMAND */
    case C1_OSC:		/* ESC ] */
	rxvt_process_osc_seq(r);
	break;

    /* 8.3.106: RESET TO INITIAL STATE */
    case 'c':
	rxvt_scr_poweron(r);
	rxvt_scrollbar_show(r, 1);
	break;

    /* 8.3.79: LOCKING-SHIFT TWO (see ISO2022) */
    case 'n':
	rxvt_scr_charset_choose(r, 2);
	break;

    /* 8.3.81: LOCKING-SHIFT THREE (see ISO2022) */
    case 'o':
	rxvt_scr_charset_choose(r, 3);
	break;
    }
}
/*}}} */

/*{{{ process CONTROL SEQUENCE INTRODUCER (CSI) sequences `ESC[' */
/* *INDENT-OFF* */
enum {
    CSI_ICH = 0x40,
	     CSI_CUU, CSI_CUD, CSI_CUF, CSI_CUB, CSI_CNL, CSI_CPL, CSI_CHA,
    CSI_CUP, CSI_CHT, CSI_ED , CSI_EL , CSI_IL , CSI_DL , CSI_EF , CSI_EA ,
    CSI_DCH, CSI_SEE, CSI_CPR, CSI_SU , CSI_SD , CSI_NP , CSI_PP , CSI_CTC,
    CSI_ECH, CSI_CVT, CSI_CBT, CSI_SRS, CSI_PTX, CSI_SDS, CSI_SIMD, CSI_5F,
    CSI_HPA, CSI_HPR, CSI_REP, CSI_DA , CSI_VPA, CSI_VPR, CSI_HVP, CSI_TBC,
    CSI_SM , CSI_MC , CSI_HPB, CSI_VPB, CSI_RM , CSI_SGR, CSI_DSR, CSI_DAQ,
    CSI_70 , CSI_71 , CSI_72 , CSI_73 , CSI_74 , CSI_75 , CSI_76 , CSI_77 ,
    CSI_78 , CSI_79 , CSI_7A , CSI_7B , CSI_7C , CSI_7D , CSI_7E , CSI_7F 
};

#define make_byte(b7,b6,b5,b4,b3,b2,b1,b0)			\
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4)	\
     | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | (b0))
#define get_byte_array_bit(array, bit)				\
    (!!((array)[(bit) / 8] & (128 >> ((bit) & 7))))

const unsigned char csi_defaults[] = {
    make_byte(1,1,1,1,1,1,1,1),	/* @, A, B, C, D, E, F, G, */
    make_byte(1,1,0,0,1,1,0,0),	/* H, I, J, K, L, M, N, O, */
    make_byte(1,0,1,1,1,1,1,0),	/* P, Q, R, S, T, U, V, W, */
    make_byte(1,1,1,0,0,0,1,0),	/* X, Y, Z, [, \, ], ^, _, */
    make_byte(1,1,1,0,1,1,1,0),	/* `, a, b, c, d, e, f, g, */
    make_byte(0,0,1,1,0,0,0,0),	/* h, i, j, k, l, m, n, o, */
    make_byte(0,0,0,0,0,0,0,0),	/* p, q, r, s, t, u, v, w, */
    make_byte(0,0,0,0,0,0,0,0)	/* x, y, z, {, |, }, ~,    */
};
/* *INDENT-ON* */

/* INTPROTO */
void
rxvt_process_csi_seq(rxvt_t *r)
{
    unsigned char   ch, priv, i;
    unsigned int    nargs, p;
    int             n, ndef;
    int             arg[ESC_ARGS];

    for (nargs = ESC_ARGS; nargs > 0;)
	arg[--nargs] = 0;

    priv = 0;
    ch = rxvt_cmd_getc(r);
    if (ch >= '<' && ch <= '?') {	/* '<' '=' '>' '?' */
	priv = ch;
	ch = rxvt_cmd_getc(r);
    }
/* read any numerical arguments */
    for (n = -1; ch < CSI_ICH; ) {
	if (isdigit(ch)) {
	    if (n < 0)
		n = ch - '0';
	    else
		n = n * 10 + ch - '0';
	} else if (ch == ';') {
	    if (nargs < ESC_ARGS)
		arg[nargs++] = n;
	    n = -1;
	} else if (ch == '\b') {
	    rxvt_scr_backspace(r);
	} else if (ch == C0_ESC) {
	    rxvt_process_escape_seq(r);
	    return;
	} else if (ch < ' ') {
	    rxvt_process_nonprinting(r, ch);
	}
	ch = rxvt_cmd_getc(r);
    }

    if (ch > CSI_7F)
	return;

    if (nargs < ESC_ARGS)
	arg[nargs++] = n;

    i = ch - CSI_ICH;
    ndef = get_byte_array_bit(csi_defaults, i);
    for (p = 0; p < nargs; p++)
	if (arg[p] == -1)
	    arg[p] = ndef;

#ifdef DEBUG_CMD
    fprintf(stderr, "CSI ");
    for (p = 0; p < nargs; p++)
	fprintf(stderr, "%d%s", arg[p], p < nargs - 1 ? ";" : "");
    fprintf(stderr, "%c\n", ch);
#endif

/*
 * private mode handling
 */
    if (priv) {
	switch (priv) {
	case '>':
	    if (ch == CSI_DA)	/* secondary device attributes */
		rxvt_tt_printf(r, "\033[>%d;%-.8s;0c", 'R', VSTRING);
	    break;
	case '?':
	    if (ch == 'h' || ch == 'l' || ch == 'r' || ch == 's' || ch == 't')
		rxvt_process_terminal_mode(r, ch, priv, nargs, arg);
	    break;
	}
	return;
    }

    switch (ch) {
/*
 * ISO/IEC 6429:1992(E) CSI sequences (defaults in parentheses)
 */
#ifdef PRINTPIPE
    case CSI_MC:		/* 8.3.83: (0) MEDIA COPY */
	switch (arg[0]) {
	case 0:			/* initiate transfer to primary aux device */
	    rxvt_scr_printscreen(r, 0);
	    break;
	case 5:			/* start relay to primary aux device */
	    rxvt_process_print_pipe(r);
	    break;
	}
	break;
#endif

    case CSI_CUU:		/* 8.3.22: (1) CURSOR UP */
    case CSI_VPR:		/* 8.3.161: (1) LINE POSITION FORWARD */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_CUD:		/* 8.3.19: (1) CURSOR DOWN */
    case CSI_VPB:		/* 8.3.160: (1) LINE POSITION BACKWARD */
	rxvt_scr_gotorc(r, arg[0], 0, RELATIVE);
	break;

    case CSI_CUB:		/* 8.3.18: (1) CURSOR LEFT */
    case CSI_HPB: 		/* 8.3.59: (1) CHARACTER POSITION BACKWARD */
#ifdef ISO6429
	arg[0] = -arg[0];
#else				/* emulate common DEC VTs */
	arg[0] = arg[0] ? -arg[0] : -1;
#endif
    /* FALLTHROUGH */
    case CSI_CUF:		/* 8.3.20: (1) CURSOR RIGHT */
    case CSI_HPR:		/* 8.3.60: (1) CHARACTER POSITION FORWARD */
#ifdef ISO6429
	rxvt_scr_gotorc(r, 0, arg[0], RELATIVE);
#else				/* emulate common DEC VTs */
	rxvt_scr_gotorc(r, 0, arg[0] ? arg[0] : 1, RELATIVE);
#endif
	break;

    case CSI_CPL:		/* 8.3.13: (1) CURSOR PRECEDING LINE */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_CNL:		/* 8.3.12: (1) CURSOR NEXT LINE */
	rxvt_scr_gotorc(r, arg[0], 0, R_RELATIVE);
	break;

    case CSI_CHA:		/* 8.3.9: (1) CURSOR CHARACTER ABSOLUTE */
    case CSI_HPA:		/* 8.3.58: (1) CURSOR POSITION ABSOLUTE */
	rxvt_scr_gotorc(r, 0, arg[0] - 1, R_RELATIVE);
	break;

    case CSI_VPA:		/* 8.3.159: (1) LINE POSITION ABSOLUTE */
	rxvt_scr_gotorc(r, arg[0] - 1, 0, C_RELATIVE);
	break;

    case CSI_CUP:		/* 8.3.21: (1,1) CURSOR POSITION */
    case CSI_HVP:		/* 8.3.64: (1,1) CHARACTER AND LINE POSITION */
	rxvt_scr_gotorc(r, arg[0] - 1, nargs < 2 ? 0 : (arg[1] - 1), 0);
	break;

    case CSI_CBT:		/* 8.3.7: (1) CURSOR BACKWARD TABULATION */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_CHT:		/* 8.3.10: (1) CURSOR FORWARD TABULATION */
	rxvt_scr_tab(r, arg[0]);
	break;

    case CSI_ED:		/* 8.3.40: (0) ERASE IN PAGE */
	rxvt_scr_erase_screen(r, arg[0]);
	break;

    case CSI_EL:		/* 8.3.42: (0) ERASE IN LINE */
	rxvt_scr_erase_line(r, arg[0]);
	break;

    case CSI_ICH:		/* 8.3.65: (1) INSERT CHARACTER */
	rxvt_scr_insdel_chars(r, arg[0], INSERT);
	break;

    case CSI_IL:		/* 8.3.68: (1) INSERT LINE */
	rxvt_scr_insdel_lines(r, arg[0], INSERT);
	break;

    case CSI_DL:		/* 8.3.33: (1) DELETE LINE */
	rxvt_scr_insdel_lines(r, arg[0], DELETE);
	break;

    case CSI_ECH:		/* 8.3.39: (1) ERASE CHARACTER */
	rxvt_scr_insdel_chars(r, arg[0], ERASE);
	break;

    case CSI_DCH:		/* 8.3.26: (1) DELETE CHARACTER */
	rxvt_scr_insdel_chars(r, arg[0], DELETE);
	break;

    case CSI_SD:		/* 8.3.114: (1) SCROLL DOWN */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_SU:		/* 8.3.148: (1) SCROLL UP */
	rxvt_scroll_text(r, r->screen.tscroll, r->screen.bscroll, arg[0], 0);
	break;

    case CSI_DA:		/* 8.3.24: (0) DEVICE ATTRIBUTES */
	rxvt_tt_write(r, (const unsigned char *)VT100_ANS,
		      (unsigned int)(sizeof(VT100_ANS) - 1));
	break;

    case CSI_SGR:		/* 8.3.118: (0) SELECT GRAPHIC RENDITION */
	rxvt_process_sgr_mode(r, nargs, arg);
	break;

    case CSI_DSR:		/* 8.3.36: (0) DEVICE STATUS REPORT */
	switch (arg[0]) {
	case 5:			/* DSR requested */
	    rxvt_tt_printf(r, "\033[0n");
	    break;
	case 6:			/* CPR requested */
	    rxvt_scr_report_position(r);
	    break;
#if defined (ENABLE_DISPLAY_ANSWER)
	case 7:			/* unofficial extension */
	    rxvt_tt_printf(r, "%-.250s\n", r->h->rs[Rs_display_name]);
	    break;
#endif
	case 8:			/* unofficial extension */
	    rxvt_xterm_seq(r, XTerm_title, APL_NAME "-" VERSION, CHAR_ST);
	    break;
	}
	break;

    case CSI_TBC:		/* 8.3.155: (0) TABULATION CLEAR */
	switch (arg[0]) {
	case 0:			/* char tab stop cleared at active position */
	    rxvt_scr_set_tab(r, 0);
	    break;
	/* case 1: */		/* line tab stop cleared in active line */
	/* case 2: */		/* char tab stops cleared in active line */
	case 3:			/* all char tab stops are cleared */
	/* case 4: */		/* all line tab stops are cleared */
	case 5:			/* all tab stops are cleared */
	    rxvt_scr_set_tab(r, -1);
	    break;
	}
	break;

    case CSI_CTC:		/* 8.3.17: (0) CURSOR TABULATION CONTROL */
	switch (arg[0]) {
	case 0:			/* char tab stop set at active position */
	    rxvt_scr_set_tab(r, 1);
	    break;		/* = ESC H */
	/* case 1: */		/* line tab stop set at active line */
	case 2:			/* char tab stop cleared at active position */
	    rxvt_scr_set_tab(r, 0);
	    break;		/* = ESC [ 0 g */
	/* case 3: */		/* line tab stop cleared at active line */
	/* case 4: */		/* char tab stops cleared at active line */
	case 5:			/* all char tab stops are cleared */
	    rxvt_scr_set_tab(r, -1);
	    break;		/* = ESC [ 3 g */
	/* case 6: */		/* all line tab stops are cleared */
	}
	break;

    case CSI_RM:		/* 8.3.107: RESET MODE */
	if (arg[0] == 4)
	    rxvt_scr_insert_mode(r, 0);
	break;

    case CSI_SM:		/* 8.3.126: SET MODE */
	if (arg[0] == 4)
	    rxvt_scr_insert_mode(r, 1);
	break;

/*
 * PRIVATE USE beyond this point.  All CSI_7? seqeunces here
 */ 
    case CSI_72:		/* DECSTBM: set top and bottom margins */
	if (nargs == 1)
	    rxvt_scr_scroll_region(r, arg[0] - 1, MAX_ROWS - 1);
	else if (nargs == 0 || arg[0] >= arg[1])
	    rxvt_scr_scroll_region(r, 0, MAX_ROWS - 1);
	else 
	    rxvt_scr_scroll_region(r, arg[0] - 1, arg[1] - 1);
	break;

    case CSI_73:
	rxvt_scr_cursor(r, SAVE);
	break;
    case CSI_75:
	rxvt_scr_cursor(r, RESTORE);
	break;

#ifndef NO_FRILLS
    case CSI_74:
	rxvt_process_window_ops(r, arg, nargs);
	break;
#endif

    case CSI_78:		/* DECREQTPARM */
	if (arg[0] == 0 || arg[0] == 1)
	    rxvt_tt_printf(r, "\033[%d;1;1;112;112;1;0x", arg[0] + 2);
    /* FALLTHROUGH */

    default:
	break;
    }
}
/*}}} */

#ifndef NO_FRILLS
/* ARGSUSED */
/* INTPROTO */
void
rxvt_process_window_ops(rxvt_t *r, const int *args, unsigned int nargs)
{
    int             x, y;
    char           *s;
    XWindowAttributes wattr;
    Window          wdummy;

    if (nargs == 0)
	return;
    switch (args[0]) {
    /*
     * commands
     */
    case 1:			/* deiconify window */
	XMapWindow(r->Xdisplay, r->TermWin.parent[0]);
	break;
    case 2:			/* iconify window */
	XIconifyWindow(r->Xdisplay, r->TermWin.parent[0],
		       DefaultScreen(r->Xdisplay));
	break;
    case 3:			/* set position (pixels) */
	XMoveWindow(r->Xdisplay, r->TermWin.parent[0], args[1], args[2]);
	break;
    case 4:			/* set size (pixels) */
	rxvt_set_widthheight(r, (unsigned int)args[2], (unsigned int)args[1]);
	break;
    case 5:			/* raise window */
	XRaiseWindow(r->Xdisplay, r->TermWin.parent[0]);
	break;
    case 6:			/* lower window */
	XLowerWindow(r->Xdisplay, r->TermWin.parent[0]);
	break;
    case 7:			/* refresh window */
	rxvt_scr_touch(r, True);
	break;
    case 8:			/* set size (chars) */
	rxvt_set_widthheight(r, (unsigned int)(args[2] * r->TermWin.fwidth),
			     (unsigned int)(args[1] * r->TermWin.fheight));
	break;
    default:
	if (args[0] >= 24)	/* set height (chars) */
	    rxvt_set_widthheight(r, (unsigned int)r->TermWin.width,
				 (unsigned int)(args[1] * r->TermWin.fheight));
	break;
    /*
     * reports - some output format copied from XTerm
     */
    case 11:			/* report window state */
	XGetWindowAttributes(r->Xdisplay, r->TermWin.parent[0], &wattr);
	rxvt_tt_printf(r, "\033[%dt", wattr.map_state == IsViewable ? 1 : 2);
	break;
    case 13:			/* report window position */
	XGetWindowAttributes(r->Xdisplay, r->TermWin.parent[0], &wattr);
	XTranslateCoordinates(r->Xdisplay, r->TermWin.parent[0], wattr.root,
			      -wattr.border_width, -wattr.border_width,
			      &x, &y, &wdummy);
	rxvt_tt_printf(r, "\033[3;%d;%dt", x, y);
	break;
    case 14:			/* report window size (pixels) */
	XGetWindowAttributes(r->Xdisplay, r->TermWin.parent[0], &wattr);
	rxvt_tt_printf(r, "\033[4;%d;%dt", wattr.height, wattr.width);
	break;
    case 18:			/* report window size (chars) */
	rxvt_tt_printf(r, "\033[8;%d;%dt", r->TermWin.nrow, r->TermWin.ncol);
	break;
#if 0 /* XXX: currently disabled due to security concerns */
    case 20:			/* report icon label */
	XGetIconName(r->Xdisplay, r->TermWin.parent[0], &s);
	rxvt_tt_printf(r, "\033]L%-.200s\234", s ? s : "");	/* 8bit ST */
	break;
    case 21:			/* report window title */
	XFetchName(r->Xdisplay, r->TermWin.parent[0], &s);
	rxvt_tt_printf(r, "\033]l%-.200s\234", s ? s : "");	/* 8bit ST */
	break;
#endif
    }
}
#endif

/*----------------------------------------------------------------------*/
/*
 * get input up until STRING TERMINATOR (or BEL)
 * ends_how is terminator used.  returned input must be free()d
 */
/* INTPROTO */
unsigned char  *
rxvt_get_to_st(rxvt_t *r, unsigned char *ends_how)
{
    int             seen_esc = 0;	/* seen escape? */
    unsigned int    n = 0;
    unsigned char  *s;
    unsigned char   ch, string[STRING_MAX];

    for (; (ch = rxvt_cmd_getc(r));) {
	if (ch == C0_BEL
	    || ch == CHAR_ST
	    || (ch == 0x5c && seen_esc))	/* 7bit ST */
	    break;
	if (ch == C0_ESC) {
	    seen_esc = 1;
	    continue;
	} else if (ch == '\t')
	    ch = ' ';	/* translate '\t' to space */
	else if (ch < 0x08 || (ch > 0x0d && ch < 0x20))
	    return NULL;	/* other control character - exit */
	if (n < sizeof(string) - 1)
	    string[n++] = ch;
	seen_esc = 0;
    }
    string[n++] = '\0';
    if ((s = (unsigned char *)rxvt_malloc(n)) == NULL)
	return NULL;
    *ends_how = (ch == 0x5c ? C0_ESC : ch);
    STRNCPY(s, string, n);
    return s;
}

/*----------------------------------------------------------------------*/
/*
 * process DEVICE CONTROL STRING `ESC P ... (ST|BEL)' or `0x90 ... (ST|BEL)'
 */
/* INTPROTO */
void
rxvt_process_dcs_seq(rxvt_t *r)
{
    unsigned char    eh, *s;
/*
 * Not handled yet
 */
    s = rxvt_get_to_st(r, &eh);
    if (s)
	free(s);
    return;
}

/*----------------------------------------------------------------------*/
/*
 * process OPERATING SYSTEM COMMAND sequence `ESC ] Ps ; Pt (ST|BEL)'
 */
/* INTPROTO */
void
rxvt_process_osc_seq(rxvt_t *r)
{
    unsigned char   ch, eh, *s;
    int             arg;

    ch = rxvt_cmd_getc(r);
    for (arg = 0; isdigit(ch); ch = rxvt_cmd_getc(r))
	arg = arg * 10 + (ch - '0');

    if (ch == ';') {
	s = rxvt_get_to_st(r, &eh);
	if (s) {
    /*
     * rxvt_menubar_dispatch() violates the constness of the string,
     * so do it here
     */
	    if (arg == XTerm_Menu)
#if 0 /* XXX: currently disabled due to security concerns */
		rxvt_menubar_dispatch(r, (char *)s);
#else
		0;
#endif
	    else
		rxvt_xterm_seq(r, arg, (char *)s, eh);
	    free(s);
	}
    }
}
/*
 * XTerm escape sequences: ESC ] Ps;Pt (ST|BEL)
 *       0 = change iconName/title
 *       1 = change iconName
 *       2 = change title
 *       4 = change color
 *      12 = change text color
 *      13 = change mouse foreground color 
 *      17 = change highlight character colour
 *      18 = change bold character color
 *      19 = change underlined character color 
 *      46 = change logfile (not implemented)
 *      50 = change font
 *
 * rxvt extensions:
 *      10 = menu (may change in future)
 *      20 = bg pixmap
 *      39 = change default fg color
 *      49 = change default bg color
 *      55 = dump scrollback buffer and all of screen
 */
/* EXTPROTO */
void
rxvt_xterm_seq(rxvt_t *r, int op, const char *str, unsigned char resp __attribute__((unused)))
{
    int             changed = 0;
    int             fd;
    int             color;
    char           *buf, *name;

    assert(str != NULL);
    switch (op) {
    case XTerm_name:
	rxvt_set_title(r, str);
    /* FALLTHROUGH */
    case XTerm_iconName:
	rxvt_set_iconName(r, str);
	break;
    case XTerm_title:
	rxvt_set_title(r, str);
	break;
    case XTerm_Color:
	for (buf = (char *)str; buf && *buf;) {
	    if ((name = STRCHR(buf, ';')) == NULL)
		break;
	    *name++ = '\0';
	    color = atoi(buf);
	    if (color < 0 || color >= TOTAL_COLORS)
		break;
	    if ((buf = STRCHR(name, ';')) != NULL)
		*buf++ = '\0';
	    rxvt_set_window_color(r, color + minCOLOR, name);
	}
	break;
#ifndef NO_CURSORCOLOR
    case XTerm_Color_cursor:
	rxvt_set_window_color(r, Color_cursor, str);
	break;
#endif
    case XTerm_Color_pointer:
	rxvt_set_window_color(r, Color_pointer, str);
	break;
#ifndef NO_BOLD_UNDERLINE_REVERSE
    case XTerm_Color_BD:
	rxvt_set_window_color(r, Color_BD, str);
	break;
    case XTerm_Color_UL:
	rxvt_set_window_color(r, Color_UL, str);
	break;
    case XTerm_Color_RV:
	rxvt_set_window_color(r, Color_RV, str);
	break;
#endif

    case XTerm_Menu:
    /*
     * rxvt_menubar_dispatch() violates the constness of the string,
     * so DON'T do it here
     */
	break;
    case XTerm_Pixmap:
	if (*str != ';') {
	    rxvt_scale_pixmap(r, "");	/* reset to default scaling */
	    rxvt_set_bgPixmap(r, str);	/* change pixmap */
	    rxvt_scr_touch(r, True);
	}
	while ((str = STRCHR(str, ';')) != NULL) {
	    str++;
	    changed += rxvt_scale_pixmap(r, str);
	}
	if (changed) {
	    rxvt_resize_pixmap(r);
	    rxvt_scr_touch(r, True);
	}
	break;

    case XTerm_restoreFG:
	rxvt_set_window_color(r, Color_fg, str);
	break;
    case XTerm_restoreBG:
	rxvt_set_window_color(r, Color_bg, str);
	break;
    case XTerm_logfile:
	break;
    case XTerm_font:
	rxvt_change_font(r, 0, str);
	break;
#if 0
    case XTerm_dumpscreen:	/* no error notices */
	if ((fd = open(str, O_RDWR | O_CREAT | O_EXCL, 0600)) >= 0) {
	    rxvt_scr_dump(r, fd);
	    close(fd);
	}
	break;
#endif
    }
}
/*----------------------------------------------------------------------*/

/*{{{ process DEC private mode sequences `ESC [ ? Ps mode' */
/*
 * mode can only have the following values:
 *      'l' = low
 *      'h' = high
 *      's' = save
 *      'r' = restore
 *      't' = toggle
 * so no need for fancy checking
 */
/* INTPROTO */
int
rxvt_privcases(rxvt_t *r, int mode, unsigned long bit)
{
    int             state;

    if (mode == 's') {
	r->h->SavedModes |= (r->h->PrivateModes & bit);
	return -1;
    } else {
	if (mode == 'r')
	    state = (r->h->SavedModes & bit) ? 1 : 0;	/* no overlapping */
	else
	    state = (mode == 't') ? !(r->h->PrivateModes & bit) : mode;
	PrivMode(state, bit);
    }
    return state;
}

/* we're not using priv _yet_ */
/* INTPROTO */
void
rxvt_process_terminal_mode(rxvt_t *r, int mode, int priv __attribute__((unused)), unsigned int nargs, const int *arg)
{
    unsigned int    i, j;
    int             state;
    static const struct {
	const int       argval;
	const unsigned long bit;
    } argtopriv[] = {
	{ 1, PrivMode_aplCUR },
	{ 2, PrivMode_vt52 },
	{ 3, PrivMode_132 },
	{ 4, PrivMode_smoothScroll },
	{ 5, PrivMode_rVideo },
	{ 6, PrivMode_relOrigin },
	{ 7, PrivMode_Autowrap },
	{ 9, PrivMode_MouseX10 },
#ifdef menuBar_esc
	{ menuBar_esc, PrivMode_menuBar },
#endif
#ifdef scrollBar_esc
	{ scrollBar_esc, PrivMode_scrollBar },
#endif
	{ 25, PrivMode_VisibleCursor },
	{ 35, PrivMode_ShiftKeys },
	{ 40, PrivMode_132OK },
	{ 47, PrivMode_Screen },
	{ 66, PrivMode_aplKP },
#ifndef NO_BACKSPACE_KEY
	{ 67, PrivMode_BackSpace },
#endif
	{ 1000, PrivMode_MouseX11 },
	{ 1010, PrivMode_TtyOutputInh },
	{ 1011, PrivMode_Keypress },
	{ 1047, PrivMode_Screen },
    };

    if (nargs == 0)
	return;

/* make lo/hi boolean */
    if (mode == 'l')
	mode = 0;		/* reset */
    else if (mode == 'h')
	mode = 1;		/* set */

    for (i = 0; i < nargs; i++) {
	state = -1;

	/* basic handling */
	for (j = 0; j < (sizeof(argtopriv)/sizeof(argtopriv[0])); j++)
	    if (argtopriv[j].argval == arg[i]) {
		state = rxvt_privcases(r, mode, argtopriv[j].bit);
		break;
	    }
	
	/* extra handling for values with state unkept  */
	if (state == -1)
	    switch (arg[i]) {
	    case 1048:		/* alternative cursor save */
		if (mode == 0)
		    rxvt_scr_cursor(r, RESTORE);
		else if (mode == 1)
		    rxvt_scr_cursor(r, SAVE);
	    /* FALLTHROUGH */
	    default:
		continue;	/* for(;i;) */
	    }

	/* extra handling for values with valid 0 or 1 state */
	switch (arg[i]) {
	/* case 1:	- application cursor keys */
	case 2:			/* VT52 mode */
	      /* oddball mode.  should be set regardless of set/reset
	       * parameter.  Return from VT52 mode with an ESC < from
	       * within VT52 mode
	       */
	    PrivMode(1, PrivMode_vt52);
	    break;
	case 3:			/* 80/132 */
	    if (r->h->PrivateModes & PrivMode_132OK)
		rxvt_set_widthheight(r,
		    (unsigned int)((state ? 132 : 80) * r->TermWin.fwidth),
		    (unsigned int)r->TermWin.height);
	    break;
	case 4:			/* smooth scrolling */
	    if (state)
		r->Options &= ~Opt_jumpScroll;
	    else
		r->Options |= Opt_jumpScroll;
	    break;
	case 5:			/* reverse video */
	    rxvt_scr_rvideo_mode(r, state);
	    break;
	case 6:			/* relative/absolute origins  */
	    rxvt_scr_relative_origin(r, state);
	    break;
	case 7:			/* autowrap */
	    rxvt_scr_autowrap(r, state);
	    break;
	/* case 8:	- auto repeat, can't do on a per window basis */
	case 9:			/* X10 mouse reporting */
	    if (state)		/* orthogonal */
		r->h->PrivateModes &= ~(PrivMode_MouseX11);
	    break;
#ifdef menuBar_esc
	case menuBar_esc:
	    rxvt_map_menuBar(r, state);
	    break;
#endif
#ifdef scrollBar_esc
	case scrollBar_esc:
	    if (rxvt_scrollbar_mapping(r, state)) {
		rxvt_resize_all_windows(r, 0, 0, 0);
		rxvt_scr_touch(r, True);
	    }
	    break;
#endif
	case 25:		/* visible/invisible cursor */
	    rxvt_scr_cursor_visible(r, state);
	    break;
	/* case 35:	- shift keys */
	/* case 40:	- 80 <--> 132 mode */
	case 47:		/* secondary screen */
	    rxvt_scr_change_screen(r, state);
	    break;
	/* case 66:	- application key pad */
	/* case 67:	- backspace key */
	case 1000:		/* X11 mouse reporting */
	    if (state)		/* orthogonal */
		r->h->PrivateModes &= ~(PrivMode_MouseX10);
	    break;
#if 0
	case 1001:
	    break;		/* X11 mouse highlighting */
#endif
	case 1010:		/* scroll to bottom on TTY output inhibit */
	    if (state)
		r->Options &= ~Opt_scrollTtyOutput;
	    else
		r->Options |= Opt_scrollTtyOutput;
	    break;
	case 1011:		/* scroll to bottom on key press */
	    if (state)
		r->Options |= Opt_scrollTtyKeypress;
	    else
		r->Options &= ~Opt_scrollTtyKeypress;
	    break;
	case 1047:		/* secondary screen w/ clearing */
	    if (r->h->current_screen != PRIMARY)
		rxvt_scr_erase_screen(r, 2);
	    rxvt_scr_change_screen(r, state);
	/* FALLTHROUGH */
	default:
	    break;
	}
    }
}
/*}}} */

/*{{{ process sgr sequences */
/* INTPROTO */
void
rxvt_process_sgr_mode(rxvt_t *r, unsigned int nargs, const int *arg)
{
    unsigned int    i;
    short           rendset;
    int             rendstyle;

    if (nargs == 0) {
	rxvt_scr_rendition(r, 0, ~RS_None);
	return;
    }
    for (i = 0; i < nargs; i++) {
	rendset = -1;
	switch (arg[i]) {
	case 0:
	    rendset = 0, rendstyle = ~RS_None;
	    break;
	case 1:
	    rendset = 1, rendstyle = RS_Bold;
	    break;
	case 4:
	    rendset = 1, rendstyle = RS_Uline;
	    break;
	case 5:
	    rendset = 1, rendstyle = RS_Blink;
	    break;
	case 7:
	    rendset = 1, rendstyle = RS_RVid;
	    break;
	case 22:
	    rendset = 0, rendstyle = RS_Bold;
	    break;
	case 24:
	    rendset = 0, rendstyle = RS_Uline;
	    break;
	case 25:
	    rendset = 0, rendstyle = RS_Blink;
	    break;
	case 27:
	    rendset = 0, rendstyle = RS_RVid;
	    break;
	}
	if (rendset != -1) {
	    rxvt_scr_rendition(r, rendset, rendstyle);
	    continue;		/* for(;i;) */
	}

	switch (arg[i]) {
	case 30:
	case 31:		/* set fg color */
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	    rxvt_scr_color(r, (unsigned int)(minCOLOR + (arg[i] - 30)),
			   Color_fg);
	    break;
#ifdef TTY_256COLOR
	case 38:
	    if (nargs > i + 2 && arg[i + 1] == 5) {
		rxvt_scr_color(r, (unsigned int)(minCOLOR + arg[i + 2]),
			       Color_fg);
		i += 2;
	    }
	    break;
#endif
	case 39:		/* default fg */
	    rxvt_scr_color(r, Color_fg, Color_fg);
	    break;

	case 40:
	case 41:		/* set bg color */
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	    rxvt_scr_color(r, (unsigned int)(minCOLOR + (arg[i] - 40)),
			   Color_bg);
	    break;
#ifdef TTY_256COLOR
	case 48:
	    if (nargs > i + 2 && arg[i + 1] == 5) {
		rxvt_scr_color(r, (unsigned int)(minCOLOR + arg[i + 2]),
			       Color_bg);
		i += 2;
	    }
	    break;
#endif
	case 49:		/* default bg */
	    rxvt_scr_color(r, Color_bg, Color_bg);
	    break;

#ifndef NO_BRIGHTCOLOR
	case 90:
	case 91:		/* set bright fg color */
	case 92:
	case 93:
	case 94:
	case 95:
	case 96:
	case 97:
	    rxvt_scr_color(r, (unsigned int)(minBrightCOLOR + (arg[i] - 90)),
			   Color_fg);
	    break;
	case 100:
	case 101:		/* set bright bg color */
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107:
	    rxvt_scr_color(r, (unsigned int)(minBrightCOLOR + (arg[i] - 100)),
			   Color_bg);
	    break;
#endif
	}
    }
}
/*}}} */

/*{{{ process Rob Nation's own graphics mode sequences */
/* INTPROTO */
void
rxvt_process_graphics(rxvt_t *r)
{
    unsigned char   ch, cmd = rxvt_cmd_getc(r);

#ifndef RXVT_GRAPHICS
    if (cmd == 'Q') {		/* query graphics */
	rxvt_tt_printf(r, "\033G0\n");	/* no graphics */
	return;
    }
/* swallow other graphics sequences until terminating ':' */
    do
	ch = rxvt_cmd_getc(r);
    while (ch != ':');
#else
    unsigned int    nargs;
    int             args[NGRX_PTS];
    unsigned char  *text = NULL;

    if (cmd == 'Q') {		/* query graphics */
	rxvt_tt_printf(r, "\033G1\n");	/* yes, graphics (color) */
	return;
    }
    for (nargs = 0; nargs < (sizeof(args) / sizeof(args[0])) - 1;) {
	int             neg;

	ch = rxvt_cmd_getc(r);
	neg = (ch == '-');
	if (neg || ch == '+')
	    ch = rxvt_cmd_getc(r);

	for (args[nargs] = 0; isdigit(ch); ch = rxvt_cmd_getc(r))
	    args[nargs] = args[nargs] * 10 + (ch - '0');
	if (neg)
	    args[nargs] = -args[nargs];

	nargs++;
	args[nargs] = 0;
	if (ch != ';')
	    break;
    }

    if ((cmd == 'T') && (nargs >= 5)) {
	int             i, len = args[4];

	text = rxvt_malloc((len + 1) * sizeof(char));

	if (text != NULL) {
	    for (i = 0; i < len; i++)
		text[i] = rxvt_cmd_getc(r);
	    text[len] = '\0';
	}
    }
    rxvt_Gr_do_graphics(r, cmd, nargs, args, text);
#endif
}
/*}}} */

/* ------------------------------------------------------------------------- */

/*{{{ Read and process output from the application */
/* LIBPROTO */
void
rxvt_main_loop(rxvt_t *r)
{
    unsigned char   ch, *str;
    int             nlines, refreshnow;
    struct rxvt_hidden *h = r->h;

    h->cmdbuf_ptr = h->cmdbuf_endp = h->cmdbuf_base;

#if defined(__CYGWIN32__)
    /* once we know the shell is running, send the screen size.  Again! */
    (void)rxvt_cmd_getc(r);	/* wait for something */
    h->cmdbuf_ptr--;		/* unget it - reprocess it below */
    rxvt_tt_winsize(r->cmd_fd, r->TermWin.ncol, r->TermWin.nrow);
#endif

    refreshnow = 0;
    for (;;) {
	ch = rxvt_cmd_getc(r);	/* wait for something */

	if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
	/* Read a text string from the input buffer */
	/*
	 * point `str' to the start of the string,
	 * decrement first since it was post incremented in rxvt_cmd_getc()
	 */
	    for (str = --h->cmdbuf_ptr, nlines = 0;
		 h->cmdbuf_ptr < h->cmdbuf_endp; ) {
		ch = *h->cmdbuf_ptr++;
		if (ch == '\n') {
		    nlines++;
		    h->refresh_count++;
		    if (!(r->Options & Opt_jumpScroll)
			|| (h->refresh_count >= (h->refresh_limit
						 * (r->TermWin.nrow - 1)))) {
			refreshnow = 1;
			break;
		    }
		} else if (ch < ' ' && ch != '\t' && ch != '\r') {
		/* unprintable */
		    h->cmdbuf_ptr--;
		    break;
		}
	    }
	    rxvt_scr_add_lines(r, str, nlines, (h->cmdbuf_ptr - str));
/*
 * If there have been a lot of new lines, then update the screen
 * What the heck I'll cheat and only refresh less than every page-full.
 * the number of pages between refreshes is h->refresh_limit, which
 * is incremented here because we must be doing flat-out scrolling.
 *
 * refreshing should be correct for small scrolls, because of the
 * time-out
 */
	    if (refreshnow) {
		refreshnow = 0;
		if ((r->Options & Opt_jumpScroll)
		    && h->refresh_limit < REFRESH_PERIOD)
		    h->refresh_limit++;
		rxvt_scr_refresh(r, h->refresh_type);
	    }
	} else
	    switch (ch) {
	    default:
		rxvt_process_nonprinting(r, ch);
		break;
	    case C0_ESC:	/* escape char */
		rxvt_process_escape_seq(r);
		break;
	    /* case 0x9b: */	/* CSI */
	    /*  rxvt_process_csi_seq(r); */
	    }
    }
/* NOTREACHED */
}

/*
 * Send printf() formatted output to the command.
 * Only use for small amounts of data.
 */
/* EXTPROTO */
void
rxvt_tt_printf(rxvt_t *r, const char *fmt,...)
{
    va_list         arg_ptr;
    unsigned char   buf[256];

    va_start(arg_ptr, fmt);
    vsprintf((char *)buf, fmt, arg_ptr);
    va_end(arg_ptr);
    rxvt_tt_write(r, buf, (unsigned int)STRLEN(buf));
}

/* ---------------------------------------------------------------------- */
/* Addresses pasting large amounts of data and rxvt hang
 * code pinched from xterm (v_write()) and applied originally to
 * rxvt-2.18 - Hops
 * Write data to the pty as typed by the user, pasted with the mouse,
 * or generated by us in response to a query ESC sequence.
 */
/* EXTPROTO */
void
rxvt_tt_write(rxvt_t *r, const unsigned char *d, unsigned int len)
{
#define MAX_PTY_WRITE 128	/* 1/2 POSIX minimum MAX_INPUT */
    int             riten;
    unsigned int    p;
    unsigned char  *v_buffer,	/* start of physical buffer        */
		   *v_bufstr,	/* start of current buffer pending */
		   *v_bufptr,	/* end of current buffer pending   */
		   *v_bufend;	/* end of physical buffer          */

    if (r->h->v_bufstr == NULL && len > 0) {
	p = (len / MAX_PTY_WRITE + 1) * MAX_PTY_WRITE;
	v_buffer = v_bufstr = v_bufptr = rxvt_malloc(p);
	v_bufend = v_buffer + p;
    } else {
	v_buffer = r->h->v_buffer;
	v_bufstr = r->h->v_bufstr;
	v_bufptr = r->h->v_bufptr;
	v_bufend = r->h->v_bufend;
    }

/*
 * Append to the block we already have.  Always doing this simplifies the
 * code, and isn't too bad, either.  If this is a short block, it isn't
 * too expensive, and if this is a long block, we won't be able to write
 * it all anyway.
 */
    if (len > 0) {
	if (v_bufend < v_bufptr + len) {	/* run out of room */
	    if (v_bufstr != v_buffer) {
	    /* there is unused space, move everything down */
		MEMMOVE(v_buffer, v_bufstr,
			(unsigned int)(v_bufptr - v_bufstr));
		v_bufptr -= v_bufstr - v_buffer;
		v_bufstr = v_buffer;
	    }
	    if (v_bufend < v_bufptr + len) {
	    /* still won't fit: get more space */
	    /* use most basic realloc because an error is not fatal. */
		unsigned int    size = v_bufptr - v_buffer;
		unsigned int    reallocto;

		reallocto = ((size + len) / MAX_PTY_WRITE + 1) * MAX_PTY_WRITE;
		v_buffer = realloc(v_buffer, reallocto);
	    /* save across realloc */
		if (v_buffer) {
		    v_bufstr = v_buffer;
		    v_bufptr = v_buffer + size;
		    v_bufend = v_buffer + reallocto;
		} else {	/* no memory: ignore entire write request */
		    rxvt_print_error("data loss: cannot allocate buffer space");
		    v_buffer = v_bufstr;	/* restore clobbered pointer */
		}
	    }
	}
	if (v_bufend >= v_bufptr + len) {	/* new stuff will fit */
	    MEMCPY(v_bufptr, d, len);
	    v_bufptr += len;
	}
    }
/*
 * Write out as much of the buffer as we can.
 * Be careful not to overflow the pty's input silo.
 * We are conservative here and only write a small amount at a time.
 *
 * If we can't push all the data into the pty yet, we expect write
 * to return a non-negative number less than the length requested
 * (if some data written) or -1 and set errno to EAGAIN,
 * EWOULDBLOCK, or EINTR (if no data written).
 *
 * (Not all systems do this, sigh, so the code is actually
 * a little more forgiving.)
 */

    if ((p = v_bufptr - v_bufstr) > 0) {
	riten = write(r->cmd_fd, v_bufstr, min(p, MAX_PTY_WRITE));
	if (riten < 0)
	    riten = 0;
	v_bufstr += riten;
	if (v_bufstr >= v_bufptr)	/* we wrote it all */
	    v_bufstr = v_bufptr = v_buffer;
    }
/*
 * If we have lots of unused memory allocated, return it
 */
    if (v_bufend - v_bufptr > MAX_PTY_WRITE * 8) { /* arbitrary hysteresis */
    /* save pointers across realloc */
	unsigned int    start = v_bufstr - v_buffer;
	unsigned int    size = v_bufptr - v_buffer;
	unsigned int    reallocto;
	
	reallocto = (size / MAX_PTY_WRITE + 1) * MAX_PTY_WRITE;
	v_buffer = realloc(v_buffer, reallocto);
	if (v_buffer) {
	    v_bufstr = v_buffer + start;
	    v_bufptr = v_buffer + size;
	    v_bufend = v_buffer + reallocto;
	} else {
	/* should we print a warning if couldn't return memory? */
	    v_buffer = v_bufstr - start;	/* restore clobbered pointer */
	}
    }
    r->h->v_buffer = v_buffer;
    r->h->v_bufstr = v_bufstr;
    r->h->v_bufptr = v_bufptr;
    r->h->v_bufend = v_bufend;
}
/*----------------------- end-of-file (C source) -----------------------*/
