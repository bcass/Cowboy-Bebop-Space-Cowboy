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
*			     Special clan module			    *
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

CLAN_DATA * first_clan;
CLAN_DATA * last_clan;

SENATE_DATA * first_senator;
SENATE_DATA * last_senator;

PLANET_DATA * first_planet;
PLANET_DATA * last_planet;

GUARD_DATA * first_guard;
GUARD_DATA * last_guard;

/* local routines */
void	fread_clan	args( ( CLAN_DATA *clan, FILE *fp ) );
void	fread_clan_dependencies	args( ( CLAN_DATA *clan, FILE *fp ) );
bool	load_clan_file	args( ( char *clanfile ) );
bool	load_clan_file_dependencies	args( ( char *clanfile ) );
void	write_clan_list	args( ( void ) );
void	fread_planet	args( ( PLANET_DATA *planet, FILE *fp ) );
bool	load_planet_file	args( ( char *planetfile ) );
void	write_planet_list	args( ( void ) );
void	support_update	args( ( void ) );
void	generate_name args( ( CHAR_DATA *victim));
void	citizen_generate	args(( void ));
void 	check_winners	args( ( void ) );
void    update_planettrade args( ( void ) );


extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern int      top_affect;

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_cmp( name, clan->name ) )
         return clan;
    return NULL;
}

PLANET_DATA *get_planet( char *name )
{
    PLANET_DATA *planet;

    for ( planet = first_planet; planet; planet = planet->next )
       if ( !str_cmp( name, planet->name ) )
         return planet;
    return NULL;
}

void write_clan_list( )
{
    CLAN_DATA *tclan;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open clan.lst for writing!\n\r", 0 );
 	return;
    }
    for ( tclan = first_clan; tclan; tclan = tclan->next )
	fprintf( fpout, "%s\n", tclan->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void write_planet_list( )
{
    PLANET_DATA *tplanet;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", PLANET_DIR, PLANET_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open planet.lst for writing!\n\r", 0 );
 	return;
    }
    for ( tplanet = first_planet; tplanet; tplanet = tplanet->next )
	fprintf( fpout, "%s\n", tplanet->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA *clan )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !clan )
    {
	bug( "save_clan: null clan pointer!", 0 );
	return;
    }

    if ( !clan->filename || clan->filename[0] == '\0' )
    {
	sprintf( buf, "save_clan: %s has no filename", clan->name );
	bug( buf, 0 );
	return;
    }

    sprintf( filename, "%s%s", CLAN_DIR, clan->filename );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_clan: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#CLAN\n" );
	fprintf( fp, "Name         %s~\n",	clan->name		);
	fprintf( fp, "Filename     %s~\n",	clan->filename		);
	fprintf( fp, "Description  %s~\n",	clan->description	);
	fprintf( fp, "Immortal     %s~\n",	clan->immortal		);
	fprintf( fp, "Leader       %s~\n",	clan->leader		);
	fprintf( fp, "NumberOne    %s~\n",	clan->number1		);
	fprintf( fp, "NumberTwo    %s~\n",	clan->number2		);
	fprintf( fp, "PKills       %d\n",	clan->pkills		);
	fprintf( fp, "PDeaths      %d\n",	clan->pdeaths		);
	fprintf( fp, "MKills       %d\n",	clan->mkills		);
	fprintf( fp, "MDeaths      %d\n",	clan->mdeaths		);
	fprintf( fp, "Type         %d\n",	clan->clan_type		);
	fprintf( fp, "Members      %d\n",	clan->members		);
	fprintf( fp, "Board        %d\n",	clan->board		);
	fprintf( fp, "Storeroom    %d\n",	clan->storeroom		);
	fprintf( fp, "GuardOne     %d\n",	clan->guard1		);
	fprintf( fp, "GuardTwo     %d\n",	clan->guard2		);
	fprintf( fp, "PatrolOne    %d\n",	clan->patrol1		);
	fprintf( fp, "PatrolTwo    %d\n",	clan->patrol2		);
	fprintf( fp, "TrooperOne   %d\n",	clan->trooper1		);
	fprintf( fp, "TrooperTwo   %d\n",	clan->trooper2		);
	fprintf( fp, "Funds        %lld\n",	clan->funds		);
	fprintf( fp, "Jail         %d\n",       clan->jail            	);
	fprintf( fp, "Supportvnum  %d\n",	clan->supportvnum	);
	fprintf( fp, "WarWon	   %d\n",	clan->warwoncount	);
	fprintf( fp, "WarLost	   %d\n",	clan->warlostcount	);
	if ( clan->mainclan )
   	   fprintf( fp, "MainClan     %s~\n",	clan->mainclan->name	);
	if ( clan->war1 )
	{
	    fprintf( fp, "War1	   %s~\n",	clan->war1->name	);
	    fprintf( fp, "War1Counter	%d\n",	clan->war1counter	);
	}
	if( clan->war2)
	{
	    fprintf( fp, "War2	   %s~\n",	clan->war2->name	);
	    fprintf( fp, "War2Counter	%d\n",	clan->war2counter	);
	}
	if( clan->ally1)
	    fprintf( fp, "Ally1	   %s~\n",	clan->ally1->name	);
	if( clan->ally2)
	    fprintf( fp, "Ally2    %s~\n", 	clan->ally2->name	);
	if(clan->warkills)
		fprintf( fp, "WarKills %d\n",	clan->warkills	); /* added for capship GRP on 2006-08-06 by Funf */
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void save_planet( PLANET_DATA *planet )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !planet )
    {
	bug( "save_planet: null planet pointer!", 0 );
	return;
    }

    if ( !planet->filename || planet->filename[0] == '\0' )
    {
	sprintf( buf, "save_planet: %s has no filename", planet->name );
	bug( buf, 0 );
	return;
    }

    sprintf( filename, "%s%s", PLANET_DIR, planet->filename );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_planet: fopen", 0 );
    	perror( filename );
    }
    else
    {
        AREA_DATA *pArea;

	fprintf( fp, "#PLANET\n" );
	fprintf( fp, "Name         %s~\n",	planet->name		);
	fprintf( fp, "Filename     %s~\n",	planet->filename        );
	fprintf( fp, "BaseValue    %lld\n",	planet->base_value      );
	fprintf( fp, "Flags        %d\n",	planet->flags           );
	fprintf( fp, "PopSupport   %f\n",	planet->pop_support     );
	fprintf( fp, "FrustLevel   %f\n",	planet->frust_level	);
	fprintf( fp, "Hospital     %d\n",	planet->hospital	);
	fprintf( fp, "Lowgang	   %d\n",	planet->lowgang		);
	fprintf( fp, "Highgang	   %d\n", 	planet->highgang	);
	if ( planet->starsystem && planet->starsystem->name )
        	fprintf( fp, "Starsystem   %s~\n",	planet->starsystem->name);
	if ( planet->governed_by && planet->governed_by->name )
        	fprintf( fp, "GovernedBy   %s~\n",	planet->governed_by->name);
	if ( planet->frustrated_by && planet->frustrated_by->name )
		fprintf( fp, "FrustratedBy   %s~\n",	planet->frustrated_by->name);
	fprintf( fp, "Supply	   %d\n", 	planet->supply	);
	fprintf( fp, "SupplyRate   %d\n", 	planet->supply_rate	);
	fprintf( fp, "ContrabandSupply	%d\n", 	planet->contraband_supply	);
	fprintf( fp, "ContrabandSupplyRate	%d\n", 	planet->contraband_supply_rate	);
	fprintf( fp, "Demand	   %d\n", 	planet->demand	);
	fprintf( fp, "DemandRate   %d\n", 	planet->demand_rate	);
	fprintf( fp, "Security	   %d\n", 	planet->security	);
	for( pArea = planet->first_area ; pArea ; pArea = pArea->next_on_planet )
	    if (pArea->filename)
         	fprintf( fp, "Area         %s~\n",	pArea->filename  );
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual clan data.
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

void fread_clan( CLAN_DATA *clan, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
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
	case 'A':
            if ( !str_cmp( word, "Ally1" ) )
            {
                /*clan->ally1 = get_clan ( fread_string(fp) );*/ // Dependent code - Funf
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "Ally2" ) )
            {
                /*clan->ally2 = get_clan ( fread_string(fp) );*/
                fMatch = TRUE;
            }

	    break;

	case 'B':
	    KEY( "Board",	clan->board,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	clan->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!clan->name)
		  clan->name		= STRALLOC( "" );
		if (!clan->immortal)
		  clan->immortal	= STRALLOC( "" );
		if (!clan->leader)
		  clan->leader		= STRALLOC( "" );
		if (!clan->description)
		  clan->description 	= STRALLOC( "" );
		if (!clan->number1)
		  clan->number1		= STRALLOC( "" );
		if (!clan->number2)
		  clan->number2		= STRALLOC( "" );
		if (!clan->tmpstr)
		  clan->tmpstr		= STRALLOC( "" );
		return;
	    }
	    break;

	case 'F':
	    KEY( "Funds",	clan->funds,		fread_long( fp ) );
	    KEY( "Filename",	clan->filename,		fread_string_nohash( fp ) );
	    break;

	case 'G':
	    KEY( "GuardOne",	clan->guard1,		fread_number( fp ) );
	    KEY( "GuardTwo",	clan->guard2,		fread_number( fp ) );
	    break;

	case 'I':
	    KEY( "Immortal",	clan->immortal,		fread_string( fp ) );
	    break;

	case 'J':
	    KEY( "Jail",	clan->jail,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Leader",	clan->leader,		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "MDeaths",	clan->mdeaths,		fread_number( fp ) );
	    KEY( "Members",	clan->members,		fread_number( fp ) );
	    KEY( "MKills",	clan->mkills,		fread_number( fp ) );
	    KEY( "MainClan",	clan->tmpstr,		fread_string( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	clan->name,		fread_string( fp ) );
	    KEY( "NumberOne",	clan->number1,		fread_string( fp ) );
	    KEY( "NumberTwo",	clan->number2,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "PDeaths",	clan->pdeaths,		fread_number( fp ) );
	    KEY( "PKills",	clan->pkills,		fread_number( fp ) );
	    KEY( "PatrolOne",	clan->patrol1,		fread_number( fp ) );
	    KEY( "PatrolTwo",	clan->patrol2,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Supportvnum", clan->supportvnum,	fread_number( fp ) );
	    KEY( "Storeroom",	clan->storeroom,	fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Type",	clan->clan_type,	fread_number( fp ) );
	    KEY( "TrooperOne",	clan->trooper1,		fread_number( fp ) );
	    KEY( "TrooperTwo",	clan->trooper2,		fread_number( fp ) );
	    break;
	case 'W':
	    if ( !str_cmp( word, "War1" ) )
            {
                /* clan->war1 = get_clan ( fread_string(fp) ); */
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "War2" ) )
            {
                /* clan->war2 = get_clan ( fread_string(fp) ); */
                fMatch = TRUE;
            }
	    KEY( "War1Counter",	clan->war1counter,	fread_number( fp ));
	    KEY( "War2Counter",	clan->war2counter,	fread_number( fp ));
	    KEY( "WarWon",	clan->warwoncount,	fread_number( fp ));
	    KEY( "WarLost",	clan->warlostcount,	fread_number( fp ));
	    KEY( "WarKills",	clan->warkills,	fread_number( fp )); /* added for capship GRP on 2006-08-06 by Funf */
	    break;

	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_clan: no match: %s", word );
	    bug( buf, 0 );
	}

    }
}

void fread_clan_dependencies( CLAN_DATA *clan, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
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
	case 'A':
            if ( !str_cmp( word, "Ally1" ) )
            {
                clan->ally1 = get_clan ( fread_string(fp) );
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "Ally2" ) )
            {
                clan->ally2 = get_clan ( fread_string(fp) );
                fMatch = TRUE;
            }

	    break;

	case 'B':
		if ( !str_cmp( word, "Board" ) ) fMatch = TRUE;
	    break;

	case 'D':
		if ( !str_cmp( word, "Description" ) ) fMatch = TRUE;
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		 return;
	    }
	    break;

	case 'F':
	    if ( !str_cmp( word, "Funds" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "Filename" ) ) fMatch = TRUE;
	    break;

	case 'G':
	    if ( !str_cmp( word, "GuardOne" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "GuardTwo" ) ) fMatch = TRUE;
	    break;

	case 'I':
	    if ( !str_cmp( word, "Immortal" ) ) fMatch = TRUE;
	    break;

	case 'J':
	    if ( !str_cmp( word, "Jail" ) ) fMatch = TRUE;
	    break;

	case 'L':
	    if ( !str_cmp( word, "Leader" ) ) fMatch = TRUE;
	    break;

	case 'M':
	    if ( !str_cmp( word, "MDeaths" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "Members" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "MKills" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "MainClan" ) ) fMatch = TRUE;
	    break;

	case 'N':
	    if ( !str_cmp( word, "Name" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "NumberOne" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "NumberTwo" ) ) fMatch = TRUE;
	    break;

	case 'P':
	    if ( !str_cmp( word, "PDeaths" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "PKills" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "PatrolOne" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "PatrolTwo" ) ) fMatch = TRUE;
	    break;

	case 'S':
	    if ( !str_cmp( word, "Supportvnum" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "Storeroom" ) ) fMatch = TRUE;
	    break;

	case 'T':
	    if ( !str_cmp( word, "Type" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "TrooperOne" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "TrooperTwo" ) ) fMatch = TRUE;
	    break;
	case 'W':
	    if ( !str_cmp( word, "War1" ) )
            {
                clan->war1 = get_clan ( fread_string(fp) );
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "War2" ) )
            {
                clan->war2 = get_clan ( fread_string(fp) );
                fMatch = TRUE;
            }
	    if ( !str_cmp( word, "War1Counter" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "War2Counter" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "WarWon" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "WarLost" ) ) fMatch = TRUE;
	    if ( !str_cmp( word, "WarKills" ) ) fMatch = TRUE;
	    break;

	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_clan_dependencies: no match: %s", word );
	    bug( buf, 0 );
	}

    }
}

