/***************************************************************************
*                    Cowboy Bebop: Space Cowboy 1.5                        *
*--------------------------------------------------------------------------*
* Bebop Code changes (c) 2003-2008 - Spike/Teiwaz/Kristen/Gatz/Funf        *
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
*			    Battle & death module			   *
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#if defined(__CYGWIN__)
#include <io.h>
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include "mud.h"

extern char		lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *	gch_prev;

/* From Skills.c */
int ris_save( CHAR_DATA *ch, int chance, int ris );

/*
 * Local functions.
 */
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
int	align_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
ch_ret	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	obj_hitroll	args( ( OBJ_DATA *obj ) );
bool    get_cover( CHAR_DATA *ch );
bool	dual_flip = FALSE;
bool check_anti_tanking( CHAR_DATA *ch, CHAR_DATA *victim );
void    ricochet	args( ( CHAR_DATA *ch) );

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
         OBJ_DATA *obj;

         if ( ( obj = get_eq_char( ch, WEAR_WIELD ) 	)
         &&   (IS_SET( obj->extra_flags, ITEM_POISONED) )	)
                  return TRUE;

         return FALSE;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;

    return TRUE;
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;

    return TRUE;
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;

    return TRUE;
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
	STRFREE( ch->hunting->name );
	DISPOSE( ch->hunting );
	ch->hunting = NULL;
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
	STRFREE( ch->hating->name );
	DISPOSE( ch->hating );
	ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
	STRFREE( ch->fearing->name );
	DISPOSE( ch->fearing );
	ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hating )
      stop_hating( ch );

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
      stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    return;
}


