// cl_team.c -- team management, name generation and parsing


#include "client.h"

char *teamSkinNames[NUM_TEAMSKINS] =
{
	"Urban",
	"Jungle",
	"Desert",
	"Arctic"
};

/*
======================
CL_GiveNameCmd
======================
*/
void CL_GiveNameCmd( void )
{
	char	*name;
	int		i, j, num;

	if ( Cmd_Argc() < 3 )
	{
		Com_Printf( "Usage: givename <gender> <category> [num]\n" );
		return;
	}

	// get gender
	for ( i = 0; i < NAME_LAST; i++ )
		if ( !Q_strcmp( Cmd_Argv(1), name_strings[i] ) )
			break;

	if ( i == NAME_LAST )
	{
		Com_Printf("'%s' isn't a gender! (male and female are)\n", Cmd_Argv(1) );
		return;
	}

	if ( Cmd_Argc() > 3 )
	{
		num = atoi( Cmd_Argv(3) );
		if ( num < 1 ) num = 1;
		if ( num > 20 ) num = 20;
	} else num = 1;

	for ( j = 0; j < num; j++ )
	{
		// get first name
		name = Com_GiveName( i, Cmd_Argv(2) );
		if ( !name )
		{
			Com_Printf( "No first name in category '%s'\n", Cmd_Argv(2) );
			return;
		}
		Com_Printf( name );

		// get last name
		name = Com_GiveName( i + LASTNAME, Cmd_Argv(2) );
		if ( !name )
		{
			Com_Printf( "\nNo last name in category '%s'\n", Cmd_Argv(2) );
			return;
		}

		// print out name
		Com_Printf( " %s\n", name );
	}
}

// initialized in CL_ResetTeams

// TODO: Parsing from file
rank_t ranks[MAX_RANKS];	// Global list of all ranks defined in medals.ufo.
int numRanks;			// The number of entries in the list above.


/*======================
CL_GenerateCharacter
======================*/
void CL_GenerateCharacter( char *team, base_t* base )
{
	character_t *chr;

	// check for too many characters
	if ( base->numWholeTeam >= (int)cl_numnames->value )
		return;

	// reset character
	chr = &base->wholeTeam[base->numWholeTeam];
	memset( chr, 0, sizeof( character_t ) );

	// link inventory
	chr->inv = &base->teamInv[base->numWholeTeam];
	Com_DestroyInventory( chr->inv );

	// get ucn
	chr->ucn = base->nextUCN++;

	// new attributes
	Com_CharGenAbilitySkills (chr, 0, 75, 0, 75);

	// get model and name
	chr->skin = Com_GetModelAndName( team, chr->path, chr->body, chr->head, chr->name );
	Cvar_ForceSet( va( "mn_name%i", base->numWholeTeam ), chr->name );

	// Starting rank is the first one in the list (as defined in medals.ufo)
	/*if ( numRanks == 0) {								// DEBUG
		Q_strncpyz( ranks[0].name, "Rookie", MAX_MEDALTITLE); 	// DEBUG
		numRanks = 1;								// DEBUG
	}											// DEBUG
	*/
	chr->rank = &ranks[0];

	base->numWholeTeam++;
}


/*
======================
CL_ResetCharacters
======================
*/
void CL_ResetCharacters( base_t* base )
{
	int	i;

	// reset inventory data
	for ( i = 0; i < MAX_WHOLETEAM; i++ )
	{
		Com_DestroyInventory( &base->teamInv[i] );
//		base->teamInv[i].c[csi.idFloor] = base->equipment;
		base->wholeTeam[i].inv = &base->teamInv[i];
	}

	// reset hire info
	Cvar_ForceSet( "cl_selected", "0" );
	base->numWholeTeam = 0;
	base->numOnTeam = 0;
	base->numHired = 0;
	base->teamMask = 0;
}


/*
======================
CL_GenerateNamesCmd
======================
*/
void CL_GenerateNamesCmd( void )
{
	Cbuf_AddText( "disconnect\ngame_exit\n" );
}


/*
======================
CL_ChangeNameCmd
======================
*/
void CL_ChangeNameCmd( void )
{
	int sel;
	sel = cl_selected->value;

	// maybe called without base initialized or active
	if ( !baseCurrent )
		return;

	if ( sel >= 0 && sel < baseCurrent->numWholeTeam )
		Q_strncpyz( baseCurrent->wholeTeam[sel].name, Cvar_VariableString( "mn_name" ), MAX_VAR );
}


