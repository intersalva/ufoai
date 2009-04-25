/**
 * @file cl_cinematic.h
 * @brief Header file for cinematics
 */

/*
Copyright (C) 2002-2007 UFO: Alien Invasion team.

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

#ifndef CLIENT_CL_CINEMATIC_H

typedef struct {
	int				vr[256];
	int				ug[256];
	int				vg[256];
	int				ub[256];
	int				yy[256];
} yuvTable_t;

enum {
	CINEMATIC_TYPE_ROQ,
	CINEMATIC_TYPE_OGM
};

typedef struct {
	char			name[MAX_QPATH];	/**< virtuell filesystem path with file suffix */

	qboolean		replay;	/**< autmatically replay in endless loop */
	qboolean		inMenu;	/**< is this cinematic shown in a menu node? */
	int				x, y, w, h; /**< for drawing in the menu maybe */

	qboolean		noSound;	/**< no sound while playing the cinematic */
	yuvTable_t		yuvTable;

	int cinematicType;
} cinematic_t;

extern cinematic_t cin;

void CIN_StopCinematic(void);
void CIN_PlayCinematic(const char *name);
void CIN_Shutdown(void);
void CIN_Init(void);
void CIN_SetParameters(int x, int y, int w, int h, int cinStatus, qboolean noSound);
void CIN_RunCinematic(void);

typedef enum {
	CIN_STATUS_NONE,	/**< not playing */
	CIN_STATUS_FULLSCREEN,	/**< fullscreen cinematic */

	/* don't stop running sounds for these - but the above */
	CIN_STATUS_MENU		/**< cinematic inside a menu node */
} cinStatus_t;

#endif /* CLIENT_CL_CINEMATIC_H */