int max_fight( CHAR_DATA *ch )
{
    return 8;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch, *rch_next;
    AFFECT_DATA *paf, *paf_next;
    TIMER	*timer, *timer_next;
    ch_ret     retcode;
    SKILLTYPE	*skill;

    lst_ch = NULL;
    for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
    {
	set_cur_char( ch );

	if ( ch == first_char && ch->prev )
	{
	   bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
	   ch->prev = NULL;
	}

	gch_prev	= ch->prev;

	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );
	    bug( "Short-cutting here", 0 );
	    ch->prev = NULL;
	    gch_prev = NULL;
	    do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
	}

	/*
	 * See if we got a pointer to someone who recently died...
	 * if so, either the pointer is bad... or it's a player who
	 * "died", and is back at the healer...
	 * Since he/she's in the char_list, it's likely to be the later...
	 * and should not already be in another fight already
	 */
	if ( char_died(ch) )
	    continue;

	/*
	 * See if we got a pointer to some bad looking data...
	 */
	if ( !ch->in_room || !ch->name )
	{
	    log_string( "violence_update: bad ch record!  (Shortcutting.)" );
	    sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
	    	(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
	    log_string( buf );
	    log_string( lastplayercmd );
	    if ( lst_ch )
	      sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
	      		(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
	    else
	      strcpy( buf, "lst_ch: NULL" );
	    log_string( buf );
	    gch_prev = NULL;
	    continue;
	}


	for ( timer = ch->first_timer; timer; timer = timer_next )
	{
	    timer_next = timer->next;
	    if ( --timer->count <= 0 )
	    {
		// Pretty big timer edit - Gatz
		if ( timer->type == TIMER_DO_FUN )
		{
		    int tempsub;
		    DO_FUN *fun;

		    fun = timer->do_fun;
		    tempsub = ch->substate;
		    ch->substate = timer->value;
		    extract_timer( ch, timer );
		    timer = NULL;
		    //(timer->do_fun)( ch, "" );
		    (fun)(ch, "");
		    if ( char_died(ch) )
		      break;
		    ch->substate = tempsub;
		}
		if(timer)
			extract_timer( ch, timer );
	    }
	}

	if ( char_died(ch) )
	  continue;

	/*
	 * We need spells that have shorter durations than an hour.
	 * So a melee round sounds good to me... -Thoric
	 */
	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
	      paf_next	= paf->next;
	      if ( paf->duration > 0 )
		paf->duration--;
	      else
	      if ( paf->duration < 0 )
		;
	      else
	      {
		  if ( !paf_next
		  ||    paf_next->type != paf->type
		  ||    paf_next->duration > 0 )
		  {
		      skill = get_skilltype(paf->type);
		      if ( paf->type > 0 && skill && skill->msg_off )
		      {
                          set_char_color( AT_WEAROFF, ch );
			  send_to_char( skill->msg_off, ch );
			  send_to_char( "\n\r", ch );
		      }
		  }
		  if (paf->type == gsn_possess)
	          {
	            ch->desc->character       = ch->desc->original;
    	            ch->desc->original        = NULL;
    		    ch->desc->character->desc = ch->desc;
   	            ch->desc->character->switched = NULL;
    		    ch->desc                  = NULL;
		  }
		  affect_remove( ch, paf );
	      }
	}

	if ( ( victim = who_fighting( ch ) ) == NULL
	||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
	    continue;
	// Anti-tank code! - Gatz. Code was stupid - Funf
	/* if( IS_NPC(ch) && !IS_NPC(victim)
	&& check_anti_tanking(ch, victim) )
		victim = who_fighting(ch); */

        retcode = rNONE;

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE ) && victim->adren == 0
		&& ch->adren == 0 )
	{
	   sprintf( buf, "violence_update: %s fighting %s in a SAFE room.",
	   	ch->name, victim->name );
	   log_string( buf );
	   stop_fighting( ch, TRUE );
	}
	else
	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	 /*   retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
	else*/
	{
            stop_fighting( ch, FALSE );
	// Don't want that! - Gatz
       // retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		/*Random VERY minor bleeding during a fight*/
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_BLEEDING] > 0 && number_range(1,4) == 1)
		{
			damage(ch, ch, ch->pcdata->condition[COND_BLEEDING], TYPE_UNDEFINED);
			act( AT_BLOOD, "You're losing blood...", ch, NULL, NULL, TO_CHAR);
			act( AT_BLOOD, "$n is losing blood...", ch, NULL, NULL, TO_ROOM);
		}
	}


	if ( char_died(ch) )
	    continue;

	if ( retcode == rCHAR_DIED
	|| ( victim = who_fighting( ch ) ) == NULL )
	    continue;

	/*
	 *  Mob triggers
	 */
	rprog_rfight_trigger( ch );
	if ( char_died(ch) )
	    continue;
	mprog_hitprcnt_trigger( ch, victim );
	if ( char_died(ch) )
	    continue;
	mprog_fight_trigger( ch, victim );
	if ( char_died(ch) )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	for ( rch = ch->in_room->first_person; rch; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

	    if ( IS_AWAKE(rch) && !rch->fighting )
	    {
		/*
		 * PC's auto-assist others in their group.
		 */
		if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
		{
		    if ( ( !IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM) )
		    &&   is_same_group(ch, rch) )
			multi_hit( rch, victim, TYPE_UNDEFINED );
		    continue;
		}

		/*
		 * NPC's assist NPC's of same type or 12.5% chance regardless.
		 */
		if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM)
		&&  !IS_SET(rch->act, ACT_NOASSIST) )
		{
		    if ( char_died(ch) )
			break;
		    if ( rch->pIndexData == ch->pIndexData
		    ||   number_bits( 3 ) == 0 )
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			target = NULL;
			number = 0;			for ( vch = ch->in_room->first_person; vch; vch = vch->next )
			{
			    if ( can_see( rch, vch )
			    &&   is_same_group( vch, victim )
			    &&   number_range( 0, number ) == 0 )
			    {
				target = vch;
				number++;
			    }
			}

			if ( target )
			    multi_hit( rch, target, TYPE_UNDEFINED );
		    }
		}
	    }
	}
    }

    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int     chance;
    int	    dual_bonus;
    ch_ret  retcode;


    /* add timer if player is attacking another player */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
      add_timer( ch, TIMER_RECENTFIGHT, 20, NULL, 0 );

    if ( !IS_NPC(ch) && IS_SET( ch->act, PLR_NICE ) && !IS_NPC( victim ) )
      return rNONE;

    if ( (retcode = one_hit( ch, victim, dt )) != rNONE )
      return retcode;

    if ( who_fighting( ch ) != victim || dt == gsn_backstab || dt == gsn_circle)
	return rNONE;

    /* Very high chance of hitting compared to chance of going berserk */
    /* 40% or higher is always hit.. don't learn anything here though. */
    /* -- Altrag */
    chance = IS_NPC(ch) ? 100 : (ch->pcdata->learned[gsn_berserk]*5/2);
    if ( IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < chance )
      if ( (retcode = one_hit( ch, victim, dt )) != rNONE ||
            who_fighting( ch ) != victim )
        return retcode;
	// Fighting type progs moved here - Gatz
        rprog_rfight_trigger( ch );
       // if ( char_died(ch) )
         //   continue;
        mprog_hitprcnt_trigger( ch, victim );
       // if ( char_died(ch) )
         //   continue;
        mprog_fight_trigger( ch, victim );
      //  if ( char_died(ch) )
        //    continue;


    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
      dual_bonus = IS_NPC(ch) ? (ch->skill_level[COMBAT_ABILITY] / 10) : (ch->pcdata->learned[gsn_dual_wield] / 10);
      chance = IS_NPC(ch) ? ch->top_level : ch->pcdata->learned[gsn_dual_wield];
      if ( number_percent( ) < chance )
      {
	learn_from_success( ch, gsn_dual_wield );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
      }
      else
	learn_from_failure( ch, gsn_dual_wield );
    }
    else
      dual_bonus = 0;

    if ( ch->move < 10 )
      dual_bonus = -20;

    /*
     * NPC predetermined number of attacks			-Thoric
     */
    if ( IS_NPC(ch) && ch->numattacks > 0 )
    {
	for ( chance = 0; chance <= ch->numattacks; chance++ )
	{
	   retcode = one_hit( ch, victim, dt );
	   if ( retcode != rNONE || who_fighting( ch ) != victim )
	     return retcode;
	}
	return retcode;
    }

    chance = IS_NPC(ch) ? ch->top_level
	   : (int) ((ch->pcdata->learned[gsn_second_attack]+dual_bonus)/1.5);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_second_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_second_attack );

    chance = IS_NPC(ch) ? ch->top_level
	   : (int) ((ch->pcdata->learned[gsn_third_attack]+(dual_bonus*1.5))/2);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_third_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_third_attack );

    retcode = rNONE;

    chance = IS_NPC(ch) ? (int) (ch->top_level / 4) : 0;
    if ( number_percent( ) < chance )
	retcode = one_hit( ch, victim, dt );

    if ( retcode == rNONE )
    {
	int move;

	if ( !IS_AFFECTED(ch, AFF_FLYING)
	&&   !IS_AFFECTED(ch, AFF_FLOATING) )
	  move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, ch->in_room->sector_type)] );
	else
	  move = encumbrance( ch, 1 );
	if ( ch->move )
	  ch->move = UMAX( 0, ch->move - move );
    }


    return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
    int bonus;

    bonus = 0;	*gsn_ptr = -1;
    if ( !IS_NPC(ch) && wield )
    {
		switch(wield->value[3])
		{
		   default:	*gsn_ptr = -1;			break;
    	       case 3:      *gsn_ptr = gsn_swords;     	break;
    	       case 2:	*gsn_ptr = gsn_knives;	        break;
    	       case 4:	*gsn_ptr = gsn_flexible_arms;	break;
    	       case 5:	*gsn_ptr = gsn_talonous_arms;	break;
    	       case 6:	*gsn_ptr = gsn_rifles; bonus = get_curr_wis(ch) > 15 ? 1 : 0; 	break; /* High WIS helps ranged accuracy - Funf */
    	       case 8:	*gsn_ptr = gsn_bludgeons;	break;
    	       case 9:	*gsn_ptr = gsn_pistols;	bonus = get_curr_wis(ch) > 15 ? 1 : 0; 	break;
    	       case 11:	*gsn_ptr = gsn_heavy_weapons;	break;

		}
		if ( *gsn_ptr != -1 )
			bonus = (int) ( ( ch->pcdata->learned[*gsn_ptr] - 50 ) / 10 );

	}
    return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll( OBJ_DATA *obj )
{
	int tohit = 0;
	AFFECT_DATA *paf;

	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	for ( paf = obj->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
    sh_int lvl;

    if ( !IS_NPC(ch) )		/* players get much less effect */
    {
	lvl = UMAX( 1, (ch->skill_level[FORCE_ABILITY])  );
	if ( number_percent() + (victim->skill_level[COMBAT_ABILITY] - lvl) < 35 )
	  return lvl;
	else
	  return 0;
    }
    else
    {
	lvl = ch->top_level;
	if ( number_percent() + (victim->skill_level[COMBAT_ABILITY] - lvl) < 70 )
	  return lvl;
	else
	  return 0;
    }
}

/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int dam, x;
    int diceroll;
    int attacktype, cnt;
    int	prof_bonus;
    int	prof_gsn;
    ch_ret retcode;
    int chance;
    bool fail;
    AFFECT_DATA af;


    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     * Changed to Create No Deaths. Stunning Only. Jet<RGC>8-6-3
     */
    if ( victim->position == POS_INCAP || ch->in_room != victim->in_room )
	return rVICT_DIED;


    /*
     * Figure out the weapon doing the damage			-Thoric
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
       if ( dual_flip == FALSE )
       {
	 dual_flip = TRUE;
	 wield = get_eq_char( ch, WEAR_WIELD );
       }
       else
	 dual_flip = FALSE;
    }
    else
      wield = get_eq_char( ch, WEAR_WIELD );

    prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );

    if ( ch->fighting		/* make sure fight is already started */
    &&   dt == TYPE_UNDEFINED
    &&   IS_NPC(ch)
    &&   ch->attacks != 0 )
    {
	  cnt = 0;
	  for ( ;; )
	  {
	     x = number_range( 0, 6 );
	     attacktype = 1 << x;
	     if ( IS_SET( ch->attacks, attacktype ) )
	       break;
	     if ( cnt++ > 16 )
	     {
	       attacktype = 0;
	       break;
	     }
	  }
	  if ( attacktype == ATCK_BACKSTAB )
	    attacktype = 0;
	  if ( wield && number_percent( ) > 25 )
	    attacktype = 0;
	  switch ( attacktype )
	  {
	    default:
	      break;
	    case ATCK_PUNCH:
	      ch->focus = victim->name;
	      WAIT_STATE(ch , 3);
	      do_punch( ch, ch->focus );
	      WAIT_STATE(ch, 2);
	      retcode = global_retcode;
	      break;
	    case ATCK_KICK:
	      ch->focus = victim->name;
	      WAIT_STATE(ch, 2);
	    do_kick( ch, ch->focus );
	      WAIT_STATE(ch, 3);
	      retcode = global_retcode;
	      break;
	    case ATCK_TRIP:
	      attacktype = 0;
	      break;
	  }
	  if ( attacktype )
	    return retcode;
    }

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    thac0_00 = 18;
    thac0_32 = 8;
    thac0     = interpolate( (int) ( ch->skill_level[COMBAT_ABILITY] / 4 ), thac0_00, thac0_32 ) - GET_HITROLL(ch);
    victim_ac = URANGE( -50, (int) (GET_AC(victim) / 10), 50 );

    /* if you can't see what's coming... */
    if ( wield && !can_see_obj( victim, wield) )
	victim_ac += 1;
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

	/* Lucky bastards... - Funf */
	victim_ac += (get_curr_lck(ch) - 10) / 3;

    /*
     * The moment of excitement!
     */
    diceroll = number_range( 1, 40);

    if ( diceroll == 1 || ( diceroll != 40 && diceroll < thac0 - victim_ac ) )
   {
	   // Dex training
	if( !IS_NPC(victim) && number_range(0,5) == 1)
	   ;//victim->passive_dextrain += 2;
	           // Shell stuff - Gatz
            make_shell( ch );
            wield = get_eq_char( ch, WEAR_WIELD );
            if( wield != NULL)
            {
                if(wield->value[3] == WEAPON_RIFLE || wield->value[3] == WEAPON_PISTOL)
				{
                	wield->value[4] -= 1;
					if( wield->value[4] < 0)
					wield->value[4] = 0;
				}
            }

		    /* Physical hit with no damage (hit natural AC) -Shade */
	    if ( diceroll < thac0 - victim_ac
		&& diceroll >= thac0 - (10 + (dex_app[get_curr_dex(ch)].defensive / 10))
		&& diceroll != 19
		&& diceroll != 0 )
		{
	    if ( prof_gsn != -1 )
	    learn_from_failure( ch, prof_gsn );
	    act( AT_PLAIN, "Your attack is deflected by $N's armor.", ch, NULL, victim, TO_CHAR );
	    act( AT_PLAIN, "Your armor deflects $n's attack.", ch, NULL, victim, TO_VICT );
		/*
	    if (!victim->fighting && victim->in_room == ch->in_room && IS_NPC(victim))
    	    {
		    set_fighting( victim, ch );
    		    start_hating( victim, ch );
    		    start_hunting( victim, ch );
	    } */
		// Slight mod to the original snippet
	    damage( ch, victim, 0, dt );
	    ricochet( ch );
	    tail_chain( );
 	    return rNONE;
		}
	    else
		{

	/* Miss. */
	if ( prof_gsn != -1 )
		learn_from_failure( ch, prof_gsn );
	damage( ch, victim, 0, dt );
	ricochet( ch );
	tail_chain( );
	return rNONE;
		}
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* Fixed again by Kristen */
	dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie )+ ch->damplus;
    else
	dam = number_range( wield->value[1], wield->value[2] );
    // Con Training - Gatz
    if(!IS_NPC(victim))
	  ;//victim->passive_contrain += 2;

    /*
     * Bonuses.
     */

    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
      dam *= (int) ( 1 + ( prof_bonus / 100. ) );


    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
	dam += (int) (dam * ch->pcdata->learned[gsn_enhanced_damage] / 120.);
	learn_from_success( ch, gsn_enhanced_damage );
    }

    /* Damage is too low!! -Kristen */
    dam *= 4;

    /* Help newbs out!! --Funf 2008-02-22 */
    if(!IS_NPC(ch) && IS_NPC(victim))
    	dam += UMAX((75 - (ch->skill_level[COMBAT_ABILITY] / 2) - victim->skill_level[COMBAT_ABILITY]), 0);

    // Rocket Launchers and LAW does boosted damage! - Gatz
    if(wield != NULL)
    	if(wield != NULL && wield->value[3] == WEAPON_HEAVY_WEAPON)
    	{
		dam *= 3;
    	}

    // Too high! - Gatz
    if ( !IS_AWAKE(victim) )
	dam *= 1.5;
    if ( dt == gsn_backstab )
	dam *= 2;
	if ( dt == gsn_quickdraw )
	dam *= 1.8;

    if ( dt == gsn_circle )
 	dam *= 1.25;

    plusris = 0;

    if ( wield )
    {
      if ( IS_SET( wield->extra_flags, ITEM_MAGIC ) )
        dam = ris_damage( victim, dam, RIS_MAGIC );
      else
        dam = ris_damage( victim, dam, RIS_NONMAGIC );

      /*
       * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
       */
      plusris = obj_hitroll( wield );
    }
    else
      dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
	int x, res, imm, sus, mod;

	if ( plusris )
	   plusris = RIS_PLUS1 << UMIN(plusris, 7);

	/* initialize values to handle a zero plusris */
	imm = res = -1;  sus = 1;

	/* find high ris */
	for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
	{
	   if ( IS_SET( victim->immune, x ) )
		imm = x;
	   if ( IS_SET( victim->resistant, x ) )
		res = x;
	   if ( IS_SET( victim->susceptible, x ) )
		sus = x;
	}
	mod = 10;
	if ( imm >= plusris )
	  mod -= 10;
	if ( res >= plusris )
	  mod -= 2;
	if ( sus <= plusris )
	  mod += 2;

	/* check if immune */
	if ( mod <= 0 )
	  dam = -1;
	if ( mod != 10 )
	  dam = (dam * mod) / 10;
    }

    /* race modifier */

    if ( victim->race == RACE_DUINUOGWUIN )
       dam /= 5;

    /*
      * check to see if weapon has AMMO -Kristen-
      */

     if ( dt == (TYPE_HIT + WEAPON_RIFLE ) && wield && wield->item_type == ITEM_WEAPON )
     {
     	if ( wield->value[4] < 1  )
     	{
            act( AT_YELLOW, "$n points their rifle at you, but nothing happens.",  ch, NULL, victim, TO_VICT    );
            act( AT_YELLOW, "*CLICK* ... Your rifle needs ammunition!", ch, NULL, victim, TO_CHAR    );
            if ( IS_NPC(ch) )
     	    {
     	        do_remove( ch, wield->name );
     	    }
            return rNONE;
     	}

        else
     	    wield->value[4]--;
     }

     else if ( dt == (TYPE_HIT + WEAPON_HEAVY_WEAPON ) && wield && wield->item_type == ITEM_WEAPON )
     {
     	if ( wield->value[4] < 1  )
     	{
	    	act( AT_YELLOW, "$n points their heavy weapon at you, but nothing happens.",  ch, NULL, victim, TO_VICT    );
            act( AT_YELLOW, "Your heavy weapon needs reloading ...", ch, NULL, victim, TO_CHAR    );
            if ( IS_NPC(ch) )
     	    {
     	        do_remove( ch, wield->name );
     	    }
            return rNONE;
     	}
     	else
     	    wield->value[4]--;
     }

     else if ( ( dt == (TYPE_HIT + WEAPON_PISTOL ) || dt == gsn_quickdraw ) && wield && wield->item_type == ITEM_WEAPON )
     {
     	if ( wield->value[4] < 1  )
     	{
            act( AT_YELLOW, "$n points their pistol at you, but nothing happens.",  ch, NULL, victim, TO_VICT    );
            act( AT_YELLOW, "*CLICK* ... Your pistol needs reloading!", ch, NULL, victim, TO_CHAR    );
            if ( IS_NPC(ch) )
     	    {
     	        do_remove( ch, wield->name );
     	    }
     	    else
     	    {
				do_ammo( ch, "" ); /* Making pistols more uber - Funf */
			}
            return rNONE;
     	}
     	else
          wield->value[4]--;
     }


    dam -= get_curr_con(victim) - 10; /* CON give damage resistance - Funf */


    if ( dam <= 0 )
	dam = 1;

    if ( prof_gsn != -1 )
    {
      if ( dam > 0 )
        learn_from_success( ch, prof_gsn );
      else
        learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];
	    bool found = FALSE;

	    if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    {
		act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    {
		act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		found = TRUE;
	    }
	    if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    {
		act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		found = TRUE;
	    }
	    if ( found )
	      return rNONE;
	}
	dam = 0;
    }
    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
      return retcode;
    if ( char_died(ch) )
      return rCHAR_DIED;
    if ( char_died(victim) )
      return rVICT_DIED;

    retcode = rNONE;
    if ( dam == 0 )
      return retcode;

