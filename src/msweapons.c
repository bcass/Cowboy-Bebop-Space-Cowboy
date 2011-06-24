/*
 ****************************************************************************
 *	 Completly rewritten by Funf	2006-03-10								*
 *	 																		*
 *	 Massive overhaul. Components, weapons, disable types, MONO, etc.		*
 *																			*
 *   Re-written by Kristen													*
 *									   										*
 *   Changed all ship weapons to Cowboy Bebop theme			   				*
 *									   										*
 *   Originally creator Antrox										   		*
 ****************************************************************************
*/

/*
OLD Weapon List <- Outdated

00 - Dual Machineguns
01 - 105mm Rifle
02 - 30mm Cannon
03 - Grappling Claw
04 - Machine Gun
05 - Cluster Missile Launcher
06 - Multiple Munitions Launcher
07 - Plasma Cannon
08 - Gatling Gun
09 - Super Plasma Cannon
10 - None

*/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 *	External functions
 */
bool    autofly(SHIP_DATA *ship);

/*
 *	Global varibles
 */
SHIP_WEAPON_DATA	*first_weapon;
SHIP_WEAPON_DATA	*last_weapon;

/*
 *	Local functions
 */
void do_newweapon(CHAR_DATA *ch, char *argument);
void do_destroyweapon(CHAR_DATA *ch, char *argument);
void do_setweapon(CHAR_DATA *ch, char *argument);
void do_showweapon(CHAR_DATA *ch, char *argument);
void do_weapons(CHAR_DATA *ch, char *argument);
SHIP_WEAPON_DATA * obj_to_weapon(OBJ_DATA *obj);
void do_fire(CHAR_DATA *ch, char *argument);
void shoot_weapon(CHAR_DATA *ch, SHIP_DATA*, SHIP_WEAPON_DATA *weapon);
void reset_weapon(SHIP_WEAPON_DATA *weapon);
void save_weapons();
void load_weapons();


/* Create new weapon - Funf */
void do_newweapon(CHAR_DATA *ch, char *argument)
{
	sh_int index;
	SHIP_WEAPON_DATA *weapon;
	char arg1[MAX_STRING_LENGTH];

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char("Syntax: newweapon <index> <name>\n\r",ch);
		return;
	}

	argument = one_argument( argument, arg1 );

	index = atoi(arg1);

	if ( index < 0 || index > 10000)
	{
		send_to_char("Weapon index needs to be a positive integer.\n\r",ch);
		return;
	}

	for( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		if ( weapon->index == index )
		{
			send_to_char("Weapon index needs to be unique.\n\r",ch);
			return;
		}
	}

	CREATE( weapon, SHIP_WEAPON_DATA, 1 );

	reset_weapon( weapon );

	weapon->index = index;
	weapon->name = STRALLOC( argument );

	LINK( weapon, first_weapon, last_weapon, next, prev);

	save_weapons();

	send_to_char("&GDone.\n\r",ch);

	return;
}

/* Destroy and unlink weapon - Funf */
void do_destroyweapon(CHAR_DATA *ch, char *argument)
{
	SHIP_WEAPON_DATA *weapon;
	sh_int index;
	bool found = FALSE;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char("Usage: destroyweapon <weapon index>\n\r", ch);
		return;
	}

	index = atoi(argument);

	if ( index < 0 || index > 10000)
	{
		send_to_char("Weapon index needs to be a positive integer.\n\r",ch);
		return;
	}

	for( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		if ( weapon->index == index )
		{
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		send_to_char("No weapon with specified index.\n\r",ch);
		return;
	}

	UNLINK( weapon, first_weapon, last_weapon, next, prev);
	STRFREE( weapon->name );
	DISPOSE( weapon );

	save_weapons();

	send_to_char("&GDone.\n\r",ch);

	return;
}