/*
======================
CL_ChangeSkinCmd
======================
*/
void CL_ChangeSkinCmd( void )
{
	int sel, newSkin, i;
	sel = cl_selected->value;
	if ( sel >= 0 && sel < baseCurrent->numWholeTeam )
	{
		newSkin = (int)Cvar_VariableValue( "mn_skin" ) + 1;
		if ( newSkin >= NUM_TEAMSKINS || newSkin < 0 ) newSkin = 0;

		baseCurrent->curTeam[sel].skin = newSkin;
		for ( i = 0; i < baseCurrent->numWholeTeam; i++ )
			if ( baseCurrent->wholeTeam[i].ucn == baseCurrent->curTeam[sel].ucn )
			{
				baseCurrent->wholeTeam[i].skin = newSkin;
				break;
			}

		Cvar_SetValue( "mn_skin", newSkin );
		Cvar_Set( "mn_skinname", _(teamSkinNames[newSkin]) );
	}
}

/*
======================
CL_TeamCommentsCmd
======================
*/
void CL_TeamCommentsCmd( void )
{
	char	comment[MAX_VAR];
	FILE	*f;
	int		i;

	for ( i = 0; i < 8; i++ )
	{
		// open file
		f = fopen( va( "%s/save/team%i.mpt", FS_Gamedir(), i ), "rb" );
		if ( !f )
		{
			Cvar_Set( va( "mn_slot%i", i ), "" );
			continue;
		}

		// read data
		fread( comment, 1, MAX_VAR, f );
		Cvar_Set( va( "mn_slot%i", i ), comment );
		fclose( f );
	}
}


/*
======================
CL_ItemDescription
======================
*/
char	itemText[MAX_MENUTEXTLEN];

void CL_ItemDescription( int item )
{
	objDef_t	*od;

	// select item
	od = &csi.ods[item];
	Cvar_Set( "mn_itemname", _(od->name) );

	// set models
//	if ( od->link == NONE )
	{
		Cvar_Set( "mn_item", od->kurz );
		Cvar_Set( "mn_weapon", "" );
		Cvar_Set( "mn_ammo", "" );
	}
	/*else {
		Cvar_Set( "mn_weapon", od->kurz );
		Cvar_Set( "mn_ammo", csi.ods[od->link].kurz );
		Cvar_Set( "mn_item", "" );
	}*/

	// set description text
	if ( RS_ItemIsResearched(od->kurz)  )
	{
		if ( od->weapon )
		{
			if ( Q_strncmp(od->type, "ammo", 4) )
			{
				Com_sprintf( itemText, MAX_MENUTEXTLEN, va( _("Primary:\t%s\nSecondary:\t%s\nDamage:\t%i / %i\nTime units:\t%i / %i\nRange:\t%1.1f / %1.1f\nSpreads:\t%1.1f / %1.1f\nAmmo:\t%i\n"),
					od->fd[0].name, od->fd[1].name, (int)(od->fd[0].damage[0] * od->fd[0].shots + od->fd[0].spldmg[0]), (int)(od->fd[1].damage[0] * od->fd[1].shots + od->fd[0].spldmg[0]),
					(od->fd[0].time), (od->fd[1].time), (od->fd[0].range / 32.0), (od->fd[1].range / 32.0),
					(od->fd[0].spread[0] + od->fd[0].spread[1])/2, (od->fd[1].spread[0] + od->fd[1].spread[1])/2, (int)(od->ammo)
				) );
			}
			else
			{
				//TODO: Show primary and secondary parameters for this type of ammo here
				Com_sprintf( itemText, MAX_MENUTEXTLEN, va( _("Damage:\t%i / %i\nTime units:\t%i / %i\nRange:\t%1.1f / %1.1f\nSpreads:\t%1.1f / %1.1f\nAmmo:\t%i\n"),
					(int)(od->fd[0].damage[0] * od->fd[0].shots + od->fd[0].spldmg[0]), (int)(od->fd[1].damage[0] * od->fd[1].shots + od->fd[0].spldmg[0]),
					(od->fd[0].time), (od->fd[1].time), (od->fd[0].range / 32.0), (od->fd[1].range / 32.0),
					(od->fd[0].spread[0] + od->fd[0].spread[1])/2, (od->fd[1].spread[0] + od->fd[1].spread[1])/2, (int)(od->ammo)
				) );
			}
		}
		// just an item
		// only primary definition
		else
		{
			Com_sprintf( itemText, MAX_MENUTEXTLEN, va( _("Action:\t%s\nTime units:\t%i\nRange:\t%1.1f\n"),
				od->fd[0].name, od->fd[0].time, (od->fd[0].range / 32.0)
			) );
		}
		menuText[TEXT_STANDARD] = itemText;
	} else {
		Com_sprintf( itemText, MAX_MENUTEXTLEN, _("Unknown - need to research this") );
		menuText[TEXT_STANDARD] = itemText;
	}
	// else menuText[TEXT_STANDARD] = NULL; // TODO: should not be needed anymore.
}