/* weapon spells	-Thoric */
    if ( wield
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	AFFECT_DATA *aff;

	for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
	for ( aff = wield->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
    }



     /*  folks with rifles move and snipe instead of getting eaten up in one spot -Kristen- */
     if ( IS_NPC(victim) )
     {
         OBJ_DATA *wield;

         wield = get_eq_char( victim, WEAR_WIELD );
         if ( wield != NULL && wield->value[3] == WEAPON_RIFLE && get_cover( victim ) == TRUE )
         {
         	start_hating( victim, ch );
	     	start_hunting( victim, ch );
         }
     }

    tail_chain( );
    return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage( CHAR_DATA *ch, sh_int dam, int ris )
{
   sh_int modifier;

   modifier = 10;
   if ( IS_SET(ch->immune, ris ) )
     modifier -= 10;
   if ( IS_SET(ch->resistant, ris ) )
     modifier -= 2;
   if ( IS_SET(ch->susceptible, ris ) )
     modifier += 2;
   if ( modifier <= 0 )
     return -1;
   if ( modifier == 10 )
     return dam;
   return (dam * modifier) / 10;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf1[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int dameq;
    bool npcvict;
    bool loot;
    int  xp_gain;
    OBJ_DATA *damobj;
    ch_ret retcode;
    sh_int dampmod;

    long long int init_gold, new_gold, gold_diff;

    retcode = rNONE;

    if ( !ch )
    {
	bug( "Damage: null ch!", 0 );
	return rERROR;
    }
    if ( !victim )
    {
	bug( "Damage: null victim!", 0 );
	return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
	return rVICT_DIED;

    npcvict = IS_NPC(victim);

    /*
     * Check damage types for RIS				-Thoric
     */
    if ( dam && dt != TYPE_UNDEFINED )
    {
	if ( IS_FIRE(dt) )
	  dam = ris_damage(victim, dam, RIS_FIRE);
	else
	if ( IS_COLD(dt) )
	  dam = ris_damage(victim, dam, RIS_COLD);
	else
	if ( IS_ACID(dt) )
	  dam = ris_damage(victim, dam, RIS_ACID);
	else
	if ( IS_ELECTRICITY(dt) )
	  dam = ris_damage(victim, dam, RIS_ELECTRICITY);
	else
	if ( IS_ENERGY(dt) )
	  dam = ris_damage(victim, dam, RIS_ENERGY);
	else
	if ( IS_DRAIN(dt) )
	  dam = ris_damage(victim, dam, RIS_DRAIN);
	else
	if ( dt == gsn_poison || IS_POISON(dt) )
	  dam = ris_damage(victim, dam, RIS_POISON);
	else
	if ( dt == (TYPE_HIT + 7) || dt == (TYPE_HIT + 8) )
	  dam = ris_damage(victim, dam, RIS_BLUNT);
	else
	if ( dt == (TYPE_HIT + 2) || dt == (TYPE_HIT + 11)
	||   dt == (TYPE_HIT + 10) )
	  dam = ris_damage(victim, dam, RIS_PIERCE);
	else
	if ( dt == (TYPE_HIT + 1) || dt == (TYPE_HIT + 3)
	||   dt == (TYPE_HIT + 4) || dt == (TYPE_HIT + 5) )
	  dam = ris_damage(victim, dam, RIS_SLASH);

	if ( dam == -1 )
	{
	    if ( dt >= 0 && dt < top_sn )
	    {
		bool found = FALSE;
		SKILLTYPE *skill = skill_table[dt];

		if ( skill->imm_char && skill->imm_char[0] != '\0' )
		{
		   act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->imm_room && skill->imm_room[0] != '\0' )
		{
		   act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )
		   return rNONE;
	    }
	    dam = 0;
	}
    }

    if ( dam && npcvict && ch != victim )
    {
	if ( !IS_SET( victim->act, ACT_SENTINEL ) )
 	{
	   if ( victim->hunting )
	   {
	     if ( victim->hunting->who != ch )
	     {
		STRFREE( victim->hunting->name );
		victim->hunting->name = QUICKLINK( ch->name );
		victim->hunting->who  = ch;
	     }
           }
	   else
	     start_hunting( victim, ch );
	}

      if ( victim->hating )
      {
	if ( victim->hating->who != ch )
	{
	   STRFREE( victim->hating->name );
	   victim->hating->name = QUICKLINK( ch->name );
	   victim->hating->who  = ch;
	}
      }
      else
	start_hating( victim, ch );
    }

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) && victim->adren == 0 )
	    return rNONE;


	if ( victim->position > POS_STUNNED )
	{
	    if ( !victim->fighting )
		set_fighting( victim, ch );
	    if ( victim->fighting )
		victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( !ch->fighting )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   npcvict
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		retcode = multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return retcode;
	    }
	}


	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );


	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) && ch->race != RACE_DEFEL )
	{
	    affect_strip( ch, gsn_invis );
	    affect_strip( ch, gsn_mass_invis );
	    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
	}

	/* Take away Hide */
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	     REMOVE_BIT(ch->affected_by, AFF_HIDE);
	/*
	 * Damage modifiers.
	 */
	if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	    dam /= 2;

	if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	    dam -= (int) (dam / 4);

	if ( dam < 0 )
	    dam = 0;

	/*
	 * Check for disarm, trip, parry, and dodge.
	 */
	if ( dt >= TYPE_HIT )
	{
	    if ( IS_NPC(ch)
	    &&   IS_SET( ch->attacks, DFND_DISARM )
	    &&   number_percent( ) < ch->skill_level[COMBAT_ABILITY] / 2 )
		disarm( ch, victim );

	    if ( IS_NPC(ch)
	    &&   IS_SET( ch->attacks, ATCK_TRIP )
	    &&   number_percent( ) < ch->skill_level[COMBAT_ABILITY] )
		trip( ch, victim );

	    if ( check_parry( ch, victim ) )
		return rNONE;
	    if ( check_dodge( ch, victim ) )
		return rNONE;
	}



    /*
     * Check control panel settings and modify damage
     */
    if ( IS_NPC(ch) )
    {
	if ( npcvict )
	  dampmod = sysdata.dam_mob_vs_mob;
	else
	  dampmod = sysdata.dam_mob_vs_plr;
    }
    else
    {
	if ( npcvict )
	  dampmod = sysdata.dam_plr_vs_mob;
	else
	  dampmod = sysdata.dam_plr_vs_plr;
    }
    if ( dampmod > 0 )
      dam = ( dam * dampmod ) / 100;

	dam_message( ch, victim, dam, dt );
    }


    /*
     * Code to handle equipment getting damaged, and also support  -Thoric
     * bonuses/penalties for having or not having equipment where hit
     * Bleeding support added -Seven
     */
    if (dam > 10 && dt != TYPE_UNDEFINED)
    {
	/* get a random body eq part */
	dameq  = number_range(WEAR_LIGHT, WEAR_EYES);
	damobj = get_eq_char(victim, dameq);
	if ( damobj )
	{
	  if ( dam > get_obj_resistance(damobj) && victim->position > POS_STUNNED && victim->hit > 100 ) /* prevent armor bashing - Funf */
	  {
		 if ( number_range( 1,5 ) != 5 ) /* Added 20% chance to avoid eq damage -Funf */
		 {
	     	set_cur_obj(damobj);
	     	damage_obj(damobj);
		 }
	  }
	  dam -= 5;  /* add a bonus for having something to block the blow */
		/* 1 in 16 chance to begin or worsen bleeding */
		if (dam > 18)
		{
		if ( !IS_NPC(victim) && number_range( 1,10 ) == 1 )
			gain_condition( victim, COND_BLEEDING, 1 );
		}
	}
	else
	  dam += 5;  /* add penalty for bare skin! */
		/* 1 in 6 chance to begin or worsen bleeding */
	    if (dam > 18)
		{
		if ( !IS_NPC(victim) && number_range( 1,5 ) == 1 )
			gain_condition( victim, COND_BLEEDING, 1 );
		}
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

    victim->hit -= dam;

    /*
     * Get experience based on % of damage done			-Thoric
     */
    if ( dam && ch != victim
    &&  !IS_NPC(ch) && ch->fighting && ch->fighting->xp )
    {
	xp_gain = (int) (xp_compute( ch, victim ) * 1 * dam) / victim->max_hit;
	gain_exp( ch, xp_gain, COMBAT_ABILITY );
    }

    if ( !IS_NPC(victim)
    &&   victim->top_level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
       victim->hit = 1;

    /* Make sure newbies dont die */

    if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
	victim->hit = 1;

    if ( dam > 0 && dt > TYPE_HIT
    && !IS_AFFECTED( victim, AFF_POISON )
    &&  is_wielding_poisoned( ch )
    && !IS_SET( victim->immune, RIS_POISON )
    && !saves_poison_death( ch->skill_level[COMBAT_ABILITY], victim ) )
    {
	AFFECT_DATA af;

	af.type      = gsn_poison;
	af.duration  = 20;
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = AFF_POISON;
	affect_join( victim, &af );
    }
    // Prevent players from killing others whilst Toggled
    if( victim->hit < 1 && !IS_NPC(ch) && !IS_NPC(victim) &&
	IS_SET(ch->pcdata->flags, PCFLAG_TOGGLE) )
    {
      act( AT_ACTION, "$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM );
      act( AT_HURT, "You are stunned, but will probably recover.", victim, NULL, NULL, TO_CHAR );
      stop_fighting( victim, TRUE );
      victim->hit = -10;
      update_pos( victim );
      return rNONE;
     }


    if ( !npcvict
    &&   get_trust(victim) >= LEVEL_IMMORTAL
    &&	 get_trust(ch)	   >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( AT_DYING, "$n is mortally wounded, and about to die.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( "&RYou are knocked unconcious, and are bleeding freely.\n\r",victim);

	break;

    case POS_INCAP:
	act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( "&RYou are incapacitated and will slowly die, if not aided.\n\r",victim);
	break;

    case POS_STUNNED:
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
	  act( AT_ACTION, "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	  send_to_char( "&RYou are stunned, but will probably recover.\n\r",victim);
	}
	break;

    case POS_DEAD:
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];

	    if ( skill->die_char && skill->die_char[0] != '\0' )
	      act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
	    if ( skill->die_vict && skill->die_vict[0] != '\0' )
	      act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
	    if ( skill->die_room && skill->die_room[0] != '\0' )
	      act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT );
	}
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_NOKILL )  )
	   act( AT_YELLOW, "$n flees for $s life ... barely escaping certain death!", victim, 0, 0, TO_ROOM );
	else if ( IS_NPC(victim) && IS_SET( victim->act, ACT_DROID )  )
	   act( AT_DEAD, "$n EXPLODES into many small pieces!", victim, 0, 0, TO_ROOM );
	else
	   if( IS_NPC(victim))
	   {
		// This prevents no Corpse mobs from being 'dead' per se
		if(!IS_SET(victim->act, ACT_NOCORPSE))
			act( AT_DEAD, "$n is DEAD!", victim, 0, 0, TO_ROOM );
		else
			act( AT_DEAD, "$n has been DEFEATED!", victim, 0, 0, TO_ROOM);
	   }
	   else
	   	act( AT_DEAD, "$n is about to DIE!", victim, 0, 0, TO_ROOM );

	break;

    default:
	if ( dam > (victim->max_hit / 4 - 60))
	{
	   act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
   	   /* Start bleeding unless level is 5 or less */
	   if ( !IS_NPC(victim) && victim->top_level > 5 )
	   gain_condition( victim, COND_BLEEDING, 3 );

	}
	if ( victim->hit < (victim->max_hit / 4 - 40) )
	{
	   act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!", victim, 0, 0, TO_CHAR );
	}
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim)		/* lets make NPC's not slaughter PC's */
    &&   !IS_AFFECTED( victim, AFF_PARALYSIS ) )
    {
	if ( victim->fighting
	&&   victim->fighting->who->hunting
	&&   victim->fighting->who->hunting->who == victim )
	   stop_hunting( victim->fighting->who );

	if ( victim->fighting
	&&   victim->fighting->who->hating
	&&   victim->fighting->who->hating->who == victim )
	   stop_hating( victim->fighting->who );

	stop_fighting( victim, TRUE );
    }

    if ( victim->hit <=0 && !IS_NPC(victim))
    {
       	OBJ_DATA *obj;
       	OBJ_DATA *obj_next;
       	int cnt=0;
		long long int wulongs = victim->gold;

       	REMOVE_BIT( victim->act, PLR_ATTACKER );

       	stop_fighting( victim, TRUE );

       	if ( ( obj = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL )
       	   unequip_char( victim, obj );
       	if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) != NULL )
       	   unequip_char( victim, obj );
       	if ( ( obj = get_eq_char( victim, WEAR_HOLD ) ) != NULL )
       	   unequip_char( victim, obj );
       	if ( ( obj = get_eq_char( victim, WEAR_MISSILE_WIELD ) ) != NULL )
       	   unequip_char( victim, obj );
       	if ( ( obj = get_eq_char( victim, WEAR_LIGHT ) ) != NULL )
          unequip_char( victim, obj );

		/* Drop Wulongs when you die -Kristen- */
		if ( victim->gold != 0)
		{
			victim->gold = 0;

		    for ( obj = ch->in_room->first_content; obj; obj = obj_next )
		    {
				obj_next = obj->next_content;

				switch ( obj->pIndexData->vnum )
				{
				case OBJ_VNUM_MONEY_ONE:
				   wulongs += 1;
				   extract_obj( obj );
				   break;

				case OBJ_VNUM_MONEY_SOME:
				   wulongs += obj->value[0];
				   wulongs += obj->value[1] * 2000000000LL;
				   extract_obj( obj );
				   break;
				}

		    }

		    act( AT_ACTION, "$n drops some wulongs.", victim, NULL, NULL, TO_ROOM );
		    obj_to_room( create_money( wulongs ), victim->in_room );
		    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
				save_char_obj( victim );
		}

    for ( obj = victim->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc == WEAR_NONE )
	    {
		do_drop(victim, "all");
		break;
		if ( obj->pIndexData->progtypes & DROP_PROG && obj->count > 1 )
		{
		   ++cnt;
		   separate_obj( obj );
		   obj_from_char( obj );
		   if ( !obj_next )
		     obj_next = victim->first_carrying;
		}
		else
		{
		   cnt += obj->count;
		   obj_from_char( obj );
		}
		act( AT_ACTION, "$n drops $p.", victim, obj, NULL, TO_ROOM );
		act( AT_ACTION, "You drop $p.", victim, obj, NULL, TO_CHAR );
		obj = obj_to_room( obj, victim->in_room );
	    }
	}

       if ( IS_NPC( ch ) && !IS_NPC( victim ) )
       {
           long long int lose_exp;
           lose_exp = UMAX( ( victim->experience[COMBAT_ABILITY] - exp_level ( victim->skill_level[COMBAT_ABILITY] ) ) , 0 );
           victim->experience[COMBAT_ABILITY] -= lose_exp;
       }

      add_timer( victim, TIMER_RECENTFIGHT, 100, NULL, 0 );

    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !npcvict )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );

	}
	else
	if ( !IS_NPC(ch) )		/* keep track of mob vnum killed */
	    add_kill( ch, victim );

