/*
 ****************************************************************************
 *   Functions written, rewritten, and collected by Funf					*
 *									   										*
 *   Functions having to do with ships, space.c was becoming too full.		*
 ****************************************************************************
*/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

bool    autofly(SHIP_DATA *ship);
SHIP_DATA * get_ship(char *);

extern SHIP_DATA * first_ship;
extern SHIP_DATA * last_ship;

extern SPACE_DATA * first_starsystem;
extern SPACE_DATA * last_starsystem;

extern SHIP_WEAPON_DATA * first_weapon;
extern SHIP_WEAPON_DATA * last_weapon;

void calc_ship_speed( SHIP_DATA *ship );
void calc_ship_stats( SHIP_DATA *ship );
void do_installcomponent( CHAR_DATA *ch, char *argument );
void do_detachcomponent( CHAR_DATA *ch, char *argument );
void do_setship( CHAR_DATA *ch, char *argument );
void do_showship( CHAR_DATA *ch, char *argument );
void do_makeship( CHAR_DATA *ch, char *argument );
void do_copyship( CHAR_DATA *ch, char *argument );
void do_paint(CHAR_DATA *ch, char *argument );
void do_transship( CHAR_DATA *ch ,char *argument );
void do_renameship( CHAR_DATA *ch, char *argument );
void do_ships( CHAR_DATA *ch, char *argument );
void do_allships( CHAR_DATA *ch, char *argument );
long long int get_ship_value( SHIP_DATA *ship );
void write_ship_list();
void save_ship( SHIP_DATA *ship );
void fread_ship( SHIP_DATA *ship, FILE *fp );
void fread_ship_components( SHIP_DATA *ship, FILE *fp );
void fread_ship_tradegoods( SHIP_DATA *ship, FILE *fp );
bool load_ship_file( char *shipfile );
void load_ships( );

bool has_component ( SHIP_DATA *ship, int component_type )
{
	OBJ_DATA *component;

	for ( component = ship->first_component; component ; component = component->next_content )
	{
		if ( component->value[0] == component_type )
			return TRUE;
	}
	return FALSE;
}

void calc_ship_speed( SHIP_DATA *ship )
{
	int mass;
	sh_int speed, manuever;
	OBJ_DATA *component;
	SHIP_DATA *dockedship;

	mass = 0;
	speed = 0;
	manuever = 0;

	if (!ship)
	{
		bug("calc_ship_speed: NULL ship pointer!");
		return;
	}


	if(ship->first_component)
	{

		for( component = ship->first_component ; component ; component = component->next_content )
		{
			mass += component->weight;
			if ( component->value[0] == COMPONENT_ENGINE )
			{
				speed += component->value[1];
				manuever += component->value[4];
			}
			else if ( component->value[0] == COMPONENT_THRUSTER )
			{
				manuever += component->value[1];
				speed += component->value[4];
			}
		}

		/*if(ship->hangar) Annoying code! -- Funf 2007-04-04
		{

			for (dockedship = (get_room_index(ship->hangar))->first_ship ; dockedship ; dockedship = dockedship->next_in_room )
			{
				if(dockedship)
				{
					calc_ship_speed(dockedship);
					mass += dockedship->mass / 10;
				}
			}
		} */

	}

	switch(ship->class) /* Temp manuever code. Hard coded per ship class. --Funf 2007-03-06 */
	{
		case 0: manuever += 100; break; /* Fighter */
		case 1: manuever += 50;  break; /* Midship */
		case 2: manuever += 25;  break; /* Capship */
	}


	speed -= mass;

	if (speed < 0) speed = 0;

	ship->realspeed = speed;
	if(ship->currspeed > ship->realspeed)
		ship->currspeed = ship->realspeed;
	ship->manuever = manuever;
	ship->mass = mass;

	return;
}

void calc_ship_stats( SHIP_DATA *ship )
{
	int energy, hull, comm, cargo;
	sh_int ammo;
	OBJ_DATA *component;

	if (!ship)
	{
		bug("calc_ship_stats: NULL ship pointer!");
		return;
	}

	energy = hull = ammo = 0;

	comm = ship->comm; // legacy comm support

	switch(ship->class)
	{
	case MIDSIZE_SHIP:
		cargo = 1;
		break;
	case CAPITAL_SHIP:
		cargo = 5;
		break;
	default:
		cargo = 0;
	};

	if(ship->first_component)
	{

		for( component = ship->first_component ; component ; component = component->next_content )
		{
			if ( component->value[0] == COMPONENT_HULL )
			{
				hull += component->value[1];
			}
			else if ( component->value[0] == COMPONENT_FUEL )
			{
				energy += component->value[1];
			}
			else if ( component->value[0] == COMPONENT_AMMO )
			{
				ammo += component->value[1];
			}
			else if ( component->value[0] == COMPONENT_COMM )
			{
				comm += component->value[1];
			}
			else if ( component->value[0] == COMPONENT_CARGO )
			{
				cargo += component->value[1];
			}
		}

	}


	ship->comm = comm;
	ship->cargo = cargo;
	if(ship->maxhull != hull)   /* BUGFIX: Zero hull could never be repaired. Now, maxhull change = auto-ship-restore! - Funf 2007-04-03 */
		ship->hull = hull;
	ship->maxhull = hull;
	ship->maxenergy = energy;
	if(ship->energy > ship->maxenergy)
		ship->energy = ship->maxenergy;
	ship->maxammo = ammo;
	if(ship->ammo > ship->maxammo)
		ship->ammo = ship->maxammo;

	calc_ship_speed( ship );

	return;
}