void fread_planet( PLANET_DATA *planet, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
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

	case 'A':
	    if ( !str_cmp( word, "Area" ) )
	    {
	        char aName[MAX_STRING_LENGTH];
                AREA_DATA *pArea;

	     	sprintf (aName, fread_string(fp));
		for( pArea = first_area ; pArea ; pArea = pArea->next )
	          if (pArea->filename && !str_cmp(pArea->filename , aName ) )
	          {
	             pArea->planet = planet;
	             LINK( pArea, planet->first_area, planet->last_area, next_on_planet, prev_on_planet);
	          }
                fMatch = TRUE;
	    }

	    break;

	case 'B':
	    KEY( "BaseValue",	planet->base_value,		fread_number( fp ) );
	    break;

	case 'C':
		KEY( "ContrabandSupply",	planet->contraband_supply,		        fread_number( fp ) );
		KEY( "ContrabandSupplyRate",	planet->contraband_supply_rate,		        fread_number( fp ) );
		break;

	case 'D':
		KEY( "Demand",	planet->demand,		        fread_number( fp ) );
		KEY( "DemandRate",	planet->demand_rate,		        fread_number( fp ) );
		break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!planet->name)
		  planet->name		= STRALLOC( "" );
		return;
	    }
	    break;

	case 'F':
	    KEY( "Filename",	planet->filename,		fread_string_nohash( fp ) );
	    KEY( "Flags",	planet->flags,		        fread_number( fp ) );
	    KEY( "FrustLevel",  planet->frust_level,		fread_number( fp ) );
            if ( !str_cmp( word, "FrustratedBy" ) )
            {
                planet->frustrated_by = get_clan ( fread_string(fp) );
                fMatch = TRUE;
            }
	    break;

	case 'G':
	    if ( !str_cmp( word, "GovernedBy" ) )
	    {
	     	planet->governed_by = get_clan ( fread_string(fp) );
                fMatch = TRUE;
	    }
	    break;
	case 'H':
	    KEY("Highgang",	planet->highgang,	fread_number( fp ));
	    KEY("Hospital",	planet->hospital,	fread_number( fp ));
	    break;
	case 'L':
	    KEY("Lowgang",	planet->lowgang,	fread_number( fp ));
	    break;
	case 'N':
	    KEY( "Name",	planet->name,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "PopSupport",	planet->pop_support,		fread_number( fp ) );
	    break;

	case 'S':
		KEY( "Security",	planet->security,		        fread_number( fp ) );
		KEY( "Supply",	planet->supply,		        fread_number( fp ) );
		KEY( "SupplyRate",	planet->supply_rate,		        fread_number( fp ) );
	    if ( !str_cmp( word, "Starsystem" ) )
	    {
	     	planet->starsystem = starsystem_from_name ( fread_string(fp) );
                if (planet->starsystem)
                {
                     SPACE_DATA *starsystem = planet->starsystem;

                     LINK( planet , starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
                }
                fMatch = TRUE;
	    }
	    break;

	case 'T':
	    KEY( "Taxes",	planet->base_value,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_planet: no match: %s", word );
	    bug( buf, 0 );
	}

    }
}


/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan;
    FILE *fp;
    bool found;

    CREATE( clan, CLAN_DATA, 1 );
    clan->next_subclan = NULL;
    clan->prev_subclan = NULL;
    clan->last_subclan = NULL;
    clan->first_subclan = NULL;
    clan->mainclan     = NULL;
    clan->war1	= NULL;
    clan->war2  = NULL;
    clan->ally1 = NULL;
    clan->ally2 = NULL;

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

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
		bug( "Load_clan_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "CLAN"	) )
	    {
	    	fread_clan( clan, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_clan_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
    {

	ROOM_INDEX_DATA *storeroom;

	LINK( clan, first_clan, last_clan, next, prev );

	if ( clan->storeroom == 0
	|| (storeroom = get_room_index( clan->storeroom )) == NULL )
	{
	    log_string( "Storeroom not found" );
	    return found;
	}

	sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
	if ( ( fp = fopen( filename, "r" ) ) != NULL )
	{
	    int iNest;
	    bool found;
	    OBJ_DATA *tobj, *tobj_next;

	    log_string( "Loading clan storage room" );
	    rset_supermob(storeroom);
	    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
		rgObjNest[iNest] = NULL;

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
		    bug( "Load_clan_vault: # not found.", 0 );
		    bug( clan->name, 0 );
		    break;
		}

		word = fread_word( fp );
		if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
		  fread_obj  ( supermob, fp, OS_CARRY );
		else
		if ( !str_cmp( word, "END"    ) )	/* Done		*/
		  break;
		else
		{
		    bug( "Load_clan_vault: bad section.", 0 );
		    bug( clan->name, 0 );
		    break;
		}
	    }
	    fclose( fp );
	    for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
	    {
		tobj_next = tobj->next_content;
		obj_from_char( tobj );
		obj_to_room( tobj, storeroom );
	    }
	    release_supermob();
	}
	else
	    log_string( "Cannot open clan vault" );
    }
    else
      DISPOSE( clan );


    return found;
}

bool load_clan_file_dependencies( char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan;
    FILE *fp;
    bool found;

	if ( !(clan = get_clan( clanfile )) )
	{
		return FALSE;
	}

    clan->war1	= NULL;
    clan->war2  = NULL;
    clan->ally1 = NULL;
    clan->ally2 = NULL;

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

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
		bug( "Load_clan_file_dependencies: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "CLAN"	) )
	    {
	    	fread_clan_dependencies( clan, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_clan_file_dependencies: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    return found;
}

bool load_planet_file( char *planetfile )
{
    char filename[256];
    PLANET_DATA *planet;
    FILE *fp;
    bool found;

    CREATE( planet, PLANET_DATA, 1 );

    planet->governed_by = NULL;
    planet->frustrated_by = NULL;
    planet->next_in_system = NULL;
    planet->prev_in_system = NULL;
    planet->starsystem = NULL ;
    planet->first_area = NULL;
    planet->last_area = NULL;
    planet->first_guard = NULL;
    planet->last_guard = NULL;
    planet->citizencount = 0;
    planet->base_value = 0;
    planet->population = 0;
    planet->hospital = 0;
    planet->lowgang = 0;
    planet->highgang = 0;
    planet->supply = 0;
    planet->supply_rate = 0;
    planet->contraband_supply = 0;
    planet->contraband_supply_rate = 0;
    planet->demand = 0;
    planet->demand_rate = 0;
    planet->security = 0;

    found = FALSE;
    sprintf( filename, "%s%s", PLANET_DIR, planetfile );

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
		bug( "Load_planet_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "PLANET"	) )
	    {
	    	fread_planet( planet, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_planet_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( !found )
      DISPOSE( planet );
    else
      LINK( planet, first_planet, last_planet, next, prev );

    return found;
}


/*
 * Load in all the clan files.
 */
void load_clans( )
{
    FILE *fpList;
    char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];
    CLAN_DATA *clan;
    CLAN_DATA *bosclan;

    first_clan	= NULL;
    last_clan	= NULL;

    log_string( "Loading clans..." );

    sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
    fclose( fpReserve );

    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
	perror( clanlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_clan_file( filename ) )
	{
	  sprintf( buf, "Cannot load clan file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );

    log_string(" Done clans\n\rLoading clan dependencies...." );

    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
	 perror( clanlist );
	 exit( 1 );
    }

    for ( ; ; )
    {
	 filename = feof( fpList ) ? "$" : fread_word( fpList );
	 log_string( filename );
	 if ( filename[0] == '$' )
	   break;

	 if ( !load_clan_file_dependencies( filename ) )
	 {
	   sprintf( buf, "Cannot load clan file: %s", filename );
	   bug( buf, 0 );
	 }
    }
    fclose( fpList );

    log_string(" Done clan dependencies\n\rSorting clans...." );

    fpReserve = fopen( NULL_FILE, "r" );

    for ( clan=first_clan ; clan ; clan = clan->next )
    {
       if ( !clan->tmpstr || clan->tmpstr[0] == '\0' )
         continue;

       bosclan = get_clan ( clan->tmpstr );
       if ( !bosclan )
         continue;

       LINK( clan , bosclan->first_subclan , bosclan->last_subclan , next_subclan, prev_subclan );
       clan->mainclan = bosclan;
    }

    log_string(" Done sorting" );
    return;
}

void load_planets( )
{
    FILE *fpList;
    char *filename;
    char planetlist[256];
    char buf[MAX_STRING_LENGTH];

    first_planet	= NULL;
    last_planet	= NULL;

    log_string( "Loading planets..." );

    sprintf( planetlist, "%s%s", PLANET_DIR, PLANET_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( planetlist, "r" ) ) == NULL )
    {
	perror( planetlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_planet_file( filename ) )
	{
	  sprintf( buf, "Cannot load planet file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done planets " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_overthrow ( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    CLAN_DATA *clan;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Overthrow what clan?\n\r", ch );
	return;
    }

    clan = get_clan( argument );

    if( !clan )
    {
	send_to_char( "Invalid clan name.\n\r", ch );
	return;
    }

    if( ch->pcdata->clan )
    {
	send_to_char( "You'll have to quit your clan before you can do that\n\r", ch );
	return;
    }

    if( clan->leader[0] != '\0' )
    {
	send_to_char( "You can't overthrow a clan with a leader!\n\r", ch );
	return;
    }

    if( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD )
    {
	send_to_char( "You can't overthrow a group of bounty hunters!\n\r", ch );
	return;
    }

    if( clan->leader[0] == '\0' )
    {
	SET_BIT( ch->speaks, LANG_CLAN );
	STRFREE( ch->pcdata->clan_name );
	ch->pcdata->clan_name = QUICKLINK( clan->name );
	ch->pcdata->clan = clan;
	clan->leader = STRALLOC(ch->name);
	clan->members++;
	send_to_char( "Clan overthrown!\n\r", ch );
	sprintf( buf, "%s has been overthrown!", clan->name );
        echo_to_all( AT_RED , buf, 0 );
	save_clan( clan );
	return;
    }
}

void do_make( CHAR_DATA *ch, char *argument )
{
	send_to_char( "Huh?\n\r", ch );
	return;
}

void do_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("induct", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->immortal  )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 ) )
	;
    else
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Induct whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->pcdata->clan )
    {
      if ( victim->pcdata->clan->clan_type == CLAN_CRIME )
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your crime family!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an organization!\n\r", ch );
	return;
      }
      else if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your guild!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an organization!\n\r", ch );
	return;
      }
      else
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your organization!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an organization!\n\r", ch );
	return;
      }

    }

    clan->members++;

    victim->pcdata->clan = clan;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = QUICKLINK( clan->name );
    act( AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
    act( AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT );
    save_char_obj( victim );
    return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];
    char fname[1024];
    char name[256];
    struct stat fst;
    bool loaded;
    DESCRIPTOR_DATA *d;
    int old_room_vnum, rank, trank;



    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("outcast", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->immortal  )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 ) )
	;
    else
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Outcast whom?\n\r", ch );
	return;
    }
/*  Changed for new outcast ability
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }
*/
    if( ( victim = get_char_room( ch, arg ) ) == NULL
	 && (victim = get_char_world( ch, arg )) == NULL)
    {

    strcpy( name, arg);

    if(!str_cmp(clan->leader, ch->name))
	rank = 1;
    if(!str_cmp(clan->number1, ch->name))
	rank = 2;
    if(!str_cmp(clan->number2, ch->name))
	rank = 3;
    if(!str_cmp(clan->immortal, ch->name))
	rank = 0;

    name[0] = UPPER(name[0]);

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );
    if ( stat( fname, &fst ) != -1 )
    {
	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->next = NULL;
	d->prev = NULL;
	d->connected = CON_GET_NAME;
	d->outsize = 2000;
	CREATE( d->outbuf, char, d->outsize );

	loaded = load_char_obj( d, name, FALSE );
	add_char( d->character );
    if(!str_cmp(clan->leader, d->character->name))
	trank = 1;
    else
	if(!str_cmp(clan->number1, d->character->name))
		trank = 2;
	else
		if(!str_cmp(clan->number2, d->character->name))
			trank = 3;
		else
			if(!str_cmp(clan->immortal, d->character->name))
				trank = 0;

	if ( get_trust(d->character) >= get_trust( ch )
	     || !d->character->pcdata
	     || !d->character->pcdata->clan
	     || d->character->pcdata->clan != clan
	     || trank < rank)
	{
	   do_say( d->character, "Do *NOT* disturb me again!" );
	   send_to_char( "I think you'd better leave that player alone!\n\r", ch );
//	   char_to_room(d->character, old_room_vnum);
	   d->character->desc	= NULL;
	   do_quit( d->character, "" );
	   return;
	}
	d->character->desc	= NULL;
	d->character->retran    = old_room_vnum;
    if ( d->character->speaking & LANG_CLAN )
        d->character->speaking = LANG_COMMON;
    REMOVE_BIT( d->character->speaks, LANG_CLAN );
    --clan->members;
    if ( !str_cmp( d->character->name, ch->pcdata->clan->number1 ) )
    {
        STRFREE( ch->pcdata->clan->number1 );
        ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    if ( !str_cmp( d->character->name, ch->pcdata->clan->number2 ) )
    {
        STRFREE( ch->pcdata->clan->number2 );
        ch->pcdata->clan->number2 = STRALLOC( "" );
    }
        d->character->pcdata->clan = NULL;
        STRFREE(d->character->pcdata->clan_name);
        d->character->pcdata->clan_name = STRALLOC( "" );
        d->character->pcdata->setclantalk = NULL;
        STRFREE(d->character->pcdata->setclantalk_name);
        d->character->pcdata->setclantalk_name = STRALLOC("");
        act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, d->character, TO_CHAR );
        act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, d->character, TO_ROOM );
  //  act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, d->character, TO_VICT );
        DISPOSE( d->character->pcdata->bestowments );
        d->character->pcdata->bestowments = str_dup("");

        save_char_obj( d->character );    /* clan gets saved when pfile is saved */