/*	check_killer( ch, victim );	-added to raw_kill */

	if ( !IS_NPC(victim) || !IS_SET( victim->act, ACT_NOKILL )  )
	   loot = legal_loot( ch, victim );
        else
           loot = FALSE;

	set_cur_char(victim);
	raw_kill( ch, victim );


	if ( !IS_NPC(ch) && loot )
	{
	   if ( IS_NPC(victim) && loot ) /*no looting of players cause there is no corpses JET<RGC>08-10-03*/
		{
	   	/* Autogold by Scryn 8/12 */
	    		if ( IS_SET(ch->act, PLR_AUTOGOLD) )
	    		{
				init_gold = ch->gold;
				do_get( ch, "wulongs corpse" );
				new_gold = ch->gold;
				gold_diff = (new_gold - init_gold);
				if (gold_diff > 0)
                		{
                  			sprintf(buf1,"%lld",gold_diff);
		  			do_split( ch, buf1 );
				}
	    		}
	    		if ( IS_SET(ch->act, PLR_AUTOLOOT) )
				do_get( ch, "all corpse" );
	    		else
				do_look( ch, "in corpse" );

			if ( IS_SET(ch->act, PLR_AUTOSAC) )
			do_sacrifice( ch, "corpse" );
		}
	}
	if ( IS_SET( sysdata.save_flags, SV_KILL ) )
	   save_char_obj( ch );
	return rVICT_DIED;
    }

    if ( victim == ch )
	return rNONE;

    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc && !victim->switched )
    {
	if ( number_range( 0, victim->wait ) == 0)
	{
	    do_flee( victim, "" );
	    do_flee( victim, "" );
	    do_flee( victim, "" );
	    do_flee( victim, "" );
	    do_flee( victim, "" );
	    do_quit( victim, "" );
	    return rNONE;
	}
    }

    /*
     * Wimp out?
     */
    if ( npcvict && dam > 0 )
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
	&&   victim->hit < victim->max_hit / 2 )
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master
	&&     victim->master->in_room != victim->in_room ) )
	{
	    start_fearing( victim, ch );
	    stop_hunting( victim );
	    do_flee( victim, "" );
	}
    }

    if ( !npcvict
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait == 0 )
	do_flee( victim, "" );
    else
    if ( !npcvict && IS_SET( victim->act, PLR_FLEE ) )
	do_flee( victim, "" );

    tail_chain( );
    return rNONE;
}

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !victim )
        return FALSE;

    /* Thx Josh! */
    if ( who_fighting( ch ) == ch )
	return FALSE;
    if(IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	&& victim->adren > 0)
	return FALSE;

    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) ||
	 IS_SET( victim->in_room->room_flags, ROOM_PLR_HOME)  )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You'll have to do that elsewhere.\n\r", ch );
	return TRUE;
    }

    if ( get_trust(ch) > LEVEL_HERO )
            return FALSE;

    if ( IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;


return FALSE;

}

/* checks is_safe but without the output
   cuts out imms and safe rooms as well
   for info only */

bool is_safe_nm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    return FALSE;
}


/*
 * just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
  /* pc's can now loot .. why not .. death is pretty final */
  if ( !IS_NPC(ch) )
     return TRUE;
  /* non-charmed mobs can loot anything */
  if ( IS_NPC(ch) && !ch->master )
    return TRUE;

  return FALSE;
 }

/*
see if an attack justifies a KILLER flag --- edited so that none do but can't
murder a no pk person. --- edited again for planetary wanted flags -- well will be soon :p
 */

