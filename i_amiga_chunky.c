/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Code specific to the Amiga 500
 *
 *-----------------------------------------------------------------------------*/

#if defined __VBCC__
#include <graphics/gfxbase.h>
#endif

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "compiler.h"

#include "d_main.h"
#include "i_system.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define HORIZONTAL_RESOLUTION_LO	320
#define HORIZONTAL_RESOLUTION_HI	640

#if !defined HORIZONTAL_RESOLUTION
#define HORIZONTAL_RESOLUTION HORIZONTAL_RESOLUTION_HI
#endif

#define PLANEWIDTH			 		(HORIZONTAL_RESOLUTION/8)


#if defined __VBCC__

#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO
#define DDFSTRT_VALUE	0x0038
#define DDFSTOP_VALUE	0x00d0
#define BPLCON0_VALUE	0x1000
#else
#define DDFSTRT_VALUE	0x003c
#define DDFSTOP_VALUE	0x00d4
#define BPLCON0_VALUE	0x9000
#endif

#else
	
#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO
#define DDFSTRT_VALUE	0x0038
#define DDFSTOP_VALUE	0x00d0
#define BPLCON0_VALUE	0b0001000000000000
#else
#define DDFSTRT_VALUE	0x003c
#define DDFSTOP_VALUE	0x00d4
#define BPLCON0_VALUE	0b1001000000000000
#endif

#endif

#define DW	(HORIZONTAL_RESOLUTION/HORIZONTAL_RESOLUTION_LO)

#if defined VERTICAL_RESOLUTION_DOUBLED
#define DH	2
#else
#define DH	1
#endif

extern struct GfxBase *GfxBase;
extern struct Custom custom;

extern const int16_t CENTERY;

static uint16_t viewwindowtop;
static uint16_t statusbartop;
static uint8_t *_s_viewwindow;
static uint8_t *_s_statusbar;

static uint32_t screenpaget;
static uint8_t *screenpage;

static boolean isGraphicsModeSet = false;


#define FMODE	0x1fc

#define DDFSTRT	0x092
#define DDFSTOP	0x094

#define DIWSTRT				0x08e
#define DIWSTOP				0x090
#define DIWSTRT_VALUE		0x2c81
#define DIWSTOP_VALUE_PAL	0x2cc1
#define DIWSTOP_VALUE_NTSC	0xf4c1

#define BPLCON0	0x100

#define BPL1MOD	0x108

#define COLOR00	0x180
#define COLOR01	0x182

#define BPL1PTH	0x0e0
#define BPL1PTL	0x0e2
  

#define COPLIST_IDX_DIWSTOP_VALUE 9
#define COPLIST_IDX_COLOR00_VALUE 15
#define COPLIST_IDX_BPL1PTH_VALUE 19
#define COPLIST_IDX_BPL1PTL_VALUE 21

#define COL_START 25
#define COL_STRIDE (32*2)

#define SIZEOF_COPLIST_PREAMBLE (11*2)
#define SIZEOF_COPLIST_POSTSCRIPT (2*2)

static uint16_t __chip coplist_preamble[] = {
	FMODE,   0,
	DDFSTRT, DDFSTRT_VALUE,
	DDFSTOP, DDFSTOP_VALUE,
	DIWSTRT, DIWSTRT_VALUE,
	DIWSTOP, DIWSTOP_VALUE_PAL,
	BPLCON0, BPLCON0_VALUE,
	BPL1MOD, 0,

	COLOR00, 0x000,
	COLOR01, 0xfff,

	BPL1PTH, 0,
	BPL1PTL, 0,
};

static uint16_t __chip coplist_postscript[] = {
	0xffff, 0xfffe, // COP_WAIT_END
	0xffff, 0xfffe  // COP_WAIT_END
};

static uint16_t __chip coplist_row[] = {	
	0x5C53,0xFFFE, // wait to row start
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,
	COLOR00, 0x00F,
	COLOR00, 0x0F0,

	COLOR00, 0x000
};

static uint16_t* coplist;

static uint16_t __chip coplist_a[SIZEOF_COPLIST_PREAMBLE + SIZEOF_COPLIST_POSTSCRIPT + COL_STRIDE * VIEWWINDOWHEIGHT];
static uint16_t __chip coplist_b[SIZEOF_COPLIST_PREAMBLE + SIZEOF_COPLIST_POSTSCRIPT + COL_STRIDE * VIEWWINDOWHEIGHT];