//	char_to_room(d->character, old_room_vnum);
	do_quit(d->character, "");

	return;
    }
    send_to_char( "I'm not sure any person like that exists!\n\r", ch ); // 2009-08-10 Funf (bug 'outcast iDontExist' crashing mud)
    return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }
    // Just in case check - Gatz
    if( (victim = get_char_world(ch, arg)) == NULL)
    {
	send_to_char( "They aren't here!\r\n", ch);
	return;
    }
    if ( victim == ch )
    {
	    send_to_char( "Kick yourself out of your own clan?\n\r", ch );
	    return;
    }

    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
	    send_to_char( "This player does not belong to your clan!\n\r", ch );
	    return;
    }


    if ( victim->speaking & LANG_CLAN )
        victim->speaking = LANG_COMMON;
    REMOVE_BIT( victim->speaks, LANG_CLAN );
    --clan->members;
    if ( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
    {
	STRFREE( ch->pcdata->clan->number1 );
	ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    if ( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
    {
	STRFREE( ch->pcdata->clan->number2 );
	ch->pcdata->clan->number2 = STRALLOC( "" );
    }
    victim->pcdata->clan = NULL;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = STRALLOC( "" );
    victim->pcdata->setclantalk = NULL;
    STRFREE(victim->pcdata->setclantalk_name);
    victim->pcdata->setclantalk_name = STRALLOC("");

    act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT );
/*    sprintf(buf, "%s has been outcast from %s!", victim->name, clan->name);
    echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL); */

    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup("");

    save_char_obj( victim );	/* clan gets saved when pfile is saved */
    return;
}

void do_setclan( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setclan <clan> <field> <leader|number1|number2> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " immortal leader number1 number2 subclan\n\r", ch );
	send_to_char( " members board recall storage\n\r", ch );
	send_to_char( " funds trooper1 trooper2 jail\n\r", ch );
	send_to_char( " guard1 guard2 patrol1 patrol2 supportvnum war1 war2\n\r", ch );
	send_to_char( "peace1 peace2 ally1 ally2\n\r", ch);
	if ( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
	{
	  send_to_char( " name filename desc\n\r", ch );
	}
	return;
    }

    clan = get_clan( arg1 );
    if ( !clan )
    {
	send_to_char( "No such clan.\n\r", ch );
	return;
    }

    if ( !strcmp( arg2, "immortal" ) )
    {
	STRFREE( clan->immortal );
	if(argument != '\0')
		clan->immortal = STRALLOC( argument );
	else
		clan->immortal = NULL;
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "leader" ) )
    {
	STRFREE( clan->leader );
	clan->leader = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "subclan" ) )
    {
        CLAN_DATA *subclan;
        subclan = get_clan( argument );
        if ( !subclan )
        {
            send_to_char( "Subclan is not a clan.\n\r", ch );
            return;
        }
        if ( subclan->clan_type == CLAN_SUBCLAN || subclan->mainclan )
        {
            send_to_char( "Subclan is already part of another organization.\n\r", ch );
            return;
        }
        if ( subclan->first_subclan )
        {
            send_to_char( "Subclan has subclans of its own that need removing first.\n\r", ch );
            return;
        }
        subclan->clan_type = CLAN_SUBCLAN;
        subclan->mainclan = clan;
        LINK(subclan, clan->first_subclan, clan->last_subclan, next_subclan, prev_subclan );
	save_clan( clan );
	save_clan( subclan );
	return;
    }

    if ( !strcmp( arg2, "number1" ) )
    {
	STRFREE( clan->number1 );
	clan->number1 = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "number2" ) )
    {
	STRFREE( clan->number2 );
	clan->number2 = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "board" ) )
    {
	clan->board = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "members" ) )
    {
	clan->members = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "funds" ) )
    {
	clan->funds = atoll( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if( !strcmp( arg2, "supportvnum" ))
    {
	clan->supportvnum = atoi( argument );
	send_to_char( "Done.\n\r", ch);
	save_clan( clan );
	return;
    }
    if( !strcmp( arg2, "war1counter"))
    {
	clan->war1counter = atoi( argument );
	send_to_char( "Done.\n\r", ch);
	save_clan( clan );
	return;
    }
    if( !strcmp( arg2, "war2counter"))
    {
        clan->war2counter = atoi( argument );
        send_to_char( "Done.\n\r", ch);
        save_clan( clan );
        return;
    }

    if( !strcmp( arg2, "war1"))
    {
	if(argument[0] == '\0')
	{
		send_to_char("setclan clan war1 clan/none \n\r", ch);
		return;
	}
	if( !strcmp( argument, "none"))
	{
		if(clan->war1)
		{
			send_to_char("Removing War status, don't forgot to remove the other clans. \n\r", ch);
			clan->war1 = NULL;
			save_clan( clan );
			return;
		}
		else
		{
			send_to_char("This clan has no War1 data to remove!\n\r", ch);
			return;
		}
	}
	CLAN_DATA *victim;
	victim = get_clan( argument );
	if(!victim)
	{
		send_to_char("Clan is not a real clan!\n\r", ch);
		return;
	}
	clan->war1 = victim;
	save_clan( clan );
	save_clan( victim );
	return;
    }
    if( !strcmp( arg2, "war2"))
    {
        if(argument[0] == '\0')
        {
                send_to_char("setclan clan war2 clan/none \n\r", ch);
                return;
        }
        if( !strcmp( argument, "none"))
        {
                if(clan->war2)
                {
                        send_to_char("Removing War status, don't forgot to remove the other clans. \n\r", ch);
                        clan->war2 = NULL;
			save_clan( clan );
                        return;
                }
                else
                {
                        send_to_char("This clan has no war2 data to remove!\n\r", ch);
                        return;
                }
        }
        CLAN_DATA *victim;
        victim = get_clan( argument );
        if(!victim)
        {
                send_to_char("Clan is not a real clan!\n\r", ch);
                return;
        }
        clan->war2 = victim;
	save_clan( clan );
	save_clan( victim );
        return;
    }
    if( !strcmp( arg2, "ally1"))
    {
        if(argument[0] == '\0')
        {
                send_to_char("setclan clan ally1 clan/none \n\r", ch);
                return;
        }
        if( !strcmp( argument, "none"))
        {
                if(clan->ally1)
                {
                        send_to_char("Removing ally status, don't forgot to remove the other clans. \n\r", ch);
                        clan->ally1 = NULL;
			save_clan( clan );
                        return;
                }
                else
                {
                        send_to_char("This clan has no ally1 data to remove!\n\r", ch);
                        return;
                }
        }
        CLAN_DATA *victim;
        victim = get_clan( argument );
        if(!victim)
        {
                send_to_char("Clan is not a real clan!\n\r", ch);
                return;
        }
        clan->ally1 = victim;
	save_clan( clan );
	save_clan( victim );
        return;
    }
    if( !strcmp( arg2, "ally2"))
    {
        if(argument[0] == '\0')
        {
                send_to_char("setclan clan ally2 clan/none \n\r", ch);
                return;
        }
        if( !strcmp( argument, "none"))
        {
                if(clan->ally2)
                {
                        send_to_char("Removing ally status, don't forgot to remove the other clans. \n\r", ch);
                        clan->ally2 = NULL;
			save_clan( clan );
                        return;
                }
                else
                {
                        send_to_char("This clan has no ally2 data to remove!\n\r", ch);
                        return;
                }
        }
        CLAN_DATA *victim;
        victim = get_clan( argument );
        if(!victim)
        {
                send_to_char("Clan is not a real clan!\n\r", ch);
                return;
        }
        clan->ally2 = victim;
	save_clan( clan );
	save_clan( victim );
        return;
    }
    if ( !strcmp( arg2, "storage" ) )
    {
	clan->storeroom = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "guard1" ) )
    {
	clan->guard1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "jail" ) )
    {
	clan->jail = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "guard2" ) )
    {
	clan->guard2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "trooper1" ) )
    {
	clan->trooper1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "trooper2" ) )
    {
	clan->trooper2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !strcmp( arg2, "patrol1" ) )
    {
	clan->patrol1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "patrol2" ) )
    {
	clan->patrol2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
    {
	do_setclan( ch, "" );
	return;
    }

    if ( !strcmp( arg2, "type" ) )
    {
        if ( clan->mainclan )
        {
            UNLINK ( clan , clan->mainclan->first_subclan, clan->mainclan->last_subclan, next_subclan, prev_subclan );
            clan->mainclan = NULL;
	}
	if ( !str_cmp( argument, "crime" ) )
	  clan->clan_type = CLAN_CRIME;
	else
	if ( !str_cmp( argument, "crime family" ) )
	  clan->clan_type = CLAN_CRIME;
	else
	if ( !str_cmp( argument, "guild" ) )
	  clan->clan_type = CLAN_GUILD;
	else
	  clan->clan_type = 0;
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "name" ) )
    {
	STRFREE( clan->name );
	clan->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !strcmp( arg2, "filename" ) )
    {
	DISPOSE( clan->filename );
	clan->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	write_clan_list( );
	return;
    }

    if ( !strcmp( arg2, "desc" ) )
    {
	STRFREE( clan->description );
	clan->description = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    do_setclan( ch, "" );
    return;
}

void do_setplanet( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PLANET_DATA *planet;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setplanet <planet> <field> [value]\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " base_value flags\n\r", ch );
	send_to_char( " name filename starsystem governed_by hospital\n\r", ch );
	send_to_char( " lowgang highgang\n\r", ch);
	send_to_char( " supply supply_rate demand demand_rate\n\r", ch);
	send_to_char( " contraband_supply contraband_supply_rate security\n\r", ch);
	return;
    }

    planet = get_planet( arg1 );
    if ( !planet )
    {
	send_to_char( "No such planet.\n\r", ch );
	return;
    }


    if ( !strcmp( arg2, "name" ) )
    {
	STRFREE( planet->name );
	planet->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "hospital") )
    {
        planet->hospital = atoi(argument);
        send_to_char( "Done.\n\r", ch );
        save_planet( planet );
        return;
    }
    if ( !strcmp( arg2, "lowgang"))
    {
	planet->lowgang = atoi(argument);
	send_to_char("Done\n\r", ch);
	save_planet( planet );
	return;
    }
    if ( !strcmp( arg2, "highgang"))
    {
	planet->highgang = atoi(argument);
	send_to_char("Done\n\r", ch);
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "governed_by" ) )
    {
        CLAN_DATA *clan;
        clan = get_clan( argument );
        if ( clan )
        {
           planet->governed_by = clan;
           send_to_char( "Done.\n\r", ch );
       	   save_planet( planet );
        }
        else
           send_to_char( "No such clan.\n\r", ch );
	return;
    }

    if ( !strcmp( arg2, "starsystem" ) )
    {
        SPACE_DATA *starsystem;

        if ((starsystem=planet->starsystem) != NULL)
          UNLINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
	if ( (planet->starsystem = starsystem_from_name(argument)) )
        {
           starsystem = planet->starsystem;
           LINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
           send_to_char( "Done.\n\r", ch );
	}
	else
	       	send_to_char( "No such starsystem.\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "filename" ) )
    {
	DISPOSE( planet->filename );
	planet->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	write_planet_list( );
	return;
    }

    if ( !strcmp( arg2, "base_value" ) )
    {
	planet->base_value = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "flags" ) )
    {
        char farg[MAX_INPUT_LENGTH];

        argument = one_argument( argument, farg);

        if ( farg[0] == '\0' )
        {
           send_to_char( "Possible flags: nocapture\n\r", ch );
           return;
        }

        for ( ; farg[0] != '\0' ; argument = one_argument( argument, farg) )
        {
            if ( !str_cmp( farg, "nocapture" ) )
               TOGGLE_BIT( planet->flags, PLANET_NOCAPTURE );
            else
               ch_printf( ch , "No such flag: %s\n\r" , farg );
	}
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	return;
    }

// Trade stuff
    if ( !strcmp( arg2, "supply" ) )
    {
		planet->supply = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }
    if ( !strcmp( arg2, "supply_rate" ) )
    {
		planet->supply_rate = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }
    if ( !strcmp( arg2, "demand" ) )
    {
		planet->demand = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }
    if ( !strcmp( arg2, "demand_rate" ) )
    {
		planet->demand_rate = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }
    if ( !strcmp( arg2, "contraband_supply" ) )
    {
		planet->contraband_supply = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }
    if ( !strcmp( arg2, "contraband_supply_rate" ) )
    {
		planet->contraband_supply_rate = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }
    if ( !strcmp( arg2, "security" ) )
    {
		planet->security = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_planet( planet );
		return;
    }

    do_setplanet( ch, "" );
    return;
}

void do_showclan( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showclan <clan>\n\r", ch );
	return;
    }

    clan = get_clan( argument );
    if ( !clan )
    {
	send_to_char( "No such clan.\n\r", ch );
	return;
    }

    ch_printf( ch, "%s      : %s\n\rFilename: %s\n\r",
			clan->clan_type == CLAN_CRIME ? "Crime Family " :
			    clan->clan_type == CLAN_GUILD ? "Guild " : "Organization ",
    			clan->name,
    			clan->filename);
    ch_printf( ch, "Description: %s\n\rImmortal: %s\n\rLeader: %s\n\r",
    			clan->description,
    			clan->immortal,
    			clan->leader );
    ch_printf( ch, "Number1: %s\n\rNumber2: %s\n\rPKills: %6d    PDeaths: %6d\n\r",
    			clan->number1,
    			clan->number2,
    			clan->pkills,
    			clan->pdeaths );
    ch_printf( ch, "MKills: %6d    MDeaths: %6d\n\r",
    			clan->mkills,
    			clan->mdeaths );
    ch_printf( ch, "Type: %d\n\r",
    			clan->clan_type );
    ch_printf( ch, "Members: %3d\n\r",
    			clan->members );
    ch_printf( ch, "Board: %6d    Jail: %6d\n\r",
    			clan->board, clan->jail);
    ch_printf( ch, "Guard1: %6d    Guard2: %6d\n\r",
    			clan->guard1,
    			clan->guard2 );
    ch_printf( ch, "Patrol1: %6d    Patrol2: %6d\n\r",
    			clan->patrol1,
    			clan->patrol2 );
    ch_printf( ch, "Trooper1: %6d    Trooper2: %6d\n\r",
    			clan->trooper1,
    			clan->trooper2 );
    ch_printf( ch, "Funds: %lld\n\r",
    			clan->funds );
    ch_printf( ch, "Support Vnum: %d\n\r",
			clan->supportvnum);
    ch_printf( ch, "War1: %s	War2: %s  \n\r",
		(clan->war1)? clan->war1->name : "None",
		(clan->war2)? clan->war2->name : "None" );
    ch_printf( ch, "Ally1: %s	Ally2: %s  \n\r",
		(clan->ally1)? clan->ally1->name : "None",
		(clan->ally2)? clan->ally2->name : "None");
	ch_printf( ch, "WarKills: %d \n\r", clan->warkills ); /* added for capship GRP on 2006-08-06 by Funf */
    return;
}