void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{

    int x;

    /*
    * Charm-o-rama.
    */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	/* stop_follower( ch ); */
	if ( ch->master )
	  check_killer( ch->master, victim );
    }

    if ( IS_NPC(victim) )
    {
	if ( !IS_NPC( ch ) )
	{
	  for ( x = 0; x < 32; x++ )
	  {
	      if ( IS_SET(victim->vip_flags , 1 << x ) && !IS_SET( victim->act , ACT_DEADLY ) )
	      {
	         SET_BIT(ch->pcdata->wanted_flags, 1 << x );
	         ch_printf( ch, "&YYou are now wanted on %s.&w\n\r", planet_flags[x] , victim->short_descr );
		 place_bounty( victim, ch,  150000, 0);
	      }
	  }
	  if ( ch->pcdata->clan )
	    ch->pcdata->clan->mkills++;
	  ch->pcdata->mkills++;
	  ch->in_room->area->mkills++;
	}
	return;
    }

    if ( !IS_NPC(ch) && !IS_NPC(victim) && ch != victim)
    {
	if ( ch->pcdata->clan )
	ch->pcdata->clan->pkills++;
	  ch->pcdata->pkills++;
	update_pos(victim);
	if ( victim->pcdata->clan )
	victim->pcdata->clan->pdeaths++;
      victim->pcdata->pdeaths++;
    }


    if ( IS_NPC(ch) )
      if ( !IS_NPC(victim) )
	victim->in_room->area->mdeaths++;

    return;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( !victim )
    {
      bug( "update_pos: null victim", 0 );
      return;
    }

    if ( victim->hit > 0 )
    {
	if ( victim->position <= POS_STUNNED )
	  victim->position = POS_STANDING;
	if ( IS_AFFECTED( victim, AFF_PARALYSIS ) )
	  victim->position = POS_STUNNED;
	return;
    }

    if ( IS_NPC(victim) || victim->hit <= -75 )
    {
	if ( victim->mount )
	{
	  act( AT_ACTION, "$n falls from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	  REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	  victim->mount = NULL;
	}
	victim->position = POS_DEAD;
	return;
    }

	victim->position = POS_STUNNED;

    if ( victim->position > POS_STUNNED
    &&   IS_AFFECTED( victim, AFF_PARALYSIS ) )
      victim->position = POS_STUNNED;

    if ( victim->mount )
    {
	act( AT_ACTION, "$n falls unconscious from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	victim->mount = NULL;
    }
    return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    FIGHT_DATA *fight;

    if ( ch->fighting )
    {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Set_fighting: %s -> %s (already fighting %s)",
		ch->name, victim->name, ch->fighting->who->name );
	bug( buf, 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
      affect_strip( ch, gsn_sleep );

    /* Limit attackers -Thoric */
    if ( victim->num_fighting > max_fight(victim) )
    {
	send_to_char( "There are too many people fighting for you to join in.\n\r", ch );
	return;
    }

    CREATE( fight, FIGHT_DATA, 1 );
    fight->who	 = victim;
    fight->xp	 = (int) xp_compute( ch, victim );
    fight->align = align_compute( ch, victim );
    if ( !IS_NPC(ch) && IS_NPC(victim) )
      fight->timeskilled = times_killed(ch, victim);
    ch->num_fighting = 1;
    ch->fighting = fight;
    ch->position = POS_FIGHTING;
    victim->num_fighting++;
    if ( victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
	send_to_char( "You are disturbed!\n\r", victim->switched );
	do_return( victim->switched, "" );
    }
    return;
}

CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
    if ( !ch )
    {
	bug( "who_fighting: null ch", 0 );
	return NULL;
    }
    if ( !ch->fighting )
      return NULL;
    return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
   if ( !ch )
   {
	bug( "Free_fight: null ch!", 0 );
	return;
   }
   if ( ch->fighting )
   {
     if ( !char_died(ch->fighting->who) )
       --ch->fighting->who->num_fighting;
     DISPOSE( ch->fighting );
   }
   ch->fighting = NULL;
   if ( ch->mount )
     ch->position = POS_MOUNTED;
   else
     ch->position = POS_STANDING;
   /* Berserk wears off after combat. -- Altrag */
   if ( IS_AFFECTED(ch, AFF_BERSERK) )
   {
     affect_strip(ch, gsn_berserk);
     set_char_color(AT_WEAROFF, ch);
     send_to_char(skill_table[gsn_berserk]->msg_off, ch);
     send_to_char("\n\r", ch);
   }
   return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    free_fight( ch );
    update_pos( ch );

    if ( !fBoth )   /* major short cut here by Thoric */
      return;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( who_fighting( fch ) == ch )
	{
	    free_fight( fch );
	    update_pos( fch );
	}
    }
    return;
}



void death_cry( CHAR_DATA *ch )
{

    return;
}

void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{

    CHAR_DATA *victmp;

    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *obj_next;
    SHIP_DATA *ship;
    short drug;
    bool canget = FALSE;

    if ( !victim )
    {
      bug( "raw_kill: null victim!", 0 );
      return;
    }

/* backup in case hp goes below 1 */
    if (NOT_AUTHED(victim))
    {
      bug( "raw_kill: killing unauthed", 0 );
      return;
    }

    strcpy( arg , victim->name );

    stop_fighting( victim, TRUE );
/*
    if ( ch && !IS_NPC(ch) && !IS_NPC(victim) && ch != victim )
        claim_disintigration( ch , victim );
*/
/* Take care of polymorphed chars */
    if(IS_NPC(victim) && IS_SET(victim->act, ACT_POLYMORPHED))
    {
      char_from_room(victim->desc->original);
      char_to_room(victim->desc->original, victim->in_room);
      victmp = victim->desc->original;
      do_revert(victim, "");
      raw_kill(ch, victmp);
      return;
    }

    if ( victim->in_room && IS_NPC(victim) && victim->vip_flags != 0 && victim->in_room->area && victim->in_room->area->planet
	&& !IS_NPC(ch))
    {
       victim->in_room->area->planet->population--;
       victim->in_room->area->planet->population = UMAX( victim->in_room->area->planet->population , 0 );
       victim->in_room->area->planet->pop_support -= (float) ( 1 + 1 / (victim->in_room->area->planet->population + 1) );
       if ( victim->in_room->area->planet->pop_support < -100 )
          victim->in_room->area->planet->pop_support = -100;
    }
   // Clan Supporters die and clan might lose Pop Support on the planet - Gatz
   if(IS_NPC(victim) && !IS_NPC(ch)
	&& (IS_SET(victim->act, ACT_CITIZEN) || IS_SET(victim->act,ACT_SUPPORTER)) )
   {
	if(number_range(1,2) == 2)
		victim->in_room->area->planet->pop_support -= .5;
          // New planetary frustration code - Gatz
	  PLANET_DATA *planet;
	  CLAN_DATA   *clan;

	  if(!IS_NPC(ch))
		clan = ch->pcdata->clan;
	  planet = ch->in_room->area->planet;
	  if(planet && IS_SET(victim->act, ACT_CITIZEN))
	  {
		planet->citizencount++;
	  }
          if(planet && planet->frustrated_by && planet->frustrated_by->name
	     && planet->governed_by
	     && planet->frustrated_by != planet->governed_by
	     && clan
	     && clan->clan_type == 0)
          {
                  if( planet->frustrated_by != clan)
                  {
                        if(planet->frust_level > 0)
                                planet->frust_level -= .1 * ch->top_level/50;
                        if(planet->frust_level < 0)
                                planet->frust_level = 0;
                }
                else
                {
                        if(planet->frustrated_by == clan)
                                planet->frust_level += .3 * ch->top_level/50;
                        if(planet->frust_level > 100)
                                planet->frust_level = 100;

                }
          }
   }
   /*The following code caused the progs to fire 2 times.
    *Commented out for bug fix - Jet<RGC> 8-6-3*/

   /* mprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;
    * death_cry( victim ); *

    rprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return; */

    if ( IS_NPC(victim) || !IS_SET( victim->act, ACT_NOKILL  ) )
    mprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;

    if ( IS_NPC(victim) || !IS_SET( victim->act, ACT_NOKILL  ) )
    rprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;

	/* an attempt to incorporate pk data everywhere -Kristen- */
	check_killer( ch, victim );

    // BIG bug fix, ANSI off would make morts get a coprse.
    //
    if ( !IS_SET( victim->act, ACT_NOKILL  ) && !IS_SET( victim->act, ACT_NOCORPSE ) && IS_NPC(victim) )
    	make_corpse( victim, ch );
    if ( victim->in_room->sector_type == SECT_OCEANFLOOR
    ||   victim->in_room->sector_type == SECT_UNDERWATER
    ||   victim->in_room->sector_type == SECT_WATER_SWIM
    ||   victim->in_room->sector_type == SECT_WATER_NOSWIM )
      act( AT_BLOOD, "$n's blood slowly clouds the surrounding water.", victim, NULL, NULL, TO_ROOM );
    else if ( victim->in_room->sector_type == SECT_AIR )
      act( AT_BLOOD, "$n's blood sprays wildly through the air.", victim, NULL, NULL, TO_ROOM );
    else
      make_blood( victim );
    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	extract_char( victim, TRUE , FALSE);
	victim = NULL;
	return;
    }
 if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
	save_char_obj( victim );
//    send_to_char("&RYou are slightly aware of the IV's in your arm \n\ras you are transported to the hospital.\n\r", victim);
    send_to_char("&RYou feel a coursing of excrutiating pain through your body, and then...\n\n\r", victim );
    send_to_char("&R...nothing.  The sirens of an ambulance blare as you hear paramedics\n\r", victim );
    send_to_char("&Rshouting orders, but their voices fade away to silence.\n\n\r", victim );
    send_to_char("&GSome time later, you wake up in a hospital bed, barely clinging to life.&W\n\r", victim );
    WAIT_STATE( victim, 25 * PULSE_PER_SECOND );
    victim->adren = 0;
    for ( drug=0 ; drug <= 9 ; drug ++ )
    {
	    victim->pcdata->drug_level[drug] = 0;
	  // You need to go to re-hab for this!
	  //  victim->pcdata->addiction[drug]  = 0;
    }
    act( AT_BLOOD, "$n's life has been tested.", victim, NULL, NULL, TO_ROOM );
    // Well, server doesn't drop cores. Bug testing needs to be added to reduce this
    // giant if check = Gatz

