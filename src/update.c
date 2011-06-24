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
*			      Regular update module			   *
****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"

/* from swskills.c */
void    add_reinforcements  args( ( CHAR_DATA *ch ) );

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void    gain_addiction  args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	update_taxes	args( ( void ) );
void	char_update	args( ( void ) );
void    bank_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void	room_act_update	args( ( void ) );
void	obj_act_update	args( ( void ) );
void	char_check	args( ( void ) );
void    drunk_randoms	args( ( CHAR_DATA *ch ) );
void    halucinations	args( ( CHAR_DATA *ch ) );
void	subtract_times	args( ( struct timeval *etime,
				struct timeval *stime ) );
void    quest_update    args( ( void ) );
void	support_update  args( ( void ) );
void 	citizen_generate  args( ( void ) );
void bigshot( CHAR_DATA *ch, CHAR_DATA *victim, char *argument);
/*
 * Global Variables
 */

CHAR_DATA *	gch_prev;
OBJ_DATA *	gobj_prev;

CHAR_DATA *	timechar;

PLANET_DATA * first_planet;
PLANET_DATA * last_planet;


char * corpse_descs[] =
   {
     "The corpse of %s will soon be gone.",
     "The corpse of %s lies here.",
     "The corpse of %s lies here.",
     "The corpse of %s lies here.",
     "The corpse of %s lies here."
   };

char * d_corpse_descs[] =
   {
     "The shattered remains %s will soon be gone.",
     "The shattered remains %s are here.",
     "The shattered remains %s are here.",
     "The shattered remains %s are here.",
     "The shattered remains %s are here."
   };

extern int      top_exit;

/*
 * Advancement stuff.
 */
int max_level( CHAR_DATA *ch, int ability)
{
    int level = 0;

    if ( IS_NPC(ch) )
      return 5000;

    // Small edit because level is 200 for all this shizzle - Gatz
    if ( IS_IMMORTAL(ch) )
      return 200;
    else
      return 200;

    if ( ability == COMBAT_ABILITY )
    {
       return 200;
    }

    if ( ability == PILOTING_ABILITY )
    {
       return 200;
    }

    if ( ability == ENGINEERING_ABILITY )
    {
       return 200;
    }

    if ( ability == HUNTING_ABILITY )
    {
       return 200;
    }
    if ( ability == SMUGGLING_ABILITY )
    {
       return 200;
    }

    if ( ability == LEADERSHIP_ABILITY )
    {
       return 200;
    }

    if ( ability == DIPLOMACY_ABILITY )
    {
       return 200;
    }

    if ( ability == FORCE_ABILITY )
    {
      return 200;
    }

     if ( ability == CHEMIST_ABILITY)
    {
       return 200;
    }

    level = URANGE( 1, level, 200 );

    return level;
}

void bank_update()
{
    CHAR_DATA *ch;
 	BOUNTY_DATA *bounty;
    long long int value1, value2;
    char buf[MAX_INPUT_LENGTH];

    for ( ch = last_char; ch; ch = gch_prev )
    {
		if ( ch == first_char && ch->prev )
		{
		    bug( "char_update: first_char->prev != NULL... fixed", 0 );
		    ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char( ch );
		if ( gch_prev && gch_prev->next != ch )
		{
		    bug( "char_update: ch->prev->next != ch", 0 );
		    return;
		}
        if(!IS_NPC(ch) && ch->pcdata && IS_SET(ch->pcdata->flags, PCFLAG_FREEZE) )
	{
		ch_printf( ch, "&g[&zMonthly Bank Report&g] &WBank Freeze.\r\n");
	}

    	if ( !IS_NPC( ch ) && !IS_SET(ch->pcdata->flags, PCFLAG_FREEZE))
    	{
    	   	value1 = ch->pcdata->bank;
    	   	if ( value1 < 1000000 )
    	   		value2 = (value1 * .002);
    	   	else
    	   	if ( value1 < 10000000 )
    	   		value2 = (value1 * .001);
    	   	else
    	   	if ( value1 < 100000000 )
    	   		value2 = (value1 * .0005);
    	   	else
    	   	if ( value1 < 1000000000 )
     	   		value2 = (value1 * .00025);
     	   	else
     	   		value2 = 0;
     	   	if (ch->pcdata->bank + value2 > MAX_BANK)
     	   		value2 = 0;
    		ch->pcdata->bank += value2;
    	   	sprintf(buf, "&g[&zMonthly Bank Report&g] &W%s &ZYou made&g: &W%lld wulongs this month.\n\r", ch->name, value2);
    	   	send_to_char(buf, ch);

			if ( ch->pcdata->clan )
			{
				if ( !str_cmp( ch->pcdata->clan->name, "ISSP" ) )
    			{
					value2 = 100000;
					if (ch->pcdata->bank + value2 > MAX_BANK)
						value2 = 0;
    				ch->pcdata->bank += value2;

					bounty = get_bounty( ch->name );
					if(bounty)
					{
						ch->pcdata->bank -= value2;
						value2 = 0;
					}

    				sprintf(buf, "&b[&zMonthly Salary Report&b] &W%s &ZYou made&b: &W%lld wulongs this month.\n\r", ch->name, value2);
    	   			send_to_char(buf, ch);
				}
    			if ( !str_cmp( ch->pcdata->clan->name, "BSS" ) || !str_cmp( ch->pcdata->clan->name, "RDS" ) )
		    	{
					bounty = get_bounty( ch->name );
					if ( bounty && !IS_SET(bounty->flags, FAKE_BOUNTY) )
					{
						value2 = (long long int)( .045 * bounty->amount );
		    			if ( value2 > 550000 )
		    				value2 = 550000;
					}
		    		else
		    			value2 = 0;

					if(value2 < 0)
						value2 = 0;

					if(ch->pcdata->bank < 0)
						ch->pcdata->bank = 0;
					if(ch->pcdata->bank > MAX_BANK)
						ch->pcdata->bank = MAX_BANK;

		    		ch->pcdata->bank += value2;
		    		sprintf(buf, "&r[&zMonthly Earnings Report&r] &W%s &ZYou made&r: &W%lld wulongs this month.\n\r", ch->name, value2);
		    		send_to_char(buf, ch);
				}
			}

    	}
   	}
}

void advance_level( CHAR_DATA *ch , int ability)
{

    if ( ch->top_level < ch->skill_level[ability] && ch->top_level < 100 )
    {
          ch->top_level = URANGE( 1 , ch->skill_level[ability] , 100 );
    }

/*    if ( !IS_NPC(ch) )
	REMOVE_BIT( ch->act, PLR_BOUGHT_PET );
*/
    return;
}


void gain_exp( CHAR_DATA *ch, int gain , int ability )
{
   int chance;
   int sn, i, number = 0;
   bool count = FALSE;
   int level;

    if ( IS_NPC(ch) )
	return;

    ch->experience[ability] = UMAX( 0, ch->experience[ability] + gain );

    if (NOT_AUTHED(ch) && ch->experience[ability] >= exp_level(ch->skill_level[ability]+1))
    {
	send_to_char("You can not ascend to a higher level until you are authorized.\n\r", ch);
	ch->experience[ability] = (exp_level( ch->skill_level[ability]+1 ) - 1);
	return;
    }

    // Stat code stuff
    if( ability == PILOTING_ABILITY)
	ch->dextrain++;

    while ( ch->experience[ability] >= exp_level( ch->skill_level[ability]+1))
    {
        if ( ch->skill_level[ability] >= max_level(ch , ability) )
        {
          ch->experience[ability] = (exp_level( ch->skill_level[ability]+1 ) - 1);
	  return;
        }
	// Change to remove level from level-up message &x%d remove from infront of /n/r
	++ch->skill_level[ability];

	//ch_printf( ch, "You feel a bit more experienced.\n\r");

	switch(number_range(0,6))
	{
		case 6:
			ch_printf(ch, "You feel more confident in your ability in %s.\r\n",
				supercapitalize(ability_name[ability]) );
                break;
		case 5:
			ch_printf(ch, "%s seems slightly easier for you now.\r\n",
			 	supercapitalize(ability_name[ability]) );
                break;
		case 4:
			ch_printf(ch, "You understand %s slightly better now.\r\n",
				 supercapitalize(ability_name[ability]) );
                break;
		case 3:
			ch_printf(ch, "You feel slightly more knowledgable in %s.\r\n",
				supercapitalize(ability_name[ability]) );
		break;
		case 2:
			ch_printf(ch, "You gain more experience in the field of %s.\r\n",
				supercapitalize(ability_name[ability]) );
		break;
		case 1:
			ch_printf(ch, "You become more skilled in %s.\r\n",
				supercapitalize(ability_name[ability]) );
		break;
		default:
			ch_printf(ch, "You feel your body and mind get a little wiser in %s.\r\n",
				supercapitalize(ability_name[ability]) );
		break;
	}

    // Don't let them see the prototype skill sheet. =P
    if(ability == LEADERSHIP_ABILITY && !IS_IMMORTAL(ch))
   	return;

    for (i=1; i <= 200; i++)
    {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;
	    if(i==skill_table[sn]->min_level
		&& ch->skill_level[ability] ==  skill_table[sn]->min_level )
            {
                count = TRUE;
				number++;
            }
        }
    }
	if(count)
	{
		pager_printf( ch, "You can now learn the following skill%s: ",
				(number > 1)? "s" : "");
		send_to_char( "\n\r", ch);
        }
   for (i=1; i <= 200; i++)
   {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;

            if(i==skill_table[sn]->min_level
		&& ch->skill_level[ability] ==  skill_table[sn]->min_level
		&& count )
            {
                pager_printf(ch, "&W&R%s&W ", supercapitalize(skill_table[sn]->name) );
            }
        }
    }
   send_to_char("\n\r", ch);
   level = ch->skill_level[ability];
   level++;
   count = FALSE;
   for (i=1; i <= 200; i++)
    {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;
            if(i==skill_table[sn]->min_level
                && level ==  skill_table[sn]->min_level )
            {
                count = TRUE;
                number++;
            }
        }
    }
        if(count)
        {
                pager_printf( ch, "You are almost able to learn skill%s: ",
                                (number > 1)? "s" : "");
                send_to_char( "\n\r", ch);
        }
   for (i=1; i <= 200; i++)
   {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;

            if(i==skill_table[sn]->min_level
                && level ==  skill_table[sn]->min_level
                && count )
            {
                pager_printf(ch, "&W&R%s&W ", supercapitalize(skill_table[sn]->name) );
            }
        }
    }
   if(count)
  	 send_to_char("\n\r", ch);
   level++;
   count = FALSE;
   for (i=1; i <= 200; i++)
    {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;
            if(i==skill_table[sn]->min_level
                && level ==  skill_table[sn]->min_level )
            {
                count = TRUE;
                number++;
            }
        }
    }
        if(count)
        {
                pager_printf( ch, "You are almost able to learn skill%s: ",
                                (number > 1)? "s" : "");
                send_to_char( "\n\r", ch);
        }
   for (i=1; i <= 200; i++)
   {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;

            if(i==skill_table[sn]->min_level
                && level ==  skill_table[sn]->min_level
                && count )
            {
                pager_printf(ch, "&W&R%s&W ", supercapitalize(skill_table[sn]->name) );
            }
        }
    }
   if(count)
  	 send_to_char("\n\r", ch);

   level++;
   count = FALSE;
   for (i=1; i <= 200; i++)
    {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;
            if(i==skill_table[sn]->min_level
                && level ==  skill_table[sn]->min_level )
            {
                count = TRUE;
                number++;
            }
        }
    }
        if(count)
        {
                pager_printf( ch, "You have a little ways to go to be able to learn skill%s: ",
                                (number > 1)? "s" : "");
                send_to_char( "\n\r", ch);
        }
   for (i=1; i <= 200; i++)
   {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->guild != ability)
                continue;

            if ( ch->pcdata->learned[sn] == 0
            &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;

            if(i==skill_table[sn]->min_level
                && level ==  skill_table[sn]->min_level
                && count )
            {
                pager_printf(ch, "&W&R%s&W ", supercapitalize(skill_table[sn]->name) );
            }
        }
    }
    if(count)
	    send_to_char("\n\r", ch);


	if(ch->skill_level[ability] == 200)
		send_to_char("&WYou have now maxed out your level in this field!\n\r", ch);
	if(ch->skill_level[ability] == 150 && ability == DIPLOMACY_ABILITY)
		send_to_char("&RYou can now use Broadcast! (help broadcast)\r\n", ch);
	advance_level( ch , ability );
    }

    chance = number_range(0,2);

    if(chance == 1)
    {
		if(ch->perm_int < 15 || ch->inttrain < 200)
	    	ch->inttrain++;
		ch->wistrain++;
    }
    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
		return (ch->top_level * 2);
    }
    else
    {
		gain = UMIN( 15, ch->top_level );
		gain += get_curr_con(ch) - 10;

		switch ( ch->position )
		{
			case POS_DEAD:	   return 0;
			case POS_MORTAL:   return get_curr_con(ch) * 2 ;
			case POS_INCAP:    return get_curr_con(ch) * 2 ;
			case POS_STUNNED:  return get_curr_con(ch) * 2 ;
			case POS_SLEEPING: gain += get_curr_con(ch) * 4;	break;
			case POS_RESTING:  gain += get_curr_con(ch); 		break;
		}

    }


    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
		gain = ch->top_level;
    }
    else
    {
        if ( ch->skill_level[FORCE_ABILITY] <= 1 )
          return (0 - ch->mana);

		gain = UMIN( 5, ch->skill_level[FORCE_ABILITY] / 2 );

		if ( ch->position < POS_SLEEPING )
		    return 0;
		switch ( ch->position )
		{
			case POS_SLEEPING: gain += get_curr_int(ch) * 3;	break;
			case POS_RESTING:  gain += get_curr_int(ch) * 1.5;	break;
		}
    }

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
		gain = ch->top_level;
    }
    else
    {
		gain = UMAX( 15, 2 * ch->top_level );
		gain += get_curr_con(ch) - 10;

		switch ( ch->position )
		{
		case POS_DEAD:	   return 0;
		case POS_MORTAL:   return -1;
		case POS_INCAP:    return -1;
		case POS_STUNNED:  return 1;
		case POS_SLEEPING: gain += get_curr_con(ch) * 3;	break;
		case POS_RESTING:  gain += get_curr_con(ch) * 2;	break;
		}

	    if ( IS_AFFECTED(ch, AFF_POISON) )
			gain /= 4;
	} /* You forgot to end this else block Locke. - Funf (PS. Indent!) */

    return UMIN(gain, ch->max_move - ch->move);
}

