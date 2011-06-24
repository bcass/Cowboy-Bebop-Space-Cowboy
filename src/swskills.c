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
*		   New Star Wars Skills Unit    			   *
****************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void    add_reinforcements  args( ( CHAR_DATA *ch ) );
ch_ret  one_hit             args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int     xp_compute                ( CHAR_DATA *ch , CHAR_DATA *victim );
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit );
int ris_save( CHAR_DATA *ch, int chance, int ris );
CHAR_DATA *get_char_room_mp( CHAR_DATA *ch, char *argument );
char *strip_color(char *str);
void do_whois( CHAR_DATA *ch, char *argument );
void do_who( CHAR_DATA *ch, char *argument );
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb );
void place_bounty args ( ( CHAR_DATA *ch , CHAR_DATA *victim , long long int amount, int flags ) );
void do_rembounty(  CHAR_DATA *ch, char *argument );
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument );
bool    is_same_group       args( ( CHAR_DATA *ach, CHAR_DATA *bch ));
ROOM_INDEX_DATA * find_dock ( char * dockname );

extern int      top_affect;
CHAR_DATA *             first_char;
CHAR_DATA *             last_char;

void do_makeknife( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, bonus;
    bool checktool, checkdura, checkplas, checkoven, checkbatt;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:


    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makeknife <name>\n\r&w", ch);
                  return;
                }

		if( is_profane(arg) )
		{
			send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
			send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
			return;
		}
    	        checktool = FALSE;
                checkdura = FALSE;
                checkplas = FALSE;
                checkoven = FALSE;
				checkbatt = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DURASTEEL)
          	    	checkdura = TRUE;
                  if (obj->item_type == ITEM_DURAPLAST)
          	    	checkplas = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need a toolkit to make a knife.\n\r", ch);
                   return;
                }

                if ( !checkdura )
                {
                   send_to_char( "&RYou need some steel.\n\r", ch);
                   return;
                }

                if ( !checkplas )
                {
                   send_to_char( "&RYou need some plastic.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat the metal.\n\r", ch);
                   return;
                }
                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery to power the oven.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makeknife]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of crafting a knife.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 25 , do_makeknife , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makeknife );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeknife]);
    vnum = 10422;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkdura = FALSE;
    checkplas = FALSE;
    checkoven = FALSE;
    checkbatt = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_DURAPLAST && checkplas == FALSE )
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkplas = TRUE;
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeknife]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdura ) || ( !checkplas ) || ( !checkoven ) )
    {
       send_to_char( "&RYou finish putting together your newly created knife.\n\r", ch);
       send_to_char( "&RIt looks okay for a moment but then breaks apart into a dozen pieces.\n\r", ch);
       learn_from_failure( ch, gsn_makeknife );
       return;
    }

	if( is_master_glm(ch)
		&& number_percent() < (get_curr_int(ch) >= 18 ? 3 : (get_curr_int(ch) >= 15 ? 2 : 1) )
		)
		bonus = 2;
	else
		bonus = 0;

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 3;
    STRFREE( obj->name );
    strcpy( buf, arg );
    strcat( buf, " knife" );
    if (bonus) strcat( buf, " masterwork" );
    obj->name = STRALLOC(strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was left here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = INIT_WEAPON_CONDITION;
    obj->value[1] = (int) (level/50+bonus);      /* min dmg  */
    obj->value[2] = (int) (level/10+2+bonus);      /* max dmg */
    obj->value[3] = WEAPON_KNIFE;
    obj->value[4] = 0;
    obj->value[5] = 0;
    obj->cost = obj->value[2]*10;
    obj->weight = obj->value[2]/2;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created knife.&w\n\r", ch);
    if (bonus)
    	send_to_char( "&YYou've exceeded superb craftsmanship. You've made a masterpiece!&w\n\r", ch);
    act( AT_PLAIN, "$n finishes crafting a knife.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*200 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
        {
                xpgain = xpgain * 1.1;
        }

	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }

    learn_from_success( ch, gsn_makeknife );
}

void do_makegun( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, bonus;
    bool checktool, checkdura, checkbatt, checkoven, checkmetal, checkplas;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, power, scope;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:

    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makegun <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkmetal = FALSE;
                checkplas = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
		if(strlen_color(arg) > 75)
		{
		   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
		   return;
	        }
                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DURASTEEL)
          	    	checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_DURAPLAST)
                    checkplas = TRUE;
                  if (obj->item_type == ITEM_RARE_METAL)
                    checkmetal = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a gun.\n\r", ch);
                   return;
                }

                if ( !checkdura )
                {
                   send_to_char( "&RYou need some steel to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery to power the oven.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat the materials.\n\r", ch);
                   return;
                }

                if ( !checkplas )
                {
                   send_to_char( "&RYou need plastic for the handle.\n\r", ch);
                   return;
                }

                if ( !checkmetal )
                {
                   send_to_char( "&RYou need a rare metal for those fine components.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makegun]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a gun.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 25 , do_makegun , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makegun );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makegun]);
    vnum = 10420;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool 	= FALSE;
    checkdura 	= FALSE;
    checkbatt 	= FALSE;
    checkoven 	= FALSE;
    checkplas 	= FALSE;
    checkmetal 	= FALSE;
    power     	= 0;
    scope     	= 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_DURAPLAST && checkplas == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkplas = TRUE;
       }
       if (obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkmetal = TRUE;
       }
 	}

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makegun]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )  || ( !checkplas ) || ( !checkmetal) )
    {
       send_to_char( "&RYou hold up your new gun and aim at a leftover piece of plastic.\n\r", ch);
       send_to_char( "&RYou slowly squeeze the trigger hoping for the best...\n\r", ch);
       send_to_char( "&RYour gun backfires destroying your weapon and burning your hand.\n\r", ch);
       learn_from_failure( ch, gsn_makegun );
       return;
    }

	if( is_master_glm(ch)
		&& number_percent() < (get_curr_int(ch) >= 18 ? 3 : (get_curr_int(ch) >= 15 ? 2 : 1) )
		)
		bonus = 2;
	else
		bonus = 0;

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 2+level/40;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " pistol");
    if (bonus) strcat( buf, " masterwork" );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = INIT_WEAPON_CONDITION;       /* condition  */
    obj->value[1] = (int) (level/50+4+bonus);      /* min dmg  */
    obj->value[2] = (int) (level/10+8+bonus);      /* max dmg  */
    obj->value[3] = WEAPON_PISTOL;
    obj->value[4] = 0;
    obj->value[5] = 20;
    obj->cost = obj->value[2]*50;
    obj->weight = obj->value[2]/2;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created gun.&w\n\r", ch);
    if (bonus)
    	send_to_char( "&YYou've exceeded superb craftsmanship. You've made a masterpiece!&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new gun.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

        if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
    learn_from_success( ch, gsn_makegun );
}

void do_makesword( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance, bonus;
    bool checktool, checkdura, checkbatt, checkoven, checkplas;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, level;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:

    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makesword <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkplas = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }

                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DURASTEEL)
          	    	checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_DURAPLAST)
                    checkplas = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a sword.\n\r", ch);
                   return;
                }

                if ( !checkdura )
                {
                   send_to_char( "&RYou need some steel to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery to power the oven.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small oven to heat and shape the components.\n\r", ch);
                   return;
                }

                if ( !checkplas )
                {
                   send_to_char( "&RYou need some plastic for the hilt.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makesword]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of crafting a sword.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch, NULL, argument , TO_ROOM );
		   	   add_timer ( ch , TIMER_DO_FUN , 25 , do_makesword , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makesword );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makesword]);
    vnum = 10421;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkdura = FALSE;
    checkbatt = FALSE;
    checkoven = FALSE;
    checkplas = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_DURAPLAST && checkplas == FALSE)
       {
          checkplas = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makesword]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) || ( !checkplas ) )

    {
       send_to_char( "&RYou finish putting together your newly created sword.\n\r", ch);
       send_to_char( "&RIt looks okay for a moment but then breaks apart into a dozen pieces.\n\r", ch);
       learn_from_failure( ch, gsn_makesword );
       return;
    }

	if( is_master_glm(ch)
		&& number_percent() < (get_curr_int(ch) >= 18 ? 3 : (get_curr_int(ch) >= 15 ? 2 : 1) )
		)
		bonus = 2;
	else
		bonus = 0;

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 5;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " sword");
    if (bonus) strcat( buf, " masterwork" );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = INIT_WEAPON_CONDITION;       /* condition  */
    obj->value[1] = (int) (level/25+bonus);      /* min dmg  */
    obj->value[2] = (int) (level/25+10+bonus);      /* max dmg */
    obj->value[3] = WEAPON_SWORD;
    obj->value[4] = 0;
    obj->value[5] = 0;
    obj->cost = obj->value[2]*75;
    obj->weight = obj->value[2]/2;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created sword.&w\n\r", ch);
    if (bonus)
    	send_to_char( "&YYou've exceeded superb craftsmanship. You've made a masterpiece!&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new sword.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

         if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makesword );
}

// Just used as a 'null' for skills
void do_makespice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance;
    OBJ_DATA *obj;

    send_to_char("Huh?",ch);
    return;

    switch( ch->substate )
    {
    	default:
    	        strcpy( arg, argument );

    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RFrom what?\n\r&w", ch);
                  return;
                }

    	        if ( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
                {
                   send_to_char( "&RYou need to be in a refinery to create drugs from spice.\n\r", ch);
                   return;
                }

                if ( ms_find_obj(ch) )
                      return;

                if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
                {
                        send_to_char( "&RYou are not holding anything!\n\r&w", ch );
                        return;
                }

                if ( obj->item_type != ITEM_RAWSPICE )
                {
                       send_to_char( "&RYou can't make a drug out of that\n\r&w",ch);
                       return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_spice_refining]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of refining spice into a drug.\n\r", ch);
    		   act( AT_PLAIN, "$n begins working on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makespice , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do with the stuff.\n\r",ch);
	        learn_from_failure( ch, gsn_spice_refining );
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
    	        send_to_char("&RYou are distracted and are unable to finish your work.\n\r&w", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
           send_to_char( "You seem to have lost your spice!\n\r", ch );
           return;
    }
    if ( obj->item_type != ITEM_RAWSPICE )
    {
          send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w",ch);
          return;
    }

    obj->value[1] = URANGE (10, obj->value[1], ( IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_spice_refining]) ) +10);
    strcpy( buf, obj->name );
    STRFREE( obj->name );
    strcat( buf, " drug spice" );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, "a drug made from " );
    strcat( buf, obj->short_descr );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );
    obj->description = STRALLOC( buf );
    obj->item_type = ITEM_SPICE;

    send_to_char( "&GYou finish your work.\n\r", ch);
    act( AT_PLAIN, "$n finishes $s work.", ch,
         NULL, argument , TO_ROOM );

    obj->cost += obj->value[1]*10;
    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );
         gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }

    learn_from_success( ch, gsn_spice_refining );

}
// Drug Code - Bloody Eye
void do_makebloodyeye( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance,level, timer;
    OBJ_DATA *obj;


    switch( ch->substate )
    {
        default:
                strcpy( arg, argument );


                if ( arg[0] == '\0' )
                {
                  send_to_char( "&RFrom what?\n\r&w", ch);
                  return;
                }

                if ( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
                {
                   send_to_char( "&RYou need to be in a refinery to create drugs.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                if ( ms_find_obj(ch) )
                      return;

               if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
                {
                        send_to_char( "&RYou are not holding anything!\n\r&w", ch );
                        return;
                }

                if ( obj->item_type != ITEM_RAWSPICE )
                {
                       send_to_char( "&RYou can't make a drug out of that.\n\r&w",ch);
                       return;
                }

                chance = IS_NPC(ch) ? ch->top_level
			: (int) (ch->pcdata->learned[gsn_makebloodyeye]);
		if(ch->pcdata->learned[gsn_chemistry])
		{
			timer = 20;
			learn_from_success( ch, gsn_chemistry );
		}
		else
			timer = 30;

	        if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of creating the drug Bloody Eye.\n\r", ch);
                   act( AT_PLAIN, "$n begins working on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makebloodyeye , 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }
                send_to_char("&RYou can't figure out what to do with the stuff.\n\r",ch);
                learn_from_failure( ch, gsn_makebloodyeye );
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
                send_to_char("&RYou are distracted and are unable to finish your work.\n\r&w", ch);
                return;
    }

    ch->substate = SUB_NONE;

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
           send_to_char( "You seem to have lost your materials!\n\r", ch );
           return;
    }
    if ( obj->item_type != ITEM_RAWSPICE )
    {
          send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w",ch);
          return;
    }

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makebloodyeye]);

    //strcpy( buf, obj->name );
    STRFREE( obj->name );
    //strcat( buf, " drug bloody eye" );
    strcpy(buf, "drug vial bloody eye");
    obj->name = STRALLOC( buf );
    strcpy( buf, "A Vial of Bloody Eye" );
    /*
    strcat( buf, obj->short_descr );
    */
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );

    obj->description = STRALLOC( buf );
    obj->value[0] = 0;
    obj->value[1] = level/20 - 1;
    obj->item_type = ITEM_SPICE;

    send_to_char( "&GYou finish your work.\n\r", ch);
    act( AT_PLAIN, "$n finishes $s work.", ch,
         NULL, argument , TO_ROOM );

    obj->cost += obj->value[1]*100000;
    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) - exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
         xpgain = xpgain/2;
	 gain_exp(ch, xpgain, CHEMIST_ABILITY);
         ch_printf( ch , "You gain %d chemist experience.", xpgain );

    }

    learn_from_success( ch, gsn_makebloodyeye );
}

// Yellow Eye
void do_makeyelloweye( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance,level, timer;
    OBJ_DATA *obj;


    switch( ch->substate )
    {
        default:
                strcpy( arg, argument );

                if ( arg[0] == '\0' )
                {
                  send_to_char( "&RFrom what?\n\r&w", ch);
                  return;
                }

                if ( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
                {
                   send_to_char( "&RYou need to be in a refinery to create drugs.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                if ( ms_find_obj(ch) )
                      return;

                if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
                {
                        send_to_char( "&RYou are not holding anything!\n\r&w", ch );
                        return;
                }

                if ( obj->item_type != ITEM_RAWSPICE )
                {
                       send_to_char( "&RYou can't make a drug out of that.\n\r&w",ch);
                       return;
                }

                chance = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeyelloweye]);

		if(ch->pcdata->learned[gsn_chemistry])
                {
		     timer = 20;
		     learn_from_success( ch, gsn_chemistry );
        	}
	        else
                     timer = 30;

                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of creating the drug Yellow Eye.\n\r", ch);
                   act( AT_PLAIN, "$n begins working on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makeyelloweye , 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }
                send_to_char("&RYou can't figure out what to do with the stuff.\n\r",ch);
                learn_from_failure( ch, gsn_makeyelloweye );
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
                send_to_char("&RYou are distracted and are unable to finish your work.\n\r&w", ch);
                return;
    }

    ch->substate = SUB_NONE;

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
           send_to_char( "You seem to have lost your materials!\n\r", ch );
           return;
    }
    if ( obj->item_type != ITEM_RAWSPICE )
    {
          send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w",ch);
          return;
    }

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeyelloweye]);

    //strcpy( buf, obj->name );
    STRFREE( obj->name );
    //strcat( buf, " drug yellow eye" );
    strcpy(buf, "drug vial yellow eye");
    obj->name = STRALLOC( buf );
    strcpy( buf, "A Vial of Yellow Eye" );
    /*
    strcat( buf, obj->short_descr );
    */
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );

    obj->description = STRALLOC( buf );
    obj->value[0] = 1;
    obj->value[1] = level/20 - 1;
    obj->item_type = ITEM_SPICE;

    send_to_char( "&GYou finish your work.\n\r", ch);
    act( AT_PLAIN, "$n finishes $s work.", ch,
         NULL, argument , TO_ROOM );

    obj->cost += obj->value[1]*100000;
    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) -
exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
         xpgain = xpgain/2;
	 gain_exp(ch, xpgain, CHEMIST_ABILITY);
         ch_printf( ch , "You gain %d chemist experience.", xpgain );

    }

    learn_from_success( ch, gsn_makeyelloweye );
}

// Purple Eye
void do_makepurpleeye( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance,level, timer;
    OBJ_DATA *obj;


    switch( ch->substate )
    {
        default:
                strcpy( arg, argument );

                if ( arg[0] == '\0' )
                {
                  send_to_char( "&RFrom what?\n\r&w", ch);
                  return;
                }

                if ( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
                {
                   send_to_char( "&RYou need to be in a refinery to create drugs.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                if ( ms_find_obj(ch) )
                      return;

                if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
                {
                        send_to_char( "&RYou are not holding anything!\n\r&w", ch );
                        return;
                }

                if ( obj->item_type != ITEM_RAWSPICE )
                {
                       send_to_char( "&RYou can't make a drug out of that.\n\r&w",ch);
                       return;
                }

                chance = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makepurpleeye]);
		if(ch->pcdata->learned[gsn_chemistry])
                {
		     timer = 20;
		     learn_from_success( ch, gsn_chemistry );
        	}
	        else
                     timer = 30;

                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of creating the drug Purple Eye.\n\r", ch);
                   act( AT_PLAIN, "$n begins working on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makepurpleeye , 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }
                send_to_char("&RYou can't figure out what to do with the stuff.\n\r",ch);
                learn_from_failure( ch, gsn_makepurpleeye );
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
                send_to_char("&RYou are distracted and are unable to finish your work.\n\r&w", ch);
                return;
    }

    ch->substate = SUB_NONE;

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
           send_to_char( "You seem to have lost your materials!\n\r", ch );
           return;
    }
    if ( obj->item_type != ITEM_RAWSPICE )
    {
          send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w",ch);
          return;
    }

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makepurpleeye]);

    //strcpy( buf, obj->name );
    STRFREE( obj->name );
    //strcat( buf, " drug purple eye" );
    strcpy(buf, "drug vial purple eye");
    obj->name = STRALLOC( buf );
    strcpy( buf, "A Vial of Purple Eye" );
    /*
    strcat( buf, obj->short_descr );
    */
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );

    obj->description = STRALLOC( buf );
    obj->value[0] = 2;
    obj->value[1] = level/20 - 1;
    obj->item_type = ITEM_SPICE;

    send_to_char( "&GYou finish your work.\n\r", ch);
    act( AT_PLAIN, "$n finishes $s work.", ch,
         NULL, argument , TO_ROOM );

    obj->cost += obj->value[1]*100000;
    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) - exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
	 xpgain = xpgain/2;
         gain_exp(ch, xpgain, CHEMIST_ABILITY);
         ch_printf( ch , "You gain %d chemist experience.", xpgain );

    }

    learn_from_success( ch, gsn_makepurpleeye );
}

// Blue Eye
void do_makeblueeye( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance,level, timer;
    OBJ_DATA *obj;

    switch( ch->substate )
    {
        default:
                strcpy( arg, argument );

                if ( arg[0] == '\0' )
                {
                  send_to_char( "&RFrom what?\n\r&w", ch);
                  return;
                }

                if ( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
                {
                   send_to_char( "&RYou need to be in a refinery to create drugs.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                if ( ms_find_obj(ch) )
                      return;

                if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
                {
                        send_to_char( "&RYou are not holding anything!\n\r&w", ch );
                        return;
                }

                if ( obj->item_type != ITEM_RAWSPICE )
                {
                       send_to_char( "&RYou can't make a drug out of that.\n\r&w",ch);
                       return;
                }

                chance = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makeblueeye]);

		if(ch->pcdata->learned[gsn_chemistry])
                {
		     timer = 20;
		     learn_from_success( ch, gsn_chemistry );
        	}
	        else
                     timer = 30;

                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of creating the drug Blue Eye.\n\r", ch);
                   act( AT_PLAIN, "$n begins working on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makeblueeye , 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }
                send_to_char("&RYou can't figure out what to do with the stuff.\n\r",ch);
                learn_from_failure( ch, gsn_makeblueeye );
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
                send_to_char("&RYou are distracted and are unable to finish your work.\n\r&w", ch);
                return;
    }

    ch->substate = SUB_NONE;

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
           send_to_char( "You seem to have lost your materials!\n\r", ch );
           return;
    }
    if ( obj->item_type != ITEM_RAWSPICE )
    {
          send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w",ch);
          return;
    }

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeblueeye]);

    //strcpy( buf, obj->name );
    STRFREE( obj->name );
    //strcat( buf, " drug blue eye" );
    strcpy(buf, "drug vial blue eye");
    obj->name = STRALLOC( buf );
    strcpy( buf, "A Vial of Blue Eye" );
    /*
    strcat( buf, obj->short_descr );
    */
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );

    obj->description = STRALLOC( buf );
    obj->value[0] = 3;
    obj->value[1] = level/20 - 1;
    obj->item_type = ITEM_SPICE;

    send_to_char( "&GYou finish your work.\n\r", ch);
    act( AT_PLAIN, "$n finishes $s work.", ch,
         NULL, argument , TO_ROOM );

    obj->cost += obj->value[1]*100000;
    {
         long long int xpgain;
         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) - exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
         xpgain = xpgain/2;
	 gain_exp(ch, xpgain, CHEMIST_ABILITY);
         ch_printf( ch , "You gain %d chemist experience.", xpgain );

    }

    learn_from_success( ch, gsn_makeblueeye );
}

// Green Eye
void do_makegreeneye( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance,level, timer;
    OBJ_DATA *obj;


    switch( ch->substate )
    {
        default:
                strcpy( arg, argument );


                if ( arg[0] == '\0' )
                {
                  send_to_char( "&RFrom what?\n\r&w", ch);
                  return;
                }

                if ( !IS_SET( ch->in_room->room_flags, ROOM_REFINERY ) )
                {
                   send_to_char( "&RYou need to be in a refinery to create drugs.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                if ( ms_find_obj(ch) )
                      return;

                if ( ( obj = get_eq_char(ch, WEAR_HOLD ) ) == NULL )
                {
                        send_to_char( "&RYou are not holding anything!\n\r&w", ch );
                        return;
                }

                if ( obj->item_type != ITEM_RAWSPICE )
                {
                       send_to_char( "&RYou can't make a drug out of that.\n\r&w",ch);
                       return;
                }

                chance = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_makegreeneye]);

		if(ch->pcdata->learned[gsn_chemistry])
                {
		     timer = 20;
		     learn_from_success( ch, gsn_chemistry );
        	}
	        else
                     timer = 30;

                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of creating the drug Green Eye.\n\r", ch);
                   act( AT_PLAIN, "$n begins working on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makegreeneye, 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }
                send_to_char("&RYou can't figure out what to do with the stuff.\n\r",ch);
                learn_from_failure( ch, gsn_makegreeneye );
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
                send_to_char("&RYou are distracted and are unable to finish your work.\n\r&w", ch);
                return;
    }

    ch->substate = SUB_NONE;

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
           send_to_char( "You seem to have lost your materials!\n\r", ch );
           return;
    }
    if ( obj->item_type != ITEM_RAWSPICE )
    {
          send_to_char( "&RYou get your tools mixed up and can't finish your work.\n\r&w",ch);
          return;
    }

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makegreeneye]);

    //strcpy( buf, obj->name );
    STRFREE( obj->name );
    //strcat( buf, " drug green eye" );
    strcpy(buf, "drug vial green eye");
    obj->name = STRALLOC( buf );
    strcpy( buf, "A Vial of Green Eye" );
    /*
    strcat( buf, obj->short_descr );
    */
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );

    obj->description = STRALLOC( buf );
    obj->value[0] = 4;
    obj->value[1] = level/20 - 1;
    obj->item_type = ITEM_SPICE;

    send_to_char( "&GYou finish your work.\n\r", ch);
    act( AT_PLAIN, "$n finishes $s work.", ch,
         NULL, argument , TO_ROOM );

    obj->cost += obj->value[1]*100000;
    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) - exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
       	 xpgain = xpgain/2;
 	 gain_exp(ch, xpgain, CHEMIST_ABILITY);
         ch_printf( ch , "You gain %d chemist experience.", xpgain );

    }

    learn_from_success( ch, gsn_makegreeneye );
}


void do_makeapplicator( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checktool, checkdura, checkbatt, checkoven, checkmetal, checkplas;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, power, scope, timer;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;


    switch( ch->substate )
    {
        default:
		strcpy( arg, argument );

                checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkmetal = FALSE;
                checkplas = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DURASTEEL)
                        checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_DURAPLAST)
                    checkplas = TRUE;
                  if (obj->item_type == ITEM_RARE_METAL)
                    checkmetal = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a applicator.\n\r", ch);
                   return;
                }

                if ( !checkdura )
                {
                   send_to_char( "&RYou need some steel to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery to power the oven.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat the materials.\n\r", ch);
                   return;
                }

                if ( !checkplas )
                {
                   send_to_char( "&RYou need plastic for the handle.\n\r", ch);
                   return;
                }

                if ( !checkmetal )
                {
                   send_to_char( "&RYou need a rare metal for those fine components.\n\r", ch);
                   return;
                }

                if(ch->pcdata->learned[gsn_chemistry])
                {
                     timer = 10;
                     learn_from_success( ch, gsn_chemistry );
                }
                else
                     timer = 20;

                chance = IS_NPC(ch) ? ch->top_level
                         : (int) (ch->pcdata->learned[gsn_makeapplicator]);
                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of making an applicator.\n\r", ch);
                   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makeapplicator , 1 );
                   ch->dest_buf = str_dup(arg);
		   return;
                }
                send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
                learn_from_failure( ch, gsn_makeapplicator );
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
                send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
                return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeapplicator]);
    vnum = 4391;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database. Please inform the administratiors.\n\r",ch);
         return;
    }

    checktool   = FALSE;
    checkdura   = FALSE;
    checkbatt   = FALSE;
    checkoven   = FALSE;
    checkplas   = FALSE;
    checkmetal  = FALSE;
    power       = 0;
    scope       = 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_DURAPLAST && checkplas == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkplas = TRUE;
       }
       if (obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkmetal = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makeapplicator]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )  || ( !checkplas) || (!checkmetal) )
    {
       send_to_char( "&RYou hold up your new applicator and put it to your eye.\n\r", ch);
       send_to_char( "&RYou slowly squeeze the button and hope it works...\n\r", ch);
       send_to_char( "&RYour applicator explodes and pieces of metal get in your eyes.\n\r", ch);
       learn_from_failure( ch, gsn_makeapplicator );
       return;
    }

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_APPLICATOR;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 2+level/10;
    strcpy( buf , "Applicator");
    STRFREE( obj->name);
    strcat( buf , "tool");
    obj->name = STRALLOC( buf );
    strcpy( buf, "An Applicator" );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->cost = level*100;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created applicator.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new applicator.", ch,
         NULL, argument , TO_ROOM );

    {
	 if ( ch->skill_level[CHEMIST_ABILITY] < 50)
	 {
         	long long int xpgain;
         	xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) - exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
	 	xpgain = xpgain/3;
         	gain_exp(ch, xpgain, CHEMIST_ABILITY);
         	ch_printf( ch , "You gain %d chemist experience.", xpgain );
    	 }
	 else
		send_to_char("Your level is too high to gain experience from this skill.\n\r", ch);
    }
    learn_from_success( ch, gsn_makeapplicator );
}

void do_none( CHAR_DATA *ch, char *argument )
{
	send_to_char("Huh?",ch);
	return;
}