/* Part of arrest -- Funf 2008-08-07
    BOUNTY_DATA *bounty;
    if(!IS_NPC(victim))
	bounty = get_bounty( victim->name );

    if( ch
	&& victim
	&& !IS_NPC(ch)
	&&  !IS_NPC(victim)
	&& ch->pcdata
	&& ch->pcdata->clan
	&& ch != victim
        && (!str_cmp(ch->pcdata->clan->name, "RBH"))
	&&  bounty
	&&  bounty->amount > 0)
    {
	canget = TRUE;
    }
    if( ch
	&& victim
	&& !IS_NPC(ch) && !IS_NPC(victim)
	&& ch->pcdata->clan
	&& !str_cmp(ch->pcdata->clan->name, "ISSP")
	&& bounty)
    {
	canget = TRUE;
    }*/

   // War stuff! - Gatz

	/* added for capship GRP on 2006-08-06 by Funf */
	if( !IS_NPC(ch) && !IS_NPC(victim) && ch->pcdata->clan && victim->pcdata->clan && !strcmp("Titan",ch->in_room->area->name) && strcmp("GLM", ch->pcdata->clan->name) )
	{
   		ch->pcdata->clan->warkills++;
   		if(ch->pcdata->clan->warkills >= 150)
   		{
			char buf[MAX_STRING_LENGTH];
			sprintf(buf, "%s has finally won the Second Titan War!", ch->pcdata->clan->name);
			echo_to_all( AT_RED , buf, 0 );
		}
	}

   if( !IS_NPC(ch) && !IS_NPC(victim) && ch->pcdata->clan && victim->pcdata->clan)
	add_point( ch->pcdata->clan, victim->pcdata->clan);

	//Part of arrest --Funf 2008-08-07
   //if(canget)
	//victim->pcdata->arrestcount++;


    //extract_char( victim, FALSE, canget);
	extract_char( victim, FALSE , FALSE);

    // Moved here to not be a pest with extract_char and moving folks with a bounty - Gatz
    // claim_bounty is now accessed via "Arrest". Moved outlaw and killer logic here. --Funf 2008-08-07
    if ( ch && !IS_NPC(ch) && !IS_NPC(victim) && ch != victim )
    {

		BOUNTY_DATA *bounty, *temp_bounty;
		long long int exp;
		char buf[MAX_STRING_LENGTH];
		int chance;

		bounty = get_bounty( victim->name );

		if ( IS_SET(victim->act , PLR_KILLER ) && !IS_NPC(ch) )
		{
		   exp = URANGE(1, xp_compute(ch, victim) , ( exp_level(ch->skill_level[HUNTING_ABILITY]+1) - exp_level(ch->skill_level[HUNTING_ABILITY]) ));
		   gain_exp( ch , exp , HUNTING_ABILITY );
		   set_char_color( AT_BLOOD, ch );
		   ch_printf( ch, "You receive %lld hunting experience for executing a wanted killer.\n\r", exp );
		}
		else if ( !IS_NPC(ch) )
		{
			SET_BIT(ch->act, PLR_KILLER );
			ch_printf( ch, "You are now wanted for the murder of %s.\n\r", victim->name );
		}

		if(IS_SET(victim->pcdata->flags, PCFLAG_OUTLAW) && !IS_NPC(ch) && !IS_NPC(victim))
		{
		  chance = number_range(1,2);
		  REMOVE_BIT(victim->pcdata->flags, PCFLAG_OUTLAW);
		  sprintf( buf, "The wretched outlaw %s has been killed!", victim->name);
		  echo_to_all ( AT_RED , buf, 0 );
		  switch(chance)
		  {
			  case 1:
				  ch->gold += 500000;
				  send_to_char("You are rewarded 500,000 wulongs!\n\r", ch);
				  break;
			  case 2:
				  ch->pcdata->quest_curr += 3;
				  send_to_char("You are rewarded 3 quest points!\n\r", ch);
				  break;
			  default:
				  ch->dextrain += 500;
				  send_to_char("You are rewarded a large amount of dexterity training!\n\r", ch);
		  }
		}

	}
    //claim_bounty( ch , victim );
    //END Outlaw code.

    if ( !victim )
    {
      bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
      return;
    }

    /* Remove all items now -Kristen 12/12/03- */
    /* Updated for new wear slots - Gatz */
/*
       if ( ( obj = get_eq_char( victim, WEAR_FINGER_L ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_FINGER_R ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_NECK_1 ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_NECK_2 ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_BODY ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_HEAD ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_LEGS ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_FEET ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_HANDS ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_ARMS ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_SHIELD ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_ABOUT ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_WAIST ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_WRIST_L ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_WRIST_R ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_EARS ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_EYES ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_ANKLE_L ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_ANKLE_R ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_FACE ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_HIP ) ) != NULL )
          unequip_char( victim, obj );
       if ( ( obj = get_eq_char( victim, WEAR_BACK ) ) != NULL )
          unequip_char( victim, obj );
  */
    do_remove(victim, "all");

    while ( victim->first_affect )
	affect_remove( victim, victim->first_affect );
    victim->hit		= 0;
    victim->affected_by	= race_table[victim->race].affected;
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    //victim->carry_weight= 0;
    //victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck = 0;
    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	    victim->damroll	= 0;
	    victim->hitroll	= 0;
    }
    victim->mental_state = -10;
    victim->pcdata->condition[COND_BLEEDING] = 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    /* Shut down some of those naked spammer killers - Blodkai */
    if ( victim->top_level < 150 )
	victim->mana	= UMAX( 1, victim->mana );
    else
	victim->mana	= 1;
    victim->move	= UMAX( 1, victim->move );




  if ( !victim )
  {
    DESCRIPTOR_DATA *d;

    /* Make sure they aren't halfway logged in. */
    for ( d = first_descriptor; d; d = d->next )
      if ( (victim = d->character) && !IS_NPC(victim)  )
        break;
    if ( d )
      close_socket( d, TRUE );
  }
  else
  {
    int x, y;

    quitting_char = victim;
    save_char_obj( victim );
    saving_char = NULL;
    extract_char( victim, TRUE, FALSE );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL;
  }

  sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
          capitalize( arg ) );
  sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
          capitalize( arg ) );

  rename( buf, buf2 );

  sprintf( buf, "%s%c/%s.clone", PLAYER_DIR, tolower(arg[0]),
          capitalize( arg ) );
  sprintf( buf2, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
          capitalize( arg ) );

  rename( buf, buf2 );

  return;


}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members, domlevel = 0, weaklevel = 200;


    /*
     * Monsters don't get kill xp's or alignment changes.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
        if (IS_SET(ch->act, PLR_QUESTOR)&&IS_NPC(victim))
        {
            if (ch->questmob == victim->pIndexData->vnum)
            {
		send_to_char("You have almost completed your QUEST!\n\r",ch);
                send_to_char("Return to the questmaster before your time runs out!\n\r",ch);
                ch->questmob = -1;
            }
        }

    if ( IS_NPC(ch) || victim == ch )
	return;

    members = 0;


    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	{
	    members++;
	}
    }
    ch->focus = NULL;

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
    }

    // Anti-Tanking Code - Gatz
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ))
	{
		if(gch->skill_level[COMBAT_ABILITY] > domlevel)
			domlevel = gch->skill_level[COMBAT_ABILITY];
		if(gch->skill_level[COMBAT_ABILITY] < weaklevel)
			weaklevel = gch->skill_level[COMBAT_ABILITY];
 	}
    }



    if(domlevel - weaklevel > 30)
    {
	// 1 member added per 10 levels over 30 difference
	members += (domlevel - weaklevel)/10;
    }

    lch = ch->leader ? ch->leader : ch;

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) )
	    continue;

	xp = (int) (xp_compute( gch, victim ) / members);

	if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
	{
	    ch_printf( ch, "&w&W&RGLM is suppose to make weapons, not use them!\n\r");
	    xp = xp/2;
	}

	gch->alignment = align_compute( gch, victim );

	if ( !IS_NPC(gch) && IS_NPC(victim) && gch->pcdata && gch->pcdata->clan
	&& !str_cmp ( gch->pcdata->clan->name , victim->mob_clan ) )
	{
	     xp = 0;
	     sprintf( buf, "You receive no experience for killing your organization's resources.\n\r");
	     send_to_char( buf, gch );
	}
	else
	{
	   sprintf( buf, "You receive %d combat experience.\n\r", xp );
	   send_to_char( buf, gch );
	}

        gain_exp( gch, xp , COMBAT_ABILITY );

        if ( lch == gch && members > 1 )
        {
           xp = URANGE( members, xp*members, (exp_level( gch->skill_level[LEADERSHIP_ABILITY]+1) - exp_level(gch->skill_level[LEADERSHIP_ABILITY] )/10) );
	   xp = xp/2;
           sprintf( buf, "You get %d leadership experience for leading your group to victory.\n\r", xp );
	   send_to_char( buf, gch );
           gain_exp( gch, xp , LEADERSHIP_ABILITY );
        }


	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( AT_MAGIC, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( AT_MAGIC, "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );

		obj_from_char( obj );
		obj = obj_to_room( obj, ch->in_room );
		oprog_zap_trigger(ch, obj);  /* mudprogs */
		if ( char_died(ch) )
		  return;
	    }
	}
    }

    return;
}


int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{

/* never cared much for this system

    int align, newalign;

    align = gch->alignment - victim->alignment;

    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/4,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/4, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment / 4);

    return newalign;

make it simple instead */

    return URANGE ( -1000,
                     (int) ( gch->alignment - victim->alignment/5 ),
                     1000 );

}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align;
    int xp;

    xp	  = (get_exp_worth( victim )
    	  *  URANGE( 1, (victim->skill_level[COMBAT_ABILITY] - gch->skill_level[COMBAT_ABILITY]) + 10, 20 )) / 10;
    align = gch->alignment - victim->alignment;

    /* bonus for attacking opposite alignment */
    if ( align >  990 || align < -990 )
	xp = (xp*5) >> 2;
    else
    /* penalty for good attacking same alignment */
    if ( gch->alignment > 300 && align < 250 )
	xp = (xp*3) >> 2;

    xp = number_range( (xp*3) >> 2, (xp*5) >> 2 );

       /* new xp cap for swreality */

    return URANGE(1, xp, ( exp_level(  gch->skill_level[COMBAT_ABILITY]+1 ) - exp_level( gch->skill_level[COMBAT_ABILITY]) )   );
}