/* Set weapon - Funf */
void do_setweapon(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	SHIP_WEAPON_DATA *weapon, *weap2;
	bool found = FALSE;
	int i;
	sh_int index;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char("Syntax: setweapon <index> <field> <value>\n\rPossible fields are:\n\r",ch);
		send_to_char("index name disabletype disablechance\n\rmindam maxdam range lag ammo needsmono\n\rto_hit\n\r",ch);
		return;
	}

	argument = one_argument( argument, arg1 );

	index = atoi(arg1);

	if ( index < 0 || index > 10000)
	{
		send_to_char("Weapon index needs to be a positive integer.\n\r",ch);
		return;
	}

	for ( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		if ( weapon->index == index )
		{
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		send_to_char("No weapon with specified index.\n\r",ch);
		return;
	}

	argument = one_argument( argument, arg2 );

	if ( !strcmp( "index", arg2 ) )
	{
		index = atoi(argument);

		if ( index < 0 || index > 10000 )
		{
			send_to_char("Weapon indexes needs to be a positive integers.\n\r",ch);
			return;
		}

		for ( weap2 = first_weapon ; weap2 ; weap2 = weap2->next )
		{
			if ( weap2->index == index )
			{
				send_to_char("Weapon indexes needs to be unique.\n\r",ch);
				return;
			}
		}

		weapon->index = index;
	}
	else if ( !strcmp( "name", arg2 ) )
	{
		if ( weapon->name )
			STRFREE( weapon->name );
		weapon->name = STRALLOC( argument );
	}
	else if ( !strcmp( "disabletype", arg2 ) )
	{
		if( !argument || argument[0] == '\0' )
		{
			send_to_char("Disable Types: None, Damage, EMP, Sabotage, MONO, Radiation.\n\r",ch); /* drive not working, change shootweapon(). - Funf */
			return;
		}

		i = atoi(argument);

		if ( !strcmp( "none", argument ) )
		{
			weapon->disable_type = DISABLE_NONE;
		}
		else if ( !strcmp( "damage", argument ) )
		{
			weapon->disable_type = DISABLE_DAMAGE;
		}
		else if ( !strcmp( "emp", argument ) )
		{
			weapon->disable_type = DISABLE_EMP;
		}
		else if ( !strcmp( "sabotage", argument ) )
		{
			weapon->disable_type = DISABLE_SABOTAGE;
		}
		else if ( !strcmp( "mono", argument ) )
		{
			weapon->disable_type = DISABLE_MONO;
		}
		else if ( !strcmp( "drive", argument ) )
		{
			weapon->disable_type = DISABLE_DRIVE;
		}
		else if ( !strcmp( "radiation", argument ) )
		{
			weapon->disable_type = DISABLE_RADIATION;
		}
		else if ( index > 0 && index < 10 )
		{
			weapon->disable_type = i;
		}
		else
		{
			send_to_char("Disable types: none, damage, emp, sabotage, mono, radiation, and drive.\n\r",ch);
			return;
		}
	}
	else if ( !strcmp( "disablechance", arg2 ) )
	{
		index = atoi(argument);

		if ( index < 0 || index > 1000 ) /* chance in 0.1% increments - Funf */
		{
			send_to_char("Disable chance: 0-1000 in 0.1% increments.\n\r",ch);
			return;
		}

		weapon->disable_chance = index;
	}
	else if ( !strcmp( "mindam", arg2 ) )
	{
		i = atoi(argument);

		if ( i > 1000000 || i < -1000000 )
		{
			send_to_char("Mindam out of bounds: -1000000 to 1000000.\n\r",ch);
			return;
		}

		weapon->mindam = i;
	}
	else if ( !strcmp( "maxdam", arg2 ) )
	{
		i = atoi(argument);

		if ( i > 1000000 || i < -1000000 )
		{
			send_to_char("Maxdam out of bounds: -1000000 to 1000000.\n\r",ch);
			return;
		}

		weapon->maxdam = i;
	}
	else if ( !strcmp( "range", arg2 ) )
	{
		i = atoi(argument);

		if ( i > 1000000 || i < 0 )
		{
			send_to_char("Range must be between 0 and 1000000.\n\r",ch);
			return;
		}

		weapon->range = i;
	}
	else if ( !strcmp( "lag", arg2 ) )
	{
		index = atoi(argument);

		if ( index < 0 || index > 10000 )
		{
			send_to_char("Lag must be between 0 and 10000.\n\r",ch);
			return;
		}

		weapon->lag = index;
	}
	else if ( !strcmp( "ammo", arg2 ) )
	{
		i = atoi(argument);

		if ( i > 1000000 || i < -1000000 )
		{
			send_to_char("Ammo usage out of bounds: -1000000 to 1000000.\n\r",ch);
			return;
		}

		weapon->ammo = i;
	}
	else if ( !strcmp( "needsmono", arg2 ) )
	{


		if ( !strcmp("true", argument) || !strcmp("yes", argument) )
		{
			weapon->needs_mono = TRUE;
		}
		else if ( !strcmp("false", argument) || !strcmp("no", argument) )
		{
			weapon->needs_mono = FALSE;
		}
		else
		{
			send_to_char("NeedsMONO Options: yes/no or true/false.\n\r",ch);
			return;
		}
	}
	else if ( !strcmp( "to_hit", arg2 ) || !strcmp("tohit", arg2) )
	{
		i = atoi(argument);

		if ( i > 1000 || i < -1000 )
		{
			send_to_char("To_hit modifier out of bounds: -1000 to 1000.\n\r",ch);
			return;
		}

		weapon->to_hit = i;
	}
	else
	{
		do_setweapon( ch, "" );
		return;
	}

	save_weapons();

	send_to_char("&GDone.\n\r",ch);

	return;
}