void do_showplanet( CHAR_DATA *ch, char *argument )
{
    PLANET_DATA *planet;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showplanet <planet>\n\r", ch );
	return;
    }

    planet = get_planet( argument );
    if ( !planet )
    {
	send_to_char( "No such planet.\n\r", ch );
	return;
    }

    ch_printf( ch, "%s\n\rFilename: %s\tBase Value: %d\tPopulation: %d\n\r",
    			planet->name,
    			planet->filename,
    			planet->base_value,
    			planet->population);
	ch_printf( ch, "Citizen Count: %d\tFlags: %s\tStarsystem: %s\n\r",
				planet->citizencount,
    			planet->flags?"NoCapture":"None",
    			planet->starsystem ? planet->starsystem->name : "None"
    			);
	ch_printf( ch, "Hospital: %d\tLowGang: %d\tHighGang: %d\n\r",
    			planet->hospital,
    			planet->lowgang,
    			planet->highgang);
	if(planet->governed_by)
		ch_printf( ch, "Governed By: %s\tPopular Support: %d\n\r",
				planet->governed_by->name,
				planet->pop_support);
	if(planet->frustrated_by)
		ch_printf( ch, "Frustrated By: %s\tFrustration Level: %f\n\r",
				planet->frustrated_by->name,
				planet->frust_level);
	ch_printf( ch, "Trade Goods: S=%d\tCS=%d\tD=%d\n\r",
				planet->supply,
				planet->contraband_supply,
				planet->demand);
	ch_printf( ch, "Trade Rate: SR=%d\tCSR=%d\tDR=%d\n\r",
				planet->supply_rate,
				planet->contraband_supply_rate,
				planet->demand_rate);
	ch_printf( ch, "Security: %d%%",
				planet->security);
    return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    CLAN_DATA *clan;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeclan <clan name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

    CREATE( clan, CLAN_DATA, 1 );
    LINK( clan, first_clan, last_clan, next, prev );
    clan->next_subclan = NULL;
    clan->prev_subclan = NULL;
    clan->last_subclan = NULL;
    clan->first_subclan = NULL;
    clan->mainclan     = NULL;
    clan->name		= STRALLOC( argument );
    clan->description	= STRALLOC( "" );
    clan->immortal	= STRALLOC( "" );
    clan->leader	= STRALLOC( "" );
    clan->number1	= STRALLOC( "" );
    clan->number2	= STRALLOC( "" );
    clan->tmpstr	= STRALLOC( "" );
    clan->war1		= NULL;
    clan->war2		= NULL;
    clan->ally1		= NULL;
    clan->ally2		= NULL;
}

void do_makeplanet( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    PLANET_DATA *planet;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeplanet <planet name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", PLANET_DIR, strlower(argument) );

    CREATE( planet, PLANET_DATA, 1 );
    LINK( planet, first_planet, last_planet, next, prev );
    planet->governed_by = NULL;
    planet->frustrated_by = NULL;
    planet->next_in_system = NULL;
    planet->prev_in_system = NULL;
    planet->starsystem = NULL ;
    planet->first_area = NULL;
    planet->last_area = NULL;
    planet->first_guard = NULL;
    planet->last_guard = NULL;
    planet->name		= STRALLOC( argument );
    planet->flags               = 0;
}

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    PLANET_DATA *planet;
    int count = 0;
    int pCount = 0;
    int support;
    long long int revenue;

    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN )
          continue;

        pCount = 0;
        support = 0;
        revenue = 0;

        for ( planet = first_planet ; planet ; planet = planet->next )
          if ( clan == planet->governed_by )
          {
            support += planet->pop_support;
            pCount++;
            revenue += get_taxes(planet);
          }

        if ( pCount > 1 )
           support /= pCount;

        ch_printf( ch, "&COrganization&R: &W%s\n\r",
                   clan->name);
        ch_printf( ch, "  &CPlanets&R: &W%-2d       &CAvg Pop Support&R: &W%-3d&W    &CRevenue&R: &W%-10ld\n\r",
                   pCount,support,revenue);
        ch_printf( ch, "  &CDivision      Leaders                 Spacecraft&R/&CVehicles&R/&CMembers&R/&CFunds\n\r");
        ch_printf( ch, "  &O%-15s %-10s %-10s %-10s %-10s%-2d %-2d %-3d %s\n\r",
                  "&CMain",clan->immortal,  clan->leader, clan->number1, clan->number2, clan->spacecraft,
			clan->vehicles, clan->members, num_punct(clan->funds) );
	ch_printf( ch, "  &CAt War With&R: &W%s%s%s%s.\n\r",
		(clan->war1)? clan->war1->name : "",
                (clan->war1 && clan->war2)? " and " : "",
                (clan->war2)? clan->war2->name : "",
		(!clan->war1 && !clan->war2)? "No one" : "");
	ch_printf( ch, "  &CCurrently Allies With&R:&W %s%s%s%s.\n\r",
                        (clan->ally1)? clan->ally1->name : "",
                        (clan->ally1 && clan->ally2)? " and " : "",
                        (clan->ally2)? clan->ally2->name : "",
			(!clan->ally1 && !clan->ally2)? "No one" : "");
/*
	ch_printf( ch, "  &CWar 1 Counter&R:&W %d	&CWar 2 Counter&R:&W %d\r\n",
		clan->war1counter, clan->war2counter);
*/
        if ( clan->first_subclan )
        {
           CLAN_DATA *subclan;

           for ( subclan = clan->first_subclan ; subclan ; subclan = subclan->next_subclan )
               ch_printf( ch, "  &c%-20s %-10s %-10s %-10s %-2d %d %-3s %s\n\r",
                  subclan->name, subclan->leader, subclan->number1, subclan->number2, subclan->spacecraft, subclan->vehicles, subclan->members,num_punct(subclan->funds) );
        }
        ch_printf( ch , "\n\r" );
        count++;
    }

    ch_printf( ch, "&CAutonomous Groups      Leaders             Spacecraft&R/&CVehicles&R/&CMembers&R/&CFunds&W\n\r");
    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type != CLAN_CRIME )
          continue;

        ch_printf( ch, "&C%-22s &c%-10s %-10s %-10s %-2d %-2d %-3d %s\n\r",
                  clan->name, clan->leader, clan->number1, clan->number2, clan->spacecraft, clan->vehicles, clan->members, num_punct(clan->funds) );
        count++;
    }

    if ( !count )
    {
	set_char_color( AT_BLOOD, ch);
        send_to_char( "There are no organizations currently formed.\n\r", ch );
    }

    set_char_color( AT_WHITE, ch );
    send_to_char( "\n\r&CSee also&R: &B(&WPlanets&B)&W\n\r", ch );

}

void do_newclans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    PLANET_DATA *planet;
    int count = 0;
    int pCount = 0;
    int support;
    long long int revenue;

    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN )
          continue;

        pCount = 0;
        support = 0;
        revenue = 0;

        for ( planet = first_planet ; planet ; planet = planet->next )
          if ( clan == planet->governed_by )
          {
            support += planet->pop_support;
            pCount++;
            revenue += get_taxes(planet);
          }

        if ( pCount > 1 )
           support /= pCount;

        ch_printf( ch, "&gOrganization&r: &G%s\n\r",
                   clan->name);
        ch_printf( ch, "  &BPlanets&r: &z%-2d	&BAvg Pop Support&r: &z%-3d&z\n\r",
                   pCount,support);

	if(ch->pcdata->clan == clan || IS_IMMORTAL(ch))
	{
        	ch_printf( ch, "  &BOfficers&r:&z %-8s %-8s %-8s %-8s\r\n",
                	   clan->immortal,clan->leader, clan->number1, clan->number2);
		ch_printf( ch, "  &BSpacecrafts Owned&r: &z%-2d&O    &BFunds&r: &z%-3s   &BRevenue&r:&z%-10ld \r\n",
				 clan->spacecraft, num_punct(clan->funds), revenue);

		ch_printf( ch, "  &BAt War With&r: &z%s%s%s%s.\n\r",
                (clan->war1)? clan->war1->name : "",
                (clan->war1 && clan->war2)? " and " : "",
                (clan->war2)? clan->war2->name : "",
                (!clan->war1 && !clan->war2)? "No one" : "");

		if(clan->war1 && clan->war1->name)
		{
			if(clan->war1->war1 && clan->war1->war1 == clan)
			{
				ch_printf(ch, "  &BWar with %s status&r: &z%s\r\n",
					clan->war1->name,
					(clan->war1counter == clan->war1->war1counter)? "Tied" :
					(clan->war1counter > clan->war1->war1counter)? "Winning" : "Losing");
			}

			if(clan->war1->war2 && clan->war1->war2 == clan)
			{
                                ch_printf(ch, "  &BWar with %s status&r: &z%s\r\n",
                                        clan->war1->name,
                                        (clan->war1counter == clan->war1->war2counter)? "Tied" :
                                        (clan->war1counter > clan->war1->war2counter)? "Winning" : "Losing");
			}

		}

                if(clan->war2 && clan->war2->name)
                {
                        if(clan->war2->war1 && clan->war2->war1 == clan)
                                ch_printf(ch, "  &BWar with %s status&r: &z%s\r\n",
                                        clan->war2->name,
                                        (clan->war2counter == clan->war2->war1counter)? "Tied" :
                                        (clan->war2counter > clan->war2->war1counter)? "Winning" : "Losing");
                        if(clan->war2->war2 && clan->war2->war2 == clan)
                                ch_printf(ch, "  &BWar with %s status&r: &z%s\r\n",
                                        clan->war2->name,
                                        (clan->war2counter == clan->war2->war2counter)? "Tied" :
                                        (clan->war2counter > clan->war2->war2counter)? "Winning" : "Losing");
                }

	        ch_printf( ch, "  &BCurrently Allies With&r:&z %s%s%s%s.\n\r",
                        (clan->ally1)? clan->ally1->name : "",
                        (clan->ally1 && clan->ally2)? " and " : "",
                        (clan->ally2)? clan->ally2->name : "",
                        (!clan->ally1 && !clan->ally2)? "No one" : "");
	}
        /* We don't have sub clans - Gatz
	if ( clan->first_subclan )
        {
           CLAN_DATA *subclan;

           for ( subclan = clan->first_subclan ; subclan ; subclan = subclan->next_subclan)
               ch_printf( ch, "  &O%-20s %-10s %-10s %-10s\n\r",
                  subclan->name, subclan->leader, subclan->number1, subclan->number2);

	}
*/
        ch_printf( ch , "\n\r" );
        count++;
    }

    ch_printf( ch, "&gAutonomous Groups&r:             \n\r");

    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type != CLAN_CRIME )
          continue;
	ch_printf( ch, "  &G%s\n\r",
        	clan->name);

        count++;
    }

    if ( !count )
    {
        set_char_color( AT_BLOOD, ch);
        send_to_char( "There are no organizations currently formed.\n\r", ch );
    }

   // set_char_color( AT_BLOOD, ch );
    send_to_char( "\n\r&BSee also&w&W&r: &B(&zPlanets&B)&z\n\r", ch );

}


void do_planets( CHAR_DATA *ch, char *argument )
{
    PLANET_DATA *planet;
    int count = 0;
    AREA_DATA   *area;
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );

    set_char_color( AT_WHITE, ch );
    for ( planet = first_planet; planet; planet = planet->next )
    {
		if(!str_cmp(arg1, "") || nifty_is_name_prefix(arg1, planet->name))
		{
			ch_printf( ch, "&CPlanet&R: &W%-15s    &CGoverned By&R: &W%s %s\n\r",
					   planet->name ,
					   planet->governed_by ? planet->governed_by->name : "",
					   IS_SET(planet->flags, PLANET_NOCAPTURE ) ? "(permanent)" : "" );
			ch_printf( ch, "&CValue&R: &W%-9lld&R/&W%-9lld ",
					   get_taxes(planet) , planet->base_value);
			ch_printf( ch, "&CPopulation&R: &W%-5d    &CPop Support&R: &W%.1f%\n\r",
					   planet->population , planet->pop_support );
			ch_printf( ch, "&CFrustrated By&R: &W%-5s       &CTolerance Level&R: &W%.1f %%\n\r",
			 (planet->frustrated_by)? planet->frustrated_by->name: "None",
			 (100.00 - planet->frust_level));
			ch_printf( ch, "&CSupply&R: &W%-5d              &CDemand&R: &W%-5d\n\r",
					   planet->supply , planet->demand );
			if ( IS_IMMORTAL(ch) )
			{
			  ch_printf( ch, "&CAreas&R: &W");
			  for ( area = planet->first_area ; area ; area = area->next_on_planet )
				 ch_printf( ch , "%s&R,&W  ", area->filename );
			  ch_printf( ch, "\n\r" );
			}
			ch_printf( ch, "\n\r" );
		}

        count++;
    }

    if ( !count )
    {
	set_char_color( AT_BLOOD, ch);
        send_to_char( "There are no planets currently formed.\n\r", ch );
    }

}

void update_planettrade()
{
	// Loop through each planet and update the current tradegood levels by the rate.
	PLANET_DATA *planet;

	for ( planet = first_planet; planet; planet = planet->next )
	{
		planet->supply += planet->supply_rate;
		planet->contraband_supply += planet->contraband_supply_rate;
		planet->demand += planet->demand_rate;
		if(planet->supply > 20) planet->supply = 20;
		if(planet->contraband_supply > 10) planet->contraband_supply = 10;
		if(planet->demand > 20) planet->demand = 20;
	}

}

void do_orders( CHAR_DATA *ch, char *argument )
{
}

void do_giveplanet(CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int pop;
	PLANET_DATA *planet;
	PLANET_DATA *cplanet;
	CLAN_DATA *clan;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if ( !ch->pcdata->clan )
	{
	  send_to_char( "Huh?\n\r", ch );
	  return;
	}

	if ( str_cmp( ch->name, ch->pcdata->clan->leader ) && str_cmp( ch->name, ch->pcdata->clan->immortal ) )
	{
	  send_to_char( "Only your leader can do that!\r\n", ch );
	  return;
	}

	if ( arg1[0] == '\0' )
	{
	  send_to_char( "Usage: giveplanet <planet> <clan>\n\r", ch );
	  return;
	}

	if ( arg2[0] == '\0' )
	{
	  send_to_char( "Usage: giveplanet <planet> <clan>\n\r", ch );
	  return;
	}

	planet = get_planet ( arg1 );
	if ( !planet )
	{
	   send_to_char( "No such planet.\n\r", ch );
	   return;
	}

	clan = get_clan (arg2);
	if ( !clan )
	{
	   send_to_char( "No such clan.\n\r", ch );
	   return;
	}

	if(clan->clan_type == 1)
	{
	   send_to_char( "You can't give planets to organizations!\n\r", ch );
	}

	if(planet->governed_by != ch->pcdata->clan)
	{
	   send_to_char( "You can't give what you don't own!\n\r", ch );
	}

	if(planet->governed_by == ch->pcdata->clan)
	{
	  planet->governed_by = clan;
	  pop = planet->pop_support*-1;
	  planet->pop_support = pop;
	  planet->frustrated_by = NULL;
	  planet->frust_level = 0;

	  sprintf( buf , "Control of %s has been given to %s by %s.",
planet->name, clan->name, ch->pcdata->clan->name);
	  echo_to_all( AT_RED , buf , 0 );

	  save_planet( planet );
	}
}

