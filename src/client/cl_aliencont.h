/**
 * @file cl_aliencont.h
 * @brief Header file for alien containment stuff
 */

/*
Copyright (C) 2002-2006 UFO: Alien Invasion team.

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

#ifndef CLIENT_CL_ALIENCONT_H
#define CLIENT_CL_ALIENCONT_H

#define MAX_CARGO		256
#define MAX_ALIENCONT_CAP	512

typedef enum {		/* types of aliens */
	AL_ORTNOK,
	AL_TAMAN,
	AL_SHEVAAR,
	AL_FLYER,
	AL_UNKNOWN	/* dummy, to get all */
} alienType_t;

typedef enum {		/* specializations of aliens */
	AS_PILOT,
	AS_GUNNER,
	AS_HARVESTER,
	AS_SOLDIER
} alienSpec_t;

typedef enum {		/* cases of alien amount calculation */
	AL_RESEARCH,
	AL_KILL
} alienCalcType_t;

typedef struct aliensCont {
	int idx;			/* self link */
	char alientype[MAX_VAR];	/* type of alien */ /* FIXME: alienType_t here */
	int state;			/* dead or alive */
	int amount;			/* amount */
} aliensCont_t;

typedef struct aliensTmp {
	char alientype[MAX_VAR];	/* type of alien */
	int state;			/* dead or alive */
	int amount;			/* amount */
} aliensTmp_t;

/**
 * Collecting aliens functions
 */

void AL_FillInContainment(void);
char *AL_AlienTypeToName(alienType_t type);
void CL_CollectingAliens(void);
void AL_AddAliens(void);
void AL_CountAll(void);
void AL_RemoveAliens(alienType_t alientype, int amount, alienCalcType_t action);

/**
 * Menu functions
 */

#endif /* CLIENT_CL_ALIENCONT_H */

