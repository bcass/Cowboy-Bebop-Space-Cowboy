/***************************************************************************
*                    Cowboy Bebop: Space Cowboy 1.5                        *
*--------------------------------------------------------------------------*
* Bebop Code changes (c) 2003-2011 - Spike/Teiwaz/Kristen/Gatz/Funf        *
*--------------------------------------------------------------------------*
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997, 1998 by Sean Cooper                                  *
* -------------------------------------------------------------------------*
* Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
*--------------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
* ------------------------------------------------------------------------ *
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
* Chastain, Michael Quan, and Mitchell Tse.                                *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
* ------------------------------------------------------------------------ *
*                 Tradegood and trucking functionality                     *
****************************************************************************/


#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

//bool    autofly(SHIP_DATA *ship);
//SHIP_DATA * get_ship(char *);

//extern SHIP_DATA * first_ship;
//extern SHIP_DATA * last_ship;

//extern SPACE_DATA * first_starsystem;
//extern SPACE_DATA * last_starsystem;

//extern SHIP_WEAPON_DATA * first_weapon;
//extern SHIP_WEAPON_DATA * last_weapon;

extern PLANET_DATA * first_planet;
extern PLANET_DATA * last_planet;

long long int calculate_buygood (PLANET_DATA * selling_planet, sh_int type)
{

	if(!selling_planet)
	{
		bug( "calculate_buygood: NULL planet!", 0 );
		return 0;
	}

	switch (type)
	{
		case TRADEGOOD_STANDARD:
			if(selling_planet->supply <= 0)
				return 0;
			else
				return (long long int) 100000.0 / pow(selling_planet->supply, 1.2);
			break;
		case TRADEGOOD_CONTRABAND:
			if(selling_planet->contraband_supply <= 0)
				return 0;
			else
				return (long long int) 500000.0 / pow(selling_planet->contraband_supply, 1.3);
			break;
		default:
			return 0;
	}
}

long long int calculate_sellgood (PLANET_DATA * buying_planet, TRADEGOOD_DATA * good)
{
	PLANET_DATA *good_planet;
	int distance = 1;
	long long int value = 0;

	if(!buying_planet)
	{
		bug( "calculate_sellgood: NULL planet!", 0 );
		return 0;
	}

	if(!good)
	{
		bug( "calculate_sellgood: NULL tradegood!", 0 );
		return 0;
	}

	for(good_planet = first_planet;good_planet;good_planet = good_planet->next) //Search planets
	{
		if(!str_cmp(good->origin,good_planet->name)) //Found the planet the good was bought at
		{
			break;
		}
	}

	if(!good_planet)
	{
		bug( "calculate_sellgood: Tradegood has a NULL origin planet!", 0 );
		return 0;
	}

	distance = abs(good_planet->starsystem->xpos - buying_planet->starsystem->xpos)
			+ abs(good_planet->starsystem->ypos - buying_planet->starsystem->ypos) / 4;

	if(distance <= 0) distance = 1; // Selling within the same system is okay, in theory.

	if(buying_planet->demand <= 0)
		return 0;

	switch (good->type)
	{
		case TRADEGOOD_STANDARD:
			value = (long long int) (2.0 * pow(buying_planet->demand, 1.3) * (distance));
			return value <= 0 ? 0 : value;
			break;
		case TRADEGOOD_CONTRABAND:
			value = (long long int) (4.0 * pow(buying_planet->demand, 1.5) * (distance));
			return value <= 0 ? 0 : value;
			break;
		default:
			return 0;
	}

}