/* Installs a component. Checks for class. */
void do_installcomponent( CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;
	OBJ_DATA *component, *c2;
	bool found = FALSE;
	char arg1[MAX_INPUT_LENGTH];
	int count;

	argument = one_argument( argument, arg1 );

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Syntax: installcomponent <ship> <component>\n\r", ch );
		return;
	}

	ship = get_ship( arg1 );
	if (!ship || ship->in_room != ch->in_room ) /* Ship doesn't exist or not in room */
	{
		send_to_char( "&RThat ship isn't here!&w\n\r", ch );
		return;
	}

	found = FALSE;

	if ( !IS_IMMORTAL(ch) && ( !check_pilot(ch, ship) || !strcmp(ship->owner, "Public") || !strcmp(ship->owner, "public") ) )
	{
		send_to_char( "That isn't your ship!\n\r", ch );
		return;
	}

	for( component = ch->first_carrying; component; component = component->next_content )
	{
		if ( component->item_type == ITEM_SHIPCOMPONENT && ( nifty_is_name( argument, component->name ) ) && can_see_obj( ch, component ) )
		{
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		send_to_char( "You don't have a component like that.\n\r", ch );
		return;
	}

	count = 0;

	for ( c2 = ship->first_component ; c2 ; c2 = c2->next_content )
	{
		if ( component->value[0] == c2->value[0] )
			++count;
	}

	if ( component->value[0] == COMPONENT_WEAPON && count >= 3 )
	{
		send_to_char( "You can't install more than three weapons!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_HULL && count > 1 )
	{
		send_to_char( "You can't install more than one hull!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_FUEL && count > 1 )
	{
		send_to_char( "You can't install more than one fuel tank!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_AMMO && count > 1 )
	{
		send_to_char( "You can't install more than one ammo store!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_THRUSTER && count > 1 )
	{
		send_to_char( "You can't install more than one thruster unit!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_COMM && count > 1 )
	{
		send_to_char( "You can't install more than one comm device!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_MONO && count > 1 )
	{
		send_to_char( "You can't install more than one MONO computer!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_SCANNER && count > 1 )
	{
		send_to_char( "You can't install more than one scanner!\n\r", ch);
		return;
	}

	if ( component->value[0] == COMPONENT_CARGO && count > 1 )
	{
		send_to_char( "You can't install more than one cargo container!\n\r", ch);
		return;
	}

	switch(ship->class) // Removed 2 engine mids and 3 engine capitals. 2006-10-16 Funf
	{
	case FIGHTER_SHIP:
		if (!((component->value[2]) & 1))
		{
			send_to_char( "This component can't be installed in a fighter!\n\r", ch);
			return;
		}
		if ( component->value[0] == COMPONENT_ENGINE && count >= 1 )
		{
			send_to_char( "You can't install more than one engine in a fighter!\n\r", ch);
			return;
		}
		break;
	case MIDSIZE_SHIP:
		if (!((component->value[2]) & 2))
		{
			send_to_char( "This component can't be installed in a midship!\n\r", ch);
			return;
		}
		if ( component->value[0] == COMPONENT_ENGINE && count >= 1 )
		{
			send_to_char( "You can't install more than one engine in a midship!\n\r", ch);
			return;
		}
		break;
	case CAPITAL_SHIP:
		if (!((component->value[2]) & 4))
		{
			send_to_char( "This component can't be installed in a capital ship!\n\r", ch);
			return;
		}
		if ( component->value[0] == COMPONENT_ENGINE && count >= 1 )
		{
			send_to_char( "You can't install more than one engine in a capital ship!\n\r", ch);
			return;
		}
		break;
	default:
		send_to_char( "This class of ship can't be installed with components.\n\r", ch );
		return;
	}

	switch(component->value[0])
	{
	case COMPONENT_WEAPON:
		send_to_char( "Weapon successfully installed.\n\r", ch );
		/*component->value[3] = DISABLE_NONE;*/
		component->value[4] = FALSE;
		break;
	case COMPONENT_ENGINE:
		send_to_char( "Engine successfully installed.\n\r", ch );
		break;
	case COMPONENT_HULL:
		send_to_char( "Hull plating successfully installed.\n\r", ch );
		break;
	case COMPONENT_FUEL:
		send_to_char( "Fuel tank successfully installed.\n\r", ch );
		break;
	case COMPONENT_AMMO:
		send_to_char( "Ammo store successfully installed.\n\r", ch );
		break;
	case COMPONENT_THRUSTER:
		send_to_char( "Thruster successfully installed.\n\r", ch );
		break;
	case COMPONENT_COMM:
		send_to_char( "Comm device successfully installed.\n\r", ch );
		break;
	case COMPONENT_MONO:
		send_to_char( "MONO computer successfully installed.\n\r", ch );
		break;
	case COMPONENT_SCANNER:
		send_to_char( "Scanner successfully installed.\n\r", ch );
		break;
	case COMPONENT_CARGO:
		send_to_char( "Cargo container successfully installed.\n\r", ch );
		break;
	default:
		send_to_char( "Unknown component type, please report this to an administrator\n\r", ch );
		return;
	}

	separate_obj( component );

	obj_from_char( component );

	WAIT_STATE( ch, 50 );

	LINK( component, ship->first_component, ship->last_component, next_content, prev_content);

	calc_ship_stats( ship );

	save_ship( ship );

	return;
}

void do_detachcomponent( CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;
	OBJ_DATA *component;
	bool found = FALSE;
	char arg1[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg1 );

	if( !ch )
	{
		bug("Detachcomponent: Null ch");
		return;
	}

	if( !argument || argument[0] == '\0' )
	{
		send_to_char( "Detach what?\n\r", ch );
		return;
	}

	ship = get_ship( arg1 );
	if (!ship || ship->in_room != ch->in_room ) /* Ship doesn't exist or not in room */
	{
		send_to_char( "&RThat ship isn't here!&w\n\r", ch );
		return;
	}

	found = FALSE;

	if ( !IS_IMMORTAL(ch) && ( !check_pilot(ch, ship) || !strcmp(ship->owner, "Public") || !strcmp(ship->owner, "public") ) )
	{
		send_to_char( "That isn't your ship!\n\r", ch );
		return;
	} /* Ok, we need some more granular checking for clan components. Removable by clan officers only. -Funf 2008-01-15 */
	else if (!IS_IMMORTAL(ch) && !( !str_cmp(ch->name,ship->owner) ||
			 						!str_cmp(ch->name,ship->pilot) || !str_cmp(ch->name,ship->copilot)
			 					  ) ) /* Player not explicitly allowed? Must be clan. */
	{
    	if ( !(    !str_cmp(ch->name, ch->pcdata->clan->leader)
    			|| !str_cmp(ch->name, ch->pcdata->clan->number1)
    		    || !str_cmp(ch->name, ch->pcdata->clan->number2)
    		  )
    	   )
    	{
			send_to_char( "You must be a clan officer to remove components from this ship!\n\r", ch );
        	return;
		}
   	}


	if ( ship->first_component == NULL )
	{
		send_to_char( "There are no components attached to this ship!\n\r", ch );
		return;
	}

	for( component = ship->first_component; component; component = component->next_content )
	{
		if ( component->item_type == ITEM_SHIPCOMPONENT && ( nifty_is_name( argument, component->name ) ) )
		{
			found = TRUE;
			break;
		}
	}

    if (!strcmp(argument, "all"))
    {
		component = ship->first_component;
		while( component )
		{
			UNLINK( component, ship->first_component, ship->last_component, next_content, prev_content);
			if ( component->value[0] == COMPONENT_COMM )
				ship->comm -= component->value[1]; // legacy comm support -Funf 2008-06-24
			obj_to_char( component, ch );
			component = ship->first_component; // Destructing the list means I can't traverse normally -Funf
		}
		WAIT_STATE( ch, 100 );
		send_to_char( "All components successfully detached.\n\r", ch );
		calc_ship_stats( ship );
		save_ship( ship );
		if(ch->carry_weight > can_carry_w(ch))
		{
			act(AT_RED, "Your body shakes slightly and you realize you can't hold onto all your items!",
			ch, NULL, NULL, TO_CHAR);
			act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
			do_drop(ch, "all");
		}

		return;
	}

	if (!found)
	{
		send_to_char( "The ship doesn't have any component like that.\n\r", ch );
		return;
	}

	UNLINK( component, ship->first_component, ship->last_component, next_content, prev_content);

	obj_to_char( component, ch );

	WAIT_STATE( ch, 50 );

	send_to_char( "Component successfully detached.\n\r", ch );

	calc_ship_stats( ship );

	save_ship( ship );

	return;
}


void do_setship( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    SHIP_DATA *ship;
    int  tempnum;
    ROOM_INDEX_DATA *roomindex;

    if ( IS_NPC( ch ) )
    {
		send_to_char( "Huh?\n\r", ch );
		return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
    {
		send_to_char( "Usage: setship <ship> <field> <values>\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( "filename name owner copilot pilot description home\n\r", ch );
		send_to_char( "cockpit entrance turret1 turret2 hangar\n\r", ch );
		send_to_char( "engineroom firstroom lastroom shipyard\n\r", ch );
		send_to_char( "manuever speed cargo\n\r", ch );
		send_to_char( "hull energy chaff ammo\n\r", ch );
		send_to_char( "comm sensor astroarray class\n\r", ch );
		send_to_char( "pilotseat coseat gunseat navseat\n\r", ch );
		return;
    }

    ship = get_ship( arg1 );
    if ( !ship )
    {
		send_to_char( "No such ship.\n\r", ch );
		return;
    }

    if ( !str_cmp( arg2, "owner" ) )
    {
    	CLAN_DATA *clan;
        if ( ship->type != MOB_SHIP && (clan = get_clan( ship->owner )) != NULL )
        {
        	if ( ship->class <= SHIP_PLATFORM )
            	clan->spacecraft--;
          	else
            	clan->vehicles--;
        }
		STRFREE( ship->owner );
		ship->owner = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		if ( ship->type != MOB_SHIP && (clan = get_clan( ship->owner )) != NULL )
    	{
        	if ( ship->class <= SHIP_PLATFORM )
        		clan->spacecraft++;
        	else
        	    clan->vehicles++;
        }
	return;
    }

    if ( !str_cmp( arg2, "home" ) )
    {
		STRFREE( ship->home );
		ship->home = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "pilot" ) )
    {
		STRFREE( ship->pilot );
		ship->pilot = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "copilot" ) )
    {
		STRFREE( ship->copilot );
		ship->copilot = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
	}

    if ( !str_cmp( arg2, "firstroom" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    		send_to_char("That room doesn't exist.\n\r",ch);
    		return;
    	}
		ship->firstroom = tempnum;
        ship->lastroom = tempnum;
        ship->cockpit = tempnum;
        ship->coseat = tempnum;
        ship->pilotseat = tempnum;
        ship->gunseat = tempnum;
        ship->navseat = tempnum;
        ship->entrance = tempnum;
        ship->turret1 = 0;
        ship->turret2 = 0;
        ship->hangar = 0;
		send_to_char( "You will now need to set the other rooms in the ship.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "lastroom" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    		send_to_char("That room doesn't exist.\n\r",ch);
    		return;
    	}
    	if ( tempnum < ship->firstroom )
    	{
    	   	send_to_char("The last room on a ship must be greater than or equal to the first room.\n\r",ch);
           	return;
    	}
    	if ( ship->class == FIGHTER_SHIP && (tempnum - ship->firstroom) > 5 )
    	{
    	   	send_to_char("Starfighters may have up to 5 rooms only.\n\r",ch);
    	   	return;
    	}
		if ( ship->class == MIDSIZE_SHIP && (tempnum - ship->firstroom) > 25 )
    	{
    	   	send_to_char("Midships may have up to 25 rooms only.\n\r",ch);
    	   	return;
    	}
		if ( ship->class == CAPITAL_SHIP && (tempnum - ship->firstroom) > 100 )
    	{
    	   	send_to_char("Capital Ships may have up to 100 rooms only.\n\r",ch);
    	   	return;
    	}
		ship->lastroom = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "cockpit" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
    	if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship... \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		if ( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hangar )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->cockpit = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "pilotseat" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
    	if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship... \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		if ( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hangar )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->pilotseat = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }
    if ( !str_cmp( arg2, "coseat" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
    	if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship... \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		if ( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hangar )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->coseat = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }
    if ( !str_cmp( arg2, "navseat" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    		send_to_char("That room doesn't exist.\n\r",ch);
    		return;
    	}
    	if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship... \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		if ( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hangar )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->navseat = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }
    if ( !str_cmp( arg2, "gunseat" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
    	if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship... \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		if ( tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hangar )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->gunseat = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "entrance" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
    	if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship... \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		ship->entrance = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "turret1" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
		if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
    	if ( ship->class == FIGHTER_SHIP )
    	{
    	   	send_to_char("Starfighters can't have extra laser turrets.\n\r",ch);
    	   	return;
    	}
		if ( tempnum == ship->cockpit || tempnum == ship->entrance 	||
    	     tempnum == ship->turret2 || tempnum == ship->hangar 	|| tempnum == ship->engineroom )
    	{
    		send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->turret1 = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "turret2" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
		if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
		if ( ship->class == FIGHTER_SHIP )
    	{
    	   	send_to_char("Starfighters can't have extra laser turrets.\n\r",ch);
    	   	return;
    	}
		if ( tempnum == ship->cockpit || tempnum == ship->entrance ||
    	     tempnum == ship->turret1 || tempnum == ship->hangar || tempnum == ship->engineroom )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->turret2 = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "hangar" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
		if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
    	if ( tempnum == ship->cockpit || tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->engineroom )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		if ( ship->class == FIGHTER_SHIP )
		{
	   		send_to_char("Monopods are too small to have hangars for other ships!\n\r",ch);
	   		return;
		}
		ship->hangar = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

   if ( !str_cmp( arg2, "engineroom" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.\n\r",ch);
    	   	return;
    	}
		if ( tempnum < ship->firstroom || tempnum > ship->lastroom )
    	{
    	   	send_to_char("That room number is not in that ship .. \n\rIt must be between Firstroom and Lastroom.\n\r",ch);
           	return;
    	}
    	if ( tempnum == ship->cockpit || tempnum == ship->entrance ||
    	     tempnum == ship->turret1 || tempnum == ship->turret2 || tempnum == ship->hangar )
    	{
    	   	send_to_char("That room is already being used by another part of the ship\n\r",ch);
           	return;
    	}
		ship->engineroom = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "shipyard" ) )
    {
        tempnum = atoi(argument);
    	roomindex = get_room_index(tempnum);
    	if (roomindex == NULL)
    	{
    	   	send_to_char("That room doesn't exist.",ch);
    	   	return;
    	}
		ship->shipyard = tempnum;
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
		if ( !str_cmp( argument, "republic" ) )
	  		ship->type = SHIP_REPUBLIC;
		else
		if ( !str_cmp( argument, "imperial" ) )
	  		ship->type = SHIP_IMPERIAL;
		else
		if ( !str_cmp( argument, "civilian" ) )
	  		ship->type = SHIP_CIVILIAN;
		else
		if ( !str_cmp( argument, "mob" ) )
	 	 	ship->type = MOB_SHIP;
		else
		{
	   		send_to_char( "Ship type must be either: republic, imperial, civilian or mob.\n\r", ch );
	   		return;
		}
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
		STRFREE( ship->name );
		ship->name = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
		DISPOSE( ship->filename );
		ship->filename = str_dup( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		write_ship_list( );
		return;
    }

    if ( !str_cmp( arg2, "desc" ) )
    {
		STRFREE( ship->description );
		ship->description = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "manuever" ) )
    {
		ship->manuever = URANGE( 0, atoi(argument) , 10000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "class" ) )
    {
		if ( !str_prefix(argument, "fighter") )
			ship->class = FIGHTER_SHIP;
		else if ( !str_prefix(argument, "midship") )
			ship->class = MIDSIZE_SHIP;
		if ( !str_prefix(argument, "capitalship") )
			ship->class = CAPITAL_SHIP;
		else
			ship->class = URANGE( 0, atoi(argument) , 9 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "speed" ) )
    {
		ship->realspeed = URANGE( 0, atoi(argument) , 2000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "hull" ) )
    {
		ship->hull = URANGE( 1, atoi(argument) , 500000 );
		ship->maxhull = URANGE( 1, atoi(argument) , 500000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "energy" ) || !str_cmp( arg2, "fuel" ) )
    {
		ship->energy = URANGE( 1, atoi(argument) , 1000000 );
		ship->maxenergy = URANGE( 1, atoi(argument) , 1000000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "sensor" ) )
    {
		ship->sensor = URANGE( 0, atoi(argument) , 10000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "astroarray" ) )
    {
		ship->astro_array = URANGE( 0, atoi(argument) , 10000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    if ( !str_cmp( arg2, "comm" ) )
    {
		ship->comm = URANGE( 0, atoi(argument) , 10000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

/*    if ( !str_cmp( arg2, "firstweapon" ) )
    {
        ship->firstweapon = URANGE( 0, atoi(argument) , 10 );
        send_to_char( "Done.\n\r", ch );
        save_ship( ship );
        return;
    }
    if ( !str_cmp( arg2, "secondweapon" ) )
    {
        ship->secondweapon = URANGE( 0, atoi(argument) , 10 );
        send_to_char( "Done.\n\r", ch );
        save_ship( ship );
        return;
    }
    if ( !str_cmp( arg2, "thirdweapon" ) )
    {
        ship->thirdweapon = URANGE( 0, atoi(argument) , 10 );
        send_to_char( "Done.\n\r", ch );
        save_ship( ship );
        return;
    }*/

    if ( !str_cmp( arg2, "ammo" ) )
    {
        ship->ammo = URANGE( 1, atoi(argument) , 50000 );
        ship->maxammo = URANGE( 1, atoi(argument) , 50000 );
        send_to_char( "Done.\n\r", ch );
        save_ship( ship );
        return;
    }

    if ( !str_cmp( arg2, "chaff" ) )
    {
		ship->chaff = URANGE( 0, atoi(argument) , 500 );
		ship->maxchaff = URANGE( 0, atoi(argument) , 500 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }
    if ( !str_cmp( arg2, "cargo" ) )
    {
		ship->cargo = URANGE( 0, atoi(argument) , 1000 );
		send_to_char( "Done.\n\r", ch );
		save_ship( ship );
		return;
    }

    do_setship( ch, "" );
    return;
}

void do_showship( CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;

    if ( IS_NPC( ch ) )
    {
		send_to_char( "Huh?\n\r", ch );
		return;
    }

    if ( argument[0] == '\0' )
    {
		send_to_char( "Usage: showship <ship>\n\r", ch );
		return;
    }

    ship = get_ship( argument );
    if ( !ship )
    {
		send_to_char( "No such ship.\n\r", ch );
		return;
    }
    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "%s %s : %s\n\rFilename: %s\n\r",
			   ship->type == SHIP_REPUBLIC ? "New Republic" :
		      (ship->type == SHIP_IMPERIAL ? "Imperial" :
		      (ship->type == SHIP_CIVILIAN ? "Civilian" : "Mob" ) ),
		       ship->class == FIGHTER_SHIP ? "Starfighter" :
		      (ship->class == MIDSIZE_SHIP ? "Midship" :
		      (ship->class == CAPITAL_SHIP ? "Capital Ship" :
		      (ship->class == SHIP_PLATFORM ? "Platform" :
		      (ship->class == CLOUD_CAR ? "Cloudcar" :
		      (ship->class == OCEAN_SHIP ? "Boat" :
		      (ship->class == LAND_SPEEDER ? "Speeder" :
		      (ship->class == WHEELED ? "Wheeled Transport" :
		      (ship->class == LAND_CRAWLER ? "Crawler" :
		      (ship->class == WALKER ? "Walker" : "Unknown" ) ) ) ) ) ) ) ) ),
    		   ship->name,
    		   ship->filename);
    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "Home: %s   Description: %s\n\rOwner: %s   Pilot: %s   Copilot: %s\n\r",
    		   ship->home,  ship->description,
    		   ship->owner, ship->pilot,  ship->copilot );
    ch_printf( ch, "Firstroom: %d   Lastroom: %d\n\r",
               ship->firstroom,
               ship->lastroom);
    ch_printf( ch, "Cockpit: %d   Entrance: %d   Hangar: %d  Engineroom: %d\n\r",
        	   ship->cockpit,
     		   ship->entrance,
    		   ship->hangar,
    		   ship->engineroom);
    ch_printf( ch, "Pilotseat: %d   Coseat: %d   Navseat: %d  Gunseat: %d\n\r",
    		   ship->pilotseat,
    		   ship->coseat,
    		   ship->navseat,
    		   ship->gunseat);
    ch_printf( ch, "Location: %d   Lastdoc: %d   Shipyard: %d\n\r",
    		   ship->location,
    		   ship->lastdoc,
    		   ship->shipyard);
    ch_printf( ch, "Comm: %d   Sensor: %d   Astro Array: %d\n\r",
     		   ship->comm,
    		   ship->sensor,
    		   ship->astro_array);
    ch_printf( ch, "Hull: %d/%d  Ship Condition: %s  MONO: %s\n\r",
               ship->hull, ship->maxhull,
    		   ship->shipstate == SHIP_DISABLED ? "Disabled" : "Running",
    		   ship->mono_disabled ? "Disabled" : "Active");
    ch_printf( ch, "Energy(fuel): %d/%d   Ammo: %d/%d   Chaff: %d/%d\n\r",
    		   ship->energy,
    		   ship->maxenergy,
    		   ship->ammo,
    		   ship->maxammo,
    		   ship->chaff,
    		   ship->maxchaff);
    ch_printf( ch, "Current Coordinates: %.0f %.0f %.0f\n\r",
               ship->vx, ship->vy, ship->vz );
    ch_printf( ch, "Current Heading: %.0f %.0f %.0f\n\r",
               ship->hx, ship->hy, ship->hz );
    ch_printf( ch, "Speed: %d/%d   Manueverability: %d  Evading: %d",
               ship->currspeed, ship->realspeed, ship->manuever, ship->evading );
    ch_printf( ch, "Max Cargo: %d",
               ship->cargo );
	return;
}

void do_makeship( CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *ship;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( !argument || argument[0] == '\0' )
    {
		send_to_char( "Usage: makeship <filename> <ship name>\n\r", ch );
		return;
    }

    CREATE( ship, SHIP_DATA, 1 );
    LINK( ship, first_ship, last_ship, next, prev );

    ship->name			= STRALLOC( argument );
    ship->description	= STRALLOC( "" );
    ship->owner 		= STRALLOC( "" );
    ship->copilot       = STRALLOC( "" );
    ship->pilot         = STRALLOC( "" );
    ship->home          = STRALLOC( "" );
    ship->type          = SHIP_CIVILIAN;
    ship->starsystem 	= NULL;
    ship->energy 		= ship->maxenergy;
    ship->hull 			= ship->maxhull;
    ship->in_room		= NULL;
    ship->next_in_room	= NULL;
    ship->prev_in_room	= NULL;
    ship->currjump		= NULL;
    ship->target0		= NULL;
    ship->target1		= NULL;
    ship->target2		= NULL;
    ship->first_component	= NULL;
    ship->last_component	= NULL;
    ship->cargo			= 0;
    ship->first_tradegood	= NULL;
    ship->last_tradegood	= NULL;

    ship->filename 		= str_dup( arg );
    save_ship( ship );
    write_ship_list( );

}

void do_copyship( CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;
    SHIP_DATA *old;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( !argument || argument[0] == '\0' )
    {
		send_to_char( "Usage: copyship <oldshipname> <filename> <newshipname>\n\r", ch );
		return;
    }

    old = get_ship ( arg );

    if (!old)
    {
		send_to_char( "Thats not a ship!\n\r", ch );
		return;
    }

    CREATE( ship, SHIP_DATA, 1 );
    LINK( ship, first_ship, last_ship, next, prev );

    ship->name			= STRALLOC( argument );
    ship->description	= STRALLOC( "" );
    ship->owner 		= STRALLOC( "" );
    ship->copilot       = STRALLOC( "" );
    ship->pilot         = STRALLOC( "" );
    ship->home          = STRALLOC( "" );
    ship->type          = old->type;
    ship->class         = old->class;
    ship->lasers        = old->lasers  ;
    ship->maxmissiles   = old->maxmissiles  ;
    ship->maxrockets    = old->maxrockets  ;
    ship->maxtorpedos   = old->maxtorpedos  ;
    ship->maxhull       = old->maxhull  ;
    ship->maxenergy     = old->maxenergy  ;
    ship->hyperspeed    = old->hyperspeed  ;
    ship->maxchaff      = old->maxchaff  ;
    ship->realspeed     = old->realspeed  ;
    ship->manuever      = old->manuever  ;
    ship->in_room		= NULL;
    ship->next_in_room	= NULL;
    ship->prev_in_room	= NULL;
    ship->currjump		= NULL;
    ship->target0		= NULL;
    ship->target1		= NULL;
    ship->target2		= NULL;
    ship->first_component	= NULL;
    ship->last_component	= NULL;
    ship->cargo			= 0;
    ship->first_tradegood	= NULL;
    ship->last_tradegood	= NULL;

    ship->filename      = str_dup(arg2);
    save_ship( ship );
    write_ship_list();
}


void do_paint(CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
    SHIP_DATA *ship;

	if ( IS_NPC(ch) )
		return;

    argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "&RUsage: Paint <ship name> <color> <desc>\n\r&w", ch);
		return;
	}

	if ( arg2[0] == '\0' && ( str_cmp(arg2, "Blood")
	     && str_cmp(arg2, "Green")	&& str_cmp(arg2, "White")
		 && str_cmp(arg2, "DBlue") 	&& str_cmp(arg2, "Cyan")
		 && str_cmp(arg2, "LRed") 	&& str_cmp(arg2, "LGreen")
		 && str_cmp(arg2, "LBlue") 	&& str_cmp(arg2, "Blue")
		 && str_cmp(arg2, "Brown") 	&& str_cmp(arg2, "Purple")
		 && str_cmp(arg2, "Yellow") && str_cmp(arg2, "Pink") ) )
	{
		send_to_char( "&RColors Are:\n\r&B",ch);
		send_to_char( "&B<&RBlood&B> <&RGreen&B> <&RDBlue&B> <&RCyan&B> <&RLRed&B> \n\r&w",ch);
		send_to_char( "&B<&RLGreen&B> <&RLBlue&B> <&RBlue&B> <&RBrown&B> <&RPurple&B> \n\r&w",ch);
		send_to_char( "&B<&RYellow&B> <&RPink&B> <&RWhite&B> \n\r&w",ch);
		return;
	}
	if ( argument[0] == '\0' )
	{
		send_to_char( "&RUsage: Paint <ship name> <color> <desc>\n\r&w", ch);
		send_to_char( "&RYou must include a description\n\r&w",ch);
		return;
	}

	ship = ship_in_room( ch->in_room , arg1 );
	if ( !ship )
	{
		send_to_char("No Such Ship",ch);
		return;
	}

	if ( ship->class > SHIP_PLATFORM )
	{
		send_to_char("&RThis isn't a spacecraft!\n\r",ch);
		return;
	}

	if (ship->shipstate != SHIP_DOCKED)
	{
		send_to_char("&RYour name can only be changed when your docked!\n\r",ch);
		return;
	}



	if ( !ship->owner || !ship->pilot || !ship->copilot || check_pilot(ch,ship ))
	{
		STRFREE( ship->description );
		ship->description = STRALLOC( argument );
		STRFREE( ship->color );
		ship->color = STRALLOC( arg2 );
		save_ship( ship );
		sprintf(buf, "%s has been repainted.", ship->name);
		send_to_char(buf,ch);
		act(AT_PLAIN,"$n repaints a ship..",ch,NULL,argument,TO_ROOM); /* Room echo - Funf */
		return;
	}
	else
	{
		sprintf(buf, "Sorry, %s, You are not able to paint that ship, try it when your a pilot.",ch->name);
		send_to_char(buf,ch);
		return;
	}
    do_setship( ch, "" );
    return;
}


void do_transship( CHAR_DATA *ch ,char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int arg3;
    SHIP_DATA *ship;

    if ( IS_NPC( ch ) )
    {
		send_to_char( "Huh?\n\r", ch );
		return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    ship = get_ship( arg1 );
	if ( !ship )
    {
		send_to_char( "No such ship.\n\r", ch );
		return;
    }

	arg3 = atoi( arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
    {
		send_to_char( "Usage: transship <ship> <vnum>\n\r", ch );
		return;
    }

	ship->shipyard = arg3;
	ship->shipstate = SHIP_READY;

    if ( ship->class != SHIP_PLATFORM && ship->type != MOB_SHIP )
    {
    	extract_ship( ship );
        ship_to_room( ship , ship->shipyard );

        ship->location = ship->shipyard;
        ship->lastdoc = ship->shipyard;
        ship->shipstate = SHIP_DOCKED;
    }

    if (ship->starsystem)
    	ship_from_starsystem( ship, ship->starsystem );

    save_ship(ship);
	send_to_char( "Ship Transferred.\n\r", ch );
}

void do_renameship( CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;
   	CLAN_DATA *clan;
   	char *classinfo, *temp;

    if ( (ship = ship_from_cockpit( ch->in_room->vnum ) ) == NULL)
    {
    	send_to_char( "You must be in the cockpit of a ship to do that!\n\r", ch);
        return;
    }

    if( ( (clan = get_clan(ship->owner)) == NULL ) || str_cmp( clan->leader, ch->name ) )
    	if( str_cmp( ship->owner, ch->name ) )
        {
        	send_to_char( "&RYou do not own this ship.\n\r", ch);
        	return;
        }

    if( get_ship( argument ) != NULL )
    {
    	send_to_char( "&RA ship already exists of that name.\n\r", ch);
        return;
    }

    if( ch->gold < 10000000 )
    {
        send_to_char( "&RYou do not have enough wulongs for this request.\n\r", ch);
        return;
    }

    if( is_profane( argument ) )
    {
		send_to_char( "&RYour name has been deemed profane.\n\r", ch);
		return;
	}

	ch->gold -= 10000000;

	temp = STRALLOC( ship->name ); /* Save old name */
	STRFREE( ship->name ); /* Delete old name */
    strcat( argument, strrchr( temp, ',' ) ); /* Move class info over. POSSIBLE BUFFER OVERFLOW! -Funf */
    ship->name = STRALLOC( argument ); /* Assign new name */
    STRFREE( temp ); /* Free temp string */
    save_ship( ship );
    send_to_char( "&GTransaction Complete. Name changed.\n\r", ch );
    return;
}


void do_ships( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    SHIP_DATA *ship;
    CHAR_DATA *victim;
    int count;
    OBJ_DATA *obj, *obj_next;
    bool found;


    if ( !IS_NPC(ch) )
    {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
    	    if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
    	    {
    	            found = TRUE;
    	    }
    	    if(found)
    	            break;
         }
    	if(!found && !IS_IMMORTAL(ch))
    	{
		send_to_char("&RYou need a computer to check this!\r\n", ch);
		return;
	}

	// Set victim to argument, default to caller.
	argument = one_argument( argument, arg1 );
	if (arg1[0] == '\0')
		strcpy(arg1, ch->name);
	victim = get_char_world(ch, arg1 );
	if (!victim)
		victim = ch;

	count = 0;
	send_to_char( "&YShips owned by the player or their organization:\n\r", ch );
	send_to_char( "\n\r&WShip                               Owner\n\r",ch);
	for ( ship = first_ship; ship; ship = ship->next )
	{
		// Does this ship belong to the specified player, if so, just print it.
		if ( str_cmp(ship->owner, arg1) )
		{
			// It doesn't belong to the player. Don't print if unclanned or the clan doesn't match.
			if ( !victim->pcdata || !victim->pcdata->clan || str_cmp(ship->owner,victim->pcdata->clan->name) || ship->class > SHIP_PLATFORM )
				continue;
			// Don't show a player's clan's ships unless looking at yourself or you are an imm.
			if ( !IS_IMMORTAL(ch) && ch != victim )
				continue;
		}

		if (ship->type == MOB_SHIP)
			continue;
		else if (ship->type == SHIP_REPUBLIC)
			set_char_color( AT_BLOOD, ch );
		else if (ship->type == SHIP_IMPERIAL)
			set_char_color( AT_DGREEN, ch );
		else
			set_char_color( AT_BLUE, ch );
		if  ( ship->in_room )
			ch_printf( ch, "%-35s %-15s (%s)\n\r", ship->name, ship->owner, ship->in_room->name );
		else
			ch_printf( ch, "%-35s %-15s\n\r", ship->name, ship->owner );

		count++;

	}

	if ( !count )
		send_to_char( "There are no ships owned by them.\n\r", ch );
	}


    count =0;
    send_to_char( "&Y\n\rThe following ships are docked here:\n\r", ch );

    send_to_char( "\n\r&WShip                               Owner \n\r", ch );
    for ( ship = first_ship; ship; ship = ship->next )
    {
    	if ( ship->location != ch->in_room->vnum || ship->class > SHIP_PLATFORM)
        	continue;

        if (ship->type == MOB_SHIP)
        	continue;
        else if (ship->type == SHIP_REPUBLIC)
           	set_char_color( AT_BLOOD, ch );
        else if (ship->type == SHIP_IMPERIAL)
           	set_char_color( AT_DGREEN, ch );
        else
          	set_char_color( AT_BLUE, ch );

        ch_printf( ch, "%-35s %-15s", ship->name, ship->owner );
        if (ship->type == MOB_SHIP || ship->class == SHIP_PLATFORM )
        {
          	ch_printf( ch, "\n\r");
          	continue;
        }
        if ( !str_cmp(ship->owner, "Public") )
        {
          	ch_printf( ch, "%lld to rent.\n\r", get_ship_value(ship)/100 );
        }
        else if ( str_cmp(ship->owner, "") )
          	ch_printf( ch, "%s", "\n\r" );
        else
		ch_printf( ch, "\n\r");
/*           	ch_printf( ch, "%lld to buy.\n\r", get_ship_value(ship) );
		Removed because of current Ship Auction system */
        count++;
    }

    if ( !count )
    {
        send_to_char( "There are no ships docked here.\n\r", ch );
    }
}

void do_allships( CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;
    int count = 0;

    count = 0;
    send_to_char( "&Y\n\rThe following ships are currently formed:\n\r", ch );

    send_to_char( "\n\r&WShip                               Owner\n\r", ch );

    if ( IS_IMMORTAL( ch ) )
    	for ( ship = first_ship; ship; ship = ship->next )
    	    if (ship->type == MOB_SHIP)
            	ch_printf( ch, "&w%-35s %-15s\n\r", ship->name, ship->owner );

    for ( ship = first_ship; ship; ship = ship->next )
    {
    	if ( ship->class > SHIP_PLATFORM )
        	continue;

        if (ship->type == MOB_SHIP)
           	continue;
        else if (ship->type == SHIP_REPUBLIC)
           	set_char_color( AT_BLOOD, ch );
        else if (ship->type == SHIP_IMPERIAL)
           	set_char_color( AT_DGREEN, ch );
        else
          	set_char_color( AT_BLUE, ch );

        ch_printf( ch, "%-35s %-15s ", strip_color(ship->name), ship->owner );
        if (ship->type == MOB_SHIP || ship->class == SHIP_PLATFORM )
        {
        	ch_printf( ch, "\n\r");
          	continue;
        }
        if ( !str_cmp(ship->owner, "Public") )
        {
          	ch_printf( ch, "%lld to rent.\n\r", get_ship_value(ship)/100 );
        }
        else if ( str_cmp(ship->owner, "") )
          	ch_printf( ch, "%s", "\n\r" );
        else
		ch_printf( ch, "\n\r");
        //	ch_printf( ch, "%lld to buy.\n\r", get_ship_value(ship));

      	count++;

	}

    if ( !count )
    {
    	send_to_char( "There are no ships currently formed.\n\r", ch );
		return;
    }
	set_char_color(AT_WHITE, ch);
       ch_printf(ch, "\r\n&WTheir are currently %d ships formed.\r\n", count);
}

long long int get_ship_value( SHIP_DATA *ship )
{
	long long int price;

    if (ship->class == FIGHTER_SHIP)
    	price = 5000;
    else if (ship->class == MIDSIZE_SHIP)
    	price = 50000;
    else if (ship->class == CAPITAL_SHIP)
        price = 500000;
    else
        price = 2000;

    if ( ship->class <= CAPITAL_SHIP )
    	price += ( ship->manuever*100*(1+ship->class) );

    price += ( ship->tractorbeam * 100 );
    price += ( ship->realspeed * 10 );
    price += ( ship->astro_array *5 );
    price += ( 5 * ship->maxhull );
    price += ( 2 * ship->maxenergy );
    price += ( 100 * ship->maxchaff );

    if (ship->maxenergy > 5000 )
    	price += ( (ship->maxenergy-5000)*20 ) ;
    if (ship->maxenergy > 10000 )
    	price += ( (ship->maxenergy-10000)*50 );
    if (ship->maxhull > 1000)
        price += ( (ship->maxhull-1000)*10 );
    if (ship->maxhull > 10000)
        price += ( (ship->maxhull-10000)*20 );
    if (ship->realspeed > 100 )
        price += ( (ship->realspeed-100)*500 ) ;
    if (ship->lasers > 5 )
        price += ( (ship->lasers-5)*500 );
    if (ship->lasers)
     	price += ( 500 + 500 * ship->lasers );
    if (ship->maxmissiles)
     	price += ( 1000 + 100 * ship->maxmissiles );
    if (ship->maxrockets)
     	price += ( 2000 + 200 * ship->maxmissiles );
    if (ship->maxtorpedos)
     	price += ( 1500 + 150 * ship->maxmissiles );
    if (ship->maxammo)
		price += ( 100 + 100 * ship->maxammo );

    if (ship->missiles )
     	price += ( 250 * ship->missiles );
    else if (ship->torpedos )
     	price += ( 500 * ship->torpedos );
    else if (ship->rockets )
        price += ( 1000 * ship->rockets );

    if (ship->turret1)
        price += 5000;

    if (ship->turret2)
        price += 5000;

    if (ship->hyperspeed)
        price += ( 1000 + ship->hyperspeed * 10 );

    if (ship->hangar)
        price += ( ship->class == MIDSIZE_SHIP ? 50000 : 100000 );

	if (ship->cargo)
		price += ship->cargo * 20000;

    price *= 1.5;

    return price;

}

void write_ship_list( )
{
    SHIP_DATA *tship;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", SHIP_DIR, SHIP_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
    	bug( "FATAL: cannot open ship.lst for writing!\n\r", 0 );
        return;
    }
    for ( tship = first_ship; tship; tship = tship->next )
    	fprintf( fpout, "%s\n", tship->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}


void save_ship( SHIP_DATA *ship )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *component;
    TRADEGOOD_DATA *good;

    if ( !ship )
    {
		bug( "save_ship: null ship pointer!", 0 );
		return;
    }

    if ( !ship->filename || ship->filename[0] == '\0' )
    {
		sprintf( buf, "save_ship: %s has no filename", ship->name );
		bug( buf, 0 );
		return;
    }

    sprintf( filename, "%s%s", SHIP_DIR, ship->filename );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_ship: fopen", 0 );
    	perror( filename );
    }
    else
    {
		fprintf( fp, "#SHIP\n" );
		fprintf( fp, "Name         %s~\n",	ship->name		);
		fprintf( fp, "Filename     %s~\n",	ship->filename		);
        fprintf( fp, "Description  %s~\n",	ship->description	);
		fprintf( fp, "Owner        %s~\n",	ship->owner		);
		fprintf( fp, "Pilot        %s~\n",      ship->pilot             );
		fprintf( fp, "Copilot      %s~\n",      ship->copilot           );
		fprintf( fp, "Class        %d\n",	ship->class		);
		fprintf( fp, "Tractorbeam  %d\n",	ship->tractorbeam	);
		fprintf( fp, "Shipyard     %d\n",	ship->shipyard		);
		fprintf( fp, "Hangar       %d\n",	ship->hangar    	);
		fprintf( fp, "Turret1      %d\n",	ship->turret1		);
		fprintf( fp, "Turret2      %d\n",	ship->turret2		);
		fprintf( fp, "Statet0      %d\n",	ship->statet0		);
		fprintf( fp, "Statet1      %d\n",	ship->statet1		);
		fprintf( fp, "Statet2      %d\n",	ship->statet2		);
		fprintf( fp, "Lasers       %d\n",	ship->lasers    	);
		fprintf( fp, "Missiles     %d\n",	ship->missiles		);
		fprintf( fp, "Maxmissiles  %d\n",	ship->maxmissiles	);
		fprintf( fp, "Rockets     %d\n",	ship->rockets		);
		fprintf( fp, "Maxrockets  %d\n",	ship->maxrockets	);
		fprintf( fp, "Torpedos     %d\n",	ship->torpedos		);
		fprintf( fp, "Maxtorpedos  %d\n",	ship->maxtorpedos	);
		fprintf( fp, "Lastdoc      %d\n",	ship->lastdoc		);
		fprintf( fp, "Firstroom    %d\n",	ship->firstroom		);
		fprintf( fp, "Lastroom     %d\n",	ship->lastroom		);
		fprintf( fp, "Shield       %d\n",	0					);
		fprintf( fp, "Maxshield    %d\n",	0					);
		fprintf( fp, "Hull         %d\n",	ship->hull		);
		fprintf( fp, "Maxhull      %d\n",	ship->maxhull		);
		fprintf( fp, "Maxenergy    %d\n",	ship->maxenergy		);
        fprintf( fp, "Maxammo      %d\n",       ship->maxammo 		);
        fprintf( fp, "Ammo         %d\n",       ship->ammo              );
        fprintf( fp, "Hyperspeed   %d\n",	 ship->hyperspeed	);
		fprintf( fp, "Comm         %d\n",	ship->comm		);
		fprintf( fp, "Chaff        %d\n",	ship->chaff		);
		fprintf( fp, "Maxchaff     %d\n",	ship->maxchaff		);
		fprintf( fp, "Sensor       %d\n",	ship->sensor		);
		fprintf( fp, "Astro_array  %d\n",	ship->astro_array	);
		fprintf( fp, "Realspeed    %d\n",	ship->realspeed		);
		fprintf( fp, "Type         %d\n",	ship->type		);
		fprintf( fp, "Cockpit      %d\n",	ship->cockpit		);
		fprintf( fp, "Coseat       %d\n",	ship->coseat		);
		fprintf( fp, "Pilotseat    %d\n",	ship->pilotseat		);
		fprintf( fp, "Gunseat      %d\n",	ship->gunseat		);
		fprintf( fp, "Navseat      %d\n",	ship->navseat		);
		fprintf( fp, "Engineroom   %d\n",       ship->engineroom        );
		fprintf( fp, "Entrance     %d\n",       ship->entrance          );
		fprintf( fp, "Shipstate    %d\n",	ship->shipstate		);
		fprintf( fp, "Missilestate %d\n",	ship->missilestate	);
		fprintf( fp, "Energy       %d\n",	ship->energy		);
		fprintf( fp, "Manuever     %d\n",       ship->manuever          );
		fprintf( fp, "Mass         %d\n",       ship->mass          );
		fprintf( fp, "Cargo        %d\n",       ship->cargo          );
		fprintf( fp, "Home         %s~\n",      ship->home              );
	/* Failed attempt at making it so ships re-loaded in their correct spots
		fprintf( fp, "WasFlying	   %d\n",	ship->wasflying		);
		if(ship->wasflying == 1)
		{
			fprintf( fp, "VXSetting		%f\n",	ship->vx);
			fprintf( fp, "VYSetting		%f\n",	ship->vy);
			fprintf( fp, "VZSetting		%f\n",	ship->vz);
			fprintf( fp, "HXSetup		%f\n",	ship->hx);
			fprintf( fp, "HYSetup		%f\n",	ship->hy);
			fprintf( fp, "HZSetup		%f\n",	ship->hz);
			fprintf( fp, "Currspeed		%d\n",	ship->currspeed);
			fprintf( fp, "Hyperdistance	%d\n",	ship->hyperdistance);
			fprintf( fp, "Shipstate		%d\n",	ship->shipstate);
			fprintf( fp, "Currjump		%s~\n",	ship->currjump);
			fprintf( fp, "System		%s~\n",	ship->starsystem->name);
			fprintf( fp, "Location		%d\n",	ship->location);
		}
	*/
		fprintf( fp, "End\n\n"						);
		fprintf( fp, "#COMPONENTS\n"						); /* Components added by Funf */
		for ( component = ship->first_component ; component ; component = component->next_content )
		{
			fprintf( fp, "Vnum         %d\n",		component->pIndexData->vnum	);
		}
		fprintf( fp, "End\n\n"			);
		if(ship->first_tradegood)
			fprintf( fp, "#TRADEGOODS\n"						); /* Trade goods added by Funf 2010-05-18 */
		for ( good = ship->first_tradegood ; good ; good = good->next )
		{
			fprintf( fp, "Origin       %s~\n",		good->origin	);
			fprintf( fp, "Type         %d\n",		good->type	);
			fprintf( fp, "Value        %lld\n",		good->value	);
			fprintf( fp, "End\n",		good->value	);
		}
		fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual ship data.
 */

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

void fread_ship( SHIP_DATA *ship, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int dummy_number;


    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

        case 'A':
	     KEY( "Ammo",      ship->ammo,        fread_number( fp ) );
             KEY( "Astro_array",      ship->astro_array,       fread_number( fp ) );
             break;

        case 'C':
	  //   KEY( "Currspeed",   ship->currspeed,	fread_number( fp ) );
             KEY( "Cargo",       ship->cargo,      fread_number( fp ) );
             KEY( "Cockpit",     ship->cockpit,          fread_number( fp ) );
             KEY( "Coseat",     ship->coseat,          fread_number( fp ) );
             KEY( "Class",       ship->class,            fread_number( fp ) );
             KEY( "Copilot",     ship->copilot,          fread_string( fp ) );
             KEY( "Comm",        ship->comm,      fread_number( fp ) );
             KEY( "Chaff",       ship->chaff,      fread_number( fp ) );
             break;


	case 'D':
	    KEY( "Description",	ship->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "Engineroom",    ship->engineroom,      fread_number( fp ) );
	    KEY( "Entrance",	ship->entrance,	        fread_number( fp ) );
	    KEY( "Energy",      ship->energy,        fread_number( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!ship->home)
		  ship->home		= STRALLOC( "" );
		if (!ship->name)
		  ship->name		= STRALLOC( "" );
		if (!ship->owner)
		  ship->owner		= STRALLOC( "" );
		if (!ship->description)
		  ship->description 	= STRALLOC( "" );
		if (!ship->copilot)
		  ship->copilot 	= STRALLOC( "" );
		if (!ship->pilot)
		  ship->pilot   	= STRALLOC( "" );
		if (ship->shipstate != SHIP_DISABLED)
		  ship->shipstate = SHIP_DOCKED;
		if (ship->statet0 != LASER_DAMAGED)
		  ship->statet0 = LASER_READY;
		if (ship->statet1 != LASER_DAMAGED)
		  ship->statet1 = LASER_READY;
		if (ship->statet2 != LASER_DAMAGED)
		  ship->statet2 = LASER_READY;
		if (ship->missilestate != MISSILE_DAMAGED)
		  ship->missilestate = MISSILE_READY;
		if (ship->shipyard <= 0)
		  ship->shipyard = ROOM_LIMBO_SHIPYARD;
		if (ship->lastdoc <= 0)
		  ship->lastdoc = ship->shipyard;
		ship->bayopen     = FALSE;
		ship->autopilot   = FALSE;
		ship->hatchopen = FALSE;
		ship->mono_disabled = FALSE;
		ship->evading = 0;
		if (ship->navseat <= 0)
		  ship->navseat = ship->cockpit;
		if (ship->gunseat <= 0)
		  ship->gunseat = ship->cockpit;
		if (ship->coseat <= 0)
		  ship->coseat = ship->cockpit;
		if (ship->pilotseat <= 0)
		  ship->pilotseat = ship->cockpit;
		if (ship->missiletype == 1)
		{
		  ship->torpedos = ship->missiles;    /* for back compatability */
		  ship->missiles = 0;
		}
		if (ship->cargo <= 0)
		  ship->cargo = 0;
		ship->starsystem = NULL;
		ship->ammo = ship->maxammo;
		ship->energy = ship->maxenergy;
		ship->hull = ship->maxhull;
		ship->in_room=NULL;
                ship->next_in_room=NULL;
                ship->prev_in_room=NULL;

		return;
	    }
	    break;

	case 'F':
	    KEY( "Filename",	ship->filename,		fread_string_nohash( fp ) );
        KEY( "Firstroom",   ship->firstroom,        fread_number( fp ) );
        break;

    case 'G':
        KEY( "Gunseat",     ship->gunseat,          fread_number( fp ) );
        break;

    case 'H':
	/*
	KEY( "HXSetup",	ship->hx,	fread_number( fp ));
	KEY( "HYSetup",	ship->hy,	fread_number( fp ));
	KEY( "HZSetup",	ship->hz,	fread_number( fp ));
	KEY( "Hyperdistance",	ship->hyperdistance,	fread_number( fp ));
	*/
        KEY( "Home" , ship->home, fread_string( fp ) );
        KEY( "Hyperspeed",   ship->hyperspeed,      fread_number( fp ) );
        KEY( "Hull",      ship->hull,        fread_number( fp ) );
        KEY( "Hangar",  ship->hangar,      fread_number( fp ) );
        break;

    case 'L':
        KEY( "Laserstr",   ship->lasers,   (sh_int)  ( fread_number( fp )/10 ) );
        KEY( "Lasers",   ship->lasers,      fread_number( fp ) );
        KEY( "Lastdoc",    ship->lastdoc,       fread_number( fp ) );
        KEY( "Lastroom",   ship->lastroom,        fread_number( fp ) );
	//KEY( "Location",   ship->location,	fread_number( fp ) );
        break;

    case 'M':
        KEY( "Manuever",   ship->manuever,      fread_number( fp ) );
        KEY( "Mass",   ship->mass,      fread_number( fp ) );
        KEY( "Maxmissiles",   ship->maxmissiles,      fread_number( fp ) );
        KEY( "Maxtorpedos",   ship->maxtorpedos,      fread_number( fp ) );
        KEY( "Maxrockets",   ship->maxrockets,      fread_number( fp ) );
        KEY( "Maxshield",      ship->maxshield,        fread_number( fp ) );
        KEY( "Missiles",   ship->missiles,      fread_number( fp ) );
        KEY( "Missiletype",   ship->missiletype,      fread_number( fp ) );
		KEY( "Maxammo",      ship->maxammo,        fread_number( fp ) );
        KEY( "Maxenergy",      ship->maxenergy,        fread_number( fp ) );
        KEY( "Missilestate",   ship->missilestate,        fread_number( fp ) );
        KEY( "Maxhull",      ship->maxhull,        fread_number( fp ) );
        KEY( "Maxchaff",       ship->maxchaff,      fread_number( fp ) );
        break;

	case 'N':
	    KEY( "Name",	ship->name,		fread_string( fp ) );
	    KEY( "Navseat",     ship->navseat,          fread_number( fp ) );
        break;

  	case 'O':
        KEY( "Owner",            ship->owner,            fread_string( fp ) );
        KEY( "Objectnum",        dummy_number,        fread_number( fp ) );
        break;

   	case 'P':
        KEY( "Pilot",            ship->pilot,            fread_string( fp ) );
        KEY( "Pilotseat",     ship->pilotseat,          fread_number( fp ) );
        break;

   	case 'R':
        KEY( "Realspeed",   ship->realspeed,       fread_number( fp ) );
        KEY( "Rockets",     ship->rockets,         fread_number( fp ) );
        break;

   	case 'S':
        KEY( "Shipyard",    ship->shipyard,      fread_number( fp ) );
        KEY( "Sensor",      ship->sensor,       fread_number( fp ) );
        KEY( "Shield",      ship->shield,        fread_number( fp ) );
        KEY( "Shipstate",   ship->shipstate,        fread_number( fp ) );
        KEY( "Statet0",   ship->statet0,        fread_number( fp ) );
        KEY( "Statet1",   ship->statet1,        fread_number( fp ) );
        KEY( "Statet2",   ship->statet2,        fread_number( fp ) );
	/*
            if ( !str_cmp( word, "System" ) )
            {
                ship->starsystem = starsystem_from_name ( fread_string(fp) );
                fMatch = TRUE;
            }

        break;
	*/

	case 'T':
	    KEY( "Type",	ship->type,	fread_number( fp ) );
	    KEY( "Tractorbeam", ship->tractorbeam,      fread_number( fp ) );
	    KEY( "Turret1",	ship->turret1,	fread_number( fp ) );
	    KEY( "Turret2",	ship->turret2,	fread_number( fp ) );
	    KEY( "Torpedos",	ship->torpedos,	fread_number( fp ) );
	    break;

	/*
	case 'W':
	    KEY( "WasFlying",	ship->wasflying,	fread_number( fp ) );
	    break;
	case 'V':
	    KEY( "VXSetting",	ship->vx,	fread_number( fp ));
	    KEY( "VYSetting",	ship->vy,	fread_number( fp ));
	    KEY( "VZSetting",	ship->vz,	fread_number( fp ));
	    break;
	*/
	}
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_ship: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}


void fread_ship_components( SHIP_DATA *ship, FILE *fp )
{
	char *word;
	char buf[MAX_STRING_LENGTH];
	int vnum;
	int i;
	bool fMatch;
	OBJ_DATA *component;

	ship->first_component = NULL;
	ship->last_component = NULL;

	for (;;)
    {
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		case '*':
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
	    	{
				return;
			}
		case 'V':
			if ( !str_cmp( word, "Vnum" ) )
			{
				vnum  = fread_number( fp );
				fMatch = TRUE;

				CREATE( component, OBJ_DATA, 1 );
				LINK( component, ship->first_component, ship->last_component, next_content, prev_content);

				component->next = NULL;
				component->prev = NULL;
				component->first_content = NULL;
				component->last_content = NULL;
				component->in_obj = NULL;
				component->carried_by = NULL;
				component->first_extradesc = NULL;
				component->last_extradesc = NULL;
				component->first_affect = NULL;
				component->last_affect = NULL;

				if ( (component->pIndexData = get_obj_index( vnum ) ) == NULL )
				{
					char buf[MAX_STRING_LENGTH];
					sprintf(buf, "fread_ship_components: No index for object %d", vnum);
					bug(buf);
					DISPOSE( component );
					continue;
				}

				component->in_room = NULL;;
				component->armed_by = STRALLOC( "" );
			    component->name = STRALLOC( component->pIndexData->name );
			    component->short_descr = STRALLOC( component->pIndexData->short_descr );
			    component->description = STRALLOC( component->pIndexData->description );
			    component->action_desc = STRALLOC( component->pIndexData->action_desc );
			    component->item_type = component->pIndexData->item_type;
			    component->mpscriptpos = 0;
			    component->extra_flags = component->pIndexData->extra_flags;
			    component->magic_flags = component->pIndexData->magic_flags;
			    component->wear_flags = component->pIndexData->wear_flags;
	    		component->mpact = NULL;
			    component->mpactnum = 0;
			    component->wear_loc = -1;
			    component->weight = component->pIndexData->weight;
			    component->cost = component->pIndexData->cost;
			    component->level = component->pIndexData->level;
			    component->timer = 0;
			    for(i = 0; i < 6 ; ++i )
			    	component->value[i] = component->pIndexData->value[i];
		    	component->count = 1;
			    component->serial = component->pIndexData->serial;
			    component->phonenumber = 0;
    			component->calling = NULL;
			}
			break;
		}

		if ( !fMatch )
		{
		    sprintf( buf, "Fread_ship_components: no match: %s", word );
		    bug( buf, 0 );
		}
	}

}



void fread_ship_tradegoods( SHIP_DATA *ship, FILE *fp )
{
	char *word;
	char buf[MAX_STRING_LENGTH];
	bool fMatch;
	TRADEGOOD_DATA *good;
	char *	origin;
	long long int value;
    sh_int type;

	ship->first_tradegood = NULL;
	ship->last_tradegood = NULL;

	for (;;)
    {
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		case '*':
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
	    	{
				fMatch = TRUE;
				if (!origin || !str_cmp(origin, "(null)") || !str_cmp(origin, ""))
				{
					//DISPOSE(origin); //Whoops, always creating and saving a null good on every ship... This will fix the problem. Funf 2010-10-12
					return;
				}
		  		if (value <= 0) value = 0;
		  		if (type <= 0) type = 1;
		  		CREATE( good, TRADEGOOD_DATA, 1 );
		  		good->in_ship = ship;
		  		good->origin = origin;
		  		good->value = value;
		  		good->type = type;
		  		LINK( good, ship->first_tradegood, ship->last_tradegood, next, prev);
				//return; Load more than 1, who'da thunk it? --Funf
			}
		case 'O':
			KEY( "Origin",      origin,            fread_string( fp ) );
			break;
		case 'T':
			KEY( "Type",    type,      fread_number( fp ) );
			break;
		case 'V':
			KEY( "Value",    value,      fread_long( fp ) );
			break;
		case '#':
			return; // Abandon this folly if I accidently hit #END.
		}

		if ( !fMatch )
		{
		    sprintf( buf, "Fread_ship_tradegoods: no match: %s", word );
		    bug( buf, 0 );
		}
	}

}


/*
 * Load a ship file
 */

bool load_ship_file( char *shipfile )
{
    char filename[256];
    SHIP_DATA *ship;
    FILE *fp;
    bool found;
    ROOM_INDEX_DATA *pRoomIndex;
    CLAN_DATA *clan;

    CREATE( ship, SHIP_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", SHIP_DIR, shipfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_ship_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "SHIP"	) )
	    {
	    	fread_ship( ship, fp );
	    	/*break;*/
	    }
	    else
	    if ( !str_cmp( word, "COMPONENTS" ) )
	    {
			fread_ship_components( ship, fp );
			/*break;*/
		}
		else
		if ( !str_cmp( word, "TRADEGOODS" ) )
		{
			fread_ship_tradegoods( ship, fp );
		}
		else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_ship_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }
    if ( !(found) )
      DISPOSE( ship );
    else
    {
       LINK( ship, first_ship, last_ship, next, prev );
       if ( !str_cmp("Public",ship->owner) || ship->type == MOB_SHIP )
       {

         if ( ship->class != SHIP_PLATFORM && ship->type != MOB_SHIP && ship->class != CAPITAL_SHIP )
         {
           extract_ship( ship );
           ship_to_room( ship , ship->shipyard );

           ship->location = ship->shipyard;
           ship->lastdoc = ship->shipyard;
           ship->shipstate = SHIP_DOCKED;
           }

     ship->currspeed=0;
     ship->energy=ship->maxenergy;
     ship->ammo=ship->maxammo;
     ship->chaff=ship->maxchaff;
     ship->hull=ship->maxhull;

     ship->statet1 = LASER_READY;
     ship->statet2 = LASER_READY;
     ship->statet0 = LASER_READY;
     ship->missilestate = LASER_READY;

     ship->currjump=NULL;
     ship->target0=NULL;
     ship->target1=NULL;
     ship->target2=NULL;

     ship->hatchopen = FALSE;
     ship->bayopen = FALSE;

     ship->missiles = ship->maxmissiles;
     ship->torpedos = ship->maxtorpedos;
     ship->rockets = ship->maxrockets;
     ship->ammo = ship->maxammo;
     ship->autorecharge = FALSE;
     ship->autotrack = FALSE;
     ship->autospeed = FALSE;

     ship->mono_disabled = FALSE;
	 ship->evading = 0;


       }
       else if ( ( pRoomIndex = get_room_index( ship->lastdoc ) ) != NULL
            && ship->class != CAPITAL_SHIP && ship->class != SHIP_PLATFORM )
       {
              LINK( ship, pRoomIndex->first_ship, pRoomIndex->last_ship, next_in_room, prev_in_room );
              ship->in_room = pRoomIndex;
              ship->location = ship->lastdoc;
       }


       if ( ship->class == SHIP_PLATFORM || ship->type == MOB_SHIP || ship->class == CAPITAL_SHIP )
       {
		  extract_ship( ship ); // Remove ship from any room it might have. --Funf 2011-02-07
          ship_to_starsystem(ship, starsystem_from_name(ship->home) );
          ship->vx = number_range( -2000 , 2000 );
          ship->vy = number_range( -2000 , 2000 );
          ship->vz = number_range( -2000 , 2000 );
          ship->hx = 1;
          ship->hy = 1;
          ship->hz = 1;
          ship->shipstate = SHIP_READY;
          ship->autopilot = TRUE;
          ship->autorecharge = TRUE;
       }

         if ( ship->type != MOB_SHIP && (clan = get_clan( ship->owner )) != NULL )
         {
          if ( ship->class <= SHIP_PLATFORM )
             clan->spacecraft++;
          else
             clan->vehicles++;
         }

         calc_ship_stats(ship); /* - Funf */

    }

    return found;
}


/*
 * Load in all the ship files.
 */
void load_ships( )
{
    FILE *fpList;
    char *filename;
    char shiplist[256];
    char buf[MAX_STRING_LENGTH];


    first_ship	= NULL;
    last_ship	= NULL;

    log_string( "Loading ships..." );

    sprintf( shiplist, "%s%s", SHIP_DIR, SHIP_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( shiplist, "r" ) ) == NULL )
    {
	perror( shiplist );
	exit( 1 );
    }

    for ( ; ; )
    {

	filename = feof( fpList ) ? "$" : fread_word( fpList );

	if ( filename[0] == '$' )
	  break;

	if ( !load_ship_file( filename ) )
	{
	  sprintf( buf, "Cannot load ship file: %s", filename );
	  bug( buf, 0 );
	}

    }
    fclose( fpList );
    log_string(" Done ships " );
    fpReserve = fopen( NULL_FILE, "r" );
    /* Failled attempt to fix copyover so it fixed ship's location
	fix_ship_space();
	*/
    return;
}

void resetship( SHIP_DATA *ship )
{
	TRADEGOOD_DATA *tg;
	TRADEGOOD_DATA *old_tg;

	ship->shipstate = SHIP_READY;

    if ( ship->class != SHIP_PLATFORM && ship->type != MOB_SHIP )
    {
    	extract_ship( ship );
        ship_to_room( ship , ship->shipyard );

        ship->location = ship->shipyard;
        ship->lastdoc = ship->shipyard;
        ship->shipstate = SHIP_DOCKED;
    }

    if (ship->starsystem)
    	ship_from_starsystem( ship, ship->starsystem );

    ship->currspeed=0;
    ship->energy=ship->maxenergy;
    ship->chaff=ship->maxchaff;
    ship->hull=ship->maxhull;

    ship->statet1 = LASER_READY;
    ship->statet2 = LASER_READY;
    ship->statet0 = LASER_READY;
    ship->missilestate = LASER_READY;

    ship->currjump=NULL;
    ship->target0=NULL;
    ship->target1=NULL;
    ship->target2=NULL;

    ship->hatchopen = FALSE;
    ship->bayopen = FALSE;

    ship->missiles = ship->maxmissiles;
    ship->torpedos = ship->maxtorpedos;
    ship->rockets = ship->maxrockets;
    ship->ammo = ship->maxammo;
    ship->autorecharge = FALSE;
    ship->autotrack = FALSE;
    ship->autospeed = FALSE;

    for(tg=ship->first_tradegood;tg;)
    {
   		UNLINK( tg, ship->first_tradegood, ship->last_tradegood, next, prev );
   		STRFREE(tg->origin);
   		old_tg = tg;
   		tg = tg->next;
	    DISPOSE(old_tg);
	}

/*		Owners never lose their ships -12/13/03 Kristen-
     if ( str_cmp("Public",ship->owner) && ship->type != MOB_SHIP )
     {
        CLAN_DATA *clan;

        if ( ship->type != MOB_SHIP && (clan = get_clan( ship->owner )) != NULL )
          if ( ship->class <= SHIP_PLATFORM )
             clan->spacecraft--;
          else
             clan->vehicles--;

        STRFREE( ship->owner );
        ship->owner = STRALLOC( "" );
        STRFREE( ship->pilot );
        ship->pilot = STRALLOC( "" );
        STRFREE( ship->copilot );
        ship->copilot = STRALLOC( "" );
     }
*/
	save_ship(ship);
}

void do_resetship( CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *ship;

    ship = get_ship( argument );
    if (ship == NULL)
    {
      	send_to_char("&RNo such ship!\n\r",ch);
        return;
    }

    resetship( ship );

    if ( ( ship->class == SHIP_PLATFORM || ship->type == MOB_SHIP || ship->class == CAPITAL_SHIP )
        && ship->home )
    {
    	ship_to_starsystem(ship, starsystem_from_name(ship->home) );
        ship->vx = number_range( -2000 , 2000 );
        ship->vy = number_range( -2000 , 2000 );
        ship->vz = number_range( -2000 , 2000 );
        ship->shipstate = SHIP_READY;
        ship->autopilot = TRUE;
        ship->autorecharge = TRUE;
    }

}