void do_guilds( CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *clan;
    PLANET_DATA *planet;
    int count = 0;
    int pCount = 0;
    int support;
    long long int revenue;

    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_PLAIN || clan->clan_type == CLAN_SUBCLAN )
          continue;

        pCount = 0;
        support = 0;
        revenue = 0;

        for ( planet = first_planet ; planet ; planet = planet->next )
          if ( clan == planet->governed_by )
          {
            support += planet->pop_support;
            pCount++;
            revenue += get_taxes(planet);
          }

        if ( pCount > 1 )
           support /= pCount;

        ch_printf( ch, "&CIndependent Group&R: &W%s\n\r",
                   clan->name);
        ch_printf( ch, "  &CPlanets&R: &W%-2d       &CAvg Pop Support&R: &W%-3d&W    &CRevenue&R: &W%-10ld\n\r",
                   pCount,support,revenue);
        ch_printf( ch, "  &CDivision             Leaders             Spacecraft&R/&CVehicles&R/&CMembers&R/&CFunds\n\r");
        ch_printf( ch, "  &O%-20s %-10s %-10s %-10s %-2d %-2d %-3d %lld\n\r",
                  "&CMain", clan->leader, clan->number1, clan->number2, clan->spacecraft, clan->vehicles, clan->members, clan->funds );
        if ( clan->first_subclan )
        {
           CLAN_DATA *subclan;

           for ( subclan = clan->first_subclan ; subclan ; subclan = subclan->next_subclan )
               ch_printf( ch, "  &c%-20s %-10s %-10s %-10s %-2d %d %-3d %lld\n\r",
                  subclan->name, subclan->leader, subclan->number1, subclan->number2, subclan->spacecraft, subclan->vehicles, subclan->members, subclan->funds );
        }
        ch_printf( ch , "\n\r" );
        count++;
    }
}

void do_shove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    bool nogo;
    ROOM_INDEX_DATA *to_room;
    int chance;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );


    if ( arg[0] == '\0' )
    {
	send_to_char( "Shove whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You shove yourself around, to no avail.\n\r", ch);
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	send_to_char("You can't shove from this room!\n\r", ch);
	return;
    }

    if ( (victim->position) != POS_STANDING )
    {
	act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Shove them in which direction?\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );
    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&  get_timer(victim, TIMER_SHOVEDRAG) <= 0)
    {
	send_to_char("That character cannot be shoved right now.\n\r", ch);
	return;
    }
    victim->position = POS_SHOVE;
    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
        victim->position = POS_STANDING;
	return;
    }
    to_room = pexit->to_room;

    if ( IS_NPC(victim) )
    {
	send_to_char("You can only shove player characters.\n\r", ch);
	return;
    }

    if (ch->in_room->area != to_room->area
    &&  !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
      victim->position = POS_STANDING;
      return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
		if ( ch->adren < 5) ch->adren++;
		if ( victim->adren < 5) victim->adren++;
	}

chance = 50;

/* Add 3 points to chance for every str point above 15, subtract for
below 15 */

chance += ((get_curr_str(ch) - 15) * 3);

chance += (ch->top_level - victim->top_level);

/* Debugging purposes - show percentage for testing */

/* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/

if (chance < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
    act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
    move_char( victim, get_exit(ch->in_room,exit_dir), 0);
    if ( !char_died(victim) )
      victim->position = POS_STANDING;
    WAIT_STATE(ch, 12);
    /* Remove protection from shove/drag if char shoves -- Blodkai */
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    &&   get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    bool nogo;
    int chance;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drag whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char("You can only drag player characters.\n\r", ch);
	return;
    }

    if ( victim->fighting )
    {
        send_to_char( "You try, but can't get close enough.\n\r", ch);
        return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Drag them in which direction?\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );

    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&   get_timer( victim, TIMER_SHOVEDRAG ) <= 0)
    {
	send_to_char("That character cannot be dragged right now.\n\r", ch);
	return;
    }

    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
	return;
    }

    to_room = pexit->to_room;

    if (ch->in_room->area != to_room->area
    && !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
      victim->position = POS_STANDING;
      return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_PLR_HOME) )
    {
		if ( ch->adren < 5) ch->adren++;
		if ( victim->adren < 5) victim->adren++;
	}

    chance = 50;


/*
sprintf(buf, "Drag percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/
if (chance < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    if ( victim->position < POS_STANDING )
    {
	sh_int temp;

	temp = victim->position;
	victim->position = POS_DRAG;
	act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT );
	move_char( victim, get_exit(ch->in_room,exit_dir), 0);
	if ( !char_died(victim) )
	  victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
	move_char( ch, get_exit(ch->in_room,exit_dir), 0);
	WAIT_STATE(ch, 12);
	return;
    }
    send_to_char("You cannot do that to someone who is standing.\n\r", ch);
    return;
}

void do_enlist( CHAR_DATA *ch, char *argument )
{

        CLAN_DATA *clan;
	BOUNTY_DATA *bounty;

	if ( IS_NPC(ch) || !ch->pcdata )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

        if ( ch->pcdata->clan )
        {
            ch_printf( ch , "You will have to resign from %s before you can join a new organization.\n\r", ch->pcdata->clan->name );
            return;
        }


        if ( IS_SET( ch->in_room->room_flags , ROOM_R_RECRUIT ) )
           clan = get_clan( "RBH" );
        else if ( IS_SET( ch->in_room->room_flags , ROOM_E_RECRUIT ) )
           clan = get_clan( "GLM" );
        else
        {
            send_to_char( "You don't seem to be in a recruitment office.\n\r", ch );
	    return;
        }

        if ( !clan )
        {
            send_to_char( "They don't seem to be recruiting right now.\n\r", ch );
	    return;
        }

	bounty = get_bounty( ch->name);
	if( bounty != NULL)
	{
		ch_printf(ch,"They don't allow dangerous criminals in %s!", clan->name);
		return;
	}

        SET_BIT( ch->speaks, LANG_CLAN );
        ++clan->members;

	STRFREE( ch->pcdata->clan_name );
	ch->pcdata->clan_name = QUICKLINK( clan->name );
	ch->pcdata->clan = clan;
	ch_printf( ch, "Welcome to %s.\n\r", clan->name );

        save_clan ( clan );
	return;

}

void do_resign( CHAR_DATA *ch, char *argument)
{

    CLAN_DATA *clan;
    long long int lose_exp, xplose;
    char buf[MAX_STRING_LENGTH];

	lose_exp = xplose = 0;

    if ( IS_NPC(ch) || !ch->pcdata )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

    clan =  ch->pcdata->clan;

    if ( clan == NULL )
    {
	    send_to_char( "You have to join an organization before you can quit it.\n\r", ch );
	    return;
	}

    if ( !str_cmp( ch->name, ch->pcdata->clan->leader ) )
    {
    	ch_printf( ch, "You can't resign from %s ... you are the leader!\n\r", clan->name );
        return;
    }

    if ( !str_cmp( ch->name, ch->pcdata->clan->immortal ) )
    {
    	ch_printf( ch, "You can't resign from %s ... you are the immortal!\n\r", clan->name );
        return;
    }
   // GLM Loses Engineering EXP - Gatz

  if( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") )  )
   {
	xplose = UMAX( ch->experience[ENGINEERING_ABILITY]/10 , 0 );
    	ch->experience[ENGINEERING_ABILITY] -= xplose;
   }


    if ( ch->speaking & LANG_CLAN )
      ch->speaking = LANG_COMMON;
    REMOVE_BIT( ch->speaks, LANG_CLAN );
    --clan->members;
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
    act( AT_MAGIC, "You resign your position in $t", ch, clan->name, NULL , TO_CHAR );
/*    sprintf(buf, "%s has quit %s!", ch->name, clan->name);
    echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL); */

    lose_exp = UMAX( ch->experience[DIPLOMACY_ABILITY] - exp_level( ch->skill_level[DIPLOMACY_ABILITY]  ) , 0 );

    ch_printf( ch, "You lose %lld diplomacy experience.\n\r", lose_exp );
    ch->experience[DIPLOMACY_ABILITY] -= lose_exp;

    if( xplose > 0  )
    {
	ch_printf( ch, "You lose %lld engineering experience.\n\r", xplose );
    }

    DISPOSE( ch->pcdata->bestowments );
    ch->pcdata->bestowments = str_dup("");
    STRFREE( ch->pcdata->setclantalk_name);
    ch->pcdata->setclantalk_name = STRALLOC( "" );
    ch->pcdata->setclantalk = NULL;

    save_char_obj( ch );	/* clan gets saved when pfile is saved */

    return;

}

void do_clan_withdraw( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    long long int amount;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "You don't seem to belong to an organization to withdraw funds from...\n\r", ch );
	return;
    }

    if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
       send_to_char( "You must be in a bank to do that!\n\r", ch );
       return;
    }

    if ( (ch->pcdata && ch->pcdata->bestowments && is_name("withdraw", ch->pcdata->bestowments) )
    ||   !str_cmp( ch->name, ch->pcdata->clan->immortal  )
    ||   !str_cmp( ch->name, ch->pcdata->clan->leader  ) )
	;
    else
    {
   	send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability." ,ch );
   	return;
    }

    clan = ch->pcdata->clan;

    amount = atoll( argument );

    if ( !amount )
    {
	send_to_char( "How much would you like to withdraw?\n\r", ch );
	return;
    }

    if ( amount > clan->funds )
    {
	ch_printf( ch,  "%s doesn't have that much!\n\r", clan->name );
	return;
    }

    if ( amount < 0 )
    {
	ch_printf( ch,  "Nice try...\n\r" );
	return;
    }

    ch_printf( ch,  "You withdraw %lld wulongs from %s's funds.\n\r",
amount, clan->name );

    clan->funds -= amount;
    ch->gold += amount;
    save_clan ( clan );

}


void do_clan_donate( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    long long int amount;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "You don't seem to belong to an organization to donate to...\n\r", ch );
	return;
    }

    if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
       send_to_char( "You must be in a bank to do that!\n\r", ch );
       return;
    }

    clan = ch->pcdata->clan;

    amount = atoll( argument );

    if ( !amount )
    {
	send_to_char( "How much would you like to donate?\n\r", ch );
	return;
    }

    if ( amount < 0 )
    {
	ch_printf( ch,  "Nice try...\n\r" );
	return;
    }

    if ( amount > ch->gold )
    {
	send_to_char( "You don't have that much!\n\r", ch );
	return;
    }

    ch_printf( ch,  "You donate %lld wulongs to %s's funds.\n\r", amount,
clan->name );

    clan->funds += amount;
    ch->gold -= amount;
    save_clan ( clan );

}

void do_newclan ( CHAR_DATA *ch , char *argument )
{
	send_to_char( "This command is being recycled to conserve thought.\n\r", ch );
	return;
}

void do_appoint ( CHAR_DATA *ch , char *argument )
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( IS_NPC( ch ) || !ch->pcdata )
      return;

    if ( !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (  str_cmp( ch->name, ch->pcdata->clan->leader ) && str_cmp( ch->name, ch->pcdata->clan->immortal ) )
    {
	send_to_char( "Only your leader can do that!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Useage: appoint <name> < first | second >\n\r", ch );
	return;
    }

    if ( !str_cmp( argument , "first" )  )
    {
    	if ( ch->pcdata->clan->number1 && str_cmp( ch->pcdata->clan->number1 , "" ) )
        {
        	send_to_char( "You already have someone in that position ... demote them first.\n\r", ch );
	    	return;
	    }

        STRFREE( ch->pcdata->clan->number1 );
        ch->pcdata->clan->number1 = STRALLOC( capitalize(arg) );
    }
    else if ( !str_cmp( argument , "second" )  )
    {
        if ( ch->pcdata->clan->number2 && str_cmp( ch->pcdata->clan->number2 , "" ))
        {
        	send_to_char( "You already have someone in that position ... demote them first.\n\r", ch );
	    	return;
	 	}

        STRFREE( ch->pcdata->clan->number2 );
        ch->pcdata->clan->number2 = STRALLOC(capitalize(arg) );
    }
    else do_appoint( ch , "" );
    save_clan ( ch->pcdata->clan );

}