/*
======================
CL_AddWeaponAmmo
======================
*/
item_t CL_AddWeaponAmmo( equipDef_t *ed, int type )
{
	item_t item;
	int i;

	item.a = 0;
	item.m = NONE;
	item.t = NONE;
	if ( ed->num[type] <= 0 )
		return item;

	ed->num[type]--;
	item.t = type;

	if ( !csi.ods[type].reload )
	{
		item.a = csi.ods[type].ammo;
		item.m = type;
		return item;
	}

	item.a = 0;
	item.m = NONE;
	// Search for any complete clips
	for ( i = 0; i < csi.numODs; i++ )
	{
		if ( csi.ods[i].link == type )
		{
			item.m = i;
			if ( ed->num[i] > 0 )
			{
				ed->num[i]--;
				item.a = csi.ods[type].ammo;
				return item;
			}
		}
	}
	// Failed to find a complete clip - see if there's any loose ammo
	for ( i = 0; i < csi.numODs; i++ )
	{
		if ( csi.ods[i].link == type && ed->num_loose[i] > 0)
		{
			if ( item.m != NONE && ed->num_loose[i] > item.a )
			{
				// We previously found some ammo, but we've now found other
				// loose ammo of a different (but appropriate) type with
				// more bullets.  Put the previously found ammo back, so
				// we'll take the new type.
				ed->num_loose[item.m] = item.a;
				item.m = NONE;
			}
			if ( item.m == NONE )
			{
				// Found some loose ammo to load the weapon with
				item.a = ed->num_loose[i];
				ed->num_loose[i] = 0;
				item.m = i;
			}
		}
	}
	return item;
}

/*
======================
CL_CheckInventory
======================
*/
void CL_CheckInventory( equipDef_t *equip )
{
	character_t	*cp;
	invList_t	*ic, *next;
	int p, container;

	// Iterate through in container order (right hand, left hand, belt,
	// armor, backpack) at the top level, i.e. each squad member fills
	// their right hand, then each fills their left hand, etc.  The effect
	// of this is that when things are tight, everyone has the opportunity
	// to get their preferred weapon(s) loaded and in their hands before
	// anyone fills their backpack with spares.  We don't want the first
	// person in the squad filling their backpack with spare ammo leaving
	// others with unloaded guns in their hands.
	for ( container = 0; container < csi.numIDs; container++ )
	{
		for ( p = 0, cp = baseCurrent->curTeam; p < baseCurrent->numOnTeam; p++, cp++ )
		{
			for ( ic = cp->inv->c[container]; ic; ic = next )
			{
				next = ic->next;
				if ( equip->num[ic->item.t] > 0 )
					ic->item = CL_AddWeaponAmmo( equip, ic->item.t );
				else
					Com_RemoveFromInventory( cp->inv, container, ic->x, ic->y );
			}
		}
	}
}

/*
======================
CL_CleanTempInventory
======================
*/
void CL_CleanTempInventory( void )
{
	int i, k;

	if ( ! baseCurrent )
		return;

	Com_DestroyInventory( &baseCurrent->equipment );
	for ( i = 0; i < MAX_WHOLETEAM; i++ )
		for ( k = 0; k < csi.numIDs; k++ )
			if ( k == csi.idEquip ) baseCurrent->teamInv[i].c[k] = NULL;
			else if ( csi.ids[k].temp ) Com_EmptyContainer( &baseCurrent->teamInv[i], k );
}

