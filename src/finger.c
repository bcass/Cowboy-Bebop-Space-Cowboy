/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			           Finger Module                                  *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
  #include <dirent.h>
#endif
#include "mud.h"

// local declarations - Draco

void read_finger args (( CHAR_DATA *ch, char *argument ));
void fread_finger args ((CHAR_DATA *ch, FILE *fp, char *laston));


/* Finger snippet courtesy of unknown author. Installed by Samson 4-6-98 */
/* File read/write code redone using standard Smaug I/O routines - Samson 9-12-98 */
void do_finger( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

  if( IS_NPC(ch) )
  {
     send_to_char( "Mobs can't use the finger command.\n\r", ch );
     return;
  }

  if ( argument[0] == '\0' )
  {
      send_to_char("Finger whom?\n\r", ch );
      return;
  }

  victim = get_char_world(ch, argument);

  if ( ( victim == NULL ) || (!victim) )
  {
      read_finger( ch, argument );
      return;
  }

  if ( IS_IMMORTAL(victim) && !IS_IMMORTAL(ch) )
  {
	send_to_char( "Cannot finger an immortal.\n\r", ch );
	return;
  }
/*
  if ( ( ch->top_level < victim->top_level && ch->top_level < LEVEL_INFINITE ) && IS_IMMORTAL(ch) )
  {
	send_to_char( "Cannot finger an immortal above your own level.\n\r", ch );
	return;
  }
*/
  if ( IS_NPC( victim ) )
  {
      read_finger( ch, argument );
      return;
  }

  if ( !can_see( ch, victim ) )
  {
      send_to_char("They aren't here.\n\r", ch );
      return;
  }

  send_to_char("&w          Finger Info\n\r", ch);
  send_to_char("          -----------\n\r", ch);
  ch_printf(ch, "&wName : &G%-20s &wAge: &G%d\n\r", victim->name, victim->pcage );
  ch_printf(ch, "&wSex  : &G%-20s\n\r",
                victim->sex == SEX_MALE   ? "Male"   :
                victim->sex == SEX_FEMALE ? "Female" : "Neutral" );
  ch_printf(ch, "&wTitle: &G%s\n\r", victim->pcdata->title );
  ch_printf(ch, "&wHomepage: &G%s\n\r", victim->pcdata->homepage ? victim->pcdata->homepage : "None" );
//  ch_printf(ch, "&wClan: &G%s\n\r", victim->pcdata->clan ? victim->pcdata->clan->name : "Unclanned" );
  ch_printf(ch, "&wLast on: &G%s\n\r", (char *) ctime( &ch->logon ) );
  return;
}

void read_finger( CHAR_DATA *ch, char *argument )
{
  FILE *fpFinger;
  char buf[MAX_STRING_LENGTH];
  char fingload[MAX_INPUT_LENGTH];
  char *laston;
  struct stat fst;

  buf[0] = '\0';

  sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(argument[0]),
		capitalize( argument ) );
  if ( stat( buf, &fst ) == -1 )
  {
     send_to_char( "&YNo such player exists.\n\r", ch );
     if(!IS_IMMORTAL(ch))
	return;
  }
 

  laston = ctime( &fst.st_mtime );

  sprintf( fingload, "%s%c/%s.F", PLAYER_DIR, tolower(argument[0]),
                     capitalize( argument ) );

  if ( stat( fingload, &fst ) != -1 )
  {
    if ( ( fpFinger = fopen ( fingload, "r" ) ) != NULL )
    {
      for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fpFinger );
	    if ( letter == '*' )
	    {
		fread_to_eol( fpFinger );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "finger.c: read_finger: # not found in finger file.", 0 );
		bug( fingload, 0 );
		ch_printf( ch, "&YThe finger file for %s is unreadable.\n\r", capitalize ( argument ) );
		break;
	    }

	    word = fread_word( fpFinger );
	    if ( !strcmp( word, "FINGER" ) )
	    	fread_finger( ch, fpFinger, laston );
	    else
	    if ( !strcmp( word, "END"    ) )	/* Done		*/
		break;
	    else
	    {
		bug( "finger.c: read_finger: bad section.", 0 );
		bug( fingload, 0 );
		ch_printf( ch, "&YThe finger file for %s is unreadable.\n\r", capitalize ( argument ) );
		break;
	    }
	}
	fclose( fpFinger );
    }
  }
  else
    ch_printf( ch, "&G%s does not have a finger file to read.\n\r", capitalize( argument ) );
  return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )		\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_finger( CHAR_DATA *ch, FILE *fp, char *laston )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    char *email = NULL;
    char *homepage = NULL;
    char *name = NULL;
    char *site = NULL;
    char *title = NULL;
    int  race = 0, sex = 0;
    int level = 0;
    bool fMatch;

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

      case 'E':
	    if ( !strcmp( word, "End" ) )
	        goto finger_display;
	    KEY( "Email",	    email,		fread_string_nohash( fp ) );
	    break;

   	case 'H':
	    KEY( "Homepage",	homepage,	fread_string_nohash( fp ) );
	    break;
	case 'L':
	    KEY( "Level",	level,		fread_number( fp ) );
	    break;
	case 'N':
	    KEY ("Name", 		name, 	fread_string( fp ) );
	    break;

	case 'R':
	    KEY( "Race",        race,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Sex",		sex,		fread_number( fp ) );
	    KEY( "Site",		site,		fread_string( fp ) );
	    break;

	case 'T':
	    KEY( "Title",		title,	fread_string( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "finger.c: fread_finger: no match: %s", word );
	    bug( buf, 0 );
	}
    }