void do_makerawdrug( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, strength, weight, timer;
    bool checktool, checkdrink, checkbatt, checkchem, checkoven;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;


    strcpy( arg , argument );

    switch( ch->substate )
    {
        default:

                if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makerawdrug <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

                checktool  = FALSE;
                checkdrink = FALSE;
                checkbatt  = FALSE;
                checkchem  = FALSE;
                checkoven  = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
                    checkdrink = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_CHEMICAL)
                    checkchem = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a raw drug.\n\r", ch);
                   return;
                }

                if ( !checkdrink )
                {
                   send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery for the oven.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need a oven to heat the materials.\n\r", ch);
                   return;
                }

                if ( !checkchem )
                {
                   send_to_char( "&RYou need chemicals to make raw drugs!\n\r", ch);
                   return;
                }

                chance = IS_NPC(ch) ? ch->top_level
                         : (int) (ch->pcdata->learned[gsn_makerawdrug]);

                if(ch->pcdata->learned[gsn_chemistry])
                {
                        timer = 15;
                        learn_from_success( ch, gsn_chemistry );
                }
                else
                        timer = 20;



                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin the long process of making a raw drug.\n\r", ch);
                   act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , timer , do_makerawdrug , 1 );
                   ch->dest_buf   = str_dup(arg);
                   return;
                }
                send_to_char("&RYou can't figure out how to mix the drug correctly.\n\r",ch);
                learn_from_failure( ch, gsn_makerawdrug );
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
                send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
                return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makerawdrug]);
    vnum = 4390;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administratiors.\n\r", ch);
         return;
    }

    checktool = FALSE;
    checkdrink = FALSE;
    checkbatt = FALSE;
    checkchem = FALSE;
    checkoven = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE && obj->value[1] == 0 )
       {
          checkdrink = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkdrink = TRUE;
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_CHEMICAL)
       {
          strength = URANGE( 10, obj->value[0], level * 5 );
          weight = obj->weight;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkchem = TRUE;
       }
       if (obj->item_type == ITEM_OVEN && checkoven == FALSE)
          checkoven = TRUE;
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makerawdrug]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem ) || ( !checkoven))
    {
       send_to_char( "&RJust as you are about to finish your work,\n\ryou drop your raw drug ruining it.\n\r", ch);
       learn_from_failure( ch, gsn_makerawdrug );
       return;
    }

    obj = create_object( pObjIndex, level );

    SET_BIT( obj->wear_flags, ITEM_HOLD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );

    strcpy( buf, obj->name );
    STRFREE( obj->name );
    strcat( buf, " raw drug " );
    strcat( buf, argument ); /* append user's name too, Funf */
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, "{Raw Drug} " );
    strcat( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    strcat( buf, " was foolishly left lying around here." );
    STRFREE( obj->description );
    obj->description = STRALLOC( buf );
    obj->item_type = ITEM_RAWSPICE;

    obj->cost = 100000;
    SET_BIT( obj->extra_flags , ITEM_CONTRABAND);
    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and gaze at your new raw drug.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new raw drug.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

	 if ( ch->skill_level[CHEMIST_ABILITY] < 50)
	 {
         	 xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[CHEMIST_ABILITY]+1) - exp_level(ch->skill_level[CHEMIST_ABILITY]) ) );
         	xpgain = xpgain/4;
	 	gain_exp(ch, xpgain, CHEMIST_ABILITY);
         	ch_printf( ch , "You gain %d chemist experience.", xpgain );
         }
	 else
        	 send_to_char("Your level is too high to gain experience for this skill.\n\r" ,ch);
    }
        learn_from_success( ch, gsn_makerawdrug );
}



void do_makegrenade( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, strength, weight;
    bool checktool, checkdrink, checkbatt, checkchem, checkcirc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:


    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makegrenade <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool  = FALSE;
                checkdrink = FALSE;
                checkbatt  = FALSE;
                checkchem  = FALSE;
                checkcirc  = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
          	    checkdrink = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                    checkcirc = TRUE;
                  if (obj->item_type == ITEM_CHEMICAL)
                    checkchem = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a grenade.\n\r", ch);
                   return;
                }

                if ( !checkdrink )
                {
                   send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a small battery for the timer.\n\r", ch);
                   return;
                }

                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit for the timer.\n\r", ch);
                   return;
                }

                if ( !checkchem )
                {
                   send_to_char( "&RSome explosive chemicals would come in handy!\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makegrenade]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a grenade.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 25 , do_makegrenade , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makegrenade );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makegrenade]);
    vnum = 10425;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkdrink = FALSE;
    checkbatt = FALSE;
    checkchem = FALSE;
    checkcirc = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE && obj->value[1] == 0 )
       {
          checkdrink = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_CHEMICAL)
       {
          strength = URANGE( 10, obj->value[0], level * 5 );
          weight = obj->weight;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkchem = TRUE;
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcirc = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makegrenade]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem ) || ( !checkcirc) )
    {
       send_to_char( "&RJust as you are about to finish your work,\n\ryour newly created grenade explodes in your hands...doh!\n\r", ch);
       learn_from_failure( ch, gsn_makegrenade );
       return;
    }

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_GRENADE;
    SET_BIT( obj->wear_flags, ITEM_HOLD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = weight;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " grenade");
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = strength/2;
    obj->value[1] = strength;
    obj->cost = obj->value[1]*5;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created grenade.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new grenade.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makegrenade );
}

void do_makelandmine( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, strength, weight;
    bool checktool, checkdrink, checkbatt, checkchem, checkcirc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:

    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makelandmine <name>\n\r&w", ch);
                  return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool  = FALSE;
                checkdrink = FALSE;
                checkbatt  = FALSE;
                checkchem  = FALSE;
                checkcirc  = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DRINK_CON && obj->value[1] == 0 )
          	    checkdrink = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                    checkcirc = TRUE;
                  if (obj->item_type == ITEM_CHEMICAL)
                    checkchem = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a landmine.\n\r", ch);
                   return;
                }

                if ( !checkdrink )
                {
                   send_to_char( "&RYou will need an empty drink container to mix and hold the chemicals.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a small battery for the detonator.\n\r", ch);
                   return;
                }

                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit for the detonator.\n\r", ch);
                   return;
                }

                if ( !checkchem )
                {
                   send_to_char( "&RSome explosive chemicals would come in handy!\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makelandmine]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a landmine.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a drink container and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 25 , do_makelandmine , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makelandmine );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makelandmine]);
    vnum = 10427;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkdrink = FALSE;
    checkbatt = FALSE;
    checkchem = FALSE;
    checkcirc = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_DRINK_CON && checkdrink == FALSE && obj->value[1] == 0 )
       {
          checkdrink = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_CHEMICAL)
       {
          strength = URANGE( 10, obj->value[0], level * 5 );
          weight = obj->weight;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkchem = TRUE;
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcirc = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makelandmine]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdrink ) || ( !checkbatt ) || ( !checkchem ) || ( !checkcirc) )
    {
       send_to_char( "&RJust as you are about to finish your work,\n\ryour newly created landmine explodes in your hands...doh!\n\r", ch);
       learn_from_failure( ch, gsn_makelandmine );
       return;
    }

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_LANDMINE;
    SET_BIT( obj->wear_flags, ITEM_HOLD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = weight;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " landmine");
    obj->name = STRALLOC(strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = strength/2;
    obj->value[1] = strength;
    obj->cost = obj->value[1]*5;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created landmine.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new landmine.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makelandmine );
}
void do_makelight( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, strength;
    bool checktool, checkbatt, checkchem, checkcirc, checklens;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:


    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makeflashlight <name>\n\r&w", ch);
                  return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool  = FALSE;
                checkbatt  = FALSE;
                checkchem  = FALSE;
                checkcirc  = FALSE;
                checklens = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                    checkcirc = TRUE;
                  if (obj->item_type == ITEM_CHEMICAL)
                    checkchem = TRUE;
                  if (obj->item_type == ITEM_LENS)
                    checklens = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a light.\n\r", ch);
                   return;
                }

                if ( !checklens )
                {
                   send_to_char( "&RYou need a lens to make a light.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery for the light to work.\n\r", ch);
                   return;
                }

                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit.\n\r", ch);
                   return;
                }

                if ( !checkchem )
                {
                   send_to_char( "&RSome chemicals to light would come in handy!\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makelight]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a light.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makelight , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makelight );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makelight]);
    vnum = 10428;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checklens = FALSE;
    checkbatt = FALSE;
    checkchem = FALSE;
    checkcirc = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          strength = obj->value[0];
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_CHEMICAL)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkchem = TRUE;
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcirc = TRUE;
       }
       if (obj->item_type == ITEM_LENS && checklens == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checklens = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makelight]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checklens ) || ( !checkbatt ) || ( !checkchem ) || ( !checkcirc) )
    {
       send_to_char( "&RJust as you are about to finish your work,\n\ryour newly created light explodes in your hands...doh!\n\r", ch);
       learn_from_failure( ch, gsn_makelight );
       return;
    }

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_LIGHT;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 3;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " light");
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[2] = strength;
    obj->cost = obj->value[2];

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created light.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new light.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*100 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makelight );
}

void do_makejewelry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checktool, checkoven, checkmetal, checkbatt;
    OBJ_DATA *obj;
    OBJ_DATA *metal;
    int value, cost;


    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);

    if ( !str_cmp( arg, "body" )
    || !str_cmp( arg, "head" )
    || !str_cmp( arg, "legs" )
    || !str_cmp( arg, "arms" )
    || !str_cmp( arg, "about" )
    || !str_cmp( arg, "waist" )
    || !str_cmp( arg, "hold" )
    || !str_cmp( arg, "feet" )
    || !str_cmp( arg, "hands" )
    || !str_cmp( arg, "back") )
    {
        send_to_char( "&RYou cannot make jewelry for that body part.\n\r&w", ch);
        send_to_char( "&RTry MAKEARMOR.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "shield" ) )
    {
        send_to_char( "&RYou cannot make jewelry worn as a shield.\n\r&w", ch);
        send_to_char( "&RTry MAKESHIELD.\n\r&w", ch);
        return;
    }
/* Let's just be nice...*sighs*
    if ( !str_cmp( arg, "eyes" ) )
	{
	        send_to_char( "&RYou cannot make jewelry worn as on the eyes.\n\r&w", ch);
	        return;
    }
*/
    if( !str_cmp( arg, "hip") )
    {
	send_to_char("&RYou cannot make jewelry for the hip!\n\r&w", ch);
	send_to_char("&RTry MAKECONTAINER.\n\r", ch);
	return;
    }
    if ( !str_cmp( arg, "wield" ) )
    {
        send_to_char( "&RAre you going to fight with your jewelry?\n\r&w", ch);
        send_to_char( "&RTry MAKEKNIFE...\n\r&w", ch);
        return;
    }

    switch( ch->substate )
    {
    	default:


    	        if ( arg2[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makejewelry <wearloc> <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg2) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool = FALSE;
                checkoven = FALSE;
		checkbatt = FALSE;
                checkmetal = FALSE;
                if(strlen_color(arg2) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_OVEN)
          	    checkoven = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_RARE_METAL)
          	    checkmetal = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need a toolkit.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need an oven.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery to power the oven.\n\r", ch);
                   return;
                }

                if ( !checkmetal )
                {
                   send_to_char( "&RYou need some precious metal.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makejewelry]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of creating some jewelry.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s toolkit and some metal and begins to work.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , chance == 100 ? 8 : 15 , do_makejewelry , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do.\n\r",ch);
	        learn_from_failure( ch, gsn_makejewelry );
    	   	return;

    	case 1:
    		if ( !ch->dest_buf )
    		     return;
    		if ( !ch->dest_buf_2 )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		strcpy(arg2, ch->dest_buf_2);
    		DISPOSE( ch->dest_buf_2);
    		break;

    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		DISPOSE( ch->dest_buf_2 );
    		ch->substate = SUB_NONE;
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makejewelry]);

    checkmetal = FALSE;
    checkoven = FALSE;
    checkbatt = FALSE;
    checktool = FALSE;
    value=0;
    cost=0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE)
       {
          checkmetal = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          metal = obj;
       }
       if (obj->item_type == ITEM_CRYSTAL)
       {
          cost += obj->cost;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makejewelry]) ;

    if ( number_percent( ) > chance*2  || ( !checkoven ) || ( !checktool ) || ( !checkmetal ) || (!checkbatt) )
    {
       send_to_char( "&RYou hold up your newly created jewelry.\n\r", ch);
       send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch);
       send_to_char( "&Rpiece of junk you've ever seen. You quickly hide your mistake...\n\r", ch);
       learn_from_failure( ch, gsn_makejewelry );
       return;
    }

    obj = metal;
	int ac;

    obj->item_type = ITEM_ARMOR;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_WEAR_NECK );
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was dropped here." );
    obj->description = STRALLOC( buf );
    ac = ( (int) ( obj->value[1] * ( ch->pcdata->learned[gsn_makejewelry]) / 100 ) );
    ac -= 3;
    if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	ac += 2;
    if ( ac < 1 )
    	ac = 1;
    obj->value[0] = ac;
    obj->value[1] = ac;
    obj->cost *= 10;
    obj->cost += cost;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created jewelry.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making  some new jewelry.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*100 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makejewelry );

}

void do_makearmor( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checksew, checkfab;
    OBJ_DATA *obj;
    OBJ_DATA *material;
    int value;


    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);

    if ( !str_cmp( arg, "ears" )
    || !str_cmp( arg, "finger" )
    || !str_cmp( arg, "neck" )
    || !str_cmp( arg, "wrist" )
    || !str_cmp( arg, "hip")  )
    {
        send_to_char( "&RYou cannot make clothing for that body part.\n\r&w", ch);
        send_to_char( "&RTry MAKEJEWELRY.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "shield" ) )
    {
        send_to_char( "&RYou cannot make clothing worn as a shield.\n\r&w", ch);
        send_to_char( "&RTry MAKESHIELD.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "eyes" ) )
    {
        send_to_char( "&RYou cannot make clothing worn on the eyes.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "wield" ) )
    {
        send_to_char( "&RAre you going to fight with your clothing?\n\r&w", ch);
        send_to_char( "&RTry MAKEBLADE...\n\r&w", ch);
        return;
    }

    switch( ch->substate )
    {
    	default:


    	        if ( arg2[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makearmor <wearloc> <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg2) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checksew = FALSE;
                checkfab = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg2) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_FABRIC)
                    checkfab = TRUE;
                  if (obj->item_type == ITEM_THREAD)
          	    checksew = TRUE;
                }

                if ( !checkfab )
                {
                   send_to_char( "&RYou need some sort of fabric or material.\n\r", ch);
                   return;
                }

                if ( !checksew )
                {
                   send_to_char( "&RYou need a needle and some thread.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makearmor]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of creating some armor.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , (chance == 100) ? 8 : 15 , do_makearmor , 1 ); /* Adept = half time - Funf */
    		   ch->dest_buf = str_dup(arg);
    		   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do.\n\r",ch);
	        learn_from_failure( ch, gsn_makearmor );
    	   	return;

    	case 1:
    		if ( !ch->dest_buf )
    		     return;
    		if ( !ch->dest_buf_2 )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		strcpy(arg2, ch->dest_buf_2);
    		DISPOSE( ch->dest_buf_2);
    		break;

    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		DISPOSE( ch->dest_buf_2 );
    		ch->substate = SUB_NONE;
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makearmor]);

    checksew = FALSE;
    checkfab = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_THREAD)
          checksew = TRUE;
       if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
       {
          checkfab = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          material = obj;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makearmor]) ;

    if ( number_percent( ) > chance*2  || ( !checkfab ) || ( !checksew ) )
    {
       send_to_char( "&RYou hold up your newly created armor.\n\r", ch);
       send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch);
       send_to_char( "&Rgarment you've ever seen. You quickly hide your mistake...\n\r", ch);
       learn_from_failure( ch, gsn_makearmor );
       return;
    }

    obj = material;
	int ac;

    obj->item_type = ITEM_ARMOR;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_WEAR_BODY );
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was dropped here." );
    obj->description = STRALLOC( buf );
    ac = ( (int) ( obj->value[1] * ( ch->pcdata->learned[gsn_makearmor]) / 100 ) );
    ac -= 5;
    if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	ac += 2;
    if ( ac < 1 )
    	ac = 1;
    obj->value[0] = ac;
    obj->value[1] = ac;
    obj->cost *= 10;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created garment.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes sewing some new armor.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*100 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makearmor );
}


void do_makecomlink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance;
    bool checktool, checkgem, checkbatt, checkcirc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, value;

    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);

    if ( !str_cmp( arg, "finger" )
    || !str_cmp( arg, "hip")
    || !str_cmp( arg, "body")
    || !str_cmp( arg, "waist")
    || !str_cmp( arg, "ankle")
    || !str_cmp( arg, "back")
    || !str_cmp( arg, "hands")
    || !str_cmp( arg, "legs")
    || !str_cmp( arg, "feet")
    || !str_cmp( arg, "eyes")
    || !str_cmp( arg, "about")
    || !str_cmp( arg, "arms") )
    {
        send_to_char( "&RYou cannot make a comlink for that body part.\n\r&w", ch);
        send_to_char( "&RTry MAKEARMOR.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "shield" ) )
    {
        send_to_char( "&RYou cannot make a comlink worn as a shield.\n\r&w", ch);
        send_to_char( "&RTry MAKESHIELD.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "eyes" ) )
    {
        send_to_char( "&RYou cannot make a comlink worn on the eyes.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "wield" ) )
    {
        send_to_char( "&RAre you going to fight with your comlink?\n\r&w", ch);
        send_to_char( "&RTry MAKEBLADE...\n\r&w", ch);
        return;
    }

    //strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:


    	        if ( arg[0] == '\0' || arg2[0] == '\0')
                {
                  send_to_char( "&RUsage: Makecomlink <wearlocation> <name>\n\r&w", ch);
                  return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool = FALSE;
                checkgem  = FALSE;
                checkbatt = FALSE;
                checkcirc = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_CRYSTAL)
          	    checkgem = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                  checkbatt = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                  checkcirc = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a comlink.\n\r", ch);
                   return;
                }

                if ( !checkgem )
                {
                   send_to_char( "&RYou need a small crystal.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a power source for your comlink.\n\r", ch);
                   return;
                }

                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makecomlink]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a comlink.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makecomlink , 1 );
    		   //ch->dest_buf = str_dup(arg);
		   ch->dest_buf = str_dup(arg);
                   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makecomlink );
    	   	return;
/* Makecomlink now has more than one wearloc
    	case 1:
    		if ( !ch->dest_buf )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;

    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
*/
        case 1:
                if ( !ch->dest_buf )
                     return;
                if ( !ch->dest_buf_2 )
                     return;
                strcpy(arg, ch->dest_buf);
                DISPOSE( ch->dest_buf);
                strcpy(arg2, ch->dest_buf_2);
                DISPOSE( ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE( ch->dest_buf );
                DISPOSE( ch->dest_buf_2 );
                ch->substate = SUB_NONE;
                send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
                return;
    }

    ch->substate = SUB_NONE;

    vnum = 10430;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkgem  = FALSE;
    checkbatt = FALSE;
    checkcirc = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_CRYSTAL && checkgem == FALSE)
       {
          checkgem = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          checkcirc = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makecomlink]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkcirc ) || ( !checkbatt ) || ( !checkgem ) )
    {
       send_to_char( "&RYou hold up your newly created comlink....\n\r", ch);
       send_to_char( "&Rand it falls apart in your hands.\n\r", ch);
       learn_from_failure( ch, gsn_makecomlink );
       return;
    }

    obj = create_object( pObjIndex, ch->top_level );

    obj->item_type = ITEM_COMLINK;
    //SET_BIT( obj->wear_flags, ITEM_HOLD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_WEAR_BODY );
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->weight = 3;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    strcat( buf, " comlink" );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was left here." );
    obj->description = STRALLOC( buf );
    obj->cost = 50;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created comlink.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes crafting a comlink.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*100 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makecomlink );

}

void do_makeshield( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance;
    bool checktool, checkbatt, checkoven, checkdura;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, level;


    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:


    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makeshield <name>\n\r&w", ch);
                  return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkdura = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a workshop.\n\r", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_DURAPLAST)
                    checkdura = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make an shield.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a power source for the oven.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need an oven to heat the components.\n\r", ch);
                   return;
                }

                if ( !checkdura )
                {
                   send_to_char( "&RYou still need some plastic for your shield.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makeshield]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of crafting a shield.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 20 , do_makeshield , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makeshield );
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
    	        send_to_char("&RYou are interrupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeshield]);
    vnum = 10429;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkbatt = FALSE;
    checkoven = FALSE;
    checkdura = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;

       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;

       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_DURAPLAST && checkdura == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkdura = TRUE;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeshield]);

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkbatt )
                                       || ( !checkoven ) || ( !checkdura ) )

    {
       send_to_char( "&RYou hold up your new shield and flip it over hoping for the best.\n\r", ch);
       send_to_char( "&RThe shield falls apart and shatters into many pieces.\n\r", ch);
       learn_from_failure( ch, gsn_makeshield );
       return;
    }

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_ARMOR;
    SET_BIT( obj->wear_flags, ITEM_WEAR_SHIELD );
    obj->level = level;
    obj->weight = 2;
    STRFREE( obj->name );
    strcpy( buf, arg );
    strcat( buf, " shield" );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = (int) (level/5 - 5 );      /* condition */
    obj->value[1] = (int) (level/5 - 5);      /* armor */
    if(obj->value[0] < 1)
    {
	obj->value[0] = 1;
	obj->value[1] = 1;
    }
    if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
    {
	obj->value[0] += 2;
	obj->value[1] += 2;
    }
    obj->value[4] = 0;
    obj->value[5] = 0;
    obj->cost = obj->value[2]*100;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created shield.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new shield.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makeshield );

}

void do_makecontainer( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checksew, checkfab;
    OBJ_DATA *obj;
    OBJ_DATA *material;
    int value;


    argument = one_argument( argument, arg );
    strcpy( arg2 , argument );

    if ( !str_cmp( arg, "eyes" )
    || !str_cmp( arg, "ears" )
    || !str_cmp( arg, "finger" )
    || !str_cmp( arg, "neck" )
    || !str_cmp( arg, "wrist" ) )
    {
        send_to_char( "&RYou cannot make a container for that body part.\n\r&w", ch);
        send_to_char( "&RTry MAKEJEWELRY.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "feet" )
    || !str_cmp( arg, "hands" )
    || !str_cmp( arg, "face" ) )
   {
        send_to_char( "&RYou cannot make a container for that body part.\n\r&w", ch);
        send_to_char( "&RTry MAKEARMOR.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "shield" ) )
    {
        send_to_char( "&RYou cannot make a container a shield.\n\r&w", ch);
        send_to_char( "&RTry MAKESHIELD.\n\r&w", ch);
        return;
    }
    if ( !str_cmp( arg, "wield" ) )
    {
        send_to_char( "&RAre you going to fight with a container?\n\r&w", ch);
        send_to_char( "&RTry MAKEBLADE...\n\r&w", ch);
        return;
    }

    switch( ch->substate )
    {
    	default:


    	        if ( arg2[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makecontainer <wearloc> <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg2) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checksew = FALSE;
                checkfab = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg2) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_FABRIC)
                    checkfab = TRUE;
                  if (obj->item_type == ITEM_THREAD)
          	    checksew = TRUE;
                }

                if ( !checkfab )
                {
                   send_to_char( "&RYou need some sort of fabric or material.\n\r", ch);
                   return;
                }

                if ( !checksew )
                {
                   send_to_char( "&RYou need a needle and some thread.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makecontainer]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of creating a bag.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makecontainer , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do.\n\r",ch);
	        learn_from_failure( ch, gsn_makecontainer );
    	   	return;

    	case 1:
    		if ( !ch->dest_buf )
    		     return;
    		if ( !ch->dest_buf_2 )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		strcpy(arg2, ch->dest_buf_2);
    		DISPOSE( ch->dest_buf_2);
    		break;

    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		DISPOSE( ch->dest_buf_2 );
    		ch->substate = SUB_NONE;
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makecontainer]);

    checksew = FALSE;
    checkfab = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_THREAD)
          checksew = TRUE;
       if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
       {
          checkfab = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          material = obj;
       }
    }

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makecontainer]) ;

    if ( number_percent( ) > chance*2  || ( !checkfab ) || ( !checksew ) )
    {
       send_to_char( "&RYou hold up your newly created container.\n\r", ch);
       send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch);
       send_to_char( "&Rcontainer you've ever seen. You quickly hide your mistake...\n\r", ch);
       learn_from_failure( ch, gsn_makecontainer );
       return;
    }

    obj = material;

    obj->item_type = ITEM_CONTAINER;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_HOLD );
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was dropped here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = level;
    obj->value[1] = 0;
    obj->value[2] = 0;
    obj->value[3] = 10;
    obj->cost *= 2;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created container.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes sewing a new container.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*100 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
        learn_from_success( ch, gsn_makecontainer );
}

void do_makemissile( CHAR_DATA *ch, char *argument )
{
    /* don't think we really need this */
    send_to_char( "&RSorry, this skill isn't finished yet :(\n\r", ch);
}

void do_gemcutting( CHAR_DATA *ch, char *argument )
{
     send_to_char( "&RSorry, this skill isn't finished yet :(\n\r", ch);
}

void do_reinforcements( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    long long int credits;

    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYour reinforcements are already on the way.\n\r", ch );
    	            return;
    	        }

    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\n\r", ch );
    	            return;
    	        }

    	        if ( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 50 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough wulongs to send for reinforcements.\n\r" );
    	            return;
    	        }

    	        chance = (int) (ch->pcdata->learned[gsn_reinforcements]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_reinforcements , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for reinforcements but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_reinforcements );
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
    	        send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    send_to_char( "&GYour reinforcements are on the way.\n\r", ch);
    credits = ch->skill_level[LEADERSHIP_ABILITY] * 50;
    ch_printf( ch, "It cost you %lld wulongs.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_reinforcements );

    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_STORMTROOPER;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_TROOPER;
    else
       ch->backup_mob = MOB_VNUM_MERCINARY;

    ch->backup_wait = number_range(1,2);

}

void do_postguard( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    long long int credits;

    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYou already have backup coming.\n\r", ch );
    	            return;
    	        }

    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
    	            return;
    	        }

    	        if ( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 30 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough wulongs.\n\r",
ch );
    	            return;
    	        }

    	        chance = (int) (ch->pcdata->learned[gsn_postguard]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_postguard , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for a guard but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_postguard );
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
    	        send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    send_to_char( "&GYour guard is on the way.\n\r", ch);

    credits = ch->skill_level[LEADERSHIP_ABILITY] * 30;
    ch_printf( ch, "It cost you %lld wulongs.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_postguard );

    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_GUARD;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_GUARD;
    else
       ch->backup_mob = MOB_VNUM_BOUNCER;

    ch->backup_wait = 1;

}

void add_reinforcements( CHAR_DATA *ch )
{
     MOB_INDEX_DATA  * pMobIndex;
     OBJ_DATA        * rifle;
     OBJ_INDEX_DATA  * pObjIndex;

     if ( ( pMobIndex = get_mob_index( ch->backup_mob ) ) == NULL )
        return;

     if ( ch->backup_mob == MOB_VNUM_STORMTROOPER ||
          ch->backup_mob == MOB_VNUM_NR_TROOPER   ||
          ch->backup_mob == MOB_VNUM_MERCINARY       )
     {
        CHAR_DATA * mob[3];
        int         mob_cnt;

        send_to_char( "Your reinforcements have arrived.\n\r", ch );
        for ( mob_cnt = 0 ; mob_cnt < 3 ; mob_cnt++ )
        {
            int ability;
            mob[mob_cnt] = create_mobile( pMobIndex );
            char_to_room( mob[mob_cnt], ch->in_room );
            act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[mob_cnt], TO_ROOM );
            mob[mob_cnt]->top_level = ch->skill_level[LEADERSHIP_ABILITY]/3;
            for ( ability = 0 ; ability < MAX_ABILITY ; ability++ )
                     mob[mob_cnt]->skill_level[ability] = mob[mob_cnt]->top_level;
            mob[mob_cnt]->hit = mob[mob_cnt]->top_level*15;
            mob[mob_cnt]->max_hit = mob[mob_cnt]->hit;
            mob[mob_cnt]->armor = 100- mob[mob_cnt]->top_level*2.5;
            mob[mob_cnt]->damroll = mob[mob_cnt]->top_level/5;
            mob[mob_cnt]->hitroll = mob[mob_cnt]->top_level/5;
            if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
            {
                 rifle = create_object( pObjIndex, mob[mob_cnt]->top_level );
                 obj_to_char( rifle, mob[mob_cnt] );
                 equip_char( mob[mob_cnt], rifle, WEAR_WIELD );
            }
            if ( mob[mob_cnt]->master )
	       stop_follower( mob[mob_cnt] );
	    add_follower( mob[mob_cnt], ch );
            SET_BIT( mob[mob_cnt]->affected_by, AFF_CHARM );
        }
     }
     else
     {
        CHAR_DATA *mob;
        int ability;

        mob = create_mobile( pMobIndex );
        char_to_room( mob, ch->in_room );
        if ( ch->pcdata && ch->pcdata->clan )
        {
          char tmpbuf[MAX_STRING_LENGTH];

          STRFREE( mob->name );
          mob->name = STRALLOC( ch->pcdata->clan->name );
          sprintf( tmpbuf , "(%s) %s" , ch->pcdata->clan->name  , mob->long_descr );
          STRFREE( mob->long_descr );
          mob->long_descr = STRALLOC( tmpbuf );
        }
        act( AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM );
        send_to_char( "Your guard has arrived.\n\r", ch );
        mob->top_level = ch->skill_level[LEADERSHIP_ABILITY];
        for ( ability = 0 ; ability < MAX_ABILITY ; ability++ )
            mob->skill_level[ability] = mob->top_level;
        mob->hit = mob->top_level*15;
        mob->max_hit = mob->hit;
        mob->armor = 100- mob->top_level*2.5;
        mob->damroll = mob->top_level/5;
        mob->hitroll = mob->top_level/5;
        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTECH_E11 ) ) != NULL )
        {
            rifle = create_object( pObjIndex, mob->top_level );
            obj_to_char( rifle, mob );
            equip_char( mob, rifle, WEAR_WIELD );
        }

        /* for making this more accurate in the future */

        if ( mob->mob_clan )
           STRFREE ( mob->mob_clan );
        if ( ch->pcdata && ch->pcdata->clan )
           mob->mob_clan = STRALLOC( ch->pcdata->clan->name );
     }
}

