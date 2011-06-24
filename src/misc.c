/***************************************************************************
*                           STAR WARS REALITY 1.0                          *
*--------------------------------------------------------------------------*
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997 by Sean Cooper                                        *
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
*	    Misc module for general commands: not skills or spells	   *
****************************************************************************
* Note: Most of the stuff in here would go in act_obj.c, but act_obj was   *
* getting big.								   *
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include "mud.h"

SHIP_DATA * first_ship;
SHIP_DATA * last_ship;


extern int	top_exit;

void dual_pistol_ammo(CHAR_DATA *ch);

void do_buyhome( CHAR_DATA *ch, char *argument )
{
     ROOM_INDEX_DATA *room;
     AREA_DATA *pArea;

     if ( !ch->in_room )
         return;

     if ( IS_NPC(ch) || !ch->pcdata )
         return;

     if ( ch->plr_home != NULL )
     {
         send_to_char( "&RYou already have a home!\n\r&w", ch);
         return;
     }
     if( get_age(ch) < 50)
     {
	 send_to_char( "&RHomes are reserved for players with more than 50 hours.\r\n", ch);
	 return;
     }
     room = ch->in_room;

     for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
     {
         if ( room->area == pArea )
         {
             send_to_char( "&RThis area isn't installed yet!\n\r&w", ch);
             return;
         }
     }

     if ( !IS_SET( room->room_flags , ROOM_EMPTY_HOME ) )
     {
         send_to_char( "&RThis room isn't for sale!\n\r&w", ch);
         return;
     }

     if ( ch->gold < 500000 )
     {
         send_to_char( "&RThis room costs 500000 wulongs. You don't have enough!\n\r&w", ch);
         return;
     }

     if ( argument[0] == '\0' )
     {
	   send_to_char( "Set the room name.  A very brief, single line room description.\n\r", ch );
	   send_to_char( "Usage: Buyhome <Room Name>\n\r", ch );
	   return;
     }

     STRFREE( room->name );
     smash_tilde( argument );
     room->name = STRALLOC( argument );

     ch->gold -= 500000;

     REMOVE_BIT( room->room_flags , ROOM_EMPTY_HOME );
     SET_BIT( room->room_flags , ROOM_PLR_HOME );

     fold_area( room->area, room->area->filename, FALSE );

     ch->plr_home = room;
     do_save( ch , "" );

}

void do_sellhome( CHAR_DATA *ch, char *argument )
{
     ROOM_INDEX_DATA *room;
     char buf[256];

     if ( !ch->in_room )
         return;

     if ( IS_NPC(ch) || !ch->pcdata )
         return;

     if ( ch->plr_home == NULL )
     {
         send_to_char( "&RYou do not have a home to sell!\n\r&w", ch);
         return;
     }

	 if ( ch->plr_home )
    {
		 ch->gold += 375000;
      room = ch->plr_home;
      sprintf( buf, "%s%c/%s.home", PLAYER_DIR, tolower(ch->name[0]),
                                 capitalize( ch->name ) );
      remove(buf);
      STRFREE( room->name );
      room->name = STRALLOC( "An Empty Apartment" );

      REMOVE_BIT( room->room_flags , ROOM_PLR_HOME );
      SET_BIT( room->room_flags , ROOM_EMPTY_HOME );

      fold_area( room->area, room->area->filename, FALSE );
	  ch->plr_home = NULL;
    }

	 do_save( ch , "" );

}

void do_clone( CHAR_DATA *ch, char *argument )
{
     long long int credits, bank;
     long long int played;
     char clanname[MAX_STRING_LENGTH];
     char bestowments[MAX_STRING_LENGTH];
     int flags;
     ROOM_INDEX_DATA *home;

     if ( IS_NPC(ch) )
     {
       ch_printf( ch, "Yeah right!\n\r" );
       return;
     }

     if ( ch->in_room->vnum != 10001 )
     {
       ch_printf( ch, "You can do that here!\n\r" );
       return;
     }

     if ( ch->gold < ch->top_level*200 )
     {
       ch_printf( ch, "You don't have enough wulongs... You need %d.\n\r" , ch->top_level*200 );
       return;
     }
     else
     {
       ch->gold -= ch->top_level*200;

       ch_printf( ch, "You pay %d wulongs for cloning.\n\r" , ch->top_level*200 );
       ch_printf( ch, "You are escorted into a small room.\n\r\n\r" );
     }

     char_from_room( ch );
     char_to_room( ch, get_room_index( 10000 ) );

     flags   = ch->act;
     REMOVE_BIT( ch->act, PLR_KILLER );
     credits = ch->gold;
     ch->gold = 0;
     played = ch->played;
     ch->played = ch->played/2;
     bank = ch->pcdata->bank;
     home = ch->plr_home;
     ch->plr_home = NULL;
     if ( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
     {
         strcpy( clanname, ch->pcdata->clan_name);
         STRFREE( ch->pcdata->clan_name );
         ch->pcdata->clan_name = STRALLOC( "" );
         strcpy( bestowments, ch->pcdata->bestowments);
         DISPOSE( ch->pcdata->bestowments );
         ch->pcdata->bestowments = str_dup( "" );
         save_clone(ch);
         STRFREE( ch->pcdata->clan_name );
         ch->pcdata->clan_name = STRALLOC( clanname );
         DISPOSE( ch->pcdata->bestowments );
         ch->pcdata->bestowments = str_dup( clanname );
     }
     else
       save_clone( ch );
     ch->plr_home = home;
     ch->played = played;
     ch->gold = credits;
     ch->pcdata->bank = bank;
     ch->act = flags;
     char_from_room( ch );
     char_to_room( ch, get_room_index( 10002 ) );
     do_look( ch , "" );

     ch_printf( ch, "\n\r&WA small tissue sample is taken from your arm.\n\r" );
     ch_printf( ch, "&ROuch!\n\r\n\r" );
     ch_printf( ch, "&WYou have been succesfully cloned.\n\r" );

     ch->hit--;
}

void do_arm( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if ( IS_NPC(ch) || !ch->pcdata )
    {
      ch_printf( ch, "You have no idea how to do that.\n\r" );
      return;
    }

    if ( ch->pcdata->learned[gsn_grenades] <= 0 )
    {
      ch_printf( ch, "You have no idea how to do that.\n\r" );
      return;
    }

    if( IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
	IS_SET(ch->in_room->room_flags, ROOM_PLR_HOME))
    {
	ch_printf( ch, "You can't do that here.\n\r");
	return;
    }

    obj = get_eq_char( ch, WEAR_HOLD );

    if ( !obj || obj->item_type != ITEM_GRENADE )
    {
       ch_printf( ch, "You don't seem to be holding a grenade!\n\r" );
       return;
    }

    obj->timer = 1;
//  STRFREE ( obj->armed_by );
    obj->armed_by = STRALLOC ( ch->name );

    ch_printf( ch, "You arm %s.\n\r", obj->short_descr );
    act( AT_PLAIN, "$n arms $p.", ch, obj, NULL, TO_ROOM );

    learn_from_success( ch , gsn_grenades );
}

void do_ammo( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wield;
    OBJ_DATA *obj;
    bool checkammo = FALSE;
    int charge =0;

    obj = NULL;
    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield)
    {
       obj = get_eq_char( ch, WEAR_DUAL_WIELD );
       if (!obj)
          obj = get_eq_char( ch, WEAR_HOLD );
    }
    else
    {
      wield = get_eq_char( ch, WEAR_HOLD );
      obj = NULL;
    }

    if (!wield || wield->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RYou don't seem to be holding a weapon.\n\r&w", ch);
        return;
    }

    if ( wield->value[3] == WEAPON_RIFLE )
    {

      if ( obj && obj->item_type != ITEM_AMMO )
      {
        send_to_char( "&RYour hands are too full to reload your rifle.\n\r&w", ch);
        return;
      }

      if (obj)
      {
        if (( obj->value[0] != wield->value[5]) )
        {
            send_to_char( "That cartridge can't fit in the rifle.", ch);
            return;
        }
        unequip_char( ch, obj );
        checkammo = TRUE;
        charge = obj->value[0];
        separate_obj( obj );
        extract_obj( obj );
      }
      else
      {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if ( obj->item_type == ITEM_AMMO)
           {
                 if ( obj->value[0] != wield->value[5] )
                 {
                    send_to_char( "That cartridge wont fit in your rifle.", ch);
                    continue;
                 }
                 checkammo = TRUE;
                 charge = obj->value[0];
                 separate_obj( obj );
                 extract_obj( obj );
                 break;
           }
        }
      }

      if (!checkammo)
      {
        send_to_char( "&RYou don't seem to have any ammo to reload your rifle with.\n\r&w", ch);
        return;
      }

      // Create an empty clip on the ground - Gatz
      make_clip( ch );
      ch_printf( ch, "You replace your ammunition.\n\rYour rifle is loaded with %d shots.\n\r", charge );
      act( AT_PLAIN, "$n replaces the ammunition in $p.", ch, wield, NULL, TO_ROOM );

    }
    else if ( wield->value[3] == WEAPON_PISTOL )
    {

      if ( obj && obj->item_type != ITEM_BOLT && obj->value[3] != WEAPON_PISTOL )
      {
        send_to_char( "&RYour hands are too full to reload your pistol.\n\r&w", ch);
        return;
      }

      if (obj && obj->item_type == ITEM_BOLT)
      {
        if (( obj->value[0] != wield->value[5]) )
        {
            send_to_char( "That clip wont fit in your pistol.", ch);
            return;
        }
        unequip_char( ch, obj );
        checkammo = TRUE;
        charge = obj->value[0];
        separate_obj( obj );
        extract_obj( obj );
      }
      else if(wield && wield->value[3] == WEAPON_PISTOL && obj && obj->value[3] == WEAPON_PISTOL)
      { /* This whole code block added by Funf */

		dual_pistol_ammo(ch);
		return;
      }
      else
	  {
	      for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	      {
	         if ( obj->item_type == ITEM_BOLT)
	         {
	               if ( obj->value[0] !=  wield->value[5] )
	               {
	                  send_to_char( "That clip wont fit in your pistol.", ch);
	                  continue;
	               }
	               checkammo = TRUE;
	               charge = obj->value[0];
	               separate_obj( obj );
	               extract_obj( obj );
	               break;
	         }
	      }
	      if(!checkammo) /* Added by Funf */
	      {
			  OBJ_DATA * container;
			  for ( container = ch->last_carrying; container; container = container->prev_content )
			  {
			      if ( container->item_type == ITEM_CONTAINER)
			      {
					  for ( obj = container->first_content; obj; obj = obj->next_content )
					  {
						  if ( obj->item_type == ITEM_BOLT)
						  {
			          		  if ( obj->value[0] ==  wield->value[5] )
			  	      		  {
			  	    			  checkammo = TRUE;
			  	    			  charge = obj->value[0];
			  	    			  separate_obj( obj );
			  	    			  extract_obj( obj );
			  	    			  break;
						  	  }
						  }
				  	  }
			  	  }
		  	  }
		  }
      }

      if (!checkammo)
      {
        send_to_char( "&RYou don't seem to have any clips to reload your pistol with.\n\r&w", ch);
        return;
      }
      // Create an empty clip on the ground - Gatz
      make_clip( ch );
      ch_printf( ch, "You replace your clip.\n\rYour pistol is loaded with %d rounds.\n\r", charge );
      act( AT_PLAIN, "$n replaces the clip in $p.", ch, wield, NULL, TO_ROOM );

    }
    else
    {

      if ( obj && obj->item_type != ITEM_BATTERY )
      {
        send_to_char( "&RYour hands are too full to replace the ammunition.\n\r&w", ch);
        return;
      }

      if (obj)
      {
        unequip_char( ch, obj );
        checkammo = TRUE;
        charge = obj->value[0];
        separate_obj( obj );
        extract_obj( obj );
      }
      else
      {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if ( obj->item_type == ITEM_BATTERY)
           {
                 checkammo = TRUE;
                 charge = obj->value[0];
                 separate_obj( obj );
                 extract_obj( obj );
                 break;
           }
        }
      }

      if (!checkammo)
      {
        send_to_char( "&RYou don't seem to have any ammunition .\n\r&w", ch);
        return;
      }
      else if (wield->value[3] == WEAPON_HEAVY_WEAPON )
      {
         ch_printf( ch, "You replace your ammunition.\n\rYour heavy weapon is loaded with %d units.\n\r", charge );
         act( AT_PLAIN, "$n replaces the ammunition in $p.", ch, wield,NULL,  TO_ROOM );
      }
      else
      {
         ch_printf( ch, "You feel very foolish.\n\r" );
         act( AT_PLAIN, "$n tries to load ammo into $p.", ch, wield, NULL, TO_ROOM );
      }
    }

    wield->value[4] = charge;

    if (wield->value[4] > wield->value[5])
      wield->value[4] = wield->value[5]; /* Sanity check ammo values. 2009-01-22 Funf */

}

void dual_pistol_ammo(CHAR_DATA *ch)
{ /* Added in seperate function cuz I'm lazy - Funf */
	OBJ_DATA *wield, *dual, *obj, *obj2;
	int charge1 = 0, charge2 = 0;

	wield = get_eq_char( ch, WEAR_WIELD );
	dual = get_eq_char( ch, WEAR_DUAL_WIELD );

	if( !wield || !dual )
		return;
	if(wield->value[3] != WEAPON_PISTOL || dual->value[3] != WEAPON_PISTOL )
		return;

	for(obj = ch->last_carrying; obj; obj = obj->prev_content) {
		if(obj->item_type == ITEM_BOLT) {
			if(obj && !charge1 && !wield->value[4] && obj->value[0] == wield->value[5]) {
				charge1 = obj->value[0];
				separate_obj( obj );
				extract_obj( obj );
				continue;
			}
			if(obj && !charge2 && !dual->value[4] && obj->value[0] == dual->value[5]) {
				charge2 = obj->value[0];
				separate_obj( obj );
				extract_obj( obj );
			}
		}
	}

	for(obj = ch->last_carrying; obj; obj = obj->prev_content) {
		if(obj && obj->item_type == ITEM_CONTAINER) {
			for(obj2 = obj->first_content;obj2;obj2 = obj2->next_content) {
				if(obj2 && obj2->item_type == ITEM_BOLT) {
					if(!charge1 && !wield->value[4] && obj2->value[0] == wield->value[5]) {
						charge1 = obj2->value[0];
						separate_obj( obj2 );
	                 	extract_obj( obj2 );
	                 	continue;
					}
					if(!charge2 && !dual->value[4] && obj2->value[0] == dual->value[5]) {
						charge2 = obj2->value[0];
						separate_obj( obj2 );
						extract_obj( obj2 );
					}
				}
			}
		}
	}

	if(charge1) {
		make_clip( ch );
		ch_printf( ch, "You replace your clip.\n\rYour pistol is loaded with %d rounds.\n\r", charge1 );
	    act( AT_PLAIN, "$n replaces the clip in $p.", ch, wield, NULL, TO_ROOM );
		wield->value[4] = charge1;
    }
    if(charge2) {
		make_clip( ch );
		ch_printf( ch, "You replace your clip.\n\rYour pistol is loaded with %d rounds.\n\r", charge2 );
    	act( AT_PLAIN, "$n replaces the clip in $p.", ch, wield, NULL, TO_ROOM );
		dual->value[4] = charge2;
	}
	return;
}

//Stat Train Stuff -Gatz
void do_lift( CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   int chance, luck, exercise, rep;
   int round, tired;
   int fail, succeed;

   argument = one_argument( argument, arg );

   exercise = 0;
   if ( arg[0] == '\0' )
	rep = 1;
   else
	rep = atoi( arg );

/*   if(IS_SET( ch->in_room->room_flags2, ROOM_GYM ) )
   {
	exercise += 2;
   } */

   if( rep <= 0)
   {
	send_to_char("&RYou can't do negitive reps!\n\r", ch);
	return;
   }
   if( ch->position != POS_STANDING )
   {
         send_to_char( "You can't do that in your current position\n\r", ch );
         return;
   }

   tired = ch->move;
   tired -= 30*rep;
   fail = 0;

   if(ch->move < 100 || tired < 100)
   {
	send_to_char("You can't do that!\n\r", ch);
	return;
   }
   if(ch->hit < 100)
   {
	send_to_char("You are far too hurt to do this!\n\r", ch);
	return;
   }


	WAIT_STATE(ch, 2);
//	act( AT_RED, "$n tries to lift some free weights.", ch, NULL, NULL, TO_ROOM);
	act( AT_RED, "You try to lift some free weights.", ch, NULL, NULL, TO_CHAR);

   luck = ch->perm_lck/2;

   WAIT_STATE( ch, (rep*3));

   for(round = 0; round < rep; round++)
   {
	  chance = number_range(0, luck);
  	  if( chance != 0)
	  {
		if(IS_SET( ch->in_room->room_flags2, ROOM_GYM ) )
			exercise += 3;
		else
			exercise++;
	  }
	  else
	  {
		fail++;
		ch->hit -= 20;
	  }
	ch->move -= 30;
   }

   if( exercise > 0 )
	ch->strtrain += exercise;

   succeed = rep - fail;

  ch_printf( ch, "You succeed %d out of %d tries.\n\r", succeed, round);
   if(ch->hit < 0)
	ch->position = POS_STUNNED;
   return;
}

void do_study( CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   int chance, luck, exercise, rep;
   int round, tired;

   argument = one_argument( argument, arg );

   exercise = 0;
   if ( arg[0] == '\0' )
        rep = 1;
   else
        rep = atoi( arg );

   if( rep <= 0)
   {
	send_to_char("&RYou can't read negative pages!\n\r", ch);
	return;
   }

   if( ch->position != POS_STANDING )
   {
         send_to_char( "You can't do that in your current position\n\r", ch );
         return;
   }

   if(!IS_SET( ch->in_room->room_flags2, ROOM_LIBRARY ) )
   {
	send_to_char("You need to be in a library to study!\n\r", ch);
	return;
   }
   tired = ch->move;
   tired -= 30*rep;

   if(ch->move < 100 || tired < 100)
   {
        send_to_char("You can't do that!\n\r", ch);
        return;
   }
   if(ch->hit < 100)
   {
        send_to_char("You are too hurt to focus in!\n\r", ch);
        return;
   }




   act( AT_RED, "You try to study some books.", ch, NULL, NULL, TO_CHAR);

   luck = ch->perm_lck*2;

   WAIT_STATE( ch, rep);

   for(round = 0; round < rep; round++)
   {
          chance = number_range(0, luck);
          if( chance != 0)
          {
                        exercise++;
          }
          else
                ch->move -= 20;
        ch->move -= 10;
   }

   if( exercise > 0 )
   {
        ch->wistrain += exercise;
	ch->inttrain += exercise*2;
   }

   return;
}

//increased gain amount - Locke
void do_research( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int chance, luck;
	int time, message;


	if(IS_SET(ch->in_room->room_flags, ROOM_INDOORS) )
	{
		send_to_char("&RYou need to be outside for this!\n\r", ch);
		return;
	}

	if(ch->position != POS_STANDING)
	{
		send_to_char("&RYou can't do this in your current position!\n\r", ch);
		return;
	}

	strcpy( arg, argument );

	switch( ch->substate )
	{
		default:
			if ( arg[0] == '\0')
			{
			   send_to_char("&RUsage: research (time in seconds)\n\r" , ch);
			   return;
			}

			time = atoi( arg);

			if( time < 0)
			{
			   send_to_char("&RHow can you do something in negitive seconds?\n\r", ch);
			   return;
			}
			send_to_char("&RYou begin to research your surroundings.\n\r", ch);
			add_timer ( ch, TIMER_DO_FUN, time, do_research, 1);
			ch->dest_buf = str_dup(arg);
			act( AT_PLAIN, "$n begins to do some field research.\n\r", ch, NULL, NULL, TO_ROOM);
			return;

		case 1:
			if (!ch->dest_buf )
			   return;
			strcpy( arg, ch->dest_buf);
			DISPOSE( ch->dest_buf );
			break;

		case SUB_TIMER_DO_ABORT:
			DISPOSE( ch->dest_buf );
			ch->substate = SUB_NONE;
			send_to_char("&RYou are distracted and lose focus!\n\r", ch);
			return;
	}

	ch->substate = SUB_NONE;


	luck = ch->perm_lck;
	chance = number_range( 0, luck);

	if(chance == 0)
	{
		send_to_char("&RYou tried so hard...but couldn't notice anything!\n\r", ch);
		return;
	}

	message = number_range(0, 6);

	switch(message)
	{
		case 1:
			send_to_char("&RYou notice the way an insect's body is structured!\n\r" ,ch);
			break;
		case 2:
			send_to_char("&RYou notice a familiar patern for the weather in this area!\n\r", ch);
			break;
		case 3:
			send_to_char("&RYou notice the structure of a nearby rock, and note it's shape and size.\n\r", ch);
			break;
		case 4:
			send_to_char("&RYou discover a plant you have never seen before!\n\r", ch);
			break;
		case 5:
			send_to_char("&RYou get a rush of enlightment as you discover a deep spiritual connection!\n\r", ch);
			break;
		case 6:
			send_to_char("&RYou notice how the light refracts on the objects and the sun beams dances across surfaces!\n\r", ch);
			break;
		default:
			send_to_char("&RYou notice the breeze is coming from a certain direction.\n\r", ch);
	}
	time = atoi(arg);

	act( AT_PLAIN, "$n finishes their research!\n\r", ch, NULL, NULL, TO_ROOM);
	send_to_char("Your research proves fruitful!\n\r" ,ch);
	ch->inttrain += time*4;

	return;
}


//increased gain amount - Locke
void do_endurance( CHAR_DATA *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
        int chance, luck;
        int time, message;


        if(!IS_SET(ch->in_room->room_flags2, ROOM_GYM) )
        {
                send_to_char("&RYou need to be in a gym for this!\n\r", ch);
                return;
        }

        if(ch->position != POS_STANDING)
        {
                send_to_char("&RYou can't do this in your current position!\n\r", ch);
                return;
        }

        strcpy( arg, argument );

        switch( ch->substate )
        {
               default:
                        if ( arg[0] == '\0')
                        {
                           send_to_char("&RUsage: endurance (time in seconds)\n\r" , ch);
                           return;
                        }

                        time = atoi( arg);

                        if( time < 0)
                        {
                           send_to_char("&RHow can you do something in negitive seconds?\n\r", ch);
                           return;
                        }
			if(time < 10)
			{
			   send_to_char("&RWith so little time, might aswell not do it!\n\r", ch);
			   return;
			}
			if( (ch->hit - time) < 100)
			{
			   send_to_char("&RYour body couldn't handle that!\n\r", ch);
			   return;
			}
			message = number_range(0,4);
			switch(message)
			{
				case 1:
					send_to_char("&RYou lift up a heavy weight and try to hold it.\n\r", ch);
					break;
				case 2:
					send_to_char("&RYou begin to do jumping jacks.\n\r" ,ch);
					break;
				case 3:
					send_to_char("&RYou begin to shadow box at your fastest speed!\n\r", ch);
					break;
				case 4:
					send_to_char("&RYou begin to run on a treadmill as fast as you can!\n\r", ch);
					break;
				default:
					send_to_char("&RYou begin to punch and kick a punching bag!\n\r", ch);
			}
                        send_to_char("&RYou begin to do endurance training.\n\r", ch);
                        add_timer ( ch, TIMER_DO_FUN, time, do_endurance, 1);
                        ch->dest_buf = str_dup(arg);
                        act( AT_PLAIN, "$n begins to do endurance training\n\r", ch, NULL, NULL, TO_ROOM);
                        return;

                case 1:
                        if (!ch->dest_buf )
                           return;
                        strcpy( arg, ch->dest_buf);
                        DISPOSE( ch->dest_buf );
                        break;

                case SUB_TIMER_DO_ABORT:
                        DISPOSE( ch->dest_buf );
                        ch->substate = SUB_NONE;
                        send_to_char("&RYou are distracted and lose focus!\n\r", ch);
                        return;

        }

        ch->substate = SUB_NONE;


        luck = ch->perm_lck;
        chance = number_range( 0, luck);

        if(chance == 0)
        {
                send_to_char("&RYou gasp and fall over, failing to raise any endurance.\n\r", ch);
                return;
        }

        time = atoi(arg);



        act( AT_PLAIN, "$n finishes their endurance training!\n\r", ch, NULL, NULL, TO_ROOM);
        send_to_char("Your blood, sweat and tears prove successful!\n\r" ,ch);
        ch->contrain += time*2;
	ch->hit -= time;

        return;
}