/* Extremely ugly and disgusting goto hack, if there's a better way to
   do this, I'd sure like to know - Samson */

finger_display:
/*
    if ( top_level >= LEVEL_IMMORTAL && !IS_IMMORTAL(ch) )
    {
	send_to_char( "Cannot finger an immortal.\n\r", ch );
	return;
    }

    if ( ( ch->top_level < top_level && ch->top_level < LEVEL_INFINITE ) 
&& IS_IMMORTAL(ch) )
    {
	send_to_char( "Cannot finger an immortal above your own level.\n\r", ch );
	return;
    }
*/

    if( level > 100 && !IS_IMMORTAL(ch) )
    {
	send_to_char("You can not finger an Immortal!\r\n", ch);
	return;
    }

    send_to_char("&w          Finger Info\n\r", ch);
    send_to_char("          -----------\n\r", ch);
    ch_printf(ch, "&wName : &G%-20s\n\r", name );
/*    ch_printf(ch, "&wLevel: &G%-20d\n\r",  top_level );*/
    ch_printf(ch, "&wSex  : &G%-20s &w\n\r",
                sex == SEX_MALE   ? "Male"   :
                sex == SEX_FEMALE ? "Female" : "Neutral");
    ch_printf(ch, "&wTitle: &G%s\n\r", title );
    ch_printf(ch, "&wHomepage: &G%s\n\r", homepage ? homepage : "None" );
    ch_printf(ch, "&wEmail: &G%s\n\r", email ? email : "None specified" );
    ch_printf(ch, "&wLast on: &G%s\n\r", laston );
    if ( IS_IMMORTAL(ch) )
      ch_printf(ch, "&wFrom site: &G%s\n\r", site );

    return;
}

void save_finger( CHAR_DATA *ch )
{
   FILE *fp;
   char fng [MAX_INPUT_LENGTH];
/*
   if( IS_NPC(ch) || ch->top_level < 2 )
      return;
*/
   if ( ch->desc && ch->desc->original )
      ch = ch->desc->original;

   sprintf( fng, "%s%c/%s.F", PLAYER_DIR, tolower(ch->name[0]),
      capitalize( ch->name ) );

   fclose( fpReserve );
   if ( ( fp = fopen( fng, "w" ) ) == NULL )
   {
	bug( "finger.c: save_finger: fopen", 0 );
	perror( fng );
   }
   else
   {
	fprintf( fp, "#FINGER\n" );
      fprintf( fp, "Name		%s~\n",	ch->name );
	fprintf( fp, "Level		%d\n",	ch->top_level );
	fprintf( fp, "Sex		%d\n",	ch->sex );
/*	fprintf( fp, "Class		%d\n", 	ch->class );*/
	fprintf( fp, "Race		%d\n",	ch->race );
	fprintf( fp, "Title		%s~\n",	ch->pcdata->title );
	fprintf( fp, "Homepage	%s~\n",	ch->pcdata->homepage );
	fprintf( fp, "Email		%s~\n",	ch->pcdata->email );
      if ( ch->desc && ch->desc->host )
         fprintf( fp, "Site         %s~\n", ch->desc->host );
	else
         fprintf( fp, "Site         (Link-Dead)~\n" );
	fprintf( fp, "End\n\n" );
	fprintf( fp, "#END\n" );
	fclose(fp);
    }
    fpReserve = fopen ( NULL_FILE, "r" );
    return;
}

/* Added a clone of homepage to let players input their email addy - Samson 4-18-98 */
void do_email( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->email )
	  ch->pcdata->email = str_dup( "" );
	ch_printf( ch, "Your email address is: %s\n\r",
		show_tilde( ch->pcdata->email ) );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
	if ( ch->pcdata->email )
	  DISPOSE(ch->pcdata->email);
	ch->pcdata->email = str_dup("");
	send_to_char( "Email address cleared.\n\r", ch );
	return;
    }

    strcpy( buf, argument );

    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->email )
      DISPOSE(ch->pcdata->email);
    ch->pcdata->email = str_dup(buf);
    send_to_char( "Email address set.\n\r", ch );
}


