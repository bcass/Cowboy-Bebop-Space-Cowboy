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
* 		Commands for personal player settings/statictics	   *
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"

/*
 *  Locals
 */
char *tiny_affect_loc_name(int location);
int   strlen_color (char *argument);

void do_gold(CHAR_DATA * ch, char *argument)
{
   ch_printf( ch,  "&CYou have &Y%s &Cwulongs,\n\r", num_punct(ch->gold) );
   ch_printf( ch,  "&Cand you have &Y%s &Cwulongs in your bank account.\n\r", num_punct(ch->pcdata->bank) );
   return;
}


/*
 * New score command by Haus
 */
void do_score(CHAR_DATA * ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    AFFECT_DATA    *paf;
    int iLang, drug;
    sh_int statcount;

    if (IS_NPC(ch))
    {
	do_oldscore(ch, argument);
	return;
    }
    set_char_color(AT_SCORE, ch);

    ch_printf(ch, "\n\rScore for %s.\n\r", ch->name);
    if(ch->pcdata->isDisguised)
	ch_printf(ch, "Currently Disguised as: %s\r\n", ch->pcdata->disguise);
    set_char_color(AT_SCORE, ch);
    if ( get_trust( ch ) != ch->top_level )
	ch_printf( ch, "You are trusted at level %d.\r\n", get_trust( ch ) );

    send_to_char("&R----------------------------------------------------------------------------\n\r", ch);

    ch_printf(ch,   "&CAge&R: &W%3d years old &CLog In&R:&W  %s\r\n",
	ch->pcage, ctime(&(ch->logon)) );
    ch_printf( ch, "&CHours Played&R: &W%3d  &CSaved&R:  &W%s\r\n",
	(get_age(ch)),  ch->save_time ? ctime(&(ch->save_time)) : "no" );
	/*
    ch_printf(ch,   "&CHitroll&R: &W%-2.2d  &CDamroll&R: &W%-2.2d   &CArmor&R: &W%-4d        &CSaved&R:  &W%s\r",
		GET_HITROLL(ch), GET_DAMROLL(ch), GET_AC(ch),
		ch->save_time ? ctime(&(ch->save_time)) : "no\n" );
	*/
    ch_printf(ch, "&CAccuracy&R:&W ");
		switch(GET_HITROLL(ch))
		{
			case 0:
			case 1:
			       send_to_char("Very Low", ch);
			case 2:
			case 3:
			case 4:
				send_to_char("Low", ch);
			break;
			case 5:
			case 6:
				send_to_char("Medium-Low", ch);
			break;
			case 7:
			case 8:
			case 9:
				send_to_char("Medium", ch);
			break;
			case 10:
			case 11:
				send_to_char("Medium-High", ch);
			break;
			case 12:
			case 13:
			case 14:
			case 15:
				send_to_char("High", ch);
			break;
			default:
				send_to_char("Very High", ch);
			break;
		}
    ch_printf(ch, "    &CDeadliness&R:&W ");
		switch(GET_DAMROLL(ch))
		{
			case 0:
				send_to_char("Non-Existant", ch);
			break;
			case 1:
				send_to_char("Enough to Scare Flies", ch);
			break;
			case 2:
			case 3:
			case 4:
				send_to_char("Rough", ch);
			break;
			case 5:
			case 6:
			case 7:
				send_to_char("Very Rough", ch);
			break;
			case 8:
			case 9:
				send_to_char("Vicious", ch);
			break;
			case 10:
			case 11:
				send_to_char("Leathal", ch);
			break;
			case 12:
			case 13:
			case 14:
				send_to_char("Deadly", ch);
			break;
			case 15:
			case 16:
				send_to_char("Assassin", ch);
			break;
			default:
				send_to_char("God of Death", ch);
		}
    ch_printf(ch, "  &CArmor&R:&W ");
    	    if (GET_AC(ch) >= 101)
	ch_printf(ch, "The Sun would hurt you!");
    else if (GET_AC(ch) >= 80)
	ch_printf(ch, "Wind would damage you!");
    else if (GET_AC(ch) >= 55)
	ch_printf(ch, "Would have trouble deflecting small animals!");
    else if (GET_AC(ch) >= 40)
	ch_printf(ch, "Suitable for fighting handicap people.");
    else if (GET_AC(ch) >= 20)
	ch_printf(ch, "Suited for spectating fights");
    else if (GET_AC(ch) >= 10)
	ch_printf(ch, "Suited for training");
    else if (GET_AC(ch) >= 0)
	ch_printf(ch, "Decent");
    else if (GET_AC(ch) >= -10)
	ch_printf(ch, "Suited for light fighting");
    else if (GET_AC(ch) >= -20)
	ch_printf(ch, "Suited for long sparring sessions");
    else if (GET_AC(ch) >= -40)
	ch_printf(ch, "Suited for medium combat situations");
    else if (GET_AC(ch) >= -60)
	ch_printf(ch, "Good");
    else if (GET_AC(ch) >= -80)
	ch_printf(ch, "Suited for heavy fighting");
    else if (GET_AC(ch) >= -100)
	ch_printf(ch, "Suited for heavy combat situations");
    else if (GET_AC(ch) >= -200)
	ch_printf(ch, "Suitable for War");
    else if (GET_AC(ch) >= -250)
	ch_printf(ch, "Excellent Armor!");
    else if (GET_AC(ch) >= -300)
	ch_printf(ch, "God Like");
    else if (GET_AC(ch) >= -350)
	ch_printf( ch, "God of War");
    else
	ch_printf( ch, "Lord God");

    send_to_char("\n\r", ch);


    ch_printf(ch,   "&CAlign&R: &W%-5d &CWimpy&R: &W%-3d    &CQuest Points&R: &W%-2d   &CToggle&R: &W",
		ch->alignment, ch->wimpy, ch->pcdata->quest_curr );
    if(IS_SET(ch->pcdata->flags, PCFLAG_TOGGLE))
	ch_printf(ch,"Stun\n\r");
    else
	ch_printf(ch,"Kill\n\r");

    ch_printf(ch, "&CHit Points&R: &C%d &Wof &B%d   &CStamina&R: &C%d &Wof &B%d\n\r",
        ch->hit, ch->max_hit, ch->move, ch->max_move);

    ch_printf(ch, "&CStr&R:&B(&W%2d&B) &CDex&R:&B(&W%2d&B) &CCon&R:&B(&W%2d&B) &CInt&R:&B(&W%2d&B) &CWis&R:&B(&W%2d&B) &CCha&R:&B(&W%2d&B) &CLck&R:&B(&W??&B) \n\r",
	get_curr_str(ch), get_curr_dex(ch),get_curr_con(ch),get_curr_int(ch),get_curr_wis(ch),get_curr_cha(ch));



send_to_char("&R----------------------------------------------------------------------------\n\r", ch);



    ch_printf(ch, "&CWULONGS&R: &W%-10s   ", num_punct(ch->gold));
    ch_printf(ch,"&CBANK&R: &W%-11s   &CMkills&R: &W%-5.5d    &CPkills&R: &W%-5.5d\n\r",
	num_punct(ch->pcdata->bank), ch->pcdata->mkills, ch->pcdata->pkills);

    ch_printf(ch, "&CWeight&R: &W%5.5d &B(&Cmax &W%7.7d&B)    &CItems&R: &W%5.5d &B(&Cmax &W%5.5d&B)   &CPdeaths&R: &W%-5.5d\n\r",
	ch->carry_weight, can_carry_w(ch) , ch->carry_number, can_carry_n(ch), ch->pcdata->pdeaths);

    ch_printf(ch, "&CPager&R: &B(&W%c&B) &W%3d    &CAutoExit&B(&W%c&B)  &CAutoLoot&B(&W%c&B)   &CAutosac&B(&W%c&B)\n\r",
	IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
	ch->pcdata->pagerlen, IS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ',
	IS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ', IS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ');
   ch_printf( ch, "&CAFK Message&R:&W ");
   if(ch->pcdata->afkmessage == NULL)
	ch->pcdata->afkmessage = STRALLOC("");
   if(ch->pcdata->afkmessage[0] == '\0' || strlen(ch->pcdata->afkmessage) < 5)
	ch_printf( ch, "Default Message.\r\n");
   else
	ch_printf( ch, "%s\r\n", ch->pcdata->afkmessage);
    if(ch->pcdata->arrestcount > 0)
	ch_printf( ch, "&CTimes Arrested&R: &B(&W%d&B)\r\n", ch->pcdata->arrestcount);
    if(ch->pcdata->arresttime > 0)
	ch_printf( ch, "&CTicks with a Bounty&R: &B(&W%d&B)\r\n", ch->pcdata->arresttime);
    if(ch->pcdata->maxarresttime > 0)
	ch_printf( ch, "&CMaximum Time (In Ticks) with a Bounty&R: &B(&W%d&B)\r\n", ch->pcdata->maxarresttime);
    if(ch->pcdata->clantalk1 || ch->pcdata->clantalk2)
    {
	ch_printf( ch, "&CYou have the ability to use these clan radios&R:&W %s%s%s\r\n",
		(ch->pcdata->clantalk1)? ch->pcdata->clantalk1->name : "",
		(ch->pcdata->clantalk1 && ch->pcdata->clantalk2)? " and " : "",
		(ch->pcdata->clantalk2)? ch->pcdata->clantalk2->name : "");
    }
    statcount = 0;
    statcount += ch->perm_str;
    statcount += ch->perm_int;
    statcount += ch->perm_dex;
    statcount += ch->perm_con;
    statcount += ch->perm_cha;

    if(statcount >= 75)
	ch_printf(ch, "&BYour body is at the max of its ability to gain anything from Stat Training.\r\n");

    switch(ch->pcdata->atrophy)
    {
		case 1: ch_printf(ch, "&BYour body is slowly atrophying its strength.\r\n"); break;
		case 2: ch_printf(ch, "&BYour mind is slowly atrophying its wisdom.\r\n"); break;
		case 3: ch_printf(ch, "&BYour mind is slowly atrophying its intelligence.\r\n"); break;
		case 4: ch_printf(ch, "&BYour body is slowly atrophying its constitution.\r\n"); break;
		case 5: ch_printf(ch, "&BYour body is slowly atrophying its dexterity.\r\n"); break;
		case 6: ch_printf(ch, "&BYour mind is slowly atrophying its charisma.\r\n"); break;
		default: break;
	}


    switch (ch->position)
    {
	case POS_DEAD:
		sprintf(buf, "You are slowly decomposing. ");
		break;
	case POS_MORTAL:
		sprintf(buf, "You are mortally wounded. ");
		break;
	case POS_INCAP:
		sprintf(buf, "You are incapacitated. ");
		break;
	case POS_STUNNED:
		sprintf(buf, "You are stunned. ");
		break;
	case POS_SLEEPING:
		sprintf(buf, "You are sleeping. ");
		break;
	case POS_RESTING:
		sprintf(buf, "You are resting. ");
		break;
	case POS_STANDING:
		sprintf(buf, "You are standing. ");
		break;
	case POS_FIGHTING:
		sprintf(buf, "You are fighting. " );
		break;
	case POS_MOUNTED:
		sprintf(buf, "You are mounted. ");
		break;
        case POS_SITTING:
		sprintf(buf, "You are sitting. ");
		break;
    }

    send_to_char( buf, ch );

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	send_to_char("You are drunk.\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
	send_to_char("You are in danger of dehydrating.\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0)
	send_to_char("You are starving to death.\n\r", ch);
    if ( ch->position != POS_SLEEPING )
	switch( ch->mental_state / 10 )
	{
	    default:   send_to_char( "You're completely messed up!\n\r", ch );	break;
	    case -10:  send_to_char( "You're barely conscious.\n\r", ch );	break;
	    case  -9:  send_to_char( "You can barely keep your eyes open.\n\r", ch );	break;
	    case  -8:  send_to_char( "You're extremely drowsy.\n\r", ch );	break;
	    case  -7:  send_to_char( "You feel very unmotivated.\n\r", ch );	break;
	    case  -6:  send_to_char( "You feel sedated.\n\r", ch );		break;
	    case  -5:  send_to_char( "You feel sleepy.\n\r", ch );		break;
	    case  -4:  send_to_char( "You feel tired.\n\r", ch );		break;
	    case  -3:  send_to_char( "You could use a rest.\n\r", ch );		break;
	    case  -2:  send_to_char( "You feel a little under the weather.\n\r", ch );	break;
	    case  -1:  send_to_char( "You feel fine.\n\r", ch );		break;
	    case   0:  send_to_char( "You feel great.\n\r", ch );		break;
	    case   1:  send_to_char( "You feel energetic.\n\r", ch );	break;
	    case   2:  send_to_char( "Your mind is racing.\n\r", ch );	break;
	    case   3:  send_to_char( "You can't think straight.\n\r", ch );	break;
	    case   4:  send_to_char( "Your mind is going 100 miles an hour.\n\r", ch );	break;
	    case   5:  send_to_char( "You're high as a kite.\n\r", ch );	break;
	    case   6:  send_to_char( "Your mind and body are slipping apart.\n\r", ch );	break;
	    case   7:  send_to_char( "Reality is slipping away.\n\r", ch );	break;
	    case   8:  send_to_char( "You have no idea what is real, and what is not.\n\r", ch );	break;
	    case   9:  send_to_char( "You feel immortal.\n\r", ch );	break;
	    case  10:  send_to_char( "You are a Supreme Entity.\n\r", ch );	break;
	}
    else
    if ( ch->mental_state >45 )
	send_to_char( "Your sleep is filled with strange and vivid dreams.\n\r", ch );
    else
    if ( ch->mental_state >25 )
	send_to_char( "Your sleep is uneasy.\n\r", ch );
    else
    if ( ch->mental_state <-35 )
	send_to_char( "You are deep in a much needed sleep.\n\r", ch );
    else
    if ( ch->mental_state <-25 )
	send_to_char( "You are in deep slumber.\n\r", ch );
    if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	ch_printf( ch, "You are bestowed with the command(s): %s.\n\r",
		ch->pcdata->bestowments );

    if ( ch->pcdata->release_date > 0 )
	ch_printf( ch, "You are currently helled till: %24.24s \n\r",
		ctime(&ch->pcdata->release_date) );
    if ( ch->pcdata->unsilence_date > 0 )
        ch_printf( ch, "You are currently silenced till: %24.24s \n\r",
                ctime(&ch->pcdata->unsilence_date) );
    if ( ch->pcdata->bountyrelease > 0)
	ch_printf( ch, "You have about %d ticks left in Jail.\n\r", ch->pcdata->bountyrelease);
    if( ch->pcdata->weaponl == 1)
	ch_printf( ch, "You can now apply for a license again.\r\n");
    if ( ch->pcdata->weaponl > 1)
	ch_printf( ch, "You have about %d ticks left till you can get a new Weapon License.\r\n",ch->pcdata->weaponl );
    if ( ch->pcdata->weaponl == -1)
	ch_printf( ch, "Your Weapon License is suspended indefinitely. \r\n");

    if ( ch->pcdata->clan )
    {
      send_to_char( "&R----------------------------------------------------------------------------\n\r", ch);
      ch_printf(ch, "&CORGANIZATION&R: &W%-35s &CPkills&W/&CDeaths&R: &Y%3.3d&R/&C&W%3.3d",
		ch->pcdata->clan->name, ch->pcdata->clan->pkills, ch->pcdata->clan->pdeaths) ;
      send_to_char( "\n\r", ch );
      if(ch->pcdata->clan->clan_type != 1)
/*      {
      ch_printf(ch, "&CWars Won&R: &W%d	&CWars Lost&R: &W%d\r\n",
		ch->pcdata->clan->warwoncount, ch->pcdata->clan->warlostcount);
      }
*/
      if(ch->pcdata->clan->war1 || ch->pcdata->clan->war2)
      {
      ch_printf(ch, "&CAt War With&R:&W %s%s%s.",
		(ch->pcdata->clan->war1)? ch->pcdata->clan->war1->name : "",
		(ch->pcdata->clan->war1 && ch->pcdata->clan->war2)? " and " : "",
		(ch->pcdata->clan->war2)? ch->pcdata->clan->war2->name : "" );
	send_to_char("\n\r", ch);
      }
    }
    if (IS_IMMORTAL(ch))
    {
	send_to_char( "&R----------------------------------------------------------------------------\n\r", ch);

	ch_printf(ch, "&CIMMORTAL DATA&R:  &CWizinvis &B[&W%s&B]  &CWizlevel &B(&W%d&B)\n\r",
		IS_SET(ch->act, PLR_WIZINVIS) ? "&RX" : " ", ch->pcdata->wizinvis );

	ch_printf(ch, "&CBamfin&R:  &W%s\n\r", (ch->pcdata->bamfin[0] != '\0')
		? ch->pcdata->bamfin : "%s appears in a swirling mist.", ch->name);
	ch_printf(ch, "&CBamfout&R: &W%s\n\r", (ch->pcdata->bamfout[0] != '\0')
		? ch->pcdata->bamfout : "%s leaves in a swirling mist.", ch->name);


	/* Area Loaded info - Scryn 8/11*/
	if (ch->pcdata->area)
	{
	    ch_printf(ch, "&CVnums&R:   &CRoom &B(&W%-5.5d - %-5.5d&B)   &CObject &B(&W%-5.5d - %-5.5d&B)   &CMob &B(&W%-5.5d - %-5.5d&B)\n\r",
		ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
		ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
		ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
	    ch_printf(ch, "Area Loaded [%s]\n\r", (IS_SET (ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");
	}
    }
    if (ch->first_affect)
    {
	int i;
	SKILLTYPE *sktmp;

	i = 0;
	send_to_char( "&C----------------------------------------------------------------------------\n\r", ch);
	send_to_char("&CAFFECT DATA&R:                            ", ch);
	for (paf = ch->first_affect; paf; paf = paf->next)
	{
	    if ( (sktmp=get_skilltype(paf->type)) == NULL )
		continue;
	    if (ch->top_level < 20)
	    {
		ch_printf(ch, "&B[&W%-34.34s&B]    ", sktmp->name);
		if (i == 0)
		   i = 2;
		if ((++i % 3) == 0)
		   send_to_char("\n\r", ch);
	     }
	     else
	     {
		if (paf->modifier == 0)
		    ch_printf(ch, "&B[&W%-24.24s;%5d rds&B]    ",
			supercapitalize(sktmp->name),
			paf->duration);
		else
		if (paf->modifier > 999)
		    ch_printf(ch, "&B[&W%-15.15s; %7.7s;%5d rds&B]    ",
			supercapitalize(sktmp->name),
			tiny_affect_loc_name(paf->location),
			paf->duration);
		else
		    ch_printf(ch, "&B[&W%-11.11s;%+-3.3d %7.7s;%5d rds&B]&W    ",
			supercapitalize(sktmp->name),
			paf->modifier,
			tiny_affect_loc_name(paf->location),
			paf->duration);
		if (i == 0)
		    i = 1;
		if ((++i % 2) == 0)
		    send_to_char("\n\r", ch);
	    }
	}
    }
    send_to_char("\n\r", ch);
    return;
}

/*
 * Return ascii name of an affect location.
 */
char           *
tiny_affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:		return "NIL";
	case APPLY_STR:			return " STR  ";
	case APPLY_DEX:			return " DEX  ";
	case APPLY_INT:			return " INT  ";
	case APPLY_WIS:			return " WIS  ";
	case APPLY_CON:			return " CON  ";
	case APPLY_CHA:			return " CHA  ";
	case APPLY_LCK:			return " LCK  ";
	case APPLY_SEX:			return " SEX  ";
	case APPLY_LEVEL:		return " LVL  ";
	case APPLY_AGE:			return " AGE  ";
	case APPLY_MANA:		return " MANA ";
	case APPLY_HIT:			return " HV   ";
	case APPLY_MOVE:		return " MOVE ";
	case APPLY_GOLD:		return " GOLD ";
	case APPLY_EXP:			return " EXP  ";
	case APPLY_AC:			return " AC   ";
	case APPLY_HITROLL:		return " HITRL";
	case APPLY_DAMROLL:		return " DAMRL";
	case APPLY_SAVING_POISON:	return "SV POI";
	case APPLY_SAVING_ROD:		return "SV ROD";
	case APPLY_SAVING_PARA:		return "SV PARA";
	case APPLY_SAVING_BREATH:	return "SV BRTH";
	case APPLY_SAVING_SPELL:	return "SV SPLL";
	case APPLY_HEIGHT:		return "HEIGHT";
	case APPLY_WEIGHT:		return "WEIGHT";
	case APPLY_AFFECT:		return "AFF BY";
	case APPLY_RESISTANT:		return "RESIST";
	case APPLY_IMMUNE:		return "IMMUNE";
	case APPLY_SUSCEPTIBLE:		return "SUSCEPT";
	case APPLY_WEAPONSPELL:		return " WEAPON";
	case APPLY_BACKSTAB:		return "BACKSTB";
	case APPLY_PICK:		return " PICK  ";
	case APPLY_TRACK:		return " TRACK ";
	case APPLY_STEAL:		return " STEAL ";
	case APPLY_SNEAK:		return " SNEAK ";
	case APPLY_HIDE:		return " HIDE  ";
	case APPLY_PALM:		return " PALM  ";
	case APPLY_DETRAP:		return " DETRAP";
	case APPLY_DODGE:		return " DODGE ";
	case APPLY_PEEK:		return " PEEK  ";
	case APPLY_SCAN:		return " SCAN  ";
	case APPLY_GOUGE:		return " GOUGE ";
	case APPLY_SEARCH:		return " SEARCH";
	case APPLY_MOUNT:		return " MOUNT ";
	case APPLY_DISARM:		return " DISARM";
	case APPLY_KICK:		return " KICK  ";
	case APPLY_PARRY:		return " PARRY ";
	case APPLY_BASH:		return " BASH  ";
	case APPLY_STUN:		return " STUN  ";
	case APPLY_PUNCH:		return " PUNCH ";
	case APPLY_CLIMB:		return " CLIMB ";
	case APPLY_GRIP:		return " GRIP  ";
	case APPLY_WEARSPELL:		return " WEAR  ";
	case APPLY_REMOVESPELL:		return " REMOVE";
	case APPLY_EMOTION:		return "EMOTION";
	case APPLY_MENTALSTATE:		return " MENTAL";
	case APPLY_STRIPSN:		return " DISPEL";
	case APPLY_REMOVE:		return " REMOVE";
	case APPLY_DIG:			return " DIG   ";
	case APPLY_FULL:		return " HUNGER";
	case APPLY_THIRST:		return " THIRST";
	case APPLY_DRUNK:		return " DRUNK ";
	case APPLY_BLOOD:		return " BLOOD ";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "(???)";
}

char *
get_race( CHAR_DATA *ch)
{
    if ( ch->race < MAX_NPC_RACE && ch->race >= 0)
	return ( npc_race[ch->race] );
    return ("Unknown");
}

void do_oldscore( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    SKILLTYPE   *skill;

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {
       send_to_char("You can't do that in your current state of mind!\n\r", ch);
       return;
    }

    set_char_color( AT_SCORE, ch );
    ch_printf( ch,
	"You are %s%s, level %d, %d years old (%d hours).\n\r",
	ch->name,
	IS_NPC(ch) ? "" : ch->pcdata->title,
	ch->top_level,
	ch->pcage,
	(get_age(ch)) );

    if ( get_trust( ch ) != ch->top_level )
	ch_printf( ch, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );

    if ( IS_SET(ch->act, ACT_MOBINVIS) )
      ch_printf( ch, "You are mobinvis at level %d.\n\r",
            ch->mobinvis);


      ch_printf( ch,
	"You have %d/%d hit, %d/%d movement.\n\r",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move);

    ch_printf( ch,
	"You are carrying %d/%d items with weight %d/%d kg.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );

    ch_printf( ch,
	"Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: ?? \n\r",
	get_curr_str(ch),
	get_curr_int(ch),
	get_curr_wis(ch),
	get_curr_dex(ch),
	get_curr_con(ch),
	get_curr_cha(ch) );

    ch_printf( ch,
	"You have have %lld wulongs.\n\r" , ch->gold );

    if ( !IS_NPC(ch) )
    ch_printf( ch,
	"You have achieved %d glory during your life, and currently have %d.\n\r",
	ch->pcdata->quest_accum, ch->pcdata->quest_curr );

    ch_printf( ch,
	"Autoexit: %s   Autoloot: %s   Autosac: %s   Autocred: %s\n\r",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no",
  	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no" );

    ch_printf( ch, "Wimpy set to %d hit points.\n\r", ch->wimpy );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "You are hungry.\n\r",  ch );

    switch( ch->mental_state / 10 )
    {
        default:   send_to_char( "You're completely messed up!\n\r", ch ); break;
        case -10:  send_to_char( "You're barely conscious.\n\r", ch ); break;
        case  -9:  send_to_char( "You can barely keep your eyes open.\n\r", ch ); break;
        case  -8:  send_to_char( "You're extremely drowsy.\n\r", ch ); break;
        case  -7:  send_to_char( "You feel very unmotivated.\n\r", ch ); break;
        case  -6:  send_to_char( "You feel sedated.\n\r", ch ); break;
        case  -5:  send_to_char( "You feel sleepy.\n\r", ch ); break;
        case  -4:  send_to_char( "You feel tired.\n\r", ch ); break;
        case  -3:  send_to_char( "You could use a rest.\n\r", ch ); break;
        case  -2:  send_to_char( "You feel a little under the weather.\n\r", ch ); break;
        case  -1:  send_to_char( "You feel fine.\n\r", ch ); break;
        case   0:  send_to_char( "You feel great.\n\r", ch ); break;
        case   1:  send_to_char( "You feel energetic.\n\r", ch ); break;
        case   2:  send_to_char( "Your mind is racing.\n\r", ch ); break;
        case   3:  send_to_char( "You can't think straight.\n\r", ch ); break;
        case   4:  send_to_char( "Your mind is going 100 miles an hour.\n\r", ch ); break;
        case   5:  send_to_char( "You're high as a kite.\n\r", ch ); break;
        case   6:  send_to_char( "Your mind and body are slipping appart.\n\r", ch ); break;
        case   7:  send_to_char( "Reality is slipping away.\n\r", ch ); break;
        case   8:  send_to_char( "You have no idea what is real, and what is not.\n\r", ch ); break;
        case   9:  send_to_char( "You feel immortal.\n\r", ch ); break;
        case  10:  send_to_char( "You are a Supreme Entity.\n\r", ch ); break;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    case POS_MOUNTED:
	send_to_char( "Mounted.\n\r",			ch );
	break;
    case POS_SHOVE:
	send_to_char( "Being shoved.\n\r",		ch );
	break;
    case POS_DRAG:
	send_to_char( "Being dragged.\n\r",		ch );
	break;
    }

    if ( ch->top_level >= 25 )
	ch_printf( ch, "AC: %d.  ", GET_AC(ch) );

    send_to_char( "You are ", ch );
	 if ( GET_AC(ch) >=  101 ) send_to_char( "WORSE than naked!\n\r", ch );
    else if ( GET_AC(ch) >=   80 ) send_to_char( "naked.\n\r",            ch );
    else if ( GET_AC(ch) >=   60 ) send_to_char( "wearing clothes.\n\r",  ch );
    else if ( GET_AC(ch) >=   40 ) send_to_char( "slightly armored.\n\r", ch );
    else if ( GET_AC(ch) >=   20 ) send_to_char( "somewhat armored.\n\r", ch );
    else if ( GET_AC(ch) >=    0 ) send_to_char( "armored.\n\r",          ch );
    else if ( GET_AC(ch) >= - 20 ) send_to_char( "well armored.\n\r",     ch );
    else if ( GET_AC(ch) >= - 40 ) send_to_char( "strongly armored.\n\r", ch );
    else if ( GET_AC(ch) >= - 60 ) send_to_char( "heavily armored.\n\r",  ch );
    else if ( GET_AC(ch) >= - 80 ) send_to_char( "superbly armored.\n\r", ch );
    else if ( GET_AC(ch) >= -100 ) send_to_char( "divinely armored.\n\r", ch );
    else                           send_to_char( "invincible!\n\r",       ch );

    if ( ch->top_level >= 15 )
	ch_printf( ch, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );

    if ( ch->top_level >= 10 )
	ch_printf( ch, "Alignment: %d.  ", ch->alignment );

    send_to_char( "You are ", ch );
	 if ( ch->alignment >  900 ) send_to_char( "angelic.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "saintly.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "good.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "kind.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "mean.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "evil.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demonic.\n\r", ch );
    else                             send_to_char( "satanic.\n\r", ch );

    if ( ch->first_affect )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->first_affect; paf; paf = paf->next )
	    if ( (skill=get_skilltype(paf->type)) != NULL )
	{
	    ch_printf( ch, "Spell: '%s'", skill->name );

	    if ( ch->top_level >= 20 )
		ch_printf( ch,
		    " modifies %s by %d for %d rounds",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );

	    send_to_char( ".\n\r", ch );
	}
    }

    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
    {
	ch_printf( ch, "WizInvis level: %d   WizInvis is %s\n\r",
			ch->pcdata->wizinvis,
			IS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  ch_printf( ch, "Room Range: %d - %d\n\r", ch->pcdata->r_range_lo,
					 	   ch->pcdata->r_range_hi	);
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  ch_printf( ch, "Obj Range : %d - %d\n\r", ch->pcdata->o_range_lo,
	  					   ch->pcdata->o_range_hi	);
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  ch_printf( ch, "Mob Range : %d - %d\n\r", ch->pcdata->m_range_lo,
	  					   ch->pcdata->m_range_hi	);
    }

    return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA *ch, char *argument )
{
       int ability;

       for ( ability = 0 ; ability < MAX_ABILITY ; ability++ )
          if ( ability != FORCE_ABILITY || ch->skill_level[FORCE_ABILITY] > 1 )
            ch_printf( ch, "&W%-15s   &CLevel&R: &W%-3d   &CMax&R: &W%-3d   &CExp&R: &W%-10ld   &CNext&R: &W%-10ld\n\r",
            ability_name[ability], ch->skill_level[ability], max_level(ch, ability), ch->experience[ability],
            exp_level( ch->skill_level[ability]+1 ) );
          else
            ch_printf( ch, "&W%-15s   &CLevel&R:&W %-3d   &CMax&R: &W???   &CExp&R: &W???          &CNext&R: &W???\n\r",
            ability_name[ability], ch->skill_level[ability], ch->experience[ability]);
}


void do_affected ( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    SKILLTYPE *skill;

    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "by" ) )
    {
        set_char_color( AT_BLUE, ch );
        send_to_char( "\n\rImbued with:\n\r", ch );
	set_char_color( AT_SCORE, ch );
	ch_printf( ch, "%s\n\r", affect_bit_name( ch->affected_by ) );
        if ( ch->top_level >= 20 )
        {
            send_to_char( "\n\r", ch );
            if ( ch->resistant > 0 )
	    {
		set_char_color ( AT_BLUE, ch );
                send_to_char( "Resistances:  ", ch );
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->resistant, ris_flags) );
	    }
            if ( ch->immune > 0 )
	    {
                set_char_color( AT_BLUE, ch );
                send_to_char( "Immunities:   ", ch);
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->immune, ris_flags) );
	    }
            if ( ch->susceptible > 0 )
	    {
                set_char_color( AT_BLUE, ch );
                send_to_char( "Suscepts:     ", ch );
		set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->susceptible, ris_flags) );
	    }
        }
	return;
    }

    if ( !ch->first_affect )
    {
        set_char_color( AT_SCORE, ch );
        send_to_char( "\n\rNo cantrip or skill affects you.\n\r", ch );
    }
    else
    {
	send_to_char( "\n\r", ch );
        for (paf = ch->first_affect; paf; paf = paf->next)
	    if ( (skill=get_skilltype(paf->type)) != NULL )
        {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Affected:  ", ch );
            set_char_color( AT_SCORE, ch );
            if ( ch->top_level >= 20 )
            {
                if (paf->duration < 25 ) set_char_color( AT_WHITE, ch );
                if (paf->duration < 6  ) set_char_color( AT_WHITE + AT_BLINK, ch );
                ch_printf( ch, "(%5d)   ", paf->duration );
	    }
            ch_printf( ch, "%-18s\n\r", supercapitalize(skill->name) );
        }
    }
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear, dam;
    bool found, emptyslot;
    char buf[MAX_STRING_LENGTH];

    set_char_color( AT_RED, ch );
    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    set_char_color( AT_OBJECT, ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	emptyslot = FALSE;
	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	   if ( obj->wear_loc == iWear )
	   {
		emptyslot = TRUE;
		send_to_char( where_name[iWear], ch );
		if ( can_see_obj( ch, obj ) )
		{
		    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
		    strcpy( buf , "" );
		    switch ( obj->item_type )
	            {
	                default:
	                break;
			case ITEM_MEDPAC:
				ch_printf(ch, " (%d uses left) ", obj->value[0]);
			break;
	                case ITEM_ARMOR:
	    		    if ( obj->value[1] == 0 )
	      			obj->value[1] = obj->value[0];
	    		    if ( obj->value[1] == 0 )
	      			obj->value[1] = 1;
			// So there is no more color bleeding!
	    		    dam = (sh_int) ((obj->value[0] * 10) / obj->value[1]);
			    	if (dam >= 10) strcat( buf, " &w(superb) ");
			    	else if (dam >=  7) strcat( buf, " &w(good) ");
 		            else if (dam >=  5) strcat( buf, " &w(worn) ");
			    	else if (dam >=  3) strcat( buf, " &w(poor) ");
			    	else if (dam >=  1) strcat( buf, " &w(awful) ");
			    	else if (dam ==  0) strcat( buf, " &w(broken) ");
                    send_to_char( buf, ch );
	                break;

	                case ITEM_WEAPON:
	                dam = INIT_WEAPON_CONDITION - obj->value[0];
	                if (dam < 2) strcat( buf, " &w(superb) ");
                    else if (dam < 4) strcat( buf, " &w(good) ");
                    else if (dam < 7) strcat( buf, " &w(worn) ");
                    else if (dam < 10) strcat( buf, " &w(poor) ");
                    else if (dam < 12) strcat( buf, " &w(awful) ");
                    else if (dam ==  12) strcat( buf, " &w(broken) ");
                	send_to_char( buf, ch );
	                if (obj->value[3] == WEAPON_RIFLE )
	                {
	  	            ch_printf( ch, " %d / %d", obj->value[4], obj->value[5] );
	                }
			   		else
	                if (obj->value[3] == WEAPON_PISTOL )
	                {
	  	            ch_printf( ch, " %d / %d", obj->value[4], obj->value[5] );
	                }
			   	    else
	                if (obj->value[3] == WEAPON_HEAVY_WEAPON )
	                {
	  	            ch_printf( ch, " %d / %d", obj->value[4], obj->value[5] );
	                }
	                break;
                    }

		    send_to_char( "\n\r", ch );
		}
		else
		    send_to_char( "something.\n\r", ch );
		found = TRUE;
	   }
	   if(!emptyslot && iWear != WEAR_MISSILE_WIELD)
	   {
		send_to_char( where_name[iWear], ch );
		send_to_char( "Empty - Open Slot.\r\n", ch);
	   }
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( isalpha(title[0]) || isdigit(title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
	strcpy( buf, title );

    STRFREE( ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if( NOT_AUTHED(ch))
    {
	send_to_char("You must be authorized first!\n\r", ch);
	return;
    }

    if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
    {
        send_to_char( "You have lost this privilege.\n\r", ch );
        return;
    }

/*    if (  ch->in_room->vnum == 6 )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }
*/

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

/*	Worthless under new title system which is really the old title system

    if ((get_trust(ch) <= LEVEL_IMMORTAL) && (!nifty_is_name(ch->name, argument)))
     {
       send_to_char("You must include your name somewhere in your title!", ch);
       return;
     }
*/
    // Limit the size of title - Gatz

  if((strlen(argument) > 50) || strlen_color(argument) > 37)
  {
        send_to_char("&RTitle is too long, choose a smaller one.\n\r",ch);
        return;
  }

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "You change your title.\n\r", ch );
}

void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "Your homepage is: %s\n\r",
		show_tilde( ch->pcdata->homepage ) );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "Homepage cleared.\n\r", ch );
	return;
    }

    if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
    else
	sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->homepage )
      DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "Homepage set.\n\r", ch );
}



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Monsters are too dumb to do that!\n\r", ch );
	return;
    }

    if ( !ch->desc )
    {
	bug( "do_description: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_description: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "You cannot use this command from within another command.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_DESC;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->description );
	   return;

	case SUB_PERSONAL_DESC:
	   STRFREE( ch->description );
	   ch->description = copy_buffer( ch );
	   stop_editing( ch );
	   return;
    }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mobs can't set bio's!\n\r", ch );
	return;
    }

    if ( !ch->desc )
    {
	bug( "do_bio: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_bio: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "You cannot use this command from within another command.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_BIO;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->pcdata->bio );
	   return;

	case SUB_PERSONAL_BIO:
	   STRFREE( ch->pcdata->bio );
	   ch->pcdata->bio = copy_buffer( ch );
	   stop_editing( ch );
	   return;
    }
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {
       send_to_char("You can't do that in your current state of mind!\n\r", ch);
       return;
    }


      ch_printf( ch,
	"You report: %d/%d hp %d/%d mv.\n\r",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move   );


      sprintf( buf, "$n reports: %d/%d hp %d/%d.",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move   );

    act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) )
  {
    send_to_char( "NPC's can't change their prompt..\n\r", ch );
    return;
  }
  smash_tilde( argument );
  one_argument( argument, arg );
  if ( !*arg )
  {
    send_to_char( "Set prompt to what? (try help prompt)\n\r", ch );
    return;
  }
  if (ch->pcdata->prompt)
    STRFREE(ch->pcdata->prompt);

  if ( strlen(argument) > 128 )
    argument[128] = '\0';

  /* Can add a list of pre-set prompts here if wanted.. perhaps
     'prompt 1' brings up a different, pre-set prompt */
  if ( !str_cmp(arg, "default") )
    ch->pcdata->prompt = STRALLOC("");
  else
    ch->pcdata->prompt = STRALLOC(argument);
  send_to_char( "Ok.\n\r", ch );
  return;
}