//increased gain - Locke
void do_stretch( CHAR_DATA *ch, char *argument)
{
        char arg[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
        int chance, luck;
        int time, extra;


        if(ch->position != POS_SITTING && ch->position != POS_RESTING)
        {
                send_to_char("&RYou must be sitting or resting to do this!\n\r", ch);
                return;
        }

        strcpy( arg, argument );
	/*
        if ( !str_cmp( arg, "stop") )
        {
		if(!IS_SET(ch->pcdata->flags, PCFLAG_EXERCISE) )
			send_to_char("&RYou aren't even stretching!\n\r" ,ch);
		else
		{
			DISPOSE( ch->dest_buf );
                        ch->substate = SUB_NONE;
			REMOVE_BIT(ch->pcdata->flags, PCFLAG_EXERCISE);
                        send_to_char("&RYou stop stretching!\n\r", ch);
		}
                return;
	}
	*/

      if ( arg[0] == '\0')
      {
               send_to_char("&RUsage: stret #\n\r" , ch);
               return;
      }

        time = atoi( arg);

        if( time < 5)
        {
                 send_to_char("&RWhy bother if you are going to do so little?\n\r", ch);
                 return;
         }
	if( time > 70)
	{
		send_to_char("&RThat is a little extreme, don't you think?\n\r", ch);
		return;
	}

        if( (ch->move - time/2) < 100)
        {
                  send_to_char("&RYour body couldn't handle that!\n\r", ch);
                  return;
        }
        send_to_char("&RYou begin to do some stretching\n\r", ch);
        act( AT_PLAIN, "$n begins to do some stretching\n\r", ch, NULL, NULL, TO_ROOM);

	if(number_range(0,5) > 3)
		extra = ch->perm_lck/2;
	else
		extra =0;

        WAIT_STATE( ch, (time*2 - extra));

        luck = ch->perm_lck*2;
        chance = number_range( 0, luck);

        if(chance == 0)
        {
                send_to_char("&RYou grab your side in pain, you pulled something from your stretching!\n\r", ch);
		ch->hit -= time/2;
                return;
        }



        act( AT_PLAIN, "$n finishes their stretching!\n\r", ch, NULL, NULL, TO_ROOM);
        send_to_char("You feel like your stretching was a success!\n\r" ,ch);
	if(IS_SET(ch->in_room->room_flags2, ROOM_YOGA))
		ch->dextrain += time*2;
	else
	        ch->dextrain += time;
        ch->move -= time;

        return;
}

//increased gain amount - Locke
void do_socialize( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int luck, chance;

    if( IS_NPC(ch) )
    {
       send_to_char( "What would be the point of that?\n\r", ch );
       return;
    }

    argument = one_argument( argument, arg1 );


    if ( arg1[0] == '\0' )
    {
        send_to_char( "Socialize with whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch  || !IS_NPC(victim))
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "They're a little busy right now.\n\r" , ch );
        return;
    }

    if ( ch->position <= POS_SLEEPING )
    {
        send_to_char( "In your dreams or what?\n\r" , ch );
        return;
    }

    if ( victim->position <= POS_SLEEPING )
    {
        send_to_char( "You might want to wake them first...\n\r" , ch );
        return;
    }

    if( ch->hit <= 100)
    {
	send_to_char("You might wanna clean up the blood on your body first.\n\r" ,ch);
	return;
    }


    act( AT_ACTION, "$n tries to strike up a conversation with you.\n\r",
ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n tries to socialize with $N.\n\r",  ch, NULL,
victim, TO_NOTVICT );

    luck = ch->perm_lck/2;
    chance = number_range( 0, luck);

    WAIT_STATE( ch, (luck*2) );

    if(chance != 0)
    {
	  if( ch->sex == 1)
          	sprintf( buf, "%s is a good guy!" , ch->name);
	 else
                sprintf( buf, "%s is a good girl!" , ch->name);
          do_yell( victim, buf );
	  ch->chatrain += 9;
    }
    else
    {
	  sprintf( buf, "%s, you are very annoying!" , ch->name);
          do_say( victim, buf );
	  sprintf( buf, "slaps %s as hard as possible!", ch->name);
	  do_emote( victim, buf );
	/*
	  act( AT_ACTION, "You slap $n!.\n\r", ch, NULL, victim, TO_VICT  );
    	  act( AT_ACTION, "$n slaps $N as hard as they can!\n\r",  victim, NULL, ch, TO_NOTVICT );
	  act( AT_ACTION, "$N slaps you as hard as they can\n\r", victim, NULL, ch, TO_CHAR);
	*/
	  ch->hit -= 50;
    }

    return;
}

void do_use( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argd[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *device;
    OBJ_DATA *obj;
    ch_ret retcode;

    argument = one_argument( argument, argd );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg , "on" ) )
       argument = one_argument( argument, arg );

    if ( argd[0] == '\0' )
    {
	send_to_char( "Use what?\n\r", ch );
	return;
    }

    if ( ( device = get_eq_char( ch, WEAR_HOLD ) ) == NULL ||
       !nifty_is_name(argd, device->name) )
    {
        do_takedrug( ch , argd );
	return;
    }

    if ( device->item_type == ITEM_SPICE )
    {
        do_takedrug( ch , argd );
        return;
    }

    if ( device->item_type != ITEM_DEVICE )
    {
	send_to_char( "You can't figure out what it is your supposed to do with it.\n\r", ch );
	return;
    }

    if ( device->value[2] <= 0 )
    {
        send_to_char( "It has no more charge left.", ch);
        return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting )
	{
	    victim = who_fighting( ch );
	}
	else
	{
	    send_to_char( "Use on whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find your target.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    if ( device->value[2] > 0 )
    {
        device->value[2]--;
	if ( victim )
	{
          if ( !oprog_use_trigger( ch, device, victim, NULL, NULL ) )
          {
	    act( AT_MAGIC, "$n uses $p on $N.", ch, device, victim, TO_ROOM );
	    act( AT_MAGIC, "You use $p on $N.", ch, device, victim, TO_CHAR );
          }
	}
	else
	{
          if ( !oprog_use_trigger( ch, device, NULL, obj, NULL ) )
          {
	    act( AT_MAGIC, "$n uses $p on $P.", ch, device, obj, TO_ROOM );
	    act( AT_MAGIC, "You use $p on $P.", ch, device, obj, TO_CHAR );
          }
	}

	retcode = obj_cast_spell( device->value[3], device->value[0], ch, victim, obj );
	if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	{
	   bug( "do_use: char died", 0 );
	   return;
	}
    }


    return;
}

void do_takedrug( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int drug;
    int sn = 0;
    int purity;
    int duration;
    bool checkapp;


   checkapp = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
   	if (obj->item_type == ITEM_APPLICATOR)
                    checkapp = TRUE;
   }

   if(checkapp == FALSE)
    {
	send_to_char("You need an Applicator to use this!\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
	send_to_char( "Use what?\n\r", ch );
	return;
    }

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( obj->item_type == ITEM_DEVICE )
    {
        send_to_char( "Try holding it first.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_SPICE )
    {
	act( AT_ACTION, "$n looks at $p and scratches $s head.", ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "You can't quite figure out what to do with $p.", ch, obj, NULL, TO_CHAR );
	return;
    }

    drug = obj->value[0];
    purity = obj->value[1];
    duration = drug*(purity*4) + 100;


    separate_obj( obj );
    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }

    if ( ch->fighting && number_percent( ) > (get_curr_dex(ch) * 2 + 48) )
    {
	act( AT_MAGIC, "$n accidentally drops $p rendering it useless.", ch, obj, NULL, TO_ROOM );
	act( AT_MAGIC, "Oops... $p gets knocked from your hands rendering it completely useless!", ch, obj, NULL ,TO_CHAR );
    }
    else
    {
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
		act( AT_ACTION, "$n sprays some $p in $s eyes.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "You spray some $p in your eyes.", ch, obj, NULL, TO_CHAR);
	}

        if ( IS_NPC(ch) )
        {
          extract_obj( obj );
          return;
        }


        WAIT_STATE( ch, PULSE_PER_SECOND/4 );

//	gain_condition( ch, COND_THIRST, 1 );
         // Affects change from drugs = Gatz
	ch->pcdata->drug_level[drug] += duration;
	/*
	if ( ch->pcdata->drug_level[drug] >=255
	     || ch->pcdata->drug_level[drug] > ( ch->pcdata->addiction[drug]+100 ) )
	{
	   act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL,
		TO_ROOM );
	   act( AT_POISON, "You feel sick. You may have taken too much.", ch, NULL, NULL, TO_CHAR );
	     ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	     af.type      = gsn_poison;
	     af.location  = APPLY_INT;
	     af.modifier  = -5;
	     af.duration  = ch->pcdata->drug_level[drug];
	     af.bitvector = AFF_POISON;
	     affect_to_char( ch, &af );
	     ch->hit = 1;
	}
	*/

        sn = skill_lookup("drug");

	if (purity == 0)
	{
		act( AT_ACTION, "$n grabs his throat and starts to gasp.",  ch, obj, NULL, TO_ROOM );
            	act( AT_ACTION, "You grab your throat and start to gasp!", ch, obj, NULL, TO_CHAR );
		af.type = sn;
		af.location = APPLY_HITROLL;
		af.modifier = -5;
		af.duration = 10;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af);

		af.type = sn;
		af.location = APPLY_DEX;
		af.modifier = -5;
		af.duration = 10;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af);

		if ( cur_obj == obj->serial )
	          global_objcode = rOBJ_EATEN;
	        extract_obj( obj );
	        return;
	}

	ch->pcdata->addiction[drug] = purity;

	// Bloody Eye, Red Eye
	if(drug == 0)
	{
	  	send_to_char("&RGotta keep those eyes open...\n\r", ch);
		af.type = sn;
		af.location = APPLY_HITROLL;
		af.modifier = (purity / 2) / (is_affected(ch, sn)?2:1);
		af.duration = duration;
		af.bitvector = AFF_BLOODYEYE;
		affect_to_char( ch, &af );

		af.type = sn;
		af.location = APPLY_DEX;
		af.modifier = purity / (is_affected(ch, sn)?2:1);
		af.duration = duration;
		af.bitvector = AFF_BLOODYEYE;
		affect_to_char( ch, &af);

		af.type = sn;
		af.location = APPLY_STR;
		af.modifier = purity / (is_affected(ch, sn)?2:1);
		af.duration = duration;
		af.bitvector = AFF_BLOODYEYE;
		affect_to_char( ch, &af);
		if ( cur_obj == obj->serial )
                  global_objcode = rOBJ_EATEN;
                extract_obj( obj );
                return;

	}
	// Yellow Eye
	if(drug == 1)
	{
			send_to_char("&YUrge to kill rising...\n\r",ch);
			af.type = sn;
			af.location = APPLY_LCK;
			af.modifier = (purity - drug)  / (is_affected(ch, sn)?2:1);
			af.duration = duration;
			af.bitvector = AFF_NONE;
			affect_to_char( ch, &af);

			af.type = sn;
			af.location = APPLY_DAMROLL;
			af.modifier = (purity - drug)  / (is_affected(ch, sn)?2:1);
			af.duration = duration;
			af.bitvector = AFF_NONE;
			affect_to_char( ch, &af);
			if ( cur_obj == obj->serial )
        	          global_objcode = rOBJ_EATEN;
                	extract_obj( obj );
              	        return;

	}
	// Purple Eye
	if(drug == 2)
	{
		send_to_char("&pFight...fight...fight...\n\r",ch);
		af.type = sn;
		af.location = APPLY_HITROLL;
		af.modifier = purity / (is_affected(ch, sn)?2:1);
		af.duration = duration;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af);
		if ( cur_obj == obj->serial )
                  global_objcode = rOBJ_EATEN;
                extract_obj( obj );
                return;

	}


        // Blue Eye
	if(drug == 3);
	{
		send_to_char("&BYour muscles ache with the desire to kill.\n\r",ch);
		af.type = sn;
		af.location =  APPLY_DEX;
		af.modifier = purity / (is_affected(ch, sn)?2:1);
		af.duration = duration;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af);
	                if ( cur_obj == obj->serial )
                global_objcode = rOBJ_EATEN;
                extract_obj( obj );
                return;

	}
	// Green Eye
	if(drug == 4)
	{
		send_to_char("&GYou become green with envious rage.\n\r",ch);
		af.type = sn;
		af.location = APPLY_STR;
		af.modifier = purity / (is_affected(ch, sn)?2:1);
		af.duration = duration;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af);
		if ( cur_obj == obj->serial )
                  global_objcode = rOBJ_EATEN;
                extract_obj( obj );
                return;

	}


	// Just used for reference - Gatz
	/*
	switch (drug)
	{
	    default:
	    case SPICE_GLITTERSTIM:

                   af.type      = -1;
                   af.location  = APPLY_HITROLL;
                   af.modifier  = 10;
		   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
                   af.bitvector = AFF_NONE;
                   affect_to_char( ch, &af );

                   af.type      = sn;
                   af.location  = APPLY_DEX;
                   af.modifier  = 10;
                   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
                   af.bitvector = AFF_NONE;
                   affect_to_char( ch, &af );

                   af.type      = sn;
                   af.location  = APPLY_STR;
                   af.modifier  = 10;
                   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
                   af.bitvector = AFF_NONE;
                   affect_to_char( ch, &af );

	       break;

            case SPICE_CARSANUM:

               sn=skill_lookup("sanctuary");
	       if ( sn < MAX_SKILL && !IS_AFFECTED( ch, AFF_SANCTUARY ) )
	       {
	  	   af.type      = sn;
	  	   af.location  = APPLY_NONE;
	  	   af.modifier  = 0;
	  	   af.duration  = URANGE( 1, ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug] ,obj->value[1] );
	  	   af.bitvector = AFF_SANCTUARY;
	  	   affect_to_char( ch, &af );
	       }
	       break;

            case SPICE_RYLL:

	  	   af.type      = -1;
	  	   af.location  = APPLY_DEX;
	  	   af.modifier  = 1;
	  	   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
	  	   af.bitvector = AFF_NONE;
	  	   affect_to_char( ch, &af );

	  	   af.type      = -1;
	  	   af.location  = APPLY_HITROLL;
	  	   af.modifier  = 1;
	  	   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
	  	   af.bitvector = AFF_NONE;
	  	   affect_to_char( ch, &af );

	        break;

            case SPICE_ANDRIS:

	  	   af.type      = -1;
	  	   af.location  = APPLY_HITROLL;
	  	   af.modifier  = 10;
	  	   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
	  	   af.bitvector = AFF_NONE;
	  	   affect_to_char( ch, &af );

	           af.type      = sn;
	  	   af.location  = APPLY_CON;
	  	   af.modifier  = 1;
	  	   af.duration  = URANGE( 1, 2*(ch->pcdata->drug_level[drug] - ch->pcdata->addiction[drug]) ,2*obj->value[1] );
	  	   af.bitvector = AFF_NONE;
	  	   affect_to_char( ch, &af );

	       break;

        }
	*/
    }

    if ( cur_obj == obj->serial )
      global_objcode = rOBJ_EATEN;

    extract_obj( obj );

    return;
}

void jedi_bonus( CHAR_DATA *ch )
{
   if ( number_range( 1 , 100 ) == 1 )
   {
        ch->max_mana++;
   	send_to_char("&YYou are wise in your use of the force.\n\r", ch);
   	send_to_char("You feel a little stronger in your wisdom.&w\n\r", ch);
   }
}