/* Show detailed weapon info - Funf */
void do_showweapon(CHAR_DATA *ch, char *argument)
{
	SHIP_WEAPON_DATA *weapon;
	char buf[MAX_STRING_LENGTH];
	sh_int index;
	bool found = FALSE;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char("Syntax: showweapon <weapon index>\n\r",ch);
		return;
	}

	index = atoi(argument);

	if ( index < 0 )
	{
		send_to_char("Weapon index needs to be a positive integer.\n\r",ch);
		return;
	}

	for ( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		if ( weapon->index == index )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char("No such weapon found.\n\r",ch);
		return;
	}


	sprintf( buf, "Index: %d   Name: %s\n\r", weapon->index, weapon->name ? weapon->name : "<null>" );
	send_to_char( buf, ch );

	sprintf( buf, "Disable Type: %s   Disable Chance: %d.%d%%\n\r",
		weapon->disable_type == DISABLE_NONE ? "None" :
		weapon->disable_type == DISABLE_DAMAGE ? "Damage" :
		weapon->disable_type == DISABLE_EMP ? "EMP" :
		weapon->disable_type == DISABLE_SABOTAGE ? "Sabotage" :
		weapon->disable_type == DISABLE_MONO ? "MONO" :
		weapon->disable_type == DISABLE_DRIVE ? "Drive" :
		weapon->disable_type == DISABLE_RADIATION ? "Radiation" : "Unknown",
		weapon->disable_chance / 10, weapon->disable_chance % 10);
	send_to_char( buf, ch );

	sprintf( buf, "MinDam: %d   MaxDam: %d   Range: %d\n\r", weapon->mindam, weapon->maxdam, weapon->range );
	send_to_char( buf, ch );

	sprintf( buf, "Lag: %d   Ammo: %d   To_Hit: %d   NeedsMONO: %s\n\r", weapon->lag, weapon->ammo,
	weapon->to_hit, weapon->needs_mono ? "True" : "False" );
	send_to_char( buf, ch );

	return;
}

/* List all weapons - Funf */
void do_weapons(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	SHIP_WEAPON_DATA *weapon;

	for ( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		sprintf( buf, "%d - %s\n\r", weapon->index, weapon->name ? weapon->name : "<null>" );
		send_to_char( buf, ch );
	}

	return;
}