/*
======================
CL_GenerateEquipmentCmd
======================
*/
void CL_GenerateEquipmentCmd( void )
{
	equipDef_t	*ed;
	equipDef_t	unused;
	char	*name;
	int		i, p;
	int		x, y;

	assert(baseCurrent);

	if ( !baseCurrent->numHired )
	{
		MN_PopMenu( false );
		return;
	}

	// clean equipment
	CL_CleanTempInventory();

	// store hired names
	Cvar_ForceSet( "cl_selected", "0" );
	baseCurrent->numOnTeam = baseCurrent->numHired;
	baseCurrent->teamMask = baseCurrent->hiredMask;
	for ( i = 0, p = 0; i < baseCurrent->numWholeTeam; i++ )
		if ( baseCurrent->hiredMask & (1 << i) )
		{
			baseCurrent->curTeam[p] = baseCurrent->wholeTeam[i];
			Cvar_ForceSet( va( "mn_name%i", p ), baseCurrent->curTeam[p].name );
			p++;
		}

	for ( ; p < MAX_ACTIVETEAM; p++ )
	{
		Cvar_ForceSet( va( "mn_name%i", p ), "" );
		Cbuf_AddText( va( "equipdisable%i\n", p ) );
	}

	menuInventory = &baseCurrent->teamInv[0];
	selActor = NULL;

	// reset description
	Cvar_Set( "mn_itemname", "" );
	Cvar_Set( "mn_item", "" );
	Cvar_Set( "mn_weapon", "" );
	Cvar_Set( "mn_ammo", "" );
	menuText[TEXT_STANDARD] = NULL;

	if ( !curCampaign )
	{
		// search equipment definition
		name = Cvar_VariableString( "equip" );
		for ( i = 0, ed = csi.eds; i < csi.numEDs; i++, ed++ )
		{
			if ( !Q_strncmp( name, ed->name, MAX_VAR ) )
				break;
		}
		if ( i == csi.numEDs )
		{
			Com_Printf( "Equipment '%s' not found!\n", name );
			return;
		}
		unused = *ed;
	}
	else unused = ccs.eCampaign;

	// manage inventory
	CL_CheckInventory( &unused );

	for ( i = 0; i < csi.numODs; i++ )
		while ( unused.num[i] )
		{
			// 'tiny hack' to add the equipment correctly into buy categories
			baseCurrent->equipment.c[csi.idEquip] = baseCurrent->equipment.c[csi.ods[i].buytype];

			Com_FindSpace( &baseCurrent->equipment, i, csi.idEquip, &x, &y );
			if ( x >= 32 && y >= 16 ) break;
			Com_AddToInventory( &baseCurrent->equipment, CL_AddWeaponAmmo( &unused, i ), csi.idEquip, x, y );

			baseCurrent->equipment.c[csi.ods[i].buytype] = baseCurrent->equipment.c[csi.idEquip];
		}

	baseCurrent->equipment.c[csi.idEquip] = NULL;
}


/*
======================
CL_EquipTypeCmd
======================
*/
void CL_EquipTypeCmd( void )
{
	int		num;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: equip_type <category>\n" );
		return;
	}

	// read and range check
	num = atoi( Cmd_Argv( 1 ) );
	if ( num < 0 && num >= NUM_BUYTYPES )
		return;

	// display new items
	baseCurrent->equipType = num;
	if ( menuInventory ) menuInventory->c[csi.idEquip] = baseCurrent->equipment.c[num];
}

/*
======================
CL_SelectCmd
======================
*/
void CL_SelectCmd( void )
{
	char *command;
	int num;

	// check syntax
	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: team_select <num>\n" );
		return;
	}
	num = atoi( Cmd_Argv(1) );

	// change highlights
	command = Cmd_Argv(0);
	*strchr( command, '_' ) = 0;

	if ( !Q_strncmp( command, "equip", 5 ) )
	{
		if ( !baseCurrent || num >= baseCurrent->numOnTeam ) return;
		if ( menuInventory && menuInventory != baseCurrent->curTeam[num].inv )
		{
			baseCurrent->curTeam[num].inv->c[csi.idEquip] = menuInventory->c[csi.idEquip];
			menuInventory->c[csi.idEquip] = NULL;
		}
		menuInventory = baseCurrent->curTeam[num].inv;
	}
	else if ( !Q_strncmp( command, "team", 4 ) )
	{
		if ( !baseCurrent || num >= baseCurrent->numWholeTeam ) return;
	}
	else if ( !Q_strncmp( command, "hud", 3 ) )
	{
		CL_ActorSelectList( num );
		return;
	}

	// console commands
	Cbuf_AddText( va( "%sdeselect%i\n", command, (int)cl_selected->value ) );
	Cbuf_AddText( va( "%sselect%i\n", command, num ) );
	Cvar_ForceSet( "cl_selected", va( "%i", num ) );

	// set info cvars
	if ( !Q_strncmp( command, "team", 4 ) ) CL_CharacterCvars( &baseCurrent->wholeTeam[num] );
	else CL_CharacterCvars( &baseCurrent->curTeam[num] );
}

/*
======================
CL_UpdateHireVar
======================
*/
void CL_UpdateHireVar ( void )
{
	aircraft_t* air = NULL;

	// maybe we are in multiplayer - and there is no baseCurrent
	if ( baseCurrent->aircraftCurrent )
	{
		air = (aircraft_t*)baseCurrent->aircraftCurrent;
		Cvar_Set( "mn_hired", va( "%i of %i\n", air->teamSize, air->size ) );
	}
	else
		Cvar_Set( "mn_hired", va( "%i of %i\n", baseCurrent->numHired, MAX_ACTIVETEAM ) );

}