void sith_penalty( CHAR_DATA *ch )
{
   if ( number_range( 1 , 100 ) == 1 )
   {
        ch->max_mana++ ;
        if (ch->max_hit > 100)
   	  ch->max_hit--   ;
   	ch->hit--   ;
   	send_to_char("&zYour body grows weaker as your strength in the dark side grows.&w\n\r",ch);
   }
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *source;
    sh_int    dest_item, src_item1, src_item2, src_item3, src_item4;
    int       diff;
    bool      all = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* munch optional words */
    if ( (!str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ))
    &&    argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    else
	dest_item = obj->item_type;

    src_item1 = src_item2 = src_item3 = src_item4 = -1;
    switch( dest_item )
    {
	default:
	  act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
	  send_to_char( "You cannot fill that.\n\r", ch );
	  return;
	/* place all fillable item types here */
	case ITEM_DRINK_CON:
	  src_item1 = ITEM_FOUNTAIN;	src_item2 = ITEM_BLOOD;		break;
	case ITEM_HERB_CON:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_PIPE:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_CONTAINER:
	  src_item1 = ITEM_CONTAINER;	src_item2 = ITEM_CORPSE_NPC;
	  src_item3 = ITEM_CORPSE_PC;	src_item4 = ITEM_CORPSE_NPC;    break;
    }

    if ( dest_item == ITEM_CONTAINER )
    {
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
	    return;
	}
	if ( get_obj_weight( obj ) / obj->count
	>=   obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }
    else
    {
	diff = obj->value[0] - obj->value[1];
	if ( diff < 1 || obj->value[1] >= obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }

    if ( dest_item == ITEM_PIPE
    &&   IS_SET( obj->value[3], PIPE_FULLOFASH ) )
    {
	send_to_char( "It's full of ashes, and needs to be emptied first.\n\r", ch );
	return;
    }

    if ( arg2[0] != '\0' )
    {
      if ( dest_item == ITEM_CONTAINER
      && (!str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 )) )
      {
	all = TRUE;
	source = NULL;
      }
      else
      /* This used to let you fill a pipe from an object on the ground.  Seems
         to me you should be holding whatever you want to fill a pipe with.
         It's nitpicking, but I needed to change it to get a mobprog to work
         right.  Check out Lord Fitzgibbon if you're curious.  -Narn */
      if ( dest_item == ITEM_PIPE )
      {
        if ( ( source = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You don't have that item.\n\r", ch );
	   return;
	}
	if ( source->item_type != src_item1 && source->item_type != src_item2
	&&   source->item_type != src_item3 &&   source->item_type != src_item4  )
	{
	   act( AT_PLAIN, "You cannot fill $p with $P!", ch, obj, source, TO_CHAR );
	   return;
	}
      }
      else
      {
	if ( ( source =  get_obj_here( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You cannot find that item.\n\r", ch );
	   return;
	}
      }
    }
    else
	source = NULL;

    if ( !source && dest_item == ITEM_PIPE )
    {
	send_to_char( "Fill it with what?\n\r", ch );
	return;
    }

    if ( !source )
    {
	bool      found = FALSE;
	OBJ_DATA *src_next;

	found = FALSE;
	separate_obj( obj );
	for ( source = ch->in_room->first_content;
	      source;
	      source = src_next )
	{
	    src_next = source->next_content;
	    if (dest_item == ITEM_CONTAINER)
	    {
		if ( !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
		||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		  continue;
		if ( all && arg2[3] == '.'
		&&  !nifty_is_name( &arg2[4], source->name ) )
		   continue;
		obj_from_room(source);
		if ( source->item_type == ITEM_MONEY )
		{
		   ch->gold += source->value[0];
		   extract_obj( source );
		}
		else
		   obj_to_obj(source, obj);
		found = TRUE;
	    }
	    else
	    if (source->item_type == src_item1
	    ||  source->item_type == src_item2
	    ||  source->item_type == src_item3
	    ||  source->item_type == src_item4 )
	    {
		found = TRUE;
		break;
	    }
	}
	if ( !found )
	{
	    switch( src_item1 )
	    {
		default:
		  send_to_char( "There is nothing appropriate here!\n\r", ch );
		  return;
		case ITEM_FOUNTAIN:
		  send_to_char( "There is no fountain or pool here!\n\r", ch );
		  return;
		case ITEM_BLOOD:
		  send_to_char( "There is no blood pool here!\n\r", ch );
		  return;
		case ITEM_HERB_CON:
		  send_to_char( "There are no herbs here!\n\r", ch );
		  return;
		case ITEM_HERB:
		  send_to_char( "You cannot find any smoking herbs.\n\r", ch );
		  return;
	    }
	}
	if (dest_item == ITEM_CONTAINER)
	{
	  act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
	  return;
	}
    }

    if (dest_item == ITEM_CONTAINER)
    {
	OBJ_DATA *otmp, *otmp_next;
	char name[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	char *pd;
	bool found = FALSE;

	if ( source == obj )
	{
	    send_to_char( "You can't fill something with itself!\n\r", ch );
	    return;
	}

	switch( source->item_type )
	{
	    default:	/* put something in container */
		if ( !source->in_room	/* disallow inventory items */
		||   !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
		||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
		obj_from_room(source);
		obj_to_obj(source, obj);
		break;
	    case ITEM_MONEY:
		send_to_char( "You can't do that... yet.\n\r", ch );
		break;
	    case ITEM_CORPSE_PC:
		if ( IS_NPC(ch) )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}

		    pd = source->short_descr;
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );

		    if ( str_cmp( name, ch->name ) && !IS_IMMORTAL(ch) )
		    {
			bool fGroup;

			fGroup = FALSE;
			for ( gch = first_char; gch; gch = gch->next )
			{
			    if ( !IS_NPC(gch)
			    &&   is_same_group( ch, gch )
			    &&   !str_cmp( name, gch->name ) )
			    {
				fGroup = TRUE;
				break;
			    }
			}
			if ( !fGroup )
			{
			    send_to_char( "That's someone else's corpse.\n\r", ch );
			    return;
			}
		     }

	    case ITEM_CONTAINER:
		if ( source->item_type == ITEM_CONTAINER  /* don't remove */
		&&   IS_SET(source->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
		    return;
		}
	    case ITEM_DROID_CORPSE:
	    case ITEM_CORPSE_NPC:
		if ( (otmp=source->first_content) == NULL )
		{
		    send_to_char( "It's empty.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		for ( ; otmp; otmp = otmp_next )
		{
		    otmp_next = otmp->next_content;

		    if ( !CAN_WEAR(otmp, ITEM_TAKE)
		    ||   (IS_OBJ_STAT( otmp, ITEM_PROTOTYPE) && !can_take_proto(ch))
		    ||    ch->carry_number + otmp->count > can_carry_n(ch)
		    ||    ch->carry_weight + get_obj_weight(otmp) > can_carry_w(ch)
		    ||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
			> obj->value[0] )
			continue;
		    obj_from_obj(otmp);
		    obj_to_obj(otmp, obj);
		    found = TRUE;
		}
		if ( found )
		{
		   act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		   act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		}
		else
		   send_to_char( "There is nothing appropriate in there.\n\r", ch );
		break;
	}
	return;
    }

    if ( source->value[1] < 1 )
    {
	send_to_char( "There's none left!\n\r", ch );
	return;
    }
    if ( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
      separate_obj( source );
    separate_obj( obj );

    switch( source->item_type )
    {
	default:
	  bug( "do_fill: got bad item type: %d", source->item_type );
	  send_to_char( "Something went wrong...\n\r", ch );
	  return;
	case ITEM_FOUNTAIN:
	  if ( obj->value[1] != 0 && obj->value[2] != 0 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 0;
	  obj->value[1] = obj->value[0];
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_BLOOD:
	  if ( obj->value[1] != 0 && obj->value[2] != 13 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 13;
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  if ( (source->value[1] -= diff) < 1 )
	  {
 	     extract_obj( source );
	     make_bloodstain( ch );
	  }
	  return;
	case ITEM_HERB:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another type of herb in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  act( AT_ACTION, "You fill $p with $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p with $P.", ch, obj, source, TO_ROOM );
	  if ( (source->value[1] -= diff) < 1 )
 	     extract_obj( source );
	  return;
	case ITEM_HERB_CON:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another type of herb in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    argument = one_argument( argument, arg );
    /* munch optional words */
    if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    if ( (obj->item_type == ITEM_FOUNTAIN)
	    ||   (obj->item_type == ITEM_BLOOD) )
		break;

	if ( !obj )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
	separate_obj(obj);

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 40 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	if ( obj->carried_by == ch )
	{
	    act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
	}
	else
	{
	    act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
	}
	break;

    case ITEM_POTION:
	if ( obj->carried_by == ch )
	   do_quaff( ch, obj->name );
	else
	   send_to_char( "You're not carrying that.\n\r", ch );
	break;

    case ITEM_FOUNTAIN:
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
	   send_to_char( "You take a long thirst quenching drink.\n\r", ch );
	}

	if ( !IS_NPC(ch) )
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	//Removed liq_table[liquid].liq_name, who wants to see that? - Gatz
	   act( AT_ACTION, "$n drinks from $p.",
		ch, obj, NULL, TO_ROOM );
	   act( AT_ACTION, "You drink from $p.",
		ch, obj, NULL, TO_CHAR );
	}

	amount = 1; /* UMIN(amount, obj->value[1]); */
	/* what was this? concentrated drinks?  concentrated water
	   too I suppose... sheesh! */

	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );

	if ( !IS_NPC(ch) )
	{
	    if ( ch->pcdata->condition[COND_DRUNK]  > 24 )
		send_to_char( "You feel quite sloshed.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 18 )
		send_to_char( "You feel very drunk.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 12 )
		send_to_char( "You feel drunk.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 8 )
		send_to_char( "You feel a little drunk.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 5 )
		send_to_char( "You feel light headed.\n\r", ch );
	    if ( ch->pcdata->condition[COND_THIRST] > 30 )
		send_to_char( "You do not feel thirsty.\n\r", ch );
	}

	if ( obj->value[3] )
	{
	    /* The drink was poisoned! */
	    AFFECT_DATA af;

	    act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
	    act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
	    ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	    af.type      = gsn_poison;
	    af.duration  = 3 * obj->value[3];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}

	obj->value[1] -= amount;
	break;
    }
    WAIT_STATE(ch, PULSE_PER_SECOND );
    return;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ch_ret retcode;
    int foodcond;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) || ch->pcdata->condition[COND_FULL] > 5 )
	if ( ms_find_obj(ch) )
	    return;

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)",  ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
	    return;
	}
    }

    /* required due to object grouping */
    separate_obj( obj );

    WAIT_STATE( ch, PULSE_PER_SECOND/2 );

    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }
    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
      if ( !obj->action_desc || obj->action_desc[0]=='\0' )
      {
        act( AT_ACTION, "$n eats $p.",  ch, obj, NULL, TO_ROOM );
        act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
      }
      else
        actiondesc( ch, obj, NULL );
    }

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( obj->timer > 0 && obj->value[1] > 0 )
	   foodcond = (obj->timer * 10) / obj->value[1];
	else
	   foodcond = 10;

	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    if ( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	}

	if (  obj->value[3] != 0
	||   (foodcond < 4 && number_range( 0, foodcond + 1 ) == 0) )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    if ( obj->value[3] != 0 )
	    {
		act( AT_POISON, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
		act( AT_POISON, "You choke and gag.", ch, NULL, NULL, TO_CHAR );
		ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	    }
	    else
	    {
		act( AT_POISON, "$n gags on $p.", ch, obj, NULL, TO_ROOM );
		act( AT_POISON, "You gag on $p.", ch, obj, NULL, TO_CHAR );
		ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
	    }

	    af.type      = gsn_poison;
	    af.duration  = 2 * obj->value[0]
	    		 * (obj->value[3] > 0 ? obj->value[3] : 1);
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
	/* allow pills to fill you, if so desired */
	if ( !IS_NPC(ch) && obj->value[4] )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    if ( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	}
	retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

    if ( obj->serial == cur_obj )
      global_objcode = rOBJ_EATEN;
    extract_obj( obj );
    return;
}

void do_quaff( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ch_ret retcode;

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( obj->item_type != ITEM_POTION )
    {
	if ( obj->item_type == ITEM_DRINK_CON )
	   do_drink( ch, obj->name );
	else
	{
	   act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
	   act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
	}
	return;
    }

    separate_obj( obj );
    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }

    /*
     * If fighting, chance of dropping potion			-Thoric
     */
    if ( ch->fighting && number_percent( ) > (get_curr_dex(ch) * 2 + 48) )
    {
	act( AT_MAGIC, "$n accidentally drops $p and it smashes into a thousand fragments.", ch, obj, NULL, TO_ROOM );
	act( AT_MAGIC, "Oops... $p gets knocked from your hands and smashes into pieces!", ch, obj, NULL ,TO_CHAR );
    }
    else
    {
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	    act( AT_ACTION, "$n quaffs $p.",  ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You quaff $p.", ch, obj, NULL, TO_CHAR );
	}

        WAIT_STATE( ch, PULSE_PER_SECOND/4 );

	retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    }
    if ( cur_obj == obj->serial )
      global_objcode = rOBJ_QUAFFED;
    extract_obj( obj );
    return;
}


void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;
    ch_ret    retcode;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Activate what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	act( AT_ACTION, "$n attempts to activate $p ... the silly fool.",  ch, scroll, NULL, TO_ROOM );
	act( AT_ACTION, "You try to activate $p. (Now what?)", ch, scroll, NULL, TO_CHAR );
	return;
    }



    obj = NULL;
    if ( arg2[0] == '\0' )
	victim = ch;
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    separate_obj( scroll );
    act( AT_MAGIC, "$n activate $p.", ch, scroll, NULL, TO_ROOM );
    act( AT_MAGIC, "You activate $p.", ch, scroll, NULL, TO_CHAR );


    WAIT_STATE( ch, PULSE_PER_SECOND/2 );

    retcode = obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    if ( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    if ( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );

    if ( scroll->serial == cur_obj )
      global_objcode = rOBJ_USED;
    extract_obj( scroll );
    return;
}


/*
 * Function to handle the state changing of a triggerobject (lever)  -Thoric
 */
void pullorpush( CHAR_DATA *ch, OBJ_DATA *obj, bool pull )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA		*rch;
    bool		 isup;
    ROOM_INDEX_DATA	*room,  *to_room;
    EXIT_DATA		*pexit, *pexit_rev;
    int			 edir;
    char		*txt;

    if ( IS_SET( obj->value[0], TRIG_UP ) )
      isup = TRUE;
    else
      isup = FALSE;
    switch( obj->item_type )
    {
	default:
	  sprintf( buf, "You can't %s that!\n\r", pull ? "pull" : "push" );
	  send_to_char( buf, ch );
	  return;
	  break;
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
	  if ( (!pull && isup) || (pull && !isup) )
	  {
		sprintf( buf, "It is already %s.\n\r", isup ? "up" : "down" );
		send_to_char( buf, ch );
		return;
 	  }
	case ITEM_BUTTON:
	  if ( (!pull && isup) || (pull & !isup) )
	  {
		sprintf( buf, "It is already %s.\n\r", isup ? "in" : "out" );
		send_to_char( buf, ch );
		return;
	  }
	  break;
    }
    if( (pull) && IS_SET(obj->pIndexData->progtypes,PULL_PROG) )
    {
	if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )
 	  REMOVE_BIT( obj->value[0], TRIG_UP );
 	oprog_pull_trigger( ch, obj );
        return;
    }
    if( (!pull) && IS_SET(obj->pIndexData->progtypes,PUSH_PROG) )
    {
	if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )
	  SET_BIT( obj->value[0], TRIG_UP );
	oprog_push_trigger( ch, obj );
        return;
    }

    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
      sprintf( buf, "$n %s $p.", pull ? "pulls" : "pushes" );
      act( AT_ACTION, buf,  ch, obj, NULL, TO_ROOM );
      sprintf( buf, "You %s $p.", pull ? "pull" : "push" );
      act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
    }

    if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )
    {
	if ( pull )
	  REMOVE_BIT( obj->value[0], TRIG_UP );
	else
	  SET_BIT( obj->value[0], TRIG_UP );
    }
    if ( IS_SET( obj->value[0], TRIG_TELEPORT )
    ||   IS_SET( obj->value[0], TRIG_TELEPORTALL )
    ||   IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
    {
	int flags;

	if ( ( room = get_room_index( obj->value[1] ) ) == NULL )
	{
	    bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );
	    return;
	}
	flags = 0;
	if ( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )
	  SET_BIT( flags, TELE_SHOWDESC );
	if ( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )
	  SET_BIT( flags, TELE_TRANSALL );
	if ( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
	  SET_BIT( flags, TELE_TRANSALLPLUS );

	teleport( ch, obj->value[1], flags );
	return;
    }

    if ( IS_SET( obj->value[0], TRIG_RAND4 )
    ||	 IS_SET( obj->value[0], TRIG_RAND6 ) )
    {
	int maxd;

	if ( ( room = get_room_index( obj->value[1] ) ) == NULL )
	{
	    bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );
	    return;
	}

	if ( IS_SET( obj->value[0], TRIG_RAND4 ) )
	  maxd = 3;
	else
	  maxd = 5;

	randomize_exits( room, maxd );
	for ( rch = room->first_person; rch; rch = rch->next_in_room )
	{
	   send_to_char( "You hear a loud rumbling sound.\n\r", rch );
	   send_to_char( "Something seems different...\n\r", rch );
	}
    }
    if ( IS_SET( obj->value[0], TRIG_DOOR ) )
    {
	room = get_room_index( obj->value[1] );
	if ( !room )
	  room = obj->in_room;
	if ( !room )
	{
	  bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );
	  return;
	}
	if ( IS_SET( obj->value[0], TRIG_D_NORTH ) )
	{
	  edir = DIR_NORTH;
	  txt = "to the north";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_SOUTH ) )
	{
	  edir = DIR_SOUTH;
	  txt = "to the south";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_EAST ) )
	{
	  edir = DIR_EAST;
	  txt = "to the east";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_WEST ) )
	{
	  edir = DIR_WEST;
	  txt = "to the west";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_UP ) )
	{
	  edir = DIR_UP;
	  txt = "from above";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_DOWN ) )
	{
	  edir = DIR_DOWN;
	  txt = "from below";
	}
	else
	{
	  bug( "PullOrPush: door: no direction flag set.", 0 );
	  return;
	}
	pexit = get_exit( room, edir );
	if ( !pexit )
	{
	    if ( !IS_SET( obj->value[0], TRIG_PASSAGE ) )
	    {
		bug( "PullOrPush: obj points to non-exit %d", obj->value[1] );
		return;
	    }
	    to_room = get_room_index( obj->value[2] );
	    if ( !to_room )
	    {
		bug( "PullOrPush: dest points to invalid room %d", obj->value[2] );
		return;
	    }
	    pexit = make_exit( room, to_room, edir );
	    pexit->keyword	= STRALLOC( "" );
	    pexit->description	= STRALLOC( "" );
	    pexit->key		= -1;
	    pexit->exit_info	= 0;
	    top_exit++;
	    act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR );
	    act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM );
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_UNLOCK )
	&&   IS_SET( pexit->exit_info, EX_LOCKED) )
	{
	    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
		REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_LOCK   )
	&&  !IS_SET( pexit->exit_info, EX_LOCKED) )
	{
	    SET_BIT(pexit->exit_info, EX_LOCKED);
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
		SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_OPEN   )
	&&   IS_SET( pexit->exit_info, EX_CLOSED) )
	{
	    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	    for ( rch = room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d opens.", rch, NULL, pexit->keyword, TO_CHAR );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
		for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	    check_room_for_traps( ch, trap_door[edir]);
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_CLOSE   )
	&&  !IS_SET( pexit->exit_info, EX_CLOSED) )
	{
	    SET_BIT(pexit->exit_info, EX_CLOSED);
	    for ( rch = room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d closes.", rch, NULL, pexit->keyword, TO_CHAR );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		SET_BIT( pexit_rev->exit_info, EX_CLOSED );
		for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	    check_room_for_traps( ch, trap_door[edir]);
	    return;
	}
    }
}


void do_pull( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Pull what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
    }

    pullorpush( ch, obj, TRUE );
}

void do_push( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Push what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
    }

    pullorpush( ch, obj, FALSE );
}

/* pipe commands (light, tamp, smoke) by Thoric */
void do_tamp( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pipe;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Tamp what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (pipe = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( pipe->item_type != ITEM_PIPE )
    {
	send_to_char( "You can't tamp that.\n\r", ch );
	return;
    }
    if ( !IS_SET( pipe->value[3], PIPE_TAMPED ) )
    {
	act( AT_ACTION, "You gently tamp $p.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n gently tamps $p.", ch, pipe, NULL, TO_ROOM );
	SET_BIT( pipe->value[3], PIPE_TAMPED );
	return;
    }
    send_to_char( "It doesn't need tamping.\n\r", ch );
}

void do_smoke( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pipe;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Smoke what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (pipe = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( pipe->item_type != ITEM_PIPE )
    {
	act( AT_ACTION, "You try to smoke $p... but it doesn't seem to work.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n tries to smoke $p... (I wonder what $e's been putting his $s pipe?)", ch, pipe, NULL, TO_ROOM );
	return;
    }
    if ( !IS_SET( pipe->value[3], PIPE_LIT ) )
    {
	act( AT_ACTION, "You try to smoke $p, but it's not lit.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n tries to smoke $p, but it's not lit.", ch, pipe, NULL, TO_ROOM );
	return;
    }
    if ( pipe->value[1] > 0 )
    {
	if ( !oprog_use_trigger( ch, pipe, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "You draw thoughtfully from $p.", ch, pipe, NULL, TO_CHAR );
	   act( AT_ACTION, "$n draws thoughtfully from $p.", ch, pipe, NULL, TO_ROOM );
	}

	if ( IS_VALID_HERB( pipe->value[2] ) && pipe->value[2] < top_herb )
	{
	    int sn		= pipe->value[2] + TYPE_HERB;
	    SKILLTYPE *skill	= get_skilltype( sn );

	    WAIT_STATE( ch, skill->beats );
	    if ( skill->spell_fun )
		obj_cast_spell( sn, UMIN(skill->min_level, ch->top_level),
			ch, ch, NULL );
	    if ( obj_extracted( pipe ) )
		return;
	}
	else
	    bug( "do_smoke: bad herb type %d", pipe->value[2] );

	SET_BIT( pipe->value[3], PIPE_HOT );
	if ( --pipe->value[1] < 1 )
	{
	   REMOVE_BIT( pipe->value[3], PIPE_LIT );
	   SET_BIT( pipe->value[3], PIPE_DIRTY );
	   SET_BIT( pipe->value[3], PIPE_FULLOFASH );
	}
    }
}

void do_light( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pipe;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Light what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (pipe = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( pipe->item_type != ITEM_PIPE )
    {
	send_to_char( "You can't light that.\n\r", ch );
	return;
    }
    if ( !IS_SET( pipe->value[3], PIPE_LIT ) )
    {
	if ( pipe->value[1] < 1 )
	{
	  act( AT_ACTION, "You try to light $p, but it's empty.", ch, pipe, NULL, TO_CHAR );
	  act( AT_ACTION, "$n tries to light $p, but it's empty.", ch, pipe, NULL, TO_ROOM );
	  return;
	}
	act( AT_ACTION, "You carefully light $p.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n carefully lights $p.", ch, pipe, NULL, TO_ROOM );
	SET_BIT( pipe->value[3], PIPE_LIT );
	return;
    }
    send_to_char( "It's already lit.\n\r", ch );
}

void do_empty( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Empty what?\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( obj->count > 1 )
      separate_obj(obj);

    switch( obj->item_type )
    {
	default:
	  act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
	  return;
	case ITEM_PIPE:
	  act( AT_ACTION, "You gently tap $p and empty it out.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n gently taps $p and empties it out.", ch, obj, NULL, TO_ROOM );
	  REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );
	  REMOVE_BIT( obj->value[3], PIPE_LIT );
	  obj->value[1] = 0;
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] < 1 )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
	  obj->value[1] = 0;
	  return;
	case ITEM_CONTAINER:
	  if ( IS_SET(obj->value[1], CONT_CLOSED) )
	  {
		act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
		return;
	  }
	  if ( !obj->first_content )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  if ( arg2[0] == '\0' )
	  {
		if ( IS_SET( ch->in_room->room_flags, ROOM_NODROP )
		|| ( !IS_NPC(ch) &&  IS_SET( ch->act, PLR_LITTERBUG ) ) )
		{
		       set_char_color( AT_MAGIC, ch );
		       send_to_char( "A magical force stops you!\n\r", ch );
		       set_char_color( AT_TELL, ch );
		       send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
		       return;
		}
		if ( IS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
		{
		   send_to_char( "You can't seem to do that here...\n\r", ch );
		   return;
		}
		if ( empty_obj( obj, NULL, ch->in_room ) )
		{
		    act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
		    act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
		    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
			save_char_obj( ch );
		}
		else
		    send_to_char( "Hmmm... didn't work.\n\r", ch );
	  }
	  else
	  {
		OBJ_DATA *dest = get_obj_here( ch, arg2 );

		if ( !dest )
		{
		    send_to_char( "You can't find it.\n\r", ch );
		    return;
		}
		if ( dest == obj )
		{
		    send_to_char( "You can't empty something into itself!\n\r", ch );
		    return;
		}
		if ( dest->item_type != ITEM_CONTAINER )
		{
		    send_to_char( "That's not a container!\n\r", ch );
		    return;
		}
		if ( IS_SET(dest->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
		    return;
		}
		separate_obj( dest );
		if ( empty_obj( obj, dest, NULL ) )
		{
		    act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
		    act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
		    if ( !dest->carried_by
		    &&    IS_SET( sysdata.save_flags, SV_PUT ) )
			save_char_obj( ch );
		}
		else
		    act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
	  }
	  return;
    }
}

/*
 * Apply a salve/ointment					-Thoric
 */
void do_apply( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ch_ret retcode;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Apply what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_SALVE )
    {
	act( AT_ACTION, "$n starts to rub $p on $mself...",  ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "You try to rub $p on yourself...", ch, obj, NULL, TO_CHAR );
	return;
    }

    separate_obj( obj );

    --obj->value[1];
    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
	if ( !obj->action_desc || obj->action_desc[0]=='\0' )
	{
	    act( AT_ACTION, "$n rubs $p onto $s body.",  ch, obj, NULL, TO_ROOM );
	    if ( obj->value[1] <= 0 )
		act( AT_ACTION, "You apply the last of $p onto your body.", ch, obj, NULL, TO_CHAR );
	    else
		act( AT_ACTION, "You apply $p onto your body.", ch, obj, NULL, TO_CHAR );
	}
	else
	    actiondesc( ch, obj, NULL );
    }

    WAIT_STATE( ch, obj->value[2] );
    retcode = obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
    if ( retcode == rNONE )
	retcode = obj_cast_spell( obj->value[5], obj->value[0], ch, ch, NULL );

    if ( !obj_extracted(obj) && obj->value[1] <= 0 )
	extract_obj( obj );

    return;
}

void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
    char charbuf[MAX_STRING_LENGTH];
    char roombuf[MAX_STRING_LENGTH];
    char *srcptr = obj->action_desc;
    char *charptr = charbuf;
    char *roomptr = roombuf;
    const char *ichar;
    const char *iroom;

while ( *srcptr != '\0' )
{
  if ( *srcptr == '$' )
  {
    srcptr++;
    switch ( *srcptr )
    {
      case 'e':
        ichar = "you";
        iroom = "$e";
        break;

      case 'm':
        ichar = "you";
        iroom = "$m";
        break;

      case 'n':
        ichar = "you";
        iroom = "$n";
        break;

      case 's':
        ichar = "your";
        iroom = "$s";
        break;

      /*case 'q':
        iroom = "s";
        break;*/

      default:
        srcptr--;
        *charptr++ = *srcptr;
        *roomptr++ = *srcptr;
        break;
    }
  }
  else if ( *srcptr == '%' && *++srcptr == 's' )
  {
    ichar = "You";
    iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;
  }
  else
  {
    *charptr++ = *srcptr;
    *roomptr++ = *srcptr;
    srcptr++;
    continue;
  }

  while ( ( *charptr = *ichar ) != '\0' )
  {
    charptr++;
    ichar++;
  }

  while ( ( *roomptr = *iroom ) != '\0' )
  {
    roomptr++;
    iroom++;
  }
  srcptr++;
}

*charptr = '\0';
*roomptr = '\0';

/*
sprintf( buf, "Charbuf: %s", charbuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER );
sprintf( buf, "Roombuf: %s", roombuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER );
*/

switch( obj->item_type )
{
  case ITEM_BLOOD:
  case ITEM_FOUNTAIN:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  case ITEM_DRINK_CON:
    act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );
    return;

  case ITEM_PIPE:
    return;

  case ITEM_ARMOR:
  case ITEM_WEAPON:
  case ITEM_LIGHT:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  case ITEM_FOOD:
  case ITEM_PILL:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  default:
    return;
}
return;
}

void do_hail( CHAR_DATA *ch , char *argument )
{
    int vnum;
    ROOM_INDEX_DATA *room;

    if ( !ch->in_room )
       return;

    if ( ch->position < POS_FIGHTING )
    {
       send_to_char( "You might want to stop fighting first!\n\r", ch );
       return;
    }

    if ( ch->position < POS_STANDING )
    {
       send_to_char( "You might want to stand up first!\n\r", ch );
       return;
    }

    if ( IS_SET( ch->in_room->room_flags , ROOM_INDOORS ) )
    {
       send_to_char( "You'll have to go outside to do that!\n\r", ch );
       return;
    }

    if ( IS_SET( ch->in_room->room_flags , ROOM_SPACECRAFT ) )
    {
       send_to_char( "You can't do that on spacecraft!\n\r", ch );
       return;
    }

    if ( ch->gold < (ch->top_level-9)  )
    {
       send_to_char( "You don't have enough wulongs!\n\r", ch );
       return;
    }

    vnum = ch->in_room->vnum;

    for ( vnum = ch->in_room->area->low_r_vnum  ;  vnum <= ch->in_room->area->hi_r_vnum  ;  vnum++ )
    {
            room = get_room_index ( vnum );

            if ( room != NULL )
             if ( IS_SET(room->room_flags , ROOM_HOTEL ) )
                break;
             else
                room = NULL;
    }

    if ( room == NULL )
    {
       send_to_char( "There doesn't seem to be any taxis nearby!\n\r", ch );
       return;
    }

    ch->gold -= UMAX(ch->top_level-9 , 0);

    act( AT_ACTION, "$n hails a speederbike, and drives off to seek shelter.", ch, NULL, NULL,  TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, room );

    send_to_char( "A speederbike picks you up and drives you to a safe location.\n\rYou pay the driver 20 wulongs.\n\r\n\n" , ch );
    act( AT_ACTION, "$n $T", ch, NULL, "arrives on a speederbike, gets off and pays the driver before it leaves.",  TO_ROOM );

    do_look( ch, "auto" );

}