void do_torture( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, dam;
    bool fail;

    if ( !IS_NPC(ch)
    &&  ch->pcdata->learned[gsn_torture] <= 0  )
    {
	send_to_char(
	    "Your mind races as you realize you have no idea how to do that.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't do that right now.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( ch->mount )
    {
	send_to_char( "You can't get close enough while mounted.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Torture whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Are you masacistic or what...\n\r", ch );
	return;
    }

    if ( !IS_AWAKE(victim) )
    {
	send_to_char( "You need to wake them first.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->fighting )
    {
	send_to_char( "You can't torture someone whos in combat.\n\r", ch );
	return;
    }

    ch->alignment = ch->alignment -= 100;
    ch->alignment = URANGE( -1000, ch->alignment, 1000 );

    WAIT_STATE( ch, skill_table[gsn_torture]->beats );

    fail = FALSE;
    chance = ris_save( victim, ch->skill_level[HUNTING_ABILITY], RIS_PARALYSIS );
    if ( chance == 1000 )
      fail = TRUE;
    else
      fail = saves_para_petri( chance, victim );

    if ( !IS_NPC(ch) && !IS_NPC(victim) )
      chance = sysdata.stun_plr_vs_plr;
    else
      chance = sysdata.stun_regular;
    if ( !fail
    && (  IS_NPC(ch)
    || (number_percent( ) + chance) < ch->pcdata->learned[gsn_torture] ) )
    {
	learn_from_success( ch, gsn_torture );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, PULSE_VIOLENCE );
	act( AT_SKILL, "$N slowly tortures you. The pain is excruciating.", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You torture $N, leaving $M screaming in pain.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n tortures $N, leaving $M screaming in agony!", ch, NULL, victim, TO_NOTVICT );

        dam = dice( ch->skill_level[HUNTING_ABILITY]/10 , 4 );
        dam = URANGE( 0, victim->max_hit-10, dam );
        victim->hit -= dam;
        victim->max_hit -= dam;

        ch_printf( victim, "You lose %d permanent hit points." ,dam);
        ch_printf( ch, "They lose %d permanent hit points." , dam);

    }
    else
    {
	act( AT_SKILL, "$N tries to cut off your finger!", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You mess up big time.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n tries to painfully torture $N.", ch, NULL, victim, TO_NOTVICT );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
    }
    return;

}

void do_disguise( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int chance;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Disguise yourself as what?\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
    ch->pcdata->isDisguised = FALSE;
	send_to_char( "Undisguised.\n\r", ch );
	return;
    }

    if ( strlen(argument) > 50 )
	argument[50] = '\0';

    chance = (int) (ch->pcdata->learned[gsn_disguise]);

    if ( number_percent( ) > chance )
    {
        send_to_char( "You try to disguise yourself but fail.\n\r", ch );
        return;
    }

    if ( isalpha(argument[0]) || isdigit(argument[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, argument );
    }
    else
	strcpy( buf, argument );

    STRFREE( ch->pcdata->disguise );
    ch->pcdata->disguise = STRALLOC( buf );
    send_to_char( "You successfully disguise yourself.\n\r", ch );
    learn_from_success( ch, gsn_disguise );
    ch->pcdata->isDisguised = TRUE;
}

void do_mine( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool shovel;
    sh_int move;

   if ( ch->pcdata->learned[gsn_mine] <= 0 )
    {
      ch_printf( ch, "You have no idea how to do that.\n\r" );
      return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "And what will you mine the room with?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
      if ( obj->item_type == ITEM_SHOVEL )
      {
	  shovel = TRUE;
	  break;
      }

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
    if ( !obj )
    {
        send_to_char( "You don't see on here.\n\r", ch );
        return;
    }

    separate_obj(obj);
    if ( obj->item_type != ITEM_LANDMINE )
    {
	act( AT_PLAIN, "That's not a landmine!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	act( AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR );
        return;
    }

    switch( ch->in_room->sector_type )
    {
	case SECT_CITY:
	case SECT_INSIDE:
	    send_to_char( "The floor is too hard to dig through.\n\r", ch );
	    return;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
	    send_to_char( "You cannot bury a mine in the water.\n\r", ch );
	    return;
	case SECT_AIR:
	    send_to_char( "What?  Bury a mine in the air?!\n\r", ch );
	    return;
    }

    if ( obj->weight > (UMAX(5, (can_carry_w(ch) / 10)))
    &&  !shovel )
    {
	send_to_char( "You'd need a shovel to bury something that big.\n\r", ch );
	return;
    }

    move = (obj->weight * 50 * (shovel ? 1 : 5)) / UMAX(1, can_carry_w(ch));
    move = URANGE( 2, move, 1000 );
    if ( move > ch->move )
    {
	send_to_char( "You don't have the energy to bury something of that size.\n\r", ch );
	return;
    }
    ch->move -= move;

    SET_BIT( obj->extra_flags, ITEM_BURRIED );
    WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );

    STRFREE ( obj->armed_by );
    obj->armed_by = STRALLOC ( ch->name );

    ch_printf( ch, "You arm and bury %s.\n\r", obj->short_descr );
    act( AT_PLAIN, "$n arms and buries $p.", ch, obj, NULL, TO_ROOM );

    learn_from_success( ch, gsn_mine );

    return;
}

void do_first_aid( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA   *medpac;
   CHAR_DATA  *victim;
   int         heal;
   char        buf[MAX_STRING_LENGTH];

   if ( ch->position == POS_FIGHTING )
   {
         send_to_char( "You can't do that while fighting!\n\r",ch );
         return;
   }

   medpac = get_eq_char( ch, WEAR_HOLD );
   if ( !medpac || medpac->item_type != ITEM_MEDPAC )
   {
         send_to_char( "You need to be holding a medpac.\n\r",ch );
         return;
   }

   if ( medpac->value[0] <= 0 )
   {
         send_to_char( "Your medpac seems to be empty.\n\r",ch );
         return;
   }

   if ( argument[0] == '\0' )
      victim = ch;
   else
      victim = get_char_room( ch, argument );

   if ( !victim )
   {
       ch_printf( ch, "I don't see any %s here...\n\r" , argument );
       return;
   }

   heal = number_range( 1, 150 );

   if ( heal > ch->pcdata->learned[gsn_first_aid]*2 )
   {
       ch_printf( ch, "You fail in your attempt at first aid.\n\r");
       learn_from_failure( ch , gsn_first_aid );
       return;
   }

   if ( victim == ch )
   {
       ch_printf( ch, "You tend to your wounds.\n\r");
       sprintf( buf , "$n uses %s to help heal $s wounds." , medpac->short_descr );
       act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );
   }
   else
   {
       sprintf( buf , "You tend to $N's wounds." );
       act( AT_ACTION, buf, ch, NULL, victim, TO_CHAR );
       sprintf( buf , "$n uses %s to help heal $N's wounds." , medpac->short_descr );
       act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );
       sprintf( buf , "$n uses %s to help heal your wounds." , medpac->short_descr );
       act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );
   }

   --medpac->value[0];
   victim->hit += URANGE ( 0, heal , victim->max_hit - victim->hit );

   learn_from_success( ch , gsn_first_aid );
}

void do_snipe( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA        * wield;
   char              arg[MAX_INPUT_LENGTH];
   char              arg2[MAX_INPUT_LENGTH];
   sh_int            dir, dist;
   sh_int            max_dist = 3;
   EXIT_DATA       * pexit;
   ROOM_INDEX_DATA * was_in_room;
   ROOM_INDEX_DATA * to_room;
   CHAR_DATA       * victim;
   int               chance;
   char              buf[MAX_STRING_LENGTH];
   bool              pfound = FALSE;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
	IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You'll have to do that elsewhere.\n\r", ch );
	return;
    }

   if ( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
   {
         send_to_char( "You can't do that while wielding two weapons.",ch );
         return;
   }

   wield = get_eq_char( ch, WEAR_WIELD );
   if ( !wield || wield->item_type != ITEM_WEAPON || wield->value[3] != WEAPON_RIFLE )
   {
         send_to_char( "You don't seem to be holding a rifle",ch );
         return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if ( ( dir = get_door( arg ) ) == -1 || arg2[0] == '\0' )
   {
     send_to_char( "Usage: snipe <dir> <target>\n\r", ch );
     return;
   }

   if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
   {
     send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
     return;
   }

   if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
   {
     send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
     return;
   }

   if ( wield->value[4] == 0)
   {
     WAIT_STATE( ch , 15 );
     send_to_char("&YClick!! Your rifle needs to be reloaded!\n\r",ch);
     return;
   }

   was_in_room = ch->in_room;

   for ( dist = 0; dist <= max_dist; dist++ )
   {
     if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
        break;

     if ( !pexit->to_room )
        break;

    to_room = NULL;
    if ( pexit->distance > 1 )
       to_room = generate_exit( ch->in_room , &pexit );

    if ( to_room == NULL )
       to_room = pexit->to_room;

     char_from_room( ch );
     char_to_room( ch, to_room );


     if ( IS_NPC(ch) && ( victim = get_char_room_mp( ch, arg2 ) ) != NULL )
     {
        pfound = TRUE;
        break;
     }
     else if ( !IS_NPC(ch) && ( victim = get_char_room( ch, arg2 ) ) != NULL )
     {
        pfound = TRUE;
        break;
     }


     if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
        break;

   }

   char_from_room( ch );
   char_to_room( ch, was_in_room );

   if ( !pfound )
   {
       ch_printf( ch, "You don't see that person to the %s!\n\r", dir_name[dir] );
       char_from_room( ch );
       char_to_room( ch, was_in_room );
       return;
   }

    if ( victim == ch )
    {
	send_to_char( "Shoot yourself ... really?\n\r", ch );
	return;
    }

    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You can't shoot them there.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PACIFIST) )
    {
	send_to_char("&BThey are too pacifistic to be sniped!\n\r" , ch);
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
    {
      send_to_char( "You feel too nice to do that!\n\r", ch );
      return;
    }

    chance = IS_NPC(ch) ? 100
           : (int)  (ch->pcdata->learned[gsn_snipe]) ;

    switch ( dir )
    {
        case 0:
        case 1:
           dir += 2;
           break;
        case 2:
        case 3:
           dir -= 2;
           break;
        case 4:
        case 7:
           dir += 1;
           break;
        case 5:
        case 8:
           dir -= 1;
           break;
        case 6:
           dir += 3;
           break;
        case 9:
           dir -=3;
           break;
    }

    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    WAIT_STATE( ch, 20 );
    if ( number_percent() < chance )
    {
       sprintf( buf , "A rifle shot fires at you from the %s." , dir_name[dir] );
       act( AT_ACTION, buf , victim, NULL, ch, TO_CHAR );
       act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );
       sprintf( buf, "A rifle shot fires at $N from the %s." , dir_name[dir] );
       act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );
   //  Bug fix, rifle took 2 bullets and not 1 - Gatz
   //    wield->value[4] -= 1;
       if(!IS_NPC(ch) && !IS_NPC(victim))
       {
       		if(ch->adren < 5)
		       ch->adren++;
	       if(victim->adren < 5)
		       ch->adren++;
       }
       one_hit( ch, victim, TYPE_UNDEFINED );

       if ( char_died(ch) )
          return;

       stop_fighting( ch , TRUE );

       learn_from_success( ch, gsn_snipe );
    }
    else
    {
       act( AT_ACTION, "You fire at $N but don't even come close.", ch, NULL, victim, TO_CHAR );
       sprintf( buf, "A rifle shot fired from the %s barely misses you." , dir_name[dir] );
       act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );
       learn_from_failure( ch, gsn_snipe );
       wield->value[4] -= 1;
    }

    char_from_room( ch );
    char_to_room( ch, was_in_room );
    make_shell( ch );
    act( AT_YELLOW, "You hear the sound of your empty rifle shell hit the ground.", ch, NULL, NULL, TO_CHAR);
    act( AT_YELLOW, "$n's rifle shell hits the ground with a loud ping.", ch, NULL, NULL, TO_ROOM);
    act( AT_PLAIN, "You lower your rifle and prepare for another shot.", ch, NULL, NULL, TO_CHAR);
    act( AT_PLAIN, "$n lowers $s rifle and prepares for another shot.", ch, ch, NULL, TO_ROOM);

   if ( IS_NPC(ch) )
      WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
   else
   {
   	if ( number_percent() < ch->pcdata->learned[gsn_third_attack] )
   	     WAIT_STATE( ch, 1 * PULSE_PER_SECOND );
   	else if ( number_percent() < ch->pcdata->learned[gsn_second_attack] )
   	     WAIT_STATE( ch, 2 * PULSE_PER_SECOND );
   	else
   	     WAIT_STATE( ch, 3 * PULSE_PER_SECOND );
   }
   if ( IS_NPC( victim ) && !char_died(victim) )
   {
      if ( IS_SET( victim->act , ACT_SENTINEL ) )
      {
         victim->was_sentinel = victim->in_room;
         REMOVE_BIT( victim->act, ACT_SENTINEL );
      }

      start_hating( victim , ch );
      start_hunting( victim, ch );

   }

}


/* syntax throw <obj> [direction] [target] */

void do_throw( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA        * obj;
   OBJ_DATA        * tmpobj;
   char              arg[MAX_INPUT_LENGTH];
   char              arg2[MAX_INPUT_LENGTH];
   char              arg3[MAX_INPUT_LENGTH];
   sh_int            dir;
   EXIT_DATA       * pexit;
   ROOM_INDEX_DATA * was_in_room;
   ROOM_INDEX_DATA * to_room;
   CHAR_DATA       * victim;
   char              buf[MAX_STRING_LENGTH];


   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   was_in_room = ch->in_room;

   if ( arg[0] == '\0' )
   {
     send_to_char( "Usage: throw <object> [direction] [target]\n\r", ch );
     return;
   }


   obj = get_eq_char( ch, WEAR_MISSILE_WIELD );
   if ( !obj || !nifty_is_name( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_HOLD );
      if ( !obj || !nifty_is_name( arg, obj->name ) )
          obj = get_eq_char( ch, WEAR_WIELD );
          if ( !obj || !nifty_is_name( arg, obj->name ) )
              obj = get_eq_char( ch, WEAR_DUAL_WIELD );
              if ( !obj || !nifty_is_name( arg, obj->name ) )
   if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_HOLD );
      if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
          obj = get_eq_char( ch, WEAR_WIELD );
          if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
              obj = get_eq_char( ch, WEAR_DUAL_WIELD );
              if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
   {
         ch_printf( ch, "You don't seem to be holding or wielding %s.\n\r", arg );
         return;
   }

    if ( IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
    {
	act( AT_PLAIN, "You can't throw $p.", ch, obj, NULL, TO_CHAR );
	return;
    }


   if ( ch->position == POS_FIGHTING )
   {
       victim = who_fighting( ch );
       if ( char_died ( victim ) )
           return;
       act( AT_ACTION, "You throw $p at $N.", ch, obj, victim, TO_CHAR );
       act( AT_ACTION, "$n throws $p at $N.", ch, obj, victim, TO_NOTVICT );
       act( AT_ACTION, "$n throw $p at you.", ch, obj, victim, TO_VICT );
   }
   else if ( arg2[0] == '\0' )
   {
       sprintf( buf, "$n throws %s at the floor." , obj->short_descr );
       act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
       ch_printf( ch, "You throw %s at the floor.\n\r", obj->short_descr );

       victim = NULL;
   }
   else  if ( ( dir = get_door( arg2 ) ) != -1 )
   {
      if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      {
         send_to_char( "Are you expecting to throw it through a wall!?\n\r", ch );
         return;
      }


      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
          send_to_char( "Are you expecting to throw it  through a door!?\n\r", ch );
          return;
      }


      switch ( dir )
      {
        case 0:
        case 1:
           dir += 2;
           break;
        case 2:
        case 3:
           dir -= 2;
           break;
        case 4:
        case 7:
           dir += 1;
           break;
        case 5:
        case 8:
           dir -= 1;
           break;
        case 6:
           dir += 3;
           break;
        case 9:
           dir -=3;
           break;
      }

      to_room = NULL;
      if ( pexit->distance > 1 )
       to_room = generate_exit( ch->in_room , &pexit );

      if ( to_room == NULL )
       to_room = pexit->to_room;


      char_from_room( ch );
      char_to_room( ch, to_room );

      victim = get_char_room( ch, arg3 );

      if ( victim )
      {
        if ( is_safe( ch, victim ) )
	{
		char_from_room( ch );
		char_to_room( ch, was_in_room);
		return;
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
        {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
        }

        if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
        {
        send_to_char( "You feel too nice to do that!\n\r", ch );
        return;
        }

        char_from_room( ch );
        char_to_room( ch, was_in_room );


        if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
	     IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) ||
	     IS_SET( victim->in_room->room_flags, ROOM_SAFE ) ||
             IS_SET( victim->in_room->room_flags, ROOM_PLR_HOME))
        {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You'll have to do that elsewhere.\n\r", ch );
	char_from_room(ch);
	char_to_room( ch, was_in_room);
	return;
        }

        to_room = NULL;
        if ( pexit->distance > 1 )
           to_room = generate_exit( ch->in_room , &pexit );

        if ( to_room == NULL )
           to_room = pexit->to_room;


        char_from_room( ch );
        char_to_room( ch, to_room );

        sprintf( buf , "Someone throws %s at you from the %s." , obj->short_descr , dir_name[dir] );
        act( AT_ACTION, buf , victim, NULL, ch, TO_CHAR );
        act( AT_ACTION, "You throw %p at $N.", ch, obj, victim, TO_CHAR );
        sprintf( buf, "%s is thrown at $N from the %s." , obj->short_descr , dir_name[dir] );
        act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );


      }
      else
      {
         ch_printf( ch, "You throw %s %s.\n\r", obj->short_descr , dir_name[get_dir( arg2 )] );
         sprintf( buf, "%s is thrown from the %s." , obj->short_descr , dir_name[dir] );
         act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );

      }
   }
   else if ( ( victim = get_char_room( ch, arg2 ) ) != NULL )
   {
        if ( is_safe( ch, victim ) )
	return;

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
        {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
        }

        if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
        {
        send_to_char( "You feel too nice to do that!\n\r", ch );
        return;
        }

   }
   else
   {
       ch_printf( ch, "They don't seem to be here!\n\r");
       return;
   }


   if ( obj == get_eq_char( ch, WEAR_WIELD )
   && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

   unequip_char( ch, obj );
   separate_obj( obj );
   obj_from_char( obj );
   obj = obj_to_room( obj, ch->in_room );

   if ( obj->item_type != ITEM_GRENADE )
       damage_obj ( obj );

/* NOT NEEDED UNLESS REFERING TO OBJECT AGAIN

   if( obj_extracted(obj) )
      return;
*/
   if ( ch->in_room !=  was_in_room )
   {
     char_from_room( ch );
     char_to_room( ch, was_in_room );
   }

   if ( !victim || char_died( victim ) )
       learn_from_failure( ch, gsn_throw );
   else
   {

       WAIT_STATE( ch, skill_table[gsn_throw]->beats );
       if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_throw] )
       {
	 	learn_from_success( ch, gsn_throw );
     	if ( ( obj->item_type == ITEM_WEAPON ) && ( obj->value[3] == 2 || obj->value[3] == 3 ) )
    		global_retcode = damage( ch, victim, ( number_range( obj->value[1] , obj->value[2] ) * 5 ), TYPE_HIT );
	 	else
	 		global_retcode = damage( ch, victim, ( number_range( obj->weight*2 , (obj->weight*2 + ch->perm_str) ) * 5 ), TYPE_HIT );
       }
       else
       {
	 learn_from_failure( ch, gsn_throw );
	 global_retcode = damage( ch, victim, 0, TYPE_HIT );
       }

       if ( IS_NPC( victim ) && !char_died ( victim) )
       {
          if ( IS_SET( victim->act , ACT_SENTINEL ) )
          {
             victim->was_sentinel = victim->in_room;
             REMOVE_BIT( victim->act, ACT_SENTINEL );
             start_hating( victim , ch );
             start_hunting( victim, ch );
		  }

       }

   }

   return;

}

void do_beg( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int percent;
    long long int amount, xp;

    if ( IS_NPC (ch) ) return;

    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Beg for money from whom?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( victim->position == POS_FIGHTING )
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

    if ( !IS_NPC( victim ) )
    {
	send_to_char( "You beg them for money.\n\r", ch );
	act( AT_ACTION, "$n begs you to give $s some change.\n\r", ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n begs $N for change.\n\r",  ch, NULL, victim, TO_NOTVICT );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_beg]->beats );
    percent  = number_percent( );

    if ( percent > ch->pcdata->learned[gsn_beg] + get_curr_cha(ch) - 10  )
    {
	/*
	 * Failure.
	 */
	send_to_char( "You beg them for money but don't get any!\n\r", ch );
	act( AT_ACTION, "$n is really getting on your nerves with all this begging!\n\r", ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n begs $N for money.\n\r",  ch, NULL, victim, TO_NOTVICT );

        if ( victim->alignment < 0 && victim->top_level >= ch->top_level+5 )
	{
	  sprintf( buf, "%s is an annoying beggar and needs to be taught a lesson!", ch->name );
	  do_yell( victim, buf );
      global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	}

	learn_from_failure( ch, gsn_beg );

	return;
    }


    act( AT_ACTION, "$n begs $N for money.\n\r",  ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "$n begs you for money!\n\r", ch, NULL, victim, TO_VICT    );

    amount = UMIN( victim->gold , number_range(1, 10) + get_curr_cha(ch) );
    if ( amount <= 0 )
    {
    do_look( victim , ch->name );
	do_say( victim , "Sorry I have nothing to spare." );
	learn_from_failure( ch, gsn_beg );
	return;
    }

	ch->gold     += amount;
	victim->gold -= amount;
	ch_printf( ch, "%s gives you %lld wulongs.\n\r", victim->short_descr, amount );
	learn_from_success( ch, gsn_beg );
	xp = UMIN( amount*10 , ( exp_level( ch->skill_level[SMUGGLING_ABILITY]+1) - exp_level( ch->skill_level[SMUGGLING_ABILITY])  )  );
        xp = UMIN( xp , xp_compute( ch, victim ) );
        gain_exp( ch, xp, SMUGGLING_ABILITY );
        ch_printf( ch, "&WYou gain %lld smuggling experience points!\n\r", xp );
        act( AT_ACTION, "$N gives $n some money.\n\r",  ch, NULL, victim, TO_NOTVICT );
        act( AT_ACTION, "You give $n some money.\n\r", ch, NULL, victim, TO_VICT    );

	return;

}

void do_pickshiplock( CHAR_DATA *ch, char *argument )
{
   do_pick( ch, argument );
}

void do_hijack( CHAR_DATA *ch, char *argument )
{
    int chance;
    SHIP_DATA *ship;
    char buf[MAX_STRING_LENGTH];

    	        if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }

    	        if ( ship->class > SHIP_PLATFORM )
    	        {
    	            send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	            return;
    	        }

    	        if ( (ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL )
    	        {
    	            send_to_char("&RYou don't seem to be in the pilot seat!\n\r",ch);
    	            return;
    	        }

    	        if ( strcmp(ship->owner,"Public") && check_pilot( ch , ship ) )
    	        {
    	            send_to_char("&RWhat would be the point of that!\n\r",ch);
    	            return;
    	        }

    	        if ( ship->type == MOB_SHIP && get_trust(ch) < 102 )
    	        {
    	            send_to_char("&RThis ship isn't pilotable by mortals at this point in time...\n\r",ch);
    	            return;
    	        }

                if  ( ship->class == SHIP_PLATFORM )
                {
                   send_to_char( "You can't do that here.\n\r" , ch );
                   return;
                }

    	        if ( ship->lastdoc != ship->location )
                {
                     send_to_char("&rYou don't seem to be docked right now.\n\r",ch);
                     return;
                }

    	        if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
    	        {
    	            send_to_char("The ship is not docked right now.\n\r",ch);
    	            return;
    	        }

                if ( ship->shipstate == SHIP_DISABLED )
    	        {
    	            send_to_char("The ships drive is disabled .\n\r",ch);
    	            return;
    	        }

				// BUGFIX: Keep people from racking up their bounty by always failing. 2009-03-20 Funf
				if (   ( ship->class == FIGHTER_SHIP && !ch->pcdata->learned[gsn_starfighters])
					|| ( ship->class == MIDSIZE_SHIP && !ch->pcdata->learned[gsn_midships])
					|| ( ship->class == CAPITAL_SHIP && !ch->pcdata->learned[gsn_capitalships]) )
				{
					send_to_char("You fail to work the controls properly!\n\r",ch);
					return;
				}

                chance = IS_NPC(ch) ? ch->top_level
 	                 : (int)  (ch->pcdata->learned[gsn_hijack]) - 8; // No particular reason for 8 - Funf
                 if ( number_percent( ) > chance )
 				{
     		        send_to_char("You fail to figure out the correct launch code.\n\r",ch);
 					send_to_char("&RThe authorities have been notified!&w\n\r",ch);
 					place_bounty( ch, ch, 25000, 0);
                     learn_from_failure( ch, gsn_hijack );
    	            return;
                }

    	        if ( ship->class == FIGHTER_SHIP )
                    chance = IS_NPC(ch) ? ch->top_level
	                 : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
                if ( ship->class == MIDSIZE_SHIP )
                    chance = IS_NPC(ch) ? ch->top_level
	                 : (int)  (ch->pcdata->learned[gsn_midships]) ;
                if ( ship->class == CAPITAL_SHIP )
                    chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_capitalships]);
                if ( number_percent( ) < chance )
    		{

    		   if (ship->hatchopen)
    		   {
    		     ship->hatchopen = FALSE;
    		     sprintf( buf , "The hatch on %s closes." , ship->name);
       	             echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
       	             echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch slides shut." );
       	           }
    		   set_char_color( AT_GREEN, ch );
    		   send_to_char( "Launch sequence initiated.\n\r", ch);
    		   act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch,
		        NULL, argument , TO_ROOM );
		   echo_to_ship( AT_YELLOW , ship , "The ship hums as it lifts off the ground.");
    		   sprintf( buf, "%s begins to launch.", ship->name );
    		   echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
    		   ship->shipstate = SHIP_LAUNCH;
    		   ship->currspeed = ship->realspeed;
    		   if ( ship->class == FIGHTER_SHIP )
                      learn_from_success( ch, gsn_starfighters );
                   if ( ship->class == MIDSIZE_SHIP )
                      learn_from_success( ch, gsn_midships );
                   if ( ship->class == CAPITAL_SHIP )
                      learn_from_success( ch, gsn_capitalships );

                   learn_from_success( ch, gsn_hijack );
                   return;
                }
                set_char_color( AT_RED, ch );
	        send_to_char("You fail to work the controls properly!\n\r",ch);
	        if ( ship->class == FIGHTER_SHIP )
                    learn_from_failure( ch, gsn_starfighters );
                if ( ship->class == MIDSIZE_SHIP )
    	            learn_from_failure( ch, gsn_midships );
                if ( ship->class == CAPITAL_SHIP )
                    learn_from_failure( ch, gsn_capitalships );
    	   	return;

}