static const int16_t colors[14] =
{
	0x000,													// normal
	0x800, 0x900, 0xa00, 0xb00, 0xc00, 0xd00, 0xe00, 0xf00,	// red
	0x440, 0x550, 0x660, 0x770,								// yellow
	0x070													// green
};

// why does this crash if it's not chip?
static uint16_t __chip amiga_palette[] = {
0x000,0x110,0x100,0x444,0xfff,0x111,0x111,0x000,0x000,0x231,0x220,0x110,0x010,0x432,0x432,0x321,
0xfbb,0xfaa,0xfaa,0xe99,0xe88,0xd88,0xd77,0xd77,0xc66,0xc66,0xb55,0xb55,0xb44,0xa44,0xa33,0xa33,
0x933,0x922,0x822,0x822,0x811,0x711,0x711,0x711,0x600,0x600,0x500,0x500,0x500,0x400,0x400,0x400,
0xfed,0xfed,0xfdc,0xfdb,0xfcb,0xfca,0xfb9,0xfb9,0xfb8,0xfa7,0xea7,0xe96,0xd96,0xd85,0xc85,0xc74,
0xb74,0xb74,0xa64,0xa63,0x963,0x853,0x853,0x752,0x742,0x642,0x542,0x531,0x431,0x321,0x321,0x220,
0xeee,0xeee,0xddd,0xddd,0xddd,0xccc,0xccc,0xbbb,0xbbb,0xbbb,0xaaa,0xaaa,0x999,0x999,0x999,0x888,
0x888,0x777,0x777,0x666,0x666,0x666,0x555,0x555,0x444,0x444,0x444,0x333,0x333,0x222,0x222,0x222,
0x7f6,0x6e6,0x6d5,0x5c5,0x5b4,0x5a4,0x493,0x493,0x382,0x372,0x262,0x251,0x141,0x130,0x120,0x010,
0xba8,0xb98,0xa97,0xa87,0x986,0x976,0x976,0x875,0x865,0x764,0x754,0x654,0x653,0x543,0x543,0x532,
0x986,0x875,0x864,0x753,0x653,0x542,0x432,0x431,0x776,0x675,0x664,0x564,0x553,0x443,0x342,0x332,
0xff7,0xed5,0xdb4,0xc92,0xa71,0x951,0x840,0x720,0xfff,0xfdd,0xfbb,0xf99,0xf77,0xf55,0xf33,0xf11,
0xf00,0xe00,0xe00,0xd00,0xc00,0xb00,0xb00,0xa00,0x900,0x800,0x700,0x700,0x600,0x500,0x400,0x400,
0xeef,0xccf,0xaaf,0x88f,0x77f,0x55f,0x33f,0x11f,0x00f,0x00e,0x00c,0x00b,0x009,0x008,0x006,0x005,
0xfff,0xfed,0xfdb,0xfc9,0xfb7,0xfa5,0xf83,0xf71,0xf71,0xe60,0xd60,0xd50,0xc50,0xc40,0xb40,0xa40,
0xfff,0xffd,0xffb,0xff8,0xff6,0xff4,0xff2,0xff0,0xa30,0x930,0x920,0x820,0x432,0x421,0x321,0x210,
0x005,0x004,0x003,0x002,0x002,0x001,0x000,0x000,0xf94,0xfe4,0xf7f,0xf0f,0xc0c,0x909,0x606,0xa66
};


static void I_UploadNewPalette(int8_t pal)
{
	coplist[COPLIST_IDX_COLOR00_VALUE] = colors[pal];
}