void do_buygood( CHAR_DATA *ch, char *argument )
{
    int chance;
    SHIP_DATA *ship;
	char arg1[MAX_INPUT_LENGTH];
	int count = 0;
	sh_int type;
	long long int price;
	TRADEGOOD_DATA *tg;
	PLANET_DATA *planet;

    argument = one_argument( argument, arg1 );

    if (nifty_is_name_prefix(arg1, "list")) // Price listing logic here
    {
		if ((ship = ship_from_cockpit(ch->in_room->vnum))  != NULL ) // In a ship
		{
			if (ship->shipstate != SHIP_DOCKED)
			{
				send_to_char("&RThe ship must be docked to do that!\n\r",ch);
				return;
			}
			ch_printf( ch, "&GCurrent prices: Standard %lld; Contraband %lld\n\r", calculate_buygood(ship->in_room->area->planet, TRADEGOOD_STANDARD), calculate_buygood(ship->in_room->area->planet, TRADEGOOD_CONTRABAND));
			return;
		}
		else // Anywhere on a planet
		{
			if(!ch->in_room || !ch->in_room->area || !ch->in_room->area->planet)
			{
				send_to_char("&RYou must be on a planet to do that!\n\r",ch);
				return;
			}
			ch_printf( ch, "&GCurrent prices: Standard %lld; Contraband %lld\n\r", calculate_buygood(ch->in_room->area->planet, TRADEGOOD_STANDARD), calculate_buygood(ch->in_room->area->planet, TRADEGOOD_CONTRABAND));
			return;
		}
	}

	if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
		return;
	}

	if (ship->shipstate != SHIP_DOCKED)
	{
		send_to_char("&RThe ship must be docked to do that!\n\r",ch);
		return;
	}

	if (nifty_is_name_prefix(arg1, "standard") || nifty_is_name_prefix(arg1, "contraband"))
	{
		//Check for failure
		if ( ship->class == FIGHTER_SHIP )
			 chance = IS_NPC(ch) ? ch->top_level
			 : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
		if ( ship->class == MIDSIZE_SHIP )
			 chance = IS_NPC(ch) ? ch->top_level
				 : (int)  (ch->pcdata->learned[gsn_midships]) ;
		if ( ship->class == CAPITAL_SHIP )
			  chance = IS_NPC(ch) ? ch->top_level
				 : (int) (ch->pcdata->learned[gsn_capitalships]);
		if ( number_percent( ) > chance )
		{
			send_to_char("&RYou fail to work the controls properly.\n\r",ch);
			if ( ship->class == FIGHTER_SHIP )
			   learn_from_failure( ch, gsn_starfighters );
			if ( ship->class == MIDSIZE_SHIP )
			   learn_from_failure( ch, gsn_midships );
			if ( ship->class == CAPITAL_SHIP )
				learn_from_failure( ch, gsn_capitalships );
		   return;
		}

		if(!ship->in_room || !ship->in_room->area || !ship->in_room->area->planet)
		{
			bug( "do_buygood: NULL planet!", 0 );
			return; //BUG!!
		}
		else planet = ship->in_room->area->planet;//Set planet we're on

		//Determine which type we're buying
		if (nifty_is_name_prefix(arg1, "standard") )
		{
			type = TRADEGOOD_STANDARD;
			if(planet->supply <= 0)
			{
				send_to_char("&RThere are no goods for sale today!\n\r",ch);
				return;
			}
		}
		else if (nifty_is_name_prefix(arg1, "contraband"))
		{
			type = TRADEGOOD_CONTRABAND;
			if(planet->contraband_supply <= 0)
			{
				send_to_char("&RThere's none of that to buy!\n\r",ch);
				return;
			}
		}

		//Check that the ship has the cargo capacity

		if(ship->cargo <= 0)
		{
			send_to_char("&RYou need to install a cargo hold first!\n\r",ch);
			return;
		}

		for(tg = ship->first_tradegood, count = 0;tg;tg = tg->next, ++count); // Get a count of tradegoods on a ship

		if(count >= ship->cargo)
		{
			send_to_char("&RYou don't have enough room to take on anymore cargo!\n\r",ch);
			return;
		}

		//Check that the player has the funds

		price = calculate_buygood(planet, type);

		if ( ch->gold < price )
		{
			ch_printf(ch, "&RBuying a tradegood here costs %lld. You don't have enough wulongs!\n\r" , price );
			return;
   		}

		if (!IS_NPC(ch) && ch->pcdata->learned[gsn_discount] > number_percent() )
		{
			ch_printf(ch, "&CYou manage to haggle the broker for a %.0f percent discount!\r\n", get_curr_cha(ch)/5.0);
			learn_from_success(ch, gsn_discount);
			price -= (long long int) price * (get_curr_cha(ch) / 500.0);
		}

		//Contraband? Check if caught...

		ch->gold -= price; // Make sure the price is paid even if they get caught! --Funf

		if(type == TRADEGOOD_CONTRABAND && planet->security)
		{
			chance = planet->security; // Planet's security
			chance -= get_curr_lck(ch) + 10; // Bonus for high luck
			if(planet->governed_by && ch->pcdata && ch->pcdata->clan && ch->pcdata->clan == planet->governed_by)
				chance -= (int) (planet->pop_support + 100) / 8; // Bonus for owning the planet
			if(planet->frustrated_by && ch->pcdata && ch->pcdata->clan && ch->pcdata->clan == planet->frustrated_by)
				chance += (int) planet->frust_level / 4; // Penalty for frustrating the planet
			if(number_percent() < chance)
			{
				send_to_char( "&RYour attempt to smuggle illegal goods is discovered!\n\r", ch );
				place_bounty( ch, ch, 20000, 0);
				return;
			}
			else
			{
				gain_exp(ch, (int)price/100, SMUGGLING_ABILITY);
				ch_printf( ch , "&YYou gain %d smuggling experience.\n\r", (int)price/100 );
			}
		}

   		//Create the tradegood and execute the sale

	    CREATE( tg, TRADEGOOD_DATA, 1 );
	    tg->in_ship = ship;
	    tg->origin = STRALLOC(planet->name);
	    tg->value = price;
	    tg->type = type;

		if(type == TRADEGOOD_STANDARD) planet->supply--;
		else planet->contraband_supply--;

	    ch_printf(ch, "&YYou bought a %s trade good for %lld wulongs!\n\r",
	    	 tg->type == TRADEGOOD_STANDARD ? "standard" :
	    	 tg->type == TRADEGOOD_CONTRABAND ? "contraband" : "unknown",
	    	 tg->value
	    	 );
	    LINK( tg, ship->first_tradegood, ship->last_tradegood, next, prev );
	    save_ship( ship );
	    save_planet(planet);
	}
	else
	{
		send_to_char( "&GOptions: list, standard, contraband\n\r", ch);
		return;
	}


    act( AT_PLAIN, "$n fiddles with one of the consoles.", ch,
         NULL, argument , TO_ROOM );
    echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");


    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}