void gain_addiction( CHAR_DATA *ch )
{
    short drug;
    ch_ret retcode;
    AFFECT_DATA af;

    for ( drug=0 ; drug <= 9 ; drug ++ )
    {

       if ( ch->pcdata->addiction[drug] < ch->pcdata->drug_level[drug] )
          ch->pcdata->addiction[drug]++;

       if ( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug]+150 )
       {

           if  (ch->pcdata->addiction[drug] == 0)
           {
	       if ( !IS_AFFECTED( ch, AFF_BLIND ) )
	          {
	             	af.type      = gsn_blindness;
	      		af.location  = APPLY_AC;
	      		af.modifier  = 10;
	      		af.duration  = ch->pcdata->addiction[drug];
	      		af.bitvector = AFF_BLIND;
	      		affect_to_char( ch, &af );
	   	  }
	   }
	   if  (ch->pcdata->addiction[drug] == 1)
           {
                  if ( !IS_AFFECTED( ch, AFF_WEAKEN ) )
	          {
	             	af.type      = -1;
	      		af.location  = APPLY_DAMROLL;
	      		af.modifier  = -10;
	      		af.duration  = ch->pcdata->addiction[drug];
	      		af.bitvector = AFF_WEAKEN;
	      		affect_to_char( ch, &af );
	   	  }
	   }
           if  (ch->pcdata->addiction[drug] == 2)
           {
                  if ( !IS_AFFECTED( ch, AFF_WEAKEN ) )
	          {
	             	af.type      = -1;
	      		af.location  = APPLY_DEX;
	      		af.modifier  = -5;
	      		af.duration  = ch->pcdata->addiction[drug];
	      		af.bitvector = AFF_WEAKEN;
	      		affect_to_char( ch, &af );
	   	  }
	   }
           if  (ch->pcdata->addiction[drug] == 3)
           {
                  if ( !IS_AFFECTED( ch, AFF_WEAKEN ) )
	          {
	             	af.type      = -1;
	      		af.location  = APPLY_CON;
	      		af.modifier  = -5;
	      		af.duration  = ch->pcdata->addiction[drug];
	      		af.bitvector = AFF_WEAKEN;
	      		affect_to_char( ch, &af );
	   	  }
	   }
	   if  (ch->pcdata->addiction[drug] == 4)
           {
                  if ( !IS_AFFECTED( ch, AFF_WEAKEN ) )
                  {
                        af.type      = -1;
                        af.location  = APPLY_CON;
                        af.modifier  = -5;
                        af.duration  = ch->pcdata->addiction[drug];
                        af.bitvector = AFF_WEAKEN;
                        affect_to_char( ch, &af );
                  }
	   }

       }

       if ( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug]+200 )
       {
           ch_printf ( ch, "You feel like you are going to die. You NEED %s\n\r.",   spice_table[drug] );
           worsen_mental_state( ch, 15 );
           retcode = damage(ch, ch, 15, TYPE_UNDEFINED);
       }
       else if ( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug]+100 )
       {
           ch_printf ( ch, "You need some %s.\n\r",   spice_table[drug] );
           worsen_mental_state( ch, 10 );
       }
       else if ( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug]+50 )
       {
           ch_printf ( ch, "You really crave some %s.\n\r",   spice_table[drug] );
           worsen_mental_state( ch, 8 );
       }
       else if ( ch->pcdata->addiction[drug] > ch->pcdata->drug_level[drug]+25 )
       {
           ch_printf ( ch, "Some more %s would feel quite nice.\n\r",   spice_table[drug] );
       	   worsen_mental_state( ch, 5);
       }
       else if ( ch->pcdata->addiction[drug] < ch->pcdata->drug_level[drug]-50 )
       {
           act( AT_POISON, "$n bends over and vomits.\n\r", ch, NULL, NULL, TO_ROOM );
	   act( AT_POISON, "You vomit.\n\r", ch, NULL, NULL, TO_CHAR );
	   make_vomit( ch );
           ch->pcdata->drug_level[drug] -=10;
	   if(ch->hit > 0)
		ch->hit -= 60;
	   if(ch->hit < 0 && ch->position != POS_STUNNED)
	   {
		act( AT_ACTION, "$n keels over from a drug overdose.", ch, NULL,
		NULL, TO_ROOM);
		act( AT_ACTION, "You keel over from a drug overdose.", ch, NULL,
		NULL, TO_CHAR);
		update_pos( ch );
	  }
       }

       if ( ch->pcdata->drug_level[drug] > 1 )
          ch->pcdata->drug_level[drug] -=2;
       else if ( ch->pcdata->drug_level[drug] > 0 )
          ch->pcdata->drug_level[drug] -=1;
       else if ( ch->pcdata->addiction[drug] > 0 && ch->pcdata->drug_level[drug] <= 0 )
          ch->pcdata->addiction[drug]--;
    }

}

void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;
    ch_ret retcode;

    if ( value == 0 || IS_NPC(ch) || get_trust(ch) >= LEVEL_IMMORTAL || NOT_AUTHED(ch))
	return;

    condition	        	    = ch->pcdata->condition[iCond];
    ch->pcdata->condition[iCond]    = URANGE( 0, condition + value, 48 );
    // Bleeding code
    if ( iCond == COND_BLEEDING )
    {
	//ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 100 );
	if((ch->pcdata->condition[iCond] - value) > 0)
		ch->pcdata->condition[iCond] -= value;
	else
		ch->pcdata->condition[iCond] = 0;
	return;
    }

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:

          break;

	case COND_THIRST:

          break;

	case COND_DRUNK:
	    if ( condition != 0 ) {
                set_char_color( AT_SOBER, ch );
		send_to_char( "You are sober.\n\r", ch );
	    }
	    retcode = rNONE;
	    break;
	case COND_BLEEDING:
		// The below is remove because it was not needed
		  retcode = rNONE;
            break;

	default:
	    bug( "Gain_condition: invalid condition type %d", iCond );
	    retcode = rNONE;
	    break;
	}
    }

    if ( retcode != rNONE )
      return;

    if ( ch->pcdata->condition[iCond] == 1 )
    {
	switch ( iCond )
	{

	case COND_DRUNK:
	    if ( condition != 0 ) {
                set_char_color( AT_SOBER, ch );
		send_to_char( "You are feeling a little less light headed.\n\r", ch );
            }
	    break;
	}
    }

/* We don't need hunger or thirst! - Gatz */
    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    int door;
    ch_ret     retcode;

    retcode = rNONE;

    /* Examine all mobs. */
    for ( ch = last_char; ch; ch = gch_prev )
    {
	set_cur_char( ch );
	if ( ch == first_char && ch->prev )
	{
	    bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}

	gch_prev = ch->prev;

	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );
	    bug( "Short-cutting here", 0 );
	    gch_prev = NULL;
	    ch->prev = NULL;
	    do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
	}

	if ( !IS_NPC(ch) )
	{
	    drunk_randoms(ch);
	    halucinations(ch);
	    continue;
	}
/** Extract Day and Night mobs ** Rythmic **/
    // Some tweaks by Gatz

    if ( IS_SET(ch->act, ACT_DAY ) &&
	(time_info.hour <  5 || time_info.hour >= 19) )
	{
    act(AT_ACTION, "$n notices the sun setting and leaves.", ch, NULL, NULL, TO_ROOM);
    extract_char(ch, TRUE, FALSE);
	continue;
    }

    if ( IS_SET(ch->act, ACT_NIGHT ) &&
	(time_info.hour < 19  && time_info.hour >= 5 ))
    {
    act(AT_ACTION, "$n notices the sun rising and leaves.", ch, NULL, NULL, TO_ROOM);
    extract_char(ch, TRUE, FALSE);
	continue;
    }

	if ( !ch->in_room
	||   IS_AFFECTED(ch, AFF_CHARM)
	||   IS_AFFECTED(ch, AFF_PARALYSIS) )
	    continue;
    // Frustration Code for Mobiles - Gatz

    if(IS_NPC(ch) && ch->frustration > 10 && number_range(0, 30) > 29)
    	ch->frustration--;
    if(IS_NPC(ch) && ch->frustration > 0 && number_range(0, 70) > 65)
	ch->frustration--;
    if(IS_NPC(ch) && ch->frustrated_by && ch->frustration == 0)
	ch->frustrated_by = NULL;


/* Clean up 'animated corpses' that are not charmed' - Scryn */

        if ( ch->pIndexData->vnum == 5 && !IS_AFFECTED(ch, AFF_CHARM) )
	{
	  if(ch->in_room->first_person)
	    act(AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM);

   	  if(IS_NPC(ch)) /* Guard against purging switched? */
	    extract_char(ch, TRUE, FALSE);
	  continue;
	}

	if ( !IS_SET( ch->act, ACT_RUNNING )
	&&   !IS_SET( ch->act, ACT_SENTINEL )
	&&   !ch->fighting && ch->hunting )
	{
	  if (  ch->top_level < 20 )
	   WAIT_STATE( ch, 6 * PULSE_PER_SECOND );
	  else	if (  ch->top_level < 40 )
	   WAIT_STATE( ch, 5 * PULSE_PER_SECOND );
	  else if (  ch->top_level < 60 )
	   WAIT_STATE( ch, 4 * PULSE_PER_SECOND );
	  else	if (  ch->top_level < 80 )
	   WAIT_STATE( ch, 3 * PULSE_PER_SECOND );
	  else	if (  ch->top_level < 100 )
	   WAIT_STATE( ch, 2 * PULSE_PER_SECOND );
	  else
	   WAIT_STATE( ch, 1 * PULSE_PER_SECOND );
	  hunt_victim( ch );
	  continue;
	}
        else if ( !ch->fighting && !ch->hunting
        && !IS_SET( ch->act, ACT_RUNNING)
        && ch->was_sentinel && ch->position >= POS_STANDING )
	{
	   act( AT_ACTION, "$n leaves.", ch, NULL, NULL, TO_ROOM );
	   char_from_room( ch );
	   char_to_room( ch , ch->was_sentinel );
	   act( AT_ACTION, "$n arrives.", ch, NULL, NULL, TO_ROOM );
	   SET_BIT( ch->act , ACT_SENTINEL );
	   ch->was_sentinel = NULL;
	}

	/* Examine call for special procedure */
	if ( !IS_SET( ch->act, ACT_RUNNING )
	&&    ch->spec_fun )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	    if ( char_died(ch) )
		continue;
	}

        if ( !IS_SET( ch->act, ACT_RUNNING )
	&&    ch->spec_2 )
	{
	    if ( (*ch->spec_2) ( ch ) )
		continue;
	    if ( char_died(ch) )
		continue;
	}


	/* Check for mudprogram script on mob */
	if ( IS_SET( ch->pIndexData->progtypes, SCRIPT_PROG ) )
	{
	    mprog_script_trigger( ch );
	    continue;
	}

	if ( ch != cur_char )
	{
	    bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
	    continue;
	}

	/* That's all for sleeping / busy monster */
	if ( ch->position != POS_STANDING )
	    continue;


	if ( IS_SET(ch->act, ACT_MOUNTED ) )
	{
	    if ( IS_SET(ch->act, ACT_AGGRESSIVE) )
		do_emote( ch, "snarls and growls." );
	    continue;
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE )
	&&   IS_SET(ch->act, ACT_AGGRESSIVE) )
	    do_emote( ch, "glares around and snarls." );


	/* MOBprogram random trigger */
	if ( ch->in_room->area->nplayer > 0 )
	{
	    mprog_random_trigger( ch );
	    if ( char_died(ch) )
		continue;
	    if ( ch->position < POS_STANDING )
	        continue;
	}

        /* MOBprogram hour trigger: do something for an hour */
        mprog_hour_trigger(ch);

	if ( char_died(ch) )
	  continue;

	rprog_hour_trigger(ch);
	if ( char_died(ch) )
	  continue;

	if ( ch->position < POS_STANDING )
	  continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->first_content
	&&   number_bits( 2 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = NULL;
	    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max
		&& !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_RUNNING)
	&&   !IS_SET(ch->act, ACT_SENTINEL)
	&&   !IS_SET(ch->act, ACT_PROTOTYPE)
	&& ( door = number_bits( 5 ) ) <= 9
	&& ( pexit = get_exit(ch->in_room, door) ) != NULL
	&&   pexit->to_room
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->to_room->area == ch->in_room->area ) )
	{
	    retcode = move_char( ch, pexit, 0 );
						/* If ch changes position due
						to it's or someother mob's
						movement via MOBProgs,
						continue - Kahn */
	    if ( char_died(ch) )
	      continue;
	    if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
	    ||    ch->position < POS_STANDING )
	        continue;
	}

	/* Flee */
	if ( ch->hit < ch->max_hit / 2
	&& ( door = number_bits( 4 ) ) <= 9
	&& ( pexit = get_exit(ch->in_room,door) ) != NULL
	&&   pexit->to_room
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )
	{
	    CHAR_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = ch->in_room->first_person;
		  rch;
		  rch  = rch->next_in_room )
	    {
		if ( is_fearing(ch, rch) )
		{
		    switch( number_bits(2) )
		    {
			case 0:
			  sprintf( buf, "Get away from me, %s!", rch->name );
			  break;
			case 1:
			  sprintf( buf, "Leave me be, %s!", rch->name );
			  break;
			case 2:
			  sprintf( buf, "%s is trying to kill me!  Help!", rch->name );
			  break;
			case 3:
			  sprintf( buf, "Someone save me from %s!", rch->name );
			  break;
		    }
		    do_yell( ch, buf );
		    found = TRUE;
		    break;
		}
	    }
	    if ( found )
		retcode = move_char( ch, pexit, 0 );
	}
    }

    return;
}