/*
 * Revamped by Thoric to be more realistic
 */
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    sh_int dampc;
    struct skill_type *skill = NULL;
    bool gcflag = FALSE;
    bool gvflag = FALSE;
    int adren, avmessage, acmessage;

    if ( ! dam )
      dampc = 0;
    else
      dampc = ( (dam * 1000) / victim->max_hit) +
              ( 50 - ((victim->hit * 50) / victim->max_hit) );

   /*		     10 * percent					*/
	 	 if ( dam   ==    0 ) { vs = "&Rm&ri&Rs&rs&W";	vp = "&Rm&ri&Rs&rs&Re&rs&W";		 }
    else if ( dampc <=    5 ) { vs = "&Cb&ca&Cr&ce&Cl&cy &Cs&cc&Cr&ca&Ct&cc&Ch&W";vp = "&Cb&ca&Cr&ce&Cl&cy &Cs&cc&Cr&ca&Ct&cc&Ch&ce&Cs&W";}
    else if ( dampc <=   10 ) { vs = "&Gs&gc&Gr&ga&Gt&gc&Gh&W";	vp = "&Gs&gc&Gr&ga&Gt&gc&Gh&ge&Gs&W";	}
    else if ( dampc <=   20 ) { vs = "&Pn&pi&Pc&pk&W";	vp = "&Pn&pi&Pc&pk&Ps&W";		 }
    else if ( dampc <=   30 ) { vs = "&Bg&br&Ba&bz&Be&W";	vp = "&Bg&br&Ba&bz&Be&bs&W";		 }
    else if ( dampc <=   40 ) { vs = "&Cb&Rr&Cu&Ri&Cs&Re&W";	vp = "&Cb&Rr&Cu&Ri&Cs&Re&Cs&W";		 }
    else if ( dampc <=   50 ) { vs = "&Gh&Ci&Gt&W";	vp = "&Ch&Gi&Ct&Gs&W";		 }
    else if ( dampc <=   60 ) { vs = "&Yi&Gn&Yj&Gu&Yr&Ge&W";	vp = "&Yi&Gn&Yj&Gu&Yr&Ge&Ys&W";		 }
    else if ( dampc <=   75 ) { vs = "&Rt&Bh&Rr&Ba&Rs&Bh&W";	vp = "&Rt&Bh&Rr&Ba&Rs&Bh&Re&Bs&W";	 }
    else if ( dampc <=   80 ) { vs = "&Yw&Co&Yu&Cn&Yd&W";	vp = "&Yw&Co&Yu&Cn&Yd&Cs&W";		 }
    else if ( dampc <=   90 ) { vs = "&rm&ca&ru&cl&W";    vp = "&rm&ca&ru&cl&rs&W";		 }
    else if ( dampc <=  125 ) { vs = "&Rd&ge&Rc&gi&Rm&ga&Rt&ge&W";vp = "&Rd&ge&Rc&gi&Rm&ga&Rt&ge&Rs&W";	 }
    else if ( dampc <=  150 ) { vs = "&Pd&Ce&Pv&Ca&Ps&Ct&Pa&Ct&Pe&W";vp = "&Pd&Ce&Pv&Ca&Ps&Ct&Pa&Ct&Pe&Cs&W";	 }
    else if ( dampc <=  200 ) { vs = "&Rm&Ya&Ri&Ym&W";	vp = "&Rm&Ya&Ri&Ym&Rs&W";		 }
    else if ( dampc <=  300 ) { vs = "&CMUTILATE&W";vp = "&CMUTILATES&W";	 }
    else if ( dampc <=  400 ) { vs = "&GDISEMBOWEL&W";vp = "&GDISEMBOWELS&W";	 }
    else if ( dampc <=  500 ) { vs = "&RMASSACRE&W";  vp = "&RMASSACRES&W";	 }
    else if ( dampc <=  600 ) { vs = "&YPULVERIZE&W"; vp = "&YPULVERIZES&W";	 }
    else if ( dampc <=  750 ) { vs = "&RE&rV&RI&rS&RC&rE&RR&rA&RT&RE&W";vp = "&RE&rV&RI&rS&RC&rE&RR&rA&RT&rE&RS&W";	 }
    else if ( dampc <=  990 ) { vs = "&R* &COBLITERATE &R*&W"; vp = "&R* &COBLITERATES &R*&W";	}
    else                      { vs = "&R*&C*&R* &CANNIHILATE &R*&C*&R*&W"; vp = "*** ANNIHILATES ***";	}

    punct   = (dampc <= 30) ? '.' : '!';

    if ( dam == 0 && (!IS_NPC(ch) &&
       (IS_SET(ch->pcdata->flags, PCFLAG_GAG)))) gcflag = TRUE;

    if ( dam == 0 && (!IS_NPC(victim) &&
       (IS_SET(victim->pcdata->flags, PCFLAG_GAG)))) gvflag = TRUE;

    if ( dt >=0 && dt < top_sn )
	skill = skill_table[dt];

    if ( dt == TYPE_HIT || dam==0 )
    {
	sprintf( buf1, "$n %s $N%c",  vp, punct );
	sprintf( buf2, "You %s $N%c", vs, punct );
	sprintf( buf3, "$n %s you%c", vp, punct );
    }
    else
    if ( dt > TYPE_HIT && is_wielding_poisoned( ch ) )
    {
	if ( dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	    attack	= attack_table[dt - TYPE_HIT];
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
        }

	sprintf( buf1, "$n's poisoned %s %s $N%c", attack, vp, punct );
	sprintf( buf2, "Your poisoned %s %s $N%c", attack, vp, punct );
	sprintf( buf3, "$n's poisoned %s %s you%c", attack, vp, punct );
    }
    else
    {
	if ( skill )
	{
	    attack	= skill->noun_damage;
	    if ( dam == 0 )
	    {
		bool found = FALSE;

		if ( skill->miss_char && skill->miss_char[0] != '\0' )
		{
		   act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->miss_room && skill->miss_room[0] != '\0' )
		{
		   act( AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )	/* miss message already sent */
		  return;
	    }
	    else
	    {
		if ( skill->hit_char && skill->hit_char[0] != '\0' )
		  act( AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR );
		if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
		  act( AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT );
		if ( skill->hit_room && skill->hit_room[0] != '\0' )
		  act( AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
	    }
	}
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	    attack	= attack_table[dt - TYPE_HIT];
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
	}

	sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
	sprintf( buf2, "Your %s %s $N%c",  attack, vp, punct );
	sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
    }

    if ( ch->skill_level[COMBAT_ABILITY] >= 100 )
       sprintf( buf2, "%s You do %d points of damage.", buf2, dam);

    act( AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT );
    if (!gcflag)  act( AT_HIT, buf2, ch, NULL, victim, TO_CHAR );
    if (!gvflag) act( AT_HITME, buf3, ch, NULL, victim, TO_VICT );

   //Adrenaline - Gatz

    adren = number_range(0,3);

    if(!IS_NPC(ch) && !IS_NPC(victim))
    {
	if(ch->adren == 0)
            acmessage == 0;
        if(victim->adren == 0)
            avmessage == 0;

	if(adren > 0)
	{
		if(victim->adren < 5)
		{
			if ( avmessage == 0 )
			{
			  send_to_char( "&RYou feel your heart rate speed up!\n\r", victim);
		          avmessage++;
   		        }
                   	victim->adren += adren;
			if(victim->adren > 5)
			{
				send_to_char( "&RYou feel your chest start to burn with Adrenaline!\n\r", victim);
				victim->adren = 5;
			}

		}
		if(ch->adren < 5)
		{
			if ( acmessage == 0)
			{
                          send_to_char( "&RYou feel your heart rate speed up!\n\r", ch);
		          acmessage++;
                     	}
                        ch->adren += adren;
			if(ch->adren > 5)
			{
				send_to_char( "&RYou feel your chest start to burn with Adrenaline!\n\r", ch);
				ch->adren = 5;
			}

		}
	}

  }


    return;
}


void do_kill( CHAR_DATA *ch, char *argument )
{
     send_to_char( "We don't have kill or murder. Please see help focus.\n\r", ch);
}