/* Takes a Component OBJ and spits back the indicated weapon or NULL on error - Funf */
SHIP_WEAPON_DATA * obj_to_weapon(OBJ_DATA * obj)
{
	SHIP_WEAPON_DATA *weapon;

	if(!obj)
		return NULL;

	if(obj->item_type != ITEM_SHIPCOMPONENT)
		return NULL;

	if(obj->value[0] != COMPONENT_WEAPON)
		return NULL;

	for ( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		if (obj->value[1] == weapon->index)
			return weapon;
	}

	return NULL; /* Not found */
}


/* Player command, fire a weapon - Funf */
void do_fire(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SHIP_DATA *ship;
	OBJ_DATA *component;
	int weapon = 0, count = 0;

	if (  (ship = ship_from_turret(ch->in_room->vnum))  == NULL )
    {
    	send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
        return;
    }


    if (ship->shipstate == SHIP_HYPERSPACE)
    {
        send_to_char("&RYou can only do that in realspace!\n\r",ch);
        return;
    }

    if (ship->starsystem == NULL)
    {
        send_to_char("&RYou can't do that until after you've finished launching!\n\r",ch);
        return;
    }

    if ( autofly(ship) )
	{
    	send_to_char("&RYou'll have to turn off the ships autopilot first.\n\r",ch);
    	return;
    }

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char("&RYou must specify which weapon to fire!\n\r&WOptions:\n\r",ch);
		count = 0;
		for (component = ship->first_component ; component ; component = component->next_content )
		{
			if ( component->value[0] == COMPONENT_WEAPON )
			{
				count++;
				sprintf(buf, "&Y%d &W- &R%s&w\n\r", count, component->short_descr); // Short description may not match name... -Funf
				send_to_char(buf, ch);
			}
		}

		return;
	}

	weapon = atoi(arg); /* Player specified weapon by number */

	if ( weapon > 0 && weapon <= 100 ) /* If atoi worked and player provided valid number */
	{
		for ( component = ship->first_component ; component ; component = component->next_content ) /* Search components */
		{
			if ( component->value[0] == COMPONENT_WEAPON )
			{
				++count;
				if ( count == weapon ) /* If found */
				{
					if ( component->value[3] != DISABLE_NONE )
					{
						send_to_char("&RThat weapon is disabled!\n\r",ch);
				        return;
					}
					if ( component->value[4] ) /* If in use */
					{
						send_to_char("&RThat weapon is currently in use!\n\r",ch);
				        return;
					}
					component->value[4] = 1; /* Set in use to true */
					shoot_weapon( ch, ship, obj_to_weapon(component) ); /* Shoot the weapon */
					component->value[4] = 0; /* Set in use to false */
					return; /* and get out of here */
				}
			}
		}
	}

	for ( component = ship->first_component ; component ; component = component->next_content )
	{
		if ( component->value[0] == COMPONENT_WEAPON && nifty_is_name( arg, component->name ) ) /* Specified by weapon name and matched! */
		{
			if ( component->value[3] != DISABLE_NONE )
			{
				send_to_char("&RThat weapon is disabled!\n\r",ch);
		        return;
			}
			if ( component->value[4] ) /* If in use */
			{
				send_to_char("&RThat weapon is currently in use!\n\r",ch);
		        return;
			}
			component->value[4] = 1; /* Set in use to true */
			shoot_weapon( ch, ship, obj_to_weapon(component) ); /* Shoot the weapon */
			component->value[4] = 0; /* Set in use to false */
			return; /* and get out of here */
		}
	}

	send_to_char("&RNo such weapon!\n\r&WOptions:\n\r",ch); /* Couldn't find it by number or name */
	count = 0;
	for (component = ship->first_component ; component ; component = component->next_content )
	{
		if ( component->value[0] == COMPONENT_WEAPON )
		{
			count++;
			sprintf(buf, "&Y%d &W- &R%s&w\n\r", count, component->short_descr);
			send_to_char(buf, ch);
		}
	}


	return;
}


