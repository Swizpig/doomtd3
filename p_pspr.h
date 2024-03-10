/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023, 2024 by
 *  Frenkel Smeijers
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
 *  Sprite animation.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_PSPR__
#define __P_PSPR__

/* Basic data types.
 * Needs fixed point, and BAM angles. */

#include "m_fixed.h"
#include "tables.h"

/* Needs to include the precompiled sprite animation tables.
 *
 * Header generated by multigen utility.
 * This includes all the data for thing animation,
 * i.e. the Thing Atrributes table and the Frame Sequence table.
 */

#include "info.h"


/*
 * Overlay psprites are scaled shapes
 * drawn directly on the view screen,
 * coordinates are given for a 320*200 view screen.
 */

typedef enum
{
  ps_weapon,
  ps_flash,
  NUMPSPRITES
} psprnum_t;

typedef struct
{
  const state_t *state;       /* a NULL state means not active */
  int16_t     tics;
  int16_t sx;
  fixed_t sy;
} pspdef_t;


struct player_s;
weapontype_t P_SwitchWeapon(struct player_s *player);
boolean P_CheckAmmo(struct player_s *player);
void P_SetupPsprites(struct player_s *curplayer);
void P_MovePsprites(struct player_s *curplayer);
void P_DropWeapon(struct player_s *player);

weapontype_t P_WeaponCycleUp(struct player_s *player);
weapontype_t P_WeaponCycleDown(struct player_s *player);


void A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_ReFire();
void A_FirePistol();
void A_Light1();
void A_FireShotgun();
void A_Light2();


#endif