void do_add_patrol ( CHAR_DATA *ch , char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    long long int credits;

    if ( IS_NPC( ch ) || !ch->pcdata )
        return;

    strcpy( arg, argument );

    switch( ch->substate )
    {
        default:
                if ( ch->backup_wait )
                {
                    send_to_char( "&RYou already have backup coming.\n\r", ch );
                    return;
                }

                if ( !ch->pcdata->clan )
                {
                    send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
                    return;
                }

                if ( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 30 )
                {
                    ch_printf( ch, "&RYou dont have enough wulongs.\n\r",
ch );
                    return;
                }

                chance = (int) (ch->pcdata->learned[gsn_addpatrol]);
                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
                   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , 1 , do_add_patrol , 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }

                if ( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 30 )
                {
                    ch_printf( ch, "&RYou dont have enough wulongs.\n\r",
ch );
                    return;
                }


                chance = (int) (ch->pcdata->learned[gsn_addpatrol]);
                if ( number_percent( ) < chance )
                {
                   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
                   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
                        NULL, argument , TO_ROOM );
                   add_timer ( ch , TIMER_DO_FUN , 1 , do_add_patrol , 1 );
                   ch->dest_buf = str_dup(arg);
                   return;
                }
                send_to_char("&RYou call for a guard but nobody answers.\n\r",ch);
                learn_from_failure( ch, gsn_addpatrol );
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
                send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
                return;
    }

    ch->substate = SUB_NONE;

    send_to_char( "&GYour guard is on the way.\n\r", ch);
    credits = ch->skill_level[LEADERSHIP_ABILITY] * 30;
    ch_printf( ch, "It cost you %lld wulongs.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_addpatrol );

    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_PATROL;
    else if ( nifty_is_name( "rebel" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_PATROL;
    else
       ch->backup_mob = MOB_VNUM_MERC_PATROL;

    ch->backup_wait = 1;

}

void do_special_forces ( CHAR_DATA *ch , char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    long long int credits;

    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYour forces are already on the way.\n\r", ch );
    	            return;
    	        }

    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for special forces.\n\r", ch );
    	            return;
    	        }

    	        if ( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 60 )
    	        {
   	            ch_printf( ch, "&RYou dont have enough wulongs to send for elite forces.\n\r" );
    	            return;
    	        }

    	        chance = (int) (ch->pcdata->learned[gsn_specialforces]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for special forces.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders into $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_special_forces , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for forces but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_specialforces );
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
    	        send_to_char("&RYou are interrupted before you can finish your call.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    send_to_char( "&GYour forces are on the way.\n\r", ch);
    credits = ch->skill_level[LEADERSHIP_ABILITY] * 60;
    ch_printf( ch, "It cost you %lld wulongs.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_specialforces );

    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_GUARD;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_GUARD;
    else
       ch->backup_mob = MOB_VNUM_BOUNCER;

    ch->backup_wait = number_range(1,2);

}

void do_elite_guard ( CHAR_DATA *ch , char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    long long int credits;

    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYou already have backup coming.\n\r", ch );
    	            return;
    	        }

    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
    	            return;
    	        }

    	        if ( ch->gold < ch->skill_level[LEADERSHIP_ABILITY] * 40 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough wulongs.\n\r",
ch );
    	            return;
    	        }

    	        chance = (int) (ch->pcdata->learned[gsn_eliteguard]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders into $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_elite_guard , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for a guard but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_eliteguard );
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
    	        send_to_char("&RYou are interrupted before you can finish your call.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    send_to_char( "&GYour guard is on the way.\n\r", ch);

    credits = ch->skill_level[LEADERSHIP_ABILITY] * 40;
    ch_printf( ch, "It cost you %lld wulongs.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_postguard );

    if ( nifty_is_name( "empire" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_IMP_ELITE;
    else if ( nifty_is_name( "republic" , ch->pcdata->clan->name ) )
       ch->backup_mob = MOB_VNUM_NR_ELITE;
    else
       ch->backup_mob = MOB_VNUM_BOUNCER_ELITE;

    ch->backup_wait = 1;

}

void do_jail ( CHAR_DATA *ch , char *argument )
{
    CHAR_DATA *victim =NULL;
    CLAN_DATA   *clan =NULL;
    ROOM_INDEX_DATA *jail =NULL;

    if ( IS_NPC (ch) ) return;

    if ( !ch->pcdata || ( clan = ch->pcdata->clan ) == NULL )
    {
	send_to_char( "Only members of organizations can jail their enemies.\n\r", ch );
	return;
    }

    jail = get_room_index( clan->jail );
    if ( !jail && clan->mainclan )
       jail = get_room_index( clan->mainclan->jail );

    if ( !jail )
    {
	send_to_char( "Your orginization does not have a suitable prison.\n\r", ch );
	return;
    }

    if ( jail->area && ch->in_room->area
    && jail->area != ch->in_room->area &&
    ( !jail->area->planet || jail->area->planet != ch->in_room->area->planet ) )
    {
         send_to_char( "Your organization's prison is to far away.\n\r", ch );
	 return;
    }

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Jail who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "That would be a waste of time.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( ch->position <= POS_SLEEPING )
    {
        send_to_char( "In your dreams or what?\n\r" , ch );
        return;
    }

    if ( victim->position >= POS_SLEEPING )
    {
        send_to_char( "You will have to stun them first.\n\r" , ch );
        return;
    }

	send_to_char( "You have them escorted off to jail.\n\r", ch );
	act( AT_ACTION, "You have a strange feeling that you've been moved.\n\r", ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n has $N escorted away.\n\r",  ch, NULL, victim, TO_NOTVICT );

    char_from_room ( victim );
    char_to_room ( victim , jail );

    act( AT_ACTION, "The door opens briefly as $n is shoved into the room.\n\r",  victim, NULL, NULL, TO_ROOM );

    learn_from_success( ch , gsn_jail );

    return;
}

void do_smalltalk ( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    PLANET_DATA *planet = NULL;
    CLAN_DATA   *clan = NULL;
    int percent;

   if ( IS_NPC(ch) || !ch->pcdata )
   {
       send_to_char( "What would be the point of that.\n\r", ch );
   }

    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Create smalltalk with whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( victim->position == POS_FIGHTING )
    {
        send_to_char( "They're a little busy right now.\n\r" , ch );
        return;
    }


    if ( !IS_NPC(victim) || victim->vip_flags == 0 )
    {
        send_to_char( "Diplomacy would be wasted on them.\n\r" , ch );
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

    WAIT_STATE( ch, skill_table[gsn_smalltalk]->beats );

    percent = number_percent();

    if ( percent - ch->skill_level[DIPLOMACY_ABILITY] + victim->top_level > ch->pcdata->learned[gsn_smalltalk]  )
    {
	/*
	 * Failure.
	 */
	send_to_char( "You attempt to make smalltalk with them.. but are ignored.\n\r", ch );
	act( AT_ACTION, "$n is really getting on your nerves with all this chatter!\n\r", ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n asks $N about the weather but is ignored.\n\r",  ch, NULL, victim, TO_NOTVICT );

        if ( victim->alignment < -500 && victim->top_level >= ch->top_level+5 )
	{
	  sprintf( buf, "SHUT UP %s!", ch->name );
	  do_yell( victim, buf );
    //  global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	}

	return;
    }

    ch->chatrain++;

    send_to_char( "You strike up a short conversation with them.\n\r", ch );
    act( AT_ACTION, "$n smiles at you and says, 'hello'.\n\r", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n chats briefly with $N.\n\r",  ch, NULL, victim, TO_NOTVICT );

    if ( IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
      return;

    if ( ( clan = ch->pcdata->clan->mainclan ) == NULL )
       clan = ch->pcdata->clan;

    planet = ch->in_room->area->planet;
    if(!planet)
	return;

    if ( clan != planet->governed_by )
    	    planet->pop_support -= 0.1;
    else
	    planet->pop_support += 0.1;
    send_to_char( "Popular support for your organization increases slightly.\n\r", ch );

    gain_exp(ch, victim->top_level*10, DIPLOMACY_ABILITY);
    ch_printf( ch , "You gain %d diplomacy experience.\n\r", victim->top_level*10 );

    learn_from_success( ch, gsn_smalltalk );

    if ( planet->pop_support > 100 )
        planet->pop_support = 100;
    if( planet->pop_support < -100)
	planet->pop_support = -100;
}

void do_propaganda ( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    PLANET_DATA *planet;
    CLAN_DATA   *clan;
    int percent, level;

   if ( IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
       send_to_char( "What would be the point of that.\n\r", ch );
       return;
   }

    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Spread propaganda to who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( victim->position == POS_FIGHTING )
    {
        send_to_char( "They're a little busy right now.\n\r" , ch );
        return;
    }


    if ( victim->vip_flags == 0 )
    {
        send_to_char( "Diplomacy would be wasted on them.\n\r" , ch );
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

    if ( ( clan = ch->pcdata->clan->mainclan ) == NULL )
       clan = ch->pcdata->clan;

    planet = ch->in_room->area->planet;

    if(!planet)
    {
	send_to_char("You are not on a planet to do this!\r\n", ch);
	return;
    }

    if(victim->frustrated_by && victim->frustrated_by == ch && victim->frustration > 3)
    {
	send_to_char("You try to tell them about your organization.\r\n", ch);
        sprintf( buf, "No thanks, %s, I'm tired of all your crap." , ch->name);
        do_say( victim, buf );
        return;
    }

    ch->chatrain++;

    sprintf( buf, ", and the evils of %s" , planet->governed_by ? planet->governed_by->name : "their current leaders" );
    ch_printf( ch, "You speak to them about the benefits of the %s%s.\n\r", ch->pcdata->clan->name,
        planet->governed_by == clan ? "" : buf );
    act( AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n tells $N about their organization.\n\r",  ch, NULL, victim, TO_NOTVICT );

    WAIT_STATE( ch, skill_table[gsn_propaganda]->beats );

	percent = number_percent();

    if ( percent - get_curr_cha(ch) + victim->top_level > ch->pcdata->learned[gsn_propaganda]
	&& !IS_IMMORTAL(ch))
    {

    if ( planet->governed_by != clan )
	{
	  sprintf( buf, "%s is a traitor!" , ch->name);
	  do_yell( victim, buf );
	  victim->frustrated_by = ch;
          victim->frustration++;
	  // New planetary frustration code - Gatz
	  if( planet->frustrated_by && planet->frustrated_by->name)
	  {
		  if( planet->frustrated_by != clan)
		  {
			if(planet->frust_level > 0)
				planet->frust_level -= .1;
			if(planet->frust_level < 0)
			{
				planet->frust_level = 0;
				planet->frustrated_by = NULL;
			}
	  	}
	  	else
	  	{
			if(planet->frustrated_by == clan)
				planet->frust_level += .1 * (ch->top_level/50);
			if(planet->frust_level > 100)
				planet->frust_level = 100;
	  	}
	  }
	  if(ch->pcdata && ch->pcdata->clan
	     && !planet->frustrated_by)
	  {
		planet->frustrated_by = clan;
		planet->frust_level += .5 * ch->top_level/50;
	  }

      global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	}

	return;
    }

    level = IS_NPC(ch) ? ch->top_level
                         : (int) (ch->pcdata->learned[gsn_propaganda]);

    if ( planet->governed_by == clan )
    {
       planet->pop_support += (.01 * level/2);
       send_to_char( "Popular support for your organization increases.\n\r", ch );
       if( planet->frustrated_by && planet->frust_level > 0)
       {
		planet->frust_level -= .1 + ch->top_level/50;
		if(planet->frust_level <= 0)
		{
			planet->frust_level = 0;
			planet->frustrated_by = NULL;
		}
       }
    }
    else
    {
       planet->pop_support -= (.01 * level)/2;
       send_to_char( "Popular support for the current government decreases.\n\r", ch );
    }

    gain_exp(ch, victim->top_level*100, DIPLOMACY_ABILITY);
    ch_printf( ch , "You gain %d diplomacy experience.\n\r", victim->top_level*100 );

    learn_from_success( ch, gsn_propaganda );

    if ( planet->pop_support > 100 )
        planet->pop_support = 100;
    if ( planet->pop_support < -100 )
        planet->pop_support = -100;

}

void do_bribe ( CHAR_DATA *ch , char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    PLANET_DATA *planet;
    CLAN_DATA   *clan;
    int percent;
    long long int amount;

    if ( IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
    {
      send_to_char( "What would be the point of that.\n\r", ch );
      return;
    }

    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Bribe who how much?\n\r", ch );
	return;
    }

    amount = atoll( argument );

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( amount <= 0 )
    {
       	send_to_char( "A little bit more money would be a good plan.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( victim->position == POS_FIGHTING )
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

    if ( victim->vip_flags == 0 )
    {
        send_to_char( "Diplomacy would be wasted on them.\n\r" , ch );
        return;
    }

    ch->gold -= amount;
    victim->gold += amount;

	ch_printf( ch, "You give them a small gift on behalf of %s.\n\r", ch->pcdata->clan->name );
	act( AT_ACTION, "$n offers you a small bribe.\n\r", ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n gives $N some money.\n\r",  ch, NULL, victim, TO_NOTVICT );

    if ( !IS_NPC( victim ) )
	return;

    WAIT_STATE( ch, skill_table[gsn_bribe]->beats );

	percent = number_percent();

    if ( percent - amount + victim->top_level > ch->pcdata->learned[gsn_bribe]  )
        return;

    if ( ( clan = ch->pcdata->clan->mainclan ) == NULL )
       clan = ch->pcdata->clan;

    planet = ch->in_room->area->planet;


    if ( clan == planet->governed_by )
    {
      planet->pop_support += URANGE( 0.1 , amount/1000 , 2 );
      send_to_char( "Popular support for your organization increases slightly.\n\r", ch );

      amount = UMIN( amount ,( exp_level(ch->skill_level[DIPLOMACY_ABILITY]+1) - exp_level(ch->skill_level[DIPLOMACY_ABILITY]) ) );

      gain_exp(ch, amount , DIPLOMACY_ABILITY);
      ch_printf( ch , "You gain %lld diplomacy experience.\n\r", amount );

      learn_from_success( ch, gsn_bribe );
    }

    if ( planet->pop_support > 100 )
        planet->pop_support = 100;
}

void do_mass_propaganda ( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    PLANET_DATA *planet;
    CLAN_DATA   *clan;

   if ( IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
       send_to_char( "What would be the point of that?\n\r", ch );
       return;
   }

    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( victim->vip_flags == 0 )
    {
        send_to_char( "Diplomacy would be wasted on them.\n\r" , ch );
        return;
    }

    if ( ch->position <= POS_SLEEPING )
    {
        send_to_char( "In your dreams or what?\n\r" , ch );
        return;
    }

    if ( ( clan = ch->pcdata->clan->mainclan ) == NULL )
    clan = ch->pcdata->clan;

    planet = ch->in_room->area->planet;

    sprintf( buf, ", and the evils of %s" , planet->governed_by ? planet->governed_by->name : "their current leaders" );
    ch_printf( ch, "You speak to them about the benefits of the %s%s.\n\r", ch->pcdata->clan->name,
        planet->governed_by == clan ? "" : buf );
    act( AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n tells $N about their organization.\n\r",  ch, NULL, victim, TO_NOTVICT );


    WAIT_STATE( ch, skill_table[gsn_masspropaganda]->beats );

    if ( number_percent() - get_curr_cha(ch) + victim->top_level > ch->pcdata->learned[gsn_masspropaganda]  )
    {
        if ( planet->governed_by != clan )
		{
		  sprintf( buf, "%s is a traitor!" , ch->name);
		  do_yell( victim, buf );
   		  global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		}

	return;
    }

    if ( planet->governed_by == clan )
    {
       planet->pop_support += 2 + ch->top_level/50;
       send_to_char( "Popular support for your organization increases.\n\r", ch );
    }
    else
    {
       planet->pop_support -= ch->top_level/50;
       send_to_char( "Popular support for the current government decreases.\n\r", ch );
    }

    gain_exp(ch, victim->top_level*100, DIPLOMACY_ABILITY);
    ch_printf( ch , "You gain %d diplomacy experience.\n\r", victim->top_level*100 );

    learn_from_success( ch, gsn_masspropaganda );

    if ( planet->pop_support > 100 )
        planet->pop_support = 100;
    if ( planet->pop_support < -100 )
        planet->pop_support = -100;
}


void do_gather_intelligence ( CHAR_DATA *ch , char *argument )
{}

void do_rumors( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	CHAR_DATA *hater;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
		send_to_char( "Spread rumors to who?\n\r", ch );
		return;
    }

    if ( arg2[0] == '\0' )
    {
		send_to_char( "Spread rumors about who?\n\r", ch );
		return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "I don't see that person here.\n\r", ch );
		return;
    }

    if ( IS_SET( victim->act , ACT_DEADLY ))
    {
	send_to_char( "That person is too enraged to listen!", ch );
	return;
    }

    hater = victim;

    if ( ch == victim )
    {
		send_to_char( "You spread rumors to yourself... schizo.\n\r", ch );
		return;
    }

    if ( !IS_NPC (victim) )
    {
    	send_to_char( "They wouldn't care.\n\r", ch );
    	return;
    }

    if ( IS_SET( victim->act, ACT_PACIFIST) )
    {
	send_to_char( "They are too peacful to care.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "You can't spread rumors about that person currently.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
        send_to_char( "You can't spread rumors about yourself!\n\r", ch);
	return;
    }

    if(IS_NPC(victim))
    {
	send_to_char("You can only spread rumors about Players!\r\n", ch);
	return;
    }

    if(victim->played < 6)
    {
		send_to_char("It's not nice to spread rumors about newbies!\r\n", ch);
		return;
	}

    if((number_range(10,120) - get_curr_cha(ch)/3 - ch->pcdata->learned[gsn_rumors]) > 0)
    {
//	send_to_char("You fail at spreading rumors!\r\n", ch);
        sprintf( buf, "You spread rumors about %s to %s.\n\r" , victim->name, hater->name );
        send_to_char( buf, ch );
        act( AT_ACTION, "$n spreads rumors to $N.",  ch, NULL, hater, TO_ROOM );
        sprintf(arg1, "Nice try, %s!  I hate you!", ch->name);
        do_say(hater, arg1);
        hater->frustrated_by = ch; // Frustration tracks to the rumor spreader -Funf 2009-02-12
        start_hating( hater, ch );
        start_hunting( hater, ch );
        SET_BIT(hater->act, ACT_DEADLY);

	return;
    }

    sprintf( buf, "You spread rumors about %s to %s.\n\r" , victim->name, hater->name );
    send_to_char( buf, ch );
    act( AT_ACTION, "$n spreads rumors to $N.",  ch, NULL, hater, TO_ROOM );
    sprintf(arg1, "I can't believe this! I hate %s!", victim->name);
    do_say(hater, arg1);
    hater->frustrated_by = ch; // Frustration tracks to the rumor spreader -Funf 2009-02-12
    start_hating( hater, victim );
    start_hunting( hater, victim );
    SET_BIT(hater->act, ACT_DEADLY);
    learn_from_success(ch, gsn_rumors);
    return;
}

/*
void do_hack_systems( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj, *obj_next;
	bool found = FALSE;
	bool checktool = FALSE;
	bool power = FALSE;
    /* Doesn't work! Not used anymore - Gatz
    if( !argument || argument[0] == '\0' )
    {
    	send_to_char( "You must select a tool to hack with.\n\r", ch );
    	return;
    }

    obj = argument;

    if(obj->item_type != ITEM_COMPUTER)
    {
	send_to_char("&RYou need a computer to hack!\n\r", ch);
	return;
    }

   *//*
    checktool = FALSE;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }
   if( !IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_HACKWARN))
   {
	send_to_char("&RWARNING! Hacking is illegal in the galaxy of Cowboy Bebop: Space Cowboy!\r\n",ch);
	send_to_char("&YIf you hack and fail, you will get a bounty which could lead to attacks by ISSP or RBH!\r\n", ch);
	send_to_char("&RThis is your first and only warning, next time you hack you wont see this.\r\n", ch);
	SET_BIT(ch->pcdata->flags, PCFLAG_HACKWARN);
	return;
   }
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
    	if (obj->item_type == ITEM_COMLINK)
	{
        	checktool = TRUE;
	}
	if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
	{
		found = TRUE;
	}
	if(obj->item_type == ITEM_COMPUTER && obj->value[2] == 1)
		power = TRUE;
	if(found && checktool && power)
		break;
    }
    if ( !checktool )
    {
    	send_to_char( "&RYou need a comlink to hack.\n\r", ch);
        return;
    }
    if( !found )
    {
	send_to_char("&RYou need a working computer to hack.\n\r", ch);
	return;
    }

    if(!power)
    {
	send_to_char("&RMaybe you should 'bootup' your computer first.\n\r", ch);
	return;
    }

    checktool 	= FALSE;

        if( found )
  	{

		WAIT_STATE( ch, 15 );

		act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

		if ( number_percent() - get_curr_int(ch)/5 > ch->pcdata->learned[gsn_hack_systems] - 10 )
      		{
			send_to_char( "Your attempt to hack is discovered!\n\r", ch );
			place_bounty( ch, ch, 3000, 0);
			for( obj = ch->last_carrying; obj; obj = obj->prev_content)
			{
				if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
				{
					if((obj->value[0]-1) > 0)
					{
						obj->value[0]--;
						send_to_char("&RYour computer was damaged!\n\r", ch);
					}
					else
					{
						obj->value[0]--;
						send_to_char( "Your computer was destroyed!\n\r", ch);
					}
					break;
				}
			}
			if(ch->perm_int < 15 && number_range(0,4) >= 3)
				ch->inttrain++;
			return;
		}

		send_to_char( "You successfully scout the system.\n\r", ch );
		learn_from_success( ch, gsn_hack_systems );
		gain_exp(ch, ch->top_level/2, FORCE_ABILITY);
    		ch_printf( ch , "You gain %d hacking experience.\n\r", ch->top_level/2 );
		return;
	}
   	send_to_char( "You need a computer to hack with.\n\r", ch );
  	return;
}*/

void do_hackwho( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *obj_next;
	bool found = FALSE, bootup = FALSE;
	CHAR_DATA *victim;
/* hackwho no longer illegal - Funf 2007-11-13
   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }*/
	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {

        	if ( obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
        	{
        	        found = TRUE;
			if(obj->value[2] == 1)
				bootup = TRUE;
        	        break;
		}

        }

	if( argument[0] = '\0')
	{
		send_to_char("&RSyntax: Hackwho 'name'\n\r", ch);
		return;
	}
/*
	if((victim = get_char_world(ch, argument)) == NULL)
 	{
   		send_to_char("That character is not online!\r", ch);
		   return;
        }
*/

/*
   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }*/

	if(found)
	{
	    if (!bootup)
            {
              act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
	      return;
            }
        	if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackwho] )
    		{
        	        send_to_char( "Your attempt to hack fails!\n\r", ch );
        	        /*place_bounty( ch, ch, 10000, 0);*/
        	       if(obj->value[0]-1 > 0)
               		{
                	        obj->value[0]--;
                	        send_to_char("&RYour computer was damaged!\n\r", ch);
               		}
               		else
               		{
                	        obj->value[0]--;
                	        send_to_char( "Your computer was destroyed!\n\r", ch);
               		}
			if(ch->perm_int < 15 && number_range(0,4) >= 3)
				ch->inttrain++;

                	return;
        	}
		do_whois(ch, argument);
       		 send_to_char( "You successfully hacked information.\n\r", ch );

			if(ch->skill_level[FORCE_ABILITY] < 30)
			{
				gain_exp(ch, (ch->top_level/10), FORCE_ABILITY);
				ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/10) );
			}

		learn_from_success(ch, gsn_hackwho);
	//	do_whois(ch, argument);
	}
	else
		send_to_char("You need a working computer for this!\n\r", ch);

	return;
}


void do_hackbaydoor( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *comlink, *computer;
    SHIP_DATA *ship;
    SHIP_DATA *eShip = NULL;
	bool checktool, checkcomm, checkpower;
    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);

    if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    {
        send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
        return;
    }

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }


    if (ship->shipstate == SHIP_HYPERSPACE )
    {
        send_to_char("&RYou can only do that in realspace!\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char("&ROverride the controls of what ship?\n\r", ch);
        return;
    }

    checktool = FALSE;
    checkcomm = FALSE;
    checkpower = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
    	if (obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
	{
        	checktool = TRUE;
		computer = obj;
	}
	if(obj->item_type == ITEM_COMPUTER && obj->value[2] > 0)
		checkpower = TRUE;
	/*
            if (obj->value[2] == 0 )
            {
              act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
	      return;
            }
	*/
        if (obj->item_type == ITEM_COMLINK)
        {
			checkcomm = TRUE;
	}
    }

    if ( !checktool )
    {
    	send_to_char( "&RYou need a computer to hack.\n\r", ch);
        return;
    }

    if ( !checkcomm )
    {
        send_to_char( "&RYou need a communication device.\n\r", ch);
        return;
    }

    if(!checkpower)
    {
	send_to_char("&RYou need to 'bootup' your computer first.\r\n", ch);
	return;
    }

    checktool 	= FALSE;
    checkcomm 	= FALSE;

    eShip = get_ship_here(arg,ship->starsystem);

    if ( eShip == NULL )
    {
        send_to_char("&RThat ship isn't here!\n\r",ch);
        return;
    }

    if ( abs(eShip->vx - ship->vx) > 100*((ship->comm)+(eShip->comm)+20) ||
    abs(eShip->vy - ship->vy) > 100*((ship->comm)+(eShip->comm)+20) ||
    abs(eShip->vz - ship->vz) > 100*((ship->comm)+(eShip->comm)+20) )
    {
    	send_to_char("&RThat ship is out of the range of your comm system.\n\r&w", ch);
    	return;
    }
    if ( eShip->class != MIDSIZE_SHIP && eShip->class != CAPITAL_SHIP  )
    {
	send_to_char("&RTarget ships needs to atleast have a bay!\r\n", ch);
	return;
    }

	WAIT_STATE( ch, 20 );
	act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

	if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackbaydoor]  )
        {
		send_to_char( "Your attempt to hack is discovered!\n\r", ch );
		place_bounty( ch, ch, 10000, 0);
		for( obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                      if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
                      {
                                if((obj->value[0]-1) > 0)
                                {
                                                obj->value[0]--;
                                                send_to_char("&RYour computer was damaged!\n\r", ch);
                                }
                                else
                                {
                                                obj->value[0]--;
                                                send_to_char( "Your computer was destroyed!\n\r", ch);
                                }
                                break;
                      }
                 }

		if(ch->perm_int < 15 && number_range(0,4) >= 3)
			ch->inttrain++;

		return;
	}

	send_to_char( "You successfully hacked the ship's system.\n\r", ch );
	learn_from_success( ch, gsn_hackbaydoor );
	gain_exp(ch, (ch->top_level/2 + 20), FORCE_ABILITY);
   	ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+20) );

    if ( eShip->bayopen == FALSE )
    {
        eShip->bayopen = TRUE;
        send_to_char( "&GBay Doors Opened. Confirmed.\n\r", ch);
        sprintf( buf ,"%s's bay doors open." , eShip->name );
        echo_to_system( AT_YELLOW, eShip, buf , NULL );
        return;
    }

    if ( eShip->bayopen == TRUE )
    {
        eShip->bayopen = FALSE;
        send_to_char( "&GBay Doors Closed. Confirmed.\n\r", ch);
        sprintf( buf ,"%s's bay doors close." , eShip->name );
        echo_to_system( AT_YELLOW, eShip, buf , NULL );
        return;
    }

	return;
}

void do_hackbanktransfer(CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    unsigned long long int amount = 0; /* Made unsigned - Funf */
    int level = 0;
    CHAR_DATA *victim1,*victim2;
    OBJ_DATA *obj;
    bool comlink = FALSE, computer = FALSE, power = FALSE;

    argument = one_argument( argument , arg1 );
    argument = one_argument( argument , arg2 );
    argument = one_argument( argument , arg3 );

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }

    if ( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You must be at a bank terminal to use this skill.\n\r", ch );
        return;
	}


    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->pIndexData->item_type == ITEM_COMLINK)
     	  comlink = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0)
	  computer = TRUE;

 	if(obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[2] > 0)
		power = TRUE;

    }

    if(!comlink)
    {
	send_to_char("&RYou need a comlink to hack!\n\r", ch);
	return;
    }
    if(!computer)
    {
	send_to_char("&RYou need a working computer to hack!\n\r", ch);
	return;
    }

    if (!power )
    {
        act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
	return;
     }

    if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("&CHacktransfer: Bank Account 1/Bank Account 2/Amount\n\r", ch);
	return;
    }
    if(!IS_NPC(ch) && ch->pcdata->hackcount >= 5)
    {
        send_to_char("&RHacking any more is too much of a risk!\r\n", ch);
        return;
    }

    amount = atoi(arg3);
    if( amount < 5)
    {
	send_to_char("&RYou need to pick an amount higher than that!\r\n", ch);
	return;
    }

    if( ( ( victim1 = get_char_world(ch, arg1) ) == NULL ) || !victim1->desc )
    {
        send_to_char("&RNo such player online for sender!&W\n\r", ch);
        return;
    }
    if( ( ( victim2 = get_char_world(ch, arg2) ) == NULL ) || !victim2->desc )
    {
        send_to_char("&RNo such player online for reciever!&W\n\r", ch);
        return;
    }
    if( ch == victim1)
    {
	send_to_char("&RYou might aswell transfer it legally!\n\r", ch);
	return;
    }
    if( ch == victim2)
    {
	send_to_char("&RIsn't that a bit obvious for the ISSP to spot?\n\r", ch);
	return;
    }
    if( IS_NPC(victim1) || IS_NPC(victim2) || NOT_AUTHED(victim1) || NOT_AUTHED(victim2))
    {
	send_to_char("&RTarget characters need to be players and authorized first!\n\r", ch);
	return;
    }
   if(IS_IMMORTAL(victim1) || IS_IMMORTAL(victim2))
   {
	if(!IS_IMMORTAL(ch))
	{
		send_to_char("&RThey probably wouldn't like that.\r\n", ch);
		return;
	}
   }

   if(get_age(victim1) < 6 || get_age(victim2) < 6)
   {
	send_to_char("&RTarget characters need to have played atleast 6 hours first!\r\n", ch);
	return;
   }
   // level = ch->pcdata->learned[gsn_hackbanktransfer];
    level = ch->skill_level[FORCE_ABILITY];
    level = level*1000;
    if( amount > level)
    {
	ch_printf(ch, "&RYou can only transfer up to %d!", level);
	return;
    }
    if( victim1->pcdata->bank < amount )
    {
	send_to_char("&RThe selected sender's account doesn't have that much!\n\r", ch);
	return;
    }
    if( (victim2->pcdata->bank + amount) > MAX_BANK) /* replaced hard coded value - Funf */
    {
	send_to_char("&RThat would put the reciever's account over the limit!\n\r", ch);
	return;
    }
    if( amount == 0)
    {
	send_to_char("&RTransfering 0 wulongs would be pointless!\n\r", ch);
	return;
    }
    WAIT_STATE( ch, 25 );

    act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

    if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackbanktransfer]
	|| number_range(0,5) > 4)
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 10000, 0);
    //     send_to_char( "Your computer was destroyed!\n\r", ch);
         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
               {
	/*
                   separate_obj( obj );
                   obj_from_char( obj );
                   extract_obj( obj );
*/
               if(obj->value[0]-1 > 0)
               {
                        obj->value[0]--;
                        send_to_char("&RYour computer was damaged!\n\r", ch);
               }
               else
               {
                        obj->value[0]--;
                        send_to_char( "Your computer was destroyed!\n\r", ch);
               }

                   break;
               }
         }
	if(ch->perm_int < 15 && number_range(0,4) >= 3)
		ch->inttrain++;
         return;
    }
   victim1->pcdata->bank -= amount;
   victim2->pcdata->bank += amount;
   ch_printf( ch, "You were able to hack %lld wulongs from %s to %s!\n\r", amount, victim1->name, victim2->name);
   ch_printf( victim2 , "&R1#22@3 &Ctransfers &Y%lld &Cwulongs to your account.&W\n\r" , amount);
   gain_exp(ch, (ch->top_level/2+40), FORCE_ABILITY);
   ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+40) );

   learn_from_success( ch, gsn_hackbanktransfer);
   ch->pcdata->hackcount;
   return;
}

