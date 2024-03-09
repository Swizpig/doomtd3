/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2023-2024 Frenkel Smeijers
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
 *      Video hardware specific code
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "r_defs.h"
#include "r_main.h"

void I_InitGraphicsHardwareSpecificCode(void);
void I_SetPalette(int8_t pal);
void I_FinishUpdate(void);


void R_DrawColumn (const draw_column_vars_t *dcvars);
void R_DrawColumnFlat(int16_t texture, const draw_column_vars_t *dcvars);
void R_DrawFuzzColumn (const draw_column_vars_t *dcvars);

#if !defined FLAT_SPAN
void R_DrawSpan(uint16_t y, uint16_t x1, uint16_t x2, const draw_span_vars_t *dsvars);
#endif


void V_DrawBackground(void);
void V_DrawRaw(int16_t num, uint16_t offset);
void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch);


#endif