/*
======================
CL_MarkTeamCmd
======================
*/
void CL_MarkTeamCmd( void )
{
	int i;

	// check if we are allowed to be here?
	// we are only allowed to be here if we already set up a base
	// or are in multiplayer mode
	if ( ! baseCurrent )
	{
		Com_Printf("No base set up\n");
		MN_PopMenu(false);
		return;
	}

	baseCurrent->numHired = baseCurrent->numOnTeam;

	CL_UpdateHireVar();

	for ( i = 0; i < baseCurrent->numWholeTeam; i++ )
	{
		Cvar_ForceSet( va( "mn_name%i", i ), baseCurrent->wholeTeam[i].name );
		if ( baseCurrent->hiredMask & (1 << i) )
			Cbuf_AddText( va( "listadd%i\n", i ) );
	}

	for ( i = baseCurrent->numWholeTeam; i < (int)cl_numnames->value; i++ )
	{
		Cbuf_AddText( va( "listdisable%i\n", i ) );
		Cvar_ForceSet( va( "mn_name%i", i ), "" );
	}
}


/*
======================
CL_HireActorCmd
======================
*/
void CL_HireActorCmd( void )
{
	int num;
	aircraft_t* air = NULL;

	// check syntax
	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: hire <num>\n" );
		return;
	}
	num = atoi( Cmd_Argv(1) );

	if ( baseCurrent && baseCurrent->aircraftCurrent )
		air = (aircraft_t*)baseCurrent->aircraftCurrent;

	if ( num >= baseCurrent->numWholeTeam )
		return;

	if ( baseCurrent->hiredMask & (1 << num) )
	{
		// unhire
		Cbuf_AddText( va( "listdel%i\n", num ) );
		baseCurrent->hiredMask &= ~(1 << num);
		baseCurrent->numHired--;
		if ( air )
			air->teamSize--;
	}
	else if ( baseCurrent->numHired < MAX_ACTIVETEAM && ( ( air && air->size > air->teamSize ) || ! air ) )
	{
		// hire
		Cbuf_AddText( va( "listadd%i\n", num ) );
		baseCurrent->hiredMask |= (1 << num);
		baseCurrent->numHired++;
		if ( air && air->size >= air->teamSize )
			air->teamSize++;
	}

	// select the desired one anyways
	CL_UpdateHireVar();
	Cbuf_AddText( va( "team_select %i\n", num ) );
}


/*
======================
CL_MessageMenuCmd
======================
*/
char nameBackup[MAX_VAR];
char cvarName[MAX_VAR];

void CL_MessageMenuCmd( void )
{
	char *msg;

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: msgmenu <msg>\n" );
		return;
	}

	msg = Cmd_Argv( 1 );
	if ( msg[0] == '?'  )
	{
		// start
		Cbuf_AddText( "messagemenu\n" );
		Q_strncpyz( cvarName, msg+1, MAX_VAR );
		Q_strncpyz( nameBackup, Cvar_VariableString( cvarName ), MAX_VAR );
		Q_strncpyz( msg_buffer, nameBackup, sizeof(msg_buffer) );
		msg_bufferlen = strlen( nameBackup );
	}
	else if ( msg[0] == '!' )
	{
		// cancel
		Cvar_ForceSet( cvarName, nameBackup );
		Cvar_ForceSet( va( "%s%i", cvarName, (int)cl_selected->value ), nameBackup );
		Cbuf_AddText( va( "%s_changed\n", cvarName ) );
	}
	else if ( msg[0] == ':' )
	{
		// end
		Cvar_ForceSet( cvarName, msg+1 );
		Cvar_ForceSet( va( "%s%i", cvarName, (int)cl_selected->value ), msg+1 );
		Cbuf_AddText( va( "%s_changed\n", cvarName ) );
	}
	else
	{
		// continue
		Cvar_ForceSet( cvarName, msg );
		Cvar_ForceSet( va( "%s%i", cvarName, (int)cl_selected->value ), msg );
	}
}

/*
======================
CL_SaveTeam
======================
*/
void CL_SaveTeam( char *filename )
{
	sizebuf_t	sb;
	byte	buf[MAX_TEAMDATASIZE];
	FILE	*f;
	char	*name;
	int		res;

	assert(baseCurrent);

	// create the save dir - if needed
	FS_CreatePath( filename );
	// open file
	f = fopen( filename, "wb" );
	if ( !f )
	{
		Com_Printf( "Couldn't write file (%s).\n", filename );
		return;
	}

	// create data
	SZ_Init( &sb, buf, MAX_TEAMDATASIZE );

	name = Cvar_VariableString( "mn_teamname" );
	if ( !strlen(name) )
		Cvar_Set( "mn_teamname", _("NewTeam") );
	// store teamname
	MSG_WriteString( &sb, name );

	// store team
	CL_SendTeamInfo( &sb, baseCurrent->wholeTeam, baseCurrent->numWholeTeam );

	// store assignement
	MSG_WriteFormat( &sb, "lbbl", baseCurrent->teamMask, baseCurrent->numOnTeam, baseCurrent->numHired, baseCurrent->hiredMask );

	// write data
	res = fwrite( buf, 1, sb.cursize, f );
	fclose( f );

	if ( res == sb.cursize )
		Com_Printf( "Team '%s' saved.\n", filename );
}