void do_hackshipdoor( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj, *comp;
    EXIT_DATA *pexit;
    SHIP_DATA *ship;
    bool computer = FALSE, comlink = FALSE, power = FALSE;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if (obj->item_type == ITEM_COMLINK)
        {
                comlink = TRUE;
        }
	if(obj->item_type == ITEM_COMPUTER && obj->value[2]==1)
	{
		power = TRUE;
	}
        if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
        {
                computer = TRUE;
		comp = obj;
        }
    }

    if(!comlink)
    {
	send_to_char("&RYou need a comlink to hack!\n\r", ch);
	return;
    }
    if(!computer)
    {
	send_to_char("&RYou need a computer to hack!\n\r", ch);
	return;
    }
    if(!power)
    {
	send_to_char("&RYou should bootup your computer first!\n\r", ch);
	return;
    }


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Hack what ship door?\n\r", ch );
	return;
    }


    if ( ( ship = ship_in_room(ch->in_room, arg ) ) != NULL )
    {
             char buf[MAX_STRING_LENGTH];

             if ( check_pilot( ch , ship ) )
    	     {
    	            send_to_char("&RWhat would be the point of that!\n\r",ch);
    	            return;
    	     }

	     if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   	     {
        	send_to_char( "&RThat ship has already started to launch",ch);
        	return;
   	     }
	     if(ship->hatchopen)
	     {
		send_to_char("&RThe hatch is already open!\n\r", ch);
		return;
	     }

   	     WAIT_STATE( ch, 10 );

   	     if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackshipdoor]
		  || number_range(1,5) > 4 )
             {
		send_to_char( "You were detected!\n\r", ch);
                place_bounty( ch, ch, 5000, 0);
		/*
                send_to_char( "Your computer was destroyed!\n\r", ch);
		separate_obj( comp );
                obj_from_char( comp );
                extract_obj( comp );*/
		if(comp->value[0]-1 > 0)
               {
                        comp->value[0]--;
                        send_to_char("&RYour computer was damaged!\n\r", ch);
               }
               else
               {
                        comp->value[0]--;
                        send_to_char( "Your computer was destroyed!\n\r", ch);
               }
		if(ch->perm_int < 15 && number_range(0,4) >= 3)
			ch->inttrain++;
		return;
             }

   	     if ( !ship->hatchopen)
             {
   		ship->hatchopen = TRUE;
   		act( AT_PLAIN, "You hack the lock and hatch opens on $T!", ch, NULL, ship->name, TO_CHAR );
   		act( AT_PLAIN, "$n hacked open the hatch on $T!", ch, NULL, ship->name, TO_ROOM );
   		echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch opens from the outside." );
                learn_from_success( ch, gsn_hackshipdoor );
                gain_exp(ch, ch->top_level/2, FORCE_ABILITY);
                ch_printf( ch , "You gain %d hacking experience.\n\r", ch->top_level/2 );
	     }
       	     return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}

void do_hacklaunch( CHAR_DATA *ch, char *argument )
{
    int chance;
    long long int price = 0;
    SHIP_DATA *ship;
    char buf[MAX_STRING_LENGTH];
    bool computer = FALSE, comlink = FALSE, power = FALSE;
    OBJ_DATA *obj, *obj_next;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }


    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if (obj->item_type == ITEM_COMLINK)
        {
                comlink = TRUE;
        }
        if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
        {
                computer = TRUE;
        }
	if(obj->item_type == ITEM_COMPUTER && obj->value[2]==1)
	{
		power = TRUE;
	}

    }
    if(!comlink)
    {
        send_to_char("&RYou need a comlink to hack!\n\r", ch);
        return;
    }
    if(!computer)
    {
        send_to_char("&RYou need a working computer to hack!\n\r", ch);
        return;
    }
    if(!power)
    {
	send_to_char("&RMaybe you should 'bootup' your computer first!\n\r", ch);
	return;
    }

    if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    {
        send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
        return;
    }

    if ( ship->class > SHIP_PLATFORM )
    {
        send_to_char("&RThis isn't a spacecraft!\n\r",ch);
        return;
    }

    if ( (ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL )
    {
        send_to_char("&RYou don't seem to be in the pilot seat!\n\r",ch);
        return;
    }

    if ( autofly(ship) )
    {
        send_to_char("&RThe ship is set on autopilot, you'll have to turn it off first.\n\r",ch);
        return;
    }
    if  ( ship->class == SHIP_PLATFORM )
    {
        send_to_char( "You can't do that here.\n\r" , ch );
        return;
    }
    if ( ship->lastdoc != ship->location )
    {
        send_to_char("&rYou don't seem to be docked right now.\n\r",ch);
        return;
    }

    if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
    {
         send_to_char("The ship is not docked right now.\n\r",ch);
         return;
    }
    WAIT_STATE( ch, 10 );

    act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

    if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hacklaunch] || number_range(1,5) > 3 )
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 10000, 0);
       //  send_to_char( "Your computer was destroyed!\n\r", ch);
         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
               {
		/*
                   separate_obj( obj );
                   obj_from_char( obj );
                   extract_obj( obj );
		*/
               if(obj->value[0]-1 > 0)
               {
                        obj->value[0]--;
                        send_to_char("&RYour computer was damaged!\n\r", ch);
               }
               else
               {
                        obj->value[0]--;
                        send_to_char( "Your computer was destroyed!\n\r", ch);
               }

                   break;
               }
         }
	if(ch->perm_int < 15 && number_range(0,4) >= 3)
		ch->inttrain++;
         return;
    }

                if ( ship->class == FIGHTER_SHIP )
                price=20;
            if ( ship->class == MIDSIZE_SHIP )
                price=50;
            if ( ship->class == CAPITAL_SHIP )
                price=500;

            price += ( ship->maxhull-ship->hull );
            price += ( 10 * ( ship->maxammo-ship->ammo) );
            if (ship->missiles )
                price += ( 50 * (ship->maxmissiles-ship->missiles) );
            else if (ship->torpedos )
                price += ( 75 * (ship->maxtorpedos-ship->torpedos) );
            else if (ship->rockets )
                price += ( 150 * (ship->maxrockets-ship->rockets) );

            if (ship->shipstate == SHIP_DISABLED )
                price += 200;
            if ( ship->missilestate == MISSILE_DAMAGED )
                price += 100;
            if ( ship->statet0 == LASER_DAMAGED )
                price += 50;
            if ( ship->statet1 == LASER_DAMAGED )
                price += 50;
            if ( ship->statet2 == LASER_DAMAGED )
                price += 50;
                if ( ch->gold < price )
            {
                ch_printf(ch, "&RYou don't have enough funds to prepare this ship for launch.\n\r");
                return;
            }

            ch->gold -= price;
            ch_printf(ch, "&GYou pay %lld wulongs to ready the ship for launch.\n\r", price );
        gain_exp(ch, (ch->top_level/2+40), FORCE_ABILITY);
        ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+40) );
        learn_from_success( ch, gsn_hacklaunch );


        /*ship->energy = ship->maxenergy;*/
        ship->chaff = ship->maxchaff;
        ship->missiles = ship->maxmissiles;
        ship->torpedos = ship->maxtorpedos;
        ship->rockets = ship->maxrockets;
        ship->autorecharge = FALSE;
        ship->autotrack = FALSE;
        ship->autospeed = FALSE;
        ship->hull = ship->maxhull;
        ship->ammo = ship->maxammo;

        ship->missilestate = MISSILE_READY;
        ship->statet0 = LASER_READY;
        ship->statet1 = LASER_READY;
        ship->statet2 = LASER_READY;
        ship->shipstate = SHIP_DOCKED;

        if (ship->hatchopen)
        {
                ship->hatchopen = FALSE;
            sprintf( buf , "The hatch on %s closes." , ship->name);
            echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
            echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch slides shut." );
        }
        set_char_color( AT_GREEN, ch );
        send_to_char( "Launch sequence initiated.\n\r", ch);
        act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch, NULL, argument , TO_ROOM );
        echo_to_ship( AT_YELLOW , ship , "The ship hums as it lifts off the ground.");
        sprintf( buf, "%s begins to launch.", ship->name );
        echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
        ship->shipstate = SHIP_LAUNCH;
        ship->currspeed = ship->realspeed;
        return;
}

void do_hackbroadcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    bool computer = FALSE, comlink = FALSE, power = FALSE;
    OBJ_DATA *obj, *obj_next;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }

	if ( !IS_SET( ch->in_room->room_flags2, ROOM_BROADCAST ) )
    {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You must be at a broadcast station to use this skill.\n\r", ch );
        return;
	}

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if (obj->item_type == ITEM_COMLINK)
        {
                comlink = TRUE;
        }
        if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
        {
                computer = TRUE;
        }
	if(obj->item_type == ITEM_COMPUTER && obj->value[2]==1)
	{
		power = TRUE;
	}
    }
    if(!comlink)
    {
        send_to_char("&RYou need a comlink to hack!\n\r", ch);
        return;
    }
    if(!computer)
    {
        send_to_char("&RYou need a working computer to hack!\n\r", ch);
        return;
    }
    if(!power)
    {
	send_to_char("&RMaybe you should 'bootup' your computer first.\n\r", ch);
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->hackcount >= 5)
    {
	send_to_char("&RHacking any more is too much of a risk!\r\n", ch);
	return;
    }
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "What are you trying to broadcast?\n\r", ch );
        return;
    }



    if ( number_percent() - (get_curr_int(ch)*2) > ch->pcdata->learned[gsn_hackbroadcast])
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 5000, 0);
       //  send_to_char( "Your computer was destroyed!\n\r", ch);
         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
               {
	/*
                   separate_obj( obj );
                   obj_from_char( obj );
                   extract_obj( obj );
	*/
               if(obj->value[0]-1 > 0)
               {
                        obj->value[0]--;
                        send_to_char("&RYour computer was damaged!\n\r", ch);
               }
               else
               {
                        obj->value[0]--;
                        send_to_char( "Your computer was destroyed!\n\r", ch);
               }

                   break;
               }
         }
	if(ch->perm_int < 15 && number_range(0,4) >= 3)
		ch->inttrain++;
         return;
    }

    gain_exp(ch, (ch->top_level/20), FORCE_ABILITY);
    ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/20) );
    learn_from_success( ch, gsn_hackbroadcast);
    ch->pcdata->hackcount++;
    talk_channel( ch, argument, CHANNEL_103, "Broadcast" );

    return;
}

void do_hackbounty(CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long long int amount;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool comlink = FALSE, computer = FALSE, power = FALSE;
    BOUNTY_DATA *bounty;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }


    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->pIndexData->item_type == ITEM_COMLINK)
          comlink = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0)
          computer = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[2]==1)
	  power = TRUE;
    }

    if(!comlink)
    {
        send_to_char("&RYou need a comlink to hack!\n\r", ch);
        return;
    }

    if(!computer)
    {
        send_to_char("&RYou need a working computer to hack!\n\r", ch);
        return;
    }

    if(!power)
    {
	send_to_char("&RMaybe you should 'bootup' your computer first!\n\r", ch);
	return;
    }

    if( arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("&RHackbounty 'add/hide/show' 'name' # (Number for Add only)\n\r", ch);
	return;
    }

    if( ( ( victim = get_char_world(ch, arg2) ) == NULL ))
    {
        send_to_char("&RNo such player online!&W\n\r", ch);
        return;
    }
    if(IS_NPC(victim))
    {
		send_to_char("&RYou can't hackbounty mobs!&w\n\r", ch);
		return;
	}
    if(!IS_NPC(ch) && ch->pcdata->hackcount >= 5)
    {
        send_to_char("&RHacking any more is too much of a risk!\r\n", ch);
        return;
    }

    if(!str_cmp(arg1, "add"))
 	   amount = atoi(arg3);
    WAIT_STATE( ch, 25 );
    if(!str_cmp(arg1, "add") && amount < 1)
    {
	send_to_char("&RThat is too little to matter!\r\n", ch);
	return;
    }
    if(!str_cmp(arg1, "add") && amount > 1000000)
    {
	send_to_char("&RThat would be a bit obvious wouldn't it?\r\n", ch);
	return;
    }
    act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

    if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackbounty] || number_percent() < 15 )
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 20000, 0);
         ch->pcdata->hackcount = 6; /* Added additional fail chance and stop on fail. -Funf */
      //   send_to_char( "Your computer was destroyed!\n\r", ch);
         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
               {
		/*
                   separate_obj( obj );
                   obj_from_char( obj );
                   extract_obj( obj );
		*/
               if(obj->value[0]-1 > 0)
               {
                        obj->value[0]--;
                        send_to_char("&RYour computer was damaged!\n\r", ch);
               }
               else
               {
                        obj->value[0]--;
                        send_to_char( "Your computer was destroyed!\n\r", ch);
               }

                   break;
               }
         }
	if(ch->perm_int < 15 && number_range(0,4) >= 3)
		ch->inttrain++;
         return;
   }
   if ( !str_prefix( arg1 , "add" ) )
   {
	if( amount < 5000)
	{
		send_to_char("&RYou need to put at least 5,000 wulongs on their head!\n\r", ch);
		return;
	}
	bounty = get_bounty( victim->name );
	if(bounty && IS_SET(bounty->flags, HIDDEN_BOUNTY) )
	{
		REMOVE_BIT(bounty->flags, HIDDEN_BOUNTY);
		send_to_char("You have revealed their bounty!\n\r", ch);
		//return;
	}
	if(bounty && !IS_SET(bounty->flags, FAKE_BOUNTY) )
	{
		send_to_char("&RThey already have a real bounty!\n\r", ch);
		return;
	}
	if(bounty && (bounty->amount + amount > 100000000))
	{
		send_to_char("&ROUT OF BOUNDS ERROR: Line 7445, contact ISSP to correct error.\r\n", ch);
		return;
	}
	sprintf( buf, "[ERRORTHREAD1A#] has added %s wulongs to the bounty on %s.", num_punct(amount) , victim->name );
        echo_to_all ( AT_RED , buf, 0 );
	place_bounty(ch, victim, amount, FAKE_BOUNTY);
   }
   else if ( !str_prefix( arg1 , "hide" ) || !str_prefix( arg1 , "remove" ) )
   {
	//do_rembounty(  ch, arg2 );
     bounty = get_bounty( victim->name );
     if( !bounty )
     {
		send_to_char("&RThey don't have any bounty!\n\r", ch);
		return;
	 }
	 if( IS_SET(bounty->flags, FAKE_BOUNTY) )
	 {
		remove_bounty(bounty);
		send_to_char("You removed any record of their bounty.\n\r", ch);
	 }
	 else if( IS_SET(bounty->flags, HIDDEN_BOUNTY) )
	 {
		 send_to_char("You discover traces of a hidden bounty already in place...\n\r", ch);
	 }
	 else
	 {
		SET_BIT(bounty->flags, HIDDEN_BOUNTY);
		send_to_char("You hide all electronic records of their bounty.\n\r", ch);
	 }

   }
   else if ( !str_prefix( arg1 , "show" ) )
   {
	   	bounty = get_bounty( victim->name );
	   	if(bounty && IS_SET(bounty->flags, HIDDEN_BOUNTY) )
	   	{
	   		REMOVE_BIT(bounty->flags, HIDDEN_BOUNTY);
	   		send_to_char("You discover traces of a hidden bounty. You carefully restore their bounty!\n\r", ch);
	   		//return;
		}
   }
   else
   {
	send_to_char("&RInvalid selection!\n\rOptions: add hide show", ch);
	return;
   }
   gain_exp(ch, (ch->top_level/2+40), FORCE_ABILITY);
   ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+40) );
   learn_from_success( ch, gsn_hackbounty);
   ch->pcdata->hackcount++;
   return;
}





void do_toggle(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];



    strcpy( arg , argument );

    if( arg[0] == '\0')
    {
	if(IS_SET(ch->pcdata->flags, PCFLAG_TOGGLE))
	{
		send_to_char("You are now toggled to kill!\n\r", ch);
		REMOVE_BIT(ch->pcdata->flags, PCFLAG_TOGGLE);
	}
	else
	{
		send_to_char("You are now toggled to stun!\n\r", ch);
		SET_BIT(ch->pcdata->flags, PCFLAG_TOGGLE);
	}
    }
    if(!str_cmp( arg, "stun" ) )
    {
	if(!IS_SET(ch->pcdata->flags, PCFLAG_TOGGLE))
	{
 		send_to_char("You are now toggled to stun!\n\r", ch);
                SET_BIT(ch->pcdata->flags, PCFLAG_TOGGLE);
	}
	else
		send_to_char("You are already set to stun!\n\r", ch);
    }
    if(!str_cmp( arg, "kill" ) )
    {
        if(IS_SET(ch->pcdata->flags, PCFLAG_TOGGLE))
        {
                send_to_char("You are now toggled to kill!\n\r", ch);
                REMOVE_BIT(ch->pcdata->flags, PCFLAG_TOGGLE);
        }
        else
                send_to_char("You are already set to kill!\n\r", ch);
    }

    return;
}
void do_makerifle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, bonus;
    bool checktool, checkdura, checkbatt, checkoven, checkmetal, checkplas;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, power, scope;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:


    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: makerifle <name>\n\r&w", ch);
                  return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

    	        checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkmetal = FALSE;
                checkplas = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_DURASTEEL)
          	    	checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_DURAPLAST)
                    checkplas = TRUE;
                  if (obj->item_type == ITEM_RARE_METAL)
                    checkmetal = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a rifle.\n\r", ch);
                   return;
                }

                if ( !checkdura )
                {
                   send_to_char( "&RYou need some steel to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a battery to power the oven.\n\r", ch);
                   return;
                }

                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat the materials.\n\r", ch);
                   return;
                }

                if ( !checkplas )
                {
                   send_to_char( "&RYou need plastic for the handle.\n\r", ch);
                   return;
                }

                if ( !checkmetal )
                {
                   send_to_char( "&RYou need a rare metal for those fine components.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makerifle]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a rifle.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 25 , do_makerifle , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makerifle );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makerifle]);
    vnum = 10431;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool 	= FALSE;
    checkdura 	= FALSE;
    checkbatt 	= FALSE;
    checkoven 	= FALSE;
    checkplas 	= FALSE;
    checkmetal 	= FALSE;
    power     	= 0;
    scope     	= 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_DURAPLAST && checkplas == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkplas = TRUE;
       }
       if (obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkmetal = TRUE;
       }
 	}

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makerifle]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )  || (
!checkplas ) || ( !checkmetal) )
    {
       send_to_char( "&RYou hold up your new rifle and aim at a leftover piece of plastic.\n\r", ch);
       send_to_char( "&RYou slowly squeeze the trigger hoping for the best...\n\r", ch);
       send_to_char( "&RYour rifle backfires destroying your weapon and burning your hand.\n\r", ch);
       learn_from_failure( ch, gsn_makerifle );
       return;
    }

	if( is_master_glm(ch)
		&& number_percent() < (get_curr_int(ch) >= 18 ? 3 : (get_curr_int(ch) >= 15 ? 2 : 1) )
		)
		bonus = 2;
	else
		bonus = 0;

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 2+level/10;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " rifle");
    if (bonus) strcat( buf, " masterwork" );
    obj->name = STRALLOC( strip_color(buf));
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = INIT_WEAPON_CONDITION;       /* condition  */
    obj->value[1] = (int) (level/50+6+bonus);      /* min dmg  */
    obj->value[2] = (int) (level/10+15+bonus);      /* max dmg  */
    obj->value[3] = WEAPON_RIFLE;
    obj->value[4] = 0;
    obj->value[5] = 20;
    obj->cost = obj->value[2]*50;
    obj->weight = obj->value[2]/2;

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created rifle.&w\n\r", ch);
    if (bonus)
    	send_to_char( "&YYou've exceeded superb craftsmanship. You've made a masterpiece!&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new rifle.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50,(exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );
         gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
    learn_from_success( ch, gsn_makerifle );
}

void do_webwho(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	bool computer = FALSE, power = FALSE;

        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0)
	   {
          	 computer = TRUE;
	   }
	   if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[2] ==1 )
	   {
		 power = TRUE;
	   }
        }
	if(IS_IMMORTAL(ch))
		computer = TRUE;

	if(!computer && !IS_SET( ch->in_room->room_flags2, ROOM_COMPUTER ) )
	{
		send_to_char("&RYou need a computer or a computer terminal to check who is currently in the world!\n\r", ch);
		return;
	}

	if(!power && !IS_SET( ch->in_room->room_flags2, ROOM_COMPUTER ))
	{
		send_to_char("&RMaybe you should 'bootup' your computer first?\n\r", ch );
		return;
	}

	do_who( ch, "");

}

void do_makelaptop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checktool,checkbattery, checkmonitor, checkmotherboard, checkharddrive, checkcase;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, power, scope;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;


    strcpy( arg , argument );

    switch( ch->substate )
    {
    	default:
				if ( ch->skill_level[ENGINEERING_ABILITY] < 50 )
				{
				  send_to_char( "&RYou don't have the engineering skills to know where to start.\n\r&w", ch);
                  return;
				}

    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: makelaptop <name>\n\r&w", ch);
                  return;
                }

   		checktool = FALSE;
		checkbattery = FALSE;
		checkmonitor = FALSE;
		checkmotherboard = FALSE;
		checkharddrive = FALSE;
		checkcase = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_FACTORY ) )
                {
                   send_to_char( "&RYou need to be in a factory or workshop to do that.\n\r", ch);
                   return;
                }
                if(strlen_color(arg) > 75)
                {
                   send_to_char("&RYou can't make items with names longer than 75 characters!\r\n", ch);
                   return;
                }
                if( is_profane(arg) )
                {
                        send_to_char("Your name has been tested and found to be profain in some way!\r\n", ch);
                        send_to_char("If this was not profain, and the filter made a mistake send a personal gnote to imms!\r\n", ch);
                        return;
                }

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_MONITOR)
          	    checkmonitor = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbattery = TRUE;
                  if (obj->item_type == ITEM_MOTHERBOARD)
                    checkmotherboard = TRUE;
                  if (obj->item_type == ITEM_HARDDRIVE)
                    checkharddrive = TRUE;
		  if( obj->item_type == ITEM_CASE)
		    checkcase = TRUE;
                }

                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a laptop.\n\r", ch);
                   return;
                }

                if ( !checkmonitor )
                {
                   send_to_char( "&RYou need a monitor to make a laptop.\n\r", ch);
                   return;
                }

                if ( !checkbattery )
                {
                   send_to_char( "&RYou need a battery to power the laptop.\n\r", ch);
                   return;
                }

                if ( !checkmotherboard )
                {
                   send_to_char( "&RYou need a motherboard for the laptop.\n\r", ch);
                   return;
                }

                if ( !checkharddrive )
                {
                   send_to_char( "&RYou need a hard drive for the computer.\n\r", ch);
                   return;
                }
		if ( !checkcase )
		{
		   send_to_char("&RYou need a case for the computer.\n\r", ch);
		   return;
		}

    	        chance = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makelaptop]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a laptop.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 25 , do_makelaptop , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makelaptop );
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
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }

    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makelaptop]);
    vnum = 10432;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }
    checktool = FALSE;
    checkbattery = FALSE;
    checkmonitor = FALSE;
    checkmotherboard = FALSE;
    checkharddrive = FALSE;
    checkcase   = FALSE;
    power     	= 0;
    scope     	= 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_BATTERY && checkbattery == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
	  checkbattery = TRUE;
       }
       if (obj->item_type == ITEM_MONITOR && checkmonitor == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkmonitor = TRUE;
       }
       if (obj->item_type == ITEM_MOTHERBOARD && checkmotherboard == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkmotherboard = TRUE;
       }
       if (obj->item_type == ITEM_HARDDRIVE && checkharddrive == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkharddrive = TRUE;
       }
       if (obj->item_type == ITEM_CASE && checkcase == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
	  checkcase = TRUE;
        }
 	}

    chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_makelaptop]) ;

    if ( number_percent( ) > chance*2  || ( !checktool ) || ( !checkbattery ) || ( !checkmonitor ) || (
!checkmotherboard )  || ( !checkharddrive ) || (!checkcase))
    {
       send_to_char("&RYou try to login to your new laptop, but it fizzles out.\n\r", ch);
       send_to_char("&RA foul smell and rich black smoke comes from the worthless machine.\n\r", ch);
       learn_from_failure( ch, gsn_makelaptop );
       return;
    }

    obj = create_object( pObjIndex, level );

    obj->item_type = ITEM_COMPUTER;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " computer"); /* Name fix - Funf */
    obj->name = STRALLOC( strip_color(buf) );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    if(ch->pcdata && ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name, "GLM"))
    {
		obj->value[0] = (int) (level/10 + get_curr_int(ch)/5 + 1);
		obj->value[1] = (int) (level/10 + get_curr_int(ch)/5 + 1);
	} else {
		obj->value[0] = (int) (level/10 + get_curr_int(ch)/5);
		obj->value[1] = (int) (level/10 + get_curr_int(ch)/5);
	}
    obj->cost = obj->value[1]*50;
    obj->weight = (int) obj->value[1]/3; /* Too heavy - Funf */

    obj = obj_to_char( obj, ch );

    send_to_char( "&GYou finish your work and hold up your newly created laptop.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new laptop.", ch,
         NULL, argument , TO_ROOM );

    {
         long long int xpgain;

         xpgain = UMIN( obj->cost*50 ,( exp_level(ch->skill_level[ENGINEERING_ABILITY]+1) - exp_level(ch->skill_level[ENGINEERING_ABILITY]) ) );

        if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
		xpgain = xpgain * 1.1;
	}
	 gain_exp(ch, xpgain, ENGINEERING_ABILITY);
         ch_printf( ch , "You gain %d engineering experience.", xpgain );
    }
    learn_from_success( ch, gsn_makelaptop );
}