void I_InitGraphics(void)
{
	LoadView(NULL);
	WaitTOF();
	WaitTOF();

	boolean pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;
	int16_t screenHeightAmiga;

	memcpy(coplist_a, coplist_preamble, sizeof(coplist_preamble));
	memcpy(coplist_b, coplist_preamble, sizeof(coplist_preamble));
	
	for (int y=0; y<VIEWWINDOWHEIGHT; y++)
	{
		memcpy(coplist_a + SIZEOF_COPLIST_PREAMBLE + y*COL_STRIDE, coplist_row, sizeof(coplist_row));
		memcpy(coplist_b + SIZEOF_COPLIST_PREAMBLE + y*COL_STRIDE, coplist_row, sizeof(coplist_row));
		// add wait til row start
		coplist_a[SIZEOF_COPLIST_PREAMBLE + y*COL_STRIDE] = 0x5c53 + (y<<8);
		coplist_b[SIZEOF_COPLIST_PREAMBLE + y*COL_STRIDE] = 0x5c53 + (y<<8);
	}
	memcpy(coplist_a + SIZEOF_COPLIST_PREAMBLE + VIEWWINDOWHEIGHT*COL_STRIDE, coplist_postscript, sizeof(coplist_postscript));
	memcpy(coplist_b + SIZEOF_COPLIST_PREAMBLE + VIEWWINDOWHEIGHT*COL_STRIDE, coplist_postscript, sizeof(coplist_postscript));

	if (pal) {
		coplist_a[COPLIST_IDX_DIWSTOP_VALUE] = coplist_b[COPLIST_IDX_DIWSTOP_VALUE] = DIWSTOP_VALUE_PAL;
		screenHeightAmiga = 256;
	} else {
		coplist_a[COPLIST_IDX_DIWSTOP_VALUE] = coplist_b[COPLIST_IDX_DIWSTOP_VALUE] = DIWSTOP_VALUE_NTSC;
		screenHeightAmiga = 200;
	}

	uint8_t *screenpage0 = Z_MallocStatic(PLANEWIDTH * screenHeightAmiga * 2);	
	memset(screenpage0, 0, PLANEWIDTH * screenHeightAmiga * 2);

	uint8_t *screenpage1 = screenpage0 + PLANEWIDTH * screenHeightAmiga;
	screenpaget = (uint32_t)screenpage0 + (uint32_t)screenpage1;
	screenpage = screenpage1;

	uint32_t addr = (uint32_t) screenpage1;
	coplist_a[COPLIST_IDX_BPL1PTH_VALUE] = addr >> 16;
	coplist_a[COPLIST_IDX_BPL1PTL_VALUE] = addr;

	coplist_b[COPLIST_IDX_BPL1PTH_VALUE] = addr >> 16;
	coplist_b[COPLIST_IDX_BPL1PTL_VALUE] = addr;

	I_UploadNewPalette(0);

	coplist = coplist_a;

	custom.dmacon = 0x0020;
	custom.cop1lc = (uint32_t) coplist_b;

	viewwindowtop = ((PLANEWIDTH - VIEWWINDOWWIDTH)      / 2) + ((screenHeightAmiga - (VIEWWINDOWHEIGHT * DH + ST_HEIGHT)) / 2) * PLANEWIDTH;
	statusbartop  = ((PLANEWIDTH - SCREENWIDTH * DW / 8) / 2) + ((screenHeightAmiga - (VIEWWINDOWHEIGHT * DH + ST_HEIGHT)) / 2) * PLANEWIDTH + VIEWWINDOWHEIGHT * DH * PLANEWIDTH;
	_s_viewwindow = screenpage + viewwindowtop;

	_s_statusbar  = Z_MallocStatic(SCREENWIDTH * ST_HEIGHT);

	OwnBlitter();
	WaitBlit();

#if defined __VBCC__
	custom.bltcon0 = 0x09F0;
#else
	custom.bltcon0 = 0b0000100111110000;
#endif
	custom.bltcon1 = 0;

	custom.bltamod = PLANEWIDTH - SCREENWIDTH * DW / 8;
	custom.bltdmod = PLANEWIDTH - SCREENWIDTH * DW / 8;

	custom.bltafwm = 0xffff;
	custom.bltalwm = 0xffff;

	custom.bltbdat = 0xffff;
	custom.bltcdat = 0xffff;

	isGraphicsModeSet = true;
}


static void I_ShutdownGraphics(void)
{
	if (isGraphicsModeSet)
	{
		DisownBlitter();
		LoadView(((struct GfxBase *) GfxBase)->ActiView);
		WaitTOF();
		WaitTOF();
		custom.cop1lc = (uint32_t) ((struct GfxBase *) GfxBase)->copinit;
		RethinkDisplay();
	}
}


static int8_t newpal;


void I_SetPalette(int8_t pal)
{
	newpal = pal;
}



#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO

#define B0 0
#define B1 1