/*
======================
CL_SaveTeamCmd
======================
*/
void CL_SaveTeamCmd( void )
{
	char	filename[MAX_QPATH];

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: saveteam <filename>\n" );
		return;
	}

	// save
	Com_sprintf( filename, MAX_QPATH, "%s/save/%s.mpt", FS_Gamedir(), Cmd_Argv( 1 ) );
	CL_SaveTeam( filename );
}


/*
======================
CL_SaveTeamSlotCmd
======================
*/
void CL_SaveTeamSlotCmd( void )
{
	char	filename[MAX_QPATH];

	// save
	Com_sprintf( filename, MAX_QPATH, "%s/save/team%s.mpt", FS_Gamedir(), Cvar_VariableString( "mn_slot" ) );
	CL_SaveTeam( filename );
}

/*
======================
CL_LoadTeamMember
======================
*/
void CL_LoadTeamMember( sizebuf_t *sb, character_t *chr )
{
	item_t item;
	int container, x, y;
	int i;

	// unique character number
	chr->ucn = MSG_ReadShort( sb );
	if ( chr->ucn >= baseCurrent->nextUCN ) baseCurrent->nextUCN = chr->ucn + 1;

	// name and model
	Q_strncpyz( chr->name, MSG_ReadString( sb ), MAX_VAR );
	Q_strncpyz( chr->path, MSG_ReadString( sb ), MAX_VAR );
	Q_strncpyz( chr->body, MSG_ReadString( sb ), MAX_VAR );
	Q_strncpyz( chr->head, MSG_ReadString( sb ), MAX_VAR );
	chr->skin = MSG_ReadByte( sb );

	// new attributes
	for (i = 0; i < SKILL_NUM_TYPES; i++)
		chr->skills[i] = MSG_ReadByte( sb );

	// load scores
	for (i = 0; i < KILLED_NUM_TYPES; i++)
		chr->kills[i] = MSG_ReadShort( sb );
	chr->assigned_missions = MSG_ReadShort( sb );

	// inventory
	Com_DestroyInventory( chr->inv );
	item.t = MSG_ReadByte( sb );
	while ( item.t != NONE )
	{
		// read info
		item.a = MSG_ReadByte( sb );
		item.m = MSG_ReadByte( sb );
		container = MSG_ReadByte( sb );
		x = MSG_ReadByte( sb );
		y = MSG_ReadByte( sb );

		// check info and add item if ok
		Com_AddToInventory( chr->inv, item, container, x, y );

		// get next item
		item.t = MSG_ReadByte( sb );
	}
}


/*
======================
CL_LoadTeam
======================
*/
void CL_LoadTeam( sizebuf_t *sb, base_t* base, int version )
{
	character_t	*chr;
	int i, p;

	// reset data
	CL_ResetCharacters( base );

	// read whole team list
	MSG_ReadByte( sb );
	base->numWholeTeam = MSG_ReadByte( sb );
	for ( i = 0, chr = base->wholeTeam; i < base->numWholeTeam; chr++, i++ )
		CL_LoadTeamMember( sb, chr );

	// get assignement
	MSG_ReadFormat( sb, "lbbl", &base->teamMask, &base->numOnTeam, &base->numHired, &base->hiredMask );

	Com_DPrintf("Load team with %i members and %i slots\n", base->numOnTeam, base->numWholeTeam );

	for ( i = 0, p = 0; i < base->numWholeTeam; i++ )
		if ( base->teamMask & (1 << i) )
			base->curTeam[p++] = base->wholeTeam[i];
}


/*
======================
CL_LoadTeamMultiplayer
======================
*/
void CL_LoadTeamMultiplayer( char *filename )
{
	sizebuf_t	sb;
	byte	buf[MAX_TEAMDATASIZE];
	FILE	*f;
	char	title[MAX_VAR];

	// return the base title
	Q_strncpyz( title, bmBases[0].title, MAX_VAR );
	MN_ClearBase( &bmBases[0] );
	Q_strncpyz( bmBases[0].title, title, MAX_VAR );

	// set base for multiplayer
	baseCurrent = &bmBases[0];
	ccs.numBases = 1;
	baseCurrent->hiredMask = 0;

	// open file
	f = fopen( filename, "rb" );
	if ( !f )
	{
		Com_Printf( "Couldn't open file '%s'.\n", filename );
		return;
	}

	// read data
	SZ_Init( &sb, buf, MAX_TEAMDATASIZE );
	sb.cursize = fread( buf, 1, MAX_TEAMDATASIZE, f );
	fclose( f );

	// load the teamname
	Cvar_Set( "mn_teamname", MSG_ReadString( &sb ) );

	// load the team
	CL_LoadTeam( &sb, &bmBases[0], SAVE_FILE_VERSION );
	baseCurrent->hiredMask = baseCurrent->teamMask;
}