void do_demote ( CHAR_DATA *ch , char *argument )
{

    if ( IS_NPC( ch ) || !ch->pcdata )
      return;

    if ( !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (  str_cmp( ch->name, ch->pcdata->clan->leader ) && str_cmp( ch->name, ch->pcdata->clan->immortal ) )
    {
	send_to_char( "Only your leader can do that!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Demote who?\n\r", ch );
	return;
    }

    if ( !str_cmp( argument , ch->pcdata->clan->number1 )  )
    {
         send_to_char( "Player Demoted!", ch );

         STRFREE( ch->pcdata->clan->number1 );
         ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    else if ( !str_cmp( argument , ch->pcdata->clan->number2 )  )
    {
         send_to_char( "Player Demoted!", ch );

         STRFREE( ch->pcdata->clan->number2 );
         ch->pcdata->clan->number2 = STRALLOC( "" );
    }
    else
    {
	send_to_char( "They seem to have been demoted already.\n\r", ch );
	return;
    }
    save_clan ( ch->pcdata->clan );

}

void do_capture ( CHAR_DATA *ch , char *argument )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   PLANET_DATA *cPlanet;
   float support = 0.0;
   int pCount = 0;
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   char bufc[MAX_STRING_LENGTH];

   if ( !ch->in_room || !ch->in_room->area)
      return;

   if ( IS_NPC(ch) || !ch->pcdata )
   {
       send_to_char ( "huh?\n\r" , ch );
       return;
   }

   if ( !ch->pcdata->clan )
   {
       send_to_char ( "You need to be a member of an organization to do that!\n\r" , ch );
       return;
   }

   if ( ch->pcdata->clan->mainclan )
      clan = ch->pcdata->clan->mainclan;
   else
      clan = ch->pcdata->clan;

   if ( !str_cmp( clan->name, "RBH" ) || clan->clan_type == CLAN_GUILD )
   {
       send_to_char ( "Bounty hunters can't control planets!\n\r" , ch );
       return;
   }

   if ( ch->pcdata && ch->pcdata->clan && ( !str_cmp(ch->pcdata->clan->name, "GLM") ) )
   {
       send_to_char ( "GLM members can't control planets! Stick to being a merchant!\n\r", ch);
       return;
   }
   if ( clan->clan_type == CLAN_GUILD )
   {
       send_to_char ( "Your organization serves a much greater purpose.\n\r" , ch );
       return;
   }



   if ( ( planet = ch->in_room->area->planet ) == NULL )
   {
       send_to_char ( "You must be on a planet to capture it.\n\r" , ch );
       return;
   }

   if ( clan == planet->governed_by )
   {
       send_to_char ( "Your organization already controls this planet.\n\r" , ch );
       return;
   }

   if ( IS_SET( planet->flags, PLANET_NOCAPTURE ) )
   {
       send_to_char ( "This planet cannot be captured.\n\r" , ch);
       return;
   }

   if ( planet->pop_support > 0 )
   {
       send_to_char ( "The population is not in favour of changing leaders right now.\n\r" , ch );
       return;
   }

   for ( cPlanet = first_planet ; cPlanet ; cPlanet = cPlanet->next )
        if ( clan == cPlanet->governed_by )
        {
            pCount++;
            support += cPlanet->pop_support;
        }

   if (planet->frustrated_by == ch->pcdata->clan && planet->frust_level > 50)
   {
       send_to_char ( "The population is too frustrated with your organization!\n\r", ch );
       return;
   }

   if ( support < 0 )
   {
       send_to_char ( "There is not enough popular support for your organization!\n\rTry improving loyalty on the planets that you already control.\n\r" , ch );
       return;
   }
/*
   // Gaffiti for clans! - Gatz
	if(str_cmp(ch->pcdata->clan->name, "ISSP"))
	{
		planet = ch->in_room->area->planet;
		if(planet && planet->governed_by && planet->governed_by->name)
		{
                obj = create_object( get_obj_index( OBJ_VNUM_GRAFFITI ), 0 );
                obj->timer      = number_range( 1, 10 );
                STRFREE( obj->description );
		switch(number_range(0,5))
		{
			case 5:
				sprintf(bufc, "'%s suks! %s forever!' has been spray painted here.",
					ch->pcdata->clan->name,  planet->governed_by->name);
			break;
			case 4:
				sprintf(bufc, "'%s is a pack of dogs! %s will never die!' has been spray painted here.",
					ch->pcdata->clan->name, planet->governed_by->name);
			break;
			case 3:
				sprintf(bufc, "'%s are jerks!' has been spray painted here.",
					ch->pcdata->clan->name);
			break;
			case 2:
				sprintf(bufc, "'%s will nevar die!' has been spray painted here.",
					planet->governed_by->name);
			break;
			case 1:
				sprintf(bufc, "'%s stinks! They are losers!' has been spray painted here.",
					ch->pcdata->clan->name);
			break;
			default:
				sprintf(bufc, "'%s will be back losers!' has been spray painted here.",
					planet->governed_by->name);
		}

		obj->description = STRALLOC( bufc );
        	obj_to_room( obj, get_room_index(number_range(planet->lowgang, planet->highgang)));
		}
	}
*/

   planet->governed_by = clan;
   planet->pop_support = 50;
   planet->frustrated_by = NULL;
   planet->frust_level = 0;

   sprintf( buf , "%s has been captured by %s!", planet->name, clan->name );
   echo_to_all( AT_RED , buf , 0 );

   save_planet( planet );

   return;
}

void do_empower ( CHAR_DATA *ch , char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("withdraw", ch->pcdata->bestowments))
    || !str_cmp( ch->name, clan->immortal  )
    || !str_cmp( ch->name, clan->leader  ) )
	;
    else
    {
	send_to_char( "You clan hasn't seen fit to bestow that ability to you!\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Empower whom to do what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	    send_to_char( "Nice try.\n\r", ch );
	    return;
    }

    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
	    send_to_char( "This player does not belong to your clan!\n\r", ch );
	    return;
    }

    if (!victim->pcdata->bestowments)
      victim->pcdata->bestowments = str_dup("");

    if ( arg2[0] == '\0' || !str_cmp( arg2, "list" ) )
    {
        ch_printf( ch, "Current bestowed commands on %s: %s.\n\r",
                      victim->name, victim->pcdata->bestowments );
        return;
    }

    if ( !str_cmp( arg2, "none" ) )
    {
        DISPOSE( victim->pcdata->bestowments );
	victim->pcdata->bestowments = str_dup("");
        ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
        ch_printf( victim, "%s has removed your bestowed clan abilities.\n\r", ch->name );
        return;
    }
    else if ( !str_cmp( arg2, "pilot" ) )
    {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to fly clan ships.\n\r",
             ch->name );
      send_to_char( "Ok, they now have the ability to fly clan ships.\n\r", ch );
    }
    else if ( !str_cmp( arg2, "withdraw" ) )
    {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to withdraw clan funds.\n\r",
             ch->name );
      send_to_char( "Ok, they now have the ablitity to withdraw clan funds.\n\r", ch );
    }
    else if ( !str_cmp( arg2, "clanbuyship" ) )
    {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to buy clan ships.\n\r",
             ch->name );
      send_to_char( "Ok, they now have the ablitity to use clanbuyship.\n\r", ch );
    }
    else if ( !str_cmp( arg2, "induct" ) )
    {
      sprintf( buf, "%s %s", victim->pcdata->bestowments, arg2 );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      ch_printf( victim, "%s has given you permission to induct new members.\n\r",
             ch->name );
      send_to_char( "Ok, they now have the ablitity to induct new members.\n\r", ch );
    }
    else
    {
      send_to_char( "Currently you may empower members with only the following:\n\r", ch );
      send_to_char( "\n\rpilot:       ability to fly clan ships\n\r", ch );
      send_to_char(     "withdraw:    ability to withdraw clan funds\n\r", ch );
      send_to_char(     "clanbuyship: ability to buy clan ships\n\r", ch );
      send_to_char(     "induct:      ability to induct new members\n\r", ch );
      send_to_char(     "none:        removes bestowed abilities\n\r", ch );
    }

    save_char_obj( victim );	/* clan gets saved when pfile is saved */
    return;


}

void save_senate( )
{
/*
    BOUNTY_DATA *tbounty;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", SYSTEM_DIR, BOUNTY_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
         bug( "FATAL: cannot open bounty.lst for writing!\n\r", 0 );
         return;
    }
    for ( tbounty = first_bounty; tbounty; tbounty = tbounty->next )
    {
        fprintf( fpout, "%s\n", tbounty->target );
        fprintf( fpout, "%lld\n", tbounty->amount );
    }
    fprintf( fpout, "$\n" );
    fclose( fpout );
*/
}

void load_senate( )
{
    first_senator = NULL;
    last_senator = NULL;
/*
    FILE *fpList;
    char *target;
    char bountylist[256];
    BOUNTY_DATA *bounty;
    long long int amount;

    first_bounty = NULL;
    last_bounty	= NULL;

    first_disintigration = NULL;
    last_disintigration	= NULL;

    log_string( "Loading disintigrations..." );

    sprintf( bountylist, "%s%s", SYSTEM_DIR, DISINTIGRATION_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( bountylist, "r" ) ) == NULL )
    {
	perror( bountylist );
	exit( 1 );
    }

    for ( ; ; )
    {
        target = feof( fpList ) ? "$" : fread_word( fpList );
        if ( target[0] == '$' )
        break;
	CREATE( bounty, BOUNTY_DATA, 1 );
        LINK( bounty, first_disintigration, last_disintigration, next, prev );
	bounty->target = STRALLOC(target);
	amount = fread_number( fpList );
	bounty->amount = amount;
    }
    fclose( fpList );
    log_string(" Done bounties " );
    fpReserve = fopen( NULL_FILE, "r" );

    return;
*/
}

void do_senate( CHAR_DATA *ch, char *argument )
{
/*
    GOV_DATA *gov;
    int count = 0;

    set_char_color( AT_WHITE, ch );
    send_to_char( "\n\rGoverning Area                 Controlled By             Value\n\r", ch );
    for ( gov = first_gov; gov; gov = gov->next )
    {
        set_char_color( AT_YELLOW, ch );
        ch_printf( ch, "%-30s %-25s %-15ld\n\r", gov->name, gov->controlled_by , gov->value );
        count++;
    }

    if ( !count )
    {
        set_char_color( AT_GREY, ch );
        send_to_char( "There are no governments to capture at this time.\n\r", ch );
	return;
    }
*/
}

void do_addsenator( CHAR_DATA *ch , char *argument )
{
/*
    GOVE_DATA *gov;

    CREATE( gov, GOV_DATA, 1 );
    LINK( gov, first_gov, last_gov, next, prev );

    gov->name		= STRALLOC( argument );
    gov->value          = atoi( arg2 );
    gov->vnum           = object;
    gov->controlled_by  = STRALLOC( "" );

    ch_printf( ch, "OK, making %s.\n\r", argument );
    save_govs();
*/
}

void do_remsenator( CHAR_DATA *ch , char *argument )

{
/*
	UNLINK( bounty, first_bounty, last_bounty, next, prev );
	STRFREE( bounty->target );
	DISPOSE( bounty );

	save_bounties();
*/
}

long long int get_taxes( PLANET_DATA *planet )
{
      long long int gain;

      gain = planet->base_value;
      gain += planet->base_value*planet->pop_support/100;
      gain += UMAX(0, planet->pop_support/10 * planet->population);

      return gain;
}

/*
    (link)->prev		= (insert)->prev;
    if ( !(insert)->prev )
      (first)			= (link);
    else
      (insert)->prev->next	= (link);
    (insert)->prev		= (link);
    (link)->next		= (insert);
*/


   // Generates supports for a clan, this is a mix of building/coding - Gatz
void support_update()
{
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    MOB_INDEX_DATA *pMobIndex;
    PLANET_DATA *planet, *cplanet;
    sh_int vnum, value, count;
    bool checkplanet;
   // The Band-Aid sort of broke, because of war counters - Gatz
   /*
    for( clan = first_clan; clan; clan = clan->next)
    {
	if(clan->war1 && !clan->war1->war1 && clan != clan->war1->war2)
	{
		clan->war1->war1 = clan;
		continue;
	}
	if(clan->war2 && !clan->war2->war2 && clan != clan->war2->war1)
	{
		clan->war2->war2 = clan;
		continue;
	}
	if(clan->war1
	   && (!clan->war1->war1 || !clan->war1->war2))
	{
		if(!clan->war1->war1 && clan->war1->war2 != clan)
		{
			clan->war1->war1 = clan;
			continue;
		}
		if(!clan->war1->war2 && clan->war1->war1 != clan)
		{
			clan->war1->war2 = clan;
			continue;
		}
	}
        if(clan->war2
           && (!clan->war2->war1 || !clan->war2->war2))
        {
                if(!clan->war2->war1 && clan->war2->war2 != clan)
                {
                        clan->war2->war1 = clan;
                        continue;
                }
                if(!clan->war2->war2 && clan->war2->war1 != clan)
                {
                        clan->war2->war2 = clan;
                        continue;
                }
        }
        if(clan->war1 == clan->war2)
		clan->war2 = NULL;
	if(clan->war1 && clan->war1->war1 && clan->war1->war2
	    && clan->war1->war1 == clan->war1->war2)
		clan->war1->war2 = NULL;
	if(clan->war2 && clan->war2->war1 && clan->war2->war2
	   && clan->war2->war1 == clan->war2->war2)
		clan->war2->war2 = NULL;

    }
*/
    // ACH! Band-aid for some clans having a bug saving this clan data - Gatz
/*
    for( clan = first_clan; clan; clan = clan->next)
    {
        if(clan->ally1 && !clan->ally1->ally1 && clan->ally1->ally2 != clan)
        {
                clan->ally1->ally1 = clan;
                continue;
        }
        if(clan->ally2 && !clan->ally2->ally2 && clan->ally2->ally1 != clan)
        {
                clan->ally2->ally2 = clan;
                continue;
        }
        if(clan->ally1
           && (!clan->ally1->ally1 || !clan->ally1->ally2))
        {
                if(!clan->ally1->ally1 && clan->ally1->ally2 != clan)
                {
                        clan->ally1->ally1 = clan;
                        continue;
                }
                if(!clan->ally1->ally2 && clan->ally1->ally1 != clan)
                {
                        clan->ally1->ally2 = clan;
                        continue;
                }
        }
        if(clan->ally2
           && (!clan->ally2->ally1 || !clan->ally2->ally2))
        {
                if(!clan->ally2->ally1 && clan->ally2->ally2 != clan)
                {
                        clan->ally2->ally1 = clan;
                        continue;
                }
                if(!clan->ally2->ally2 && clan->ally2->ally1 != clan)
                {
                        clan->ally2->ally2 = clan;
                        continue;
                }
        }
        if(clan->ally1 == clan->ally2)
                clan->ally2 = NULL;
        if(clan->ally1 && clan->ally1->ally1 && clan->ally1->ally2
            && clan->ally1->ally1 == clan->ally1->ally2)
                clan->ally1->ally2 = NULL;
        if(clan->ally2 && clan->ally2->ally1 && clan->ally2->ally2
           && clan->ally2->ally1 == clan->ally2->ally2)
                clan->ally2->ally2 = NULL;

    }
*/

    for ( planet = first_planet; planet; planet = planet->next )
    {
	/* -Removed frustration giving a planet to the people they were frustrated at... DER. -Spike
	if(planet->frustrated_by && planet->frustrated_by->name
	   && planet->frust_level == 100
	   && planet->governed_by)
	{
		char buf[MAX_STRING_LENGTH];

  		sprintf( buf , "%s has thrown %s out of power in favor of %s!!",
			planet->name, planet->governed_by->name, planet->frustrated_by->name );
   		echo_to_all( AT_RED , buf , 0 );
		planet->governed_by = planet->frustrated_by;
		planet->frustrated_by = NULL;
		planet->frust_level = 0;
		planet->pop_support = 50;
		save_planet( planet );
	}
	if(planet->frustrated_by && planet->frustrated_by->name
	   && planet->governed_by && planet->frust_level > 0
	   && planet->pop_support > 80)
	{
		planet->frust_level -= .1;
		if(planet->frust_level <= 0)
		{
			planet->frust_level = 0;
			planet->frustrated_by = NULL;
		}
	}
	*/
        checkplanet = FALSE;
        for ( clan = first_clan; clan; clan = clan->next )
        {
	        if ( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN )
        		continue;
		vnum = 0;
                if(clan == planet->governed_by)
                {
     		//        vnum = clan->supportvnum;
			if(!strcmp(clan->name, "BSS"))
				vnum = 26;
			if(!strcmp(clan->name, "ISSP"))
				vnum = 28;
			if(!strcmp(clan->name, "RDS"))
				vnum = 27;
                        checkplanet = TRUE;
                        break;
                }
        }
        if(clan != planet->governed_by)
                continue;
	count = 0;
	for ( cplanet = first_planet; cplanet; cplanet = cplanet->next )
 	{
		if(clan == cplanet->governed_by)
			count++;
	}

        if(!checkplanet)
                continue;

        if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
        {
                bug("Update Clan Support: No Mobile found.");
		continue;
        }
        if(pMobIndex->count < (count * 3)) // Changed from 15 to 3 to be in line with police. 2009-02-27 Funf
        {
        	victim = create_mobile( pMobIndex );
        	value = planet->pop_support/2 + number_range(5,15);
		if(value < 5)
			value = 5;
        	victim->top_level = value;
        	victim->armor = 100-value*2.5;
        	victim->hitroll = value/5;
        	victim->damroll = value/5;
        	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
        	{
        	  victim->pIndexData->level = value;
        	  victim->pIndexData->ac = 100-value*2.5;
        	  victim->pIndexData->hitroll = victim->hitroll;
        	  victim->pIndexData->damroll = victim->damroll;
        	}
        	REMOVE_BIT(victim->act, ACT_PROTOTYPE);
		SET_BIT(victim->act, ACT_SUPPORTER);
        	char_to_room( victim, get_room_index(number_range(planet->lowgang, planet->highgang)));
	}
    }

    // Generator some Clan Rebels! Planetary Frustration style. - Gatz
    for ( planet = first_planet; planet; planet = planet->next )
    {
        checkplanet = FALSE;
        for ( clan = first_clan; clan; clan = clan->next )
        {
                if ( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN)
                        continue;
                vnum = 0;
                if(clan == planet->frustrated_by && planet->frust_level > 50)
                {
                        vnum = clan->supportvnum;
                        checkplanet = TRUE;
                        break;
                }
        }

        if(clan != planet->governed_by)
                continue;

        if(!checkplanet)
                continue;

        if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
        {
                bug("Update Planetary Frustration: No Mobile found.");
                continue;
        }

        victim = create_mobile( pMobIndex );
        value = planet->frust_level - number_range(1,10);
        if(value < 5)
        	value = 5;
        victim->top_level = value;
        victim->armor = 100-value*2.5;
        victim->hitroll = value/5;
        victim->damroll = value/5;
        if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
        {
                victim->pIndexData->level = value;
                victim->pIndexData->ac = 100-value*2.5;
                victim->pIndexData->hitroll = victim->hitroll;
                victim->pIndexData->damroll = victim->damroll;
        }
        REMOVE_BIT(victim->act, ACT_PROTOTYPE);
        SET_BIT(victim->act, ACT_SUPPORTER);
        char_to_room( victim, get_room_index(number_range(planet->lowgang, planet->highgang)));

    }

    return;
}