void do_train( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *mob;
    bool tfound = FALSE;
    bool successful = FALSE;

    if ( IS_NPC(ch) )
	return;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:

	    	if ( arg[0] == '\0' )
                {
                   send_to_char( "Train what?\n\r", ch );
	           send_to_char( "\n\rChoices: strength, intelligence, wisdom, dexterity, constitution or charisma\n\r", ch );
	           return;
                }

	    	if ( !IS_AWAKE(ch) )
	    	{
	          send_to_char( "In your dreams, or what?\n\r", ch );
	          return;
	    	}

	    	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	       	   if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	           {
	              tfound = TRUE;
	    	      break;
                   }

	    	if ( (!mob) || (!tfound) )
	    	{
	          send_to_char( "You can't do that here.\n\r", ch );
	          return;
	    	}

	        if ( str_cmp( arg, "str" ) && str_cmp( arg, "strength" )
	        && str_cmp( arg, "dex" ) && str_cmp( arg, "dexterity" )
                && str_cmp( arg, "con" ) && str_cmp( arg, "constitution" )
                && str_cmp( arg, "cha" ) && str_cmp( arg, "charisma" )
                && str_cmp( arg, "wis" ) && str_cmp( arg, "wisdom" )
                && str_cmp( arg, "int" ) && str_cmp( arg, "intelligence" ) )
                {
                    do_train ( ch , "" );
                    return;
                }

                if ( !str_cmp( arg, "str" ) || !str_cmp( arg, "strength" ) )
                {
                      if( mob->perm_str <= ch->perm_str || ch->perm_str >= 20 + race_table[ch->race].str_plus || ch->perm_str >= 25 )
                      {
                          act( AT_TELL, "$n tells you 'I cannot help you... you are already stronger than I.'",
		             mob, NULL, ch, TO_VICT );
		          return;
                      }
                      send_to_char("&GYou begin your weight training.\n\r", ch);
                }
          	if ( !str_cmp( arg, "dex" ) || !str_cmp( arg, "dexterity" ) )
	    	{
                      if( mob->perm_dex <= ch->perm_dex || ch->perm_dex >= 20 + race_table[ch->race].dex_plus || ch->perm_dex >= 25 )
                      {
                          act( AT_TELL, "$n tells you 'I cannot help you... you are already more dextrous than I.'",
		             mob, NULL, ch, TO_VICT );
		          return;
                      }
                      send_to_char("&GYou begin to work at some challenging tests of coordination.\n\r", ch);
                }
          	if ( !str_cmp( arg, "int" ) || !str_cmp( arg, "intelligence" ) )
    		{
                      if( mob->perm_int <= ch->perm_int || ch->perm_int >= 20 + race_table[ch->race].int_plus || ch->perm_int >= 25 )
                      {
                          act( AT_TELL, "$n tells you 'I cannot help you... you are already more educated than I.'",
		             mob, NULL, ch, TO_VICT );
		          return;
                      }
                      send_to_char("&GYou begin your studies.\n\r", ch);
                }
          	if ( !str_cmp( arg, "wis" ) || !str_cmp( arg, "wisdom" ) )
    		{
                      if( mob->perm_wis <= ch->perm_wis || ch->perm_wis >= 20 + race_table[ch->race].wis_plus || ch->perm_wis >= 25 )
                      {
                          act( AT_TELL, "$n tells you 'I cannot help you... you are already far wiser than I.'",
		             mob, NULL, ch, TO_VICT );
		          return;
                      }
                      send_to_char("&GYou begin contemplating several ancient texts in an effort to gain wisdom.\n\r", ch);
                }
          	if ( !str_cmp( arg, "con" ) || !str_cmp( arg, "constitution" ) )
    		{
                      if( mob->perm_con <= ch->perm_con || ch->perm_con >= 20 + race_table[ch->race].con_plus || ch->perm_con >= 25 )
                      {
                          act( AT_TELL, "$n tells you 'I cannot help you... you are already healthier than I.'",
		             mob, NULL, ch, TO_VICT );
		          return;
                      }
                      send_to_char("&GYou begin your endurance training.\n\r", ch);
                }
          	if ( !str_cmp( arg, "cha" ) || !str_cmp( arg, "charisma" ) )
            	{
                      if( mob->perm_cha <= ch->perm_cha || ch->perm_cha >= 20 + race_table[ch->race].cha_plus || ch->perm_cha >= 25 )
                      {
                          act( AT_TELL, "$n tells you 'I cannot help you... you already are more charming than I.'",
		             mob, NULL, ch, TO_VICT );
		          return;
                      }
                      send_to_char("&GYou begin lessons in maners and ettiquite.\n\r", ch);
                }
            	add_timer ( ch , TIMER_DO_FUN , 10 , do_train , 1 );
    	    	ch->dest_buf = str_dup(arg);
    	    	return;

    	case 1:
    		if ( !ch->dest_buf )
    		   return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;

    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;
    	        send_to_char("&RYou fail to complete your training.\n\r", ch);
    		return;
    }

    ch->substate = SUB_NONE;

    if ( number_bits ( 2 ) == 0 )
    {
        successful = TRUE;
    }

    if ( !str_cmp( arg, "str" ) || !str_cmp( arg, "strength" ) )
    {
        if ( !successful )
        {
             send_to_char("&RYou feel that you have wasted alot of energy for nothing...\n\r", ch);
             return;
        }
        send_to_char("&GAfter much of excercise you feel a little stronger.\n\r", ch);
    	ch->perm_str++;
    	return;
    }

    if ( !str_cmp( arg, "dex" ) || !str_cmp( arg, "dexterity" ) )
    {
        if ( !successful )
        {
             send_to_char("&RAfter all that training you still feel like a clutz...\n\r", ch);
             return;
        }
        send_to_char("&GAfter working hard at many challenging tasks you feel a bit more coordinated.\n\r", ch);
    	ch->perm_dex++;
    	return;
    }

    if ( !str_cmp( arg, "int" ) || !str_cmp( arg, "intelligence" ) )
    {
        if ( !successful )
        {
             send_to_char("&RHitting the books leaves you only with sore eyes...\n\r", ch);
             return;
        }
        send_to_char("&GAfter much study you feel alot more knowledgeable.\n\r", ch);
    	ch->perm_int++;
    	return;
    }

    if ( !str_cmp( arg, "wis" ) || !str_cmp( arg, "wisdom" ) )
    {
        if ( !successful )
        {
             send_to_char("&RStudying the ancient texts has left you more confused than wise...\n\r", ch);
             return;
        }
        send_to_char("&GAfter contemplating several seemingly meaningless events you suddenly \n\rreceive a flash of insight into the workings of the universe.\n\r", ch);
    	ch->perm_wis++;
    	return;
    }

    if ( !str_cmp( arg, "con" ) || !str_cmp( arg, "constitution" ) )
    {
        if ( !successful )
        {
             send_to_char("&RYou spend a long aerobics session exercising very hard but finish \n\rfeeling only tired and out of breath....\n\r", ch);
             return;
        }
        send_to_char("&GAfter a long tiring excersise session you feel much healthier than before.\n\r", ch);
    	ch->perm_con++;
    	return;
    }


    if ( !str_cmp( arg, "cha" ) || !str_cmp( arg, "charisma" ) )
    {
        if ( !successful )
        {
             send_to_char("&RYou finish your self improvement session feeling a little depressed.\n\r", ch);
             return;
        }
        send_to_char("&GYou spend some time focusing on how to improve your personality and feel \n\rmuch better about yourself and the ways others see you.\n\r", ch);
    	ch->perm_cha++;
    	return;
    }

}

void do_suicide( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
  	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
    char  logbuf[MAX_STRING_LENGTH];
    BOUNTY_DATA *bounty;
    ROOM_INDEX_DATA *room;  /* Needed for losing home */
    SHIP_DATA *ship;   /* Needed for losing ship */

    if ( IS_NPC(ch) || !ch->pcdata )
    {
        send_to_char( "Yeah right!\n\r", ch );
	    return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "&RIf you really want to delete this character type suicide and your password.\n\r", ch );
	    return;
    }

    if ( strcmp( (char *) crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    send_to_char( "Sorry wrong password.\n\r", ch );
	    sprintf( logbuf , "%s attempting to commit suicide... WRONG PASSWORD!" , ch->name );
	    log_string( logbuf );
	    return;
	}
    act( AT_BLOOD, "With a sad determination and trembling hands you slit your own throat!",  ch, NULL, NULL, TO_CHAR    );
    act( AT_BLOOD, "Cold shivers run down your spine as you watch $n slit $s own throat!",  ch, NULL, NULL, TO_ROOM );
    // Clan Member List gets lowered
    if(ch->pcdata && ch->pcdata->clan)
    {
	ch->pcdata->clan->members--;
	save_clan(ch->pcdata->clan);
    }
   	int x, y;
    // Players who delete lose their home - Gatz
     if (ch->plr_home != NULL)
     {
                 ch->gold += 375000;
      room = ch->plr_home;

      STRFREE( room->name );
      room->name = STRALLOC( "An Empty Apartment" );

      REMOVE_BIT( room->room_flags , ROOM_PLR_HOME );
      SET_BIT( room->room_flags , ROOM_EMPTY_HOME );
        sprintf( buf3, "%s%c/%s.home", PLAYER_DIR, tolower(ch->name[0]),
                                 capitalize( ch->name ) );
	remove(buf3);
      fold_area( room->area, room->area->filename, FALSE );
          ch->plr_home = NULL;
     }

    bounty = get_bounty( ch->name );
    if ( bounty )
      remove_bounty(bounty);

    for ( ship = first_ship; ship; ship = ship->next )
    {
         if ( !str_cmp( ship->owner, ch->name ) )
         {
              STRFREE( ship->owner );
              ship->owner = STRALLOC( "" );
              STRFREE( ship->pilot );
              ship->pilot = STRALLOC( "" );
              STRFREE( ship->copilot );
              ship->copilot = STRALLOC( "" );

              save_ship( ship );
         }

    }
    quitting_char = ch;
    save_char_obj( ch );
    saving_char = NULL;
    extract_char( ch, TRUE, FALSE );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	save_equipment[x][y] = NULL;
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(ch->name[0]), capitalize( ch->name ) );
  	sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(ch->name[0]), capitalize( ch->name ) );
  	if ( !rename( buf, buf2 ) )
  	{
    AREA_DATA *pArea;

    sprintf( buf, "%s%s", GOD_DIR, capitalize(ch->name) );
    sprintf( buf2, "%s.are", capitalize(ch->name) );
    for ( pArea = first_build; pArea; pArea = pArea->next )
      if ( !strcmp( pArea->filename, buf2 ) )
      {
        sprintf( buf, "%s%s", BUILD_DIR, buf2 );
        if ( IS_SET( pArea->status, AREA_LOADED ) )
          fold_area( pArea, buf, FALSE );
        close_area( pArea );
        sprintf( buf2, "%s.bak", buf );
      }
  	}
  	else if ( errno == ENOENT )
  	{
    set_char_color( AT_PLAIN, ch );
    send_to_char( "Player does not exist.\n\r", ch );
  	}
}

void do_bank( CHAR_DATA *ch, char *argument ) /* Expanding bank to unsigned - Funf */ /* er, long - Funf */
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    long long int amount = 0;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool ch_comlink = FALSE;

    argument = one_argument( argument , arg1 );
    argument = one_argument( argument , arg2 );
    argument = one_argument( argument , arg3 );

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

    if ( NOT_AUTHED(ch) )
    {
      send_to_char("&CYou can not access your bank account until after you've graduated from the academy.&W\n\r", ch);
      return;
    }

      if ( IS_IMMORTAL( ch ) )
          ch_comlink = TRUE;
      else
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if (obj->pIndexData->item_type == ITEM_COMLINK)
           ch_comlink = TRUE;
        }

      if ( !ch_comlink )
      {
        if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
        {
          send_to_char( "&CYou must be in a &Rbank &Cor have a &Rcomlink &Cto do that!&W\n\r", ch );
          return;
        }
      }

    if ( arg1[0] == '\0' )
    {
       send_to_char( "&CUsage&R: &WBANK &R<&Wdeposit&R|&Wwithdrawal&R|&Wbalance&R|&Wtransfer&R> &C[&Wamount&C]&W &C[&Wreceiver&C]&W\n\r", ch );
       if( !IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_FREEZE))
		send_to_char("&RYour Bank Freeze prevents Deposits.\r\n",ch);
       return;
    }

    if (arg2[0] != '\0' )
        amount = atoll(arg2);

    if ( !str_prefix( arg1 , "deposit" ) )
    {
       if(!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_FREEZE))
       {
	  send_to_char("&RYour Bank Freeze prevents depositing.\r\n", ch);
	  return;
       }
       if ( amount  <= 0 )
       {
          send_to_char( "&CYou may only deposit amounts greater than zero.&W\n\r", ch );
          do_bank( ch , "" );
          return;
       }

       if ( ch->gold < amount )
       {
          send_to_char( "&CYou don't have that many wulongs on you.&W\n\r", ch );
          return;
       }

       if ( ( (long long int)amount + ch->pcdata->bank ) > MAX_BANK )
       {
          send_to_char( "&CThat would put your account over the limit.&W\n\r", ch );
          return;
       }

       ch->gold -= amount;
       ch->pcdata->bank += amount;

       ch_printf( ch , "&CYou deposit &Y%s &Cwulongs into your account.&W\n\r" ,num_punct(amount) );
       return;
    }
    else if ( !str_prefix( arg1 , "withdrawal" ) )
    {
       if ( amount  <= 0 )
       {
          send_to_char( "&CYou may only withdraw amounts greater than zero.&W\n\r", ch );
          do_bank( ch , "" );
          return;
       }

       if ( ch->pcdata->bank < amount )
       {
          send_to_char( "&CYou don't have that many wulongs in your account.&W\n\r", ch );
          return;
       }


       if ( ( amount + ch->gold ) > MAX_BANK )
       {
          send_to_char( "&CYou can only carry 20 billion wulongs.&W\n\r", ch );
          return;
       }

       ch->gold += amount;
       ch->pcdata->bank -= amount;

       ch_printf( ch , "&CYou withdraw &Y%s &Cwulongs from your account.&W\n\r" ,num_punct(amount) );
       return;

    }
    else if ( !str_prefix( arg1 , "balance" ) )
    {
        ch_printf( ch , "&CYou have &Y%s&C wulongs in your account.&W\n\r" , num_punct(ch->pcdata->bank) );
        return;
    }
    else if ( !str_prefix( arg1 , "transfer" ) )
    {
      if( ( ( victim = get_char_world(ch, arg3) ) == NULL ))
      {
        send_to_char("&RNo such player online.&W\n\r", ch);
        return;
      }

      if( ( IS_NPC(victim)))
      {
        send_to_char("&RNo such player online.&W\n\r", ch);
        return;
      }
      if(IS_SET(victim->pcdata->flags, PCFLAG_FREEZE))
      {
	send_to_char("&RThat player has a bank freeze on them!\r\n", ch);
	return;
      }

       if ( amount  <= 0 )
       {
          send_to_char( "&CYou may only transfer amounts greater than zero.&W\n\r", ch );
          do_bank( ch , "" );
          return;
       }

       if ( ch->pcdata->bank < amount )
       {
          send_to_char( "&CYou don't have that many wulongs in your account.&W\n\r", ch );
          return;
       }

       if ( ( amount + victim->pcdata->bank ) > MAX_BANK )
       {
          send_to_char( "&CThat would put their account over the limit.&W\n\r", ch );
          return;
       }

       ch->pcdata->bank -= amount;
       victim->pcdata->bank += amount;

       ch_printf( ch , "&CYou transfer &Y%s &Cwulongs to %s's account.&W\n\r" ,num_punct(amount), victim->name );
       ch_printf( victim , "&R%s &Ctransfers &Y%s &Cwulongs to your account.&W\n\r" , ch->name , num_punct(amount));
       return;

    }
    else
    {
      do_bank( ch , "" );
      return;
    }


}


void do_frenzy(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	int sn, level;
	short drug;
	bool check = FALSE;

	if( IS_NPC(ch) || IS_AFFECTED( ch, AFF_FRENZY))
	{
		send_to_char("&RYou can't get in any more of a frenzy!\n\r", ch);
		return;
	}
	if( ch->adren < 4)
	{
		send_to_char("Your adrenaline isn't pumping enough!\n\r", ch);
		return;
	}
	check = FALSE;
        for ( drug=0 ; drug <= 9 ; drug ++ )
        {
		if( ch->pcdata->drug_level[drug] > 0 || ch->pcdata->addiction[drug] > 0)
		{
			check = TRUE;
		}
        }
	if(check)
	{
		send_to_char("Your body is too messed up from drugs to enter a frenzy!\n\r" ,ch);
		return;
	}

	act( AT_FIRE,"You enter into a frenzy!\n\r", ch, NULL, NULL, TO_CHAR);
	act( AT_FIRE,"$n enters into a frenzy!\n\r", ch, NULL, NULL, TO_ROOM);

	level = IS_NPC(ch) ? ch->top_level :
		(int) (ch->pcdata->learned[gsn_frenzy]);

        sn =  skill_lookup("frenzy");

        af.type = sn;
        af.location = APPLY_STR;
        af.modifier = level/25;
        af.duration = 50;
        af.bitvector = AFF_FRENZY;
        affect_to_char( ch, &af );

        af.type = sn;
        af.location = APPLY_DEX;
        af.modifier = (level/25)*-1;
        af.duration = 50;
        af.bitvector = AFF_FRENZY;
        affect_to_char( ch, &af);

	if(level/50 > 0)
	{
               	af.type = sn;
               	af.location = APPLY_DAMROLL;
               	af.modifier = level/50;
               	af.duration = 50;
               	af.bitvector = AFF_FRENZY;
		affect_to_char( ch, &af);
	}

	learn_from_success( ch, gsn_frenzy );

	return;
}

void do_applylicense(CHAR_DATA *ch, char *argument)
{

	if(IS_NPC(ch))
		return;

        if ( !IS_SET( ch->in_room->room_flags , ROOM_R_RECRUIT )
         && !IS_SET( ch->in_room->room_flags , ROOM_E_RECRUIT ) )
	{
		send_to_char("You need to be in a RBH or GLM enlistment office to do this!\r\n", ch);
		return;
	}

	if(ch->pcdata->weaponl == 0)
	{
		send_to_char("You have no need to apply for a license!\r\n", ch);
		return;
	}

	if(ch->pcdata->weaponl == 1)
	{
		if((ch->gold - (ch->top_level * 2000)) < 0)
		{
			send_to_char("&RYou don't have enough Wulongs!\r\n", ch);
			return;
		}
		else
		{
			ch->gold -= (ch->top_level * 2000);
			send_to_char("&RYou have regained your Weapon License!\r\n", ch);
			ch->pcdata->weaponl = 0;
			return;
		}
	}

	send_to_char("&RYou can't apply for a License yet!\r\n", ch);

	return;
}