void update_taxes( void )
{
     PLANET_DATA *planet;
     CLAN_DATA *clan;

    for ( planet = first_planet; planet; planet = planet->next )
    {
        clan = planet->governed_by;
        if ( clan )
        {
            int sCount = 0;
            CLAN_DATA * subclan = NULL;

            if ( clan->first_subclan )
            {
            	for ( subclan = clan->first_subclan ; subclan ; subclan = subclan->next_subclan )
   		  			sCount++;

   	       		for ( subclan = clan->first_subclan ; subclan ; subclan = subclan->next_subclan )
   	       		{
					if (subclan->funds <= MAX_BANK) /* Overflow code --Funf */
   	       				subclan->funds += get_taxes(planet)/400/sCount;
            		save_clan (subclan);
           		}
				if (clan->funds <= MAX_BANK) /* Overflow code --Funf */
              		clan->funds += get_taxes(planet)/400;
              	save_clan (clan);
           	}
            else
            {
				if (clan->funds <= MAX_BANK) /* Overflow code --Funf */
               		clan->funds += get_taxes(planet)/200;
               	save_clan( clan );
            }
            save_planet( planet );
        }
    }

}


/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;
    sh_int AT_TEMP = AT_PLAIN;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun." );
        AT_TEMP = AT_YELLOW;
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The sun rises in the east." );
        AT_TEMP = AT_ORANGE;
	break;

    case 12:
        weather_info.sunlight = SUN_LIGHT;
        strcat( buf, "It's noon." );
        AT_TEMP = AT_YELLOW;
   	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The sun slowly disappears in the west." );
        AT_TEMP = AT_BLOOD;
  	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun." );
        AT_TEMP = AT_DGREY;
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 30 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 12 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character)
	    &&   d->character->in_room
	    &&   d->character->in_room->sector_type != SECT_UNDERWATER
	    &&   d->character->in_room->sector_type != SECT_OCEANFLOOR
	    &&   d->character->in_room->sector_type != SECT_UNDERGROUND )
		act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
	}
        buf[0] = '\0';
    }
    /*
     * Weather change.
     */
    if ( time_info.month >= 5 && time_info.month <= 10 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    AT_TEMP = AT_GREY;
    switch ( weather_info.sky )
    {
    default:
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy." );
	    weather_info.sky = SKY_CLOUDY;
            AT_TEMP = AT_GREY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain." );
	    weather_info.sky = SKY_RAINING;
            AT_TEMP = AT_BLUE;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear." );
	    weather_info.sky = SKY_CLOUDLESS;
            AT_TEMP = AT_WHITE;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky." );
	    weather_info.sky = SKY_LIGHTNING;
	    AT_TEMP = AT_YELLOW;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped." );
	    weather_info.sky = SKY_CLOUDY;
	    AT_TEMP = AT_WHITE;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped." );
	    weather_info.sky = SKY_RAINING;
            AT_TEMP = AT_GREY;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
	}
    }

    return;
}