static const uint8_t VGA_TO_BW_LUT[256] =
{
	B0, B0, B0, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B0, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B1, B1, B1, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#else

#define B0 0
#define B1 1
#define B2 3

static const uint8_t VGA_TO_BW_LUT_e[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#define B0 0
#define B1 2
#define B2 3

static const uint8_t VGA_TO_BW_LUT_o[256] =
{
	B0, B0, B0, B1, B2, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2, B2,
	B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1, B1,
	B1, B1, B1, B1, B1, B1, B0, B0, B1, B1, B1, B1, B1, B1, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B2, B2, B2, B2, B2, B2, B1, B1,
	B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0, B0, B0, B0, B0,
	B2, B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B1, B1, B1, B1,
	B2, B2, B2, B2, B2, B2, B2, B2, B1, B1, B1, B0, B0, B0, B0, B0,
	B0, B0, B0, B0, B0, B0, B0, B0, B2, B2, B2, B1, B1, B0, B0, B1
};

#undef B0
#undef B1
#undef B2

#endif


#define NO_PALETTE_CHANGE 100

static uint16_t st_needrefresh = 0;

void I_FinishUpdate(void)
{
	// palette
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// status bar
	if (st_needrefresh)
	{
		st_needrefresh--;

		if (st_needrefresh)
		{
			uint8_t *src = _s_statusbar;
			uint8_t *dst = screenpage + statusbartop;
#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO
			for (uint_fast8_t y = 0; y < ST_HEIGHT; y++) {
				for (uint_fast8_t x = 0; x < SCREENWIDTH * DW / 8; x++) {
					uint8_t c =    VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					c = (c << 1) | VGA_TO_BW_LUT[*src++];
					*dst++ = c;
				}

				dst += PLANEWIDTH - SCREENWIDTH * DW / 8;
			}
#else
			for (uint_fast8_t y = 0; y < ST_HEIGHT / 2; y++) {
				for (uint_fast8_t x = 0; x < SCREENWIDTH * DW / 8; x++) {
					uint8_t c =    VGA_TO_BW_LUT_e[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_e[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_e[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_e[*src++];
					*dst++ = c;
				}

				dst += PLANEWIDTH - SCREENWIDTH * DW / 8;

				for (uint_fast8_t x = 0; x < (SCREENWIDTH * DW / 8); x++) {
					uint8_t c =    VGA_TO_BW_LUT_o[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_o[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_o[*src++];
					c = (c << 2) | VGA_TO_BW_LUT_o[*src++];
					*dst++ = c;
				}

				dst += PLANEWIDTH - SCREENWIDTH * DW / 8;
			}
#endif
		}
		else
		{
			WaitBlit();

			custom.bltapt = (uint8_t*)(screenpaget - (uint32_t)screenpage) + statusbartop;
			custom.bltdpt = screenpage + statusbartop;

			custom.bltsize = (ST_HEIGHT << 6) | ((SCREENWIDTH * DW / 8) / 2);
		}
	}

	// page flip
	uint32_t addr = (uint32_t) screenpage;
	coplist[COPLIST_IDX_BPL1PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL1PTL_VALUE] = addr;
	screenpage = (uint8_t*)(screenpaget - (uint32_t)screenpage);
	_s_viewwindow = screenpage + viewwindowtop;

	custom.cop1lc = (uint32_t)coplist;
	if (coplist == coplist_a)
	{
		coplist = coplist_b;
		
	}
	else
	{
		coplist = coplist_a;
	}
}


void R_InitColormaps(void)
{
	fullcolormap = W_GetLumpByName("COLORMAP"); // Never freed
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

void R_DrawColumn(const draw_column_vars_t *dcvars)
{
	const int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *source = dcvars->source;

	const uint8_t *nearcolormap = dcvars->colormap;

	//uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH * DH) + dcvars->x;

	const uint16_t fracstep = (dcvars->iscale >> COLEXTRABITS);
	uint16_t frac = (dcvars->texturemid + (dcvars->yl - CENTERY) * dcvars->iscale) >> COLEXTRABITS;

	int16_t l = count >> 4;

#if defined VERTICAL_RESOLUTION_DOUBLED
	uint8_t c;
	while (l--)
	{
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;

		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;

		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;

		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 14: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 13: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 12: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 11: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case 10: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  9: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  8: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  7: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  6: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  5: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  4: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  3: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  2: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c; dest += PLANEWIDTH * 2; frac += fracstep;
		case  1: c = nearcolormap[source[frac>>COLBITS]]; *dest = *(dest + PLANEWIDTH) = c;
	}
#else

	// let the compiler to the unrolling
	uint16_t* dest = coplist + COL_START + 2*dcvars->x + dcvars->yl * COL_STRIDE;
	for (l=0; l<count; l++)
	{
		*dest = amiga_palette[source[frac>>COLBITS]]; dest += COL_STRIDE; frac += fracstep;
	}

#endif
}


void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	//uint8_t *dest = _s_viewwindow + (dcvars->yl * PLANEWIDTH * DH) + dcvars->x;

	uint16_t* dest = coplist + COL_START + 2*dcvars->x + COL_STRIDE*dcvars->yl;

	while (count--)
	{
#if defined VERTICAL_RESOLUTION_DOUBLED
		*dest = *(dest + PLANEWIDTH) = color;
#else
		*dest = amiga_palette[texture&0xFF];
#endif
		dest += COL_STRIDE;
	}
}


#define FUZZOFF (PLANEWIDTH)
#define FUZZTABLE 50

static const int8_t fuzzoffset[FUZZTABLE] =
{
	FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t dc_yl = dcvars->yl;
	int16_t dc_yh = dcvars->yh;

	// Adjust borders. Low...
	if (dc_yl <= 0)
		dc_yl = 1;

	// .. and high.
	if (dc_yh >= VIEWWINDOWHEIGHT - 1)
		dc_yh = VIEWWINDOWHEIGHT - 2;

	int16_t count = (dc_yh - dc_yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *nearcolormap = &fullcolormap[6 * 256];

	uint8_t *dest = _s_viewwindow + (dc_yl * PLANEWIDTH * DH) + dcvars->x;

	static int16_t fuzzpos = 0;

	do
	{
#if defined VERTICAL_RESOLUTION_DOUBLED
		*dest = *(dest + PLANEWIDTH) = nearcolormap[dest[fuzzoffset[fuzzpos] * DH]];
#else
		*dest = nearcolormap[dest[fuzzoffset[fuzzpos] * DH]];
#endif
		dest += PLANEWIDTH * DH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t *lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		memcpy(&_s_statusbar[offset - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_statusbar[offset - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH]);
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
	{
		ST_doRefresh();
		st_needrefresh = 2; //2 screen pages
	}
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t *patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte *desttop = _s_statusbar + (y * SCREENWIDTH) + x - (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t *column = (const column_t*)((const byte*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte*)column + 3;
			byte *dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			while (count--)
			{
				*dest = *source++;
				dest += SCREENWIDTH;
			}

			column = (const column_t*)((const byte*)column + column->length + 4);
		}
	}
}


segment_t I_ZoneBase(uint32_t *size)
{
	uint32_t paragraphs = 550 * 1024L / PARAGRAPH_SIZE;
	uint8_t *ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs--;
		ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	}

	// align ptr
	uint32_t m = (uint32_t) ptr;
	if ((m & (PARAGRAPH_SIZE - 1)) != 0)
	{
		paragraphs--;
		while ((m & (PARAGRAPH_SIZE - 1)) != 0)
			m = (uint32_t) ++ptr;
	}

	*size = paragraphs * PARAGRAPH_SIZE;
	printf("%lu bytes allocated for zone\n", *size);
	return D_FP_SEG(ptr);
}


segment_t I_ZoneAdditional(uint32_t *size)
{
	*size = 0;
	return 0;
}


static clock_t starttime;

#if __VBCC__

/* 

Ugh - "The clock() function always returns -1. This is correct, according to the C
standard, because on AmigaOS it is not possible to obtain the time used by the
calling process"

This, I suspect, was causing div-by-zero in the FPS calcs

*/

void I_StartClock(void)
{
	uint32_t seconds;
	uint32_t micros;

	CurrentTime(&seconds, &micros);
	starttime = (clock_t)seconds;
}


uint32_t I_EndClock(void)
{
	uint32_t seconds;
	uint32_t micros;

	CurrentTime(&seconds, &micros);
	clock_t endtime = (clock_t)seconds;

	return (endtime - starttime) * TICRATE;
}

#else
void I_StartClock(void)
{
	starttime = clock();
}


uint32_t I_EndClock(void)
{
	clock_t endtime = clock();

	return ((endtime - starttime) * TICRATE) / CLOCKS_PER_SEC;
}

#endif

void I_Error2(const char *error, ...)
{
	I_UploadNewPalette(1);
	va_list argptr;

	I_ShutdownGraphics();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


int main(int argc, const char * const * argv)
{
	UNUSED(argc);
	UNUSED(argv);

	D_DoomMain();
	return 0;
}
