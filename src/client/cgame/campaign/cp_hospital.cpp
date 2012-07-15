/**
 * @file
 * @brief Most of the hospital related stuff
 * @note Hospital functions prefix: HOS_
 */

/*
Copyright (C) 2002-2011 UFO: Alien Invasion.

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

#include "../../cl_shared.h"
#include "cp_campaign.h"
#include "cp_hospital.h"

#define GET_HP_HEALING( ab ) (1 + (ab) * 15/MAX_SKILL)

static void HOS_HealWounds (character_t* chr, int healing)
{
	int bodyPart;
	woundInfo_t *wounds = &chr->wounds;

	for (bodyPart = 0; bodyPart < chr->teamDef->bodyTemplate->numBodyParts(); ++bodyPart)
		if (wounds->treatmentLevel[bodyPart] > 0)
			wounds->treatmentLevel[bodyPart] = std::max(0, wounds->treatmentLevel[bodyPart] - healing);

}

/**
 * @brief Heals character.
 * @param[in] chr Character data of an employee
 * @param[in] hospital True if we call this function for hospital healing (false when autoheal).
 * @sa HOS_HealEmployee
 * @return true if soldiers becomes healed - false otherwise
 */
bool HOS_HealCharacter (character_t* chr, bool hospital)
{
	assert(chr);
	float healing = ccs.curCampaign->healingRate;

	if (hospital) {
		healing *= GET_HP_HEALING(chr->score.skills[ABILITY_POWER]);
		HOS_HealWounds(chr, healing);
	}

	if (chr->HP < chr->maxHP) {

		/* if the character has less that 100 hitpoints, he will be disadvantaged by using the percentage
		 * method of allocating hitpoints.  So just give the character "healing" as Hitpoints, otherwise
		 * allocate "healing" as a percentage of the characters total hitpoints. */
		if (chr->maxHP < MAX_HP)
			chr->HP = std::min(chr->HP + static_cast<int>(healing), chr->maxHP);
		else
			chr->HP = std::min(chr->HP + static_cast<int>(((healing / 100.0f) * chr->maxHP)), chr->maxHP);

		if (chr->HP == chr->maxHP)
			return false;
		return true;
	}
	return false;
}

/**
 * @brief Checks health status of all employees in all bases.
 * @sa CP_CampaignRun
 * @note Called every day.
 */
void HOS_HospitalRun (void)
{
	int i;

	for (i = 0; i < MAX_EMPL; i++) {
		employeeType_t type = (employeeType_t)i;
		E_Foreach(type, employee) {
			if (!E_IsHired(employee))
				continue;
			if (B_GetBuildingStatus(employee->baseHired, B_HOSPITAL))
				HOS_HealCharacter(&(employee->chr), true);
			else
				HOS_HealCharacter(&(employee->chr), false);
		}
	}
}

/**
 * @brief Callback for HOS_HealCharacter() in hospital.
 * @param[in] employee Pointer to the employee to heal.
 * @sa HOS_HealCharacter
 * @sa HOS_HealAll
 */
bool HOS_HealEmployee (employee_t* employee)
{
	assert(employee);
	return HOS_HealCharacter(&employee->chr, true);
}

/**
 * @brief Heal all employees in the given base
 * @param[in] base The base the employees should become healed
 * @sa HOS_HealEmployee
 */
void HOS_HealAll (const base_t* const base)
{
	int type;

	assert(base);

	for (type = 0; type < MAX_EMPL; type++) {
		E_Foreach(type, employee) {
			if (!E_IsInBase(employee, base))
				continue;
			HOS_HealEmployee(employee);
		}
	}
}


#ifdef DEBUG
/**
 * @brief Set the character HP field to maxHP.
 */
static void HOS_HealAll_f (void)
{
	int type;
	base_t *base = B_GetCurrentSelectedBase();

	if (!base)
		return;

	for (type = 0; type < MAX_EMPL; type++) {
		E_Foreach(type, employee) {
			if (!E_IsInBase(employee, base))
				continue;
			employee->chr.HP = employee->chr.maxHP;
		}
	}
}

/**
 * @brief Decrement the character HP field by one.
 */
static void HOS_HurtAll_f (void)
{
	int type, amount;
	base_t *base = B_GetCurrentSelectedBase();

	if (!base)
		return;

	if (Cmd_Argc() == 2)
		amount = atoi(Cmd_Argv(1));
	else
		amount = 1;

	for (type = 0; type < MAX_EMPL; type++) {
		E_Foreach(type, employee) {
			/* only those employees, that are in the current base */
			if (!E_IsInBase(employee, base))
				continue;
			employee->chr.HP = std::max(0, employee->chr.HP - amount);
		}
	}
}
#endif


/**
 * @brief Initial stuff for hospitals
 * Bind some of the functions in this file to console-commands that you can call ingame.
 * Called from UI_InitStartup resp. CL_InitLocal
 */
void HOS_InitStartup (void)
{
#ifdef DEBUG
	Cmd_AddCommand("debug_hosp_hurt_all", HOS_HurtAll_f, "Debug function to hurt all employees in the current base by one");
	Cmd_AddCommand("debug_hosp_heal_all", HOS_HealAll_f, "Debug function to heal all employees in the current base completely");
#endif
}

/**
 * @brief Saving function for hospital related data
 * @sa HOS_LoadXML
 * @sa SAV_GameSaveXML
 */
bool HOS_SaveXML (xmlNode_t *p)
{
	/* nothing to save here */
	return true;
}

/**
 * @brief Saving function for hospital related data
 * @sa HOS_SaveXML
 * @sa SAV_GameLoadXML
 */
bool HOS_LoadXML (xmlNode_t *p)
{
	return true;
}

/**
 * @brief Returns true if you can enter in the hospital
 * @sa B_BaseInit_f
 */
bool HOS_HospitalAllowed (const base_t* base)
{
	return !B_IsUnderAttack(base) && B_GetBuildingStatus(base, B_HOSPITAL);
}