/*
======================
CL_LoadTeamCmd
======================
*/
void CL_LoadTeamCmd( void )
{
	char	filename[MAX_QPATH];

	if ( Cmd_Argc() < 2 )
	{
		Com_Printf( "Usage: loadteam <filename>\n" );
		return;
	}

	// load
	Com_sprintf( filename, MAX_QPATH, "%s/save/%s.mpt", FS_Gamedir(), Cmd_Argv( 1 ) );
	CL_LoadTeamMultiplayer( filename );

	Com_Printf( "Team '%s' loaded.\n", Cmd_Argv( 1 ) );
}


/*
======================
CL_LoadTeamSlotCmd
======================
*/
void CL_LoadTeamSlotCmd( void )
{
	char	filename[MAX_QPATH];

	// load
	Com_sprintf( filename, MAX_QPATH, "%s/save/team%s.mpt", FS_Gamedir(), Cvar_VariableString( "mn_slot" ) );
	CL_LoadTeamMultiplayer( filename );

	Com_Printf( "Team 'team%s' loaded.\n", Cvar_VariableString( "mn_slot" ) );
}

/*
======================
CL_ResetTeams
======================
*/
void CL_ResetTeams( void )
{
	Cmd_AddCommand( "givename", CL_GiveNameCmd );
	Cmd_AddCommand( "gennames", CL_GenerateNamesCmd );
	Cmd_AddCommand( "genequip", CL_GenerateEquipmentCmd );
	Cmd_AddCommand( "equip_type", CL_EquipTypeCmd );
	Cmd_AddCommand( "team_mark", CL_MarkTeamCmd );
	Cmd_AddCommand( "team_hire", CL_HireActorCmd );
	Cmd_AddCommand( "team_select", CL_SelectCmd );
	Cmd_AddCommand( "team_changename", CL_ChangeNameCmd );
	Cmd_AddCommand( "team_changeskin", CL_ChangeSkinCmd );
	Cmd_AddCommand( "team_comments", CL_TeamCommentsCmd );
	Cmd_AddCommand( "equip_select", CL_SelectCmd );
	Cmd_AddCommand( "hud_select", CL_SelectCmd );
	Cmd_AddCommand( "saveteam", CL_SaveTeamCmd );
	Cmd_AddCommand( "saveteamslot", CL_SaveTeamSlotCmd );
	Cmd_AddCommand( "loadteam", CL_LoadTeamCmd );
	Cmd_AddCommand( "loadteamslot", CL_LoadTeamSlotCmd );
	Cmd_AddCommand( "msgmenu", CL_MessageMenuCmd );
}

/*
======================
CL_SendTeamInfo
======================
*/
void CL_SendItem( sizebuf_t *buf, item_t item, int container, int x, int y )
{
	if ( !csi.ods[item.t].reload )
	{
		// transfer with full free ammo
		item.m = item.t;
		item.a = csi.ods[item.t].ammo;
	}
	MSG_WriteFormat( buf, "bbbbbb", item.t, item.a, item.m, container, x, y );
}

void CL_SendTeamInfo( sizebuf_t *buf, character_t *team, int num )
{
	character_t	*chr;
	invList_t	*ic;
	int i, j;

	// clean temp inventory
	CL_CleanTempInventory();

	// header
	MSG_WriteByte( buf, clc_teaminfo );
	MSG_WriteByte( buf, num );

	for ( i = 0, chr = team; i < num; chr++, i++ )
	{
		// unique character number
		MSG_WriteShort( buf, chr->ucn );

		// name
		MSG_WriteString( buf, chr->name );

		// model
		MSG_WriteString( buf, chr->path );
		MSG_WriteString( buf, chr->body );
		MSG_WriteString( buf, chr->head );
		MSG_WriteByte( buf, chr->skin );

		// even new attributes
		for ( j = 0; j < SKILL_NUM_TYPES; j++ )
			MSG_WriteByte( buf, chr->skills[j] );

		// scores
		for ( j = 0; j < KILLED_NUM_TYPES; j++ )
			MSG_WriteShort( buf, chr->kills[j] );
		MSG_WriteShort( buf, chr->assigned_missions );

		// equipment
		for ( j = 0; j < csi.numIDs; j++ )
			for ( ic = chr->inv->c[j]; ic; ic = ic->next )
				CL_SendItem( buf, ic->item, j, ic->x, ic->y );

		// terminate list
		MSG_WriteByte( buf, NONE );
	}
}


/*
======================
CL_ParseResults
======================
*/
char	resultText[MAX_MENUTEXTLEN];