/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_save;
    sh_int save_count = 0;
    int recov, chance, count;
    char buf[MAX_STRING_LENGTH];


    ch_save	= NULL;
    for ( ch = last_char; ch; ch = gch_prev )
    {
	if ( ch == first_char && ch->prev )
	{
	    bug( "char_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}
	gch_prev = ch->prev;
	set_cur_char( ch );
	if ( gch_prev && gch_prev->next != ch )
	{
	    bug( "char_update: ch->prev->next != ch", 0 );
	    return;
	}

        /*
	 *  Do a room_prog rand check right off the bat
	 *   if ch disappears (rprog might wax npc's), continue
	 */
	if(!IS_NPC(ch))
	    rprog_random_trigger( ch );

	if( char_died(ch) )
	    continue;

	if(IS_NPC(ch))
	    mprog_time_trigger(ch);

	if( char_died(ch) )
	    continue;

	rprog_time_trigger(ch);

	if( char_died(ch) )
	    continue;

	/*
	 * See if player should be auto-saved.
	 */
	if ( !IS_NPC(ch)
	&&    !NOT_AUTHED(ch)
	&&    current_time - ch->save_time > (sysdata.save_frequency*60) )
	    ch_save	= ch;
	else
	    ch_save	= NULL;

	if ( ch->position >= POS_INCAP )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);
		update_pos( ch );

	    if ( ch->mana < ch->max_mana || ch->skill_level[FORCE_ABILITY] == 1 )
		ch->mana += mana_gain(ch);
		update_pos( ch );

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
		update_pos( ch );
	}


	if ( ch->position == POS_STUNNED && !IS_NPC(ch) && ch->pcdata->condition[COND_BLEEDING] == 0 )
	    update_pos( ch );
	else
		if ( IS_NPC(ch) )
			update_pos( ch );


        if ( ch->pcdata )
              gain_addiction( ch );


	if ( !IS_NPC(ch) && ch->top_level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room )
		{
		    ch->in_room->light -= obj->count;
		    act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
		    if ( obj->serial == cur_obj )
		      global_objcode = rOBJ_EXPIRED;
		    extract_obj( obj );
		}
	    }

	    if ( ch->pcdata->condition[COND_DRUNK] > 8 )
		worsen_mental_state( ch, ch->pcdata->condition[COND_DRUNK]/8 );
	    if ( ch->pcdata->condition[COND_FULL] > 1 )
	    {
		switch( ch->position )
		{
		    case POS_SLEEPING:  better_mental_state( ch, 4 );	break;
		    case POS_RESTING:   better_mental_state( ch, 3 );	break;
		    case POS_SITTING:
		    case POS_MOUNTED:   better_mental_state( ch, 2 );	break;
		    case POS_STANDING:  better_mental_state( ch, 1 );	break;
		    case POS_FIGHTING:
			if ( number_bits(2) == 0 )
			    better_mental_state( ch, 1 );
			break;
		}
	    }
	    if ( ch->pcdata->condition[COND_THIRST] > 1 )
	    {
		switch( ch->position )
		{
		    case POS_SLEEPING:  better_mental_state( ch, 5 );	break;
		    case POS_RESTING:   better_mental_state( ch, 3 );	break;
		    case POS_SITTING:
		    case POS_MOUNTED:   better_mental_state( ch, 2 );	break;
		    case POS_STANDING:  better_mental_state( ch, 1 );	break;
		    case POS_FIGHTING:
			if ( number_bits(2) == 0 )
			    better_mental_state( ch, 1 );
			break;
		}
	    }
	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL,   0 );
	    if ( ch->in_room )
	      switch( ch->in_room->sector_type )
	      {
		default:
		    gain_condition( ch, COND_THIRST, 0 );  break;
		case SECT_DESERT:
		    gain_condition( ch, COND_THIRST, 0 );  break;
		case SECT_UNDERWATER:
		case SECT_OCEANFLOOR:
		    if ( number_bits(1) == 0 )
			gain_condition( ch, COND_THIRST, 0 );  break;
	      }

	}

	if ( !char_died(ch) )
	{
	    /*
	     * Careful with the damages here,
	     *   MUST NOT refer to ch after damage taken,
	     *   as it may be lethal damage (on NPC).
	     */
	    if ( IS_AFFECTED(ch, AFF_POISON) )
	    {
		act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
		act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
		ch->mental_state = URANGE( 20, ch->mental_state
				 + 4 , 100 );
		damage( ch, ch, 6, gsn_poison );
	    }
		/* Bleeding update code -Seven */
	if ( !IS_NPC(ch))
        {
	// Bug fix
	bool sleep = FALSE;
	    if( ch->position == POS_SLEEPING)
		sleep = TRUE;
            if ( ch->pcdata->condition[COND_BLEEDING] > 0 )
            {
				switch( ch->pcdata->condition[COND_BLEEDING] )
				{
					default:
					{
					act( AT_BLOOD, "You are bleeding profusely!", ch, NULL, NULL, TO_CHAR );
					act( AT_BLOOD, "$n is bleeding profusely.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   1:
					{
					act( AT_BLOOD, "Blood drips slowly from your wounds...", ch, NULL, NULL, TO_CHAR );
	    				act( AT_BLOOD, "Blood drips slowly from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   2:
					{
					act( AT_BLOOD, "Blood drips slowly from your wounds...", ch, NULL, NULL, TO_CHAR );
	    				act( AT_BLOOD, "Blood drips slowly from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   3:
					{
					act( AT_BLOOD, "Blood is dripping quickly from your wounds...", ch, NULL, NULL, TO_CHAR );
    					act( AT_BLOOD, "Blood is dripping quickly from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   4:
					{
					act( AT_BLOOD, "Blood is dripping quickly from your wounds...", ch, NULL, NULL, TO_CHAR );
    					act( AT_BLOOD, "Blood is dripping quickly from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   5:
					{
					act( AT_BLOOD, "Blood is dripping quickly from your wounds...", ch, NULL, NULL, TO_CHAR );
    					act( AT_BLOOD, "Blood is dripping quickly from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   6:
					{
					act( AT_BLOOD, "Blood runs freely from your wounds...", ch, NULL, NULL, TO_CHAR );
					act( AT_BLOOD, "Blood runs freely from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
					case   7:
					{
					act( AT_BLOOD, "Blood runs freely from your wounds...", ch, NULL, NULL, TO_CHAR );
					act( AT_BLOOD, "Blood runs freely from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					case   8:
					{
					act( AT_BLOOD, "Blood runs freely from your wounds...", ch, NULL, NULL, TO_CHAR );
    					act( AT_BLOOD, "Blood runs freely from $n's wounds.", ch, NULL, NULL, TO_ROOM );
					}
					break;
				}
			/* 4 in 5 chance to slow bleeding on a tick
			 * A work in progress...
			 */
				if ( number_range( 1, 5 ) > 1 )
				{
					if( IS_IMMORTAL(ch))
					{
						ch->pcdata->condition[COND_BLEEDING] = 0;
					}
					if ( ch->pcdata->condition[COND_BLEEDING] != 1
						&& (ch->position == POS_SITTING
						|| ch->position == POS_SLEEPING
						|| ch->position == POS_RESTING) )
					{
						gain_condition( ch, COND_BLEEDING,  -2 );
						if ( ch->pcdata->condition[COND_BLEEDING] == 0 )
							send_to_char("Your bleeding stops.\n\r",ch);
						else
							send_to_char("Your bleeding slows...\n\r",ch);
					}
					else
					{
						gain_condition( ch, COND_BLEEDING,  -1 );
						if ( ch->pcdata->condition[COND_BLEEDING] == 0 )
							send_to_char("Your bleeding stops.\n\r",ch);
						else
							send_to_char("Your bleeding slows a little...\n\r",ch);
					}
				}
				make_blood( ch );
				worsen_mental_state( ch, ch->pcdata->condition[COND_BLEEDING] );
			//	gain_condition( ch, COND_THIRST,  -1 );
		/* Damage moved to the end of the code to prevent characters who
		 * have bled to death from being addressed after the fact (crash bug fix)
		 */
				if ( ch->pcdata->condition[COND_BLEEDING] == 0 )
					damage(ch, ch, (ch->max_hit * 1/100), TYPE_UNDEFINED);
				else
					damage(ch, ch, (ch->pcdata->condition[COND_BLEEDING] * (ch->max_hit * 1/100)), TYPE_UNDEFINED);
			}

		if( sleep == TRUE)
			ch->position = POS_SLEEPING;
		}
		/* End of bleeding update code */
	    // Removed else that was here
	    if ( ch->position == POS_INCAP )
		damage( ch, ch, 1, TYPE_UNDEFINED );
	    else
	    if ( ch->position == POS_MORTAL )
		damage( ch, ch, 4, TYPE_UNDEFINED );
	    if ( char_died(ch) )
		continue;
	    if ( ch->mental_state >= 30 )
		switch( (ch->mental_state+5) / 10 )
		{
		    case  3:
		    	send_to_char( "You feel feverish.\n\r", ch );
			act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  4:
		    	send_to_char( "You do not feel well at all.\n\r", ch );
			act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  5:
		    	send_to_char( "You need help!\n\r", ch );
			act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  6:
		    	send_to_char( "Seekest thou a cleric.\n\r", ch );
			act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  7:
		    	send_to_char( "You feel reality slipping away...\n\r", ch );
			act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  8:
		    	send_to_char( "You begin to understand... everything.\n\r", ch );
			act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  9:
		    	send_to_char( "You are ONE with the universe.\n\r", ch );
			act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case 10:
		    	send_to_char( "You feel the end is near.\n\r", ch );
			act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
		    	break;
		}
	    if ( ch->mental_state <= -30 )
		switch( (abs(ch->mental_state)+5) / 10 )
		{
		    case  10:
			if ( ch->position > POS_SLEEPING )
			{
			   if ( (ch->position == POS_STANDING
			   ||    ch->position < POS_FIGHTING)
			   &&    number_percent()+10 < abs(ch->mental_state) )
				do_sleep( ch, "" );
			   else
				send_to_char( "You're barely conscious.\n\r", ch );
			}
			break;
		    case   9:
			if ( ch->position > POS_SLEEPING )
			{
			   if ( (ch->position == POS_STANDING
			   ||    ch->position < POS_FIGHTING)
			   &&   (number_percent()+20) < abs(ch->mental_state) )
				do_sleep( ch, "" );
			   else
				send_to_char( "You can barely keep your eyes open.\n\r", ch );
			}
			break;
		    case   8:
			if ( ch->position > POS_SLEEPING )
			{
			   if ( ch->position < POS_SITTING
			   &&  (number_percent()+30) < abs(ch->mental_state) )
				do_sleep( ch, "" );
			   else
				send_to_char( "You're extremely drowsy.\n\r", ch );
			}
			break;
		    case   7:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel very unmotivated.\n\r", ch );
			break;
		    case   6:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel sedated.\n\r", ch );
			break;
		    case   5:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel sleepy.\n\r", ch );
			break;
		    case   4:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel tired.\n\r", ch );
			break;
		    case   3:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You could use a rest.\n\r", ch );
			break;
		}

	    if ( ch->backup_wait > 0 )
	    {
	       --ch->backup_wait;
	       if ( ch->backup_wait == 0 )
	          add_reinforcements( ch );
	    }

	if( !IS_NPC(ch) && ch->pcdata->unsilence_date == 0 &&
	    IS_SET(ch->act, PLR_SILENCE) )
	{
                send_to_char("&RYour silence sentence is now over.\n\r", ch);
                REMOVE_BIT( ch->act, PLR_SILENCE );
                save_char_obj( ch );
	}

	//Un-silence code
	if(!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->unsilence_date > 0 &&
	   ch->pcdata->unsilence_date <= current_time )
	{
		send_to_char("&RYour silence sentence is now over.\n\r", ch);
		STRFREE( ch->pcdata->silenceed_by);
		REMOVE_BIT( ch->act, PLR_SILENCE );
		ch->pcdata->silenceed_by = NULL;
		ch->pcdata->unsilence_date = 0;
		save_char_obj( ch );
	}

	if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->release_date > 0 &&
	     ch->pcdata->release_date <= current_time )
	{
	   ROOM_INDEX_DATA *location;

	   if(ch->pcdata->bountyrelease <= 0)
		   location = get_room_index(400);
	   else
	  	 location = get_room_index( ROOM_PRISON );

	   MOBtrigger = FALSE;
	   char_from_room(ch);
	   char_to_room( ch, location );
	   send_to_char( "You have been released from your hell sentance.\n\r" ,ch);
           do_look( ch, "auto" );
	   STRFREE( ch->pcdata->helled_by);
	   ch->pcdata->helled_by = NULL;
	   ch->pcdata->release_date = 0;
	   save_char_obj( ch );
	}

	// Medic Skill Recover - Gatz
	if(!IS_NPC(ch) && ch->pcdata->learned[gsn_recover])
	{
		if(ch->hit < 900)
		{
			ch->hit += number_range(20,30);
			update_pos( ch );
			learn_from_success( ch, gsn_recover);
		}
	}

        // Players can no longer get hungry or thirsty - Gatz
	if( !IS_NPC(ch) )
	{
		ch->pcdata->condition[COND_THIRST] = 48;
		ch->pcdata->condition[COND_FULL] = 48;
	}
	if(!IS_NPC(ch) && ch->pcdata->hackcount > 0 && number_range(0,4) > 2)
	{
		ch->pcdata->hackcount--;
	}
	BOUNTY_DATA *bounty;
	bounty = get_bounty(ch->name);

	if(bounty && bounty->amount < 0)
	{
		// This fixed a bug and sets the bounty to 50k;
		bounty->amount = 50000;
	}

	if(!IS_NPC(ch) && bounty && ch->pcdata->weaponl == 0)
	{
        ch_printf( ch, "&BISSP tells you 'Hey! Weapon Licenses aren't for criminals!");
		ch->pcdata->weaponl = -1;
	}
	if(!IS_NPC(ch) && !bounty && ch->pcdata->weaponl > 1)
	{
		ch->pcdata->weaponl--;
		if(ch->pcdata->weaponl == 1)
		{
			ch_printf( ch, "&BISSP tells you 'You may now re-apply for a Weapon License.");
		}
	}
        if(!IS_NPC(ch) && !bounty && ch->pcdata->weaponl == -1)
	{
		ch_printf(ch, "&BISSP tells you 'After reviewing our files, we will allow you get a new license after a mild time delay.");
		ch->pcdata->weaponl = number_range( ch->top_level/10, ch->top_level/2);
	}
	if(!IS_NPC(ch))
	{
		if(bounty  && !IS_SET(bounty->flags, FAKE_BOUNTY) )
		{
			ch->pcdata->arresttime++;
			if(ch->pcdata->arresttime > ch->pcdata->maxarresttime)
				ch->pcdata->maxarresttime = ch->pcdata->arresttime;
		}
		if(!bounty && ch->pcdata->arresttime > 0)
			ch->pcdata->arresttime = 0;
	}
	if(!IS_NPC(ch) && bounty && !IS_SET(bounty->flags, FAKE_BOUNTY) && ch->pcdata && ch->pcdata->clan
	   && (!str_cmp(ch->pcdata->clan->name, "GLM") || !str_cmp(ch->pcdata->clan->name, "RBH")))
	{
		ch_printf( ch,
			"&BISSP tells you 'Hey! %s is not for criminals!",
				ch->pcdata->clan->name);
	       if ( ch->speaking & LANG_CLAN )
      			ch->speaking = LANG_COMMON;
    		REMOVE_BIT( ch->speaks, LANG_CLAN );
  		--ch->pcdata->clan->members;
    		if ( !str_cmp( ch->name, ch->pcdata->clan->number1 ) )
	        {
			STRFREE( ch->pcdata->clan->number1 );
			ch->pcdata->clan->number1 = STRALLOC( "" );
    		}
   	        if ( !str_cmp( ch->name, ch->pcdata->clan->number2 ) )
    	       	{
			STRFREE( ch->pcdata->clan->number2 );
			ch->pcdata->clan->number2 = STRALLOC( "" );
  		}


    		ch->pcdata->clan = NULL;
    		STRFREE(ch->pcdata->clan_name);
    		ch->pcdata->clan_name = STRALLOC( "" );
	}
	// Prison-Jail Code - Gatz
	if( !IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->bountyrelease > 0 && ch->desc && ch->pcdata->release_date <= 0)
	{
		ch->pcdata->bountyrelease--;
		if(ch->pcdata->bountyrelease <= 0)
		{
			send_to_char("A large man comes in and tells you your sentence is up.\n\r",ch);
			act( AT_PLAIN, "A large man comes in and tells $n $s sentence is up", ch, ch, ch, TO_ROOM);
			char_from_room( ch );
			char_to_room( ch , get_room_index( 3543 ) );
			do_look( ch, "");
			act( AT_PLAIN, "$n gets thrown onto the street by a large officer.", ch, ch, ch, TO_ROOM);
		}
	}
	// Fixes a slight bug
        if(!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->bountyrelease <= 0 && IS_SET(ch->in_room->area->flags, AFLAG_JAIL) )
        {
                send_to_char("A large man comes in and tells you your sentence is up.\n\r",ch);
                act( AT_PLAIN, "A large man comes in and tells $n $s sentence is up", ch, ch, ch, TO_ROOM);
                char_from_room( ch );
                char_to_room( ch , get_room_index( 3543 ) );
                do_look( ch, "");
                act( AT_PLAIN, "$n gets thrown onto the street by a large officer.", ch, ch, ch, TO_ROOM);
	}

	// This just updates older players who weren't around when AFK Messages were created! - Gatz
 	if(!IS_NPC(ch) && (ch->pcdata->afkmessage == NULL || strlen(ch->pcdata->afkmessage) < 5))
		ch->pcdata->afkmessage = STRALLOC("");
	// Stat Code
	// Don't hurt the newbies!
	// Don't decay the LD people!
	// Lower Gradually - Locke
	// Slightly Optimized - Funf
	// Getting rid of stat drop per Spike - Funf 2007-10-23
	if (!IS_NPC(ch))
	switch (ch->pcdata->atrophy)
	{
		case 0: default: break;
		case 1: //STR
			if(ch->perm_str <= 10) break;
			ch->strtrain -= (ch->perm_str - 10) * 2;
			if(ch->perm_str==11&&ch->strtrain<200||
			   ch->perm_str==12&&ch->strtrain<300||
			   ch->perm_str==13&&ch->strtrain<500||
			   ch->perm_str==14&&ch->strtrain<750||
			   ch->perm_str==15&&ch->strtrain<1000||
			   ch->perm_str==16&&ch->strtrain<4500||
			   ch->perm_str==17&&ch->strtrain<9000||
			   ch->perm_str==18&&ch->strtrain<12000)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str--;
				if(ch->carry_weight > can_carry_w(ch))
				{
					act(AT_RED, "You body shakes slightly and you feel like you can't hold onto all your items!",
					ch, NULL, NULL, TO_CHAR);
					act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
					do_drop(ch, "all");
				}
			}
			break;
		case 2: //WIS
			if(ch->perm_wis <= 10) break;
			ch->wistrain -= (ch->perm_wis - 10) * 2;
			if(ch->perm_wis==11&&ch->wistrain<100||
			   ch->perm_wis==12&&ch->wistrain<200||
			   ch->perm_wis==13&&ch->wistrain<350||
			   ch->perm_wis==14&&ch->wistrain<550||
			   ch->perm_wis==15&&ch->wistrain<750||
			   ch->perm_wis==16&&ch->wistrain<1000||
			   ch->perm_wis==17&&ch->wistrain<4300||
			   ch->perm_wis==18&&ch->wistrain<6600)
			{
				send_to_char("&RThe world seems simpler somehow...\n\r", ch);
				ch->perm_wis--;
			}
			break;
		case 3: //INT
			if(ch->perm_int <= 10) break;
			ch->inttrain -= (ch->perm_int - 10) * 2;
			if(ch->perm_int==11&&ch->inttrain<170||
			   ch->perm_int==12&&ch->inttrain<250||
			   ch->perm_int==13&&ch->inttrain<400||
			   ch->perm_int==14&&ch->inttrain<550||
			   ch->perm_int==15&&ch->inttrain<700||
			   ch->perm_int==16&&ch->inttrain<1750||
			   ch->perm_int==17&&ch->inttrain<3100||
			   ch->perm_int==18&&ch->inttrain<5500)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int--;
			}
			break;
		case 4: //CON
			if(ch->perm_con <= 10) break;
			ch->contrain -= (ch->perm_con - 10) * 2;
			if(ch->perm_con==11&&ch->contrain<200||
			   ch->perm_con==12&&ch->contrain<500||
			   ch->perm_con==13&&ch->contrain<750||
			   ch->perm_con==14&&ch->contrain<1000||
			   ch->perm_con==15&&ch->contrain<1500||
			   ch->perm_con==16&&ch->contrain<3100||
			   ch->perm_con==17&&ch->contrain<6600||
			   ch->perm_con==18&&ch->contrain<9500)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con--;
			}
			break;
		case 5: //DEX
			if(ch->perm_dex <= 10) break;
			ch->dextrain -= (ch->perm_dex - 10) * 2;
			if(ch->perm_dex==11&&ch->dextrain<450||
			   ch->perm_dex==12&&ch->dextrain<800||
			   ch->perm_dex==13&&ch->dextrain<1500||
			   ch->perm_dex==14&&ch->dextrain<2250||
			   ch->perm_dex==15&&ch->dextrain<5600||
			   ch->perm_dex==16&&ch->dextrain<10500||
			   ch->perm_dex==17&&ch->dextrain<13100||
			   ch->perm_dex==18&&ch->dextrain<16300)
			{
				send_to_char("&RYou feel your joints stiffen.\n\r", ch);
				ch->perm_dex--;
				if(ch->carry_weight > can_carry_w(ch))
				{
					act(AT_RED, "You body shakes slightly and you feel like you can't hold onto all your items!",
					ch, NULL, NULL, TO_CHAR);
					act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
					do_drop(ch, "all");
				}
			}
			break;
		case 6: //CHA
			if(ch->perm_cha <= 10) break;
			ch->chatrain -= (ch->perm_cha - 10) * 2;
			if(ch->perm_cha==11&&ch->chatrain<200||
			   ch->perm_cha==12&&ch->chatrain<400||
			   ch->perm_cha==13&&ch->chatrain<600||
			   ch->perm_cha==14&&ch->chatrain<900||
			   ch->perm_cha==15&&ch->chatrain<2250||
			   ch->perm_cha==16&&ch->chatrain<4700||
			   ch->perm_cha==17&&ch->chatrain<7100||
			   ch->perm_cha==18&&ch->chatrain<12500)
			{
				send_to_char("&RYou feel a bit more introverted.\n\r", ch);
				ch->perm_cha--;
			}
			break;
	}
	/*
	if(get_age(ch) > 6 && !IS_NPC(ch) && ch->desc)
	{
	if(ch->perm_str > 10 && !IS_IMMORTAL(ch) && number_range(1,3) >= 2)
	{
		if(ch->perm_str==18&&ch->strtrain<12025||
		    ch->perm_str==17&&ch->strtrain<9025||
		    ch->perm_str==16&&ch->strtrain<4525||
		    ch->perm_str==15&&ch->strtrain<1025||
	    	     ch->perm_str==14&&ch->strtrain<775||
		     ch->perm_str==13&&ch->strtrain<525||
		     ch->perm_str==12&&ch->strtrain<325||
		     ch->perm_str==11&&ch->strtrain<225)
			send_to_char("&RYour body feels like you need some strength training.\n\r", ch);

		chance = number_range(0,1);
		if(ch->strtrain>=3)
			ch->strtrain-=3;
		else
			ch->strtrain=0;
		if(chance==0)
		{
			//send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
			if(ch->perm_str==18&&ch->strtrain<12000)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==17&&ch->strtrain<9000)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==16&&ch->strtrain<4500)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==15&&ch->strtrain<1000)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==14&&ch->strtrain<750)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==13&&ch->strtrain<500)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==12&&ch->strtrain<300)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			} else
			if(ch->perm_str==11&&ch->strtrain<200)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str-=1;
			}
			if(ch->carry_weight > can_carry_w(ch))
			{
				act(AT_RED, "You body shakes slightly and you feel like you can't hold onto all your items!",
				ch, NULL, NULL, TO_CHAR);
				act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
				do_drop(ch, "all");
			}
		}*/
/*		if(ch->strtrain >= 2)
			ch->strtrain -= 2;
		else
			ch->strtrain = 0;
		if(ch->strtrain == 0)
		{
			chance = number_range(0,1);
			if(chance == 0)
			{
				send_to_char("&RYou feel your body weaken slightly.\n\r", ch);
				ch->perm_str -= 1;
				ch->strtrain += 60;
				if(ch->carry_weight > can_carry_w(ch))
				{
					act(AT_RED, "You body shakes slightly and you feel like you can't hold onto all your items!",
						ch, NULL, NULL, TO_CHAR);
					act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
					do_drop(ch, "all");
				}
			}
		}*//*
	}
	if(ch->perm_int > 10 && !IS_IMMORTAL(ch) && number_range(1,3) >= 2)
        {
		if(ch->perm_int==18&&ch->inttrain<5525||
		    ch->perm_int==17&&ch->inttrain<3125||
		    ch->perm_int==16&&ch->inttrain<1775||
		    ch->perm_int==15&&ch->inttrain<725||
	    	     ch->perm_int==14&&ch->inttrain<575||
		     ch->perm_int==13&&ch->inttrain<425||
		     ch->perm_int==12&&ch->inttrain<275||
		     ch->perm_int==11&&ch->inttrain<195)
                	send_to_char("&RYour mind feels weakened and needs to learn more.\n\r", ch);
		chance = number_range(0,1);
                if(ch->inttrain >= 3)
                        ch->inttrain -= 3;
                else
	                ch->inttrain = 0;
		if(count==0)
		{
			//send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
         		if(ch->perm_int==18&&ch->inttrain<5500)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==17&&ch->inttrain<3100)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==16&&ch->inttrain<1750)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==15&&ch->inttrain<700)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==14&&ch->inttrain<550)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==13&&ch->inttrain<400)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==12&&ch->inttrain<250)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			} else
			if(ch->perm_int==11&&ch->inttrain<170)
			{
				send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
				ch->perm_int-=1;
			}
		}*/
/*                if(ch->inttrain == 0)
                {
                        chance = number_range(0,1);
                        if(chance == 0)
                        {
                                send_to_char("&RYou feel like you forgot something, but can't remember.\n\r", ch);
                                ch->perm_int -= 1;
				ch->inttrain += 60;
                        }
                }*//*
        }

        if(ch->perm_con > 10 && !IS_IMMORTAL(ch) && number_range(1,3) >= 2)
        {
		if(ch->perm_con==18&&ch->contrain<9525||
		    ch->perm_con==17&&ch->contrain<6625||
		    ch->perm_con==16&&ch->contrain<3125||
		    ch->perm_con==15&&ch->contrain<1525||
	    	     ch->perm_con==14&&ch->contrain<1025||
		     ch->perm_con==13&&ch->contrain<775||
		     ch->perm_con==12&&ch->contrain<525||
		     ch->perm_con==11&&ch->contrain<225)
                	send_to_char("&RYour body feels like it could use conditoning.\n\r", ch);
		chance = number_range(0,1);
                if(ch->contrain >= 3)
                        ch->contrain -= 3;
                else
	                ch->contrain = 0;
		if(count==0)
		{
//			send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
         		if(ch->perm_con==18&&ch->contrain<9500)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			}  else
			if(ch->perm_con==17&&ch->contrain<6600)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			}  else
			if(ch->perm_con==16&&ch->contrain<3100)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			}  else
			if(ch->perm_con==15&&ch->contrain<1500)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			}  else
			if(ch->perm_con==14&&ch->contrain<1000)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			}  else
			if(ch->perm_con==13&&ch->contrain<750)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			} else
			if(ch->perm_con==12&&ch->contrain<500)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			} else
			if(ch->perm_con==11&&ch->contrain<200)
			{
				send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
				ch->perm_con-=1;
			}
		}*/
/*                if(ch->contrain >= 2)
                        ch->contrain -= 2;
                else
                        ch->contrain = 0;
                if(ch->contrain == 0)
                {
                        chance = number_range(0,1);
                        if(chance == 0)
                        {
                                send_to_char("&RYou body feels softer and more easily damaged.\n\r", ch);
                                ch->perm_con -= 1;
				ch->contrain += 60;
                        }
                }*//*
        }

        if(ch->perm_cha > 10 && !IS_IMMORTAL(ch) && number_range(1,3) >= 2)
        {
		if(ch->perm_cha==18&&ch->chatrain<12525||
		    ch->perm_cha==17&&ch->chatrain<7125||
		    ch->perm_cha==16&&ch->chatrain<4725||
		    ch->perm_cha==15&&ch->chatrain<2275||
	    	     ch->perm_cha==14&&ch->chatrain<925||
		     ch->perm_cha==13&&ch->chatrain<625||
		     ch->perm_cha==12&&ch->chatrain<425||
		     ch->perm_cha==11&&ch->chatrain<225)
               		send_to_char("&RYou feel like your character needs improvement\n\r", ch);
		chance = number_range(0,1);
                if(ch->chatrain >= 3)
                        ch->chatrain -= 3;
                else
	                ch->chatrain = 0;
		if(count==0)
		{
			//send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
         		if(ch->perm_cha==18&&ch->chatrain<12500)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==17&&ch->chatrain<7100)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==16&&ch->chatrain<4700)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==15&&ch->chatrain<2250)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==14&&ch->chatrain<900)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==13&&ch->chatrain<600)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==12&&ch->chatrain<400)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			} else
			if(ch->perm_cha==11&&ch->chatrain<200)
			{
				send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
				ch->perm_cha-=1;
			}
		}*/
/*                if(ch->chatrain >= 2)
                        ch->chatrain -= 2;
                else
                        ch->chatrain = 0;
                if(ch->chatrain == 0)
                {
                        chance = number_range(0,1);
                        if(chance == 0)
                        {
                                send_to_char("&RYou feel your popularity and charisma decrease.\n\r", ch);
                                ch->perm_cha -= 1;
                                ch->chatrain += 60;
                        }
                }*//*
        }
        if(ch->perm_dex > 10 && !IS_IMMORTAL(ch) && number_range(1,3) >= 2)
        {
		if(ch->perm_dex==18&&ch->dextrain<16325||
		    ch->perm_dex==17&&ch->dextrain<13125||
		    ch->perm_dex==16&&ch->dextrain<10525||
		    ch->perm_dex==15&&ch->dextrain<5025||
	    	     ch->perm_dex==14&&ch->dextrain<2275||
		     ch->perm_dex==13&&ch->dextrain<1525||
		     ch->perm_dex==12&&ch->dextrain<825||
		     ch->perm_dex==11&&ch->dextrain<425)
                	send_to_char("&RYou feel your body getting less nimble and getting sore.\n\r", ch);
		chance = number_range(0,1);
                if(ch->dextrain >= 3)
                        ch->dextrain -= 3;
                else
	                ch->dextrain = 0;
		if(count==0)
		{
			//send_to_char("&RYou feel your body get less dexterous\n\r", ch);
         		if(ch->perm_dex==18&&ch->dextrain<16300)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==17&&ch->dextrain<13100)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==16&&ch->dextrain<10500)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==15&&ch->dextrain<5600)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==14&&ch->dextrain<2250)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==13&&ch->dextrain<1500)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==12&&ch->dextrain<800)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			} else
			if(ch->perm_dex==11&&ch->dextrain<450)
			{
				send_to_char("&RYou feel your body get less dexterous\n\r", ch);
				ch->perm_dex-=1;
			}
			if(ch->carry_weight > can_carry_w(ch))
	                {
				act(AT_RED, "You body shakes slightly and you feel like you can't hold 	onto all your items!", ch, NULL, NULL, TO_CHAR);
				act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
                        	do_drop(ch, "all");
                	}
		}*/
/*                if(ch->dextrain >= 1)
                        ch->dextrain -= 1;
                else
                        ch->dextrain = 0;
                if(ch->dextrain == 0)
                {
                        chance = number_range(0,1);
                        if(chance == 0)
                        {
                                send_to_char("&RYou feel your body get less dexterous\n\r", ch);
                                ch->perm_dex -= 1;
                                ch->dextrain += 60;
				if(ch->carry_weight > can_carry_w(ch))
                                {
                                        act(AT_RED, "You body shakes slightly and you feel like you can't hold onto all your items!",
                                                ch, NULL, NULL, TO_CHAR);
                                        act(AT_RED, "$n's body shakes slightly and $e doesn't look well.", ch,NULL, NULL, TO_ROOM);
                                        do_drop(ch, "all");
                                }
                        }
                }*//*
        }

	}//End stat code -Locke*/

	sh_int statcount = 0, oldstr, olddex, oldint, oldcon, oldcha, oldstatcount;
/*
	if(ch->perm_str > 9)
		statcount += (ch->perm_str - 10);
	if(ch->perm_int > 9)
		statcount += (ch->perm_int - 10);
	if(ch->perm_dex > 9)
		statcount += (ch->perm_dex - 10);
	if(ch->perm_con > 9)
		statcount += (ch->perm_con - 10);
	if(ch->perm_cha > 9)
		statcount += (ch->perm_cha - 10);
*/
	statcount = 0;
	statcount += ch->perm_str;
	statcount += ch->perm_int;
	statcount += ch->perm_dex;
	statcount += ch->perm_con;
	statcount += ch->perm_cha;

	oldstr = ch->perm_str;
	olddex = ch->perm_dex;
	oldint = ch->perm_int;
	oldcon = ch->perm_con;
	// Not really needed because it is the last one, but just in case
	oldcha = ch->perm_cha;
	oldstatcount = statcount;

	if(statcount < 75)
	{
	if(ch->perm_str < 18)
	{
		if(ch->perm_str == 10)
		{
			if(ch->strtrain >= 200)
			{
			send_to_char("&RYou feel your body grow stronger.\n\r", ch);
			ch->perm_str += 1;
			}
		}
                if(ch->perm_str == 11)
                {
                        if(ch->strtrain >= 300)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
                if(ch->perm_str == 12)
                {
                        if(ch->strtrain >= 500)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
                if(ch->perm_str == 13)
                {
                        if(ch->strtrain >= 750)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
                if(ch->perm_str == 14)
                {
                        if(ch->strtrain >= 1000)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
                if(ch->perm_str == 15)
                {
                        if(ch->strtrain >= 4500)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
                if(ch->perm_str == 16)
                {
                        if(ch->strtrain >= 9000)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
                if(ch->perm_str == 17)
                {
                        if(ch->strtrain >= 12000)
                        {
                        send_to_char("&RYou feel your body grow stronger.\n\r", ch);
                        ch->perm_str += 1;
                        }
                }
	}
	}

	if(ch->perm_str > oldstr)
		statcount++;

        if(ch->perm_wis < 18)
        {
                if(ch->perm_wis == 10)
                {
                        if(ch->wistrain >= 100)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 11)
                {
                        if(ch->wistrain >= 200)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 12)
                {
                        if(ch->wistrain >= 350)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 13)
                {
                        if(ch->wistrain >= 550)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 14)
                {
                        if(ch->wistrain >= 750)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 15)
                {
                        if(ch->wistrain >= 1000)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 16)
                {
                        if(ch->wistrain >= 4300)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }
                if(ch->perm_wis == 17)
                {
                        if(ch->wistrain >= 6600)
                        {
                        send_to_char("&RYou feel your wisdom grow slightly.\n\r", ch);
                        ch->perm_wis += 1;
                        }
                }

        }
	if(statcount < 75)
	{
        if(ch->perm_int < 18)
        {
                if(ch->perm_int == 10)
                {
                        if(ch->inttrain >= 170)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 11)
                {
                        if(ch->inttrain >= 250)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 12)
                {
                        if(ch->inttrain >= 400)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 13)
                {
                        if(ch->inttrain >= 550)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 14)
                {
                        if(ch->inttrain >= 700)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 15)
                {
                        if(ch->inttrain >= 1750)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 16)
                {
                        if(ch->inttrain >= 3100)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }
                if(ch->perm_int == 17)
                {
                        if(ch->inttrain >= 5500)
                        {
                        send_to_char("&RYour intelligence expands slightly.\n\r", ch);
                        ch->perm_int += 1;
                        }
                }

        }
	}

	if(ch->perm_int > oldint)
		statcount++;

	if(statcount < 75)
	{
	if(ch->perm_con < 18)
        {
                if(ch->perm_con == 10)
                {
                        if(ch->contrain >= 200)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
                if(ch->perm_con == 11)
                {
                        if(ch->contrain >= 500)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
                if(ch->perm_con == 12)
                {
                        if(ch->contrain >= 750)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
                if(ch->perm_con == 13)
                {
                        if(ch->contrain >= 1000)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
                if(ch->perm_con == 14)
                {
                        if(ch->contrain >= 1500)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
                if(ch->perm_con == 15)
                {
                        if(ch->contrain >= 3100)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
                if(ch->perm_con == 16)
                {
                        if(ch->contrain >= 6600)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
		if(ch->perm_con == 17)
                {
                        if(ch->contrain >= 9500)
                        {
                        send_to_char("&RYour condition increases slightly.\n\r", ch);
                        ch->perm_con += 1;
                        }
                }
        }
	}

	if(ch->perm_con > oldcon)
		statcount++;

	if(statcount < 75)
	{
        if(ch->perm_dex < 18)
        {
                if(ch->perm_dex == 10)
                {
                        if(ch->dextrain >= 450)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 11)
                {
                        if(ch->dextrain >= 800)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 12)
                {
                        if(ch->dextrain >= 1500)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 13)
                {
                        if(ch->dextrain >= 2250)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 14)
                {
                        if(ch->dextrain >= 5600)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 15)
                {
                        if(ch->dextrain >= 10500)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 16)
                {
                        if(ch->dextrain >= 13100)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }
                }
                if(ch->perm_dex == 17)
                {
                        if(ch->dextrain >= 16300)
                        {
                        send_to_char("&RYou feel your body become more flexible.\n\r", ch);
                        ch->perm_dex += 1;
                        }

                }
        }
	}

	if(ch->perm_dex > olddex)
		statcount++;

	if(statcount < 75)
	{
        if(ch->perm_cha < 18)
        {
                if(ch->perm_cha == 10)
                {
                        if(ch->chatrain >= 200)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 11)
                {
                        if(ch->chatrain >= 400)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 12)
                {
                        if(ch->chatrain >= 600)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 13)
                {
                        if(ch->chatrain >= 900)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 14)
                {
                        if(ch->chatrain >= 2250)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 15)
                {
                        if(ch->chatrain >= 4700)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 16)
                {
                        if(ch->chatrain >= 7100)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }
                if(ch->perm_cha == 17)
                {
                        if(ch->chatrain >= 12500)
                        {
                        send_to_char("&RYou feel more charasmatic.\n\r", ch);
                        ch->perm_cha += 1;
                        }
                }

        }
	}
	// Just tells the player why they can't gain any more.
	if(oldstatcount < statcount && statcount == 70)
	{
		send_to_char("You feel your body shudder slightly as it can no longer take any more training.\r\n", ch);
	}
	// Fix a bug with negitive carry weight.
	if(ch->carry_weight < 0)
		ch->carry_weight = 0;

	if(ch->position == POS_STANDING && !IS_NPC(ch))
	{
	    // You okay their buddy? Realistic Damage things - Gatz
	    if(ch->hit < 300 && ch->hit > 201)
	    {
		if(ch->alignment > 300)
		{
			act(AT_RED, "$n grabs $s side and winces in pain!\n\r", ch, NULL, NULL, TO_ROOM);
			act(AT_RED, "You grab your side and wince in pain!\n\r", ch, NULL, NULL, TO_CHAR);
		}
		else
		{
			act(AT_RED, "$n snarls slightly as $e grabs his side in pain!\r\n", ch, NULL, NULL, TO_ROOM);
			act(AT_RED, "You snarl slightly as you grab your side in pain!\r\n", ch, NULL, NULL, TO_CHAR);
		}
	    }
	    if(ch->hit < 200 && ch->hit > 101)
	    {
		if(ch->alignment > 300)
		{
			act(AT_RED, "$n braces $sself up, barely standing!\n\r", ch, NULL, NULL, TO_ROOM);
			act(AT_RED, "You almost fall, but brace yourself up!\n\r", ch, NULL, NULL, TO_CHAR);
		}
		else
		{
			act(AT_RED, "$n stumbles slightly, barely able to stand.\r\n", ch, NULL, NULL, TO_ROOM);
			act(AT_RED, "You stumble slightly, barely able to stand.\r\n", ch, NULL, NULL, TO_CHAR);
		}
	    }
	    if(ch-> hit < 100 && ch->hit > 0)
	    {
		if(ch->alignment > 300)
		{
			act(AT_RED, "$n violently coughs up some blood!\n\r", ch, NULL, NULL, TO_ROOM);
			act(AT_RED, "You violently cough up some blood!\n\r", ch, NULL, NULL, TO_CHAR);
		}
		else
		{
			act(AT_RED, "$n grabs $s chest and vomits up some blood!\r\n", ch, NULL, NULL, TO_ROOM);
			act(AT_RED, "You grab your chest and vomit up some blood!\r\n", ch, NULL, NULL, TO_CHAR);
		}
	    }
	}
            // Can't see some channels - Gatz
	    if (!NOT_AUTHED(ch) && !IS_IMMORTAL(ch) && !SET_BIT(ch->deaf, CHANNEL_NEWBIE))
		SET_BIT(ch->deaf,CHANNEL_NEWBIE);

	    if (get_age(ch) < 1000 && !IS_IMMORTAL(ch) && !SET_BIT(ch->deaf,CHANNEL_AVTALK))
		SET_BIT(ch->deaf, CHANNEL_AVTALK);

	    // Gradually lose Adrenaline - Gatz
	    if (!IS_NPC(ch) && ch->adren > 0 && ch->desc)
	    {
		recov = number_range(1,2);
		if(ch->adren < recov)
			ch->adren = 0;
		else
			ch->adren -= recov;
		if(ch->adren == 0)
			send_to_char( "&RYour adrenaline subsides.\n\r",ch);
	    }


	    if ( !IS_NPC (ch) )
	    {
	     	if ( ++ch->timer > 15 && !ch->desc )
         	{
            	if ( !ch->in_room )
		{
		     	char_from_room( ch );
			char_to_room( ch , get_room_index( ROOM_VNUM_ALTAR ) );
		}
                ch->position = POS_RESTING;
                ch->hit = UMAX ( 1 , ch->hit );
		ch->adren = 0;
                save_char_obj( ch );
        	do_quit( ch, "" );
            }
	    else
	     	if ( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO ) &&  ++save_count < 10 )	/* save max of 10 per tick */
				save_char_obj( ch );
		}
	}

    }
    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
    OBJ_DATA *obj;
    sh_int AT_TEMP;

    for ( obj = last_object; obj; obj = gobj_prev )
    {
	CHAR_DATA *rch;
	char *message;

	if ( obj == first_object && obj->prev )
	{
	    bug( "obj_update: first_object->prev != NULL... fixed", 0 );
	    obj->prev = NULL;
	}
	gobj_prev = obj->prev;
	if ( gobj_prev && gobj_prev->next != obj )
	{
	    bug( "obj_update: obj->prev->next != obj", 0 );
	    return;
	}
	set_cur_obj( obj );
	if ( obj->carried_by )
	  oprog_random_trigger( obj );
	else
	if( obj->in_room && obj->in_room->area->nplayer > 0 )
	  oprog_random_trigger( obj );

        if( obj_extracted(obj) )
	  continue;

        if ( obj->item_type == ITEM_WEAPON && obj->carried_by  &&
             ( obj->wear_loc == WEAR_WIELD || obj->wear_loc == WEAR_DUAL_WIELD ) &&
             obj->value[3] != WEAPON_RIFLE && obj->value[4] > 0 &&
              obj->value[3] != WEAPON_PISTOL &&  obj->value[3] != WEAPON_HEAVY_WEAPON)
        {
           obj->value[4]--;
        }

	if ( obj->item_type == ITEM_PIPE )
	{
	    if ( IS_SET( obj->value[3], PIPE_LIT ) )
	    {
		if ( --obj->value[1] <= 0 )
		{
		  obj->value[1] = 0;
		  REMOVE_BIT( obj->value[3], PIPE_LIT );
		}
		else
		if ( IS_SET( obj->value[3], PIPE_HOT ) )
		  REMOVE_BIT( obj->value[3], PIPE_HOT );
		else
		{
		  if ( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
		  {
		    REMOVE_BIT( obj->value[3], PIPE_LIT );
		    REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
		  }
		  else
		    SET_BIT( obj->value[3], PIPE_GOINGOUT );
		}
		if ( !IS_SET( obj->value[3], PIPE_LIT ) )
		  SET_BIT( obj->value[3], PIPE_FULLOFASH );
	    }
	    else
	      REMOVE_BIT( obj->value[3], PIPE_HOT );
	}


/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

        if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC
        || obj->item_type == ITEM_DROID_CORPSE )
        {
          sh_int timerfrac = UMAX(1, obj->timer - 1);
          if ( obj->item_type == ITEM_CORPSE_PC )
	    timerfrac = (int)(obj->timer / 8 + 1);

	  if ( obj->timer > 0 && obj->value[2] > timerfrac )
	  {
            char buf[MAX_STRING_LENGTH];
            char name[MAX_STRING_LENGTH];
            char *bufptr;
            bufptr = one_argument( obj->short_descr, name );
            bufptr = one_argument( bufptr, name );
            bufptr = one_argument( bufptr, name );

	    separate_obj(obj);
            obj->value[2] = timerfrac;
            if ( obj->item_type == ITEM_DROID_CORPSE )
              sprintf( buf, d_corpse_descs[ UMIN( timerfrac - 1, 4 ) ],
                          bufptr );
            else
              sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ],
                          capitalize( bufptr ) );

            STRFREE( obj->description );
            obj->description = STRALLOC( buf );
          }
        }

	/* don't let inventory decay */
	if ( IS_OBJ_STAT(obj, ITEM_INVENTORY) )
	  continue;

        if ( obj->timer > 0 && obj->timer < 5 && obj->item_type == ITEM_ARMOR )
        {
          if ( obj->carried_by )
	  {
	    act( AT_TEMP, "$p is almost dead." , obj->carried_by, obj, NULL, TO_CHAR );
	  }
        }

	if ( ( obj->timer <= 0 || --obj->timer > 0 ) )
	  continue;


	/* if we get this far, object's timer has expired. */

         AT_TEMP = AT_PLAIN;
	 switch ( obj->item_type )
	 {
	 default:
	   message = "$p has depleted itself.";
           AT_TEMP = AT_PLAIN;
	   break;

	 case ITEM_GRENADE:
	  explode( obj );
	  return;
	break;


	 case ITEM_PORTAL:
	   message = "$p winks out of existence.";
           remove_portal(obj);
	   obj->item_type = ITEM_TRASH;		/* so extract_obj	 */
           AT_TEMP = AT_MAGIC;			/* doesn't remove_portal */
	   break;
	 case ITEM_FOUNTAIN:
	   message = "$p dries up.";
           AT_TEMP = AT_BLUE;
	   break;
	 case ITEM_CORPSE_NPC:
	   message = "$p decays into dust and blows away.";
           AT_TEMP = AT_OBJECT;
	   break;
	 case ITEM_DROID_CORPSE:
	   message = "$p rusts away into oblivion.";
           AT_TEMP = AT_OBJECT;
	   break;
	 case ITEM_CORPSE_PC:
	   message = "$p decays into dust and is blown away...";
           AT_TEMP = AT_MAGIC;
	   break;
	 case ITEM_FOOD:
	   message = "$p is devoured by a swarm of maggots.";
           AT_TEMP = AT_HUNGRY;
	   break;
         case ITEM_BLOOD:
           message = "$p slowly seeps into the ground.";
           AT_TEMP = AT_BLOOD;
           break;
         case ITEM_BLOODSTAIN:
           message = "$p dries up into flakes and blows away.";
           AT_TEMP = AT_BLOOD;
	   break;
         case ITEM_SCRAPS:
           message = "$p crumbles and decays into nothing.";
           AT_TEMP = AT_OBJECT;
	   break;
	 case ITEM_FIRE:
	   if (obj->in_room)
	     --obj->in_room->light;
	   message = "$p burns out.";
	   AT_TEMP = AT_FIRE;
	 }

	  if ( obj->carried_by )
	  {
	    act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
	  }
	  else if ( obj->in_room
	  &&      ( rch = obj->in_room->first_person ) != NULL
	  &&	!IS_OBJ_STAT( obj, ITEM_BURRIED ) )
	  {
	    act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
            act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
	  }

	if ( obj->serial == cur_obj )
	  global_objcode = rOBJ_EXPIRED;
	extract_obj( obj );
    }
    return;
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
    CHAR_DATA *ch, *ch_next;
    EXIT_DATA *pexit;
    static int cnt = 0;
    int door, retcode;

    cnt = (cnt+1) % 2;

    for ( ch = first_char; ch; ch = ch_next )
    {
	set_cur_char(ch);
	ch_next = ch->next;
	will_fall(ch, 0);

	if ( char_died( ch ) )
	  continue;

	if ( IS_NPC( ch ) )
	{
	    if ( cnt != 0 )
		continue;

	    /* running mobs	-Thoric */
	    if ( IS_SET(ch->act, ACT_RUNNING) )
	    {
		if ( !IS_SET( ch->act, ACT_SENTINEL )
		&&   !ch->fighting && ch->hunting )
		{
		    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
		    hunt_victim( ch );
		    continue;
		}

		if ( ch->spec_fun )
		{
		    if ( (*ch->spec_fun) ( ch ) )
			continue;
		    if ( char_died(ch) )
			continue;
		}
                if ( ch->spec_2 )
		{
		    if ( (*ch->spec_2) ( ch ) )
			continue;
		    if ( char_died(ch) )
			continue;
		}

		if ( !IS_SET(ch->act, ACT_SENTINEL)
		&&   !IS_SET(ch->act, ACT_PROTOTYPE)
		&& ( door = number_bits( 4 ) ) <= 9
		&& ( pexit = get_exit(ch->in_room, door) ) != NULL
		&&   pexit->to_room
		&&   !IS_SET(pexit->exit_info, EX_CLOSED)
		&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
		&& ( !IS_SET(ch->act, ACT_STAY_AREA)
		||   pexit->to_room->area == ch->in_room->area ) )
		{
		    retcode = move_char( ch, pexit, 0 );
		    if ( char_died(ch) )
			continue;
		    if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
		    ||    ch->position < POS_STANDING )
			continue;
		}
	    }
	    continue;
	}
	else
	{
	    if ( ch->mount
	    &&   ch->in_room != ch->mount->in_room )
	    {
		REMOVE_BIT( ch->mount->act, ACT_MOUNTED );
		ch->mount = NULL;
		ch->position = POS_STANDING;
		send_to_char( "No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r", ch );
	    }

	    if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
	    || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
	    {
		if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
		{
		    if ( get_trust(ch) < LEVEL_IMMORTAL )
		    {
			int dam;

			dam = number_range( ch->max_hit / 50 , ch->max_hit / 30 );
			dam = UMAX( 1, dam );
			if(  ch->hit <= 0 )
			    dam = -10;
			if ( number_bits(3) == 0 )
			  send_to_char( "You cough and choke as you try to breathe water!\n\r", ch );
			damage( ch, ch, dam, TYPE_UNDEFINED );
		    }
		}
	    }

	    if ( char_died( ch ) )
		continue;

	    if ( ch->in_room
	    && (( ch->in_room->sector_type == SECT_WATER_NOSWIM )
	    ||  ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
	    {
		if ( !IS_AFFECTED( ch, AFF_FLYING )
		&& !IS_AFFECTED( ch, AFF_FLOATING )
		&& !IS_AFFECTED( ch, AFF_AQUA_BREATH )
		&& !ch->mount )
		{
			if ( get_trust(ch) < LEVEL_IMMORTAL )
			{
			    int dam;

			    if ( ch->move > 0 )
				    ch->move--;
			    else
			    {
				dam = number_range( ch->max_hit / 50, ch->max_hit / 30 );
				dam = UMAX( 1, dam );
				if(  ch->hit <= 0 )
			           dam = -10;
				if ( number_bits(3) == 0 )
				   send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\n\r", ch );
				damage( ch, ch, dam, TYPE_UNDEFINED );
			    }
          	      }
		}
	    }

	}
    }
}


/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
void aggr_update( void )
{
    DESCRIPTOR_DATA *d, *dnext;
    CHAR_DATA *wch;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    struct act_prog_data *apdtmp;

#ifdef UNDEFD
  /*
   *  GRUNT!  To do
   *
   */
        if ( IS_NPC( wch ) && wch->mpactnum > 0
	    && wch->in_room->area->nplayer > 0 )
        {
            MPROG_ACT_LIST * tmp_act, *tmp2_act;
	    for ( tmp_act = wch->mpact; tmp_act;
		 tmp_act = tmp_act->next )
	    {
                 oprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
				      tmp_act->obj, tmp_act->vo, ACT_PROG );
                 DISPOSE( tmp_act->buf );
            }
	    for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
	    {
                 tmp2_act = tmp_act->next;
                 DISPOSE( tmp_act );
            }
            wch->mpactnum = 0;
            wch->mpact    = NULL;
        }
#endif

    /* check mobprog act queue */
    while ( (apdtmp = mob_act_list) != NULL )
    {
	wch = mob_act_list->vo;
	if ( !char_died(wch) && wch->mpactnum > 0 )
	{
	    MPROG_ACT_LIST * tmp_act;

	    while ( (tmp_act = wch->mpact) != NULL )
	    {
		if ( tmp_act->obj && obj_extracted(tmp_act->obj) )
		  tmp_act->obj = NULL;
		if ( tmp_act->ch && !char_died(tmp_act->ch) )
		  mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
					tmp_act->obj, tmp_act->vo, ACT_PROG );
		wch->mpact = tmp_act->next;
		DISPOSE(tmp_act->buf);
		DISPOSE(tmp_act);
	    }
	    wch->mpactnum = 0;
	    wch->mpact    = NULL;
        }
	mob_act_list = apdtmp->next;
	DISPOSE( apdtmp );
    }


    /*
     * Just check descriptors here for victims to aggressive mobs
     * We can check for linkdead victims to mobile_update	-Thoric
     */
    for ( d = first_descriptor; d; d = dnext )
    {
	dnext = d->next;
	if ( d->connected != CON_PLAYING || (wch=d->character) == NULL )
	   continue;

	if ( char_died(wch)
	||   IS_NPC(wch)
	||   wch->top_level >= LEVEL_IMMORTAL
	||  !wch->in_room )
	    continue;

	for ( ch = wch->in_room->first_person; ch; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   ch->fighting
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) )
	    ||   !can_see( ch, wch ) )
		continue;

	    if ( is_hating( ch, wch ) )
	    {
		found_prey( ch, wch );
		continue;
	    }

	    if ( !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||    IS_SET(ch->act, ACT_MOUNTED)
	    ||    IS_SET(ch->in_room->room_flags, ROOM_SAFE ) )
		continue;

	    victim = wch;

	    if ( !victim )
	    {
		bug( "Aggr_update: null victim.", count );
		continue;
	    }

            if ( get_timer(victim, TIMER_RECENTFIGHT) > 0 )
                continue;

	    if ( IS_NPC(ch) && IS_SET(ch->attacks, ATCK_BACKSTAB ) )
	    {
		OBJ_DATA *obj;

		if ( !ch->mount
    		&& (obj = get_eq_char( ch, WEAR_WIELD )) != NULL
    		&& obj->value[3] == 11
		&& !victim->fighting
		&& victim->hit >= victim->max_hit )
		{
		    WAIT_STATE( ch, skill_table[gsn_backstab]->beats );
		    if ( !IS_AWAKE(victim)
		    ||   number_percent( )+5 < ch->top_level )
		    {
			global_retcode = multi_hit( ch, victim, gsn_backstab );
			continue;
		    }
		    else
		    {
			global_retcode = damage( ch, victim, 0, gsn_backstab );
			continue;
		    }
		}
	    }
	    global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }

    return;
}

/* From interp.c */
bool check_social  args( ( CHAR_DATA *ch, char *command, char *argument ) );

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms( CHAR_DATA *ch )
{
    CHAR_DATA *rvch = NULL;
    CHAR_DATA *vch;
    sh_int drunk;
    sh_int position;

    if ( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )
	return;

    if ( number_percent() < 30 )
	return;

    drunk = ch->pcdata->condition[COND_DRUNK];
    position = ch->position;
    ch->position = POS_STANDING;

    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "burp", "" );
    else
    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "hiccup", "" );
    else
    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "drool", "" );
    else
    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "fart", "" );
    else
    if ( drunk > (10+(get_curr_con(ch)/5))
    &&   number_percent() < ( 2 * drunk / 18 ) )
    {
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	    if ( number_percent() < 10 )
		rvch = vch;
	check_social( ch, "puke", (rvch ? rvch->name : "") );
    }

    ch->position = position;
    return;
}

void halucinations( CHAR_DATA *ch )
{
    if ( ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0 )
    {
	char *t;

	switch( number_range( 1, UMIN(20, (ch->mental_state+5) / 5)) )
	{
	    default:
	    case  1: t = "You feel very restless... you can't sit still.\n\r";		break;
	    case  2: t = "You're tingling all over.\n\r";				break;
	    case  3: t = "Your skin is crawling.\n\r";					break;
	    case  4: t = "You suddenly feel that something is terribly wrong.\n\r";	break;
	    case  5: t = "Those damn little fairies keep laughing at you!\n\r";		break;
	    case  6: t = "You can hear your mother crying...\n\r";			break;
	    case  7: t = "Have you been here before, or not?  You're not sure...\n\r";	break;
	    case  8: t = "Painful childhood memories flash through your mind.\n\r";	break;
	    case  9: t = "You hear someone call your name in the distance...\n\r";	break;
	    case 10: t = "Your head is pulsating... you can't think straight.\n\r";	break;
	    case 11: t = "The ground... seems to be squirming...\n\r";			break;
	    case 12: t = "You're not quite sure what is real anymore.\n\r";		break;
	    case 13: t = "It's all a dream... or is it?\n\r";				break;
	    case 14: t = "They're coming to get you... coming to take you away...\n\r";	break;
	    case 15: t = "You begin to feel all powerful!\n\r";				break;
	    case 16: t = "You're light as air... the heavens are yours for the taking.\n\r";	break;
	    case 17: t = "Your whole life flashes by... and your future...\n\r";	break;
	    case 18: t = "You are everywhere and everything... you know all and are all!\n\r";	break;
	    case 19: t = "You feel immortal!\n\r";					break;
	    case 20: t = "Ahh... the power of a Supreme Entity... what to do...\n\r";	break;
	}
	send_to_char( t, ch );
    }
    return;
}

void tele_update( void )
{
    TELEPORT_DATA *tele, *tele_next;

    if ( !first_teleport )
      return;

    for ( tele = first_teleport; tele; tele = tele_next )
    {
	tele_next = tele->next;
	if ( --tele->timer <= 0 )
	{
	    if ( tele->room->first_person )
	    {
		  teleport( tele->room->first_person, tele->room->tele_vnum,
			TELE_TRANSALL );
	    }
	    UNLINK( tele, first_teleport, last_teleport, next, prev );
	    DISPOSE( tele );
	}
    }
}

#if FALSE
/*
 * Write all outstanding authorization requests to Log channel - Gorog
 */
void auth_update( void )
{
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  char log_buf [MAX_INPUT_LENGTH];
  bool first_time = TRUE;         /* so titles are only done once */

  for ( d = first_descriptor; d; d = d->next )
      {
      victim = d->character;
      if ( victim && IS_WAITING_FOR_AUTH(victim) )
         {
         if ( first_time )
            {
            first_time = FALSE;
            strcpy (log_buf, "Pending authorizations:" );
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1);
            }
         sprintf( log_buf, " %s@%s new %s", victim->name,
            victim->desc->host, race_table[victim->race].race_name);
         to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1);
         }
      }
}
#endif

void auth_update( void )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf [MAX_INPUT_LENGTH], log_buf [MAX_INPUT_LENGTH];
    bool found_hit = FALSE;         /* was at least one found? */

    strcpy (log_buf, "Pending authorizations:\n\r" );
    for ( d = first_descriptor; d; d = d->next )
    {
	if ( (victim = d->character) && IS_WAITING_FOR_AUTH(victim) )
	{
	    found_hit = TRUE;
	    sprintf( buf, " %s@%s new %s\n\r", victim->name,
		victim->desc->host, race_table[victim->race].race_name);
	    strcat (log_buf, buf);
	}
    }
    if (found_hit)
    {
	log_string( log_buf );
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1);
    }
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    static  int     pulse_taxes;
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_second;
    static  int	    pulse_fifteen;
    static  int     pulse_space;
    static  int     pulse_ship;
    static  int     pulse_recharge;
    static  int     pulse_bigshot;
    static  int	    pulse_support;
    static  int	    pulse_citizen;
    static  int	    pulse_police;
    static  int	    pulse_trade;
    struct timeval stime;
    struct timeval etime;
    CHAR_DATA *victim, *last;
    DESCRIPTOR_DATA *d;
    bool found;
    BOUNTY_DATA *bounty;
    int count = 0;
    bool afk = FALSE;

    //Don't reset this on copyover/restart...
    pulse_trade    = PULSE_TRADE;

    if ( timechar )
    {
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Starting update timer.\n\r", timechar );
      gettimeofday(&stime, NULL);
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( );
        quest_update     ( );
    }
    // Auto Big Shot - Gatz
    if ( --pulse_bigshot <= 0)
    {
		pulse_bigshot = PULSE_BIGSHOT;
        found = FALSE;
        for ( d = first_descriptor; d; d = d->next )
            if ( (d->connected == CON_PLAYING || d->connected == CON_EDITING )
				&& ( victim = d->character ) != NULL
				&&   !IS_NPC(victim)
				&&  !IS_IMMORTAL(victim)
				&&  !IS_SET(victim->act,PLR_WIZINVIS)
				&&  (bounty = get_bounty( victim->name )) > 0
			    &&   bounty)
            {
				victim = d->character;
                found = TRUE;
				count++;
            }

		if( count > 1)
		{
            for ( d = first_descriptor; d; d = d->next )
                if ( (d->connected == CON_PLAYING || d->connected == CON_EDITING )
                && ( victim = d->character ) != NULL
                &&   !IS_NPC(victim)
				&&  !IS_IMMORTAL(victim)
				&&  victim != last
                &&  !IS_SET(victim->act,PLR_WIZINVIS)
                &&  (bounty = get_bounty( victim->name )) > 0
				&&  bounty)
            {
                victim = d->character;
                if(number_range(0,2) == 1)
                    break;
	        }
		}
		else if( count == 1)
		{
            for ( d = first_descriptor; d; d = d->next )
                if ( (d->connected == CON_PLAYING || d->connected == CON_EDITING )
					&& ( victim = d->character ) != NULL
					&&   !IS_NPC(victim)
					&&  !IS_IMMORTAL(victim)
					&&  !IS_SET(victim->act,PLR_WIZINVIS)
					&&  (bounty = get_bounty( victim->name )) > 0
					&&  bounty)
                {
					victim = d->character;
					break;
				}
		}
		afk = FALSE;
		if(found && !IS_NPC(victim)  && victim->desc && IS_SET( victim->act, PLR_AFK))
			afk = TRUE;
		if(found && bounty && !IS_SET(bounty->flags, HIDDEN_BOUNTY) )
		{
			last = victim;
			bigshot( victim, victim, "report");
			if(afk)
				SET_BIT(victim->act, PLR_AFK);
		}
    }

    if ( --pulse_taxes     <= 0 )
    {
	pulse_taxes	= PULSE_TAXES ;
	update_taxes	( );
    bank_update     ( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update  ( );
    }

    if ( --pulse_support <= 0)
    {
	pulse_support	= PULSE_SUPPORT;
	support_update( );
    }
    if( --pulse_citizen <= 0)
    {
	pulse_citizen = PULSE_CITIZEN;
//	citizen_generate( );
    }
    if( --pulse_police <= 0)
    {
	pulse_police = PULSE_POLICE;
	generate_police();
    }
    if ( --pulse_space   <= 0 )
    {
       pulse_space    = PULSE_SPACE;
       update_space  ( );
	/* The MUD no larger uses Busses so there is no need for the MUD to waste it's time on Busses - Gatz
       update_bus ( );
	*/
    }
    if ( --pulse_trade   <= 0 )
    {
       pulse_trade    = PULSE_TRADE;
       update_planettrade();
    }
    if ( --pulse_recharge <= 0 )
    {
         pulse_recharge = PULSE_SPACE/3;
         recharge_ships ( );
    }

    if ( --pulse_ship   <= 0 )
    {
       pulse_ship  = PULSE_SPACE/10;
       move_ships  ( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	pulse_point     = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );

        auth_update     ( );			/* Gorog */
	weather_update	( );
	char_update	( );
	obj_update	( );
	clear_vrooms	( );			/* remove virtual rooms */
    }

    if ( --pulse_second   <= 0 )
    {
	pulse_second	= PULSE_PER_SECOND;
	char_check( );
 	/*reboot_check( "" ); Disabled to check if its lagging a lot - Scryn*/
 	/* Much faster version enabled by Altrag..
 	   although I dunno how it could lag too much, it was just a bunch
 	   of comparisons.. */
 	reboot_check(0);
    }

    if ( auction->item && --auction->pulse <= 0 )
    {
	auction->pulse = PULSE_AUCTION;
	auction_update( );
    }

    mpsleep_update(); /*Check for sleeping mud progs -rkb */
    tele_update( );
    aggr_update( );
    obj_act_update ( );
    room_act_update( );
    clean_obj_queue();		/* dispose of extracted objects */
    clean_char_queue();		/* dispose of dead mobs/quitting chars */
    if ( timechar )
    {
      gettimeofday(&etime, NULL);
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Update timing complete.\n\r", timechar );
      subtract_times(&etime, &stime);
      ch_printf( timechar, "Timing took %d.%06d seconds.\n\r",
          etime.tv_sec, etime.tv_usec );
      timechar = NULL;
    }
    tail_chain( );
    return;
}


void remove_portal( OBJ_DATA *portal )
{
    ROOM_INDEX_DATA *fromRoom, *toRoom;
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    bool found;

    if ( !portal )
    {
	bug( "remove_portal: portal is NULL", 0 );
	return;
    }

    fromRoom = portal->in_room;
    found = FALSE;
    if ( !fromRoom )
    {
	bug( "remove_portal: portal->in_room is NULL", 0 );
	return;
    }

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
	if ( IS_SET( pexit->exit_info, EX_PORTAL ) )
	{
	    found = TRUE;
	    break;
	}

    if ( !found )
    {
	bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
	return;
    }

    if ( pexit->vdir != DIR_PORTAL )
	bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

    if ( ( toRoom = pexit->to_room ) == NULL )
      bug( "remove_portal: toRoom is NULL", 0 );

    extract_exit( fromRoom, pexit );
    /* rendunancy */
    /* send a message to fromRoom */
    /* ch = fromRoom->first_person; */
    /* if(ch!=NULL) */
    /* act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM ); */

    /* send a message to toRoom */
    if ( toRoom && (ch = toRoom->first_person) != NULL )
      act( AT_PLAIN, "A magical portal above winks from existence.", ch, NULL, NULL, TO_ROOM );

    /* remove the portal obj: looks better to let update_obj do this */
    /* extract_obj(portal);  */

    return;
}

void reboot_check( time_t reset )
{
  static char *tmsg[] =
  { "SYSTEM: Reboot in 10 seconds.",
    "SYSTEM: Reboot in 30 seconds.",
    "SYSTEM: Reboot in 1 minute.",
    "SYSTEM: Reboot in 2 minutes.",
    "SYSTEM: Reboot in 3 minutes.",
    "SYSTEM: Reboot in 4 minutes.",
    "SYSTEM: Reboot in 5 minutes.",
    "SYSTEM: Reboot in 10 minutes.",
  };
  static const int times[] = { 10, 30, 60, 120, 180, 240, 300, 600 };
  static const int timesize =
      UMIN(sizeof(times)/sizeof(*times), sizeof(tmsg)/sizeof(*tmsg));
  char buf[MAX_STRING_LENGTH];
  static int trun;
  static bool init;

  if ( !init || reset >= current_time )
  {
    for ( trun = timesize-1; trun >= 0; trun-- )
      if ( reset >= current_time+times[trun] )
        break;
    init = TRUE;
    return;
  }

  if ( (current_time % 1800) == 0 )
  {
    sprintf(buf, "%.24s: %d players", ctime(&current_time), num_descriptors);
    append_to_file(USAGE_FILE, buf);
  }

  if ( new_boot_time_t - boot_time < 60*60*18 &&
      !set_boot_time->manual )
    return;

  if ( new_boot_time_t <= current_time )
  {
    CHAR_DATA *vch;
    extern bool mud_down;

    if ( auction->item )
    {
      sprintf(buf, "Sale of %s has been stopped by mud.",
          auction->item->short_descr);
      talk_auction(buf);
      obj_to_char(auction->item, auction->seller);
      auction->item = NULL;
      if ( auction->buyer && auction->buyer != auction->seller )
      {
        auction->buyer->gold += auction->bet;
        send_to_char("Your money has been returned.\n\r", auction->buyer);
      }
    }
    echo_to_all(AT_YELLOW, "You are forced from these realms by a strong "
        "presence\n\ras life here is reconstructed.", ECHOTAR_ALL);

    for ( vch = first_char; vch; vch = vch->next )
      if ( !IS_NPC(vch) )
        save_char_obj(vch);
    mud_down = TRUE;
    return;
  }

  if ( trun != -1 && new_boot_time_t - current_time <= times[trun] )
  {
    echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
    if ( trun <= 5 )
      sysdata.DENY_NEW_PLAYERS = TRUE;
    --trun;
    return;
  }
  return;
}

#if 0
void reboot_check( char *arg )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    /*struct tm *timestruct;
    int timecheck;*/
    CHAR_DATA *vch;

    /*Bools to show which pre-boot echoes we've done. */
    static bool thirty  = FALSE;
    static bool fifteen = FALSE;
    static bool ten     = FALSE;
    static bool five    = FALSE;
    static bool four    = FALSE;
    static bool three   = FALSE;
    static bool two     = FALSE;
    static bool one     = FALSE;

    /* This function can be called by do_setboot when the reboot time
       is being manually set to reset all the bools. */
    if ( !str_cmp( arg, "reset" ) )
    {
      thirty  = FALSE;
      fifteen = FALSE;
      ten     = FALSE;
      five    = FALSE;
      four    = FALSE;
      three   = FALSE;
      two     = FALSE;
      one     = FALSE;
      return;
    }

    /* If the mud has been up less than 18 hours and the boot time
       wasn't set manually, forget it. */
/* Usage monitor */

if ((current_time % 1800) == 0)
{
  sprintf(buf, "%s: %d players", ctime(&current_time), num_descriptors);
  append_to_file(USAGE_FILE, buf);
}

/* Change by Scryn - if mud has not been up 18 hours at boot time - still
 * allow for warnings even if not up 18 hours
 */
    if ( new_boot_time_t - boot_time < 60*60*18
         && set_boot_time->manual == 0 )
    {
      return;
    }
/*
    timestruct = localtime( &current_time);

    if ( timestruct->tm_hour == set_boot_time->hour
         && timestruct->tm_min  == set_boot_time->min )*/
    if ( new_boot_time_t <= current_time )
    {
       /* Return auction item to seller */
       if (auction->item != NULL)
       {
        sprintf (buf,"Sale of %s has been stopped by mud.",
                 auction->item->short_descr);
        talk_auction (buf);
        obj_to_char (auction->item, auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL && auction->seller != auction->buyer) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Your money has been returned.\n\r",auction->buyer);
        }
       }

       sprintf( buf, "You are forced from these realms by a strong magical presence" );
       echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
       sprintf( buf, "as life here is reconstructed." );
       echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );

       /* Save all characters before booting. */
       for ( vch = first_char; vch; vch = vch->next )
       {
         if ( !IS_NPC( vch ) )
           save_char_obj( vch );
       }
       mud_down = TRUE;
    }

  /* How many minutes to the scheduled boot? */
/*  timecheck = ( set_boot_time->hour * 60 + set_boot_time->min )
              - ( timestruct->tm_hour * 60 + timestruct->tm_min );

  if ( timecheck > 30  || timecheck < 0 ) return;

  if ( timecheck <= 1 ) */
  if ( new_boot_time_t - current_time <= 60 )
  {
    if ( one == FALSE )
    {
	sprintf( buf, "You feel the ground shake as the end comes near!" );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	one = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;
  }

/*  if ( timecheck == 2 )*/
  if ( new_boot_time_t - current_time <= 120 )
  {
    if ( two == FALSE )
    {
	sprintf( buf, "Lightning crackles in the sky above!" );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	two = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;
  }

/*  if ( timecheck == 3 )*/
  if (new_boot_time_t - current_time <= 180 )
  {
    if ( three == FALSE )
    {
	sprintf( buf, "Crashes of thunder sound across the land!" );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	three = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;
  }

/*  if ( timecheck == 4 )*/
  if( new_boot_time_t - current_time <= 240 )
  {
    if ( four == FALSE )
    {
	sprintf( buf, "The sky has suddenly turned midnight black." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	four = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;
  }

/*  if ( timecheck == 5 )*/
  if( new_boot_time_t - current_time <= 300 )
  {
    if ( five == FALSE )
    {
	sprintf( buf, "You notice the life forms around you slowly dwindling away." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	five = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;
  }

/*  if ( timecheck == 10 )*/
  if( new_boot_time_t - current_time <= 600 )
  {
    if ( ten == FALSE )
    {
	sprintf( buf, "The seas across the realm have turned frigid." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	ten = TRUE;
    }
    return;
  }

/*  if ( timecheck == 15 )*/
  if( new_boot_time_t - current_time <= 900 )
  {
    if ( fifteen == FALSE )
    {
	sprintf( buf, "The aura of magic which once surrounded the realms seems slightly unstable." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	fifteen = TRUE;
    }
    return;
  }

/*  if ( timecheck == 30 )*/
  if( new_boot_time_t - current_time <= 1800 )
  {
    if ( thirty == FALSE )
    {
	sprintf( buf, "You sense a change in the magical forces surrounding you." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	thirty = TRUE;
    }
    return;
  }

  return;
}
#endif

/* the auction update*/

void auction_update (void)
{
    long long int tax, pay;
    char buf[MAX_STRING_LENGTH];

    switch (++auction->going) /* increase the going state */
    {
	case 1 : /* going once */
	case 2 : /* going twice */
	    if (auction->bet > auction->starting)
		sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
			((auction->going == 1) ? "once" : "twice"), auction->bet);
	    else
		sprintf (buf, "%s: going %s (bid not received yet).", auction->item->short_descr,
			((auction->going == 1) ? "once" : "twice"));
	    talk_auction (buf);
	    break;

	case 3 : /* SOLD! */
	    if (!auction->buyer && auction->bet)
	    {
		bug( "Auction code reached SOLD, with NULL buyer, but %lld gold bid", auction->bet );
		auction->bet = 0;
	    }
	    if (auction->bet > 0 && auction->buyer != auction->seller)
	    {
		sprintf (buf, "%s sold to %s for %d.",
			auction->item->short_descr,
			IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
			auction->bet);
		talk_auction(buf);

		act(AT_ACTION, "The auctioneer materializes before you, and hands you $p.",
			auction->buyer, auction->item, NULL, TO_CHAR);
		act(AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.",
			auction->buyer, auction->item, NULL, TO_ROOM);

		if ( (auction->buyer->carry_weight
		+     get_obj_weight( auction->item ))
		>     can_carry_w( auction->buyer ) )
		{
		    act( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR );
    		    act( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item, NULL, TO_ROOM );
		    obj_to_room( auction->item, auction->buyer->in_room );
		}
		else
		    obj_to_char( auction->item, auction->buyer );
	        pay = (long long int)auction->bet * 0.9;
		tax = (long long int)auction->bet * 0.1;
		boost_economy( auction->seller->in_room->area, tax );
                auction->seller->gold += pay; /* give him the money, tax 10 % */
		sprintf(buf, "The auctioneer pays you %lld gold, charging an auction fee of %lld.\n\r", pay, tax);
		send_to_char(buf, auction->seller);
                auction->item = NULL; /* reset item */
		if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
		{
		    save_char_obj( auction->buyer );
		    save_char_obj( auction->seller );
		}
            }
            else /* not sold */
            {
                sprintf (buf, "No bids received for %s - object has been removed from auction\n\r.",auction->item->short_descr);
		talk_auction(buf);
                act (AT_ACTION, "The auctioneer appears before you to return $p to you.",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act (AT_ACTION, "The auctioneer appears before $n to return $p to $m.",
                      auction->seller,auction->item,NULL,TO_ROOM);
		if ( (auction->seller->carry_weight
		+     get_obj_weight( auction->item ))
		>     can_carry_w( auction->seller ) )
		{
		    act( AT_PLAIN, "You drop $p as it is just too much to carry"
			" with everything else you're carrying.", auction->seller,
			auction->item, NULL, TO_CHAR );
		    act( AT_PLAIN, "$n drops $p as it is too much extra weight"
			" for $m with everything else.", auction->seller,
			auction->item, NULL, TO_ROOM );
		    obj_to_room( auction->item, auction->seller->in_room );
		}
		else
		    obj_to_char (auction->item,auction->seller);
		tax = (int)auction->item->cost * 0.05;
		boost_economy( auction->seller->in_room->area, tax );
		sprintf(buf, "The auctioneer charges you an auction fee of %d.\n\r", tax );
		send_to_char(buf, auction->seller);
		if ((auction->seller->gold - tax) < 0)
		  auction->seller->gold = 0;
		else
		  auction->seller->gold -= tax;
		if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
		    save_char_obj( auction->seller );
	    } /* else */
	    auction->item = NULL; /* clear auction */
    } /* switch */
} /* func */

void subtract_times(struct timeval *etime, struct timeval *stime)
{
  etime->tv_sec -= stime->tv_sec;
  etime->tv_usec -= stime->tv_usec;
  while ( etime->tv_usec < 0 )
  {
    etime->tv_usec += 1000000;
    etime->tv_sec--;
  }
  return;
}