void do_attack( CHAR_DATA *ch, char *argument )
{
	//char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wield;

        // BIG bug fix - Gatz
        if(IS_NPC(ch) && ch->position != POS_FIGHTING)
        {
                send_to_char("Aren't you going to fight someone first?\n\r", ch);
                return;
        }


	/* 	Required because of focus	Jet/Kristen 8/11/03 */

	if (ch->focus == "none" || ch->focus == NULL)
	{
		send_to_char( "You must focus on an opponent before attacking.\n\r", ch );
		return;
	}
	else
	{
		argument = ch->focus;
	}

	/*one_argument( argument, arg ); *//* This was unneeded and causing a bug when
										groups would attack someone who shared a
										prefix with someone in the room - Funf 2008-03-24 */


	if ( ( victim = get_char_room( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}


	if ( is_safe( ch, victim ) && victim->adren == 0 )
		return;


	/*if ( ch->position == POS_FIGHTING )
	{
		send_to_char( "You do the best you can!\n\r", ch );
		return;
	}*/

	if ( victim->vip_flags != 0 )
		ch->alignment -= 10;
	wield = get_eq_char( ch, WEAR_WIELD );

	WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
	//Custom Wait_stats - Gatz
	if(!IS_NPC(ch))
	{
	  if(wield != NULL)
	  {
		if(wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_SWORD)
		  WAIT_STATE(ch, 8);
		if ( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
		  WAIT_STATE(ch, 5);
	   }
	}

	multi_hit( ch, victim, TYPE_UNDEFINED );
     	// Mobile Frustration Code - Gatz

	if(IS_NPC(victim) && number_range(0,3) == 3)
	{
		if(victim->frustration == 0 && victim->frustrated_by)
			victim->frustrated_by = ch;
		victim->frustration++;
		if(victim->frustration > 100)
			victim->frustration = 100;
	}


	// Lag for Guns/Rifles - Gatz
       if(!IS_NPC(ch))
	{
	  if( wield != NULL)
	  {
		if(wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_PISTOL)
        	  WAIT_STATE(ch,8);
       		if(wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_RIFLE)
        	  WAIT_STATE(ch,12);
		if ( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
		  WAIT_STATE(ch, 5);
	  }
	}
	// Gain some exercise from attacking!
	if(!IS_NPC(ch))
	{
		//ch->passive_strtrain += 2;
		//victim->passive_contrain++; Handled in damage code... -Funf
	}
        if (!victim->fighting && victim->in_room == ch->in_room && IS_NPC(victim))
        {
                    set_fighting( victim, ch );
                    start_hating( victim, ch );
                    start_hunting( victim, ch );
        }

	/*
	if( !IS_NPC(ch))
		set_fighting( ch, victim);
	*/
	return;
}


bool in_arena( CHAR_DATA *ch )
{

if ( !str_cmp( ch->in_room->area->filename, "arena.are" ) )
  return TRUE;

if ( ch->in_room->vnum < 29 || ch->in_room->vnum > 43 )
  return FALSE;

return TRUE;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    char buf[MAX_STRING_LENGTH];
    int attempt;
    sh_int door;
    EXIT_DATA *pexit;

    if ( !who_fighting( ch ) )
    {
	if ( ch->position == POS_FIGHTING )
	{
	  if ( ch->mount )
	    ch->position = POS_MOUNTED;
	  else
	    ch->position = POS_STANDING;
	}
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ch->move <= 0 )
    {
	send_to_char( "You're too exhausted to flee from combat!\n\r", ch );
	return;
    }

    /* No fleeing while stunned. - Narn */
    if ( ch->position < POS_FIGHTING )
	return;

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 8; attempt++ )
    {

	door = number_door( );
	if ( ( pexit = get_exit(was_in, door) ) == NULL
	||   !pexit->to_room
	|| ( IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

        affect_strip ( ch, gsn_sneak );
        REMOVE_BIT   ( ch->affected_by, AFF_SNEAK );
	if ( ch->mount && ch->mount->fighting )
	    stop_fighting( ch->mount, TRUE );
	move_char( ch, pexit, 0 );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	stop_fighting( ch, TRUE ); // BUGFIX: Stop fighting before you move. Flee was used to bypass rprogs which force move certain players. Funf 2008-07-14

	ch->in_room = was_in;
	act( AT_FLEE, "$n runs for cover!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;
	act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );
        sprintf(buf, "You run for cover!\n\r");
        send_to_char( buf, ch );

	return;
    }

    sprintf(buf, "You attempt to run for cover!\n\r");
    send_to_char( buf, ch );
    return;
}

bool get_cover( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    int attempt;
    sh_int door;
    EXIT_DATA *pexit;

    if ( !who_fighting( ch ) )
	return FALSE;

    if ( ch->position < POS_FIGHTING )
	return FALSE;

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 10; attempt++ )
    {

	door = number_door( );
	if ( ( pexit = get_exit(was_in, door) ) == NULL
	||   !pexit->to_room
	|| ( IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

        affect_strip ( ch, gsn_sneak );
        REMOVE_BIT   ( ch->affected_by, AFF_SNEAK );
	if ( ch->mount && ch->mount->fighting )
	    stop_fighting( ch->mount, TRUE );
	move_char( ch, pexit, 0 );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( AT_FLEE, "$n sprints for cover!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;
	act( AT_FLEE, "$n spins around and takes aim.", ch, NULL, NULL, TO_ROOM );

	stop_fighting( ch, TRUE );

	return TRUE;
    }

    return FALSE;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}


void do_focus(CHAR_DATA *ch,  char *argument )
{
	char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	char buf1[256];
	char log_buf [MAX_INPUT_LENGTH];
	bool check = FALSE;
	if (ch->focus == NULL)
		ch->focus = "none";
	one_argument( argument, arg );

	if ( arg[0] == '\0' )
    {
		if (ch->focus != "none")
		{
			sprintf( buf1, "Focus is on %s.\n\r", ch->focus );
			send_to_char( buf1, ch );
			return;
		}
		else
		{
			send_to_char( "Focus on whom?\n\r", ch );
			return;
		}
    }

	if ( !str_cmp( argument, "none" ) )
    {
		if (ch->focus == "none")
		{
			send_to_char( "You are going to stop focusing on nobody?\n\r", ch );
			return;
		}
		else
		{
			sprintf( buf1, "You quit focusing on %s.\n\r", ch->focus );
			send_to_char( buf1, ch );
			ch->focus = "none";
			return;
    	}
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
    }

	if ( victim == ch )
    {
		send_to_char( "A bit self centered aren't you?\n\r", ch );
		return;
    }

    // Frustration code for mobs - Gatz
    if(IS_NPC(victim) && victim->frustration > 60)
    {
	switch(number_range(0,3))
	{
		case 3:
			do_say(victim, "Oh God, don't even think about it!");
		break;
		case 2:
			do_say(victim, "You are pissing me off, buzz off!");
		break;
		case 1:
			do_say(victim, "If you want to fight, you picked the wrong day!");
		break;
		default:
			do_say(victim, "I am in a bad mood, don't mess with me.");
		break;
	}
    }

    // Ick! Can't attack people who are in your group or following!
	if ( ch->master == victim )
    {
		send_to_char("You can't attack the person you are following!\n\r",ch);
		return;
    }
       if ( is_same_group(ch,victim) )
    {
		send_to_char("You can't fight your own group!\n\r",ch);
		return;
    }
    // Pacifist - Gatz
	if(IS_NPC(victim) && IS_SET( victim->act, ACT_PACIFIST))
	{
		send_to_char("&BThey are a pacifist. \n\r", ch);
		return;
	}
	// Safe room edit!
	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE) && victim->adren == 0)
	{
		send_to_char("You can't do that here!\n\r", ch);
		return;
	}
	ch->focus = victim->name;
	check = FALSE;
	if(!IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch))
	{
   		BOUNTY_DATA *bounty;
    		if(!IS_NPC(victim))
        		bounty = get_bounty( victim->name );

		if(ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name, "RBH") && bounty)
			check = TRUE;
		 if(ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name, "ISSP") && bounty)
                        check = TRUE;
		if(ch->pcdata->clan && victim->pcdata->clan
			&& (ch->pcdata->clan->war1 == victim->pcdata->clan || ch->pcdata->clan->war2 == victim->pcdata->clan))
			check = TRUE;
	}

	if(!check && !IS_NPC(victim) && !IS_NPC(ch))
	{
		sprintf(log_buf, "%s has focused on %s! Possible Illegal Action!", ch->name, victim->name);
		log_string( log_buf );
        	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
	}
	act( AT_PLAIN, "You focus your attention on $N.", ch, NULL, victim, TO_CHAR );
	act( AT_PLAIN, "$n focuses $s attention on you.", ch, NULL, victim, TO_VICT );
	act( AT_PLAIN, "$n focuses $s attention on $N.", ch, NULL, victim, TO_NOTVICT );
	// Auto-focus - Gatz

	if( !IS_NPC(victim) && !IS_NPC(ch) && IS_SET(victim->pcdata->flags, PCFLAG_AUTOFOCUS)
	    && (victim->focus == "none" || victim->focus == NULL))
	{
		if(!IS_NPC(victim) && (victim->focus == "none" || victim->focus == NULL))
		{
		act( AT_PLAIN, "You quickly focus back on $n.", ch, NULL, victim, TO_VICT);
		act( AT_PLAIN, "$N quickly focuses on you.", ch, NULL, victim, TO_CHAR);
		act( AT_PLAIN, "$N quickly focuses on $n.", ch, NULL, victim, TO_NOTVICT);
		victim->focus = ch->name;
		}
	}

	return;
}
/*
 *  Anti-tanking code
 *  Author: Cronel (supfly@geocities.com) based on idea by Krage
 *  of FrozenMUD (empire.digiunix.net 4000)
 */

/* This is called from violence_update, for mobs fighting PCs only;
 * ch is the mob, victim is the player. Returns TRUE if "victim"
 * has changed, FALSE otherwise.
 * The code simply checks if there's someone else fighting this
 * mob in the room, and if this person is under ten levels from the
 * one the mob is currently fighting, then powerlevel-tanking has
 * been detected. Switch and kill the lowbie! Ha Ha Ha!
 */
// Anti-tanking snippet adapted to fit bebop code by Gatz
bool check_anti_tanking( CHAR_DATA *ch, CHAR_DATA *victim )
{

	CHAR_DATA *lowbie;
	CHAR_DATA *tanker;
	char buf[MAX_STRING_LENGTH];

	for( lowbie = ch->in_room->first_person ; lowbie ; lowbie = lowbie->next_in_room )
	{
		if( lowbie == ch
		|| lowbie == victim
		|| IS_NPC(lowbie) )
			continue;
		if( who_fighting(lowbie) == ch
		&& lowbie->skill_level[COMBAT_ABILITY] < victim->skill_level[COMBAT_ABILITY] - 20 )
		{
			act( AT_CARNAGE, "$n sees that $N has joined the fight, and decides $e is a much better prey!",
				ch, NULL, lowbie, TO_ROOM );
			do_yell( ch, "Younger meat, ha ha!" );
			tanker = who_fighting( ch );
			stop_fighting( ch, FALSE );
			set_fighting( ch, lowbie );
			start_hunting( ch, lowbie);
			start_hating( ch, lowbie);
			sprintf( buf, "check_anti_tanking: Tanking detected. %s is tanking for %s, with mob %d",
				tanker->name, lowbie->name,
				ch->pIndexData->vnum );
			log_string( buf );
			return TRUE;
		}

	}
	return FALSE;
}

void ricochet( CHAR_DATA *ch)
{
    CHAR_DATA *rch = NULL;
    sh_int count, turn, damage, round;
    bool weaponcheck = FALSE;
    OBJ_DATA *wield;



    if(!IS_NPC(ch))
    {
	wield = get_eq_char( ch, WEAR_WIELD );

        if( wield != NULL)
        {
		if(wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_RIFLE)
			weaponcheck = TRUE;
		if(wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_PISTOL)
			weaponcheck = TRUE;
	}
    }
    if( !weaponcheck)
	return;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
	if(rch)
		count++;
    }

    if(count > 1)
    {
	if(number_range(0,5) > 4)
	{
		round = number_range(1, count);
		rch = ch->in_room->first_person;
		for ( turn = 1; turn == round; turn++)
    		{
			rch = rch->next_in_room;

		}
		if( !rch )
		{
			bug( "ERROR: Ricochet NULL victim!", 0 );
			return;
		}
		act(AT_YELLOW, "$n's bullet ricochets and hits $N!", ch, NULL, rch, TO_NOTVICT);
		act(AT_YELLOW, "$n's bullet ricochets and hits you!", ch, NULL, rch, TO_VICT);
		act(AT_YELLOW, "Your bullet ricochets and hits $N!", ch, NULL, rch, TO_CHAR);
		damage = ch->skill_level[COMBAT_ABILITY]/2;
		damage += damage/2;
		rch->hit -= damage;
		if(IS_IMMORTAL(rch) && rch->hit <= 1)
			rch->hit = 1;
		update_pos( rch );
	}
    }
    return;
}