void do_hackbank(CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    long long int amount = 0; /* Made long - Funf */
    long long int level = 0;
    CHAR_DATA *victim;
    OBJ_DATA *obj, *comp;
    bool comlink = FALSE, computer = FALSE;

    argument = one_argument( argument , arg1 );
    argument = one_argument( argument , arg2 );
    argument = one_argument( argument , arg3 );

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }

	if ( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You must be at a bank terminal to use this skill.\n\r", ch );
        return;
	}

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->pIndexData->item_type == ITEM_COMLINK)
          comlink = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0)
       {
          computer = TRUE;
	  if(obj->value[2] == 1)
		  comp = obj;
       }
    }

    if(!comlink)
    {
        send_to_char("&RYou need a comlink to hack!\n\r", ch);
        return;
    }
    if(!computer)
    {
        send_to_char("&RYou need a working computer to hack!\n\r", ch);
        return;
    }

    if(!comp)
    {
	send_to_char("&RYou need a computer which is turned on to hack!\r\n", ch);
	return;
    }

    if (comp->value[2] == 0 )
    {
        act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
	return;
    }

    if( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("&Chackbank: 'Account Person's Name' 'Add/Remove' 'Amount'\n\r", ch);
        return;
    }

    amount = atoll(arg3);
    if( ( ( victim = get_char_world(ch, arg1) ) == NULL ))
    {
        send_to_char("&RNo such player online for account!&W\n\r", ch);
        return;
    }
    if(IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
	send_to_char("&RThey probably wouldn't like that.\r\n", ch);
	return;
    }
 //   level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_hackbank]);
    level = ch->skill_level[FORCE_ABILITY];
    level = level*5000;
    if( get_age(victim) < 6)
    {
	send_to_char("&RThe victim must have played atleast 6 hours!\r\n", ch);
	return;
    }
    if( amount > level)
    {
        ch_printf(ch, "&RYou can only transfer up to %lld!", level);
        return;
    }
    if( amount < 0)
    {
        send_to_char("&RYou can't have a negitive amount of wulongs!\n\r", ch);
        return;
    }
    if( (victim->pcdata->bank + amount) > MAX_BANK) /* removed hard coded value - Funf */
    {
        send_to_char("&RThat would put the reciever's account over the limit!\n\r", ch);
        return;
    }
    if((victim->pcdata->bank - amount) < 0 && !str_prefix( arg2 , "remove" ))
    {
	send_to_char("&RThat would put the reciever's account under the limit!\n\r", ch);
	return;
    }
    if( amount == 0)
    {
        send_to_char("&R0 wulongs would be pointless!\n\r", ch);
        return;
    }
/*
    if ( str_prefix( arg2 , "add" ) || str_prefix( arg2 , "remove" ) )
    {
	send_to_char("&RYou can only add or remove from a person's bank account!\n\r", ch);
	return;
    }
*/
    if(get_age(victim) < 6)
    {
	send_to_char("&RVictim must have played 6 hours first!\r\n", ch);
	return;
    }
    WAIT_STATE( ch, 25 );

    act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

    if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackbounty]
	|| number_range(0,5) > 4)
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 40000, 0);

         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER)
               {
		/*
                   separate_obj( obj );
                   obj_from_char( obj );
                   extract_obj( obj );
		*/
		   if(obj->value[0] - 1 <= 0)
		   {
			obj->value[0]--;
			send_to_char( "Your computer was destroyed!\n\r", ch);
		   }
		   else
		   {
			obj->value[0]--;
			send_to_char("Your computer was damaged!\n\r", ch);
		   }

                   break;
               }
         }
	if(ch->perm_int < 15 && number_range(0,4) >= 3)
		ch->inttrain++;
         return;
    }
   if ( !str_prefix( arg2 , "add" ) )
   {

	victim->pcdata->bank += amount;
        ch_printf( ch, "You were able to hack %lld wulongs to %s's account!\n\r", amount, victim->name);
        gain_exp(ch, (ch->top_level/2+60), FORCE_ABILITY);
        ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+60) );
	learn_from_success( ch, gsn_hackbank);
	return;
   }
   if ( !str_prefix( arg2 , "remove" ) )
   {
	victim->pcdata->bank -= amount;
	ch_printf( ch, "You were able to hack %lld wulongs from %s's acount!\n\r", amount, victim->name);
        gain_exp(ch, (ch->top_level/2+60), FORCE_ABILITY);
        ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+60) );
	learn_from_success( ch, gsn_hackbank);
	return;
   }

   send_to_char("&RYou can only add or remove from a person's bank account!\n\r", ch);
   return;
}

void do_hackclanaccount(CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    long long int amount; /* made long - Funf */
    int level = 0;
    CLAN_DATA *clan;
    OBJ_DATA *obj;
    bool comlink = FALSE, computer = FALSE, power = FALSE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }

	if ( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You must be at a bank terminal to use this skill.\n\r", ch );
        return;
	}

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->pIndexData->item_type == ITEM_COMLINK)
          comlink = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0)
          computer = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[2]== 1)
	  power = TRUE;
       if( computer && comlink && power)
	  break;
    }

    if(!comlink)
    {
        send_to_char("&RYou need a comlink to hack!\n\r", ch);
        return;
    }

    if(!computer)
    {
        send_to_char("&RYou need a working computer to hack!\n\r", ch);
        return;
    }
    if(!power)
    {
	send_to_char("&RMaybe you should 'bootup' your computer first?\n\r", ch);
	return;
    }
	/*
    if (obj->value[2] == 0 )
    {
         act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
	 return;
    }
	*/
    if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char("&RHackclanaccount 'add/remove' 'name' # (Number for Add only)\n\r", ch);
	return;
    }
    clan = get_clan( arg2);
    if( !clan)
    {
	send_to_char("No such clan exsists!\n\r", ch);
	return;
    }
    if(!IS_NPC(ch) && ch->pcdata->hackcount >= 5)
    {
        send_to_char("&RHacking any more is too much of a risk!\r\n", ch);
        return;
    }

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_hackclanaccount]);
    level = level*5000;
    amount = atoi(arg3);
    if( amount > level)
    {
	send_to_char("You can't hack that much wulongs!\n\r", ch);
	return;
    }
    WAIT_STATE( ch, 25 );

    act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

    if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackclanaccount]
	|| number_range(0,5) > 4)
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 10000, 0);
         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
               {
		   if( obj->value[0] -1 <= 0)
		   {
			send_to_char("Your computer was destroyed!\n\r", ch);
			return;
		   }
		   else
		   {
			send_to_char("Your computer was damaged!\n\r", ch);
			return;
		   }
		   obj->value[0]--;
                   break;
               }
         }
	if(ch->perm_int < 15 && number_range(0,4) >= 3)
		ch->inttrain++;
         return;
   }
   if ( !str_prefix( arg1 , "add" ) )
   {
	if( amount < 5000)
	{
		send_to_char("&RYou need to put at least 5,000 wulongs in their account\n\r", ch);
		return;
	}
	if( clan->funds > MAX_BANK - MAX_BANK / 4 ) /* tweaked check and removed hard coded value - Funf */
	{
		send_to_char("&RYou can't add anymore to this account!\n\r", ch);
		return;
	}
	clan->funds += amount;
   }
   else if ( !str_prefix( arg1 , "remove" ) )
   {
	if( amount <= 0 || amount > 500000)
	{
		send_to_char("You can't remove that much!\n\r", ch);
		return;
	}
	clan->funds -= amount;
   }
   else
   {
	send_to_char("&RInvalid selection!\n\r", ch);
	return;
   }
   gain_exp(ch, (ch->top_level/2+40), FORCE_ABILITY);
   ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+40) );
   learn_from_success( ch, gsn_hackclanaccount);
   ch->pcdata->hackcount++;
   return;
}

void do_newwhois( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  BOUNTY_DATA *bounty;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *obj_next;
  bool found = FALSE, power;

  buf[0] = '\0';

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }

  if(IS_NPC(ch))
    return;
        for( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {

                if ( obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
                {
                        found = TRUE;
                }
		if ( obj->item_type == ITEM_COMPUTER && obj->value[2]==1)
		{
			power = TRUE;
		}
		if(found && power)
			break;
        }
  if(!found)
  {
	send_to_char("You need a working computer to hack!\n\r", ch);
	return;
  }
  if(!power)
  {
	send_to_char("Maybe you should 'bootup' your computer first?\n\r", ch);
	return;
  }
  if(obj->value[2] == 0)
  {
         act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
         return;
  }


  if(argument[0] == '\0')
  {
    send_to_char("You must input the name of a player online.\n\r", ch);
    return;
  }

  strcat(buf, "0.");
  strcat(buf, argument);
  if( ( ( victim = get_char_world(ch, buf) ) == NULL ))
  {
    send_to_char("&C----------=<(&GConnecting...&C)>=----------&W\n\r", ch);
    send_to_char("&C----------&RAccessed WHOIS file&C----------&W\n\r", ch);
    send_to_char("Whois file: &RDENIED&w\n\r", ch);
    send_to_char("&GPossible cause is player is not on-line!&W\n\r", ch);
    send_to_char("&C----------=<(&GDisconnected.&C)>=----------&W\n\r", ch);
    return;
  }

  if(IS_NPC(victim))
  {
    send_to_char("That's not a player!\n\r", ch);
    return;
  }
                if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackwho] )
                {
                        send_to_char( "Your attempt to hack fails!\n\r", ch );
                        //place_bounty( ch, ch, 10000, 0); No longer illegal - Funf 2007-11-13
                       if(obj->value[0]-1 > 0)
                        {
                                obj->value[0]--;
                                send_to_char("&RYour computer was damaged!\n\r", ch);
                        }
                        else
                        {
                                obj->value[0]--;
                                send_to_char( "Your computer was destroyed!\n\r", ch);
                        }
                        if(ch->perm_int < 15 && number_range(0,4) >= 3)
                                ch->inttrain++;
			WAIT_STATE(ch, 5);
                        return;
                }
                 send_to_char( "You successfully hacked information.\n\r", ch );

				 if(ch->skill_level[FORCE_ABILITY] < 30)
				 {
					gain_exp(ch, (ch->top_level/2), FORCE_ABILITY);
					ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2) );
				 }
		WAIT_STATE(ch, 5);
                learn_from_success(ch, gsn_hackwho);

ch_printf( ch,
"&C----------=<(&GConnecting...&C)>=---------&W&W\n\r");
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RWhois File:&G ACCESSED&c]&C=-=-=-=-=-=-=-\n\r" );
ch_printf( ch, "&C|&RName        &R[&W%-15s&R]\n\r", victim->name );
ch_printf( ch, "&C|&RSex         &R[&W%-17s&R]\n\r", victim->sex ==
SEX_MALE ? "&CMale" :
        victim->sex == SEX_FEMALE ? "&PFemale" : "&GNeutral" );
ch_printf( ch, "&C|&RRace        &R[&W%-15s&R]\n\r",
npc_race[victim->race] );

ch_printf( ch, "&C|&RAge	     &R[&W%-15d&R]\n\r",
victim->pcage );
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RCLAN INFO&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=\n\r" );
if ( victim->pcdata->clan )
{
	if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
	{
	ch_printf( ch, "&C|&RClan        &R[&WRBH            &R]\n\r" );
	ch_printf( ch, "&C|&RPosition    &R[&W&cMember     &W&R]\n\r" );
	}
	else
	{
	ch_printf( ch, "&C|&RClan        &R[&W%-15s&R]\n\r",
	victim->pcdata->clan->name );
	ch_printf( ch, "&C|&RPosition    &R[&W%-15s&R]\n\r",
	victim->pcdata->clan->leader == victim->name ? "&CL&ceader&W" :

	victim->pcdata->clan->number1 == victim->name ? "&CF&cirst&W" :

	victim->pcdata->clan->number2 == victim->name ? "&CS&cecond&W" : "&cMember&W" );
	}
}
else
ch_printf( ch, "&C|&RClan        &R[&W%-15s&R]\n\r", "No clan available!" );
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RBIOGRAPHY&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=\n\r" );
if(victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
ch_printf( ch, "%s\n\r", victim->pcdata->bio );
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RHOMEPAGE&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r" );
if(victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
ch_printf(ch, "%s\n\r",
	victim->pcdata->homepage);
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RBOUNTY&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r" );
bounty = get_bounty( victim->name );
if ( bounty && !IS_SET(bounty->flags, HIDDEN_BOUNTY) )
	ch_printf(ch, "%s has a bounty of: %s\n\r",victim->name, num_punct(bounty->amount) );
else
	ch_printf(ch, "They do not have a bounty.\n\r" );
ch_printf( ch,
"&C-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=&W\n\r");
ch_printf( ch,
"&C----------=<(&GDisconnected.&C)>=----------&W\n\r");

  if(IS_IMMORTAL(ch))
  {

send_to_char("-------------------------------------------------\n\r",ch);

    send_to_char("&RIMMORTALS&W:\n\r", ch);

    if ( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
        ch_printf(ch, "%s was authorized by %s. Level %d.\n\r",
                victim->name, victim->pcdata->authed_by, victim->top_level);

    ch_printf(ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\n\r",
                victim->name, victim->pcdata->mkills,victim->pcdata->mdeaths );
    if ( victim->pcdata->pkills || victim->pcdata->pdeaths )
        ch_printf(ch, "%s has killed %d players, and been killed by a player %d times.\n\r",
                victim->name, victim->pcdata->pkills,victim->pcdata->pdeaths );
    if ( victim->pcdata->illegal_pk )
        ch_printf(ch, "%s has committed %d illegal player kills.\n\r",
                victim->name, victim->pcdata->illegal_pk );

    ch_printf(ch, "%s is %shelled at the moment.\n\r",
        victim->name,
        (victim->pcdata->release_date == 0) ? "not " : "");

    ch_printf(ch, "%s is %ssilenced at the moment.\n\r",
        victim->name,
        (victim->pcdata->unsilence_date == 0) ? "not " : "");


    if(victim->pcdata->release_date != 0)
      ch_printf(ch, "%s was helled by %s, and will be released on %24.24s.\n\r",
        victim->sex == SEX_MALE ? "He" :
        victim->sex == SEX_FEMALE ? "She" : "It",
        victim->pcdata->helled_by,
        ctime(&victim->pcdata->release_date));

    if(victim->pcdata->unsilence_date != 0)
      ch_printf(ch, "%s was silenced by %s, and will be released on %24.24s.\n\r",
        victim->sex == SEX_MALE ? "He" :
        victim->sex == SEX_FEMALE ? "She" : "It",
        victim->pcdata->silenceed_by,
        ctime(&victim->pcdata->unsilence_date));


    if(get_trust(victim) < get_trust(ch))
    {
      sprintf(buf2, "list %s", buf);
      do_comment(ch, buf2);
    }

    if(IS_SET(victim->act, PLR_SILENCE) || IS_SET(victim->act, PLR_NO_EMOTE)
    || IS_SET(victim->act, PLR_NO_TELL) )
    {
      sprintf(buf2, "This player has the following flags set:");
      if(IS_SET(victim->act, PLR_SILENCE))
        strcat(buf2, " silence");
      if(IS_SET(victim->act, PLR_NO_EMOTE))
        strcat(buf2, " noemote");
      if(IS_SET(victim->act, PLR_NO_TELL) )
        strcat(buf2, " notell");
      strcat(buf2, ".\n\r");
      send_to_char(buf2, ch);
    }
    if ( victim->desc && victim->desc->host[0]!='\0' )   /* added by Gorog*/
    {
      sprintf (buf2, "%s's IP info: %s ", victim->name,victim->desc->hostip);
      if (get_trust(ch) >= LEVEL_GOD)
      {
        strcat (buf2, victim->desc->user);
        strcat (buf2, "@");
        strcat (buf2, victim->desc->host);
      }
      strcat (buf2, "\n\r");
      send_to_char(buf2, ch);
    }
    if (get_trust(ch) >= LEVEL_GOD && get_trust(ch) >= get_trust( victim )&& victim->pcdata )
    {
        sprintf (buf2, "Email: %s\n\r" , victim->pcdata->email );
        send_to_char(buf2, ch);
    }
  }
}

void do_background( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  BOUNTY_DATA *bounty;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  OBJ_DATA *obj, *obj_next;
  bool found = FALSE;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if(obj->item_type == ITEM_COMPUTER && obj->value[0] > 0)
        {
                found = TRUE;
		break;
        }
    }


    if( !found )
    {
        send_to_char("&RYou need a working computer for this!\n\r", ch);
        return;
    }


  buf[0] = '\0';

  if(IS_NPC(ch))
    return;
  if( !ch->pcdata || !ch->pcdata->clan)
  {
	send_to_char("You aren't even in a clan!\r\n", ch);
	return;
  }
    if ( ch->pcdata && ch->pcdata->clan && ( str_cmp(ch->pcdata->clan->name, "ISSP") ) )
        {
                send_to_char("Only ISSP can check up on backgrounds legally!\n\r", ch);
                return;
        }

  if(argument[0] == '\0')
  {
    send_to_char("You must input the name of a player online.\n\r", ch);
    return;
  }

  strcat(buf, "0.");
  strcat(buf, argument);
  if( ( ( victim = get_char_world(ch, buf) ) == NULL ))
  {
    send_to_char("&C----------=<(&GConnecting...&C)>=----------&W\n\r", ch);
    send_to_char("&C----------&RAccessed Background Check &C----------&W\n\r", ch);
    send_to_char("Whois file: &RDENIED&w\n\r", ch);
    send_to_char("&GPossible cause is player is not on-line!&W\n\r", ch);
    send_to_char("&C----------=<(&GDisconnected.&C)>=----------&W\n\r", ch);
    return;
  }

  if(IS_NPC(victim))
  {
    send_to_char("That's not a player!\n\r", ch);
    return;
  }
ch_printf( ch,
"&C----------=<(&GConnecting...&C)>=---------&W&W\n\r");
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RISSP File:&G ACCESSED&c]&C=-=-=-=-=-=-=-\n\r" );
ch_printf( ch, "&C|&RName        &R[&W%-15s&R]\n\r", victim->name );
ch_printf( ch, "&C|&RSex         &R[&W%-17s&R]\n\r", victim->sex ==
SEX_MALE ? "&CMale" :
        victim->sex == SEX_FEMALE ? "&PFemale" : "&GNeutral" );
ch_printf( ch, "&C|&RRace        &R[&W%-15s&R]\n\r",
npc_race[victim->race] );

ch_printf( ch, "&C|&RAge         &R[&W%-15d&R]\n\r",
victim->pcage );
ch_printf( ch, "&C|&RArrested    &R[&W%-3d       Times&R]\r\n", victim->pcdata->arrestcount);
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RCLAN INFO&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=\n\r" );
if ( victim->pcdata->clan )
{
        if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
        {
        ch_printf( ch, "&C|&RClan        &R[&WRBH            &R]\n\r" );
        ch_printf( ch, "&C|&RPosition    &R[&W&cMember     &W&R]\n\r" );
        }
        else
        {
        ch_printf( ch, "&C|&RClan        &R[&W%-15s&R]\n\r",
        victim->pcdata->clan->name );
        ch_printf( ch, "&C|&RPosition    &R[&W%-15s&R]\n\r",
        victim->pcdata->clan->leader == victim->name ? "&CL&ceader&W" :

        victim->pcdata->clan->number1 == victim->name ? "&CF&cirst&W" :

        victim->pcdata->clan->number2 == victim->name ? "&CS&cecond&W" : "&cMember&W" );
        }
}
else
ch_printf( ch, "&C|&RClan        &R[&W%-15s&R]\n\r", "No clan available!" );
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RBIOGRAPHY&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=\n\r" );
if(victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
ch_printf( ch, "%s\n\r", victim->pcdata->bio );
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RHOMEPAGE&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r" );
if(victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
ch_printf(ch, "%s\n\r",
        victim->pcdata->homepage);
ch_printf( ch,
"&C-=-=-=-=-=-=-=&c[&RBOUNTY&c]&C=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r" );
bounty = get_bounty( victim->name );
if ( bounty && (!IS_SET(bounty->flags, HIDDEN_BOUNTY) || IS_IMMORTAL(ch)) )
	ch_printf(ch, "%s has a bounty of: %s\n\r",victim->name, num_punct(bounty->amount) );
else
        ch_printf(ch, "They do not have a bounty.\n\r" );
ch_printf( ch,
"&C-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=&W\n\r");
ch_printf( ch,
"&C-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=&W\n\r");
ch_printf( ch,
"&C----------=<(&GDisconnected.&C)>=----------&W\n\r");
return;
}





void do_commandeer( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *obj_next;
    bool comlink = FALSE;
    SHIP_DATA *ship;
    int chance;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];

    	        if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }

    	        if ( ship->class > SHIP_PLATFORM )
    	        {
    	            send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	            return;
    	        }

    	        if ( (ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL )
    	        {
    	            send_to_char("&RYou don't seem to be in the pilot seat!\n\r",ch);
    	            return;
    	        }

    	        if ( check_pilot( ch , ship ) )
    	        {
    	            send_to_char("&RWhat would be the point of that!\n\r",ch);
    	            return;
    	        }

    	        if ( ship->type == MOB_SHIP && get_trust(ch) < 102 )
    	        {
    	            send_to_char("&RThis ship isn't pilotable by mortals at this point in time...\n\r",ch);
    	            return;
    	        }

                if  ( ship->class == SHIP_PLATFORM )
                {
                   send_to_char( "You can't do that here.\n\r" , ch );
                   return;
                }

    	        if ( ship->lastdoc != ship->location )
                {
                     send_to_char("&rYou don't seem to be docked right now.\n\r",ch);
                     return;
                }

    	        if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
    	        {
    	            send_to_char("The ship is not docked right now.\n\r",ch);
    	            return;
    	        }

                if ( ship->shipstate == SHIP_DISABLED )
    	        {
    	            send_to_char("The ships drive is disabled .\n\r",ch);
    	            return;
    	        }

		if(!IS_NPC(ch) && !ch->pcdata->clan)
		{
                        send_to_char("&ROnly ISSP can commandeer ships legally!\n\r", ch);
                        return;
                }

		if( !IS_NPC(ch) && ch->pcdata && ch->pcdata->clan
		    && (str_cmp(ch->pcdata->clan->name, "ISSP")))
		{
			send_to_char("&ROnly ISSP can commandeer ships legally!\n\r", ch);
			return;
		}
		for( obj = ch->last_carrying; obj; obj = obj->prev_content)
		{
			if(obj->item_type == ITEM_COMLINK)
			{
				comlink = TRUE;
				break;
			}
		}
		if(!comlink)
		{
			send_to_char("&RYou need a communication device to commandeer ships!\n\r", ch);
			return;
		}
	    if ( ship->class == FIGHTER_SHIP)
			chance = IS_NPC(ch) ? ch->top_level
				 : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
		if(ship->class == FIGHTER_SHIP && chance <= 0)
		{
			send_to_char("&RYou lack the skill to pilot this vessel!\n\r", ch);
			return;
		}
		if ( ship->class == MIDSIZE_SHIP)
			chance = IS_NPC(ch) ? ch->top_level
				 : (int)  (ch->pcdata->learned[gsn_midships]) ;
		if(ship->class == MIDSIZE_SHIP && chance <=0)
		{
				send_to_char("&RYou lack the skill to pilot this vessel!\n\r", ch);
				return;
		}

		if ( ship->class == CAPITAL_SHIP)
			chance = IS_NPC(ch) ? ch->top_level
				 : (int) (ch->pcdata->learned[gsn_capitalships]);
		if(ship->class == CAPITAL_SHIP && chance <=0)
		{
				send_to_char("&RYou lack the skill to pilot this vessel!\n\r", ch);
				return;
		}


		sprintf( buf1,"&w%s says into is communication device, '&CISSP Officer %s requesting codes to commandeer vessel %s.&w'",
			ch->name,ch->name,  ship->name);
		act(AT_PLAIN, buf1, ch, NULL, NULL, TO_ROOM);
		sprintf( buf1, "&wYou say into your communication device, '&CISSP Officer %s requesting codes to commandeer vessel %s.&w'",
			 ch->name, ship->name);
		act(AT_PLAIN, buf1, ch, NULL, NULL, TO_CHAR);
		act(AT_PLAIN, "&wA voice on $n's comlink says, '&COkay Officer, permission granted.&w'",
			ch, NULL, NULL, TO_ROOM);
		act(AT_PLAIN, "&wA voice on your comlink says, '&COkay Officer, permission granted.&w'",
			ch, NULL, NULL, TO_CHAR);




    		   if (ship->hatchopen)
    		   {
    		     ship->hatchopen = FALSE;
    		     sprintf( buf , "The hatch on %s closes." , ship->name);
       	             echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
       	             echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch slides shut." );
       	           }
    		   set_char_color( AT_GREEN, ch );
    		   send_to_char( "Launch sequence initiated.\n\r", ch);
    		   act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch,
		        NULL, argument , TO_ROOM );
		   echo_to_ship( AT_YELLOW , ship , "The ship hums as it lifts off the ground.");
    		   sprintf( buf, "%s begins to launch.", ship->name );
    		   echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
    		   ship->shipstate = SHIP_LAUNCH;
    		   ship->currspeed = ship->realspeed;
    	   	return;

}


void do_bankfreeze(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	one_argument(argument, buf);

	if(buf == '\0')
	{
		send_to_char("Freeze whom?\r\n", ch);
		return;
	}

	if((victim = get_char_world(ch, buf)) == NULL)
	{
		send_to_char("You can only freeze players who are online!\r\n", ch);
		return;
	}
	if(IS_NPC(victim))
	{
		send_to_char("Not on NPCs\r\n", ch);
		return;
	}

	ch_printf(ch, "%s now has a freeze %s %s bank account.\r\n",
		victim->name,
		(IS_SET(victim->pcdata->flags, PCFLAG_FREEZE))? "removed from" : "added to",
		(victim->sex == 1)? "his" : "her");
	ch_printf(victim, "Your bank account has been %s.\r\n",
		(IS_SET(victim->pcdata->flags, PCFLAG_FREEZE))? "unfrozen" : "frozen");
	TOGGLE_BIT(victim->pcdata->flags, PCFLAG_FREEZE);

	return;
}




void do_attend( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA   *medpac;
   CHAR_DATA  *victim;
   int         heal, lag;


   if ( ch->position == POS_FIGHTING )
   {
         send_to_char( "You can't do that while fighting!\n\r",ch );
         return;
   }

   medpac = get_eq_char( ch, WEAR_HOLD );
   if ( !medpac || medpac->item_type != ITEM_MEDPAC )
   {
         send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
         return;
   }

   if ( medpac->value[0] <= 0 )
   {
         send_to_char( "Your Medical Kit seems to be empty.\n\r",ch );
         return;
   }

   victim = get_char_room( ch, argument );
   if(!victim)
   {
	send_to_char("That player isn't in the room!\r\n", ch);
	return;
   }
   if(victim == ch)
   {
	send_to_char("You can't attend yourself!\r\n", ch);
	return;
   }
   if(victim->position == POS_FIGHTING)
   {
	send_to_char("They are busy fighting!\r\n", ch);
	return;
   }

   if(ch->adren > 0)
   {
         send_to_char("Your heart is racing to much!\r\n", ch);
         return;
   }
   if(victim->adren > 0)
   {
        send_to_char("Your target has too much adrenaline!\r\n", ch);
        return;
   }

   if(victim->hit < 700)
   {
	ch_printf( ch, "%s is far too injured to just attend %s!\r\n",
		victim->name, (victim->sex == 1)? "him" : "her");
	return;
    }
    if(victim->hit >= victim->max_hit)
    {
	ch_printf(ch, "%s doesn't need your help.\r\n",
		victim->name);
	return;
    }
    medpac->value[0]--;
    if((number_range(1,100) - get_curr_int(victim) - (ch->pcdata->learned[gsn_attend]/2)) > 80)
    {
	act(AT_DGREEN, "You try to attend $N but make too many mistakes.", ch, NULL, victim, TO_CHAR);
	act(AT_DGREEN, "$n tries to attend to your wounds but makes too many mistakes.", ch, NULL, victim, TO_VICT);
	act(AT_DGREEN, "$n tries to attend to $N but seems to struggle and make mistakes.", ch, NULL, victim, TO_NOTVICT);
	return;
    }
    lag = 15;
    if(ch->pcdata->learned[gsn_master_medic])
    {
	lag -= (ch->pcdata->learned[gsn_master_medic]/10);
	learn_from_success(ch, gsn_master_medic);
    }
    act(AT_DGREEN, "You attend to $N's wounds.", ch, NULL, victim, TO_CHAR);
    act(AT_DGREEN, "$n attends to your wounds.", ch, NULL, victim, TO_VICT);
    act(AT_DGREEN, "$n attends to $N's wounds.", ch, NULL, victim, TO_NOTVICT);
    learn_from_success(ch, gsn_attend);
    heal = number_range(5, 25) + get_curr_int(ch);
    victim->hit += heal;
    if(victim->hit > victim->max_hit)
	victim->hit = victim->max_hit;
    WAIT_STATE( ch, lag);
    long long int xpgain;
    xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
    xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
    xpgain = xpgain/6;
    if(ch->skill_level[MEDIC_ABILITY] < 5)
	xpgain += 50;

    if(ch->skill_level[MEDIC_ABILITY] > 20)
            xpgain = 0;
    if(xpgain > 0)
    {
	ch_printf( ch, "You gain %lld experience from your success!\r\n", xpgain);
        gain_exp(ch, xpgain, MEDIC_ABILITY);
    }
    return;
}