/* Heavily modified generic version of do_<weapon name here>. - Funf */
void shoot_weapon(CHAR_DATA *ch, SHIP_DATA *ship, SHIP_WEAPON_DATA *weapon )
{
    int chance, i;
    SHIP_DATA *target;
    char buf[MAX_STRING_LENGTH];

	if (!ch || !weapon)
	{
		bug("shoot_weapon: Null pointer! (Undefined weapon?)");
		return;
	}

    if ( ship->ammo < weapon->ammo )
    {
        send_to_char("&ROh No! Not Enough Ammo!\n\r",ch);
        return;
    }

  	if ( ch->in_room->vnum == ship->gunseat )
   	{

    	if ( ship->shipstate == DISABLE_MONO && weapon->needs_mono ) /* MONO check */
    	{
			send_to_char("&RYou can't fire this weapon without MONO!\n\r",ch);
			return;
		}

    	if (ship->target0 == NULL )
    	{
    	  	send_to_char("&RYou need to choose a target first.\n\r",ch);
    	   	return;
    	}

    	target = ship->target0;

    	if (ship->target0->starsystem != ship->starsystem)
    	{
    	   	send_to_char("&RYour target seems to have left.\n\r",ch);
    	    ship->target0 = NULL;
    	  	return;
    	}

        if ( abs(target->vx - ship->vx) > weapon->range ||
             abs(target->vy - ship->vy) > weapon->range ||
             abs(target->vz - ship->vz) > weapon->range )
        {
        	send_to_char("&RThat ship is out of range.\n\r",ch);
    	  	return;
        }

        ship->statet0++; /* Found out what this does, in recharge_ships() this will subtract from the ship's fuel. - Funf */

		chance = IS_NPC(ch) ? ch->top_level /* Added lck factor and reduced spacecombat effectiveness - Funf */
		            : (int) ( 30 + ch->perm_dex*2 + ch->perm_lck/2 + ch->pcdata->learned[gsn_spacecombat]/10
             		+ ch->pcdata->learned[gsn_spacecombat2]/10 + ch->pcdata->learned[gsn_spacecombat3]/10 );

        chance += weapon->to_hit; /* Adjustable weapon modifier - Funf */
        chance += target->class*25; /* Fighters: No class penalty. Mids: -25% evade. Caps: -50% evade. - Funf */

        /*chance -= target->manuever/10;*/
        chance -= target->currspeed/20;

        chance -= target->evading; /* Used by Evasive Manuevers skill -Funf */

        chance -= ( abs(target->vx - ship->vx)/70 );
        chance -= ( abs(target->vy - ship->vy)/70 );
        chance -= ( abs(target->vz - ship->vz)/70 );

        chance = URANGE( 10 , chance , 90 );

        act( AT_PLAIN, "$n presses the fire button.", ch, NULL, NULL , TO_ROOM );

        if ( number_percent( ) > chance )
        {
          	sprintf( buf , "A %s fires from %s and misses you." , weapon->name, ship->name);
            echo_to_cockpit( AT_ORANGE , target , buf );

            sprintf( buf , "You shoot your %s at %s but miss." , weapon->name, target->name);
            echo_to_cockpit( AT_ORANGE , ship , buf );

    	    sprintf( buf, "A %s fires from %s and barely misses %s." , weapon->name, ship->name , target->name );
            echo_to_system( AT_ORANGE , ship , buf , target );

            learn_from_failure( ch, gsn_spacecombat );
	 	    learn_from_failure( ch, gsn_spacecombat2 );
    	    learn_from_failure( ch, gsn_spacecombat3 );

		    WAIT_STATE( ch , weapon->lag * (ship->mono_disabled ? 2 : 1) );

	  	    return;
        }

        sprintf( buf, "A %s fires from %s and hits %s." , weapon->name, ship->name, target->name );
        echo_to_system( AT_ORANGE , ship , buf , target );

        sprintf( buf , "You are hit by the %s fired from %s!" , weapon->name, ship->name);
        echo_to_cockpit( AT_BLOOD , target , buf );

        sprintf( buf , "Your %s hits %s!" , weapon->name, target->name);
        echo_to_cockpit( AT_YELLOW , ship , buf );

        learn_from_success( ch, gsn_spacecombat );
        learn_from_success( ch, gsn_spacecombat2 );
        learn_from_success( ch, gsn_spacecombat3 );

        echo_to_ship( AT_RED , target , "A small explosion vibrates through the ship." );

		if (weapon->disable_type != DISABLE_RADIATION) /* Support for damaging players, not the ship - Funf */
		{
			echo_to_ship( AT_RED , target , "A small explosion vibrates through the ship." );
            damage_ship_ch( target , weapon->mindam , weapon->maxdam , ch ); /* Hurt people */
            if(target->hull > target->maxhull) target->hull = target->maxhull; /* Fix overhealing bug - Funf */
		}
		else echo_to_ship( AT_RED , target , "A burning sensation fizzles through the ship." );

		/* disable ship - Funf */
		chance = weapon->disable_chance/10 + (get_curr_lck( ch ) > 15 ? 1 : 0); /* disable_chance in 0.1% increments - Funf */
		if ( number_percent( ) < chance ) /* chance to disable - Funf */
		{
			OBJ_DATA * component;
			int weaponcnt = 1; /* includes drive */

			/* Note: You have an equal chance of disabling any particular weapon or the drive. - Funf */
			for ( component = target->first_component ; component ; component = component->next_content )
			{
				if ( component->item_type == ITEM_SHIPCOMPONENT && component->value[0] == COMPONENT_WEAPON )
					++weaponcnt; /* how many weapons? */
			}

			chance = number_percent();

			if ( weapon->disable_type == DISABLE_MONO )
			{
				target->mono_disabled = TRUE;
				ship->autopilot = FALSE;
				ship->autorecharge = FALSE;
				echo_to_cockpit( AT_RED , target , "MONO system crash! Minimal functionality only!");
				calc_ship_speed( target ); /* refresh speed */
			}
			else if ( weapon->disable_type == DISABLE_RADIATION )
			{   /* Radiation Code. Damage all people in the ship - Funf 2007-04-05 */
				int roomnum;
				ROOM_INDEX_DATA * room;
				CHAR_DATA *	vic;
				int dmg;
				char cmd[MAX_STRING_LENGTH];

				for ( roomnum = target->firstroom ; roomnum <= target->lastroom ;roomnum++ )
				{
					room = get_room_index(roomnum);
				    for ( vic = room->first_person; vic ; vic = vic->next_in_room )
				    {
						dmg = number_range(weapon->mindam, weapon->maxdam);
						sprintf( cmd, "%s mpdamage %s 100", vic->name, vic->name );
						do_mpat( supermob, cmd ); /* damage char */
					}
				}

			}
			else if ( chance < 100 / weaponcnt )
			{
				if ( weapon->disable_type == DISABLE_NONE )
					target->shipstate = SHIP_FLYING; /* support for "healing" weapon - Funf */
				else
					target->shipstate = SHIP_DISABLED;
			}
			else
			{
				for( i = 0 ; i < weaponcnt ; i++ )
				{
					if ( chance > (100 / weaponcnt) * i && chance < (100 / weaponcnt) * i + weaponcnt )
					{
						weaponcnt = i;
						break;
					}
				}

				for ( component = target->first_component ; component ; component = component->next_content )
				{
					if ( component->item_type == ITEM_SHIPCOMPONENT && component->value[0] == COMPONENT_WEAPON )
					{
						--weaponcnt;
						if(!weaponcnt)
						{
							component->value[3] = weapon->disable_type;
						}
					}
				}

			}
		}

    	WAIT_STATE( ch , weapon->lag * (ship->mono_disabled ? 2 : 1) );

		// Adrenaline Rush! Gatz
		if(ch->adren < 5)
			ch->adren += 1;

   		ship->ammo -= weapon->ammo;

        return;
	}
}