/*
 *  This is a large CPU hog, not as bad as say Mob_update,
 *  but I suggest never letting the refresh rate be faster
 *  than 20 minutes or so - Gatz
 */
void citizen_generate()
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    MOB_INDEX_DATA *pMobIndex;
    sh_int spin, turn, ovnum, count, vnum, value;
    AREA_DATA *area;
    PLANET_DATA *planet;
    ROOM_INDEX_DATA *room;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    sh_int loc, stat;
    int mod;




    for ( planet = first_planet; planet; planet = planet->next )
    {
        count = 0;
	for ( area = planet->first_area ; area ; area = area->next_on_planet )
	{
		if(area->generatecit == 1)
			count++;
	}

	// Basicly forces each area to balance out the citizens it has.
	// planet->citizencount should be 0 at boot time but that is it!
	if(planet->citizencount <= 0)
	{
	// The Number which count divides is the number of citizens a planet can have
		spin = 80/count;
		if(count > 2);
			spin *= 2;
		if(count > 4)
			spin *= 2;
	}
	else
	{
		// For every citizen that dies, the citizen count goes up 1 in fight.c
		// Then, it basicly allows this program to replace any citizen killed.
		spin = planet->citizencount/count;
		if(spin < 0)
			spin = 1;
		planet->citizencount = 0;
	}
	if(spin < 1)
		spin = 1;
	if(count == 0)
		spin = 0;

	for( area = planet->first_area ; area ; area = area->next_on_planet )
	{
		if(area->generatecit == 0)
			continue;

		// Generate X number of Citizens in Area - Gatz
		for(turn = 1; turn <= spin; turn++)
		{
			vnum = 35;
			// Emergency Stop, if no Mobile is found the whole system can't work - Gatz
        		if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
        		{
        		        bug("Citizen Generator: No Mobile found.");
        		        break;
        		}
			room = NULL;
			/* Generate start room, which is randomize out of all the rooms
			*  an area has, however, if a room is set to nomob the generator
			*  will pick a new room and keep cycling till it finds one
			*/
			while(room == NULL)
			{
				// Vnum's use CHANGES to be used for where the mobile generates.
				vnum = number_range(area->low_r_vnum, area->hi_r_vnum);
				room = get_room_index(vnum);
				// This is so the room it places the Mob in is real
				if(room && room->name
				   && !str_cmp( room->name, "Floating in a void"))
					room = NULL;
			}

			victim = NULL;
	        	victim = create_mobile( pMobIndex );
	        	value = number_range(5, 100);
			// The newbie zone Dayton City gets weaker mobs by default
	        	if(!str_cmp(area->name, "Dayton City") && value > 50)
	        		value = number_range(1,50);
			victim->sex = number_range(1,2);
	        	victim->top_level = value;
	        	victim->armor = 100-value*2.5;
	        	victim->hitroll = value/5;
	        	victim->damroll = value/5;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
				stat = 8;
			victim->perm_str = stat;
                        stat = number_range((value/4 - 5), value/4);
                        if(stat < 8)
                                stat = 8;
			victim->perm_dex = stat;
                        stat = number_range((value/4 - 5), value/4);
                        if(stat < 8)
                                stat = 8;
			victim->perm_con = stat;
                        stat = number_range((value/4 - 5), value/4);
                        if(stat < 8)
                                stat = 8;
			victim->perm_wis = stat;
                        stat = number_range((value/4 - 5), value/4);
                        if(stat < 8)
                                stat = 8;
			victim->perm_lck = stat;
                        stat = number_range((value/4 - 5), value/4);
                        if(stat < 8)
                                stat = 8;
			victim->perm_cha = stat;
                        stat = number_range((value/4 - 5), value/4);
                        if(stat < 8)
                                stat = 8;
			victim->perm_int = stat;
			STRFREE(victim->name);
			sprintf(buf, "%s Citizen", planet->name);
			victim->name = STRALLOC( buf );
			sprintf(buf, "A %s Citizen", planet->name);
			STRFREE(victim->short_descr);
			victim->short_descr = STRALLOC( buf );
			generate_name(victim);
		//	sprintf(buf, "A mad wild dog!\r\n");
		//	victim->long_descr = STRALLOC( buf );
	        	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	        	{
	        	        victim->pIndexData->level = value;
	        	        victim->pIndexData->ac = 100-value*2.5;
	       		        victim->pIndexData->hitroll = victim->hitroll;
	                	victim->pIndexData->damroll = victim->damroll;
	        	}
       			REMOVE_BIT(victim->act, ACT_PROTOTYPE);
        		SET_BIT(victim->act, ACT_CITIZEN);

			if(number_range(0,4) > 2 && value > 30)
			{
				ovnum = 0;
				if(number_range(1,10) == 10
				   && area->rareeq != 0)
				{
					ovnum = area->rareeq;
				}
				else
					if(area->loweq != 0 && area->higheq != 0)
						ovnum = number_range(area->loweq, area->higheq);
			}
			if ( ovnum != 0 && ( pObjIndex = get_obj_index( ovnum ) ) == NULL )
    			{
  				bug("Citizen Generator: Object Error!");
        			return;
    			}
			if(ovnum != 0)
			{
				obj = create_object( pObjIndex, 1 );
				if(number_range(0,10) > 8 && value > 50)
				{
					if(number_range(1,3) == 3)
					{
						switch(number_range(0,5))
						{
							case 5:
								loc = APPLY_STR;
							break;
							case 4:
								loc = APPLY_DEX;
							break;
							case 3:
								loc = APPLY_WIS;
							break;
							case 2:
								loc = APPLY_DAMROLL;
							break;
							case 1:
								loc = APPLY_HITROLL;
							break;
							default:
								loc = APPLY_INT;
							break;
						}
						if(value > 80)
							mod = value/25;
						else
							mod = 1;
						CREATE( paf, AFFECT_DATA, 1 );
        					paf->type               = -1;
        					paf->duration           = -1;
        					paf->location           = loc;
        					paf->modifier           = mod;
        					paf->bitvector          = 0;
        					paf->next               = NULL;
        					if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        						  LINK( paf, obj->pIndexData->first_affect,
                					     obj->pIndexData->last_affect, next, prev );
        					else
       						   LINK( paf, obj->first_affect, obj->last_affect, next, prev );
        					++top_affect;
					}
					obj = obj_to_char( obj, victim );
				}
			}

        		char_to_room( victim, room);


		}
	}
	// If no citizen's are killed on a planet, the checker would assume
	// that this is firing for the first time and generate a large
	// number of citizens. This just makes sure it doesn't.
	planet->citizencount++;
    }
    return;
}

/*
 * The fun begins here! This is used for making long
 * desc's for citizens, but in reality can be used for
 * any long desc mobile generation task needed - Gatz
 */

void generate_name( CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];


	if(!IS_NPC(victim) || !IS_SET(victim->act, ACT_PROTOTYPE))
		return;

	sprintf( buf, "A");

	switch(number_range(0,33))
	{
		case 33:
			strcat(buf, " tan");
		break;
		case 32:
			strcat(buf, " pale");
		break;
		case 31:
			strcat(buf, " lanky");
		break;
		case 30:
			strcat(buf, " shifty");
		break;
		case 29:
			strcat(buf, " scrubby");
		break;
		case 28:
			strcat(buf, " trashy");
		break;
		case 27:
			strcat(buf, " flashy");
		break;
		case 26:
			strcat(buf, " slow");
		break;
		case 25:
			strcat(buf, " brawny");
		break;
		case 24:
			strcat(buf, " nerdy");
		break;
		case 23:
			strcat(buf, " sexy");
		break;
		case 22:
			strcat(buf, " bright");
		break;
		case 21:
			strcat(buf, " stout");
		break;
		case 20:
			strcat(buf, " sickly");
		break;
		case 19:
			strcat(buf, " lean");
		break;
		case 18:
			strcat(buf, " obnoxious");
		break;
		case 17:
			strcat(buf, " loud");
		break;
		case 16:
			strcat(buf, " chatty");
		break;
		case 15:
			strcat(buf, " boiserous");
		break;
		case 14:
			strcat(buf, " plain");
		break;
		case 13:
			strcat(buf, " bleak");
		break;
		case 12:
			strcat(buf, " sullen");
		break;
		case 11:
			strcat(buf, " cheery");
		break;
		case 10:
			strcat(buf, " chubby");
		break;
		case 9:
			strcat(buf, " short");
		break;
		case 8:
			strcat(buf, " funny looking");
		break;
		case 7:
			strcat(buf, " weird looking");
		break;
		case 6:
			strcat(buf, " smelly");
		break;
		case 5:
			strcat(buf, " large");
		break;
		case 4:
			strcat(buf, " skinny");
		break;
		case 3:
			strcat(buf, " tall");
		break;
		case 2:
			strcat(buf, " ugly");
		break;
		case 1:
			if(victim->sex == 1)
				strcat(buf, " handsome");
			else
				strcat(buf, " pretty");
		break;
		default:
		// Nothing case, no adjective added
		break;
	}

	switch(number_range(0,8))
	{
		case 8:
			if(victim->sex == 1)
				strcat(buf, " homeless man");
			else
				strcat(buf, " homeless woman");
		break;
		case 7:
			strcat(buf, " worker");
		break;
		case 6:
			if(victim->sex == 1)
				strcat(buf, " businesswoman");
			else
				strcat(buf, " businessman");
		break;
		case 5:
			if(victim->sex == 1)
				strcat(buf, " old man");
			else
				strcat(buf, " old woman");
		break;
		case 4:
			if(victim->sex == 1)
				strcat(buf, " young boy");
			else
				strcat(buf, " young girl");
		break;
		case 3:
			if(victim->sex == 1)
				strcat(buf, " man");
			else
				strcat(buf, " woman");
		break;
		case 2:
			if(victim->sex == 1)
				strcat(buf, " middle-aged man");
			else
				strcat(buf, " middle-aged woman");
		break;
		case 1:
			if(victim->sex == 1)
				strcat(buf, " teenage boy");
			else
				strcat(buf, " teenage girl");
		break;
		default:
			if(victim->sex == 1)
				strcat(buf, " adolescent boy");
			else
				strcat(buf, " adolescent girl");
		break;
	}
	switch(number_range(0,18))
	{
		case 18:
			strcat(buf, " is whistling");
		break;
		case 17:
			strcat(buf, " is humming a song");
		break;
		case 16:
			strcat(buf, " is pacing");
		break;
		case 15:
			strcat(buf, " is droning along");
		break;
		case 14:
			strcat(buf, " is jogging");
		break;
		case 13:
			strcat(buf, " is looking at the local sights");
		break;
		case 12:
			strcat(buf, " is causing a scene");
		break;
		case 11:
			strcat(buf, " is taking a break");
		break;
		case 10:
			strcat(buf, " is talking on a cell phone");
		break;
		case 9:
			strcat(buf, " is walking");
		break;
		case 8:
			strcat(buf, " is standing");
		break;
		case 7:
			strcat(buf, " is preaching");
		break;
		case 6:
			strcat(buf, " is begging");
		break;
		case 5:
			strcat(buf, " is reading a book");
		break;
		case 4:
			strcat(buf, " is looking around");
		break;
		case 3:
			strcat(buf, " is wearing wrinkled clothes");
		break;
		case 2:
			strcat(buf, " is smiling brightly");
		break;
		case 1:
			strcat(buf, " is grinning slightly");
		break;
		default:
			strcat(buf, " is not focused on the real world");
		break;
	}

	strcat(buf, " here.\r\n");
	STRFREE(victim->long_descr);
	victim->long_descr = STRALLOC( buf );

	return;
}


/*
 * Spiffy little code which generates up some fancy Po-Leese to protect your poor innoncents
 * - Gatz
 */