void do_bandage( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA   *medpac;
   CHAR_DATA  *victim;
   int         heal, lag;


   if ( ch->position == POS_FIGHTING )
   {
         send_to_char( "You can't do that while fighting!\n\r",ch );
         return;
   }

   medpac = get_eq_char( ch, WEAR_HOLD );
   if ( !medpac || medpac->item_type != ITEM_MEDPAC )
   {
         send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
         return;
   }

   if ( medpac->value[0] <= 0 )
   {
         send_to_char( "Your Medical Kit seems to be empty.\n\r",ch );
         return;
   }

   victim = get_char_room( ch, argument );
   if(!victim)
   {
	send_to_char("That player isn't in the room!\r\n", ch);
	return;
   }
   if(victim == ch)
   {
	send_to_char("You can't bandage yourself!\r\n", ch);
	return;
   }
   if(victim->position == POS_FIGHTING)
   {
	send_to_char("They are busy fighting!\r\n", ch);
	return;
   }
   if(victim->hit < 500)
   {
	ch_printf( ch, "%s is far too injured to just bandage %s!\r\n",
		victim->name, (victim->sex == 1)? "him" : "her");
	return;
    }
    if(victim->hit >= victim->max_hit)
    {
	ch_printf(ch, "%s doesn't need your help.\r\n",
		victim->name);
	return;
    }
    if(ch->pcdata->learned[gsn_bandage] < 50)
	send_to_char("You take a deep breath and begin.\r\n", ch);

    medpac->value[0]--;
    if((number_range(20,150) - get_curr_int(victim) - (ch->pcdata->learned[gsn_bandage]/2)) > 80)
    {
	act(AT_DGREEN, "You try to bandage $N but make too many mistakes.", ch, NULL, victim, TO_CHAR);
	act(AT_DGREEN, "$n tries to bandage to your wounds but makes too many mistakes.", ch, NULL, victim, TO_VICT);
	act(AT_DGREEN, "$n tries to bandage to $N but seems to struggle and make mistakes.", ch, NULL, victim, TO_NOTVICT);
	return;
    }
    lag = 15;
    if(ch->pcdata->learned[gsn_master_medic])
    {
	lag -= (ch->pcdata->learned[gsn_master_medic]/10);
	learn_from_success(ch, gsn_master_medic);
    }
    WAIT_STATE(ch, lag);
    act(AT_DGREEN, "You bandage $N's wounds.", ch, NULL, victim, TO_CHAR);
    act(AT_DGREEN, "$n bandages your wounds.", ch, NULL, victim, TO_VICT);
    act(AT_DGREEN, "$n bandages $N's wounds.", ch, NULL, victim, TO_NOTVICT);
    learn_from_success(ch, gsn_bandage);
    heal = number_range(30, 45) + get_curr_int(ch);
    victim->hit += heal;
    if(victim->hit > victim->max_hit)
	victim->hit = victim->max_hit;
    long long int xpgain;
    xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
    xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
    xpgain = xpgain/6;
    if(ch->skill_level[MEDIC_ABILITY] > 25)
                xpgain = 0;
    if(xpgain > 0)
    {
	ch_printf( ch, "You gain %lld experience from your success!", xpgain);
        gain_exp(ch, xpgain, MEDIC_ABILITY);
    }
    learn_from_success(ch, gsn_bandage);
    return;
}



void do_heal(CHAR_DATA *ch, char *argument)
{
        char buf[MAX_STRING_LENGTH];
        int time, heal;
	OBJ_DATA   *medpac;
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

        strcpy( arg, argument );

        switch( ch->substate )
        {
		default:


			if(arg[0] == '\0')
			{
				send_to_char("&RSyntax: Heal (Target)\r\n",ch);
				return;
			}
			if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    			{
        			send_to_char( "They aren't here.\r\n", ch );
        			return;
    			}
			if(victim == ch)
			{
				send_to_char("You can't heal yourself!\r\n", ch);
				return;
			}
			if(victim->hit < 300)
			{
				send_to_char("They are far too injured to try to use heal on them!\r\n", ch);
				return;
			}
			if(victim->hit > 999)
			{
				send_to_char("They don't need healing.\r\n", ch);
				return;
			}
			if(victim->position == POS_FIGHTING)
			{
				send_to_char("They are busy fighting!\r\n", ch);
				return;
			}
			medpac = get_eq_char( ch, WEAR_HOLD );
   			if ( !medpac || medpac->item_type != ITEM_MEDPAC )
   			{
        		        send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
         			return;
			}

			if(medpac->value[0] <= 0)
			{
				send_to_char("Your Medical Kit is empty!\r\n", ch);
				return;
			}

			time = 15;

			if(ch->pcdata->learned[gsn_master_medic])
			{
				time -= (ch->pcdata->learned[gsn_master_medic]/10);
				learn_from_success(ch, gsn_master_medic);
			}

			act(AT_DGREEN, "$n begins to try and heal $N.", ch, NULL, victim, TO_NOTVICT);
			act(AT_DGREEN, "You begin to try and heal $N.", ch, NULL, victim, TO_CHAR);
			act(AT_DGREEN, "$n begins to try and heal you.", ch, NULL, victim, TO_VICT);
			add_timer ( ch, TIMER_DO_FUN, time, do_heal, 1);
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
			act(AT_DGREEN, "$n gets distracted and stops trying to heal.", ch, NULL, NULL, TO_ROOM);
			act(AT_DGREEN, "You get distracted and stop trying to heal.", ch, NULL, NULL, TO_CHAR);
			return;
	}
	ch->substate = SUB_NONE;



	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
        	send_to_char( "They aren't here.\n\r", ch );
        	return;
    	}
	if(victim->hit < 300)
	{
		send_to_char("They are too injured to use heal on!\r\n", ch);
		return;
	}

        medpac = get_eq_char( ch, WEAR_HOLD );
        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
        {
                 send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
                 return;
        }

        medpac->value[0]--;
    	if((number_range(20,120) - get_curr_int(victim) - (ch->pcdata->learned[gsn_heal]/2)) > 80)
    	{
        	act(AT_DGREEN, "You try to heal $N but make too many mistakes.", ch, NULL, victim, TO_CHAR);
        	act(AT_DGREEN, "$n tries to heal your wounds but makes too many mistakes.", ch, NULL, victim, TO_VICT);
        	act(AT_DGREEN, "$n tries to heal $N but seems to struggle and make miostakes.", ch, NULL, victim, TO_NOTVICT);
        	return;
	}

	act(AT_DGREEN, "You heal $N's wounds.", ch, NULL, victim, TO_CHAR);
        act(AT_DGREEN, "$n heals some of your wounds.", ch, NULL, victim, TO_VICT);
        act(AT_DGREEN, "$n heals $N's wounds.", ch, NULL, victim, TO_NOTVICT);
        learn_from_success(ch, gsn_heal);
        heal = number_range(45, 70) + get_curr_int(ch);
        victim->hit += heal;
        if(victim->hit > victim->max_hit)
        	victim->hit = victim->max_hit;
		long long int xpgain;
       	xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
       	xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
	xpgain = xpgain/6;
        if(ch->skill_level[MEDIC_ABILITY] > 60)
                      xpgain = 0;
       	if(xpgain > 0)
       	{
             ch_printf( ch, "You gain %lld experience from your success!", xpgain);
       	     gain_exp(ch, xpgain, MEDIC_ABILITY);
    	}
	return;
}


void do_afkmessage(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if(IS_NPC(ch))
		return;

	if(argument[0] == '\0')
	{
		send_to_char( "Syntax: AFKMessage (Message) or AFKMessage 'Default' to clear.\r\n", ch);
		return;
	}
	if( !str_cmp( argument, "default"))
	{
		ch->pcdata->afkmessage = STRALLOC("");
		send_to_char("Message set back to default.\r\n", ch);
		return;
	}
	if( strlen(argument) > 50)
		argument[50] = '\0';
	if(strlen(argument) < 5)
	{
		send_to_char("Message must be atleast 5 characters long!\r\n", ch);
		return;
	}

	if ( isalpha(argument[0]) || isdigit(argument[0]) )
    	{
        	buf[0] = ' ';
        	strcpy( buf+1, argument );
    	}
    	else
    	    strcpy( buf, argument );

	STRFREE(ch->pcdata->afkmessage);
	ch->pcdata->afkmessage = STRALLOC( strip_color(buf) );
	send_to_char("New AFK Message set!\r\n", ch);
	return;
}

void do_self_medicate( CHAR_DATA *ch, char *argument)
{
	int timer = 0;
	OBJ_DATA   *medpac;


	switch( ch->substate)
	{
		default:


			medpac = get_eq_char( ch, WEAR_HOLD );
   			if ( !medpac || medpac->item_type != ITEM_MEDPAC )
   			{
         			send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
        	 		return;
   			}
			if(ch->adren > 0)
			{
				send_to_char("You heart is racing to fast for this!\r\n", ch);
				return;
			}
   			if ( medpac->value[0] <= 0 )
   			{
   			      send_to_char( "Your Medical Kit seems to be empty.\n\r",ch );
         		      return;
   			}
			if(ch->hit > 970)
			{
				send_to_char("You are far to healthy to gain anything from this!\r\n", ch);
				return;
			}
			timer = 6;
			if(ch->pcdata->learned[gsn_master_medic])
			{
				timer -= 2;
				learn_from_success(ch, gsn_master_medic);
			}

			add_timer(ch, TIMER_DO_FUN, 1, do_self_medicate, 1);
			act(AT_GREEN, "You start to dole out some medication for yourself", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n startes to dole out some medication.", ch, NULL, NULL, TO_ROOM);
		return;
		case 1:
			act(AT_GREEN, "You down the medication.", ch, NULL, NULL, TO_CHAR);
			act(AT_GREEN, "$n downs the medication.",ch, NULL, NULL, TO_ROOM);
                        medpac = get_eq_char( ch, WEAR_HOLD );
                        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
                        {
                                send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
                                return;
                        }

        		medpac->value[0]--;
     		        if((number_range(20,110) - get_curr_int(ch) - (ch->pcdata->learned[gsn_self_medicate]/2)) > 80)
        		{
                		act(AT_DGREEN, "You gasp as you take a bad dosage of medicine", ch, NULL, NULL, TO_CHAR);
                		act(AT_DGREEN, "$n gasps as $e took a bad dosage of medicine", ch, NULL, NULL, TO_ROOM);
                		return;
        		}


			if(ch->hit <= 970)
				ch->hit += 30;

			learn_from_success( ch, gsn_self_medicate);
			long long int xpgain;
       			xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
        		xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
			xpgain = xpgain/6;
			if(ch->skill_level[MEDIC_ABILITY] > 60)
				xpgain = 0;

        		if(xpgain > 0)
        		{
             			ch_printf( ch, "You gain %lld experience from your success!", xpgain);
             			gain_exp(ch, xpgain, MEDIC_ABILITY);
        		}
		return;
                case SUB_TIMER_DO_ABORT:

                        ch->substate = SUB_NONE;
                        act(AT_DGREEN, "$n gets distracted and puts the medicine back.", ch, NULL, NULL, TO_ROOM);
                        act(AT_DGREEN, "You get distracted and put the medicine back.", ch, NULL, NULL, TO_CHAR);

                        return;
        }

}


void do_surgery(CHAR_DATA *ch, char *argument)
{
        char buf[MAX_STRING_LENGTH];
        int time, surgery;
        OBJ_DATA   *medpac;
        CHAR_DATA *victim;
        char arg[MAX_INPUT_LENGTH];

        strcpy( arg, argument );

        switch( ch->substate )
        {
                default:



                        if(arg[0] == '\0')
                        {
                                send_to_char("&RSyntax: surgery (Target)\r\n",ch);
                                return;
                        }
                        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
                        {
                                send_to_char( "They aren't here.\r\n", ch );
                                return;
                        }
                        if(victim == ch)
                        {
                                send_to_char("You can't perform surgery on yourself!\r\n", ch);
                                return;
                        }
			if(ch->adren > 0)
			{
				send_to_char("Your heart is racing to much!\r\n", ch);
				return;
			}
			if(victim->adren > 0)
			{
				send_to_char("Your target has too much adrenaline!\r\n", ch);
				return;
			}
                        if(victim->hit < 150)
                        {
                                send_to_char("They are far too injured to try to use surgery on them!\r\n", ch);
                                return;
                        }
			if(victim->hit > 999)
			{
				send_to_char("They don't need any surgery!\r\n", ch);
				return;
			}
                        if(victim->position == POS_FIGHTING)
                        {
                                send_to_char("They are busy fighting!\r\n", ch);
                                return;
                        }
                        medpac = get_eq_char( ch, WEAR_HOLD );
                        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
                        {
                                send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
                                return;
                        }

			if(medpac->value[0] <= 0)
			{
				send_to_char("Your Medical Kit seems empty.\r\n", ch);
				return;
			}

                        time = 20;

                        if(ch->pcdata->learned[gsn_master_medic])
			{
                                time -= (ch->pcdata->learned[gsn_master_medic]/10);
				learn_from_success( ch, gsn_master_medic);
			}

                        act(AT_DGREEN, "$n begins to try surgery on $N.", ch, NULL, victim, TO_NOTVICT);
                        act(AT_DGREEN, "You begin to try surgery on $N.", ch, NULL, victim, TO_CHAR);
                        act(AT_DGREEN, "$n begins to try surgery on you.", ch, NULL, victim, TO_VICT);
                        add_timer ( ch, TIMER_DO_FUN, time, do_surgery, 1);
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

                        act(AT_DGREEN, "You get distracted and stop trying to perform surgery.", ch, NULL, NULL, TO_CHAR);
                        act(AT_DGREEN, "$n gets distracted and stops trying to perform surgery.", ch, NULL, NULL, TO_ROOM);
                        return;
        }
        ch->substate = SUB_NONE;



        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
                send_to_char( "They aren't here.\n\r", ch );
                return;
        }
        if(victim->hit < 300)
        {
                send_to_char("They are too injured to use surgery on!\r\n", ch);
                return;
        }
        medpac = get_eq_char( ch, WEAR_HOLD );
        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
        {
              send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
              return;
        }

        medpac->value[0]--;
        if((number_range(20,140) - get_curr_int(victim) - (ch->pcdata->learned[gsn_surgery]/2)) > 80)
        {
                act(AT_DGREEN, "You try to surgery $N but make too many mistakes.", ch, NULL, victim, TO_CHAR);
                act(AT_DGREEN, "$n tries to surgery your wounds but makes too many mistakes.", ch, NULL, victim, TO_VICT);
                act(AT_DGREEN, "$n tries to surgery $N but seems to struggle and make mistakes.", ch, NULL, victim, TO_NOTVICT);
		victim->hit -= 50;
		update_pos(victim);
                return;
        }


        act(AT_DGREEN, "Your surgery on $N's wounds was a success!", ch, NULL, victim, TO_CHAR);
        act(AT_DGREEN, "$n's surgery on you was a success!", ch, NULL, victim, TO_VICT);
        act(AT_DGREEN, "$n finishes surgery on $N's wounds.", ch, NULL, victim, TO_NOTVICT);
        learn_from_success(ch, gsn_surgery);
        surgery = number_range(100, 125) + get_curr_int(ch);
	if(ch->pcdata->learned[gsn_field_research])
	{
		surgery += 50;
		learn_from_success( ch, gsn_field_research);
	}
        victim->hit += surgery;
        if(victim->hit > victim->max_hit)
                victim->hit = victim->max_hit;
        long long int xpgain;
        xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
	xpgain += 150;
        xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
	xpgain = xpgain/8;
        if(ch->skill_level[MEDIC_ABILITY] > 130)
                   xpgain = 0;
        if(xpgain > 0)
        {
             ch_printf( ch, "You gain %lld experience from your success!", xpgain);
             gain_exp(ch, xpgain, MEDIC_ABILITY);
        }
        return;
}


void do_treat(CHAR_DATA *ch, char *argument)
{
        char buf[MAX_STRING_LENGTH];
        int time, treat;
        OBJ_DATA   *medpac;
        CHAR_DATA *victim;
        char arg[MAX_INPUT_LENGTH];


        strcpy( arg, argument );

        switch( ch->substate )
        {
                default:

                        if(arg[0] == '\0')
                        {
                                send_to_char("&RSyntax: treat (Target)\r\n",ch);
                                return;
                        }
                        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
                        {
                                send_to_char( "They aren't here.\r\n", ch );
                                return;
                        }
                        if(victim == ch)
                        {
                                send_to_char("You can't perform treat on yourself!\r\n", ch);
                                return;
                        }
                        if(victim->hit < 50)
                        {
                                send_to_char("They are far too injured to be treated!\r\n", ch);
                                return;
                        }
			if(victim->hit > 999)
			{
				send_to_char("They don't need treatment.\r\n", ch);
				return;
			}
                        if(ch->adren > 0)
                        {
                                send_to_char("Your heart is racing to much!\r\n", ch);
                                return;
                        }
                        if(victim->adren > 0)
                        {
                                send_to_char("Your target has too much adrenaline!\r\n", ch);
                                return;
                        }

                        if(victim->position == POS_FIGHTING)
                        {
                                send_to_char("They are busy fighting!\r\n", ch);
                                return;
                        }
                        medpac = get_eq_char( ch, WEAR_HOLD );
                        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
                        {
                                send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
                                return;
                        }
			if(medpac->value[0] <= 0)
			{
				send_to_char("Your Medical Kit seems empty.\r\n", ch);
				return;
			}

                        time = 10;

                        if(ch->pcdata->learned[gsn_master_medic])
			{
                                time -= (ch->pcdata->learned[gsn_master_medic]/20);
				learn_from_success( ch, gsn_master_medic);
			}

                        act(AT_DGREEN, "$n begins to treat $N.", ch, NULL, victim, TO_NOTVICT);
                        act(AT_DGREEN, "You begin to treat $N.", ch, NULL, victim, TO_CHAR);
                        act(AT_DGREEN, "$n begins to treat you.", ch, NULL, victim, TO_VICT);
                        add_timer ( ch, TIMER_DO_FUN, time, do_treat, 1);
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

                        act(AT_DGREEN, "You get distracted and stop treating someone.", ch, NULL, NULL, TO_CHAR);
                        act(AT_DGREEN, "$n gets distracted and stops treating someone.", ch, NULL,NULL, TO_ROOM);
                        return;
        }
        ch->substate = SUB_NONE;



        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
                send_to_char( "They aren't here.\n\r", ch );
                return;
        }
        if(victim->hit < 1)
        {
                send_to_char("They are too injured to use treat on!\r\n", ch);
                return;
        }
        medpac = get_eq_char( ch, WEAR_HOLD );
        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
        {
              send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
              return;
        }

        medpac->value[0]--;
        if((number_range(20,130) - get_curr_int(victim) - (ch->pcdata->learned[gsn_treat]/2)) > 80)
        {
                act(AT_DGREEN, "You try to treat $N but make too many mistakes.", ch, NULL, victim, TO_CHAR);
                act(AT_DGREEN, "$n tries to treat your wounds but makes too many mistakes.", ch, NULL, victim, TO_VICT);
                act(AT_DGREEN, "$n tries to treat $N but seems to struggle and make mistakes.", ch, NULL, victim, TO_NOTVICT);
                return;
        }

        act(AT_DGREEN, "You treat $N's wounds!", ch, NULL, victim, TO_CHAR);
        act(AT_DGREEN, "$n treats you!", ch, NULL, victim, TO_VICT);
        act(AT_DGREEN, "$n finishes treating $N's wounds.", ch, NULL, victim, TO_NOTVICT);
        learn_from_success(ch, gsn_treat);
        treat = 70 + get_curr_int(ch);
        victim->hit += treat;
        if(victim->hit > victim->max_hit)
                victim->hit = victim->max_hit;
        long long int xpgain;
        xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
	xpgain += 200;
        xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
	xpgain = xpgain/8;
        if(ch->skill_level[MEDIC_ABILITY] > 150)
                xpgain = 0;
        if(xpgain > 0)
        {
             ch_printf( ch, "You gain %lld experience from your success!", xpgain);
             gain_exp(ch, xpgain, MEDIC_ABILITY);
        }
        return;
}