/* Clear all values but next and prev - Funf */
void reset_weapon(SHIP_WEAPON_DATA *weapon)
{
	weapon->index = 0;
	if ( weapon->name )
		STRFREE( weapon->name );
	weapon->name = NULL;
	/* weapon->mass = 0; */
	weapon->disable_type = 0;
	weapon->disable_chance = 0;
	weapon->mindam = 0;
	weapon->maxdam = 0;
	weapon->range = 0;
	weapon->lag = 0;
	weapon->ammo = 0;
	weapon->to_hit = 0;
	weapon->needs_mono = FALSE;
	return;
}


/* Serialize the global weapon list to space/weapons.lst - Funf */
void save_weapons()
{
	FILE *fp;
    char filename[256];
    SHIP_WEAPON_DATA *weapon;

    sprintf( filename, "%s%s", SHIP_DIR, "weapons.lst" );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug("save_weapons: Couldn't open weapons.lst for writing!");
    	return;
	}

	for ( weapon = first_weapon ; weapon ; weapon = weapon->next )
	{
		fprintf( fp, "Index           %d\n",	weapon->index		);
		fprintf( fp, "Name            %s~\n",	weapon->name		);
		fprintf( fp, "DisableType     %d\n",	weapon->disable_type		);
		fprintf( fp, "DisableChance   %d\n",	weapon->disable_chance		);
		fprintf( fp, "MinDam          %d\n",	weapon->mindam		);
		fprintf( fp, "MaxDam          %d\n",	weapon->maxdam		);
		fprintf( fp, "Range           %d\n",	weapon->range		);
		fprintf( fp, "Lag             %d\n",	weapon->lag		);
		fprintf( fp, "Ammo            %d\n",	weapon->ammo		);
		fprintf( fp, "ToHit           %d\n",	weapon->to_hit		);
		fprintf( fp, "NeedsMono       %d\n",	weapon->needs_mono		);
		fprintf( fp, "EndWeapon\n" );
	}

	fprintf( fp, "END\n\n" );

	fclose( fp );

	return;
}