void do_sellgood( CHAR_DATA *ch, char *argument )
{
    int chance;
    SHIP_DATA *ship;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int count = 0;
	sh_int type;
	long long int price;
	TRADEGOOD_DATA *tg;
	bool found = FALSE;
	PLANET_DATA *planet;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
		return;
	}

	if (ship->shipstate != SHIP_DOCKED)
	{
		send_to_char("&RThe ship must be docked to do that!\n\r",ch);
		return;
	}

    if (nifty_is_name_prefix(arg1, "list")) // Price listing logic here
    {
		for(tg = ship->first_tradegood; tg; tg = tg->next)
		{
			++count;
			price = calculate_sellgood(ship->in_room->area->planet, tg);
			ch_printf( ch, "&Y%d. &gOrigin&b: &C%s  &gType&b: &C%s  &gPaid&b: &C%lld  &gSell&b: %s%lld\n\r",
				count,
				tg->origin,
				tg->type == TRADEGOOD_STANDARD ? "Standard" :
				tg->type == TRADEGOOD_CONTRABAND ? "Contraband" : "Unknown",
				tg->value,
				price > tg->value ? "&G" : "&R", // Color changes depending on profit --Funf
				price
				);
			/*ch_printf( ch, "TEST ZOMG: %s %s %d %d\r\n", ship->in_room->area->planet->name
			,ship->in_room->area->planet->starsystem->name
			,ship->in_room->area->planet->starsystem->xpos
			,ship->in_room->area->planet->starsystem->ypos);*/
		}

		if(count == 0)
		{
			ch_printf( ch, "&RYour ship isn't carrying any cargo right now!\r\n");
		}

		return;

	}

	if (nifty_is_name_prefix(arg1, "sell") || (arg1[0] <= '9' && arg1[0] >= '0'))
	{
		if(nifty_is_name_prefix(arg1, "sell")) arg1[0] = arg2[0];

		//Check for failure
		if ( ship->class == FIGHTER_SHIP )
			 chance = IS_NPC(ch) ? ch->top_level
			 : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
		if ( ship->class == MIDSIZE_SHIP )
			 chance = IS_NPC(ch) ? ch->top_level
				 : (int)  (ch->pcdata->learned[gsn_midships]) ;
		if ( ship->class == CAPITAL_SHIP )
			  chance = IS_NPC(ch) ? ch->top_level
				 : (int) (ch->pcdata->learned[gsn_capitalships]);
		if ( number_percent( ) > chance )
		{
			send_to_char("&RYou fail to work the controls properly.\n\r",ch);
			if ( ship->class == FIGHTER_SHIP )
			   learn_from_failure( ch, gsn_starfighters );
			if ( ship->class == MIDSIZE_SHIP )
			   learn_from_failure( ch, gsn_midships );
			if ( ship->class == CAPITAL_SHIP )
				learn_from_failure( ch, gsn_capitalships );
		   return;
		}

		//Find the specified good

		if(arg1[0] > '9' || arg1[0] <= '0')
		{
			send_to_char("&RPlease specify which good to sell by it's cargo listing number!\n\r",ch);
			return;
		}

		for(tg = ship->first_tradegood; tg; tg = tg->next)
		{
			++count;
			if(arg1[0] == count + (int)'0' )
			{
				found = TRUE;
				break;
			}
		}

		if(!found)
		{
			send_to_char("&RSorry, no goods could be located by that number!\n\r",ch);
			return;
		}

		if(!ship->in_room || !ship->in_room->area || !ship->in_room->area->planet)
		{
			bug( "do_sellgood: NULL planet!", 0 );
			return; //BUG!!
		}
		else planet = ship->in_room->area->planet;//Set planet we're on

		//Find the price...
		price = calculate_sellgood(planet, tg);

		//Contraband? Check if caught...

		if(type == TRADEGOOD_CONTRABAND && planet->security)
		{
			chance = planet->security; // Planet's security
			chance -= get_curr_lck(ch) + 10; // Bonus for high luck
			if(planet->governed_by && ch->pcdata && ch->pcdata->clan && ch->pcdata->clan == planet->governed_by)
				chance -= (int) (planet->pop_support + 100) / 8; // Bonus for owning the planet
			if(planet->frustrated_by && ch->pcdata && ch->pcdata->clan && ch->pcdata->clan == planet->frustrated_by)
				chance += (int) planet->frust_level / 4; // Penalty for frustrating the planet
			if(number_percent() < chance)
			{
				send_to_char( "&RYour attempt to smuggle illegal goods is discovered!\n\r", ch );
				place_bounty( ch, ch, 20000, 0);
				UNLINK( tg, ship->first_tradegood, ship->last_tradegood, next, prev ); // Destroy contraband!
				STRFREE(tg->origin);
				DISPOSE(tg);
				save_ship(ship);
				return;
			}
			else
			{
				gain_exp(ch, (int)price/100, SMUGGLING_ABILITY);
				ch_printf( ch , "&YYou gain %d smuggling experience.\n\r", (int)price/100 );
			}
		}


		//Sell it
		ch_printf( ch, "&YSold for %s%lld&Y wulongs!\n\r",
			price > tg->value ? "&G" : "&R", // Color changes depending on profit --Funf
			price);
   		ch->gold += price;
   		planet->demand--;
   		UNLINK( tg, ship->first_tradegood, ship->last_tradegood, next, prev );
   		STRFREE(tg->origin);
	    DISPOSE(tg);
	    save_ship(ship);
	    save_planet(planet);

	}
	else
	{
		send_to_char( "&GSyntax: sellgood (list/#)\n\r", ch);
		return;
	}


    act( AT_PLAIN, "$n fiddles with one of the consoles.", ch,
         NULL, argument , TO_ROOM );
    echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");


    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}


void do_resettrade( CHAR_DATA *ch, char *argument )
{
	// Loop through each planet and reset all the tradegood levels
	PLANET_DATA *planet;

	for ( planet = first_planet; planet; planet = planet->next )
	{
		planet->supply = 0;
		planet->contraband_supply = 0;
		planet->demand = 0;
		save_planet(planet);
	}
}


void do_updatetrade( CHAR_DATA *ch, char *argument )
{
	update_planettrade();
	send_to_char( "Okay.\n\r", ch);
	return;
}