void do_resuscitate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA   *medpac;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Resuscitate whom?\n\r", ch );
        return;
    }

    medpac = get_eq_char( ch, WEAR_HOLD );

    if ( !medpac || medpac->item_type != ITEM_MEDPAC )
    {
         send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
         return;
    }

    if(medpac->value[0] <= 0)
    {
         send_to_char("Your Medical Kit seems empty.\r\n", ch);
         return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Resuscitate yourself?\n\r", ch );
        return;
    }
    if(ch->adren > 0)
    {
             send_to_char("Your heart is racing to much!\r\n", ch);
             return;
    }
    if(victim->adren > 0)
    {
          send_to_char("Your target has too much adrenaline!\r\n", ch);
          return;
    }

    if ( victim->position > POS_STUNNED )
    {
        act( AT_PLAIN, "$N doesn't need your help.", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    if ( victim->hit <= -400 )
    {
        act( AT_PLAIN, "$N's condition is beyond your resuscitating ability.", ch,
             NULL, victim, TO_CHAR);
        return;
    }

    ch->alignment = ch->alignment + 20;
    ch->alignment = URANGE( -1000, ch->alignment, 1000 );

    medpac->value[0]--;
    percent = number_percent( ) - (get_curr_lck(ch) - 13);
    WAIT_STATE( ch, skill_table[gsn_resuscitate]->beats );
    if ( !IS_NPC(ch) && percent > ch->pcdata->learned[gsn_resuscitate] )
    {
        send_to_char( "You fail.\n\r", ch );
        learn_from_failure( ch, gsn_resuscitate );
        return;
    }

    ch->alignment = ch->alignment + 20;
    ch->alignment = URANGE( -1000, ch->alignment, 1000 );

    act( AT_SKILL, "You resuscitate $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n resuscitates $N!",  ch, NULL, victim, TO_NOTVICT );
    learn_from_success( ch, gsn_resuscitate );
    if ( victim->hit < 1 )
      victim->hit = 100;

    update_pos( victim );
    act( AT_SKILL, "$n resuscitates you!", ch, NULL, victim, TO_VICT    );
    return;
}


void do_group_heal(CHAR_DATA *ch, char *argument)
{
        char buf[MAX_STRING_LENGTH];
        int time, treat, group;
        OBJ_DATA   *medpac;
        CHAR_DATA *gch;


        switch( ch->substate )
        {
                default:



                        medpac = get_eq_char( ch, WEAR_HOLD );
                        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
                        {
                                send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
                                return;
                        }

                        if(ch->adren > 0)
                        {
                                send_to_char("Your heart is racing to much!\r\n", ch);
                                return;
                        }
                    	group = 0;
                        for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    			{
            			if ( is_same_group( gch, ch ) && gch != ch && gch->adren <= 0 && gch->hit < 999)
            			{
					group++;
				}
			}

			if(group < 1)
			{
				send_to_char("You need to be in a group full of injured people first!\r\n", ch);
				return;
			}
			if(medpac->value[0] < group)
			{
				send_to_char("You Medical Kit doesn't have enough uses for your group!\r\n", ch);
				return;
			}
			time = group * 4;

                        if(ch->pcdata->learned[gsn_master_medic])
			{
                                time -= (ch->pcdata->learned[gsn_master_medic]/10);
				learn_from_success( ch, gsn_master_medic);
			}

                        act(AT_DGREEN, "$n begins to run around healing $s group.", ch, NULL, NULL, TO_ROOM);
                        act(AT_DGREEN, "You begin to run around healing your group", ch, NULL, NULL, TO_CHAR);

                        add_timer ( ch, TIMER_DO_FUN, time, do_group_heal, 1);
                        return;
                case 1:
                        break;
                case SUB_TIMER_DO_ABORT:
                        DISPOSE( ch->dest_buf );
                        ch->substate = SUB_NONE;
                        act(AT_DGREEN, "$n gets distracted and stops healing his group.", ch, NULL, NULL, TO_ROOM);
                        act(AT_DGREEN, "You get distracted and stop healing your group.", ch, NULL, NULL, TO_CHAR);

                        return;
        }
        ch->substate = SUB_NONE;
        medpac = get_eq_char( ch, WEAR_HOLD );
        if ( !medpac || medpac->item_type != ITEM_MEDPAC )
        {
              send_to_char( "You need to be holding a Medical Kit.\n\r",ch );
              return;
        }

        if((number_range(20,120) - get_curr_int(ch) - (ch->pcdata->learned[gsn_group_heal]/2)) > 80)
        {
        	medpac->value[0]--;
                act(AT_DGREEN, "You try to heal your group but are too overwhelmed.", ch, NULL, NULL, TO_CHAR);
		act(AT_DGREEN, "$n tries to heal $s group, but is too overwhelmed.", ch, NULL, NULL, TO_ROOM);
                return;
        }
        for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
        {
            if ( is_same_group( gch, ch ) && gch != ch && gch->hit < 999)
            {
		if(gch->hit < 50)
		{
			act(AT_DGREEN, "$N's wounds are too much to heal.", ch, NULL, gch,TO_CHAR);
			continue;
		}
		if(gch->adren > 0)
		{
			act(AT_DGREEN, "$N has too much adrenaline to heal.", ch, NULL, gch, TO_CHAR);
			continue;
		}
		if(medpac->value[0] <= 0)
		{
			send_to_char("You ran out of Medical Supplies in your Kit!\r\n", ch);
			break;
		}
       		medpac->value[0]--;
		group++;
		act(AT_DGREEN, "$n heals some of your wounds!", ch, NULL, gch, TO_VICT);
		act(AT_DGREEN, "You heal some of $N's wounds!", ch, NULL, gch, TO_CHAR);
		treat = number_range(50, 75);
		if(ch->pcdata->learned[gsn_field_research])
		{
			treat += 50;
			learn_from_success( ch, gsn_field_research);
		}
        	gch->hit += treat;
        	if(gch->hit > gch->max_hit)
                	gch->hit = gch->max_hit;
	    }
	}

        if(group < 2)
	{
		send_to_char("&RYour group is not here!\r\n", ch);
		return;
	}

        act(AT_DGREEN, "You finish healing your group!", ch, NULL, NULL, TO_CHAR);
        act(AT_DGREEN, "$n finishes healing $s group!", ch, NULL, NULL, TO_ROOM);
        learn_from_success(ch, gsn_group_heal);
        long long int xpgain;
        xpgain = UMIN( medpac->cost*200 ,( exp_level(ch->skill_level[MEDIC_ABILITY]+1) - exp_level(ch->skill_level[MEDIC_ABILITY]) ) );
	xpgain += 300;
        xpgain -= (exp_level(ch->skill_level[MEDIC_ABILITY])/100);
	xpgain = xpgain/8;

        if(xpgain > 0)
        {
             ch_printf( ch, "You gain %lld experience from your success!", xpgain);
             gain_exp(ch, xpgain, MEDIC_ABILITY);
        }
        return;
}

void do_bootup ( CHAR_DATA *ch, char *argument )
{

        OBJ_DATA *obj, *obj_next;
        bool found = FALSE;
	int count=0;


	for( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
	  if ( obj->item_type == ITEM_COMPUTER )
	   {
	    count++;

	    if (obj->value[2] == 0 || obj->value[2] > 1)
	    {
	      act( AT_RED, "$n flips the switch on their computer, and the screen lights up.", ch, NULL, NULL, TO_ROOM);
	      act( AT_RED, "You flip the on switch on your computer.", ch, NULL, NULL, TO_CHAR );
	      act( AT_RED, "The bios detects the hardware and the unix environment starts.", ch, NULL, NULL, TO_CHAR);
	      act( AT_RED, "The desktop GUI initializes and loads, the computer is ready.", ch, NULL, NULL, TO_CHAR);
	      obj->value[2] = 1;
	      found = TRUE;
	      break;
	    }
	   }
	}
	  if (found != TRUE)
	   {
	    if (count==0)
	    {
	     act( AT_RED, "You don't have any computers to turn on...", ch, NULL, NULL, TO_CHAR);
	     return;
	    }
	    if (count>0)
	     {
	     act( AT_RED, "Your computer is already on!", ch, NULL, NULL, TO_CHAR);
	     return;
	     }

	}
}

void do_shutoff ( CHAR_DATA *ch, char *argument )
{

        OBJ_DATA *obj, *obj_next;
        bool found = FALSE;
        int count=0;


        for( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
          if ( obj->item_type == ITEM_COMPUTER )
           {
            count++;
            if (obj->value[2] > 0 )
            {
              act( AT_RED, "$n flips the switch on their computer, and the screen slowly fickers off and dies.", ch, NULL, NULL, TO_ROOM);
              act( AT_RED, "You flip the off switch on your computer.", ch, NULL, NULL, TO_CHAR);
              act( AT_RED, "The GUI fades away into a text shutdown script.", ch, NULL, NULL, TO_CHAR);
              act( AT_RED, "The script fades, and the screen flickers off.", ch, NULL, NULL, TO_CHAR);
              obj->value[2] = 0;
              found = TRUE;
              break;
            }
           }
        }
          if (found != TRUE)
           {

            if (count==0)
            {
             act( AT_RED, "You don't have any computers to turn off...", ch, NULL, NULL, TO_CHAR);
             return;
            }
            if (count>0)
             {
             act( AT_RED, "Your computer is already off!", ch, NULL, NULL, TO_CHAR);
             return;
             }
	}
}


void do_deceive(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	// Mobile
	CHAR_DATA *target;
	// Sap to get nailed
	CHAR_DATA *victim;

	if(IS_NPC(ch))
		return;

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if(arg[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: Decieve (Target) (Victim)\r\n", ch);
		return;
	}

    	if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
        	send_to_char( "You can't do that here.\n\r", ch );
        	return;
    	}

	if((target = get_char_room(ch,arg)) == NULL)
	{
		send_to_char("They are not here.\r\n", ch);
		return;
	}

	if((victim = get_char_world(ch, arg2)) == NULL)
	{
		send_to_char("That victim is not on right now.\r\n", ch);
		return;
	}

	if(!IS_AWAKE(target))
	{
		ch_printf(ch, "%s is sleeping!", target->name);
		return;
	}

	if(!IS_NPC(target))
	{
		send_to_char("The Target must be an NPC!\r\n", ch);
		return;
	}

	if(target->frustrated_by && target->frustrated_by == ch)
	{
		do_say(target, "Don't even try me, I am not in the mood.");
		return;
	}

	if(target->frustration > 100)
	{
		send_to_char("They can not get any more frustrated!\r\n", ch);
		return;
	}

	if(IS_NPC(victim))
	{
		send_to_char("You can only use this on players!\r\n", ch);
		return;
	}

	if(target->frustrated_by)
	{
		ch_printf(ch, "%s is already frustrated by someone and needs to calm down first.\r\n", target->name);
		return;
	}

	switch(number_range(0,3))
	{
		case 3:
			act(AT_RED, "You grin evilly as you try to deceive $N", ch, NULL, target, TO_CHAR);
			act(AT_RED, "$n grins evilly as $e tries to deceive $N", ch, NULL, target, TO_ROOM);
		break;
		case 2:
			act(AT_RED, "You calmly lean in to whisper things to deceive $N.", ch, NULL, target, TO_CHAR);
			act(AT_RED, "$n looks calm as $e leans over and whispers something to $N.", ch, NULL, target, TO_ROOM);
		break;
		case 1:
			act(AT_RED, "You smile caringly as you try to deceive $N.", ch, NULL, target, TO_CHAR);
			act(AT_RED, "$n smiles caringly as $e tries to deceive $N.", ch, NULL, target, TO_ROOM);
		break;
		default:
			act(AT_RED, "You try to deceive $N.", ch, NULL, target, TO_CHAR);
			act(AT_RED, "$n try to deceive $N.", ch, NULL, target, TO_ROOM);
		break;
	}

	if((number_range(20, 120) - get_curr_cha(ch)/2 - ch->pcdata->learned[gsn_deceive]) < 1)
	{
		do_say(target, "Yeah right.");
		if(number_range(0,3) == 3)
			target->frustrated_by = ch;
		target->frustration += number_range(1,6);
		return;
	}

	do_say(target, "Ugh, that idiot!");

	target->frustrated_by = victim;
	target->frustration += number_range(0,5);
	learn_from_success(ch, gsn_deceive);
}

void do_calm(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	if(IS_NPC(ch))
		return;

	if(argument[0] == '\0')
	{
		send_to_char("Syntax: Calm (Target)\r\n", ch);
		return;
	}

    	if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
        	send_to_char( "You can't do that here.\n\r", ch );
        	return;
    	}

	if((victim = get_char_room(ch,argument)) == NULL)
	{
		send_to_char("They are not here.\r\n", ch);
		return;
	}

	if(!IS_AWAKE(victim))
	{
		ch_printf(ch, "%s is sleeping!", victim->name);
		return;
	}

	if(!IS_NPC(victim))
	{
		send_to_char("The Target must be an NPC!\r\n", ch);
		return;
	}
/*	-This made calm a sad panda... er... skill.  -Spike
	if(victim->frustrated_by && victim->frustrated_by == ch && number_range(0,3) < 3)
	{
		do_say(victim, "Don't even try me, I am not in the mood.");
		return;
	}
*/
	if(victim->frustration < 3)
	{
		ch_printf(ch, "%s is already calm!\r\n", victim->name);
		return;
	}

	act(AT_PLAIN, "You try to calm down $N.", ch, NULL, victim, TO_CHAR);
	act(AT_PLAIN, "$n tries to calm down $N.", ch, NULL, victim, TO_NOTVICT);
	act(AT_PLAIN, "$n tries to calm down you.", ch, NULL, victim, TO_VICT);

	if((number_range(20, 120) - get_curr_cha(ch)/5 - ch->pcdata->learned[gsn_calm]) > 1)
	{
		act(AT_PLAIN, "You fail at calming $N!", ch, NULL, victim, TO_CHAR);
		act(AT_PLAIN, "$n fails at calming $N!", ch, NULL, victim, TO_NOTVICT);
		act(AT_PLAIN, "$n fails at calming you.", ch, NULL, victim, TO_VICT);

		if(number_range(0,3) == 3)
			victim->frustrated_by = ch;
		victim->frustration += number_range(0,6);
		return;
	}

	do_say(victim, "Thanks, that helped me a little bit.");
	send_to_char("You are successful!\r\n", ch);
	victim->frustration -= number_range(0,5);
	if(is_hating(ch, victim))
	{
		stop_hating(victim);
	}
	if(victim->frustration < 1)
	{
		victim->frustration = 0;
		victim->frustrated_by = NULL;
	}
	learn_from_success(ch, gsn_calm);
}


void do_empathize(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	if(IS_NPC(ch))
		return;

	if(argument[0] == '\0')
	{
		send_to_char("Syntax: Empathize (Target)\r\n", ch);
		return;
	}

    	if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
        	send_to_char( "You can't do that here.\n\r", ch );
        	return;
    	}

	if((victim = get_char_room(ch,argument)) == NULL)
	{
		send_to_char("They are not here.\r\n", ch);
		return;
	}

	if(victim == ch)
	{
		send_to_char("You already know how you're feeling.\r\n", ch);
		return;
	}

	if(!IS_AWAKE(victim))
	{
		ch_printf(ch, "%s is sleeping!", victim->name);
		return;
	}

	act(AT_PLAIN, "You try to empathize with $N.", ch, NULL, victim, TO_CHAR);
	act(AT_PLAIN, "$n tries to empathize with $N.", ch, NULL, victim, TO_NOTVICT);
	act(AT_PLAIN, "$n tries to empathize with you.", ch, NULL, victim, TO_VICT);


	if((number_range(20, 120) - get_curr_cha(ch)/2 - ch->pcdata->learned[gsn_empathize]) > 1)
	{
		act(AT_PLAIN, "You fail at empathizing with $N!", ch, NULL, victim, TO_CHAR);
		act(AT_PLAIN, "$n fails at empathizing with $N!", ch, NULL, victim, TO_NOTVICT);
		act(AT_PLAIN, "$n fails at empathizing with you.", ch, NULL, victim, TO_VICT);

		learn_from_failure(ch, gsn_empathize);
		return;
	}

	if(IS_NPC(victim))
	{

		if (victim->hunting)
		{
			ch_printf(ch, "&R%s wants to find %s!&w\r\n", victim->short_descr, victim->hunting->name);
		}

		if (victim->hating)
		{
			ch_printf(ch, "&R%s wants to kill %s!&w\r\n", victim->short_descr, victim->hating->name);
		}

		if (victim->frustrated_by)
		{
			ch_printf(ch, "&Y%s is really frustrated by %s!&w\r\n", victim->short_descr, victim->frustrated_by->name);
		}

		if (!victim->hunting && !victim->hating && !victim->frustrated_by)
		{
			ch_printf(ch, "&Y%s is doing fine.&w\r\n", victim->short_descr);
		}


		learn_from_success(ch, gsn_empathize);
        ch_printf( ch, "You gain %d experience from your success!", 800);
        gain_exp(ch, 800, DIPLOMACY_ABILITY);
        ch->chatrain++;
        WAIT_STATE( ch, skill_table[gsn_empathize]->beats );
		return;
	}
	else
	{
		if (victim->focus)
		{
			ch_printf(ch, "&R%s is thinking about %s...&w\r\n", victim->name, victim->focus);
		}
		else
		{
			ch_printf(ch, "&Y%s is doing fine.&w\r\n", victim->name);
		}

		learn_from_success(ch, gsn_empathize);
        ch_printf( ch, "You gain %d experience from your success!", 1000);
        gain_exp(ch, 1000, DIPLOMACY_ABILITY);
        ch->chatrain++;
        WAIT_STATE( ch, skill_table[gsn_empathize]->beats );
		return;
	}

}


void do_appeal(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	CHAR_DATA *victim;
	long long int amount;

	if(IS_NPC(ch))
		return;

	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if(arg[0] == '\0')
	{
		send_to_char("Syntax: Appeal (Target) (Clan)\r\n", ch);
		return;
	}

    	if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	{
        	send_to_char( "You can't do that here.\n\r", ch );
        	return;
    	}

	if((victim = get_char_room(ch,arg)) == NULL)
	{
		send_to_char("They are not here.\r\n", ch);
		return;
	}

	if(!IS_AWAKE(victim))
	{
		ch_printf(ch, "%s is sleeping!", victim->name);
		return;
	}

	if(!IS_NPC(victim))
	{
		send_to_char("The Target must be an NPC!\r\n", ch);
		return;
	}

	if(victim->frustrated_by && victim->frustrated_by == ch && number_range(0,3) < 3)
	{
		do_say(victim, "Don't even try me, I am not in the mood.");
		return;
	}

	clan = get_clan(arg2);

	if(!clan)
	{
		send_to_char("That isn't a real clan!\r\n", ch);
		return;
	}

	if(clan->clan_type == 1)
	{
		send_to_char("You can not donate money to autonomous groups!\r\n", ch);
		return;
	}

	if((number_range(20, 120) - get_curr_cha(ch)/5 - ch->pcdata->learned[gsn_deceive]) < 1)
	{
		do_say(victim, "Ugh, I hate it when people ask me to donate money to a clan!");
		if(number_range(0,3) == 3)
			victim->frustrated_by = ch;
		victim->frustration += number_range(0,6);
		return;
	}

	if(victim->gold < 5)
	{
		do_say(victim, "Sorry, I have no wulongs to spare.");
		return;
	}

	amount = number_range(1, victim->gold/2);
	do_say(victim, "There, glad to help!");
	ch_printf(ch, "%s has donated %lld to %s!", victim->name, amount, clan->name);
	clan->funds += amount;
	victim->gold -= amount;
	learn_from_success(ch, gsn_appeal);
	save_clan ( clan );
}


void do_support(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	CHAR_DATA *attacker;


	if(IS_NPC(ch))
		return;

	if(argument[0] == '\0')
	{
		send_to_char("&RSyntax: Support (Target)\r\n",ch);
		return;
	}

	if(ch->position != POS_FIGHTING)
	{
		send_to_char("You need to be fighting first!\r\n", ch);
		return;
	}

	attacker = who_fighting( ch );
	if(!attacker)
	{
		send_to_char("You don't seem to have anyone attacking you!\r\n", ch);
		return;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("That person is not in the room.\r\n", ch);
		return;
	}

	if(!IS_NPC(victim))
	{
		send_to_char("This can only be used on NPCs!\r\n", ch);
		return;
	}

	if(victim->position <= POS_FIGHTING)
	{
		send_to_char("They are in no position to fight!\r\n", ch);
		return;
	}

	if(is_hating(ch, victim) || is_hunting(ch, victim))
	{
		send_to_char("They can't help you!\r\n", ch);
		return;
	}


	act(AT_PLAIN, "You ask $N for help!", ch, NULL, victim, TO_CHAR);
	act(AT_PLAIN, "$n askes you for help!", ch, NULL, victim, TO_VICT);
	act(AT_PLAIN, "$n askes $N for help!", ch, NULL, victim, TO_NOTVICT);
	WAIT_STATE(ch, 10);

	if(victim->frustrated_by == ch)
	{
		send_to_char("They are too frustrated at you to help!\r\n", ch);
		act(AT_PLAIN, "$N ignores $n's asking for help!", ch, NULL, victim, TO_NOTVICT);
		return;
	}

	if(victim->frustration > 10)
	{
		if(number_range(0, victim->frustration/5) > 3)
		{
			do_say(victim, "I won't help you!");
			return;
		}
	}

	if(number_range(0, 140) - get_curr_cha(ch)/3 - ch->pcdata->learned[gsn_support] > 0)
	{
		do_say(victim, "No way, I won't help you fight!");
		return;
	}

	do_say(victim, "Sure, I will help!");
	multi_hit( victim, attacker, TYPE_UNDEFINED );
	start_hating(victim, ch);
	start_hunting(victim, ch);
	learn_from_success(ch, gsn_support);

	return;
}

void do_plead(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	if(ch->position != POS_FIGHTING)
	{
		send_to_char("You need to be fighting first!\r\n", ch);
		return;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't in the room!\r\n", ch);
		return;
	}

	if(!IS_NPC(victim))
	{
		send_to_char("You can only target NPCs!\r\n", ch);
		return;
	}

	send_to_char("You attempt to plead for your life!\r\n", ch);
	do_say(ch, "Please, let me live!");

	if(victim->frustrated_by == ch)
	{
		do_say(victim, "No way! You have pissed me -way- off!");
		victim->frustration += number_range(1,5);
		return;
	}

	if((number_range(0,5) + get_curr_cha(ch)/3 - victim->frustration/2) < 5)
	{
		if(victim->alignment > 300)
			do_say(victim, "I don't think so!");
		else
			do_say(victim, "Heh, I will enjoy watching you die.");
		victim->frustrated_by = ch;
		return;
	}

	if(number_range(0,6) == 6)
        {
                if(victim->alignment > 300)
                        do_say(victim, "I don't think so!");
                else
                        do_say(victim, "Heh, I will enjoy watching you die.");
                victim->frustrated_by = ch;
                return;
        }



	if(victim->alignment > 300)
		do_say(victim, "You weren't worth it anyways.");
	else
		do_say(victim, "Fine, baby. I will leave you alone.");

	learn_from_success(ch, gsn_plead);

	stop_fighting(victim, FALSE);
	stop_hating(victim);
	stop_hunting(victim);

	return;
}

void do_lobby ( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    PLANET_DATA *planet;
    CLAN_DATA   *clan;
    int percent, level;

   if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2);

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char( "Syntax: Lobby (Target) (Clan)\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "This isn't a good place to do that.\n\r", ch );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }

    if ( victim->position == POS_FIGHTING )
    {
        send_to_char( "They're a little busy right now.\n\r" , ch );
        return;
    }


    if ( victim->vip_flags == 0 )
    {
        send_to_char( "Diplomacy would be wasted on them.\n\r" , ch );
        return;
    }

    if( victim->frustration > 0)
    {
	do_say(victim, "I am far too frustrated to talk about politics!");
	return;
    }

    if ( ch->position <= POS_SLEEPING )
    {
        send_to_char( "You might want to wake them first...\n\r" , ch );
        return;
    }

    if ( ( clan = get_clan(arg2) ) == NULL)
    {
	send_to_char("That is not a real clan!\r\n", ch);
	return;
    }

    if(clan->clan_type == 1)
    {
	send_to_char("You can't lobby against that clan!\r\n", ch);
	return;
    }

    planet = ch->in_room->area->planet;

    if(!planet)
    {
        send_to_char("You are not on a planet to do this!\r\n", ch);
        return;
    }

    if(planet->governed_by && planet->governed_by == clan)
    {
	send_to_char("That clan can not be used! They own the planet!\r\n", ch);
	return;
    }

    ch->chatrain++;

    ch_printf(ch, "You lobby about %s.", clan->name);
    act( AT_ACTION, "$n lobbies about an organization.\n\r", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n lobbies $N about an organization.\n\r",  ch, NULL, victim, TO_NOTVICT );

    WAIT_STATE( ch, skill_table[gsn_lobby]->beats );

	percent = number_percent();

    if ( percent - get_curr_cha(ch) + victim->top_level > ch->pcdata->learned[gsn_lobby]
        || number_range(0,10) > 8)
    {

          sprintf( buf, "%s trying to start a riot!" , ch->name);
          do_yell( victim, buf );
          global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	  if(!victim->frustrated_by || victim->frustration < 2)
	  	victim->frustrated_by = ch;
	  victim->frustration += 4;
          return;
    }

    level = IS_NPC(ch) ? ch->top_level
                         : (int) (ch->pcdata->learned[gsn_lobby]);

    if ( planet->frustrated_by == clan )
    {
       ch_printf(ch, "The people of %s grow more frustrated towards %s.\r\n", planet->name, clan->name);
       planet->frust_level += .1 + ch->top_level/50;
    }
    else if(planet->frustrated_by)
    {
        planet->frust_level -= (.01 * level)/2;
	if(planet->frust_level <= 0)
	{
		planet->frust_level = 2;
		planet->frustrated_by = clan;
	}

	ch_printf(ch, "The people of %s grow less frustrated at %s and more at %s.", planet->name, planet->frustrated_by->name, clan->name);
    }
    else
    {
	planet->frustrated_by =  clan;
	planet->frust_level += .1 + ch->top_level/50;
	ch_printf(ch, "The people of %s grow frustrated at %s!", planet->name, clan->name);
    }

    if(planet->frust_level > 100)
	planet->frust_level = 100;

    gain_exp(ch, victim->top_level*100, DIPLOMACY_ABILITY);
    ch_printf( ch , "You gain %d diplomacy experience.\n\r", victim->top_level*100 );
    learn_from_success( ch, gsn_lobby );
}

void do_quickdraw( CHAR_DATA *ch, char *argument ) /* mostly copy/pasted from backstab - Funf */
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

	if (ch->focus == "none" || ch->focus == NULL)
	{
		send_to_char( "You must focus on an opponent before killing.\n\r", ch );
		return;
	}
	else
	{
		argument = ch->focus;
	}

	one_argument( argument, arg );

    	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
    	}

        if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PACIFIST))
	{
		send_to_char("&BYou can't draw on a pacifist, you monster!\n\r" ,ch);
		return;
	}

	if( victim == ch)
	{
		send_to_char("&RHow could you do that?\n\r" ,ch);
		return;
	}

	if( IS_SET( victim->in_room->room_flags, ROOM_PLR_HOME) )
	{
		send_to_char("&RYou can't do that here!\n\r" ,ch);
		return;
	}

    	if ( is_safe( ch, victim ) && victim->adren == 0 )
      		return;

		/* Can dual wield for quickdraw - Funf
   		if ( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
   		{
         	send_to_char( "You can't do that while wielding two weapons.\n\r",ch );
         	return;
  	 	}*/

    	/* Added stabbing weapon. -Narn */
    	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL || ( obj->value[3] != WEAPON_PISTOL ) )
    	{
			send_to_char( "You need to wield a pistol.\n\r", ch );
			return;
    	}

    	if ( ( obj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL && ( obj->value[3] != WEAPON_PISTOL ) )
		{
			send_to_char( "You you're going to dual wield, you must use two pistols.\n\r", ch );
			return;
    	}

    	if ( victim->fighting )
    	{
		send_to_char( "You can't draw on someone who is in combat.\n\r", ch );
		return;
    	}

    	/* Can backstab a char even if it's hurt as long as it's sleeping. -Narn */
    	if ( victim->hit < victim->max_hit && IS_AWAKE( victim ) )
    	{
    		act( AT_PLAIN, "$N is hurt and suspicious ... you can't surprise them.",
	    	ch, NULL, victim, TO_CHAR );
			return;
    	}

    	percent = number_percent( ) - (get_curr_dex(ch) - 14) + (get_curr_dex(victim) - 13);

    WAIT_STATE( ch, skill_table[gsn_quickdraw]->beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   percent < ch->pcdata->learned[gsn_quickdraw] )
    {
	learn_from_success( ch, gsn_quickdraw );
	global_retcode = one_hit( ch, victim, gsn_quickdraw );

    }
    else
    {
	learn_from_failure( ch, gsn_quickdraw );
	global_retcode = damage( ch, victim, 0, gsn_quickdraw );
    }
    return;
}

void do_recover ( CHAR_DATA *ch , char *argument )
{
	send_to_char("Huh?\n\r", ch);
	return;
}

void do_atrophy ( CHAR_DATA *ch, char *argument )
{
	if(IS_NPC(ch))
	{
		send_to_char( "NPCs can't do that!\n\r", ch );
		return;
	}

	if(argument[0] == '\0')
	{
		send_to_char( "Syntax: atrophy <str|wis|int|con|dex|cha|none>.\n\r", ch );
		return;
	}

	if ( !str_prefix( argument , "str" ) )
	{
		ch->pcdata->atrophy = 1;
		send_to_char( "Your strength starts deteriorating...\n\r", ch );
	}
	else if ( !str_prefix( argument , "wis" ) )
	{
		ch->pcdata->atrophy = 2;
		send_to_char( "Your wisdom starts deteriorating...\n\r", ch );
	}
	else if ( !str_prefix( argument , "int" ) )
	{
		ch->pcdata->atrophy = 3;
		send_to_char( "Your intelligence starts deteriorating...\n\r", ch );
	}
	else if ( !str_prefix( argument , "con" ) )
	{
		ch->pcdata->atrophy = 4;
		send_to_char( "Your constitution starts deteriorating...\n\r", ch );
	}
	else if ( !str_prefix( argument , "dex" ) )
	{
		ch->pcdata->atrophy = 5;
		send_to_char( "Your dexterity starts deteriorating...\n\r", ch );
	}
	else if ( !str_prefix( argument , "cha" ) )
	{
		ch->pcdata->atrophy = 6;
		send_to_char( "Your charisma starts deteriorating...\n\r", ch );
	}
	else if ( !str_prefix( argument , "none" ) )
	{
		ch->pcdata->atrophy = 0;
		send_to_char( "You try harder to keep yourself healthy.\n\r", ch );
	}
	else
	{
		send_to_char( "Syntax: atrophy <str|wis|int|con|dex|cha|none>.\n\r", ch );
	}
	return;
}


void do_hackcamera(CHAR_DATA *ch, char *argument )
{
	// look at the room of a player if they left their laptop on
	// look into spaceport security systems
    char arg1[MAX_INPUT_LENGTH], buffer[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj, *comp;
    bool comlink = FALSE, computer = FALSE;
    ROOM_INDEX_DATA *dock;

    argument = one_argument( argument , arg1 );

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
        IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You'll have to do that elsewhere.\n\r", ch );
        return;
    }


    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
       if (obj->pIndexData->item_type == ITEM_COMLINK)
          comlink = TRUE;
       if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0)
       {
          computer = TRUE;
          comp = obj;
       }
    }

    if(!comlink)
    {
        send_to_char("&RYou need a comlink to hack!\n\r", ch);
        return;
    }
    if(!computer)
    {
        send_to_char("&RYou need a working computer to hack!\n\r", ch);
        return;
    }

    if (comp->value[2] == 0 )
    {
        act( AT_RED, "Maybe you should 'bootup' your computer first?", ch, NULL, NULL, TO_CHAR);
        return;
    }

    if( arg1[0] == '\0')
    {
        send_to_char("&Chackcamera: [Person or Spaceport Name]\n\r", ch);
        return;
    }

    if( (victim = get_char_world(ch, arg1)) == NULL || IS_NPC(victim) )
    {
		if( (dock = find_dock(arg1)) == NULL )
		{
			send_to_char("&RNo such player or space dock!&W\n\r", ch);
			return;
		}
		victim = NULL;
    }
    if(victim && IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
        send_to_char("&RThey probably wouldn't like that.\r\n", ch);
        return;
    }

//    if( get_age(victim) < 6)
//    {
//        send_to_char("&RThe victim must have played atleast 6 hours!\r\n", ch);
//        return;
//    }

    WAIT_STATE( ch, 25 );

    act( AT_ACTION, "$n begins to tamper with a comlink and computer.\n\r",  ch, NULL, NULL, TO_ROOM );

    if ( number_percent() - get_curr_int(ch) > ch->pcdata->learned[gsn_hackcamera]
		|| number_range(0,5) > 4)
    {
         send_to_char( "Your attempt to hack is discovered!\n\r", ch );
         place_bounty( ch, ch, 30000, 0);

         for( obj = ch->last_carrying; obj; obj = obj->prev_content)
         {
               if(obj->item_type == ITEM_COMPUTER)
               {
				/*
						   separate_obj( obj );
						   obj_from_char( obj );
						   extract_obj( obj );
				*/
				   if(obj->value[0] - 1 <= 0)
				   {
					obj->value[0]--;
					send_to_char( "Your computer was destroyed!\n\r", ch);
				   }
				   else
				   {
					obj->value[0]--;
					send_to_char("Your computer was damaged!\n\r", ch);
				   }

                   break;
               }
         }
		if(ch->perm_int < 15 && number_range(0,4) >= 3)
			ch->inttrain++;
		return;
    }

	if (victim)
	{
		comlink = FALSE;
		computer = FALSE;
		for ( obj = victim->last_carrying; obj; obj = obj->prev_content )
		{
		   if (obj->pIndexData->item_type == ITEM_COMLINK)
			  comlink = TRUE;
		   if (obj->pIndexData->item_type == ITEM_COMPUTER && obj->value[0] > 0 && obj->value[2] > 0)
		   {
			  computer = TRUE;
			  comp = obj;
		   }
		}

		if(!comlink || !computer)
		{
			ch_printf( ch, "You weren't able to establish a connection to %s's computer!\n\r", victim->name);
			return;
		}

		ch_printf( victim, "&GA small LED briefly flashes on your computer.\n\r");
		sprintf(buffer,"%s look", victim->name);
		ch_printf( ch, "You were able to pick up a live feed from %s's computer camera!\n\r", victim->name);
	}
	else if (dock)
	{
		sprintf(buffer,"%d look", dock->vnum);
		ch_printf( ch, "You were able to pick up a live feed from the %s security camera!\n\r", dock->name);
	}

	do_at( ch, buffer );

	gain_exp(ch, (ch->top_level/2+30), FORCE_ABILITY);
	ch_printf( ch , "You gain %d hacking experience.\n\r", (ch->top_level/2+30) );
	learn_from_success( ch, gsn_hackcamera);
	return;
}