void do_shadowbox( CHAR_DATA *ch, char *argument)
{
	int timer = 0;


	switch(ch->substate)
	{
		default:
			if(IS_NPC(ch))
				return;
			if(!IS_SET(ch->in_room->room_flags2, ROOM_GYM))
			{
				send_to_char("&RYou need to be in a gym to do that!\r\n", ch);
				return;
			}
			if(argument[0] == '\0')
			{
				send_to_char("Syntax: Shadowbox  # (Time in Seconds)\r\n", ch);
				return;
			}

			timer = atoi(argument);

			if(timer < 3)
			{
				send_to_char("You need to Shadow Box for at least 3 second!\r\n", ch);
				return;
			}

			if(ch->hit < 300 || ch->move < 300)
			{
				send_to_char("Your body shudders slightly and refuses to start!\r\n", ch);
				return;
			}


			add_timer( ch, TIMER_DO_FUN, 3, do_shadowbox, 1);
			ch->stimer = timer;
			act(AT_GREEN, "You get ready to Shadow Box!", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n gets ready to Shadow Box!", ch, NULL, NULL, TO_ROOM);
		return;
		case 1:
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You grab the punching bag and pant slightly, finishing your routine.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n grabs the punching bag and pants slightly, finishing $s routine.", ch, NULL, NULL, TO_ROOM);
				return;
			}
                        if(!IS_SET(ch->in_room->room_flags2, ROOM_GYM))
                        {
                                send_to_char("&RYou need to be in a gym to do that!\r\n", ch);
                                return;
                        }

			// Lets decide what they do!
			switch(number_range(0,9))
			{
				case 9:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You do a right shin kick into the punching bag.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a right shin kick into the punching bag.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You do a left shin kick into the punching bag.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a left shin kick into the punching bag.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 8:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You do a spinning backhand into the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a spinning backhand into the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You do a left elbow into the punching bag.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a left elbow into the punching bag.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 7:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You do a dropkick into the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a dropkick into the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You do a low kick to the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a low kick to the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 6:
					if(ch->move < 500)
					{
						act(AT_YELLOW, "You lose focus and the punching bag hits you!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n loses focus and the punching bag hits $m!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You lose your footing and slip!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n loses $s footing and slips!", ch, NULL, NULL, TO_ROOM);
					}
					ch->contrain++;
				break;
				case 5:
					if(get_curr_str(ch) > 16)
					{
						act(AT_YELLOW, "You do a hard right elbow into the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a hard right elbow into the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You give a left uppercut into the punching bag.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a left uppercut into the punching bag.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 4:
					if(ch->alignment < 300)
					{
						act(AT_YELLOW, "You grit your teeth and punch the punching bag hard!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n grits $s teeth and punches the punching bag hard!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You gracefully do a right hook into the punching bag.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n gracefully does a right hook into the punching bag.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 3:
					switch(number_range(0,2))
					{
						case 2:
							act(AT_YELLOW, "You deliver a hard right knee into the punching bag!", ch, NULL, NULL, TO_CHAR);
							act(AT_YELLOW, "$n delivers a hard right knee into the punching bag!", ch, NULL, NULL, TO_ROOM);
						break;
						case 1:
							act(AT_YELLOW, "You deliver a hard left knee into the punching bag!", ch, NULL, NULL, TO_CHAR);
							act(AT_YELLOW, "$n delivers a hard left knee into the punching bag!", ch, NULL, NULL, TO_ROOM);
						break;
						default:
							if(get_curr_dex(ch) > 13)
							{
								act(AT_YELLOW, "You do a jumping Hi Kick to the punching bag!", ch, NULL, NULL, TO_CHAR);
								act(AT_YELLOW, "$n does a jumping Hi Kick to the punching bag!", ch, NULL, NULL, TO_ROOM);
							}
							else
							{
								act(AT_YELLOW, "You do a front kick to the punching bag.", ch, NULL, NULL, TO_CHAR);
								act(AT_YELLOW, "$n does a jumping front kick to the punching bag.", ch, NULL, NULL, TO_ROOM);
							}
						break;
					}
				break;
				case 2:
					if(get_curr_con(ch) > 14)
					{
						act(AT_YELLOW, "You do a double palm thrust right into the punching bag, sending it flying back!",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a double palm thrust into the punching bag, sending it flying back!",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You do a palm thrust right into the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a palm thrust right into the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 1:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You do two punches and kick combo to the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does two punches and a kick combo to the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You do a flurry of punches into the punching bag!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a flurry of punches into the punching bag!", ch, NULL, NULL, TO_ROOM);
					}
				break;
				default:
					if(ch->alignment < 300)
					{
						act(AT_YELLOW, "You scream and do a kick into the punching bag with all your might!",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n screams and does a kick into the punching bag with all $s might!",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You take a deep breath and do a charging roundhouse!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n takes a deep breath and does a charing roundhouse!", ch, NULL, NULL, TO_ROOM);
					}
				break;
			}
			ch->strtrain += number_range(1, get_curr_lck(ch)/5);
			if(IS_SET(ch->in_room->room_flags2, ROOM_GYM))
				ch->strtrain += number_range(1, 5);
			ch->move -= number_range(0, 50);
			if(ch->move < 300)
			{
				act(AT_GREEN, "Your body begins to feel too heavy to move and you can not finish the workout!",
					ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n's body shakes slightly and $e looks exhausted.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			ch->stimer -= 3;

                break;

                case SUB_TIMER_DO_ABORT:
                        ch->substate = SUB_NONE;
                        act(AT_GREEN, "You stay still for a moment, panting heavily.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n stays still for a moment, panting heavily.", ch, NULL, NULL, TO_ROOM);
                return;
        }

	ch->substate = SUB_TIMER_CANT_ABORT;
	add_timer(ch, TIMER_DO_FUN, 3, do_shadowbox, 1);
}





void do_newstudy( CHAR_DATA *ch, char *argument)
{
	switch(ch->substate)
	{
		default:
			if(argument[0] == '\0')
			{
				send_to_char("Syntax: Study X (Amount in Seconds)\r\n", ch);
				return;
			}

			if(!IS_SET(ch->in_room->room_flags2, ROOM_LIBRARY))
			{
				send_to_char("You need to be in a library for this!\r\n", ch);
				return;
			}

			int time = 0;
			time = atoi(argument);
			if(time < 3)
			{
				send_to_char("You need at least a time of 3 seconds!\r\n", ch);
				return;
			}
			ch->stimer = time;
			switch(number_range(0,2))
			{
				case 2:
					act(AT_GREEN, "You check the shelf and grab an Encyclopedia.", ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n checks the shelf and grabs an Encyclopedia.", ch, NULL, NULL, TO_ROOM);
					time = 3;
				break;
				case 1:
					act(AT_GREEN, "You check the shelf and grab a book of the greatest Poetry.", ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n checks the shelf and grabs a book of the greatest Poetry.", ch, NULL, NULL, TO_ROOM);
					time = 2;
				break;
				default:
					act(AT_GREEN, "You check the shelf and grab a book on Physics.", ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n checks the shelf and grabs a book on Physics.", ch, NULL, NULL, TO_ROOM);
					time = 1;
				break;
			}

			add_timer( ch, TIMER_DO_FUN, 3, do_newstudy, time);
		return;
		case 1:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You put the book back on the shelf and stretch out a little.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n puts the book back on the shelf and stretches out a little.", ch, NULL, NULL, TO_ROOM);
				return;
			}
                        if(!IS_SET(ch->in_room->room_flags2, ROOM_LIBRARY))
                        {
                                send_to_char("You need to be in a library for this!\r\n", ch);
                                return;
                        }
			// Time to see the message!
			switch(number_range(0,8))
			{
				case 8:
					act(AT_YELLOW, "You struggle to grasp the concept of Jump Gates.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks very frustrated and confused over something in $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 7:
					if(ch->alignment > 300)
					{
						act(AT_YELLOW, "You try to work out a formula's proof in the book.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n starts to do some sort of math work.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You scream at a ridiculous proof, which is far too complex.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n screams at something in $s book.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 6:
					if(get_curr_int(ch) < 14)
					{
						act(AT_YELLOW, "You grimace slightly as you notice there are 2,000 pages left to read.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n grimaces over something in $s book.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You beam a bright smile as you notice there are 2,000 pages left to read.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n beams a bright smile over something in $s book.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 5:
					if(get_curr_int(ch) > 16)
					{
						act(AT_YELLOW, "You chuckle softly as you pick up a Physics joke in the book.", ch, NULL, NULL,TO_CHAR);
						act(AT_YELLOW, "$n chuckles softly over $s book.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You frown at the lame joke in the Physics book.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n frowns about something in $s book.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 4:
					if(ch->alignment < 300)
					{
						act(AT_YELLOW, "You scream obscenities at the confusing book.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n begins screaming obscenities at the confusing book.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You break down in tears out of frustration and confusion.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n breaks down into tears from the confusing book they are reading.", ch, NULL, NULL,
							TO_ROOM);
					}
				break;
				case 3:
					act(AT_YELLOW, "You rub your brow as you try to understand how vectors work.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n rubs $s brow as $e tries to understand how vectors work.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You struggle to work out all of Newton's Laws of Physics.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n struggles as $e tries to figure out all of Newton's Laws of Physics.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					act(AT_YELLOW, "You scream in frustration as your formula for Joules isn't correct.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n screams in frustration as $s formula for Joules is incorrect.", ch, NULL, NULL, TO_ROOM);
				break;
			}
			ch->stimer -= 3;
			ch->inttrain += number_range(1, get_curr_lck(ch)) + 1;
			ch->wistrain += number_range(1, get_curr_lck(ch)) + 1;
			ch->move -= number_range(1,30);
			if(ch->move < 300)
			{
				act(AT_GREEN, "Your head hits the book, you are too exhausted to study anymore.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n's head hits the book, exhausted.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newstudy, 1);
		return;
		case 2:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You put the book back on the shelf and stretch out a little.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n puts the book back on the shelf and stretches out a little.", ch, NULL, NULL, TO_ROOM);
				return;
			}
                        if(!IS_SET(ch->in_room->room_flags2, ROOM_LIBRARY))
                        {
                                send_to_char("You need to be in a library for this!\r\n", ch);
                                return;
                        }
			// Time to see the message!
			switch(number_range(0,7))
			{
				case 7:
					act(AT_YELLOW, "You roll your eyes at a corny kid's poem.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n rolls $s eyes at something in $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 6:
					if(ch->alignment >= 300)
					{
						act(AT_YELLOW, "You get teary eyed over a poem about the Jump Gate Accident on Earth.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n gets teary eyed over something $s book.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You chuckle slightly over the Jump Gate Accident on Earth.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n chuckles quietly over something in $s book.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 5:
					act(AT_YELLOW, "You try to understand a poem from a Mars Poet.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n seems to be trying to understand a something in $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					act(AT_YELLOW, "You consider the deeper meaning of a poem.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks like they are in deep thought.", ch, NULL, NULL, TO_ROOM);
				break;
				case 3:
					act(AT_YELLOW, "You laugh at a funny poem.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n laughs at $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					if(ch->alignment > 300)
					{
						act(AT_YELLOW, "You begin to break down and cry at a sad poem.",ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n begins to break down and cry at $s.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You sniff slightly, holding back tears because of a sad poem.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n sniifs slightly and $s eyes look watery and red.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 1:
					act(AT_YELLOW, "You feel sleepy after reading a very dull poem.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n yawns slightly after reading something in $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					act(AT_YELLOW, "You cringe after reading a disturbing poem.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n cringes after reading something.", ch, NULL, NULL, TO_ROOM);
				break;
			}

			ch->stimer -= 3;
			ch->inttrain += number_range(1, get_curr_lck(ch)) + 1;
			ch->wistrain += number_range(1, get_curr_lck(ch)) + 1;
			ch->move -= number_range(1,30);
			if(ch->move < 300)
			{
				act(AT_GREEN, "Your head hits the book, you are too exhausted to study anymore.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n's head hits the book, exhausted.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newstudy, 2);
		return;
		case 3:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You put the book back on the shelf and stretch out a little.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n puts the book back on the shelf and stretches out a little.", ch, NULL, NULL, TO_ROOM);
				return;
			}
                        if(!IS_SET(ch->in_room->room_flags2, ROOM_LIBRARY))
                        {
                                send_to_char("You need to be in a library for this!\r\n", ch);
                                return;
                        }
			// Time to see the message!
			switch(number_range(0,7))
			{
				case 7:
					act(AT_YELLOW, "You read a chapter about the re-institution of Bounty Hunters.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n reads something very intently.", ch, NULL, NULL, TO_ROOM);
				break;
				case 6:
					if(!IS_NPC(ch) && ch->pcdata->pkills > 5)
					{
						act(AT_YELLOW, "You read through the pages on the evolution of firearms intently.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n reads through $s book intently.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You gloss over the pages on the evolution of firearms.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n seems unconcerned about what $e is reading.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 5:
					act(AT_YELLOW, "You try to understand why they put a chapter on pornography in an Encyclopedia.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n rolls $s eyes over a passage in $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					act(AT_YELLOW, "You read an interesting passage on the reproductive habits of the mongoose.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n raises their eyebrow slightly, reading their book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 3:
					act(AT_YELLOW, "You study the process of Photosynthesis.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks to be completely focused on $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You begin reading the history on Ancient Rome.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n is starring intently on $s book.", ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					act(AT_YELLOW, "You begin reading up on the cultures of Old Venus.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks like $e is studing hard.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					if(ch->sex == 1)
						act(AT_YELLOW, "You notice a chapter on female menstrual cycle, and skip it.", ch, NULL, NULL, TO_CHAR);
					else
						act(AT_YELLOW, "You notice a chapter on male body hair, and skip it.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks slightly disgusted and rapidly turns the pages of $m book.", ch, NULL, NULL, TO_ROOM);
				break;
			}
			ch->stimer -= 3;
			ch->inttrain += number_range(1, get_curr_lck(ch)) + 1;
			ch->wistrain += number_range(1, get_curr_lck(ch)) + 1;
			ch->move -= number_range(1,30);
			if(ch->move < 300)
			{
				act(AT_GREEN, "Your head hits the book, you are too exhausted to study anymore.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n's head hits the book, exhausted.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			ch->move -= number_range(1,30);
			if(ch->move < 300)
			{
				act(AT_GREEN, "Your head hits the book, you are too exhausted to study anymore.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n's head hits the book, exhausted.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newstudy, 3);
		return;
		case SUB_TIMER_DO_ABORT:
			ch->substate = SUB_NONE;
			act(AT_GREEN, "You yawn slightly and close the book.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n yawns slightly and closes the book $e was reading.", ch, NULL, NULL, TO_ROOM);
		return;
	}
}


void do_newendurance( CHAR_DATA *ch, char *argument)
{
	switch(ch->substate)
	{
		default:
			if(IS_NPC(ch))
				return;

			if(!IS_SET(ch->in_room->room_flags2, ROOM_GYM))
			{
				send_to_char("&RYou must be in a gym for this.\r\n", ch);
				return;
			}

			if(argument[0] == '\0')
			{
				send_to_char("&RSyntax: Endurance X (Time in Seconds)\r\n" ,ch );
				return;
			}

			if(ch->hit < 150)
			{
				send_to_char("&RYou are far to injured to train your Endurance!\r\n", ch);
				return;
			}

			int time = atoi(argument);

			if(time < 3)
			{
				send_to_char("&RYou need to do at least 3 seconds to make it worth it!\r\n", ch);
				return;
			}

			ch->stimer = time;

			switch(number_range(0,3))
			{
				case 3:
					act(AT_GREEN, "You take a deep breath, and begin holding up two large buckets of water.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n prepares $mself with a deep breath, as $e holds up two large buckets of water.",
						ch, NULL, NULL, TO_ROOM);
					time = 4;
				break;
				case 2:
					act(AT_GREEN, "You stretch slightly and start to do some fast pace running on a treadmill.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n stretches and steps onto a treadmill, starting to do some running.",
						ch, NULL, NULL, TO_ROOM);
					time = 3;
				break;
				case 1:
					act(AT_GREEN, "You ready a board and prepare to practice high density board breaking.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n readies and board and prepares for high density board breaking.",
						ch, NULL, NULL, TO_ROOM);
					time = 2;
				break;
				default:
					act(AT_GREEN, "You do a handstand and begin doing pushups whilst in a handstand.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_GREEN, "$n begins to do handstand pushups.", ch, NULL, NULL, TO_ROOM);
					time = 1;
				break;
			}

			add_timer(ch, TIMER_DO_FUN, 3, do_newendurance, time);
		return;
		case 1:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You roll back onto your feet and take a few deep breaths, finishing your training.",
					ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n rolls back onto $s feet and takes a few deep breaths.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			// Messages go here!
			switch(number_range(0,8))
			{
				case 8:
					if(ch->hit > 500)
					{
						act(AT_YELLOW, "You grin slightly as you complete two handstand pushups rapidly.", ch, NULL, NULL,
TO_CHAR);
						act(AT_YELLOW, "$n grins slightly as $e completes two handstand pushups rapidly.", ch, NULL, NULL,
TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "A tear of pain rolls down your cheek and mixes with your sweat.", ch, NULL, NULL,
TO_CHAR);
						act(AT_YELLOW, "A tear rolls down $n's cheek and mixes with $s sweat.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 7:
					act(AT_YELLOW, "You bite your lip softly as you try to complete another handstand pushup.", ch, NULL, NULL,
TO_CHAR);
					act(AT_YELLOW, "$n bites $s lip softly as $e tries to complete another handstand pushup.", ch, NULL, NULL,
TO_ROOM);
				break;
				case 6:
					act(AT_YELLOW, "Your body shakes as you complete another handstand pushup.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n's body shakes as $e completes another handstand pushup.", ch, NULL, NULL, TO_ROOM);
				break;
				case 5:
					act(AT_YELLOW, "You waver slightly, almost losing your balance.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n wavers slightly, almost losing his handstand balance.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					if(get_curr_dex(ch) > 16)
					{
						act(AT_YELLOW, "You grin and quickly do a pushup with only one arm!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n grins and does a quick one armed pushup!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You grit your teeth and do another pushup!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n grits $s teeth and does another pushup!", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 3:
					if(ch->alignment < 300)
					{
						act(AT_YELLOW, "You swear under your breath, struggling to do another pushup.", ch, NULL, NULL,
TO_CHAR);
						act(AT_YELLOW, "$n swears under $s breath, struggling to do another pushup.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You squint your eyes hard, struggling to do another pushup.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n squints $s eyes hard, struggling to do another pushup.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 2:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You do a pushup so low, your forehead touches the ground!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a pushup so low, $s forehead touches the ground!", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "Your muscules buldge slightly as they bare your weight.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n muscules buldge as they bare $s weight.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 1:
					if(get_curr_con(ch) > 16)
					{
						act(AT_YELLOW, "You grin slightly, letting everyone know how tough you are.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n grins slightly letting everyone know how tough $e is.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "Your face grimaces in pain, as you struggle to keep going.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n's face grimaces in pain as $e struggles to keep going.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				default:
					act(AT_YELLOW, "A drop of sweat runs in your eye, and you try to blink it out.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "A drop of sweat runs into $n's eye, and $e tries to blink it out.", ch, NULL, NULL, TO_ROOM);
				break;
			}
			ch->contrain += number_range(1, get_curr_lck(ch)/5);
			ch->contrain += number_range(0,2);
			ch->hit -= number_range(5, 30);
			if(ch->hit < 150)
			{
				act(AT_GREEN, "You collapse to the ground, to exhausted to continue.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n collapses to the ground exhausted.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			ch->stimer -= 3;
			add_timer(ch, TIMER_DO_FUN, 3, do_newendurance, 1);
		return;

		case 2:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You throw the board onto the ground and end your routine.",
					ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n throws the board that $e was training with on the ground.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			// Messages go here!
			switch(number_range(0,5))
			{
				case 5:
					act(AT_YELLOW, "You try to headbutt the board but stumble dizzily back.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n tries to headbutt the board but stumbles dizzily back.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You do a spinning right elbow into the board!", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a spinning right elbow into the board.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You do a spinning left elbow into the board.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a spinning left elbow into the board.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 3:
					if(get_curr_str(ch) > 16)
					{
						act(AT_YELLOW, "You line your fingers up to the board and bust a hole into it with the four inch punch!",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n lines up $s fingers to the board and breaks a hole into it with the four inch punch!",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You line your fingers up and attempt a four inch punch, but bust open your knuckles!",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n lines up $s fingers and attempts a four inch punch, but busts open $s knuckles!",
							ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 2:
					if(ch->alignment < 300)
					{
						act(AT_YELLOW, "You release an ear spliting scream and deliver a hard palm thrust into the board!",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n releases an ear spliting scream and delivers a hard palm thrust into the board!",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You leap into the air and do a front chop down on the board!", ch, NULL, NULL,
TO_CHAR);
						act(AT_YELLOW, "$n leaps into the air and does a front chop down on the board!", ch, NULL, NULL,
TO_ROOM);
					}
				break;
				case 1:
					switch(number_range(0,3))
					{
						case 3:
							act(AT_YELLOW, "You do a driving knee thrust into the board!", ch, NULL, NULL, TO_CHAR);
							act(AT_YELLOW, "$n does a driving knee thrust into the board!", ch, NULL, NULL, TO_ROOM);
						break;
						case 2:
							act(AT_YELLOW, "You do a jumping shin kick into the board!", ch, NULL, NULL, TO_CHAR);
							act(AT_YELLOW, "$n does a jumping shin kick into the board!", ch, NULL, NULL, TO_ROOM);
						break;
						case 1:
							act(AT_YELLOW, "You attempt a heel kick, but cut open your foot!", ch, NULL, NULL, TO_CHAR);
							act(AT_YELLOW, "$n attempts a heel kick but cuts open $s foot!", ch, NULL, NULL, TO_ROOM);
						break;
						default:
							act(AT_YELLOW, "You swing a heavy uppercut into the board, breaking off a piece of it!",
								ch, NULL, NULL, TO_CHAR);
							act(AT_YELLOW, "$n swings a heavy uppercut and breaks a piece of the board off!",
								ch, NULL, NULL, TO_ROOM);
						break;
					}
				break;
				default:
					if(get_curr_str(ch) > 16)
					{
						act(AT_YELLOW, "You do a double palm thrust, shattering the board!",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n does a double pam thrust, shattering the board!", ch, NULL, NULL, TO_ROOM);
						act(AT_YELLOW, "You quickly setup an new board and prepare to continue your routine.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n quickly sets up a new board and gets ready to continue $s routine.",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
				    		act(AT_YELLOW, "You try for a double palm thrust, but scream as you feel something pop.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n attempts a double palm thrust, but screams in pain as something goes wrong.",
							ch, NULL, NULL, TO_ROOM);
					}
				break;
			}
			ch->contrain += number_range(1, get_curr_lck(ch)/5);
			ch->contrain += number_range(0,2);
			ch->hit -= number_range(5, 30);
			if(ch->hit < 150)
			{
				act(AT_GREEN, "You fall over onto the board for leverage, as your hand drips blood.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n falls over onto the board for leverage, as $s hand drips blood.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			ch->stimer -= 3;
			add_timer(ch, TIMER_DO_FUN, 3, do_newendurance, 2);
		return;

		case 3:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You stop the treadmill and step off, resting for a momemnt.",
					ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n stops the treadmill and steps off, resting for a moment.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			// Messages go here!
			switch(number_range(0,5))
			{
				case 5:
					act(AT_YELLOW, "Your mind drifts off for a second and you almost trip!", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n almost trips on the treadmill!", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					if(ch->hit > 500)
					{
						act(AT_YELLOW, "You begin taking larger strides in your run.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n begins taking larger strides in $s run.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You stumble slightly and your stride gets shorter.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n stumbles slightly and $s stride gets shorter.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 3:
					act(AT_YELLOW, "You take a deep breath and try to lift your legs higher when you run.", ch, NULL, NULL,
TO_CHAR);
					act(AT_YELLOW, "$n takes a deep breath and lifts $s legs higher when $e runs.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You grimace slightly as you try to ignore a cramp in your side.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n grimaces slightly as they run on the treadmill.", ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					if(ch->alignment < 0)
					{
						act(AT_YELLOW, "You start mummbling swears under your breath because of the hard routine.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n starts to mummbling swears under $s breath becase of $s hard routine.",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You try to take your mind off the pain by thinking of something else.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n looks disconnected as $e runs on the treadmill.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				default:
					act(AT_YELLOW, "You wince in pain as you feel a blister forming on your foot.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n winces in pain as $e runs on the treadmill.", ch, NULL, NULL, TO_ROOM);
				break;
			}
			ch->contrain += number_range(1, get_curr_lck(ch)/5);
			ch->contrain += number_range(0,2);
			ch->hit -= number_range(5, 30);
			if(ch->hit < 150)
			{
				act(AT_GREEN, "You fall off the treadmill, with your legs too tired to move.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n falls hard of the treadmill.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			ch->stimer -= 3;
			add_timer(ch, TIMER_DO_FUN, 3, do_newendurance, 3);
		return;
		case 4:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You drop the buckets of water, letting your arms rest.",
					ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n drops the buckets of water he was holding, letting $s arms just dangle", ch, NULL, NULL, TO_ROOM);
				return;
			}
			// Messages go here!
			switch(number_range(0,6))
			{
				case 6:
					act(AT_YELLOW, "Some of the water in your buckets spill out, as you shake with tension.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n spills some of the water in the buckets $e was holding.",
						ch, NULL, NULL, TO_ROOM);
				break;
				case 5:
					act(AT_YELLOW, "You close your eyes, trying to ignore the pain and hold up the buckets.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n closes $s eyes, looking to be in extra pain.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					if(ch->alignment < 300)
					{
						act(AT_YELLOW, "You start yelling curses as your arms burn in pain.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n starts to yell curses because of the pain of holding buckets of water.",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You begin to cry as your arms burn in pain.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n cries in pain because $e is holding up heavy buckets of water.",
							ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 3:
					act(AT_YELLOW, "Your arms shake slightly as you try to hold the buckets still.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n's arms shake slightly as $e tries to hold up buckets of water.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "A small trickle of sweat rolls down your cheek, as you hold the buckets up.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "A small trickle of sweat rolls down $n's cheek because $e is holding up buckets of water.",
						ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					act(AT_YELLOW, "You grunt slightly as your arms start to burn in exhaustion.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n grunts slightly, as the buckets of water they hold up begin to drop a little.",
						ch, NULL, NULL, TO_ROOM);
				break;
				default:
					if(get_curr_str(ch) > 15)
					{
						act(AT_YELLOW, "You wince slightly and then raise your arms up, holding the buckets higher.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n winces slightly, then raises $s arms up, which are holding buckets of water.",
							ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "Your arms drop slightly, and you struggle to not drop the buckets of water.",
							ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n's arms drop and $e struggles to not drop the buckets of water $e is holding.",
							ch, NULL, NULL, TO_ROOM);
					}
				break;
			}
			ch->contrain += number_range(1, get_curr_lck(ch)/5);
			ch->contrain += number_range(0,2);
			ch->hit -= number_range(5, 30);
			if(ch->hit < 150)
			{
				act(AT_GREEN, "You fall to the ground in exhaustion, spilling the water in the buckets.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n falls to the ground, spelling the water he was holding in buckets.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			ch->stimer -= 3;
			add_timer(ch, TIMER_DO_FUN, 3, do_newendurance, 4);
		return;
                case SUB_TIMER_DO_ABORT:
                        ch->substate = SUB_NONE;
                        send_to_char("&RYou are distracted and lose focus!\n\r", ch);
                return;

        }
}


void do_newstretch(CHAR_DATA *ch, char *argument)
{
	switch(ch->substate)
	{
		default:
			if(IS_NPC(ch))
				return;

			if(ch->position != POS_SITTING)
			{
				send_to_char("&RYou must be sitting for this!\r\n", ch);
				return;
			}

			if(ch->move < 300)
			{
				send_to_char("&RYour body is far to sore to stretch!\r\n", ch);
				return;
			}

			ch->stimer = atoi(argument);

			if(ch->stimer < 3)
			{
				send_to_char("&RYou need to do at least 3 seconds of stretching!\r\n", ch);
				return;
			}

			act(AT_GREEN, "You begin to do some intense stretching.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n begins to do some intense stretching.", ch, NULL, NULL, TO_ROOM);
			add_timer(ch, TIMER_DO_FUN, 3, do_newstretch, 1);
		return;
		case 1:
			ch->substate = SUB_NONE;
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You relax slightly, recovering from the stretching.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n relaxes slightly, recovering from stretching.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			// Lets display a message to the user!
			switch(number_range(0,11))
			{
				case 11:
					if(number_range(0,1) == 1)
					{
						act(AT_YELLOW, "You raise your right leg in the air and try to lean back on it.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n raises $s right leg in the air and tries to lean back on it.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You raise your left leg in the air and try to lean back on it.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n raises $s left leg in the air and tries to lean back on it.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 10:
					act(AT_YELLOW, "You slowly tip your head side to side, stretching your neck.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n slowly tips $s head side to side, stretching $s neck.", ch, NULL, NULL, TO_ROOM);
				break;
				case 9:
					act(AT_YELLOW, "You try to extend your arms as far apart as possible.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n tries to extend $s arms as far apart as possible.", ch, NULL, NULL, TO_ROOM);
				break;
				case 8:
					act(AT_YELLOW, "You lean over with one hand and try to extend your fingers beyond your foot.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n leans over with one hand and tries to extend $s fingers beyond $s foot.",
						ch, NULL, NULL, TO_ROOM);
				break;
				case 7:
					act(AT_YELLOW, "You slowly lean backwards and stretch your lower back.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n slowly leans backwards and stretches $s lower back.", ch, NULL, NULL, TO_ROOM);
				break;
				case 6:
					act(AT_YELLOW, "You slowly turn slightly and begin to stretch your hips.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n slowly turns and begins to stretch $s hips.", ch, NULL, NULL, TO_ROOM);
				break;
				case 5:
					act(AT_YELLOW, "You reach upwards doing a vertical flower stretch.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n reaches upwards and does a vertical flower stretch.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					if(ch->hit < 500)
					{
						act(AT_YELLOW, "You try to do a lifting bridge stretch, but collapse slightly.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n tries to do a lifting bridge stretch, but collapses slightly.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You arch your back up and do a lifting bridge stretch.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n arches $s back and does a lifting bridge stretch.", ch, NULL, NULL, TO_ROOM);
					}
				break;
				case 3:
					act(AT_YELLOW, "You slowly lean forward and try to touch your toes.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n slowly leans forward and tries to touch $s toes.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You get into the half lotus position and stretch towards your foot.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n gets into the half lotus position and stretches towards $s foot.", ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					act(AT_YELLOW, "You get into a full lotus position and stretch out your groin.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n gets into a full lotus posiiton and stretches.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					if(get_curr_dex(ch) > 16)
					{
						act(AT_YELLOW, "You get into a full cobra stretch.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n gets into a full cobra stretch.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						act(AT_YELLOW, "You get into a dog stretch.", ch, NULL, NULL, TO_CHAR);
						act(AT_YELLOW, "$n gets into a dog stretch.", ch, NULL, NULL, TO_ROOM);
					}
				break;
			}
			ch->stimer -= 3;
			ch->dextrain += number_range(0, get_curr_lck(ch)/5);
			if(IS_SET(ch->in_room->room_flags2, ROOM_YOGA))
				ch->dextrain += number_range(0,2);
			ch->move -= (number_range(26, 50) - ((get_curr_dex(ch) + get_curr_con(ch))/ 2));
			if(ch->move < 300)
			{
				act(AT_GREEN, "Your body feels sore and you are forced to stop stretching.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n's body shudders slightly and then $e stops stretching.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newstretch, 1);
		return;
		case SUB_TIMER_DO_ABORT:
			ch->substate = SUB_NONE;
			send_to_char("You slowly release your stretch!\r\n", ch);
		return;
	}
}

void do_newresearch(CHAR_DATA *ch, char *argument)
{
	int check = 0;

	switch(ch->substate)
	{
		default:
			if(IS_NPC(ch))
				return;

			if(ch->move < 300)
			{
				send_to_char("&RYou are far too tired to research!\r\n", ch);
				return;
			}

			ch->stimer = atoi(argument);

			if(ch->stimer < 3)
			{
				send_to_char("&RYou need at least 3 seconds to do any sort of research!\r\n", ch);
				return;
			}

			if(IS_SET(ch->in_room->room_flags, ROOM_SPACECRAFT))
				check = 1;
			else if(IS_SET(ch->in_room->room_flags, ROOM_INDOORS))
				check = 2;
			else
				check = 3;

			act(AT_GREEN, "You begin your research.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n begins $s research.", ch, NULL, NULL, TO_ROOM);
			add_timer(ch, TIMER_DO_FUN, 3, do_newresearch, check);
		return;
		// Ship
		case 1:
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You complete your research.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n completes $s research.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			// Message to Players
			switch(number_range(0,6))
			{
				case 6:
					act(AT_YELLOW, "You begin to name all the possible elements the ship is composed of.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n starts to name elements off the element table.", ch, NULL, NULL, TO_ROOM);
				break;
				case 5:
					act(AT_YELLOW, "You try to figure out the materials the ship is made of.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks at the wall with an intense look.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					act(AT_YELLOW, "You try to figure out how the ship burns fuel.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks like $e is in deep thought.", ch, NULL, NULL, TO_ROOM);
				break;
				case 3:
					act(AT_YELLOW, "You run your hand across the wall, and try to figure out it's construction.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n runs $s hand across the wall and looks focused.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You look closely at vents which channel air into the comparment.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks closely at the vents in the room.", ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					act(AT_YELLOW, "You listen into the walls to hear any mechanical workings of the ship.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n puts $s ear to the wall of the ship.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					act(AT_YELLOW, "You notice some wiring and try to trace where it goes to see how power is handled.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n notices some wiring and $s eyes follow it back to where it is burried.",
						ch, NULL, NULL, TO_ROOM);
				break;
			}

			ch->stimer -= 3;
			ch->inttrain += number_range(1,5);
			check = number_range(1,30);
			check -= (get_curr_int(ch) + get_curr_wis(ch))/2;
			if(check < 1)
				check = 1;
			ch->move -= check;

			if(ch->move < 300)
			{
				act(AT_GREEN, "You become too tired to continue your research.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n stops $s research.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newresearch, 1);

		return;

		// In Doors
		case 2:
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You complete your research.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n completes $s research.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			// Message to Players
			switch(number_range(0,6))
			{
				case 6:
					act(AT_YELLOW, "You knock on the wall, looking for any hollow spots.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n knocks on the wall.", ch, NULL, NULL, TO_ROOM);
				break;
				case 5:
					act(AT_YELLOW, "You jump up and down to test the strength and give of the floor.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n jumps up and down.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					act(AT_YELLOW, "You notice the room's natural lighting.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks around the room.", ch, NULL, NULL, TO_ROOM);
				break;
				case 3:
					act(AT_YELLOW, "You wonder what wattage of light bulb goes into the lights in the room.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n stares at the rooms lighting.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You look at the structure of the room you are in, and wonder what that style is called.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks intensely at $s surroundings.", ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					act(AT_YELLOW, "You look at a window and wonder how the glass is made.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks at a window intently.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					act(AT_YELLOW, "You look at the floor and wonder if the structure was built on a cement base.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks at the floor with a intense gaze.", ch, NULL, NULL, TO_ROOM);
				break;
			}
			ch->stimer -= 3;
			ch->inttrain += number_range(1,5);
			check = number_range(1,30);
			check -= (get_curr_int(ch) + get_curr_wis(ch))/2;
			if(check < 1)
				check = 1;
			ch->move -= check;

			if(ch->move < 300)
			{
				act(AT_GREEN, "You become too tired to continue your research.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n stops $s research.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newresearch, 2);

		return;

		// Regular old Outdoors Research
		case 3:
			if(ch->stimer < 3)
			{
				act(AT_GREEN, "You complete your research.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n completes $s research.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			// Message to Players
			switch(number_range(0,6))
			{
				case 6:
					act(AT_YELLOW, "You notice an animal in the distance and wonder if it is local.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n notices something in the distance.", ch, NULL, NULL, TO_ROOM);
				break;
				case 5:
					act(AT_YELLOW, "You notice some rock formations and wonder if the rocks are Igneous, Metamphoric or Sedimentary.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n notices some rocks in the distance.", ch, NULL, NULL, TO_ROOM);
				break;
				case 4:
					act(AT_YELLOW, "You notice your surroundings and try to think of how life began.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks to be in deep thought.", ch, NULL, NULL, TO_ROOM);
				break;
				case 3:
					act(AT_YELLOW, "You notice a slight breeze and try to judge which direction it is from.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n looks around the room.", ch, NULL, NULL, TO_ROOM);
				break;
				case 2:
					act(AT_YELLOW, "You notice a small bug, and try to see if you can tell how it's body is setup.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n notices a something and then focuses in on it.", ch, NULL, NULL, TO_ROOM);
				break;
				case 1:
					act(AT_YELLOW, "You notice some vegitation and begin to mentally compare it to plants you know.",
						ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n notices something off slightly and begins to look focused at it.", ch, NULL, NULL, TO_ROOM);
				break;
				default:
					act(AT_YELLOW, "You smell something distinct, and then think of what element it could be.", ch, NULL, NULL, TO_CHAR);
					act(AT_YELLOW, "$n smells the air and then 'hmm's out loud.", ch, NULL, NULL, TO_ROOM);
				break;
			}
			ch->stimer -= 3;
			// Slight bonus for the folks who remember the old-school way of researching =P
			ch->inttrain += number_range(1,5) + number_range(0,2);
			check = number_range(1,30);
			check -= (get_curr_int(ch) + get_curr_wis(ch))/2;
			if(check < 1)
				check = 1;
			ch->move -= check;

			if(ch->move < 300)
			{
				act(AT_GREEN, "You become too tired to continue your research.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n stops $s research.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			add_timer(ch, TIMER_DO_FUN, 3, do_newresearch, 3);
		return;

		case SUB_TIMER_DO_ABORT:
			act(AT_GREEN, "You end your research.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n ends $s research.", ch, NULL, NULL, TO_ROOM);
		return;
	}
}

void do_newsocialize(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	PLANET_DATA *planet = NULL;
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool mad = FALSE;


	if(IS_NPC(ch))
		return;


	switch(ch->substate)
	{
		default:
			strcpy(arg, argument);
			if(arg[0] == '\0')
			{
				send_to_char("&RSyntax: Socialize (Target)\r\n", ch);
				return;
			}

			// Maybe in the future there can be some situational dialogue which can be setup for ships
			planet = ch->in_room->area->planet;
			if(!planet)
			{
				send_to_char("&RYou must be on a planet to socialize.\r\n", ch);
				return;
			}

			if((victim = get_char_room( ch, arg)) == NULL)
			{
				send_to_char("&RThe person you selected to socialize with is not here.\r\n", ch);
				return;
			}

			if(!IS_NPC(victim))
			{
				send_to_char("&RYou can only Socialize with NPC's. \r\n", ch);
				return;
			}

			if(victim->position <= POS_SLEEPING)
			{
				ch_printf(ch, "%s can't talk to you like this!\r\n", victim->name);
				return;
			}

			// Different styles of approaching to talk
			switch(number_range(0,4))
			{
				case 4:
					if(ch->alignment > 300)
					{
						act(AT_GREEN, "You beam a big smile towards $N and walk towards $M.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n beams a big smile at you and walks towards you.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n beams a big smile at $N and walks towards $M.", ch, NULL, victim, TO_NOTVICT);
					}
					else
					{
						act(AT_GREEN, "You grin slightly and walk towards $N.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n grins slightly and walks towards you.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n grins slightly and walks towards $N.", ch, NULL, victim, TO_NOTVICT);
					}
				break;
				case 3:
					if(get_curr_cha(ch) > 16)
					{
						act(AT_GREEN, "You confidently walk towards $N.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n confidently walks towards you.",ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n confidently walks towards $N.", ch, NULL, victim, TO_NOTVICT);
					}
					else
					{
						act(AT_GREEN, "You sheepishly walk toward $N.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n confidently walks towards you.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n confidently walks towards $N.", ch, NULL, victim, TO_NOTVICT);
					}
				break;
				case 2:
					if(ch->hit < 300)
					{
						act(AT_GREEN, "You walk slowly to $N, trying to ignore your injurys.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n walks slowly toward you, nursing $s injurys.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n walks slowly towards $N, while trying to nurse $s injurys.", ch, NULL, victim, TO_NOTVICT);
					}
					else
					{
						act(AT_GREEN, "You sprint quickly over to $N.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n sprints quickly over to you.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n sprints quickly over to $N.", ch, NULL, victim, TO_NOTVICT);
					}
				break;
				case 1:
					if(ch->pcdata->pkills > 10)
					{
						act(AT_GREEN, "You crack your knuckles and approach $N.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n cracks $s knuckles and approaches you.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n cracks $s knuckles and approaches $N.", ch, NULL, victim, TO_NOTVICT);
					}
					else
					{
						act(AT_GREEN, "You stretch your arms slightly and approach $N.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "$n stretches $s arms slightly and approaches you.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$n stretches $s arms slightly and approaches $N.", ch, NULL, victim, TO_NOTVICT);
					}
				break;
				default:
					act(AT_GREEN, "You walk up to $N.", ch, NULL, victim,TO_CHAR);
					act(AT_GREEN, "$n walks up to you.", ch, NULL, 	victim, TO_VICT);
					act(AT_GREEN, "$n walks up to $N.", ch, NULL, victim, TO_NOTVICT);
				break;
			}

			// Now it is time to see the Mobile's responce! Let the fun begin!
			/*
			 * Funny thing, frustration and frustrated_by basicly is how a
			 * mobile will tell how to respond to certain situations, of course
			 * don't discount their stats and level. - Gatz
			 */
			if(victim->frustrated_by == ch)
			{
				switch(number_range(0,3))
				{
					case 3:
						do_say(victim, "You bastard! Buzz off!");
					break;
					case 2:
						do_say(victim, "You are an annoying jerk, go away!");
					break;
					case 1:
						do_say(victim, "Why would I wanna talk to you!?!");
					break;
					default:
						do_say(victim, "You're an idiot, get out of here.");
					break;
				}
				return;
			}


			int flevel = 0;

			flevel = victim->frustration/10;
			if(flevel < 1)
				flevel = 1;

			switch(flevel)
			{
				// This option is when they are very pissed off
				default:
					do_say(victim, "Piss off, I am not in the mood to talk!");
				return;
				case 4:
					do_say(victim, "What the hell do you want?");
				break;
				case 3:
					do_say(victim, "You got something to tell me?");
				break;
				case 2:
					do_say(victim, "Yeah?");
				break;
				case 1:
					if(victim->alignment > 300)
					{
						act(AT_GREEN, "$N looks up and smiles at you.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "You look up and smile at $n.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$N looks up and smiles at $n.", ch, NULL, victim, TO_NOTVICT);
						do_say(victim, "Hi there.");
					}
					else
					{
						act(AT_GREEN, "$N looks up and grins at you.", ch, NULL, victim, TO_CHAR);
						act(AT_GREEN, "You look up and grin at $n.", ch, NULL, victim, TO_VICT);
						act(AT_GREEN, "$N looks up and grins at $n.", ch, NULL, victim, TO_NOTVICT);
						switch(number_range(0,3))
						{
							case 3:
								do_say(victim, "Yo.");
							break;
							case 2:
								do_say(victim, "Heh, how's it going?");
							break;
							case 1:
								do_say(victim, "Hey pal.");
							break;
							default:
								do_say(victim, "What is it?");
							break;
						}
					}
				break;
			}

			add_timer(ch, TIMER_DO_FUN, 3, do_newsocialize, 1);
			ch->dest_buf = str_dup(arg);
		return;
		// Intro
		case 1:
			if ( !ch->dest_buf )
                        	return;
                	strcpy(arg, ch->dest_buf);
                	DISPOSE( ch->dest_buf);
			if((victim = get_char_room(ch, arg)) == NULL)
			{
				act(AT_GREEN, "You look around the room, wondering where the person you were talking to went to.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n looks around the room, looking for someone.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			if(victim->position == POS_FIGHTING)
			{
				switch(number_range(0,3))
				{
					case 3:
						do_say(victim, "I am busy! I can't talk now!");
					break;
					case 2:
						do_say(victim, "I am fighting someone, I can't talk.");
					break;
					case 1:
						do_say(victim, "I am in the middle of a fight! I can't talk!");
					break;
					default:
						do_say(victim, "I am fighting right now, go away.");
					break;
				}

				return;
			}
                        if(!IS_NPC(victim))
                        {
                                send_to_char("&RYou can only Socialize with NPC's. \r\n", ch);
                                return;
                        }

			// Now the fun begins, lets run through all situations!
			switch(number_range(0,4))
			{
				case 4:
					if(ch->pcdata->clan)
					{
						sprintf(buf, "So, how do you like %s?", ch->pcdata->clan->name);
						ch->stimer = 0;
					}
					else
					{
						if(ch->in_room->area->planet->governed_by)
						{
							sprintf(buf, "How is %s at leading the planet?", ch->in_room->area->planet->governed_by->name);
							ch->stimer = 1;
						}
						else
						{
							sprintf(buf, "So, which clan is your favorite?");
							ch->stimer = 2;
						}
					}
				break;
				case 3:
					if(IS_SET(victim->act, ACT_SENTINEL))
					{
						sprintf(buf, "It seems you like to stand around here a lot, see anything interesting?");
						ch->stimer = 3;
					}
					else
					{
						sprintf(buf, "You seem like the type to wander. See anything interesting?");
						ch->stimer = 4;
					}
				break;
				case 2:
					if(ch->alignment < 300)
					{
						sprintf(buf, "Heh, so what is your favorite weapon?");
						ch->stimer = 5;
					}
					else
					{
						sprintf(buf, "So, what do you do for fun?");
						ch->stimer = 6;
					}
				break;
				case 1:
					if(ch->alignment > 300)
						sprintf(buf, "How are you doing today?");
					else
						sprintf(buf, "Sup?");
					ch->stimer = 7;
				break;
				default:
					if(get_curr_cha(ch) > 16)
					{
						if(victim->sex != ch->sex)
						{
							sprintf(buf, "Did anyone ever tell you that you look %s?",
								(victim->sex == 1)? "attractive" : "cute");
							ch->stimer = 8;
						}
						else
						{
							sprintf(buf, "I think you look good today.");
							ch->stimer = 9;
						}
					}
					else
					{
						sprintf(buf, "Today has been a long day.");
						ch->stimer = 10;
					}
				break;
			}


			do_say(ch, buf);
			ch->chatrain += number_range(0,3);
			add_timer(ch, TIMER_DO_FUN, 3, do_newsocialize, 2);
			ch->dest_buf = str_dup(arg);
		return;
		// This is the main part where the mobile generally has some relative AI in response
		case 2:
			if ( !ch->dest_buf )
                        	return;
                	strcpy(arg, ch->dest_buf);
                	DISPOSE( ch->dest_buf);
			if((victim = get_char_room(ch, arg)) == NULL)
			{
				act(AT_GREEN, "You look around the room, wondering where the person you were talking to went to.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n looks around the room, looking for someone.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			if(victim->position == POS_FIGHTING)
			{
				switch(number_range(0,3))
				{
					case 3:
						do_say(victim, "I am busy! I can't talk now!");
					break;
					case 2:
						do_say(victim, "I am fighting someone, I can't talk.");
					break;
					case 1:
						do_say(victim, "I am in the middle of a fight! I can't talk!");
					break;
					default:
						do_say(victim, "I am fighting right now, go away.");
					break;
				}

				return;
			}
			if(!IS_NPC(victim))
                        {
                                send_to_char("&RYou can only Socialize with NPC's. \r\n", ch);
                                return;
                        }

			// Now the fun begins, lets run through all situations!
			switch(ch->stimer)
			{
				case 0:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						sprintf(buf, "Um, that clan is really actually pretty dumb.");
						victim->frustration += number_range(0,3);
						ch->stimer = 0;
					}
					else
					{
						if(victim->alignment < 300)
						{
							sprintf(buf, "%s is cool! They have sent %d people to the hospital!",
								ch->pcdata->clan->name, ch->pcdata->clan->pkills);
							ch->stimer = 1;
						}
						else
						{
							sprintf(buf, "%s is alright, their tactics are pretty cool. I guess.",
								ch->pcdata->clan->name);
							ch->stimer = 2;
						}
					}
				break;
				case 1:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						if(victim->alignment > 300)
							sprintf(buf, "%s does a crappy job, whatever. I don't even care.", ch->pcdata->clan->name);
						else
						{
							sprintf(buf, "The only job %s does is a screw job on all the %s people.",
								ch->in_room->area->planet->governed_by->name, ch->in_room->area->planet->name);
						}
						victim->frustration += number_range(0,3);
						ch->stimer = 3;
					}
					else
					{
						switch(number_range(0,3))
						{
							case 3:
								sprintf(buf, "%s could be better, but I have seen worse.",
									ch->in_room->area->planet->governed_by->name);
								ch->stimer = 4;
							break;
							case 2:
								sprintf(buf, "%s is doing good, it is the best government this planet has had.",
									ch->in_room->area->planet->governed_by->name);
								ch->stimer = 5;
							break;
							case 1:
								sprintf( buf, "I like %s, I think they are generally doing things which help us.",
									ch->in_room->area->planet->governed_by->name);
								ch->stimer = 6;
							break;
							default:
								if(!str_cmp(ch->in_room->area->planet->governed_by->name, "ISSP"))
								{
									if(victim->alignment > 300)
										sprintf(buf, "ISSP seems to catch bounties, I guess that is cool.");
									else
										sprintf(buf, "ISSP needs to relax on catching bounties, I think.");
								}
								else if(!str_cmp(ch->in_room->area->planet->governed_by->name, "RDS"))
								{
									if(victim->alignment > 300)
										sprintf(buf, "RDS is pretty rough, I think they need to be nicer.");
									else
										sprintf(buf, "I love RDS, I love how they know how to crack skulls!");
								}
								else if(str_cmp(ch->in_room->area->planet->governed_by->name, "BSS"))
								{
									if(victim->alignment > 300)
										sprintf(buf, "BSS is an odd mix, I dunno if I can really say anything.");
									else
										sprintf(buf, "BSS needs to stop with the 'Blue Snake' crap and kill more!");
								}
								ch->stimer = 7;
							break;
						}
					}
				break;
				case 2:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						sprintf(buf, "My favorite clan? What does that have to do with anything?");
						victim->frustration += number_range(0,3);
						ch->stimer = 8;
					}
					else
					{
						switch(number_range(0,2))
						{
							case 2:
								sprintf(buf, "BSS of course! Gotta love the Snakes!");
								ch->stimer = 9;
							break;
							case 1:
								sprintf(buf, "RDS is the best! When the Dragon fies, everything dies!");
								ch->stimer = 10;
							break;
							default:
								sprintf(buf, "ISSP is the coolest, gotta root for the good guys, right?");
								ch->stimer = 11;
							break;
						}
					}
				break;
				case 3:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						sprintf(buf, "Is that supposed to be some form of an insult?");
						victim->frustration += number_range(0,3);
						ch->stimer = 12;
					}
					else
					{
						switch(number_range(0,2))
						{
							case 2:
								sprintf(buf, "I some times see a few fights, but usually nothing.");
								ch->stimer = 13;
							break;
							case 1:
								sprintf(buf, "Well, Some gum keeps me from moving so...");
								ch->stimer = 14;
							break;
							default:
								sprintf(buf, "I dunno, I am generally not paying attention to most things.");
								ch->stimer = 15;
							break;
						}
					}
				break;
				case 4:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						sprintf(buf, "Wander? Yeah, whatever.");
						victim->frustration += number_range(0,3);
						ch->stimer = 16;
					}
					else
					{
						switch(number_range(0,2))
						{
							case 2:
								sprintf(buf, "Yeah, I see awesome things. Until people try to attack me!");
								ch->stimer = 17;
							break;
							case 1:
								sprintf(buf, "Yeah, %s has some cool sites to see.",
									ch->in_room->area->planet->name);
								ch->stimer = 18;
							break;
							default:
								sprintf(buf, "I don't really care about anything I see. So I don't know really.");
								ch->stimer = 19;
							break;
						}
					}
				break;
				case 5:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						if(victim->alignment > 300)
							sprintf(buf, "Excuse me? I don't wanna talk about this.");
						else
							sprintf(buf, "Any weapon which will make you go away.");
						victim->frustration += number_range(0,3);
						ch->stimer = 20;
					}
					else
					{
						switch(number_range(0,2))
						{
							case 2:
								if(victim->alignment > 300)
								{
									sprintf(buf, "Weapons are not needed.");
									ch->stimer = 21;
								}
								else
								{
									sprintf(buf, "You can't beat fighting with your hands.");
									ch->stimer = 22;
								}
							break;
							case 1:
								if(victim->alignment > 300)
								{
									sprintf(buf, "I think guns are alright, in the right hands.");
									ch->stimer = 23;
								}
								else
								{
									sprintf(buf, "I love a good gun.");
									ch->stimer = 24;
								}
							break;
							default:
								if(victim->alignment > 300)
								{
									sprintf(buf, "Nothing beats the beauty of a well made sword.");
									ch->stimer = 25;
								}
								else
								{
									sprintf(buf, "A knife is the best way to launch a sneak attack!");
									ch->stimer = 26;
								}
							break;
						}
					}
				break;
				case 6:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						if(victim->alignment > 300)
							sprintf(buf, "Trying to avoid talking to losers like you.");
						else
							sprintf(buf, "Trying to avoid damn pests like you.");
						victim->frustration += number_range(0,3);
						ch->stimer = 27;
					}
					else
					{
						switch(number_range(0,3))
						{
							case 3:
								if(victim->alignment < 300)
									sprintf(buf, "Fight, steal, whatever I want to do.");
								else
									sprintf(buf, "Read, sometimes watch movies.");
								ch->stimer = 28;
							break;
							case 2:
								sprintf(buf, "Nothing much, really.");
								ch->stimer = 29;
							break;
							case 1:
								sprintf(buf, "I am too busy to do anything fun.");
								ch->stimer = 30;
							break;
							default:
								sprintf(buf, "%s has lots to do, I can't pick just one thing.",
									ch->in_room->area->planet->name);
								ch->stimer = 31;
							break;
						}
					}
				break;
				case 7:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						if(victim->alignment > 300)
							sprintf(buf, "I would be better if you just left me alone.");
						else
							sprintf(buf, "I would be doing good if you buzzed off!");
						victim->frustration += number_range(0,3);
						ch->stimer = 32;
					}
					else
					{
						switch(number_range(0,3))
						{
							case 3:
								sprintf(buf, "I am doing good, I have no complaints.");
								ch->stimer = 33;
							break;
							case 2:
								if(victim->hit < 500)
								{
									sprintf(buf, "How the hell do I look? I am bleeding!");
									ch->stimer = 34;
								}
								else
								{
									sprintf(buf, "I'm going fine.");
									ch->stimer = 35;
								}
							break;
							case 1:
								sprintf(buf, "I am tired.");
								ch->stimer = 36;
							break;
							default:
								sprintf(buf, "I can't wait to just relax and call it a day!");
								ch->stimer = 37;
							break;
						}
					}
				break;
				case 8:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						if(victim->alignment > 300)
							sprintf(buf, "What, are you trying to hit on me?");
						else
							sprintf(buf, "You need to go take a cold shower.");
						victim->frustration += number_range(0,3);
						ch->stimer = 38;
					}
					else
					{
						switch(number_range(0,3))
						{
							case 3:
								sprintf(buf, "Oh stop! I am going to blush.");
								ch->stimer = 39;
							break;
							case 2:
								sprintf(buf, "That is a little random, isn't it?");
								ch->stimer = 40;
							break;
							case 1:
								sprintf(buf, "Um, that is good.");
								ch->stimer = 41;
							break;
							default:
								sprintf(buf, "Thanks.");
								ch->stimer = 42;
							break;
						}
					}
				break;
				case 9:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						if(victim->alignment > 300)
							sprintf(buf, "Yeah...that is not a cool thing to say.");
						else
							sprintf(buf, "Yeah, you are weird...");
						victim->frustration += number_range(0,3);
						ch->stimer = 43;
					}
					else
					{
						switch(number_range(0,3))
						{
							case 3:
								if(victim->alignment > 300)
									sprintf(buf, "Thanks! That is sweet of you to say!");
								else
									sprintf(buf, "Uh, thanks, I think.");
								ch->stimer = 44;
							break;
							case 2:
								if(victim->alignment > 300)
									sprintf(buf, "You aren't so bad yourself.");
								else
									sprintf(buf, "Heh, are you trying to get something from me?");
								ch->stimer = 45;
							break;
							case 1:
								sprintf(buf, "I try.");
								ch->stimer = 46;
							break;
							default:
								sprintf(buf, "Why do you say that?");
								ch->stimer = 47;
							break;
						}
					}
				break;
				case 10:
					if(number_range(0, (victim->frustration/2) + 1) > 10)
					{
						sprintf(buf, "They have self help groups of that.");
						victim->frustration += number_range(0,3);
						ch->stimer = 48;
					}
					else
					{
						if(victim->alignment < 300)
						{
							sprintf(buf, "Why not call someone who cares?");
							ch->stimer = 49;
						}
						else
						{
							sprintf(buf, "You poor %s.", (ch->sex == 1)? "guy" : "girl");
							ch->stimer = 50;
						}
					}
				break;
			}


			do_say(victim, buf);
			ch->chatrain += number_range(0,3);
			add_timer(ch, TIMER_DO_FUN, 3, do_newsocialize, 3);
			ch->dest_buf = str_dup(arg);
		break;
		// This is the conclusion of the mini-dialogue

		case 3:
			if ( !ch->dest_buf )
                        	return;
                	strcpy(arg, ch->dest_buf);
                	DISPOSE( ch->dest_buf);
			if((victim = get_char_room(ch, arg)) == NULL)
			{
				act(AT_GREEN, "You look around the room, wondering where the person you were talking to went to.", ch, NULL, NULL, TO_CHAR);
				act(AT_GREEN, "$n looks around the room, looking for someone.", ch, NULL, NULL, TO_ROOM);
				return;
			}

			if(victim->position == POS_FIGHTING)
			{
				switch(number_range(0,3))
				{
					case 3:
						do_say(victim, "I am busy! I can't talk now!");
					break;
					case 2:
						do_say(victim, "I am fighting someone, I can't talk.");
					break;
					case 1:
						do_say(victim, "I am in the middle of a fight! I can't talk!");
					break;
					default:
						do_say(victim, "I am fighting right now, go away.");
					break;
				}

				return;
			}
                        if(!IS_NPC(victim))
                        {
                                send_to_char("&RYou can only Socialize with NPC's. \r\n", ch);
                                return;
                        }

			// Now the fun begins, lets run through all situations!
			switch(ch->stimer)
			{
				case 0:
					switch(number_range(0,3))
					{
						case 3:
							do_say(ch, "To each his own I guess.");
						break;
						case 2:
							sprintf(buf, "You obviously lack the ability to see why %s is awesome.",
									ch->pcdata->clan->name);
							do_say(ch, buf);
						case 1:
							sprintf(buf, "The hell you know! I bet you haven't a clue about %s!", ch->pcdata->clan->name);
							do_say(ch, buf);
							if(victim->alignment > 300)
								do_say(ch, "You shouldn't have asked me then!");
							else
								do_say(ch, "Heh, yeah right.");
						break;
						default:
							do_say(ch, "Awell, you have your opinion I guess.");
							if(victim->alignment < 300)
								do_say(victim, "Damn right, I do.");
						break;
					}
				break;
				case 1:
					switch(number_range(0,2))
					{
						case 2:
							sprintf(buf, "Yeah %s is always ready for some action!", ch->pcdata->clan->name);
							do_say(ch, buf);
						break;
						case 1:
							do_say(ch, "You know, I am in that clan?");
							if(victim->sex != ch->sex)
								do_say(victim, "I'm in love. Hehe.");
							else
								do_say(victim, "Cool.");
						break;
						default:
							do_say(ch, "I like them too.");
						break;
					}
				break;
				case 2:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Just alright? I think they are awesome!");
							if(number_range(0,2) == 2)
								do_say(victim, "I guess they are pretty cool.");
							else
								do_say(victim, "Eh, to each his own.");
						break;
						case 1:
							do_say(ch, "Alright, huh, heh.");
						break;
						default:
							do_say(ch, "Alright? Common, show a little more energy.");
							if(victim->alignment < 300)
								do_say(victim, "Yeah, I know exactly how, too. Hehe.");
						break;
					}
				break;
				case 3:
					switch(number_range(0,3))
					{
						case 3:
							if(ch->pcdata->clan && ch->in_room->area->planet->governed_by
								&& ch->pcdata->clan == ch->in_room->area->planet->governed_by)
							{
								sprintf(buf, "I think %s is doing a great job!", ch->pcdata->clan);
								do_say(ch, buf);
							}
							else
							{
								do_say(ch, "Could you do any better?");
								if(victim->alignment > 300)
									do_say(victim, "I would at least -try-.");
								else
									do_say(victim, "My worst would be much better than their best.");
							}
						break;
						case 2:
							do_say(ch, "Why say that?");
							if(victim->alignment > 300)
								do_say(victim, "Stick around and you will get why.");
							else
								do_say(victim, "Open your damn eyes and see why.");
						break;
						case 1:
							do_say(ch, "Awell, nobody is perfect!");
						break;
						default:
							do_say(ch, "That bad, huh?");
							do_say(victim, "Sometimes worse.");
						break;
					}
				break;
				case 4:
					if(number_range(0,1) == 1)
						do_say(ch, "Gotta look on the brightside, right?");
					else
						do_say(ch, "Least you are alive. That is a positive I guess.");
				break;
				case 5:
					switch(number_range(0,2))
					{
						case 2:
							if(ch->pcdata->clan && ch->in_room->area->planet->governed_by
								&& ch->pcdata->clan == 	ch->in_room->area->planet->governed_by)
							{
								do_say(ch, "Yeah, glad to see you like them. That is why -I- am in that clan.");
								do_say(victim, "Cool.");
							}
							else
								do_say(ch, "Eh, guess that is good for you.");
						case 1:
							do_say(ch, "Good to hear.");
						break;
						default:
							do_say(ch, "Fantastic.");
						break;
					}
				break;
				case 6:
					if(number_range(0,1) == 1)
					{
						if(ch->alignment > 300)
							do_say(ch, "That is great, good to see a clan helping it's planet.");
						else
							do_say(ch, "Heh, only people who need help are babies.");
					}
					else
					{
						if(ch->alignment > 300)
							do_say(ch, "That is cool.");
						else
							do_say(ch, "Yeah, that is great if you are helpless.");
					}
				break;
				case 7:
					if(number_range(0,1) == 1)
						do_say(ch, "I suppose you're right.");
					else
						do_say(ch, "I dunno about that.");
				break;
				case 8:
					if(ch->alignment > 300)
						do_say(ch, "Sorry! I was just wondering.");
					else
					{
						sprintf(buf, "Don't be a %s! It was a simple question!", (victim->sex == 1)? "ass" : "bitch");
						do_say(ch, buf);
					}
				break;
				case 9:
					if(ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name, "BSS"))
						do_say(ch, "Good choice!");
					else if(ch->pcdata->clan && str_cmp(ch->pcdata->clan->name, "BSS"))
						do_say(ch, "Heh, I think there are better clans out there.");
					else
						do_say(ch, "Interesting pick.");
				break;
				case 10:
					if(ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name, "RDS"))
						do_say(ch, "Good choice!");
					else if(ch->pcdata->clan && str_cmp(ch->pcdata->clan->name, "RDS"))
						do_say(ch, "Heh, I think there are better clans out there.");
					else
						do_say(ch, "Interesting pick.");
				break;
				case 11:
					if(ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name, "ISSP"))
						do_say(ch, "Good choice!");
					else if(ch->pcdata->clan && str_cmp(ch->pcdata->clan->name, "ISSP"))
						do_say(ch, "Heh, I think there are better clans out there.");
					else
						do_say(ch, "Interesting pick.");
				break;
				case 12:
					switch(number_range(0,2))
					{
						case 2:
							if(ch->alignment > 300)
								do_say(ch, "Sorry! I didn't mean that!");
							else
							{
								do_say(ch, "Maybe I did? What's it to you.");
								if(ch->alignment > 300)
									do_say(victim, "Don't be such a jerk!");
								else
									do_say(victim, "Heh, if you did I would have to re-arrange your face.");
							}
						break;
						case 1:
							do_say(ch, "Don't get so upset over nothing.");
						break;
						default:
							do_say(ch, "I didn't mean anything by it.");
						break;
					}
				break;
				case 13:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "A few fights, that sounds pretty rough.");
							if(victim->alignment > 300)
								do_say(victim, "I try to avoid them.");
							else
								do_say(victim, "I can handle myself.");
						break;
						case 1:
							do_say(ch, "Sounds exciting.");
							do_say(victim, "Not really.");
						break;
						default:
							do_say(ch, "You ever get in any fights?");
							if(victim->alignment > 300)
								do_say(victim, "I try not to.");
							else
								do_say(victim, "Heh, stay around and see for yourself.");
						break;
					}
				break;
				case 14:
					if(ch->alignment > 300)
						do_say(ch, "Some gum?!?!");
					else
						do_say(ch, "You have got to be kidding me.");
					if(victim->alignment > 300)
						do_say(victim, "Ha! Of course I am.");
					else
						do_say(victim, "Wow, time for the clue train to come in, next stop the obvious. Of course I am.");
				break;
				case 15:
					do_say(ch, "Well, that isn't very interesting.");
				break;
				case 16:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Only meant in the sense that you see a lot of things!");
						break;
						case 1:
							do_say(ch, "Relax, I didn't mean it offensively!");
						break;
						default:
							do_say(ch, "It is okay, I won't push you to answer...");
						break;
					}
				break;
				case 17:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "At least you're still alive.");
						break;
						case 1:
							do_say(ch, "Why would they attack you?");
							if(victim->sex == 1)
								do_say(victim, "Wulongs I guess.");
							else
								do_say(victim, "Jewelry I have, I think.");
							do_say(ch, "Really? That is...interesting. Heh.");
						break;
						default:
							do_say(ch, "Well, the best fights are the ones you can walk away from, I guess.");
						break;
					}
				break;
				case 18:
					switch(number_range(0,2))
					{
						case 2:
							sprintf(buf, "%s seems like a cool planet.", ch->in_room->area->planet->name);
							do_say(ch, buf);
						break;
						case 1:
							do_say(ch, "Maybe you could show me around sometime?");
							if(victim->alignment > 300)
								do_say(victim, "Maybe.");
							else
								do_say(victim, "Dream on.");
						break;
						default:
							do_say(ch, "That is good.");
						break;
					}
				break;
				case 19:
					if(ch->alignment > 300)
						do_say(ch, "Oh...");
					else
						do_say(ch, "Er...okay.");
				break;
				case 20:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "It would be a pretty good weapon then.");
						break;
						case 1:
							do_say(ch, "Hope that's not a threat.");
							if(victim->alignment < 300)
								do_say(victim, "Take it whatever way you want.");
						break;
						default:
							do_say(ch, "Heh, real funny.");
						break;
					}
				break;
				case 21:
					switch(number_range(0,3))
					{
						case 3:
							do_say(ch, "Not...not...needed?!?! What planet do you live on?");
							sprintf(buf, "%s, obviously.", ch->in_room->area->planet->name);
						break;
						case 2:
							do_say(ch, "In a perfect reality, weapons aren't needed but in this one they are.");
						break;
						case 1:
							do_say(ch, "A reality without guns sounds almost bizarre.");
						break;
						default:
							do_say(ch, "You need a weapon to survive in this world!");
						break;
					}
				break;
				case 22:
					switch(number_range(0,3))
					{
						case 3:
							do_say(ch, "Fighting with your hands? That is some hokey Ancient thinking.");
						break;
						case 2:
							do_say(ch, "Heh, I guess it lets you feel your victims bones breaking.");
						break;
						case 1:
							do_say(ch, "I guess that means you enjoy working harder to win.");
						break;
						default:
							do_say(ch, "Nothing beats a good gun or sharp sword.");
						break;
					}
				break;
				case 23:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "The right hands, how cliche.");
						break;
						case 1:
							do_say(ch, "The right hands are my own.");
						break;
						default:
							do_say(ch, "Heh, no matter who has what there will always be problems.");
						break;
					}
				break;
				case 24:
					switch(number_range(0,2))
					{
						case 2:
							if(victim->sex == ch->sex)
								do_say(ch, "Right on.");
							else
							{
								sprintf(buf, "A %s after my own heart!", (victim->sex == 1)? "man" : "woman");
								do_say(ch, buf);
							}
						break;
						case 1:
							do_say(ch, "Haha, definently can't beat a good gun.");
						break;
						default:
							do_say(ch, "No compliants here.");
						break;
					}
				break;
				case 25:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Not so easy to find a well made sword these days.");
						break;
						case 1:
							do_say(ch, "There is a certain art form of using a sword to cut an enemy down");
						break;
						default:
							do_say(ch, "That sounds solid to me.");
						break;
					}
				break;
				case 26:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "A knife, huh? Interesting.");
						break;
						case 1:
							do_say(ch, "Sneak attacks can be useful sometimes.");
						break;
						default:
							do_say(ch, "Sneak attacks seem cheap sometimes.");
							do_say(victim, "But affective.");
						break;
					}
				break;
				case 27:
					switch(number_range(0,2))
					{
						case 2:
							if(ch->alignment > 300)
								do_say(ch, "That's not nice!");
							else
								do_yell(ch, "I am not a loser!");
						break;
						case 1:
							do_say(ch, "Damn, that is harsh.");
						break;
						default:
							do_say(ch, "The hell's your problem?");
							if(victim->alignment > 3000)
								do_say(victim, "I am a little upset.");
							else
								do_say(victim, "You, you idiot!");
						break;
					}
				break;
				case 28:
					switch(number_range(0,2))
					{
						case 2:
							if(victim->alignment < 300)
							{
								if(ch->alignment > 300)
									do_say(ch, "Stealing? That isn't good...");
								else
									do_say(ch, "I like your additude.");
							}
							else
							{
								if(ch->alignment > 300)
									do_say(ch, "That sounds cool.");
								else
									do_say(ch, "That sounds boring.");
							}
						break;
						case 1:
							do_say(ch, "Interesting.");
						break;
						default:
							if(ch->alignment < 300)
								do_say(ch, "Yeah, fantastic.");
							else
								do_say(ch, "That's great.");
							do_say(victim, "Were you paying attention?");
							do_say(ch, "Of course...");
						break;
					}
				break;
				case 29:
					if(ch->alignment > 300)
						do_say(ch, "Nothing is better than a bad something!");
					else
						do_say(ch, "Wow, how incredibily dull.");
				break;
				case 30:
					do_say(ch, "Too busy, doing what?");
					if(victim->alignment > 300)
						do_say(victim, "Lots of things.");
					else
						do_say(victim, "Ha, wouldn't -you- like to know.");
				break;
				case 31:
					switch(number_range(0,2))
					{
						case 2:
							sprintf(buf, "Sounds like you really like %s.", ch->in_room->area->planet->name);
							do_say(ch, buf);
						break;
						case 1:
							do_say(ch, "You seem to spend a lot of time here, so it is good you like it.");
						break;
						default:
							do_say(ch, "It's good to like where you live.");
						break;
					}
				break;
				case 32:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Whoa, don't be so confrontational! I am just trying to talk!");
						break;
						case 1:
							do_say(ch, "Geez, relax.");
						break;
						default:
							do_say(ch, "Damn, you are being mean.");
						break;
					}
				break;
				case 33:
					if(ch->alignment > 300)
						do_say(ch, "Good, no compliants ain't bad.");
					else
						do_say(ch, "Good for you.");
				break;
				case 34:
					if(ch->alignment > 300)
						do_say(ch, "Ick, you should go to the hospital.");
					else
						do_say(ch, "Don't be such a baby. Suck it up.");
				break;
				case 35:
					if(number_range(0,1) == 1)
						do_say(ch, "Not a bad one.");
					else
						do_say(ch, "That's good");
				break;
				case 36:
					if(ch->alignment > 300)
						do_say(ch, "Well, you don't look tired, if that is any consulation.");
					else
						do_say(ch, "Get some rest, duh.");
				break;
				case 37:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "I wouldn't mind calling it a day soon either.");
						break;
						case 1:
							do_say(ch, "The day has just begun!");
						break;
						default:
							do_say(ch, "Still a lot left to do today.");
						break;
					}
				break;
				case 38:
					do_say(ch, "Relax.");
				break;
				case 39:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "I can't deny the truth!");
						break;
						case 1:
							do_say(ch, "Well it's true.");
						break;
						default:
							do_say(ch, "Heh, don't blush. It's ust the truth.");
						break;
					}
				break;
				case 40:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Well, I...I mean...nevermind, forget it.");
						break;
						case 1:
							if(victim->sex == 2)
								do_say(ch, "A pretty young lady such as yourself should be told!");
							else
								do_say(ch, "A cute guy like yourself shouldn't be left in the dark.");
						break;
						default:
							do_say(ch, "Well, I meant it to be a compliment.");
						break;
					}
				break;
				case 41:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Hey, anytime!");
						break;
						case 1:
							do_say(ch, "No problem.");
						break;
						default:
							do_say(ch, "Don't mention it!");
						break;
					}
				break;
				case 42:
					do_say(ch, "Your welcome.");
					do_say(victim, "You're not so bad yourself.");
				break;
				case 43:
					if(ch->alignment > 300)
						do_say(ch, "I was just trying to say something nice, don't get all moody!");
					else
						do_say(ch, "Damn, you are in a sensitive mood!");
				break;
				case 44:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "I'm not coming on to you or anything, just saying is all.");
						break;
						case 1:
							do_say(ch, "Just meant it in a nice way, I am not trying to hit on you.");
						break;
						default:
							do_say(ch, "Just thought I would compliment is all.");
						break;
					}
				break;
				case 45:
					switch(number_range(0,2))
					{
						case 2:
							if(victim->alignment > 300)
								do_say(ch, "Thanks.");
							else
								do_say(ch, "Just being friendly.");
						break;
						case 1:
							if(victim->alignment > 300)
								do_say(ch, "Too kind!");
							else
								do_say(ch, "I don't mean to sound like I am trying to get something from you!");
						break;
						default:
							if(victim->alignment > 300)
								do_say(ch, "That's nice of you to say.");
							else
								do_say(ch, "No, nothing!");
						break;
					}
				break;
				case 46:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "I can't tell if you are kidding or not.");
						break;
						case 1:
							do_say(ch, "You look like a natural!");
						break;
						default:
							do_say(ch, "Well, you try and succeed.");
						break;
					}
				break;
				case 47:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "You have nice eyes.");
						break;
						case 1:
							do_say(ch, "You have a great smile.");
						break;
						default:
							do_say(ch, "You have a fantastic body.");
						break;
					}
				break;
				case 48:
					if(ch->alignment > 300)
						do_say(ch, "Wow, don't be an idiot.");
					else
						do_say(ch, "Wow, don't be a ass.");
				break;
				case 49:
					switch(number_range(0,2))
					{
						case 2:
							do_say(ch, "Heh.");
						break;
						case 1:
							do_say(ch, "Keep talking like this and you will be calling an ambulence.");
						break;
						default:
							do_say(ch, "Hah, I can tell you who to call.");
						break;
					}
				break;
				case 50:
					if(ch->alignment > 300)
						do_say(ch, "Thanks, that is nice of you to say.");
					else
						do_say(ch, "I don't need your sympathy.");
				break;
			}

			ch->chatrain += number_range(0,3);
			// Random case for mishaps
			if(number_range(0,6) == 3)
				victim->frustration += number_range(0,4);
			if(victim->frustration > 15)
			{
				if(number_range(0,30) > 25)
				{
					do_say(victim, "Consider this conversation over, I am done talking to you.");

					victim->frustrated_by = ch;
					victim->frustration += 3;
					return;
				}
			}
			// Now we go back to the begining! We can start a new mini-dialogue
			add_timer(ch, TIMER_DO_FUN, 3, do_newsocialize, 1);
			ch->dest_buf = str_dup(arg);
		return;

		case SUB_TIMER_DO_ABORT:
                	DISPOSE( ch->dest_buf );
                	ch->substate = SUB_NONE;
                	act(AT_GREEN, "You loose focus and stop your conversation.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n looses focus and stops $s conversation.", ch, NULL, NULL, TO_ROOM);
                break;
    }

    return;
}

// Modified Gainage Command - Locke
void do_gainage(CHAR_DATA *ch, char *argument)
{
        int maxstr = 12000;
        int maxwis = 6600;
        int maxint = 5500;
        int maxcon = 9500;
        int maxdex = 16300;
        int maxcha = 12500;
	float tempstr = 0, tempwis = 0, tempint = 0, tempcon = 0, tempdex = 0, tempcha = 0;

	tempwis = ch->wistrain;
	tempcon = ch->contrain;
	tempint = ch->inttrain;
	tempstr = ch->strtrain;
	tempdex = ch->dextrain;
	tempcha = ch->chatrain;

        int pstr, pwis, pint, pcon, pdex, pcha;
        pstr = pwis = pint = pcon = pdex = pcha = 0;

	switch(ch->perm_str)
	{
		case 11:maxstr=300;break;
		case 12:maxstr=500;break;
		case 13:maxstr=750;break;
		case 14:maxstr=1000;break;
		case 15:maxstr=4500;break;
		case 16:maxstr=9000;break;
		case 17:case 18:maxstr=12000;break;
		case 10:
		default:maxstr=200;break;
	};

	switch(ch->perm_wis)
	{
		case 11:maxwis=200;break;
		case 12:maxwis=350;break;
		case 13:maxwis=550;break;
		case 14:maxwis=750;break;
		case 15:maxwis=1000;break;
		case 16:maxwis=4300;break;
		case 17:case 18:maxwis=6600;break;
		case 10:
		default:maxwis=100;break;
	};

	switch(ch->perm_dex)
	{
		case 11:maxdex=800;break;
		case 12:maxdex=1500;break;
		case 13:maxdex=2250;break;
		case 14:maxdex=5600;break;
		case 15:maxdex=10500;break;
		case 16:maxdex=13100;break;
		case 17:case 18:maxdex=16300;break;
		case 10:
		default:maxdex=450;break;
	};

	switch(ch->perm_int)
	{
		case 11:maxint=250;break;
		case 12:maxint=400;break;
		case 13:maxint=550;break;
		case 14:maxint=700;break;
		case 15:maxint=1750;break;
		case 16:maxint=3100;break;
		case 17:case 18:maxint=5500;break;
		case 10:
		default:maxint=170;break;
	};

	switch(ch->perm_cha)
	{
		case 11:maxcha=400;break;
		case 12:maxcha=600;break;
		case 13:maxcha=900;break;
		case 14:maxcha=2250;break;
		case 15:maxcha=4700;break;
		case 16:maxcha=7100;break;
		case 17:case 18:maxcha=12500;break;
		case 10:
		default:maxcha=200;break;
	};

	switch(ch->perm_con)
	{
		case 11:maxcon=500;break;
		case 12:maxcon=750;break;
		case 13:maxcon=1000;break;
		case 14:maxcon=1500;break;
		case 15:maxcon=3100;break;
		case 16:maxcon=6600;break;
		case 17:case 18:maxcon=9500;break;
		case 10:
		default:maxcon=200;break;
	};

	pstr=tempstr/maxstr*100;
	pwis = tempwis/maxwis * 100;
	pint = tempint/maxint * 100;
	pcon = tempcon/maxcon * 100;
	pdex = tempdex/maxdex * 100;
	pcha = tempcha/maxcha * 100;
        ch_printf(ch, "&CStr&R:&B(&W%d%%&B) &CDex&R:&B(&W%d%%&B) &CCon&R:&B(&W%d%%&B) &CInt&R:&B(&W%d%%&B) ",
                pstr, pdex, pcon, pint);

        ch_printf(ch, "&CWis&R:&B(&W%d%%&B) &CCha&R:&B(&W%d%%&B) \r\n", pwis, pcha);

        return;
}

/* Allows player to let people into thier home, added by Funf */
void do_visitors(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *location;

	location = ch->in_room;

	if ( ch->plr_home )
	{
		if ( ch->plr_home->vnum == location->vnum )
		{
			TOGGLE_BIT( location->room_flags2, ROOM_VISITORS );
			if ( location->room_flags2 & ROOM_VISITORS )
			{
				send_to_char("Visitors are now allowed in your home.\r\n", ch);
			}
			else
			{
				send_to_char("Visitors are now no longer allowed in your home.\r\n", ch);
			}
			return;
		}

		send_to_char("You can only use this in your home!\r\n", ch);
		return;
	}

	send_to_char("You must own a home to use this command!\r\n", ch);
}


void do_spendqp( CHAR_DATA *ch, char *argument ) /* Take a load off of imms - Funf */
{
/************************************************************************************
 * 1   qp - Change a character's height or weight or age							*
 * 5   qp - Have an immortal raise a stat point under 15 by one (DEX, STR, etc.)	*
 * 5   qp - Have an immortal tell you a stat, such as hitroll, damroll, AC.			*
 * 5   qp - Have an immortal tell you what your current levels are.					*
 * 10  qp - Have an immortal raise a stat point over 15 but less than 17 by one		*
 * 15  qp - Have an immortal raise a stat point from 17 to 18						*
 * 30  qp - Change sex																*
 * 100 qp - +1 luck																	*
 ************************************************************************************/
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument( argument , arg1 );
    argument = one_argument( argument , arg2 );
    argument = one_argument( argument , arg3 );

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

    if ( NOT_AUTHED(ch) )
    {
      send_to_char("&CYou can not spend qp until after you've graduated from the academy.&W\n\r", ch);
      return;
    }

    if ( arg1[0] == '\0' )
    {
       send_to_char( "&CUsage&R: &WSPENDQP &R<&WGAIN&R|&WSHOW&R|&WCHANGE&R> &C<&Wstat&C>&W &C[&Wvalue&C]&W\n\r", ch );
       return;
    }

    if ( !str_prefix( arg1 , "gain" ) )
    {
		if ( arg2[0] == '\0' )
		{
		    send_to_char( "&COptions&R: &R<&Woutlaw&R|&Wstr&R|&Wdex&R|&Wcon&R|&Wint&R|&Wwis&R|&Wcha&R|&Wlck&R>&W\n\r", ch );
		    return;
    	}

		if ( !str_prefix( arg2 , "outlaw" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 2)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(IS_SET(ch->pcdata->flags, PCFLAG_OUTLAW)) {
			   send_to_char( "&CYou are already an outlaw!\n\r", ch);
			   return;
		   }

		   SET_BIT(ch->pcdata->flags, PCFLAG_OUTLAW);
	       ch->pcdata->quest_curr -= 2;

	       send_to_char( "&CYou are now wanted as an outlaw!&W\n\r", ch );
	       return;
    	}

		if ( !str_prefix( arg2 , "lck" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 100)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(ch->mod_lck >= 25) {
			   send_to_char( "&CYou can not gain anymore luck.\n\r", ch);
			   return;
		   }

		   ch->perm_lck += 1;
	       ch->pcdata->quest_curr -= 100;

	       send_to_char( "&CYou gain one point of luck.&W\n\r", ch );
	       return;
    	}

    	if ( !str_prefix( arg2 , "str" ) )
	    {
			int cost, train;
			switch(ch->perm_str)
			{
				case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				case 10: train = 250; cost = 5; break;
				case 11: train = 400; cost = 5; break;
				case 12: train = 600; cost = 5; break;
				case 13: train = 850; cost = 5; break;
				case 14: train = 1100; cost = 5; break;
				case 15: train = 4600; cost = 10; break;
				case 16: train = 9100; cost = 10; break;
				case 17: train = 12100; cost = 15; break;
				default: cost = 0; train = ch->strtrain; break;
			}

	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(ch->mod_str >= 25 || ch->perm_str >= 18) {
			   send_to_char( "&CYou can not gain anymore of that.\n\r", ch);
			   return;
		   }

		   ch->perm_str += 1;
		   ch->strtrain = train;
	       ch->pcdata->quest_curr -= cost;

	       send_to_char( "&CYou gain one point of strength.&W\n\r", ch );
	       return;
	    }

	    if ( !str_prefix( arg2 , "dex" ) )
	    {
			int cost, train;
			switch(ch->perm_dex)
			{
				case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				case 10: train = 550; cost = 5; break;
				case 11: train = 900; cost = 5; break;
				case 12: train = 1600; cost = 5; break;
				case 13: train = 2350; cost = 5; break;
				case 14: train = 5750; cost = 5; break;
				case 15: train = 10600; cost = 10; break;
				case 16: train = 13200; cost = 10; break;
				case 17: train = 16400; cost = 15; break;
				default: cost = 0; train = ch->dextrain; break;
			}

	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(ch->mod_dex >= 25 || ch->perm_dex >= 18) {
			   send_to_char( "&CYou can not gain anymore of that.\n\r", ch);
			   return;
		   }

		   ch->perm_dex += 1;
		   ch->dextrain = train;
	       ch->pcdata->quest_curr -= cost;

	       send_to_char( "&CYou gain one point of dexterity.&W\n\r", ch );
	       return;
	    }

	    if ( !str_prefix( arg2 , "con" ) )
	    {
		    int cost, train;
			switch(ch->perm_con)
			{
				case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				case 10: train = 300; cost = 5; break;
				case 11: train = 600; cost = 5; break;
				case 12: train = 850; cost = 5; break;
				case 13: train = 1100; cost = 5; break;
				case 14: train = 1600; cost = 5; break;
				case 15: train = 3200; cost = 10; break;
				case 16: train = 6700; cost = 10; break;
				case 17: train = 9600; cost = 15; break;
				default: cost = 0; train = ch->contrain; break;
			}
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(ch->mod_con >= 25 || ch->perm_con >= 18) {
			   send_to_char( "&CYou can not gain anymore of that.\n\r", ch);
			   return;
		   }

		   ch->perm_con += 1;
		   ch->contrain = train;
	       ch->pcdata->quest_curr -= cost;

	       send_to_char( "&CYou gain one point of constitution.&W\n\r", ch );
	       return;
	    }

	    if ( !str_prefix( arg2 , "int" ) )
	    {
			int cost, train;
			switch(ch->perm_int)
			{
				case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				case 10: train = 270; cost = 5; break;
				case 11: train = 350; cost = 5; break;
				case 12: train = 500; cost = 5; break;
				case 13: train = 650; cost = 5; break;
				case 14: train = 800; cost = 5; break;
				case 15: train = 1850; cost = 10; break;
				case 16: train = 3200; cost = 10; break;
				case 17: train = 5600; cost = 15; break;
				default: cost = 0; train = ch->inttrain; break;
			}

	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }
	       if(ch->mod_int >= 25 || ch->perm_int >= 18) {
			   send_to_char( "&CYou can not gain anymore of that.\n\r", ch);
			   return;
		   }

		   ch->perm_int += 1;
		   ch->inttrain = train;
	       ch->pcdata->quest_curr -= cost;

	       send_to_char( "&CYou gain one point of intellegence.&W\n\r", ch );
	       return;
	    }

	    if ( !str_prefix( arg2 , "wis" ) )
	    {
			int cost, train;
			switch(ch->perm_wis)
			{
				case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				case 10: train = 100; cost = 5; break;
				case 11: train = 200; cost = 5; break;
				case 12: train = 350; cost = 5; break;
				case 13: train = 550; cost = 5; break;
				case 14: train = 750; cost = 5; break;
				case 15: train = 1000; cost = 10; break;
				case 16: train = 4300; cost = 10; break;
				case 17: train = 6600; cost = 15; break;
				default: cost = 0; train = ch->wistrain; break;
			}

	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(ch->mod_wis >= 25 || ch->perm_wis >= 18) {
			   send_to_char( "&CYou can not gain anymore of that.\n\r", ch);
			   return;
		   }

		   ch->perm_wis += 1;
		   ch->wistrain = train;
	       ch->pcdata->quest_curr -= cost;

	       send_to_char( "&CYou gain one point of wisdom.&W\n\r", ch );
	       return;
	    }

	    if ( !str_prefix( arg2 , "cha" ) )
	    {
			int cost, train;
			switch(ch->perm_cha)
			{
				case 3:case 4:case 5:case 6:case 7:case 8:case 9:
				case 10: train = 300; cost = 5; break;
				case 11: train = 500; cost = 5; break;
				case 12: train = 700; cost = 5; break;
				case 13: train = 1000; cost = 5; break;
				case 14: train = 2350; cost = 5; break;
				case 15: train = 4800; cost = 10; break;
				case 16: train = 7200; cost = 10; break;
				case 17: train = 12600; cost = 15; break;
				default: cost = 0; train = ch->chatrain; break;
			}

	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if(ch->mod_cha >= 25 || ch->perm_cha >= 18) {
			   send_to_char( "&CYou can not gain anymore of that.\n\r", ch);
			   return;
		   }

		   ch->perm_cha += 1;
		   ch->chatrain = train;
	       ch->pcdata->quest_curr -= cost;

	       send_to_char( "&CYou gain one point of charisma.&W\n\r", ch );
	       return;
	   }
/*
	    if ( !str_prefix( arg2 , "training" ) )
	    {
		   int cost = 1;

		   if ( arg3[0] == '\0' )
		   {
		     send_to_char( "&COptions&R: &R<&Wstr&R|&Wdex&R|&Wcon&R|&Wint&R|&Wwis&R|&Wcha&R>&W\n\r", ch );
		     return;
    	   }

	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < cost)
	       {
		  	 send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	 return;
	       }

		   if ( !str_prefix( arg3 , "str" ) )
		   {
		     ch->strtrain += 5000;
		     ch->pcdata->quest_curr -= cost;
		     send_to_char( "&CYou gain 5000 training points in strength.&W\n\r", ch );
		     return;
	   	   }

		   if ( !str_prefix( arg3 , "dex" ) )
		   {
		     ch->dextrain += 5000;
		     ch->pcdata->quest_curr -= cost;
		     send_to_char( "&CYou gain 5000 training points in dexterity.&W\n\r", ch );
		     return;
	   	   }

		   if ( !str_prefix( arg3 , "con" ) )
		   {
		     ch->contrain += 5000;
		     ch->pcdata->quest_curr -= cost;
		     send_to_char( "&CYou gain 5000 training points in constitution.&W\n\r", ch );
		     return;
	   	   }

		   if ( !str_prefix( arg3 , "int" ) )
		   {
		     ch->inttrain += 5000;
		     ch->pcdata->quest_curr -= cost;
		     send_to_char( "&CYou gain 5000 training points in intelligence.&W\n\r", ch );
		     return;
	   	   }

		   if ( !str_prefix( arg3 , "wis" ) )
		   {
		     ch->wistrain += 5000;
		     ch->pcdata->quest_curr -= cost;
		     send_to_char( "&CYou gain 5000 training points in wisdom.&W\n\r", ch );
		     return;
	   	   }

		   if ( !str_prefix( arg3 , "cha" ) )
		   {
		     ch->chatrain += 5000;
		     ch->pcdata->quest_curr -= cost;
		     send_to_char( "&CYou gain 5000 training points in charisma.&W\n\r", ch );
		     return;
	   	   }

	   	   send_to_char( "&COptions&R: &R<&Wstr&R|&Wdex&R|&Wcon&R|&Wint&R|&Wwis&R|&Wcha&R>&W\n\r", ch );
	       return;
	   }*/

       send_to_char( "&COptions&R: &R<&Woutlaw&R|&Wstr&R|&Wdex&R|&Wcon&R|&Wint&R|&Wwis&R|&Wcha&R|&Wlck&R>&W\n\r", ch );
       return;

	}
	else if ( !str_prefix( arg1 , "show" ) )
	{

		if ( arg2[0] == '\0' )
		{
		    send_to_char( "&COptions&R: &R<&Whitroll&R|&Wdamroll&R|&Wac&R|&Wlevels&R>&W\n\r", ch );
		    return;
    	}


		if ( !str_prefix( arg2 , "hitroll" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 5)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       ch->pcdata->quest_curr -= 5;

	       ch_printf( ch , "&CYou currently have %d hitroll.&W\n\r", ch->hitroll);
	       return;
	    }

		if ( !str_prefix( arg2 , "damroll" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 5)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       ch->pcdata->quest_curr -= 5;

	       ch_printf( ch , "&CYou currently have %d damroll.&W\n\r", ch->damroll);
	       return;
    	}

		if ( !str_prefix( arg2 , "ac" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 5)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       ch->pcdata->quest_curr -= 5;

	       ch_printf( ch , "&CYou currently have %d ac.&W\n\r", ch->armor);
	       return;
	    }

	    if ( !str_prefix( arg2 , "levels" ) )
	    {
		       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 5)
		       {
			  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
			  	return;
		       }

		       ch->pcdata->quest_curr -= 5;

			if ( !IS_NPC( ch ) )
		    {
		       int ability;

		       for ( ability = 0 ; ability < MAX_ABILITY ; ability++ )
		            ch_printf( ch, "%-15s   Level: %-3d   Max: %-3d   Exp: %-10ld   Next: %-10ld\n\r",
		    	        ability_name[ability], ch->skill_level[ability], max_level(ch, ability), ch->experience[ability],
		    	        exp_level( ch->skill_level[ability]+1 ) );
		    }
		    return;
		}

		send_to_char( "&COptions&R: &R<&Whitroll&R|&Wdamroll&R|&Wac&R|&Wlevels&R>&W\n\r", ch );
		return;
	}
	else if ( !str_prefix( arg1 , "change" ) )
	{

		if ( arg2[0] == '\0' || arg3 == '\0' )
		{
		    send_to_char( "&COptions&R: &R<&Wheight&R|&Wweight&R|&Wage&R|&Wsex&R>&W\n\r", ch );
		    return;
    	}

		if ( !str_prefix( arg2 , "height" ) )
	    {
		       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 1)
		       {
			  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
			  	return;
		       }

		       if (arg3[0] != '\0' )
		        value = atoi(arg3);

		       if ( value < 100 || value > 200)
		       {
		          send_to_char( "&CSorry, that isn't in the height range. Range is 100 to 200 cm.\n\r", ch);
		          return;
		       }

			   ch->pcheight = value;
		       ch->pcdata->quest_curr -= 1;

		       ch_printf( ch , "&CYou change your height to %d cm.&W\n\r", value);
		       return;
    	}

    	if ( !str_prefix( arg2 , "weight" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 1)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if (arg3[0] != '\0' )
	        value = atoi(arg3);

	       if ( value < 50 || value > 150)
	       {
	          send_to_char( "&CSorry, that isn't in the weight range. Range is 50 to 150 kg.\n\r", ch);
	          return;
	       }

		   ch->pcweight = value;
	       ch->pcdata->quest_curr -= 1;

	       ch_printf( ch , "&CYou change your weight to %d kg.&W\n\r", value);
	       return;
    	}

    	if ( !str_prefix( arg2 , "age" ) )
		{
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 1)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if (arg3[0] != '\0' )
	        value = atoi(arg3);

	       if ( value < 14 || value > 60)
	       {
	          send_to_char( "&CSorry, that isn't in the age range. Range is 14 to 60 years old.\n\r", ch);
	          return;
	       }

		   ch->pcage = value;
	       ch->pcdata->quest_curr -= 1;

	       ch_printf( ch , "&CYou change your age to %d.&W\n\r", value);
	       return;
    	}

    	if ( !str_prefix( arg2 , "sex" ) )
	    {
	       if(!IS_NPC(ch) && ch->pcdata->quest_curr < 30)
	       {
		  	send_to_char("&RYou don't have enough QP for that!\r\n", ch);
		  	return;
	       }

	       if ( !strcmp( arg3, "male" ) )
	       {
			   if ( ch->sex == 1 ) {
				   send_to_char( "&CWhat's the point in that?\n\r", ch);
			   } else {
				   send_to_char( "&CYou change your sex to male.&W\n\r", ch);
			   }
			   ch->pcdata->quest_curr -= 30;
			   ch->sex = 1;
			   return;
	       } else if ( !strcmp( arg3, "female" ) ) {
			   if ( ch->sex == 2 ) {
		 		   send_to_char( "&CWhat's the point in that?\n\r", ch);
			   } else {
			   	   send_to_char( "&CYou change your sex to female.&W\n\r", ch);
			   }
			   ch->pcdata->quest_curr -= 30;
			   ch->sex = 2;
			   return;
		   } else if ( !strcmp( arg3, "neutral" ) ) {
			   if ( ch->sex == 0 ) {
	 			   send_to_char( "&CWhat's the point in that?\n\r", ch);
		       } else {
			   	   send_to_char( "&CYou change your sex to neutral.&W\n\r", ch);
			   }
			   ch->pcdata->quest_curr -= 30;
			   ch->sex = 0;
			   return;
		   } else {
			   send_to_char( "&CPossible choices are male, female, and neutral.&W\n\r", ch);
		   }

	       return;
    	}

    	send_to_char( "&COptions&R: &R<&Wheight&R|&Wweight&R|&Wage&R|&Wsex&R>&W\n\r", ch );
		return;
	}


    do_spendqp( ch , "" );
    return;
}