void CL_ParseResults( sizebuf_t *buf )
{
	byte	num_spawned[MAX_TEAMS];
	byte	num_alive[MAX_TEAMS];
	byte	num_kills[MAX_TEAMS][MAX_TEAMS];
	byte	winner, we;
	char	*singleEnemy, *multiEnemy;
	int		i, j, num, res, kills;

	// get number of teams
	num = MSG_ReadByte( buf );
	if ( num > MAX_TEAMS )
		Sys_Error( "Too many teams in result message\n");

	// get winning team
	winner = MSG_ReadByte( buf );
	we = cls.team;
	singleEnemy = curCampaign ? _("Alien") : _("Enemy");
	multiEnemy = curCampaign ? _("Aliens") : _("Enemies");

	// get spawn and alive count
	for ( i = 0; i < num; i++ )
	{
		num_spawned[i] = MSG_ReadByte( buf );
		num_alive[i] = MSG_ReadByte( buf );
	}

	// get kills
	for ( i = 0; i < num; i++ )
		for ( j = 0; j < num; j++ )
			num_kills[i][j] = MSG_ReadByte( buf );

	// read terminator
	if ( MSG_ReadByte( buf ) != NONE )
		Com_Printf( "WARNING: bad result message\n" );

	// init result text
	menuText[TEXT_STANDARD] = resultText;

	// alien stats
	for ( i = 1, kills = 0; i < num; i++ ) kills += (i == we) ? 0 : num_kills[we][i];
	// needs to be cleared and then append to it
	Com_sprintf( resultText, MAX_MENUTEXTLEN, _("%s killed\t%i\n"), multiEnemy, kills );
	for ( i = 1, res = 0; i < num; i++ ) res += (i == we) ? 0 : num_alive[i];
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("%s survivors\t%i\n\n"), singleEnemy, res ) );

	// team stats
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Team losses\t%i\n"), num_spawned[we] - num_alive[we] ) );
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Friendly fire losses\t%i\n"), num_kills[we][we] ) );
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Team survivors\t%i\n\n"), num_alive[we] ) );

	// kill civilians on campaign, if not won
	if ( curCampaign && num_alive[TEAM_CIVILIAN] && winner != we )
	{
		num_kills[TEAM_ALIEN][TEAM_CIVILIAN] += num_alive[TEAM_CIVILIAN];
		num_alive[TEAM_CIVILIAN] = 0;
	}

	// civilian stats
	for ( i = 1, res = 0; i < num; i++ ) res += (i == we) ? 0 : num_kills[i][TEAM_CIVILIAN];
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Civilians killed by the %s\t%i\n"), multiEnemy, res ) );
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Civilians killed by your Team\t%i\n"), num_kills[we][TEAM_CIVILIAN] ) );
	Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Civilians saved\t%i\n\n\n"), num_alive[TEAM_CIVILIAN] ) );

	MN_PopMenu( true );
	if ( !curCampaign )
	{
		// get correct menus
		Cvar_Set( "mn_main", "main" );
		Cvar_Set( "mn_active", "" );
		MN_PushMenu( "main" );
	}
	else
	{
		mission_t	*ms;

		// get correct menus
		Cvar_Set( "mn_main", "singleplayer" );
		Cvar_Set( "mn_active", "map" );
		MN_PushMenu( "map" );

		// show money stats
		ms = selMis->def;
		ccs.reward = 0;
		if ( winner == we )
		{
			Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Collected alien technology\t%i $\n"), ms->cr_win ) );
			ccs.reward += ms->cr_win;
		}
		if ( kills )
		{
			Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Aliens killed\t%i $\n"), kills * ms->cr_alien ) );
			ccs.reward += kills * ms->cr_alien;
		}
		if ( winner == we && num_alive[TEAM_CIVILIAN] )
		{
			Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Civilians saved\t%i $\n"), num_alive[TEAM_CIVILIAN] * ms->cr_civilian ) );
			ccs.reward += num_alive[TEAM_CIVILIAN] * ms->cr_civilian;
		}
		Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("Total reward\t%i $\n\n\n"), ccs.reward ) );

		// recruits
		if ( winner == we && ms->recruits )
			Q_strcat( resultText, MAX_MENUTEXTLEN, va( _("New Recruits\t%i\n"), ms->recruits ) );

		// loot the battlefield
		CL_CollectItems( winner == we );

		// check for stunned aliens
		if ( winner == we )
			CL_CollectAliens( ms );

		// update stats
		CL_UpdateCharacterStats( winner == we );

		ccs.eCampaign = ccs.eMission;
	}

	// disconnect and show win screen
	if ( winner == we )
		MN_PushMenu( "won" );
	else
		MN_PushMenu( "lost" );
	Cbuf_AddText( "disconnect\n" );
	Cbuf_Execute();
}