void generate_police()
{
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	PLANET_DATA *planet;
	sh_int planetcount, value, stat, name;
	CHAR_DATA *victim;
	MOB_INDEX_DATA *pMobIndex;

	planetcount = value = stat = name = 0; /* Initialize your variables please! --Funf 2000-02-23 */

	for (planet = first_planet; planet; planet = planet->next )
	{
		if(planet->name && planet->population > 0)
			planetcount++;
	}

	for ( planet = first_planet; planet; planet = planet->next )
	{
		if(!planet->name && planet->population <= 0)
			continue;
		if ( ( pMobIndex = get_mob_index( 30 ) ) == NULL )
		{
		   bug("Police Generator Update: No Mobile found.");
				continue;
       	}
		if(pMobIndex->count > (planetcount * 3))
			break;
		if(pMobIndex->count < (planetcount * 3))
		{
			victim = create_mobile( pMobIndex );
			value = number_range(10,70);
			victim->top_level = value;
			victim->armor = 100-value*2.5;
			victim->hitroll = value/5;
			victim->damroll = value/5;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
				stat = 8;
			victim->perm_str = stat;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			victim->perm_dex = stat;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			victim->perm_con = stat;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			victim->perm_wis = stat;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			victim->perm_lck = stat;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			victim->perm_cha = stat;
			stat = number_range((value/4 - 5), value/4);
			if(stat < 8)
					stat = 8;
			victim->perm_int = stat;
			victim->gold = value * 100;
			STRFREE(victim->name);
			STRFREE(victim->short_descr);
			STRFREE(victim->long_descr);
			name = value/10;
			if(name < 1)
				name = 1;
			if(name > 5)
				SET_BIT(victim->affected_by, AFF_DETECT_HIDDEN);
			switch(name)
			{
				default:
					sprintf(buf1, "low level cop");
					sprintf(buf2, "A Low Level Cop");
					sprintf(buf3, "A lowely cop stands here patroling.");
				break;
				case 2:
					sprintf(buf1, "average cop");
					sprintf(buf2, "An Average Cop");
					sprintf(buf3, "An Average Cop is standing here.");
				break;
				case 3:
					sprintf(buf1, "big cop");
					sprintf(buf2, "A Big Cop");
					sprintf(buf3, "A Big Cop stands here towering over people.");
				break;
				case 4:
					sprintf(buf1, "tough cop");
					sprintf(buf2, "A Tough Cop");
					sprintf(buf3, "A Tough Cop roams around looking mean.");
				break;
				case 5:
					sprintf(buf1, "police sargeant");
					sprintf(buf2, "A Police Sargeant");
					sprintf(buf3, "A Police Sargeant stands here yelling.");
				break;
				case 6:
					sprintf(buf1, "swat member");
					sprintf(buf2, "A Swat Team Member");
					sprintf(buf3, "A Swat Team Member roams the streets on assignment.");
				break;
				case 7:
					sprintf(buf1, "special agent");
					sprintf(buf2, "A Special Agent");
					sprintf(buf3, "A Special Agent of the Police is here.");
				break;
			}
			strcat(buf3, "\r\n");
			victim->name = STRALLOC(buf1);
			victim->short_descr = STRALLOC(buf2);
			victim->long_descr = STRALLOC(buf3);

			if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			{
			  victim->pIndexData->level = value;
			  victim->pIndexData->ac = 100-value*2.5;
			  victim->pIndexData->hitroll = victim->hitroll;
			  victim->pIndexData->damroll = victim->damroll;
			}
			value = get_vip_flag( planet->name );
			if ( value < 0 || value > 31 )
				bug( "Unknown flag: %s\n\r", planet->name );
			else
				TOGGLE_BIT( victim->vip_flags, 1 << value );
			REMOVE_BIT(victim->act, ACT_PROTOTYPE);
			if(!IS_SET(victim->act, ACT_PROTOTYPE))
			{
	/*			  if(
	!(get_room_index(number_range(planet->lowgang, planet->highgang))) );
				   {
					bug( "Mob Generation: Invalid lowgang/highgang on planet %s\n\r", planet->name );
				   }*/
								char_to_room( victim, get_room_index(number_range(planet->lowgang, planet->highgang)));
			}
       	} // end if
	} // end for
} // end function

/*
 * This is a fancy way of tailling up points score by clans,
 * and has support for allies aiding clans. - Gatz
 */
void add_point( CLAN_DATA *victor, CLAN_DATA *loser)
{
	// Following just weed out obvious cases which don't apply

	if(!victor || !loser || !victor->name || !loser->name)
		return;
	if(victor == loser)
		return;

	// First is the straightforeward case, with no allies involved
	if(loser->war1 == victor || loser->war2 == victor)
	{
		 if(loser->war1 == victor)
		 {
			if(victor->war1 == loser)
			{
				victor->war1counter++;
				if((victor->war1counter)%2 == 0)
				{
					if(loser->war1counter - 1 >= 0)
						loser->war1counter--;
				}
			}
			else if(victor->war2 == loser)
			{
				victor->war2counter++;
				if((victor->war2counter)%2 == 0)
				{
					if(loser->war1counter - 1 >= 0)
						loser->war1counter--;
				}
			}
		}
		else if(loser->war2 == victor)
		{
                        if(victor->war1 == loser)
                        {
                                victor->war1counter++;
                                if((victor->war1counter)%2 == 0)
                                {
                                        if(loser->war2counter - 1 >= 0)
                                                loser->war2counter--;
                                }
                        }
                        else if(victor->war2 == loser)
                        {
                                victor->war2counter++;
                                if((victor->war2counter)%2 == 0)
                                {
                                        if(loser->war2counter - 1 >= 0)
                                                loser->war2counter--;
                                }
                        }
                }
	}
	else if((loser->war1 && loser->war1->ally1 && loser->war1->ally1 == victor)
		|| (loser->war1 && loser->war1->ally2 && loser->war1->ally2 == victor)
		|| (loser->war2 && loser->war2->ally1 && loser->war2->ally1 == victor)
		|| (loser->war2 && loser->war2->ally2 &&  loser->war2->ally2 == victor))
	{
		// This is your ally case, Good Lord, this is gonna be a mess!
		// Every IF - check condition above will require a chunk of nested Ifs below
		if((loser->war1->ally1 && loser->war1->ally1 == victor)
			|| (loser->war1->ally2 && loser->war1->ally2 == victor))
		{
			if(loser->war1->war1 == loser)
			{
				loser->war1->war1counter++;
				if((loser->war1->war1counter)%2 == 0)
				{
					if(loser->war1counter - 1 >= 0)
						loser->war1counter--;
				}
			}
			else if(loser->war1->war2 == loser)
                        {
                                loser->war1->war2counter++;
                                if((loser->war1->war2counter)%2 == 0)
                                {
                                        if(loser->war2counter - 1 >= 0)
                                                loser->war2counter--;
                                }
                        }
		}
		else if((loser->war2->ally1 && loser->war2->ally1 == victor)
			 || (loser->war2->ally2 && loser->war2->ally2 == victor))
                {
                        if(loser->war2->war1 == loser)
                        {
                                loser->war2->war1counter++;
                                if((loser->war2->war1counter)%2 == 0)
                                {
                                        if(loser->war1counter - 1 >= 0)
                                                loser->war1counter--;
                                }
                        }
                        else if(loser->war2->war2 == loser)
                        {
                                loser->war2->war2counter++;
                                if((loser->war2->war2counter)%2 == 0)
                                {
                                        if(loser->war2counter - 1 >= 0)
                                                loser->war2counter--;
                                }
                        }
                }
	}
	save_clan(victor);
	save_clan(loser);
	check_winners();
		return;
}



void check_winners()
{
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];
    char log_buf [MAX_INPUT_LENGTH];

    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_CRIME || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_SUBCLAN )
          continue;
	// It will just over-write anything that might be left there.
	sprintf(buf, "clear");
	//Fixed a bug in below line, was == -Spike
	if(clan->war1counter >= 20 && clan->war1)
	{
		// Announce the winner!
		sprintf(buf, "After a rough war, %s has beaten %s!\r\n",
			clan->name, clan->war1->name);
		sprintf( log_buf, "The Clan %s has defeated the Clan %s!",
				clan->name, clan->war1->name);
		clan->warwoncount++;
		clan->war1->warlostcount++;
		clan->war1counter = 0;
		if(clan->war1->war1 == clan)
		{
			clan->war1->war1 = NULL;
			clan->war1->war1counter = 0;
		}
		if(clan->war1->war2 == clan);
		{
			clan->war1->war2 = NULL;
			clan->war1->war2counter = 0;
		}
		clan->war1 = NULL;
	}
	if(clan->war2counter >= 20 && clan->war2)
	{
		// Announce the winner!
		sprintf(buf, "The war between %s and %s is over! %s is the winner!",
			clan->name, clan->war1->name, clan->name);
		sprintf( log_buf, "The Clan %s has defeated the Clan %s!",
                                clan->name, clan->war1->name);
		clan->warwoncount++;
                clan->war2->warlostcount++;
		if(clan->war2->war1 == clan)
		{
			clan->war2->war1 = NULL;
			clan->war2->war1counter = 0;
		}
		if(clan->war2->war2 == clan)
		{
			clan->war2->war2 = NULL;
			clan->war2->war2counter = 0;
		}
		clan->war2counter = 0;
		clan->war2 = NULL;
	}
	if(strlen(buf) > 7)
	{
		echo_to_all(AT_RED, buf, 0);
		log_string( log_buf );
            	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
	}
	save_clan(clan);
    }
}


void do_addradio( CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg1);
	one_argument(argument, arg2);

	if(IS_NPC(ch))
	{
		send_to_char("Huh?\r\n", ch);
		return;
	}

	if(!ch->pcdata || !ch->pcdata->clan)
	{
		send_to_char("Huh?\r\n", ch);
		return;
	}

	if(!ch->pcdata->clan->leader || !ch->pcdata->clan->immortal)
	{
		send_to_char("Huh?\r\n", ch);
		return;
	}

	if(ch->pcdata->clan->leader != ch->name && ch->pcdata->clan->immortal != ch->name)
	{
		send_to_char("Huh?\r\n", ch);
		return;
	}

	if(arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("&RSyntax: Addradio <add/remove> <target>\r\n", ch);
		return;
	}

	if((victim = get_char_world(ch, arg2)) == NULL)
	{
		send_to_char("&RThat player isn't online!\r\n", ch);
		return;
	}

	if(IS_NPC(victim))
	{
		send_to_char("&RThat would be pointless!\r\n", ch);
		return;
	}
	if(!str_cmp(arg1, "add"))
	{
		if(victim->pcdata->clantalk1 && victim->pcdata->clantalk2)
		{
			send_to_char("&RThey can have no more added clan radio frequences!\r\n", ch);
			return;
		}

		if(!victim->pcdata->clantalk1)
		{
			STRFREE(victim->pcdata->clantalk1_name);
			victim->pcdata->clantalk1_name = QUICKLINK( ch->pcdata->clan->name);
			victim->pcdata->clantalk1 = ch->pcdata->clan;
		}
		else if(!victim->pcdata->clantalk2)
		{
			STRFREE(victim->pcdata->clantalk2_name);
			victim->pcdata->clantalk2_name = QUICKLINK(ch->pcdata->clan->name);
			victim->pcdata->clantalk2 = ch->pcdata->clan;
		}

		ch_printf( ch, "You allow %s to use your clan radio!\r\n", victim->name);
		ch_printf( victim, "You are now allowed to use %s clan radio!\r\n", ch->pcdata->clan->name);
		save_char_obj( victim );
		return;
	}
	if(!str_cmp(arg1, "remove"))
	{
		if(!victim->pcdata->clantalk1 && !victim->pcdata->clantalk2)
		{
			send_to_char("&RThey have no clan radio frequencys!\r\n", ch);
			return;
		}

		if( (!victim->pcdata->clantalk1 ||
			 (victim->pcdata->clantalk1 && victim->pcdata->clantalk1 != ch->pcdata->clan)
			) &&
			(!victim->pcdata->clantalk2 ||
			 (victim->pcdata->clantalk2 && victim->pcdata->clantalk2 != ch->pcdata->clan)
			)
		)
		{
			send_to_char("They don't have your clantalk radio frequency!\r\n", ch);
			return;
		}

		if(victim->pcdata->clantalk1 && victim->pcdata->clantalk1 == ch->pcdata->clan)
		{
			STRFREE(victim->pcdata->clantalk1_name);
			victim->pcdata->clantalk1_name = STRALLOC( "" );
			victim->pcdata->clantalk1 = NULL;
		}

		if(victim->pcdata->clantalk2 && victim->pcdata->clantalk2 == ch->pcdata->clan)
		{
			STRFREE(victim->pcdata->clantalk2_name);
			victim->pcdata->clantalk2_name = STRALLOC( "" );
			victim->pcdata->clantalk2 = NULL;
		}

		victim->pcdata->setclantalk = NULL;
		STRFREE(victim->pcdata->setclantalk_name);
		victim->pcdata->setclantalk_name = STRALLOC("");
		save_char_obj( victim );
		ch_printf( ch, "You can no longer use %s clan radio!\r\n", ch->pcdata->clan->name);
		ch_printf( ch, "You remove %s's ability to use your clan radio!\r\n", victim->name);
		return;
	}
}

void do_setclantalk( CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	bool check = FALSE;

	if(IS_NPC(ch))
		return;

	if(!ch->pcdata->clan && !ch->pcdata->clantalk1 && !ch->pcdata->clantalk2)
	{
		send_to_char("Huh?\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if(arg[0] == '\0')
	{
		send_to_char("&RThe current clan's radios you have access to: \r\n", ch);
		if(ch->pcdata->clan && ch->pcdata->clan->clan_type != 1)
			ch_printf(ch, "&W%s\r\n", ch->pcdata->clan->name);
		if(ch->pcdata->clantalk1)
			ch_printf(ch, "&W%s\r\n", ch->pcdata->clantalk1->name);
		if(ch->pcdata->clantalk2)
			ch_printf(ch, "&W%s\r\n", ch->pcdata->clantalk2->name);
		if(!ch->pcdata->clantalk1 && !ch->pcdata->clantalk2 && !ch->pcdata->clan
			|| (ch->pcdata->clan && ch->pcdata->clan->clan_type == 1))
			ch_printf(ch, "&WNone.\r\n");
		return;
	}

	clan = get_clan( arg );
	if(!clan || clan->clan_type == 1)
	{
		send_to_char("You can't set -that- clan for a clan frequency. \r\n", ch);
		return;
	}

	if(ch->pcdata->clan && ch->pcdata->clan == clan)
		check = TRUE;

	if (ch->pcdata->clantalk1 && ch->pcdata->clantalk1 == clan)
		check = TRUE;

	if (ch->pcdata->clantalk2 && ch->pcdata->clantalk2 == clan)
		check = TRUE;

	if(!check)
	{
		send_to_char("You are not allowed to use that clan frequency. \r\n", ch);
		return;
	}

	ch->pcdata->setclantalk = clan;
	STRFREE(ch->pcdata->setclantalk_name);
	ch->pcdata->setclantalk_name = QUICKLINK( clan->name);
	save_char_obj( ch );
	ch_printf(ch, "You set %s to be your clantalk frequency. \r\n", clan->name);
	return;
}

void do_members(CHAR_DATA *ch, char *argument)
{
	send_to_char("Clan members:\r\n", ch);
	return;
}
