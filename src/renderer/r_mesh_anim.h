/**
 * @file r_mesh_anim.h
 */

/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef R_MESH_ANIM_H
#define R_MESH_ANIM_H

void R_AnimAppend(animState_t * as, struct model_s *mod, const char *name);
void R_AnimChange(animState_t * as, struct model_s *mod, const char *name);
void R_AnimRun(animState_t * as, struct model_s *mod, int msec);
const char *R_AnimGetName(animState_t * as, struct model_s *mod);
void R_InterpolateTransform(animState_t *as, int numframes, const float *tag, float *interpolated);

#endif