#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* Load the global weapon list from space/weapons.lst - Funf */
void load_weapons()
{
	FILE *fp;
    char filename[256];
    bool fMatch;
    char *word;
    SHIP_WEAPON_DATA *weapon;

	first_weapon = NULL;
	last_weapon = NULL;

    sprintf( filename, "%s%s", SHIP_DIR, "weapons.lst" );

    if ( ( fp = fopen( filename, "r" ) ) == NULL )
    {
    	bug("load_weapons: Couldn't open weapons.lst for reading!");
    	return;
	}

	CREATE( weapon, SHIP_WEAPON_DATA, 1 );

	for ( ; ; )
	{
		word   = feof( fp ) ? "END" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		case 'A':
			KEY( "Ammo",	weapon->ammo, fread_number( fp ));
			break;

		case 'D':
			KEY( "DisableChance",	weapon->disable_chance, fread_number( fp ));
			KEY( "DisableType",	weapon->disable_type, fread_number( fp ));
			break;

		case 'E':
			if ( !strcmp(word, "EndWeapon") )
			{
				LINK( weapon, first_weapon, last_weapon, next, prev);
				/* Create new weapon data */
				CREATE( weapon, SHIP_WEAPON_DATA, 1 );
				/* Set defaults */
				reset_weapon( weapon ); /* Note: resets/destroys everything BUT the links */
				/* Continue */
			}
			if ( !strcmp(word, "END") )
			{
				/* Destroy last weapon data */
				DISPOSE( weapon );
				return;
			}
			break;

		case 'I':
			KEY( "Index",	weapon->index, fread_number( fp ));
			break;

		case 'L':
			KEY( "Lag",	weapon->lag, fread_number( fp ));
			break;

		case 'M':
			KEY( "MaxDam",	weapon->maxdam, fread_number( fp ));
			KEY( "MinDam",	weapon->mindam, fread_number( fp ));
			break;

		case 'N':
			KEY( "Name",	weapon->name, fread_string( fp ));
			break;

		case 'R':
			KEY( "Range",	weapon->range, fread_number( fp ));
			break;

		case 'T':
			KEY( "ToHit",	weapon->to_hit, fread_number( fp ));
			break;
		}
	}
}

