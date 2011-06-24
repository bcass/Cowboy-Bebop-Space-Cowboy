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
*		                Space Module    			   *
****************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

SHIP_DATA * first_ship;
SHIP_DATA * last_ship;

MISSILE_DATA * first_missile;
MISSILE_DATA * last_missile;

SPACE_DATA * first_starsystem;
SPACE_DATA * last_starsystem;

int bus_pos =0;
int bus_planet =0;
int bus2_planet = 4;
int turbocar_stop =0;
int corus_shuttle =0;

#define MAX_STATION    10
#define MAX_BUS_STOP 10

#define STOP_PLANET     202

int     const   station_vnum [MAX_STATION] =
{
    215 , 216 , 217 , 218 , 219 , 220 , 221 ,222 , 223 , 224
};

char *  const   station_name [MAX_STATION] =
{
   "Menari Spaceport" , "Skydome Botanical Gardens" , "Grand Towers" ,
   "Grandis Mon Theater" , "Palace Station" , "Great Galactic Museum" ,
   "College Station" , "Holographic Zoo of Extinct Animals" ,
   "Dometown Station " , "Monument Plaza"
};

int     const   bus_vnum [MAX_BUS_STOP] =
{
    201 ,  21100 , 29001 , 28038 , 31872 , 1001 , 28613 , 3060 , 28247, 32297
};

char *  const   bus_stop [MAX_BUS_STOP+1] =
{
   "Coruscant",
   "Mon Calamari", "Adari", "Gamorr", "Tatooine" , "Honoghr" , /* "Ryloth", */
   "Kashyyyk", "Endor", "Byss", "Cloning Facilities", "Coruscant"  /* last should always be same as first */
};

/* local routines */
//void	fread_ship	args( ( SHIP_DATA *ship, FILE *fp ) ); // Moved to shipbuild.c - Funf
//bool	load_ship_file	args( ( char *shipfile ) );
sh_int get_ship_speed ( SHIP_DATA *ship );
void	write_ship_list	args( ( void ) );
void    fread_starsystem      args( ( SPACE_DATA *starsystem, FILE *fp ) );
bool    load_starsystem  args( ( char *starsystemfile ) );
void    write_starsystem_list args( ( void ) );
void    resetship args( ( SHIP_DATA *ship ) );
void    landship args( ( SHIP_DATA *ship, char *arg ) );
void    launchship args( ( SHIP_DATA *ship ) );
bool    land_bus args( ( SHIP_DATA *ship, int destination ) );
void    launch_bus args( ( SHIP_DATA *ship ) );
void    echo_to_room_dnr args( ( int ecolor , ROOM_INDEX_DATA *room ,  char *argument ) );
ch_ret drive_ship( CHAR_DATA *ch, SHIP_DATA *ship, EXIT_DATA  *exit , int fall );
bool    autofly(SHIP_DATA *ship);
bool is_facing( SHIP_DATA *ship , SHIP_DATA *target );
void sound_to_ship( SHIP_DATA *ship , char *argument );

/* from comm.c */
bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );

ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit );

void echo_to_room_dnr ( int ecolor , ROOM_INDEX_DATA *room ,  char *argument )
{
    CHAR_DATA *vic;

    if ( room == NULL )
    	return;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
	set_char_color( ecolor, vic );
	send_to_char( argument, vic );
    }
}


bool  land_bus( SHIP_DATA *ship, int destination )
{
    char buf[MAX_STRING_LENGTH];

    if ( !ship_to_room( ship , destination ) )
    {
       return FALSE;
    }
    echo_to_ship( AT_YELLOW , ship , "You feel a slight thud as the ship sets down on the ground.");
    ship->location = destination;
    ship->lastdoc = ship->location;
    ship->shipstate = SHIP_DOCKED;
    if (ship->starsystem)
        ship_from_starsystem( ship, ship->starsystem );
    sprintf( buf, "%s lands on the platform.", ship->name );
    echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
    sprintf( buf , "The hatch on %s opens." , ship->name);
    echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
    echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch opens." );
    ship->hatchopen = TRUE;
    return TRUE;
}

void    launch_bus( SHIP_DATA *ship )
{
      char buf[MAX_STRING_LENGTH];

      sprintf( buf , "The hatch on %s closes and it begins to launch." , ship->name);
      echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
      echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch slides shut." );
      ship->hatchopen = FALSE;
      extract_ship( ship );
      echo_to_ship( AT_YELLOW , ship , "The ship begins to launch.");
      ship->location = 0;
      ship->shipstate = SHIP_READY;
}

void update_bus( )
{
    SHIP_DATA *ship;
    SHIP_DATA *ship2;
    SHIP_DATA *target;
    int        destination;
    char       buf[MAX_STRING_LENGTH];

    ship = ship_from_cockpit( 529 );
    ship2 = ship_from_cockpit( 3500 );

    if ( ship == NULL && ship2 == NULL )
    	return;

    switch (bus_pos)
    {

       case 0:
            target = ship_from_hangar( bus_vnum[bus_planet] );
            if ( target != NULL && !target->starsystem )
            {
               sprintf( buf,  "An electronic voice says, 'Cannot land at %s ... it seems to have dissapeared.'", bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship , buf );
               bus_pos = 5;
            }

            target = ship_from_hangar( bus_vnum[bus2_planet] );
            if ( target != NULL && !target->starsystem )
            {
               sprintf( buf,  "An electronic voice says, 'Cannot land at %s ... it seems to have dissapeared.'", bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship2 , buf );
               bus_pos = 5;
            }

            bus_pos++;
            break;

       case 6:
            launch_bus ( ship );
            launch_bus ( ship2 );
            bus_pos++;
            break;

       case 7:
            echo_to_ship( AT_YELLOW , ship , "The ship lurches slightly as it makes the jump to lightspeed.");
            echo_to_ship( AT_YELLOW , ship2 , "The ship lurches slightly as it makes the jump to lightspeed.");
            bus_pos++;
            break;

       case 9:

            echo_to_ship( AT_YELLOW , ship , "The ship lurches slightly as it comes out of hyperspace..");
            echo_to_ship( AT_YELLOW , ship2 , "The ship lurches slightly as it comes out of hyperspace..");
            bus_pos++;
            break;

       case 1:
            destination = bus_vnum[bus_planet];
            if ( !land_bus( ship, destination ) )
            {
               sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'" , bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship , buf );
               echo_to_ship( AT_CYAN , ship , "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'");
            }
            else
            {
               sprintf( buf,  "An electronic voice says, 'Welcome to %s'" , bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship , buf);
               echo_to_ship( AT_CYAN , ship , "It continues, 'Please exit through the main ramp. Enjoy your stay.'");
            }
            destination = bus_vnum[bus2_planet];
            if ( !land_bus( ship2, destination ) )
            {
               sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'" , bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship2 , buf );
               echo_to_ship( AT_CYAN , ship2 , "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'");
            }
            else
            {
               sprintf( buf,  "An electronic voice says, 'Welcome to %s'" , bus_stop[bus2_planet] );
               echo_to_ship( AT_CYAN , ship2 , buf);
               echo_to_ship( AT_CYAN , ship2 , "It continues, 'Please exit through the main ramp. Enjoy your stay.'");
            }
            bus_pos++;
            break;

       case 5:
            sprintf( buf, "It continues, 'Next stop, %s'" , bus_stop[bus_planet+1] );
            echo_to_ship( AT_CYAN , ship , "An electronic voice says, 'Preparing for launch.'");
            echo_to_ship( AT_CYAN , ship , buf);
            sprintf( buf, "It continues, 'Next stop, %s'" , bus_stop[bus2_planet+1] );
            echo_to_ship( AT_CYAN , ship2 , "An electronic voice says, 'Preparing for launch.'");
            echo_to_ship( AT_CYAN , ship2 , buf);
            bus_pos++;
            break;

       default:
            bus_pos++;
            break;
    }

    if ( bus_pos >= 10 )
    {
       bus_pos = 0;
       bus_planet++;
       bus2_planet++;
    }

    if ( bus_planet >= MAX_BUS_STOP )
       bus_planet = 0;
    if ( bus2_planet >= MAX_BUS_STOP )
       bus2_planet = 0;

}

void move_ships( )
{
	SHIP_DATA *ship;
   	/*MISSILE_DATA *missile;
   	MISSILE_DATA *m_next;*/
   	SHIP_DATA *target;
   	float dx, dy, dz, change;
   	char buf[MAX_STRING_LENGTH];
   	CHAR_DATA *ch;
   	bool ch_found = FALSE;

   	/*for ( missile = first_missile; missile; missile = m_next )
   	{
    	m_next = missile->next;

        ship = missile->fired_from;
        target = missile->target;

        if ( target->starsystem && target->starsystem == missile->starsystem )
        {
        	if ( missile->mx < target->vx )
            	missile->mx += UMIN( missile->speed/5 , target->vx - missile->mx );
            else if ( missile->mx > target->vx )
                missile->mx -= UMIN( missile->speed/5 , missile->mx - target->vx );
            if ( missile->my < target->vy )
                missile->my += UMIN( missile->speed/5 , target->vy - missile->my );
            else if ( missile->my > target->vy )
                missile->my -= UMIN( missile->speed/5 , missile->my - target->vy );
            if ( missile->mz < target->vz )
                missile->mz += UMIN( missile->speed/5 , target->vz - missile->mz );
            else if ( missile->mz > target->vz )
                missile->mz -= UMIN( missile->speed/5 , missile->mz - target->vz );

            if ( abs(missile->mx) - abs(target->vx) <= 20 && abs(missile->mx) - abs(target->vx) >= -20
                 && abs(missile->my) - abs(target->vy) <= 20 && abs(missile->my) - abs(target->vy) >= -20
                 && abs(missile->mz) - abs(target->vz) <= 20 && abs(missile->mz) - abs(target->vz) >= -20 )
            {
            	if ( target->chaff_released <= 0)
                {
                	echo_to_room( AT_YELLOW , get_room_index(ship->gunseat), "Your missile hits its target dead on!" );
                    echo_to_cockpit( AT_BLOOD, target, "The ship is hit by a missile.");
                    echo_to_ship( AT_RED , target , "A loud explosion shakes thee ship violently!" );
                    sprintf( buf, "You see a small explosion as %s is hit by a missile" , target->name );
                    echo_to_system( AT_ORANGE , target , buf , ship );
                    for ( ch = first_char; ch; ch = ch->next )
                    	if ( !IS_NPC( ch ) && nifty_is_name( missile->fired_by, ch->name ) )
                      	{
                        	ch_found = TRUE;
                        	damage_ship_ch( target , 20+missile->missiletype*missile->missiletype*20 ,
                           	30+missile->missiletype*missile->missiletype*missile->missiletype*30 , ch );
                      	}
                    if ( !ch_found )
                    	damage_ship( target , 20+missile->missiletype*missile->missiletype*20 ,
                        			 30+missile->missiletype*missile->missiletype*ship->missiletype*30 );
                    extract_missile( missile );
                }
                else
                {
                    echo_to_room( AT_YELLOW , get_room_index(ship->gunseat), "Your missile explodes harmlessly in a cloud of chaff!" );
                    echo_to_cockpit( AT_YELLOW, target, "A missile explodes in your chaff.");
                    extract_missile( missile );
                }
            	continue;
        	}
        	else
        	{
        		missile->age++;
            	if (missile->age >= 50)
            	{
            		extract_missile( missile );
                	continue;
            	}

        	}

		}
    	else
    	{
    		extract_missile( missile );
        	continue;
    	}

	}*/
/* Ship movement */
	for ( ship = first_ship; ship; ship = ship->next )
   	{

    	if ( !ship->starsystem )
        	continue;
    	if ( ship->currspeed > 0 )
        {
        	if ( ship->vx == ship->hx && ship->vy == ship->hy && ship->vz == ship->hz )
        	{
				ship->currspeed = 0;
				echo_to_cockpit( AT_GREEN, ship, "The ship stops as it reaches its destination.");
				continue;
			}

        	change = sqrt( ship->hx*ship->hx + ship->hy*ship->hy + ship->hz*ship->hz );

        	if (change > 0)
           	{
            	dx = ship->hx/change;
            	dy = ship->hy/change;
            	dz = ship->hz/change;
            	ship->vx += (dx * get_ship_speed(ship) / 5);
            	ship->vy += (dy * get_ship_speed(ship) / 5);
            	ship->vz += (dz * get_ship_speed(ship) / 5);
           	}

        }

        if ( autofly(ship) )
        	continue;

        if ( ship->currspeed > 0 )
        {
        	if ( ship->starsystem->planet1 && strcmp(ship->starsystem->planet1,"") &&
                 abs(ship->vx - ship->starsystem->p1x) < 5 &&
                 abs(ship->vy - ship->starsystem->p1y) < 5 &&
                 abs(ship->vz - ship->starsystem->p1z) < 5 )
            {
            	sprintf( buf , "You begin orbitting %s.", ship->starsystem->planet1);
                echo_to_cockpit( AT_YELLOW, ship, buf);
                sprintf( buf , "%s begins orbiting %s.", ship->name, ship->starsystem->planet1);
                echo_to_system( AT_ORANGE , ship , buf , NULL );
                ship->currspeed = 0;
                continue;
            }
          	if ( ship->starsystem->planet2 && strcmp(ship->starsystem->planet2,"") &&
                 abs(ship->vx - ship->starsystem->p2x) < 5 &&
                 abs(ship->vy - ship->starsystem->p2y) < 5 &&
                 abs(ship->vz - ship->starsystem->p2z) < 5 )
            {
            	sprintf( buf , "You begin orbitting %s.", ship->starsystem->planet2);
                echo_to_cockpit( AT_YELLOW, ship, buf);
                sprintf( buf , "%s begins orbiting %s.", ship->name, ship->starsystem->planet2);
                echo_to_system( AT_ORANGE , ship , buf , NULL );
                ship->currspeed = 0;
                continue;
            }
            if ( ship->starsystem->planet3 && strcmp(ship->starsystem->planet3,"") &&
                 abs(ship->vx - ship->starsystem->p3x) < 5 &&
                 abs(ship->vy - ship->starsystem->p3y) < 5 &&
                 abs(ship->vz - ship->starsystem->p3z) < 5 )
            {
            	sprintf( buf , "You begin orbitting %s.", ship->starsystem->planet3);
                echo_to_cockpit( AT_YELLOW, ship, buf);
                sprintf( buf , "%s begins orbiting %s.", ship->name, ship->starsystem->planet3);
                echo_to_system( AT_ORANGE , ship , buf , NULL );
                ship->currspeed = 0;
                continue;
            }
            if ( ship->starsystem->planet4 && strcmp(ship->starsystem->planet4,"") &&
                 abs(ship->vx - ship->starsystem->p4x) < 5 &&
                 abs(ship->vy - ship->starsystem->p4y) < 5 &&
                 abs(ship->vz - ship->starsystem->p4z) < 5 )
            {
            	sprintf( buf , "You begin orbitting %s.", ship->starsystem->planet4);
                echo_to_cockpit( AT_YELLOW, ship, buf);
                sprintf( buf , "%s begins orbiting %s.", ship->name, ship->starsystem->planet4);
                echo_to_system( AT_ORANGE , ship , buf , NULL );
                ship->currspeed = 0;
                continue;
            }
        }
	}

   	for ( ship = first_ship; ship; ship = ship->next )
    	if (ship->collision)
       	{
        	echo_to_cockpit( AT_WHITE+AT_BLINK , ship,  "You have collided with another ship!" );
        	echo_to_ship( AT_RED , ship , "A loud explosion shakes the ship violently!" );
           	damage_ship( ship , ship->collision , ship->collision );
           	ship->collision = 0;
       	}
}

void recharge_ships( )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	for ( ship = first_ship; ship; ship = ship->next )
   	{
    	if (ship->statet0 > 0)
        {
        	ship->energy -= ship->statet0;
        	ship->statet0 = 0;
        }
        if (ship->statet1 > 0)
        {
        	ship->energy -= ship->statet1;
        	ship->statet1 = 0;
        }
        if (ship->statet2 > 0)
        {
        	ship->energy -= ship->statet2;
        	ship->statet2 = 0;
        }

        if (ship->missilestate == MISSILE_RELOAD_2)
        {
        	ship->missilestate = MISSILE_READY;
           	if ( ship->missiles > 0 )
            	echo_to_room( AT_YELLOW, get_room_index(ship->gunseat), "Missile launcher reloaded.");
        }

        if (ship->missilestate == MISSILE_RELOAD )
        {
           	ship->missilestate = MISSILE_RELOAD_2;
        }

        if (ship->missilestate == MISSILE_FIRED )
           ship->missilestate = MISSILE_RELOAD;

       	if ( autofly(ship) )
       	{
        	if ( ship->starsystem )
          	{
            	if (ship->target0 && ship->statet0 != LASER_DAMAGED )
             	{
                	int chance = 50;
                	SHIP_DATA * target = ship->target0;
                 	int shots;

                	for ( shots=0 ; shots <= ship->lasers ; shots++ )
                	{
                  		if (ship->shipstate != SHIP_HYPERSPACE && ship->energy > 25
                  		    && ship->target0->starsystem == ship->starsystem
                            && abs(target->vx - ship->vx) <= 1000
                            && abs(target->vy - ship->vy) <= 1000
                            && abs(target->vz - ship->vz) <= 1000
                            && ship->statet0 < ship->lasers )
                  		{
                    		if ( ship->class > MIDSIZE_SHIP || is_facing ( ship , target ) )
                    		{
             					chance += target->class*25;
             					chance -= target->manuever/10;
             					chance -= get_ship_speed(target)/20;
             					chance -= ( abs(target->vx - ship->vx)/70 );
             					chance -= ( abs(target->vy - ship->vy)/70 );
             					chance -= ( abs(target->vz - ship->vz)/70 );
             					chance = URANGE( 10 , chance , 90 );
             					if ( number_percent( ) > chance )
             					{
           		    				sprintf( buf , "%s fires at you but misses." , ship->name);
             		    			echo_to_cockpit( AT_ORANGE , target , buf );
      	                   	 		sprintf( buf, "Laserfire from %s barely misses %s." , ship->name , target->name );
                            		echo_to_system( AT_ORANGE , target , buf , NULL );
             					}
             					else
             					{
             		    		sprintf( buf, "Laserfire from %s hits %s." , ship->name, target->name );
             		    		echo_to_system( AT_ORANGE , target , buf , NULL );
                            	sprintf( buf , "You are hit by lasers from %s!" , ship->name);
                            	echo_to_cockpit( AT_BLOOD , target , buf );
                            	echo_to_ship( AT_RED , target , "A small explosion vibrates through the ship." );
                            	damage_ship( target , 5 , 10 );
                        		}
                        		ship->statet0++;
                    		}
                  		}
                	}
             	}
          	}
       	}
   	}
}

void update_space( )
{
   SHIP_DATA *ship;
   SHIP_DATA *target;
   char buf[MAX_STRING_LENGTH];
   int too_close, target_too_close;
   int recharge;
   CHAR_DATA *ch;

   for ( ship = first_ship; ship; ship = ship->next )
   {
          if ( ship->energy > 0 && ship->shipstate == SHIP_DISABLED && ship->class != SHIP_PLATFORM )
             ship->energy -= 5;
          else if ( ship->energy > 0 && ship->currspeed == 0 && ship->class == CAPITAL_SHIP)
             ship->energy += UMIN(40,ship->maxenergy - ship->energy);
          else if ( ship->energy > 0 && ship->currspeed == 0 )
             ;//ship->energy += ( 20 + ship->class*10 );  Removed in space ship recharging. 2006-10-16 Funf
          else if ( ship->energy > 0 )
          	 ship->energy -= ( 5 + ship->class*5 );
          else if ( ship->shipstate != SHIP_DOCKED )
             destroy_ship(ship , NULL);

        if ( ship->chaff_released > 0 )
           ship->chaff_released--;

        if (ship->shipstate == SHIP_HYPERSPACE)
        {
            ship->hyperdistance -= get_ship_speed(ship);
            if (ship->hyperdistance <= 0)
            {
            	ship_to_starsystem (ship, ship->currjump);

            	if (ship->starsystem == NULL)
            	{
            	    echo_to_cockpit( AT_RED, ship, "Ship lost in Hyperspace. Make new calculations.");
            	}
            	else
            	{
            	    echo_to_room( AT_YELLOW, get_room_index(ship->pilotseat), "Hyperjump complete.");
            	    echo_to_ship( AT_YELLOW, ship, "The ship lurches slightly as it comes out of hyperspace.");
		    echo_to_ship( AT_YELLOW, ship, "The ship's speed slows down to handle re-entry.");
		    ship->currspeed = 0;
            	    sprintf( buf ,"%s enters the starsystem at %.0f %.0f %.0f" , ship->name, ship->vx, ship->vy, ship->vz );
            	    echo_to_system( AT_YELLOW, ship, buf , NULL );
            	    ship->shipstate = SHIP_READY;
            	    ship->currjump = NULL;
            	    STRFREE( ship->home );
            	    ship->home = STRALLOC( ship->starsystem->name );
            	    if ( str_cmp("Public",ship->owner) )
                        save_ship(ship);

            	}
            }
            else
            {
               sprintf( buf ,"%d" , ship->hyperdistance );
               echo_to_room_dnr( AT_YELLOW , get_room_index(ship->pilotseat), "Remaining jump distance: " );
               echo_to_room( AT_WHITE , get_room_index(ship->pilotseat),  buf );
			   ship->energy -= ( 20 + (1 + ship->class ) * 10 );
            }
        }

        /* following was originally to fix ships that lost their pilot
           in the middle of a manuever and are stuck in a busy state
           but now used for timed manuevers such as turning */

    	if (ship->shipstate == SHIP_BUSY_3)
           {
              echo_to_room( AT_YELLOW, get_room_index(ship->pilotseat), "Manuever complete.");
              ship->shipstate = SHIP_READY;
           }
        if (ship->shipstate == SHIP_BUSY_2)
           ship->shipstate = SHIP_BUSY_3;
        if (ship->shipstate == SHIP_BUSY)
           ship->shipstate = SHIP_BUSY_2;

        if (ship->shipstate == SHIP_LAND_2)
           landship( ship , ship->dest );
        if (ship->shipstate == SHIP_LAND)
           ship->shipstate = SHIP_LAND_2;

        if (ship->shipstate == SHIP_LAUNCH_3)
           launchship( ship );
        if (ship->shipstate == SHIP_LAUNCH_2)
           ship->shipstate = SHIP_LAUNCH_3;
        if (ship->shipstate == SHIP_LAUNCH)
           ship->shipstate = SHIP_LAUNCH_2;

        if ( ship->starsystem && ship->currspeed > 0 )
        {
               sprintf( buf, "%d",
                           get_ship_speed(ship) );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->pilotseat),  "Speed: " );
               echo_to_room_dnr( AT_LBLUE , get_room_index(ship->pilotseat),  buf );
               sprintf( buf, "%.0f %.0f %.0f",
                           ship->vx , ship->vy, ship->vz );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->pilotseat),  "  Coords: " );
               echo_to_room( AT_LBLUE , get_room_index(ship->pilotseat),  buf );
            if ( ship->pilotseat != ship->coseat )
            {
               sprintf( buf, "%d",
                          get_ship_speed(ship) );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->coseat),  "Speed: " );
               echo_to_room_dnr( AT_LBLUE , get_room_index(ship->coseat),  buf );
               sprintf( buf, "%.0f %.0f %.0f",
                           ship->vx , ship->vy, ship->vz );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->coseat),  "  Coords: " );
               echo_to_room( AT_LBLUE , get_room_index(ship->coseat),  buf );
            }
        }

        if ( ship->starsystem && get_room_index(ship->pilotseat) != NULL)
        {
          too_close = get_ship_speed(ship) + 50;
          for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem)
          {
            target_too_close = too_close + get_ship_speed(ship);
            if ( target != ship &&
                 abs(ship->vx - target->vx) < target_too_close &&
                 abs(ship->vy - target->vy) < target_too_close &&
                 abs(ship->vz - target->vz) < target_too_close )
            {
                sprintf( buf, "Proximity alert: %s  %.0f %.0f %.0f"
                            , target->name, target->vx, target->vy, target->vz);

				for ( ch = get_room_index(ship->pilotseat)->first_person; ch; ch = ch->next_in_room )
				{
					if (!IS_SET(ch->act, PLR_NOPROXMSG))
					{
						set_char_color( AT_RED, ch );
						send_to_char( buf, ch );
						send_to_char( "\n\r",   ch );
					}
				}

            }
          }
          too_close = get_ship_speed(ship) + 100;
          if ( ship->starsystem->planet1 && strcmp(ship->starsystem->planet1,"") &&
               abs(ship->vx - ship->starsystem->p1x) < too_close &&
               abs(ship->vy - ship->starsystem->p1y) < too_close &&
               abs(ship->vz - ship->starsystem->p1z) < too_close )
          {
                sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet1,
                         ship->starsystem->p1x, ship->starsystem->p1y, ship->starsystem->p1z);
				for ( ch = get_room_index(ship->pilotseat)->first_person; ch; ch = ch->next_in_room )
				{
					if (!IS_SET(ch->act, PLR_NOPROXMSG))
					{
						set_char_color( AT_RED, ch);
						send_to_char( buf, ch );
						send_to_char( "\n\r",   ch );
					}
				}
//                echo_to_room( AT_RED , get_room_index(ship->pilotseat),  buf );
          }
          if ( ship->starsystem->planet2 && strcmp(ship->starsystem->planet2,"") &&
               abs(ship->vx - ship->starsystem->p2x) < too_close &&
               abs(ship->vy - ship->starsystem->p2y) < too_close &&
               abs(ship->vz - ship->starsystem->p2z) < too_close )
          {
                sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet2,
                         ship->starsystem->p2x, ship->starsystem->p2y, ship->starsystem->p2z);
//                echo_to_room( AT_RED , get_room_index(ship->pilotseat),  buf );
				for ( ch = get_room_index(ship->pilotseat)->first_person; ch; ch = ch->next_in_room )
				{
					if (!IS_SET(ch->act, PLR_NOPROXMSG))
					{
						set_char_color( AT_RED, ch );
						send_to_char( buf, ch );
						send_to_char( "\n\r",   ch );
					}
				}
          }
          if ( ship->starsystem->planet3 && strcmp(ship->starsystem->planet3,"") &&
               abs(ship->vx - ship->starsystem->p3x) < too_close &&
               abs(ship->vy - ship->starsystem->p3y) < too_close &&
               abs(ship->vz - ship->starsystem->p3z) < too_close )
          {
                sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet3,
                         ship->starsystem->p3x, ship->starsystem->p3y, ship->starsystem->p3z);
//                echo_to_room( AT_RED , get_room_index(ship->pilotseat),  buf );
				for ( ch = get_room_index(ship->pilotseat)->first_person; ch; ch = ch->next_in_room )
				{
					if (!IS_SET(ch->act, PLR_NOPROXMSG))
					{
						set_char_color( AT_RED, ch );
						send_to_char( buf, ch );
						send_to_char( "\n\r",   ch);
					}
				}
          }
          if ( ship->starsystem->planet4 && strcmp(ship->starsystem->planet4,"") &&
               abs(ship->vx - ship->starsystem->p4x) < too_close &&
               abs(ship->vy - ship->starsystem->p4y) < too_close &&
               abs(ship->vz - ship->starsystem->p4z) < too_close )
          {
                sprintf( buf, "Proximity alert: %s  %d %d %d", ship->starsystem->planet4,
                         ship->starsystem->p4x, ship->starsystem->p4y, ship->starsystem->p4z);
//                echo_to_room( AT_RED , get_room_index(ship->pilotseat),  buf );
				for ( ch = get_room_index(ship->pilotseat)->first_person; ch; ch = ch->next_in_room )
				{
					if (!IS_SET(ch->act, PLR_NOPROXMSG))
					{
						set_char_color( AT_RED, ch );
						send_to_char( buf, ch );
						send_to_char( "\n\r",   ch);
					}
				}
          }
        }


        if (ship->target0)
        {
               sprintf( buf, "%s   %.0f %.0f %.0f", ship->target0->name,
                          ship->target0->vx , ship->target0->vy, ship->target0->vz );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->gunseat), "Target: " );
               echo_to_room( AT_LBLUE , get_room_index(ship->gunseat),  buf );
               if (ship->starsystem != ship->target0->starsystem)
               		ship->target0 = NULL;
         }

        if (ship->target1)
        {
               sprintf( buf, "%s   %.0f %.0f %.0f", ship->target1->name,
                          ship->target1->vx , ship->target1->vy, ship->target1->vz );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->turret1), "Target: " );
               echo_to_room( AT_LBLUE , get_room_index(ship->turret1),  buf );
               if (ship->starsystem != ship->target1->starsystem)
               		ship->target1 = NULL;
         }

        if (ship->target2)
        {
               sprintf( buf, "%s   %.0f %.0f %.0f", ship->target2->name,
                          ship->target2->vx , ship->target2->vy, ship->target2->vz );
               echo_to_room_dnr( AT_BLUE , get_room_index(ship->turret2), "Target: " );
               echo_to_room( AT_LBLUE , get_room_index(ship->turret2),  buf );
               if (ship->starsystem != ship->target2->starsystem)
               		ship->target2 = NULL;
         }

   	if (ship->energy < 100 && ship->starsystem )
   	{
   	    echo_to_cockpit( AT_RED , ship,  "Warning: Ship fuel low." );
        }

        ship->energy = URANGE( 0 , ship->energy, ship->maxenergy );
   }

   for ( ship = first_ship; ship; ship = ship->next )
   {

       if (ship->autotrack && ship->target0 && ship->class < 3 )
       {
           target = ship->target0;
           too_close = get_ship_speed(ship) + 10;
           target_too_close = too_close + get_ship_speed(ship);
           if ( target != ship && ship->shipstate == SHIP_READY &&
                 abs(ship->vx - target->vx) < target_too_close &&
                 abs(ship->vy - target->vy) < target_too_close &&
                 abs(ship->vz - target->vz) < target_too_close )
           {
              ship->hx = 0-(ship->target0->vx - ship->vx);
              ship->hy = 0-(ship->target0->vy - ship->vy);
              ship->hz = 0-(ship->target0->vz - ship->vz);
              ship->energy -= ship->currspeed / 10;
              echo_to_room( AT_RED , get_room_index(ship->pilotseat), "Autotrack: Evading to avoid collision!\n\r" );
    	      if ( ship->class == FIGHTER_SHIP || ( ship->class == MIDSIZE_SHIP && ship->manuever > 50 ) )
        	ship->shipstate = SHIP_BUSY_3;
              else if ( ship->class == MIDSIZE_SHIP || ( ship->class == CAPITAL_SHIP && ship->manuever > 50 ) )
        	ship->shipstate = SHIP_BUSY_2;
    	      else
        	ship->shipstate = SHIP_BUSY;
           }
           else if  ( !is_facing(ship, ship->target0) )
           {
              ship->hx = ship->target0->vx - ship->vx;
              ship->hy = ship->target0->vy - ship->vy;
              ship->hz = ship->target0->vz - ship->vz;
              ship->energy -= ship->currspeed / 10;
              echo_to_room( AT_BLUE , get_room_index(ship->pilotseat), "Autotracking target ... setting new course.\n\r" );
    	      if ( ship->class == FIGHTER_SHIP || ( ship->class == MIDSIZE_SHIP && ship->manuever > 50 ) )
        	ship->shipstate = SHIP_BUSY_3;
              else if ( ship->class == MIDSIZE_SHIP || ( ship->class == CAPITAL_SHIP && ship->manuever > 50 ) )
        	ship->shipstate = SHIP_BUSY_2;
    	      else
        	ship->shipstate = SHIP_BUSY;
           }
       }

       if ( autofly(ship) )
       {
          if ( ship->starsystem )
          {
             if (ship->target0)
             {
                 int chance = 50;

                 /* auto assist ships */

                 for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem)
                 {
                    if ( autofly(target) )
                       if ( !str_cmp ( target->owner , ship->owner ) && target != ship )
                         if ( target->target0 == NULL && ship->target0 != target )
                         {
                           target->target0 = ship->target0;
                           sprintf( buf , "You are being targetted by %s." , target->name);
                           echo_to_cockpit( AT_BLOOD , target->target0 , buf );
                           break;
                        }
                 }

                 target = ship->target0;
                 ship->autotrack = TRUE;
                 if( ship->class != SHIP_PLATFORM )
                      ship->currspeed = ship->realspeed;
                 if ( ship->energy >200  )
                    ship->autorecharge=TRUE;


                 if (ship->shipstate != SHIP_HYPERSPACE && ship->energy > 25
                 && ship->missilestate == MISSILE_READY && ship->target0->starsystem == ship->starsystem
                 && abs(target->vx - ship->vx) <= 1200
                 && abs(target->vy - ship->vy) <= 1200
                 && abs(target->vz - ship->vz) <= 1200
                 && ship->missiles > 0 )
                 {
                   if ( ship->class > 1 || is_facing( ship , target ) )
                   {
             		chance -= target->manuever/5;
                        chance -= get_ship_speed(ship) / 20;
                        chance += target->class*target->class*25;
                        chance -= ( abs(target->vx - ship->vx)/100 );
                        chance -= ( abs(target->vy - ship->vy)/100 );
                        chance -= ( abs(target->vz - ship->vz)/100 );
                        chance += ( 30 );
                        chance = URANGE( 10 , chance , 90 );

             		if ( number_percent( ) > chance )
             		{
             		}
                        else
                        {
                                new_missile( ship , target , NULL , CONCUSSION_MISSILE );
            		 	ship->missiles-- ;
             		        sprintf( buf , "Incoming missile from %s." , ship->name);
             		        echo_to_cockpit( AT_BLOOD , target , buf );
             		        sprintf( buf, "%s fires a missile towards %s." , ship->name, target->name );
             		        echo_to_system( AT_ORANGE , target , buf , NULL );

            		 	if ( ship->class == CAPITAL_SHIP || ship->class == SHIP_PLATFORM )
                                    ship->missilestate = MISSILE_RELOAD_2;
                                else
                                    ship->missilestate = MISSILE_FIRED;
                        }
                   }
                 }

                 if( ship->missilestate ==  MISSILE_DAMAGED )
                     ship->missilestate =  MISSILE_READY;
                 if( ship->statet0 ==  LASER_DAMAGED )
                     ship->statet0 =  LASER_READY;
                 if( ship->shipstate ==  SHIP_DISABLED )
                     ship->shipstate =  SHIP_READY;

             }
             else
             {
                 ship->currspeed = 0;

                 if ( !str_cmp( ship->owner , "The Empire" ) )
                   for ( target = first_ship; target; target = target->next )
                     if ( ship->starsystem == target->starsystem )
                        if ( !str_cmp( target->owner , "The New Republic" ) )
                        {
                          ship->target0 = target;
                          sprintf( buf , "You are being targetted by %s." , ship->name);
                          echo_to_cockpit( AT_BLOOD , target , buf );
                          break;
                        }
                if ( !str_cmp( ship->owner , "The New Republic" ) )
                   for ( target = first_ship; target; target = target->next )
                     if ( ship->starsystem == target->starsystem )
                        if ( !str_cmp( target->owner , "The Empire" ) )
                        {
                          sprintf( buf , "You are being targetted by %s." , ship->name);
                          echo_to_cockpit( AT_BLOOD , target , buf );
                          ship->target0 = target;
                          break;
                        }

               if ( !str_cmp( ship->owner , "Pirates" ) )
                   for ( target = first_ship; target; target = target->next )
                     if ( ship->starsystem == target->starsystem )
                     {
                          sprintf( buf , "You are being targetted by %s." , ship->name);
                          echo_to_cockpit( AT_BLOOD , target , buf );
                          ship->target0 = target;
                          break;
                     }

             }
          }
          else
          {
               if ( number_range(1, 25) == 25 )
               {
          	  ship_to_starsystem(ship, starsystem_from_name(ship->home) );
          	  ship->vx = number_range( -2000 , 2000 );
          	  ship->vy = number_range( -2000 , 2000 );
          	  ship->vz = number_range( -2000 , 2000 );
                  ship->hx = 1;
                  ship->hy = 1;
                  ship->hz = 1;
               }
          }
       }

       if ( ( ship->class == CAPITAL_SHIP || ship->class == SHIP_PLATFORM )
       && ship->target0 == NULL )
       {
          if( ship->missiles < ship->maxmissiles )
             ship->missiles++;
          if( ship->torpedos < ship->maxtorpedos )
             ship->torpedos++;
          if( ship->rockets < ship->maxrockets )
             ship->rockets++;
	  if( ship->ammo < ship->maxammo )
	     ship->ammo++;
       }
   }

}


void write_starsystem_list( )
{
	SPACE_DATA *tstarsystem;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", SPACE_DIR, SPACE_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
         bug( "FATAL: cannot open starsystem.lst for writing!\n\r", 0 );
         return;
    }
    for ( tstarsystem = first_starsystem; tstarsystem; tstarsystem = tstarsystem->next )
    	fprintf( fpout, "%s\n", tstarsystem->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}


/*
 * Get pointer to space structure from starsystem name.
 */
SPACE_DATA *starsystem_from_name( char *name )
{
	SPACE_DATA *starsystem;

    for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
    	if ( !str_cmp( name, starsystem->name ) )
        	return starsystem;

    for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
    	if ( !str_prefix( name, starsystem->name ) )
        	return starsystem;

    return NULL;
}

/*
 * Get pointer to space structure from the dock vnun.
 */
SPACE_DATA *starsystem_from_vnum( int vnum )
{
	SPACE_DATA *starsystem;
    SHIP_DATA *ship;

    for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
    	if ( vnum == starsystem->doc1a || vnum == starsystem->doc2a    || vnum == starsystem->doc3a || vnum == starsystem->doc4a ||
        	 vnum == starsystem->doc1b || vnum == starsystem->doc2b    || vnum == starsystem->doc3b || vnum == starsystem->doc4b ||
             vnum == starsystem->doc1c || vnum == starsystem->doc2c    || vnum == starsystem->doc3c || vnum == starsystem->doc4c)
        	return starsystem;

    for ( ship = first_ship; ship; ship = ship->next )
       	if ( vnum == ship->hangar )
       		if (ship->starsystem)
        		return ship->starsystem;
        	else  // Ship docked? Find docked ship's starsystem! --Funf
        		{ // I'd do this with recursion, but I'm very afraid of imms nesting ships in themselves... --Funf
				    for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
				    	if ( ship->location == starsystem->doc1a || ship->location == starsystem->doc2a    || ship->location == starsystem->doc3a || ship->location == starsystem->doc4a ||
				        	 ship->location == starsystem->doc1b || ship->location == starsystem->doc2b    || ship->location == starsystem->doc3b || ship->location == starsystem->doc4b ||
				             ship->location == starsystem->doc1c || ship->location == starsystem->doc2c    || ship->location == starsystem->doc3c || ship->location == starsystem->doc4c)
        			return starsystem;
				}

    return NULL;
}


/*
 * Save a starsystem's data to its data file
 */
void save_starsystem( SPACE_DATA *starsystem )
{
	FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !starsystem )
    {
		bug( "save_starsystem: null starsystem pointer!", 0 );
		return;
    }

    if ( !starsystem->filename || starsystem->filename[0] == '\0' )
    {
		sprintf( buf, "save_starsystem: %s has no filename", starsystem->name );
		bug( buf, 0 );
		return;
    }

    sprintf( filename, "%s%s", SPACE_DIR, starsystem->filename );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_starsystem: fopen", 0 );
    	perror( filename );
    }
    else
    {
		fprintf( fp, "#SPACE\n" );
		fprintf( fp, "Name         %s~\n",	starsystem->name	);
		fprintf( fp, "Filename     %s~\n",	starsystem->filename	);
		fprintf( fp, "Planet1      %s~\n",	starsystem->planet1	);
		fprintf( fp, "Planet2      %s~\n",	starsystem->planet2	);
		fprintf( fp, "Planet3      %s~\n",	starsystem->planet3	);
		fprintf( fp, "Planet4      %s~\n",	starsystem->planet4	);
		fprintf( fp, "Location1a      %s~\n",	starsystem->location1a	);
		fprintf( fp, "Location1b      %s~\n",	starsystem->location1b	);
		fprintf( fp, "Location1c      %s~\n",	starsystem->location1c	);
		fprintf( fp, "Location2a       %s~\n",	starsystem->location2a	);
		fprintf( fp, "Location2b      %s~\n",	starsystem->location2b	);
		fprintf( fp, "Location2c      %s~\n",	starsystem->location2c	);
		fprintf( fp, "Location3a      %s~\n",	starsystem->location3a	);
		fprintf( fp, "Location3b      %s~\n",	starsystem->location3b	);
		fprintf( fp, "Location3c      %s~\n",	starsystem->location3c	);
		fprintf( fp, "Location4a      %s~\n",	starsystem->location4a	);
		fprintf( fp, "Location4b      %s~\n",	starsystem->location4b	);
		fprintf( fp, "Location4c      %s~\n",	starsystem->location4c	);
		fprintf( fp, "Doc1a          %d\n",	starsystem->doc1a	);
		fprintf( fp, "Doc2a          %d\n",      starsystem->doc2a       );
		fprintf( fp, "Doc3a          %d\n",      starsystem->doc3a       );
		fprintf( fp, "Doc4a          %d\n",      starsystem->doc4a       );
		fprintf( fp, "Doc1b          %d\n", 	 starsystem->doc1b	);
		fprintf( fp, "Doc2b          %d\n",      starsystem->doc2b       );
		fprintf( fp, "Doc3b          %d\n",      starsystem->doc3b       );
		fprintf( fp, "Doc4b          %d\n",      starsystem->doc4b       );
		fprintf( fp, "Doc1c          %d\n",	 starsystem->doc1c	);
		fprintf( fp, "Doc2c          %d\n",      starsystem->doc2c       );
		fprintf( fp, "Doc3c          %d\n",      starsystem->doc3c       );
		fprintf( fp, "Doc4c          %d\n",      starsystem->doc4c       );
		fprintf( fp, "P1x          %d\n",       starsystem->p1x         );
		fprintf( fp, "P1y          %d\n",       starsystem->p1y         );
		fprintf( fp, "P1z          %d\n",       starsystem->p1z         );
		fprintf( fp, "P2x          %d\n",       starsystem->p2x         );
		fprintf( fp, "P2y          %d\n",       starsystem->p2y         );
		fprintf( fp, "P2z          %d\n",       starsystem->p2z         );
		fprintf( fp, "P3x          %d\n",       starsystem->p3x         );
		fprintf( fp, "P3y          %d\n",       starsystem->p3y         );
		fprintf( fp, "P3z          %d\n",       starsystem->p3z         );
		fprintf( fp, "P4x          %d\n",       starsystem->p4x         );
		fprintf( fp, "P4y          %d\n",       starsystem->p4y         );
		fprintf( fp, "P4z          %d\n",       starsystem->p4z         );
		fprintf( fp, "Gravityp1     %d\n",       starsystem->gravityp1    );
		fprintf( fp, "Gravityp2     %d\n",       starsystem->gravityp2    );
		fprintf( fp, "Gravityp3     %d\n",       starsystem->gravityp3    );
		fprintf( fp, "Gravityp4     %d\n",       starsystem->gravityp4    );
		fprintf( fp, "Xpos          %d\n",       starsystem->xpos    );
		fprintf( fp, "Ypos          %d\n",       starsystem->ypos    );
		fprintf( fp, "G1x          %d\n",       starsystem->g1x         );
		fprintf( fp, "G1y          %d\n",       starsystem->g1y         );
		fprintf( fp, "G1z          %d\n",       starsystem->g1z         );
		fprintf( fp, "G2x          %d\n",       starsystem->g2x         );
		fprintf( fp, "G2y          %d\n",       starsystem->g2y         );
		fprintf( fp, "G2z          %d\n",       starsystem->g2z         );
		fprintf( fp, "G3x          %d\n",       starsystem->g3x         );
		fprintf( fp, "G3y          %d\n",       starsystem->g3y         );
		fprintf( fp, "G3z          %d\n",       starsystem->g3z         );
		fprintf( fp, "G4x          %d\n",       starsystem->g4x         );
		fprintf( fp, "G4y          %d\n",       starsystem->g4y         );
		fprintf( fp, "G4z          %d\n",       starsystem->g4z         );
		fprintf( fp, "G5x          %d\n",       starsystem->g5x         );
		fprintf( fp, "G5y          %d\n",       starsystem->g5y         );
		fprintf( fp, "G5z          %d\n",       starsystem->g5z         );
		fprintf( fp, "G6x          %d\n",       starsystem->g6x         );
		fprintf( fp, "G6y          %d\n",       starsystem->g6y         );
		fprintf( fp, "G6z          %d\n",       starsystem->g6z         );
		fprintf( fp, "Gate1        %s~\n",		starsystem->gate1	);
		fprintf( fp, "Gate2        %s~\n",		starsystem->gate2	);
		fprintf( fp, "Gate3        %s~\n",		starsystem->gate3	);
		fprintf( fp, "Gate4        %s~\n",		starsystem->gate4	);
		fprintf( fp, "Gate5        %s~\n",		starsystem->gate5	);
		fprintf( fp, "Gate6        %s~\n",		starsystem->gate6	);
		fprintf( fp, "Hospital	   %d\n",	starsystem->hospital	);
		fprintf( fp, "End\n\n"						);
		fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual starsystem data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )		\
				{										\
				    field  = value;						\
				    fMatch = TRUE;						\
				    break;								\
				}

void fread_starsystem( SPACE_DATA *starsystem, FILE *fp )
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

    	case 'D':
        	KEY( "Doc1a",  starsystem->doc1a,          fread_number( fp ) );
        	KEY( "Doc2a",  starsystem->doc2a,          fread_number( fp ) );
        	KEY( "Doc3a",  starsystem->doc3a,          fread_number( fp ) );
        	KEY( "Doc4a",  starsystem->doc4a,          fread_number( fp ) );
        	KEY( "Doc1b",  starsystem->doc1b,          fread_number( fp ) );
        	KEY( "Doc2b",  starsystem->doc2b,          fread_number( fp ) );
        	KEY( "Doc3b",  starsystem->doc3b,          fread_number( fp ) );
        	KEY( "Doc4b",  starsystem->doc4b,          fread_number( fp ) );
        	KEY( "Doc1c",  starsystem->doc1c,          fread_number( fp ) );
        	KEY( "Doc2c",  starsystem->doc2c,          fread_number( fp ) );
        	KEY( "Doc3c",  starsystem->doc3c,          fread_number( fp ) );
        	KEY( "Doc4c",  starsystem->doc4c,          fread_number( fp ) );
        	break;


		case 'E':
	    	if ( !str_cmp( word, "End" ) )
	    	{
				if (!starsystem->gate1)
		  			starsystem->gate1            		= STRALLOC( "" );
				if (!starsystem->gate2)
		  			starsystem->gate2            		= STRALLOC( "" );
				if (!starsystem->gate3)
		  			starsystem->gate3            		= STRALLOC( "" );
				if (!starsystem->gate4)
					starsystem->gate4            		= STRALLOC( "" );
				if (!starsystem->gate5)
					starsystem->gate5            		= STRALLOC( "" );
				if (!starsystem->gate6)
					starsystem->gate6            		= STRALLOC( "" );
				if (!starsystem->name)
					starsystem->name					= STRALLOC( "" );
				if (!starsystem->location1a)
					starsystem->location1a            = STRALLOC( "" );
				if (!starsystem->location2a)
					starsystem->location2a            = STRALLOC( "" );
				if (!starsystem->location3a)
					starsystem->location3a            = STRALLOC( "" );
				if (!starsystem->location4a)
					starsystem->location4a            = STRALLOC( "" );
				if (!starsystem->location1b)
					starsystem->location1b            = STRALLOC( "" );
				if (!starsystem->location2b)
					starsystem->location2b            = STRALLOC( "" );
				if (!starsystem->location3b)
					starsystem->location3b            = STRALLOC( "" );
				if (!starsystem->location4b)
					starsystem->location4b            = STRALLOC( "" );
				if (!starsystem->location1c)
					starsystem->location1c            = STRALLOC( "" );
				if (!starsystem->location2c)
					starsystem->location2c            = STRALLOC( "" );
				if (!starsystem->location3c)
					starsystem->location3c            = STRALLOC( "" );
				if (!starsystem->location4c)
					starsystem->location4c            = STRALLOC( "" );
				if (!starsystem->planet1)
					starsystem->planet1            	= STRALLOC( "" );
				if (!starsystem->planet2)
					starsystem->planet2            	= STRALLOC( "" );
				if (!starsystem->planet3)
					starsystem->planet3            	= STRALLOC( "" );
				if (!starsystem->planet4)
					starsystem->planet4            	= STRALLOC( "" );
				return;
	    	}
	    	break;

		case 'F':
	    	KEY( "Filename",	starsystem->filename,		fread_string_nohash( fp ) );
	    	break;

    	case 'G':
        	KEY( "Gate1",	starsystem->gate1,	fread_string( fp ) );
	    	KEY( "Gate2",	starsystem->gate2,	fread_string( fp ) );
        	KEY( "Gate3",	starsystem->gate3,	fread_string( fp ) );
	    	KEY( "Gate4",	starsystem->gate4,	fread_string( fp ) );
        	KEY( "Gate5",	starsystem->gate5,	fread_string( fp ) );
	    	KEY( "Gate6",	starsystem->gate6,	fread_string( fp ) );
        	KEY( "Gravityp1",  starsystem->gravityp1,     fread_number( fp ) );
        	KEY( "Gravityp2",  starsystem->gravityp2,     fread_number( fp ) );
        	KEY( "Gravityp3",  starsystem->gravityp3,     fread_number( fp ) );
        	KEY( "Gravityp4",  starsystem->gravityp4,     fread_number( fp ) );
	    	KEY( "G1x",  starsystem->g1x,          fread_number( fp ) );
        	KEY( "G1y",  starsystem->g1y,          fread_number( fp ) );
        	KEY( "G1z",  starsystem->g1z,          fread_number( fp ) );
        	KEY( "G2x",  starsystem->g2x,          fread_number( fp ) );
        	KEY( "G2y",  starsystem->g2y,          fread_number( fp ) );
        	KEY( "G2z",  starsystem->g2z,          fread_number( fp ) );
	    	KEY( "G3x",  starsystem->g3x,          fread_number( fp ) );
        	KEY( "G3y",  starsystem->g3y,          fread_number( fp ) );
        	KEY( "G3z",  starsystem->g3z,          fread_number( fp ) );
        	KEY( "G4x",  starsystem->g4x,          fread_number( fp ) );
        	KEY( "G4y",  starsystem->g4y,          fread_number( fp ) );
        	KEY( "G4z",  starsystem->g4z,          fread_number( fp ) );
	    	KEY( "G5x",  starsystem->g5x,          fread_number( fp ) );
        	KEY( "G5y",  starsystem->g5y,          fread_number( fp ) );
        	KEY( "G5z",  starsystem->g5z,          fread_number( fp ) );
        	KEY( "G6x",  starsystem->g6x,          fread_number( fp ) );
        	KEY( "G6y",  starsystem->g6y,          fread_number( fp ) );
        	KEY( "G6z",  starsystem->g6z,          fread_number( fp ) );
        	break;
	case 'H':
		KEY( "Hospital", 	starsystem->hospital, fread_number( fp ) );
		break;

    	case 'L':
	    	KEY( "Location1a",	starsystem->location1a,	fread_string( fp ) );
	    	KEY( "Location2a",	starsystem->location2a,	fread_string( fp ) );
	    	KEY( "Location3a",	starsystem->location3a,	fread_string( fp ) );
	    	KEY( "Location4a",	starsystem->location4a,	fread_string( fp ) );
	    	KEY( "Location1b",	starsystem->location1b,	fread_string( fp ) );
	    	KEY( "Location2b",	starsystem->location2b,	fread_string( fp ) );
	    	KEY( "Location3b",	starsystem->location3b,	fread_string( fp ) );
	    	KEY( "Location4b",	starsystem->location4b,	fread_string( fp ) );
	    	KEY( "Location1c",	starsystem->location1c,	fread_string( fp ) );
	    	KEY( "Location2c",	starsystem->location2c,	fread_string( fp ) );
	    	KEY( "Location3c",	starsystem->location3c,	fread_string( fp ) );
	    	KEY( "Location4c",	starsystem->location4c,	fread_string( fp ) );
	    	break;

		case 'N':
	    	KEY( "Name",	starsystem->name,		fread_string( fp ) );
	    	break;

    	case 'P':
        	KEY( "Planet1",	starsystem->planet1,	fread_string( fp ) );
	    	KEY( "Planet2",	starsystem->planet2,	fread_string( fp ) );
	    	KEY( "Planet3",	starsystem->planet3,	fread_string( fp ) );
	    	KEY( "Planet4",	starsystem->planet4,	fread_string( fp ) );
	    	KEY( "P1x",  starsystem->p1x,          fread_number( fp ) );
       		KEY( "P1y",  starsystem->p1y,          fread_number( fp ) );
        	KEY( "P1z",  starsystem->p1z,          fread_number( fp ) );
        	KEY( "P2x",  starsystem->p2x,          fread_number( fp ) );
        	KEY( "P2y",  starsystem->p2y,          fread_number( fp ) );
        	KEY( "P2z",  starsystem->p2z,          fread_number( fp ) );
        	KEY( "P3x",  starsystem->p3x,          fread_number( fp ) );
        	KEY( "P3y",  starsystem->p3y,          fread_number( fp ) );
        	KEY( "P3z",  starsystem->p3z,          fread_number( fp ) );
        	KEY( "P4x",  starsystem->p4x,          fread_number( fp ) );
        	KEY( "P4y",  starsystem->p4y,          fread_number( fp ) );
        	KEY( "P4z",  starsystem->p4z,          fread_number( fp ) );
        	break;

    	case 'X':
    	    KEY( "Xpos",  starsystem->xpos,     fread_number( fp ) );

    	case 'Y':
        	KEY( "Ypos",  starsystem->ypos,     fread_number( fp ) );
    	}

		if ( !fMatch )
		{
	    	sprintf( buf, "Fread_starsystem: no match: %s", word );
	    	bug( buf, 0 );
		}
    }
}

/*
 * Load a starsystem file
 */

bool load_starsystem( char *starsystemfile )
{
	char filename[256];
    SPACE_DATA *starsystem;
    FILE *fp;
    bool found;

    CREATE( starsystem, SPACE_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", SPACE_DIR, starsystemfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
		found = TRUE;
        LINK( starsystem, first_starsystem, last_starsystem, next, prev );
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
				bug( "Load_starsystem_file: # not found.", 0 );
				break;
	    	}
	    	word = fread_word( fp );
	    	if ( !str_cmp( word, "SPACE"	) )
	    	{
	    		fread_starsystem( starsystem, fp );
	    		break;
	    	}
	    	else
	    		if ( !str_cmp( word, "END"	) )
	        		break;
	    		else
	    		{
					char buf[MAX_STRING_LENGTH];

					sprintf( buf, "Load_starsystem_file: bad section: %s.", word );
					bug( buf, 0 );
					break;
	    		}
		}
		fclose( fp );
    }

    if ( !(found) )
    	DISPOSE( starsystem );

    return found;
}

/*
 * Load in all the starsystem files.
 */
void load_space( )
{
    FILE *fpList;
	char *filename;
    char starsystemlist[256];
    char buf[MAX_STRING_LENGTH];


    first_starsystem	= NULL;
    last_starsystem		= NULL;

    log_string( "Loading space..." );

    sprintf( starsystemlist, "%s%s", SPACE_DIR, SPACE_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( starsystemlist, "r" ) ) == NULL )
    {
		perror( starsystemlist );
		exit( 1 );
    }

    for ( ; ; )
    {
		filename = feof( fpList ) ? "$" : fread_word( fpList );
		if ( filename[0] == '$' )
	  		break;

		if ( !load_starsystem( filename ) )
		{
		  	sprintf( buf, "Cannot load starsystem file: %s", filename );
		  	bug( buf, 0 );
		}
    }
    fclose( fpList );
    log_string(" Done starsystems " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Get color from text
 */
char *get_colortext( char * color )
{
	if ( color == NULL )
		return "";
	if ( color[0] == '\0' )
		return "";
    if ( !str_cmp(color, "Blood") )
		return "&r";
	if ( !str_cmp(color, "Green") )
		return "&g";
	if ( !str_cmp(color, "DBlue") )
		return "&b";
	if ( !str_cmp(color, "Cyan") )
		return "&c";
	if ( !str_cmp(color, "LRed") )
		return "&R";
	if ( !str_cmp(color, "LGreen") )
		return "&G";
	if ( !str_cmp(color, "Blue") )
		return "&B";
	if ( !str_cmp(color, "LBlue") )
		return "&C";
	if ( !str_cmp(color, "Brown") )
		return "&O";
	if ( !str_cmp(color, "Purple") )
		return "&p";
	if ( !str_cmp(color, "Yellow") )
		return "&Y";
	if ( !str_cmp(color, "Pink") )
		return "&P";
	if ( !str_cmp(color, "White") )
		return "&W";
/*    return "&w";*/
	return "";
}

void do_override(CHAR_DATA *ch, char *argument)
{

    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SHIP_DATA *ship;
    SHIP_DATA *eShip = NULL;

    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);

    if ( (ship = ship_from_cockpit(ch->in_room->vnum) ) == NULL )
    {
        send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
        return;
    }

    if ( ship->class > SHIP_PLATFORM )
    {
        send_to_char("&RThis isn't a spacecraft!",ch);
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

    eShip = get_ship_here(arg,ship->starsystem);

    if ( eShip == NULL )
    {
        send_to_char("&RThat ship isn't here!\n\r",ch);
        return;
    }

    if ( eShip == ship )
    {
        send_to_char("&RYou are in the cockpit, just hit the controls!\n\r", ch);
        return;
    }

    if ( abs(eShip->vx - ship->vx) > 100*((ship->comm)+(eShip->comm)+20) ||
         abs(eShip->vy - ship->vy) > 100*((ship->comm)+(eShip->comm)+20) ||
         abs(eShip->vz - ship->vz) > 100*((ship->comm)+(eShip->comm)+20) )
    {
    	send_to_char("&RThat ship is out of the range of your comm system.\n\r&w", ch);
    	return;
    }

    if ( !check_pilot(ch,eShip) )
    {
        send_to_char("&RHey! That's not your ship!\n\r",ch);
        return;
    }

    if ( !str_prefix( arg2, "closebay" ) )
    {
        eShip->bayopen=FALSE;
        send_to_char( "&GBays Close. Confirmed.\n\r", ch);
        echo_to_cockpit( AT_YELLOW , eShip , "Bays Open");
        sprintf( buf ,"%s's bay doors close." , eShip->name );
        echo_to_system( AT_YELLOW, eShip, buf , NULL );
        return;
    }

    if ( !str_prefix( arg2, "stop" ) )
    {
    	eShip->currspeed = 0;
        send_to_char( "&GBraking Thrusters. Confirmed.\n\r", ch);
        echo_to_cockpit( AT_GREY , eShip , "Braking thrusters fire and the ship stops");
        sprintf( buf ,"%s decelerates." , eShip->name );
        echo_to_system( AT_GREY, eShip, buf , NULL );
        return;
    }

    if ( !str_prefix( arg2, "autopilot" ) )
    {
    	if ( ship->autopilot )
        {
           eShip->autopilot=FALSE;
           send_to_char( "&GYou toggle the autopilot.\n\r", ch);
           echo_to_cockpit( AT_YELLOW , eShip , "Autopilot OFF.");
           return;
    	}
        else if ( !ship->autopilot )
        {
           eShip->autopilot=TRUE;
           send_to_char( "&GYou toggle the autopilot.\n\r", ch);
           echo_to_cockpit( AT_YELLOW , eShip , "Autopilot ON.");
           return;
    	}
    }

    if ( !str_prefix( arg2, "openbay" ) )
   	{
    	send_to_char("&RYou open the bay doors of the remote ship.",ch);
    	act(AT_PLAIN,"$n flips a switch on the control panel.",ch,NULL,argument,TO_ROOM);
    	eShip->bayopen = TRUE;
    	sprintf( buf ,"%s's bay doors open." , eShip->name );
    	echo_to_system( AT_YELLOW, ship, buf , NULL );
    	return;
   	}

   	if ( !str_prefix( arg2, "disable" ) && strcmp(eShip->owner,"Public") && strcmp(eShip->owner, "public") ) /* Disable drives(MONO later) - Funf */
   	{
		send_to_char("&RYou disable the drive of the remote ship.",ch);
		act(AT_PLAIN,"$n flips a switch on the control panel.",ch,NULL,argument,TO_ROOM);
		eShip->shipstate = SHIP_DISABLED;
		sprintf( buf ,"%s comes to a stop." , eShip->name );
		echo_to_system( AT_YELLOW, ship, buf , NULL );
		return;
	}

   	send_to_char("Choices: autopilot openbay closebay stop disable\n\r", ch);
   	return;
}




void do_setstarsystem( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    SPACE_DATA *starsystem;

	if ( IS_NPC( ch ) )
    {
		send_to_char( "Huh?\n\r", ch );
		return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg2[0] == '\0' || arg1[0] == '\0' )
    {
		send_to_char( "Usage: setstarsystem <starsystem> <field> <values>\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( "name filename xpos ypos,\n\r", ch );
		send_to_char( "gate1 g1x g1y g1z gate2 g2x g2y g2z\n\r", ch );
		send_to_char( "gate3 g3x g3y g3z gate4 g4x g4y g4z\n\r", ch );
		send_to_char( "gate5 g5x g5y g5z gate6 g6x g6y g6z\n\r", ch );
		send_to_char( "planet1 p1x p1y p1z gravityp1\n\r", ch );
		send_to_char( "planet2 p2x p2y p2z gravityp2\n\r", ch );
		send_to_char( "planet3 p3x p3y p3z gravityp3\n\r", ch );
		send_to_char( "planet4 p4x p4y p4z gravityp4\n\r", ch );
		send_to_char( "location1a location1b location1c doc1a doc1b doc1c\n\r", ch );
		send_to_char( "location2a location2b location2c doc2a doc2b doc2c\n\r", ch );
		send_to_char( "location3a location3b location3c doc3a doc3b doc3c\n\r", ch );
		send_to_char( "location4a location4b location4c doc4a doc4b doc4c\n\r", ch );
		send_to_char( "Hospital\n\r", ch);
		send_to_char( "", ch );
		return;
    }

    starsystem = starsystem_from_name( arg1 );
    if ( !starsystem )
    {
		send_to_char( "No such starsystem.\n\r", ch );
		return;
    }


    if ( !str_cmp( arg2, "doc1a" ) )
    {
		starsystem->doc1a = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc1b" ) )
    {
		starsystem->doc1b = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc1c" ) )
    {
		starsystem->doc1c = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "doc2a" ) )
    {
		starsystem->doc2a = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc2b" ) )
    {
		starsystem->doc2b = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc2c" ) )
    {
		starsystem->doc2c = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "doc3a" ) )
    {
		starsystem->doc3a = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc3b" ) )
    {
		starsystem->doc3b = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc3c" ) )
    {
		starsystem->doc3c = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "doc4a" ) )
    {
		starsystem->doc4a = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc4b" ) )
    {
		starsystem->doc4b = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "doc4c" ) )
    {
		starsystem->doc4c = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "g1x" ) )
    {
		starsystem->g1x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g1y" ) )
    {
		starsystem->g1y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g1z" ) )
    {
		starsystem->g1z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "g2x" ) )
    {
		starsystem->g2x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g2y" ) )
    {
		starsystem->g2y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g2z" ) )
    {
		starsystem->g2z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "g3x" ) )
    {
		starsystem->g3x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g3y" ) )
    {
		starsystem->g3y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g3z" ) )
    {
		starsystem->g3z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "g4x" ) )
    {
		starsystem->g4x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g4y" ) )
    {
		starsystem->g4y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g4z" ) )
    {
		starsystem->g4z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "g5x" ) )
    {
		starsystem->g5x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g5y" ) )
    {
		starsystem->g5y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g5z" ) )
    {
		starsystem->g5z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "g6x" ) )
    {
		starsystem->g6x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g6y" ) )
    {
		starsystem->g6y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "g6z" ) )
    {
		starsystem->g6z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "p1x" ) )
    {
		starsystem->p1x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p1y" ) )
    {
		starsystem->p1y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p1z" ) )
    {
		starsystem->p1z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "p2x" ) )
    {
		starsystem->p2x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p2y" ) )
    {
		starsystem->p2y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p2z" ) )
    {
		starsystem->p2z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "p3x" ) )
    {
		starsystem->p3x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p3y" ) )
    {
		starsystem->p3y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p3z" ) )
    {
		starsystem->p3z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "p4x" ) )
    {
		starsystem->p4x = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p4y" ) )
    {
		starsystem->p4y = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "p4z" ) )
    {
		starsystem->p4z = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "xpos" ) )
    {
		starsystem->xpos = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "ypos" ) )
    {
		starsystem->ypos = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gravityp1" ) )
    {
		starsystem->gravityp1 = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gravityp2" ) )
    {
		starsystem->gravityp2 = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gravityp3" ) )
    {
		starsystem->gravityp3 = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gravityp4" ) )
    {
		starsystem->gravityp4 = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
		STRFREE( starsystem->name );
		starsystem->name = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gate1" ) )
    {
		STRFREE( starsystem->gate1 );
		starsystem->gate1 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gate2" ) )
    {
		STRFREE( starsystem->gate2 );
		starsystem->gate2 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gate3" ) )
    {
		STRFREE( starsystem->gate3 );
		starsystem->gate3 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gate4" ) )
    {
		STRFREE( starsystem->gate4 );
		starsystem->gate4 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gate5" ) )
    {
		STRFREE( starsystem->gate5 );
		starsystem->gate5 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "gate6" ) )
    {
		STRFREE( starsystem->gate6 );
		starsystem->gate6 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "planet1" ) )
    {
		STRFREE( starsystem->planet1 );
		starsystem->planet1 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "planet2" ) )
    {
		STRFREE( starsystem->planet2 );
		starsystem->planet2 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "planet3" ) )
    {
		STRFREE( starsystem->planet3 );
		starsystem->planet3 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "planet4" ) )
    {
		STRFREE( starsystem->planet4 );
		starsystem->planet4 = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location1a" ) )
    {
		STRFREE( starsystem->location1a );
		starsystem->location1a = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location1b" ) )
    {
		STRFREE( starsystem->location1b );
		starsystem->location1b = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location1c" ) )
    {
		STRFREE( starsystem->location1c );
		starsystem->location1c = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "location2a" ) )
    {
		STRFREE( starsystem->location2a  );
		starsystem->location2a = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location2b" ) )
    {
		STRFREE( starsystem->location2a );
		starsystem->location2b = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location2c" ) )
    {
		STRFREE( starsystem->location2c );
		starsystem->location2c = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "location3a" ) )
    {
		STRFREE( starsystem->location3a );
		starsystem->location3a = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location3b" ) )
    {
		STRFREE( starsystem->location3b );
		starsystem->location3b = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location3c" ) )
    {
		STRFREE( starsystem->location3c );
		starsystem->location3c = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "location4a" ) )
    {
		STRFREE( starsystem->location4a );
		starsystem->location4a = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location4b" ) )
    {
		STRFREE( starsystem->location4b );
		starsystem->location4b = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }
    if ( !str_cmp( arg2, "location4c" ) )
    {
		STRFREE( starsystem->location4c );
		starsystem->location4c = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_starsystem( starsystem );
		return;
    }

    if ( !str_cmp( arg2, "hospital" ))
    {
		starsystem->hospital = atoi(argument);
		send_to_char("Done.\n\r", ch);
		save_starsystem( starsystem );
		return;
    }
    do_setstarsystem( ch, "" );
    return;
}

void showstarsystem( CHAR_DATA *ch , SPACE_DATA *starsystem )
{
    ch_printf( ch, "Starsystem:%s     Filename: %s    Xpos: %d   Ypos: %d\n\r",
    		   starsystem->name,
    		   starsystem->filename,
    		   starsystem->xpos, starsystem->ypos);
	ch_printf( ch, "Hospital: %d\n\r", starsystem->hospital);
    ch_printf( ch, "Planet1: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
    		   starsystem->planet1, starsystem->gravityp1,
    		   starsystem->p1x , starsystem->p1y, starsystem->p1z);
    ch_printf( ch, "     Doc1a: %5d (%s)\n\r",
    		   starsystem->doc1a, starsystem->location1a);
    ch_printf( ch, "     Doc1b: %5d (%s)\n\r",
    		   starsystem->doc1b, starsystem->location1b);
    ch_printf( ch, "     Doc1c: %5d (%s)\n\r",
    		   starsystem->doc1c, starsystem->location1c);
    ch_printf( ch, "Planet2: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
    		   starsystem->planet2, starsystem->gravityp2,
    		   starsystem->p2x , starsystem->p2y, starsystem->p2z);
    ch_printf( ch, "     Doc2a: %5d (%s)\n\r",
    		   starsystem->doc2a, starsystem->location2a);
    ch_printf( ch, "     Doc2b: %5d (%s)\n\r",
    		   starsystem->doc2b, starsystem->location2b);
    ch_printf( ch, "     Doc2c: %5d (%s)\n\r",
    		   starsystem->doc2c, starsystem->location2c);
    ch_printf( ch, "Planet3: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
    		   starsystem->planet3, starsystem->gravityp3,
    		   starsystem->p3x , starsystem->p3y, starsystem->p3z);
    ch_printf( ch, "     Doc3a: %5d (%s)\n\r",
    		   starsystem->doc3a, starsystem->location3a);
    ch_printf( ch, "     Doc3b: %5d (%s)\n\r",
    		   starsystem->doc3b, starsystem->location3b);
    ch_printf( ch, "     Doc3c: %5d (%s)\n\r",
    		   starsystem->doc3c, starsystem->location3c);
    ch_printf( ch, "Planet4: %s   Gravity: %d   Coordinates: %d %d %d\n\r",
    		   starsystem->planet4, starsystem->gravityp4,
    		   starsystem->p4x , starsystem->p4y, starsystem->p4z);
    ch_printf( ch, "     Doc4a: %5d (%s)\n\r",
    		   starsystem->doc4a, starsystem->location4a);
    ch_printf( ch, "     Doc4b: %5d (%s)\n\r",
    		   starsystem->doc4b, starsystem->location4b);
    ch_printf( ch, "     Doc4c: %5d (%s)\n\r",
    		   starsystem->doc4c, starsystem->location4c);
    ch_printf( ch, "Gate1: %s   Coordinates: %d %d %d\n\r",
    		   starsystem->gate1, starsystem->g1x , starsystem->g1y, starsystem->g1z);
    ch_printf( ch, "Gate2: %s   Coordinates: %d %d %d\n\r",
    		   starsystem->gate2, starsystem->g2x , starsystem->g2y, starsystem->g2z);
    ch_printf( ch, "Gate3: %s   Coordinates: %d %d %d\n\r",
    		   starsystem->gate3, starsystem->g3x , starsystem->g3y, starsystem->g3z);
    ch_printf( ch, "Gate4: %s   Coordinates: %d %d %d\n\r",
    		   starsystem->gate4, starsystem->g4x , starsystem->g4y, starsystem->g4z);
    ch_printf( ch, "Gate5: %s   Coordinates: %d %d %d\n\r",
    		   starsystem->gate5, starsystem->g5x , starsystem->g5y, starsystem->g5z);
    ch_printf( ch, "Gate6: %s   Coordinates: %d %d %d\n\r",
    		   starsystem->gate6, starsystem->g6x , starsystem->g6y, starsystem->g6z);
    return;
}

void do_showstarsystem( CHAR_DATA *ch, char *argument )
{
	SPACE_DATA *starsystem;

   	starsystem = starsystem_from_name( argument );

   	if ( starsystem == NULL )
    	send_to_char("&RNo such starsystem.\n\r",ch);
   	else
    	showstarsystem(ch , starsystem);
}

void do_makestarsystem( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char filename[256];
    SPACE_DATA *starsystem;

    if ( !argument || argument[0] == '\0' )
    {
		send_to_char( "Usage: makestarsystem <starsystem name>\n\r", ch );
		return;
    }

	CREATE( starsystem, SPACE_DATA, 1 );
    LINK( starsystem, first_starsystem, last_starsystem, next, prev );

    starsystem->name					= STRALLOC( argument );

	starsystem->location1a            	= STRALLOC( "" );
	starsystem->location2a            	= STRALLOC( "" );
	starsystem->location3a            	= STRALLOC( "" );
	starsystem->location4a            	= STRALLOC( "" );
	starsystem->location1b            	= STRALLOC( "" );
	starsystem->location2b            	= STRALLOC( "" );
	starsystem->location3b            	= STRALLOC( "" );
	starsystem->location4b            	= STRALLOC( "" );
	starsystem->location1c            	= STRALLOC( "" );
	starsystem->location2c            	= STRALLOC( "" );
	starsystem->location3c            	= STRALLOC( "" );
	starsystem->location4c            	= STRALLOC( "" );
	starsystem->planet1            		= STRALLOC( "" );
	starsystem->planet2            		= STRALLOC( "" );
	starsystem->planet3            		= STRALLOC( "" );
	starsystem->planet4            		= STRALLOC( "" );
	starsystem->gate1            		= STRALLOC( "" );
	starsystem->gate2            		= STRALLOC( "" );
	starsystem->gate3            		= STRALLOC( "" );
	starsystem->gate4            		= STRALLOC( "" );
	starsystem->gate5            		= STRALLOC( "" );
	starsystem->gate6            		= STRALLOC( "" );

    argument = one_argument( argument, arg );
    sprintf( filename, "%s.system" , strlower(arg) );
    starsystem->filename = str_dup( filename );
    save_starsystem( starsystem );
    write_starsystem_list();
}

void do_starsystems( CHAR_DATA *ch, char *argument )
{
    SPACE_DATA *starsystem;
    int count = 0;

    for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
    {
        set_char_color( AT_NOTE, ch );
        ch_printf( ch, "%s\n\r", starsystem->name );
        count++;
    }

    if ( !count )
    {
        send_to_char( "There are no starsystems currently formed.\n\r", ch );
		return;
    }
}

void echo_to_ship( int color , SHIP_DATA *ship , char *argument )
{
	int room;

	for ( room = ship->firstroom ; room <= ship->lastroom ;room++ )
		echo_to_room( color , get_room_index(room) , argument );
}

void sound_to_ship( SHIP_DATA *ship , char *argument )
{
    int roomnum;
    ROOM_INDEX_DATA *room;
	CHAR_DATA *vic;

    for ( roomnum = ship->firstroom ; roomnum <= ship->lastroom ;roomnum++ )
    {
    	room = get_room_index( roomnum );
        if ( room == NULL )
        	continue;
        for ( vic = room->first_person; vic; vic = vic->next_in_room )
        {
	   		//if ( !IS_NPC(vic) && IS_SET( vic->act, PLR_SOUND ) )
	     	//	send_to_char( argument, vic );  --removing sound for noproxmsg 2009-01-13 Funf
        }
    }
}

void echo_to_cockpit( int color , SHIP_DATA *ship , char *argument )
{
	int room;

    for ( room = ship->firstroom ; room <= ship->lastroom ;room++ )
    {
    	if ( room == ship->cockpit || room == ship->navseat
          || room == ship->pilotseat || room == ship->coseat
          || room == ship->gunseat || room == ship->engineroom
          || room == ship->turret1 || room == ship->turret2 )
        	echo_to_room( color , get_room_index(room) , argument );
    }
}

void echo_to_system( int color , SHIP_DATA *ship , char *argument , SHIP_DATA *ignore )
{
	SHIP_DATA *target;

    if (!ship->starsystem)
    	return;

    for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
    {
    	if (target != ship && target != ignore )
        	echo_to_cockpit( color , target , argument );
    }
}

bool is_facing( SHIP_DATA *ship , SHIP_DATA *target )
{
	float dy, dx, dz, hx, hy, hz;
    float cosofa;

    hx = ship->hx;
    hy = ship->hy;
    hz = ship->hz;

    dx = target->vx - ship->vx;
    dy = target->vy - ship->vy;
    dz = target->vz - ship->vz;

    cosofa = ( hx*dx + hy*dy + hz*dz ) / ( sqrt(hx*hx+hy*hy+hz*hz) + sqrt(dx*dx+dy*dy+dz*dz) );

    if ( cosofa > 0.75 )
    	return TRUE;

 	return FALSE;
}



SHIP_DATA * ship_in_room( ROOM_INDEX_DATA *room, char *name )
{
    SHIP_DATA *ship;

    if ( !room )
    	return NULL;

    for ( ship = room->first_ship ; ship ; ship = ship->next_in_room )
    	if ( !str_cmp( name, ship->name ) )
        	return ship;

    for ( ship = room->first_ship ; ship ; ship = ship->next_in_room )
    	if ( nifty_is_name_prefix( name, ship->name ) )
        	return ship;

    return NULL;
}

/*
 * Get pointer to ship structure from ship name.
 */
SHIP_DATA *get_ship( char *name )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( !str_cmp( name, ship->name ) )
        	return ship;

    for ( ship = first_ship; ship; ship = ship->next )
       	if ( nifty_is_name_prefix( name, ship->name ) )
         	return ship;

    return NULL;
}

/*
 * Checks if ships in a starsystem and returns poiner if it is.
 */
SHIP_DATA *get_ship_here( char *name , SPACE_DATA *starsystem)
{
    SHIP_DATA *ship;

    if ( starsystem == NULL )
    	return NULL;

    for ( ship = starsystem->first_ship ; ship; ship = ship->next_in_starsystem )
    	if ( !str_cmp( name, ship->name ) )
        	return ship;

    for ( ship = starsystem->first_ship; ship; ship = ship->next_in_starsystem )
       	if ( nifty_is_name_prefix( name, ship->name ) )
         	return ship;

    return NULL;
}


/*
 * Get pointer to ship structure from ship name.
 */
SHIP_DATA *ship_from_pilot( char *name )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( !str_cmp( name, ship->pilot ) )
        	return ship;
       	if ( !str_cmp( name, ship->copilot ) )
         	return ship;
       	if ( !str_cmp( name, ship->owner ) )
         	return ship;
    return NULL;
}


/*
 * Get pointer to ship structure from cockpit, turret, or entrance ramp vnum.
 */

SHIP_DATA *ship_from_cockpit( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->cockpit || vnum == ship->turret1 || vnum == ship->turret2
       	  || vnum == ship->pilotseat || vnum == ship->coseat || vnum == ship->navseat
          || vnum == ship->gunseat  || vnum == ship->engineroom )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_pilotseat( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->pilotseat )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_coseat( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->coseat )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_navseat( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->navseat )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_gunseat( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->gunseat )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_engine( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    {
    	if (ship->engineroom)
      	{
        	if ( vnum == ship->engineroom )
          		return ship;
      	}
      	else
      	{
        	if ( vnum == ship->cockpit )
          		return ship;
      	}
    }

    return NULL;
}



SHIP_DATA *ship_from_turret( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->gunseat || vnum == ship->turret1 || vnum == ship->turret2 )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_entrance( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->entrance )
        	return ship;
    return NULL;
}

SHIP_DATA *ship_from_hangar( int vnum )
{
    SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    	if ( vnum == ship->hangar )
        	return ship;
    return NULL;
}



/*
void fix_ship_space()
{
	SHIP_DATA *ship;

    for ( ship = first_ship; ship; ship = ship->next )
    {
	if(ship->wasflying == 1)
		ship_to_starsystem (ship, ship->currjump);
    }
}
*/



void do_speeders( CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *ship;
    int count;

    if ( !IS_NPC(ch) )
    {
      count = 0;
      send_to_char( "&YThe following are owned by you or by your organization:\n\r", ch );
      send_to_char( "\n\r&WVehicle                            Owner\n\r",ch);
      for ( ship = first_ship; ship; ship = ship->next )
      {
        if ( str_cmp(ship->owner, ch->name) )
        {
           if ( !ch->pcdata || !ch->pcdata->clan || str_cmp(ship->owner,ch->pcdata->clan->name) || ship->class <= SHIP_PLATFORM )
               continue;
        }
        if ( ship->location != ch->in_room->vnum || ship->class <= SHIP_PLATFORM)
               continue;

        if (ship->type == MOB_SHIP)
           continue;
        else if (ship->type == SHIP_REPUBLIC)
           set_char_color( AT_BLOOD, ch );
        else if (ship->type == SHIP_IMPERIAL)
           set_char_color( AT_DGREEN, ch );
        else
          set_char_color( AT_BLUE, ch );

        ch_printf( ch, "%-35s %-15s\n\r", ship->name, ship->owner );

        count++;
      }

      if ( !count )
      {
        send_to_char( "There are no land or air vehicles owned by you.\n\r", ch );
      }

    }


    count =0;
    send_to_char( "&Y\n\rThe following vehicles are parked here:\n\r", ch );

    send_to_char( "\n\r&WVehicle                            Owner          Cost/Rent\n\r", ch );
    for ( ship = first_ship; ship; ship = ship->next )
    {
        if ( ship->location != ch->in_room->vnum || ship->class <= SHIP_PLATFORM)
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

        if ( !str_cmp(ship->owner, "Public") )
        {
          ch_printf( ch, "%lld to rent.\n\r", get_ship_value(ship)/100 );
        }
        else if ( str_cmp(ship->owner, "") )
          ch_printf( ch, "%s", "\n\r" );
        else
           ch_printf( ch, "%lld to buy.\n\r", get_ship_value(ship) );

        count++;
    }

    if ( !count )
    {
        send_to_char( "There are no sea air or land vehicles here.\n\r", ch );
    }
}

void do_allspeeders( CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *ship;
    int count = 0;

      count = 0;
      send_to_char( "&Y\n\rThe following sea/land/air vehicles are currently formed:\n\r", ch );

      send_to_char( "\n\r&WVehicle                            Owner\n\r", ch );
      for ( ship = first_ship; ship; ship = ship->next )
      {
        if ( ship->class <= SHIP_PLATFORM )
           continue;

        if (ship->type == MOB_SHIP)
           continue;
        else if (ship->type == SHIP_REPUBLIC)
           set_char_color( AT_BLOOD, ch );
        else if (ship->type == SHIP_IMPERIAL)
           set_char_color( AT_DGREEN, ch );
        else
          set_char_color( AT_BLUE, ch );


        ch_printf( ch, "%-35s %-15s ", ship->name, ship->owner );

        if ( !str_cmp(ship->owner, "Public") )
        {
          ch_printf( ch, "%lld to rent.\n\r", get_ship_value(ship)/100 );
        }
        else if ( str_cmp(ship->owner, "") )
          ch_printf( ch, "%s", "\n\r" );
        else
           ch_printf( ch, "%lld to buy.\n\r", get_ship_value(ship) );

        count++;
      }

      if ( !count )
      {
        send_to_char( "There are none currently formed.\n\r", ch );
	return;
      }


}




void ship_to_starsystem( SHIP_DATA *ship , SPACE_DATA *starsystem )
{
	if ( starsystem == NULL )
    	return;

    if ( ship == NULL )
    	return;

    if ( starsystem->first_ship == NULL )
        starsystem->first_ship = ship;

    if ( starsystem->last_ship )
    {
    	starsystem->last_ship->next_in_starsystem = ship;
        ship->prev_in_starsystem = starsystem->last_ship;
    }

    starsystem->last_ship = ship;

    ship->starsystem = starsystem;

}

void new_missile( SHIP_DATA *ship , SHIP_DATA *target , CHAR_DATA *ch , int missiletype )
{
     SPACE_DATA *starsystem;
     MISSILE_DATA *missile;

     if ( ship  == NULL )
        return;

     if ( target  == NULL )
        return;

     if ( ( starsystem = ship->starsystem ) == NULL )
        return;

     CREATE( missile, MISSILE_DATA, 1 );
     LINK( missile, first_missile, last_missile, next, prev );

     missile->target = target;
     missile->fired_from = ship;
     if ( ch )
        missile->fired_by = STRALLOC( ch->name );
     else
        missile->fired_by = STRALLOC( "" );
     missile->missiletype = missiletype;
     missile->age =0;
     if ( missile->missiletype == HEAVY_BOMB )
       missile->speed = 20;
     else if ( missile->missiletype == PROTON_TORPEDO )
       missile->speed = 200;
     else if ( missile->missiletype == CONCUSSION_MISSILE )
       missile->speed = 300;
     else
       missile->speed = 50;

     missile->mx = ship->vx;
     missile->my = ship->vy;
     missile->mz = ship->vz;

     if ( starsystem->first_missile == NULL )
        starsystem->first_missile = missile;

     if ( starsystem->last_missile )
     {
         starsystem->last_missile->next_in_starsystem = missile;
         missile->prev_in_starsystem = starsystem->last_missile;
     }

     starsystem->last_missile = missile;

     missile->starsystem = starsystem;

}

void ship_from_starsystem( SHIP_DATA *ship , SPACE_DATA *starsystem )
{

	if ( starsystem == NULL )
    	return;

    if ( ship == NULL )
        return;

    if ( starsystem->last_ship == ship )
        starsystem->last_ship = ship->prev_in_starsystem;

    if ( starsystem->first_ship == ship )
        starsystem->first_ship = ship->next_in_starsystem;

    if ( ship->prev_in_starsystem )
        ship->prev_in_starsystem->next_in_starsystem = ship->next_in_starsystem;

    if ( ship->next_in_starsystem)
        ship->next_in_starsystem->prev_in_starsystem = ship->prev_in_starsystem;

    ship->starsystem = NULL;
    ship->next_in_starsystem = NULL;
    ship->prev_in_starsystem = NULL;

}

void extract_missile( MISSILE_DATA *missile )
{
    SPACE_DATA *starsystem;

     if ( missile == NULL )
        return;

     if ( ( starsystem = missile->starsystem ) != NULL )
     {

      if ( starsystem->last_missile == missile )
        starsystem->last_missile = missile->prev_in_starsystem;

      if ( starsystem->first_missile == missile )
        starsystem->first_missile = missile->next_in_starsystem;

      if ( missile->prev_in_starsystem )
        missile->prev_in_starsystem->next_in_starsystem = missile->next_in_starsystem;

      if ( missile->next_in_starsystem)
        missile->next_in_starsystem->prev_in_starsystem = missile->prev_in_starsystem;

      missile->starsystem = NULL;
      missile->next_in_starsystem = NULL;
      missile->prev_in_starsystem = NULL;

     }

     UNLINK( missile, first_missile, last_missile, next, prev );

     missile->target = NULL;
     missile->fired_from = NULL;
     if (  missile->fired_by )
        STRFREE( missile->fired_by );

     DISPOSE( missile );

}

bool is_rental( CHAR_DATA *ch , SHIP_DATA *ship )
{
	if ( !str_cmp("Public",ship->owner) )
    	return TRUE;

   	return FALSE;
}

bool check_pilot( CHAR_DATA *ch , SHIP_DATA *ship )
{
   	if ( !str_cmp(ch->name,ship->owner) || !str_cmp(ch->name,ship->pilot)
      || !str_cmp(ch->name,ship->copilot) || !str_cmp("Public",ship->owner) )
    	return TRUE;

   	if ( !IS_NPC(ch) && ch->pcdata && ch->pcdata->clan )
   	{
    	if ( !str_cmp(ch->pcdata->clan->name,ship->owner) || !str_cmp(ch->pcdata->clan->name,ship->pilot)
    		|| !str_cmp(ch->pcdata->clan->name,ship->copilot) )
        	return TRUE;
   	}

   	return FALSE;
}

bool extract_ship( SHIP_DATA *ship )
{
   	ROOM_INDEX_DATA *room;

    if ( ( room = ship->in_room ) != NULL )
    {
    	UNLINK( ship, room->first_ship, room->last_ship, next_in_room, prev_in_room );
        ship->in_room = NULL;
    }
    return TRUE;
}

void damage_ship_ch( SHIP_DATA *ship , int min , int max , CHAR_DATA *ch )
{
    int damage;
    long long int xp;

    damage = number_range( min , max );

    xp = ( exp_level( ch->skill_level[PILOTING_ABILITY]+1) - exp_level( ch->skill_level[PILOTING_ABILITY]) ) / 25 ;
    xp = UMIN( get_ship_value( ship ) /100 , xp ) ;
    gain_exp( ch , xp , PILOTING_ABILITY );

    /* if ( damage > 0 )
    {
    	if ( number_range(1, 100) <= 1 && ship->shipstate != SHIP_DISABLED )
        {
        	echo_to_cockpit( AT_BLOOD + AT_BLINK , ship , "Ships Drive DAMAGED!" );
           	ship->shipstate = SHIP_DISABLED;
        }
        if ( number_range(1, 100) <= 2 && ship->statet1 != LASER_DAMAGED && ship->turret1 )
        {
           	echo_to_room( AT_BLOOD + AT_BLINK , get_room_index(ship->turret1) , "Turret DAMAGED!" );
           	ship->statet1 = LASER_DAMAGED;
        }
        if ( number_range(1, 100) <= 2 && ship->statet2 != LASER_DAMAGED && ship->turret2 )
        {
           	echo_to_room( AT_BLOOD + AT_BLINK , get_room_index(ship->turret2) , "Turret DAMAGED!" );
           	ship->statet2 = LASER_DAMAGED;
        }
    } */ // Moved to msweapons.c -> shoot_weapon --Funf

    {
		int roomnum;
		ROOM_INDEX_DATA * room;
		CHAR_DATA *	vic;
		char cmd[MAX_STRING_LENGTH];

		for ( roomnum = ship->firstroom ; roomnum <= ship->lastroom ;roomnum++ )
		{
			room = get_room_index(roomnum);
			for ( vic = room->first_person; vic ; vic = vic->next_in_room )
			{
				if ( vic->adren < 5 ) vic->adren++;
			}
		}
	}


    ship->hull -= damage;

    if ( ship->hull <= 0 )
    {
    	destroy_ship( ship , ch );

       	xp =  ( exp_level( ch->skill_level[PILOTING_ABILITY]+1) - exp_level( ch->skill_level[PILOTING_ABILITY]) );
       	xp = UMIN( get_ship_value( ship ) , xp );
       	gain_exp( ch , xp , PILOTING_ABILITY);
       	ch_printf( ch, "&WYou gain %lld piloting experience!\n\r", xp );
       	return;
    }

    if ( ship->hull <= ship->maxhull/20 )
       	echo_to_cockpit( AT_BLOOD+ AT_BLINK , ship , "WARNING! Ship hull severely damaged!" );

}

void damage_ship( SHIP_DATA *ship , int min , int max )
{
    int damage;

    damage = number_range( min , max );

    if ( damage > 0 )
    {
        if ( number_range(1, 100) <= 1 && ship->shipstate != SHIP_DISABLED )
        {
        	echo_to_cockpit( AT_BLOOD + AT_BLINK , ship , "Ships Drive DAMAGED!" );
        	ship->shipstate = SHIP_DISABLED;
        }
        if ( number_range(1, 100) <= 2 && ship->statet1 != LASER_DAMAGED && ship->turret1 )
        {
           	echo_to_room( AT_BLOOD + AT_BLINK , get_room_index(ship->turret1) , "Turret DAMAGED!" );
           	ship->statet1 = LASER_DAMAGED;
        }
        if ( number_range(1, 100) <= 2 && ship->statet2 != LASER_DAMAGED && ship->turret2 )
        {
           	echo_to_room( AT_BLOOD + AT_BLINK , get_room_index(ship->turret2) , "Turret DAMAGED!" );
           	ship->statet2 = LASER_DAMAGED;
        }
    }

    ship->hull -= damage;

    if ( ship->hull <= 0 )
    {
       destroy_ship( ship , NULL );
       return;
    }

    if ( ship->hull <= ship->maxhull/20 )
       echo_to_cockpit( AT_BLOOD+ AT_BLINK , ship , "WARNING! Ship hull severely damaged!" );

}

void destroy_ship( SHIP_DATA *ship , CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    int  roomnum;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *robj;
    CHAR_DATA *rch;

    sprintf( buf , "%s explodes in a blinding flash of light!", ship->name );
    echo_to_system( AT_WHITE + AT_BLINK , ship , buf , NULL );

    if ( ship->class == FIGHTER_SHIP )

    echo_to_ship( AT_WHITE + AT_BLINK , ship , "A blinding flash of light burns your eyes...");
    echo_to_ship( AT_WHITE , ship , "But before you have a chance to scream...\n\rYou are ripped apart as your spacecraft explodes...");

    for ( roomnum = ship->firstroom ; roomnum <= ship->lastroom ; roomnum++ )
    {
    	room = get_room_index(roomnum);

        if (room != NULL)
        {
        	rch = room->first_person;
        	while ( rch )
          	{
            	if ( IS_IMMORTAL(rch) )
             	{
               		char_from_room(rch);
               		char_to_room( rch, get_room_index(wherehome(rch)) );
             	}
             	else
             	{
               		if ( ch && rch->pcdata ) /* Don't log mob deaths - Funf */
					{
						sprintf( log_buf, "%s killed by %s at %d (In the Ship %s)",
                			rch->name,
               		 		(IS_NPC(ch) ? ch->short_descr : ch->name),
             		   		rch->in_room->vnum,
							ship->name );
            			log_string( log_buf );
            			to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );

						if( get_bounty( rch->name ) )
							claim_bounty(ch, rch);
						else
            				raw_kill( ch , rch );
					}
               		else
                 		raw_kill( rch , rch );
             	}
          		rch = room->first_person;

          	}

          	for ( robj = room->first_content ; robj ; robj = robj->next_content )
          	{
				if ( !IS_SET( room->room_flags, ROOM_CLANSTOREROOM ) ) /* Don't destroy clanstorerooms on ships, 'kay? */
				{
            		separate_obj( robj );
            		extract_obj( robj );
				}
          	}
    	}
    }
    resetship(ship);

}

bool ship_to_room(SHIP_DATA *ship , int vnum )
{
    ROOM_INDEX_DATA *shipto;

    if ( (shipto=get_room_index(vnum)) == NULL )
    	return FALSE;
    LINK( ship, shipto->first_ship, shipto->last_ship, next_in_room, prev_in_room );
    ship->in_room = shipto;
    return TRUE;
}


void do_board( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *fromroom;
   	ROOM_INDEX_DATA *toroom;
   	SHIP_DATA *ship;

   	if ( !argument || argument[0] == '\0')
   	{
       	send_to_char( "Board what?\n\r", ch );
       	return;
   	}

   	if ( ( ship = ship_in_room( ch->in_room , argument ) ) == NULL )
   	{
    	act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
        return;
   	}

   	if ( IS_SET( ch->act, ACT_MOUNTED ) )
   	{
    	act( AT_PLAIN, "You can't go in there riding THAT.", ch, NULL, argument, TO_CHAR );
        return;
   	}

   	fromroom = ch->in_room;

    if ( ( toroom = get_room_index( ship->entrance ) ) != NULL )
   	{
   		if ( ! ship->hatchopen )
   	   	{
   	    	send_to_char( "&RThe hatch is closed!\n\r", ch);
   	      	return;
   	   	}

   		if ( ship->hull == 0 )
   	   	{
   	    	send_to_char( "&RThe hull is not sturdy enough to board!\n\r", ch);
   	      	return;
   	   	}

        if ( toroom->tunnel > 0 )
        {
	        CHAR_DATA *ctmp;
	        int count = 0;

	       	for ( ctmp = toroom->first_person; ctmp; ctmp = ctmp->next_in_room )
	       		if ( ++count >= toroom->tunnel )
	       		{
            		send_to_char( "There is no room for you in there.\n\r", ch );
		  			return;
	       		}
        }

        if ( ship->shipstate == SHIP_LAUNCH || ship->shipstate == SHIP_LAUNCH_2 || ship->shipstate == SHIP_LAUNCH_3 )
        {
        	send_to_char("&rThat ship has already started launching!\n\r",ch);
            return;
        }

        act( AT_PLAIN, "$n enters $T.", ch, NULL, ship->name , TO_ROOM );
	    act( AT_PLAIN, "You enter $T.", ch, NULL, ship->name , TO_CHAR );
   	    char_from_room( ch );
   	    char_to_room( ch , toroom );
   	    act( AT_PLAIN, "$n enters the ship.", ch, NULL, argument , TO_ROOM );
        do_look( ch , "auto" );
        calc_ship_speed( ship ); /* Dynamically calculate speed - Funf */

	}
    else
    	send_to_char("That ship has no entrance!\n\r", ch);
}

bool rent_ship( CHAR_DATA *ch , SHIP_DATA *ship )
{
	long long int price;

    if ( IS_NPC ( ch ) )
    	return FALSE;

    price = get_ship_value( ship )/100;

    if ( ch->gold < price )
    {
    	ch_printf(ch, "&RRenting this ship costs %lld. You don't have enough wulongs!\n\r" , price );
        return FALSE;
    }

    ch->gold -= price;
    ch_printf(ch, "&GYou pay %lld wulongs to rent the ship.\n\r" , price );
    return TRUE;

}

void do_leaveship( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *fromroom;
    ROOM_INDEX_DATA *toroom;
    SHIP_DATA *ship;

    fromroom = ch->in_room;

    if  ( (ship = ship_from_entrance(fromroom->vnum)) == NULL )
    {
    	send_to_char( "I see no exit here.\n\r" , ch );
        return;
    }

    if  ( ship->class == SHIP_PLATFORM )
    {
        send_to_char( "You can't do that here.\n\r" , ch );
        return;
    }

    if ( ship->lastdoc != ship->location )
    {
        send_to_char("&rMaybe you should wait until the ship lands.\n\r",ch);
        return;
    }

    if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
    {
        send_to_char("&rPlease wait till the ship is properly docked.\n\r",ch);
        return;
    }

    if ( ! ship->hatchopen )
    {
    	send_to_char("&RYou need to open the hatch first.\n\r" , ch );
    	return;
    }

    if ( ( toroom = get_room_index( ship->location ) ) != NULL )
    {
        act( AT_PLAIN, "$n exits the ship.", ch, NULL, argument , TO_ROOM );
	    act( AT_PLAIN, "You exit the ship.", ch, NULL, argument , TO_CHAR );
   	    char_from_room( ch );
   	    char_to_room( ch , toroom );
   	    act( AT_PLAIN, "$n steps out of a ship.", ch, NULL, argument , TO_ROOM );
        do_look( ch , "auto" );
        calc_ship_speed( ship ); /* Dynamically calculate speed - Funf */
    }
    else
    	send_to_char ( "The exit doesn't seem to be working properly.\n\r", ch );
}

void do_launch( CHAR_DATA *ch, char *argument )
{
    int chance;
    long long int price = 0;
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

    if  ( ship->realspeed <= 0 ) /* Mass support - Funf */
    {
		send_to_char( "&RThe ship is too heavy!\n\r" , ch );
		return;
	}

    if  ( ship->energy <= 0 ) /* Bug fix - Funf 2007-04-03 */
    {
		send_to_char( "&RTry launching when you have some fuel!\n\r" , ch );
		return;
	}

    if ( !check_pilot( ch , ship ) )
    {
        send_to_char("&RHey, thats not your ship! Try renting a public one.\n\r",ch);
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

	if ( ship->class == FIGHTER_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
	if ( ship->class == MIDSIZE_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_midships]) ;
	if ( ship->class == CAPITAL_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_capitalships]);
	if ( number_percent( ) < chance )
	{
		if ( is_rental(ch,ship) )
			if( !rent_ship(ch,ship) )
				return;
		if ( !is_rental(ch,ship) )
		{
    		if ( ship->class == FIGHTER_SHIP )
            	price=20;
            if ( ship->class == MIDSIZE_SHIP )
                price=50;
            if ( ship->class == CAPITAL_SHIP )
                price=500;

// No hull on launch -Spike         price += ( ship->maxhull-ship->hull );
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
        }

    	if ( ch->pcdata && ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name,ship->owner) )
        {
        	if ( ch->pcdata->clan->funds < price )
            {
            	ch_printf(ch, "&R%s doesn't have enough funds to prepare this ship for launch.\n\r", ch->pcdata->clan->name );
                return;
            }

            ch->pcdata->clan->funds -= price;
            ch_printf(ch, "&GIt costs %s %lld wulongs to ready this ship for launch.\n\r", ch->pcdata->clan->name, price );
        }
        else if ( str_cmp( ship->owner , "Public" ) )
        {
        	if ( ch->gold < price )
            {
            	ch_printf(ch, "&RYou don't have enough funds to prepare this ship for launch.\n\r");
                return;
            }

            ch->gold -= price;
            ch_printf(ch, "&GYou pay %lld wulongs to ready the ship for launch.\n\r", price );

       	}

        ship->energy = ship->maxenergy;
        ship->chaff = ship->maxchaff;
        ship->missiles = ship->maxmissiles;
       	ship->torpedos = ship->maxtorpedos;
       	ship->rockets = ship->maxrockets;
       	ship->autorecharge = FALSE;
       	ship->autotrack = FALSE;
       	ship->autospeed = FALSE;
// No repair on launch -Spike       	ship->hull = ship->maxhull;
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

			if ( ship_from_hangar( ship->location ) ) /* Launching from land dock would send NULL pointer - Funf */
    			calc_ship_speed( ship_from_hangar( ship->location ) ); /* Dynamically calculate speed - Funf */

    		if (ship->manuever > 100)
    			ship->shipstate = SHIP_LAUNCH_2;
    		else
    			ship->shipstate = SHIP_LAUNCH;

    		ship->currspeed = ship->realspeed;

    		if ( ship->class == FIGHTER_SHIP )
            	learn_from_success( ch, gsn_starfighters );
            if ( ship->class == MIDSIZE_SHIP )
                learn_from_success( ch, gsn_midships );
            if ( ship->class == CAPITAL_SHIP )
                learn_from_success( ch, gsn_capitalships );
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

void launchship( SHIP_DATA *ship )
{
	char buf[MAX_STRING_LENGTH];
    SHIP_DATA *target;
    int plusminus;

    ship_to_starsystem( ship, starsystem_from_vnum( ship->location ) );


    if ( ship->starsystem == NULL )
    {
    	echo_to_room( AT_YELLOW , get_room_index(ship->pilotseat) , "Launch path blocked .. Launch aborted.");
       	echo_to_ship( AT_YELLOW , ship , "The ship slowly sets back back down on the landing pad.");
       	sprintf( buf ,  "%s slowly sets back down." ,ship->name );
       	echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
       	ship->shipstate = SHIP_DOCKED;
       	return;
    }

    extract_ship(ship);

    if ( ship_from_hangar( ship->location ) ) /* Launching from land dock would send NULL pointer - Funf */
	    calc_ship_speed( ship_from_hangar( ship->location ) ); /* Dynamically calculate speed - Funf */

    ship->location = 0;


    if (ship->shipstate != SHIP_DISABLED)
       	ship->shipstate = SHIP_READY;

    plusminus = number_range ( -1 , 2 );
    if (plusminus > 0 )
        ship->hx = 1;
    else
        ship->hx = -1;

    plusminus = number_range ( -1 , 2 );
    if (plusminus > 0 )
       	ship->hy = 1;
    else
        ship->hy = -1;

    plusminus = number_range ( -1 , 2 );
    if (plusminus > 0 )
        ship->hz = 1;
    else
        ship->hz = -1;

    if (ship->lastdoc == ship->starsystem->doc1a ||
        ship->lastdoc == ship->starsystem->doc1b ||
        ship->lastdoc == ship->starsystem->doc1c )
    {
    	ship->vx = ship->starsystem->p1x;
    	ship->vy = ship->starsystem->p1y;
       	ship->vz = ship->starsystem->p1z;
    }
    else if (ship->lastdoc == ship->starsystem->doc2a ||
             ship->lastdoc == ship->starsystem->doc2b ||
             ship->lastdoc == ship->starsystem->doc2c )
    {
       	ship->vx = ship->starsystem->p2x;
       	ship->vy = ship->starsystem->p2y;
       	ship->vz = ship->starsystem->p2z;
    }
    else if (ship->lastdoc == ship->starsystem->doc3a ||
             ship->lastdoc == ship->starsystem->doc3b ||
             ship->lastdoc == ship->starsystem->doc3c )
    {
       	ship->vx = ship->starsystem->p3x;
       	ship->vy = ship->starsystem->p3y;
       	ship->vz = ship->starsystem->p3z;
    }
    else if (ship->lastdoc == ship->starsystem->doc4a ||
             ship->lastdoc == ship->starsystem->doc4b ||
             ship->lastdoc == ship->starsystem->doc4c )
    {
       	ship->vx = ship->starsystem->p4x;
       	ship->vy = ship->starsystem->p4y;
       	ship->vz = ship->starsystem->p4z;
    }
    else
    {
    	for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
      	{
        	if (ship->lastdoc == target->hangar)
          	{
             	ship->vx = target->vx;
             	ship->vy = target->vy;
             	ship->vz = target->vz;
          	}
       	}
    }

    ship->energy -= (100+100*ship->class);

    ship->vx += (ship->hx*ship->currspeed*2);
    ship->vy += (ship->hy*ship->currspeed*2);
    ship->vz += (ship->hz*ship->currspeed*2);

    echo_to_room( AT_GREEN , get_room_index(ship->location) , "Launch complete.\n\r");
    echo_to_ship( AT_YELLOW , ship , "The ship leaves the platform far behind as it flies into space." );
    sprintf( buf ,"%s enters the starsystem at %.0f %.0f %.0f" , ship->name, ship->vx, ship->vy, ship->vz );
    echo_to_system( AT_YELLOW, ship, buf , NULL );
    sprintf( buf, "%s lifts off into space.", ship->name );
    echo_to_room( AT_YELLOW , get_room_index(ship->lastdoc) , buf );

}


void do_land( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    SHIP_DATA *ship;
    SHIP_DATA *target;
    int vx, vy ,vz;

    strcpy( arg, argument );

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
		send_to_char("&RYou need to be in the pilot seat!\n\r",ch);
		return;
	}

	if ( autofly(ship) )
	{
		send_to_char("&RYou'll have to turn off the ships autopilot first.\n\r",ch);
		return;
	}
	// Adrenaline prevents landing - Gatz
	if(ch->adren > 0)
	{
		send_to_char("&RYour heart is racing too fast for that!\n\r", ch);
		return;
	}
	if  ( ship->class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't land platforms\n\r" , ch );
		return;
	}

	if (ship->shipstate == SHIP_DISABLED)
	{
		send_to_char("&RThe ships drive is disabled. Unable to land.\n\r",ch);
		return;
	}

        if (ship->class == CAPITAL_SHIP)
        {
		send_to_char("&RYou can't land this!\n\r" ,ch);
		return;
	}

	if (ship->shipstate == SHIP_DOCKED)
	{
		send_to_char("&RThe ship is already docked!\n\r",ch);
		return;
	}

	if (ship->shipstate == SHIP_HYPERSPACE)
	{
		send_to_char("&RYou can only do that in realspace!\n\r",ch);
		return;
	}

	if (ship->shipstate != SHIP_READY)
	{
		send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
		return;
	}

	if ( ship->starsystem == NULL )
	{
		send_to_char("&RThere's nowhere to land around here!",ch);
		return;
	}

	if ( ship->energy < (25 + 25*ship->class) )
	{
		send_to_char("&RTheres not enough fuel!\n\r",ch);
		return;
	}

	if ( argument[0] == '\0' )
	{
		set_char_color(  AT_CYAN, ch );
		ch_printf(ch, "%s" , "Land where?\n\r\n\rChoices: ");

		if ( ship->starsystem->doc1a )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location1a,
    	                        ship->starsystem->planet1,
    	                        ship->starsystem->p1x,
    	                        ship->starsystem->p1y,
    	                        ship->starsystem->p1z );
		if ( ship->starsystem->doc1b )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location1b,
    	                        ship->starsystem->planet1,
    	                        ship->starsystem->p1x,
    	                        ship->starsystem->p1y,
    	                        ship->starsystem->p1z );
		if ( ship->starsystem->doc1c )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location1c,
    	                        ship->starsystem->planet1,
    	                        ship->starsystem->p1x,
    	                        ship->starsystem->p1y,
    	                        ship->starsystem->p1z );
		if ( ship->starsystem->doc2a )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location2a,
    	                        ship->starsystem->planet2,
    	                        ship->starsystem->p2x,
    	                        ship->starsystem->p2y,
    	                        ship->starsystem->p2z );
		if ( ship->starsystem->doc2b )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location2b,
    	                        ship->starsystem->planet2,
    	                        ship->starsystem->p2x,
    	                        ship->starsystem->p2y,
    	                        ship->starsystem->p2z );
		if ( ship->starsystem->doc2c )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location2c,
    	                        ship->starsystem->planet2,
    	                        ship->starsystem->p2x,
    	                        ship->starsystem->p2y,
    	                        ship->starsystem->p2z );
		if ( ship->starsystem->doc3a )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location3a,
    	                        ship->starsystem->planet3,
    	                        ship->starsystem->p3x,
    	                        ship->starsystem->p3y,
    	                        ship->starsystem->p3z );
		if ( ship->starsystem->doc3b )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location3b,
    	                        ship->starsystem->planet3,
    	                        ship->starsystem->p3x,
    	                        ship->starsystem->p3y,
    	                        ship->starsystem->p3z );
		if ( ship->starsystem->doc3c )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location3c,
    	                        ship->starsystem->planet3,
    	                        ship->starsystem->p3x,
    	                        ship->starsystem->p3y,
    	                        ship->starsystem->p3z );
		if ( ship->starsystem->doc4a )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location4a,
    	                        ship->starsystem->planet4,
    	                        ship->starsystem->p4x,
    	                        ship->starsystem->p4y,
    	                        ship->starsystem->p4z );
		if ( ship->starsystem->doc4b )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location4b,
    	                        ship->starsystem->planet4,
    	                        ship->starsystem->p4x,
    	                        ship->starsystem->p4y,
    	                        ship->starsystem->p4z );
		if ( ship->starsystem->doc4c )
			ch_printf(ch, "%s (%s)  %d %d %d\n\r         " ,
    	                        ship->starsystem->location4c,
    	                        ship->starsystem->planet4,
    	                        ship->starsystem->p4x,
    	                        ship->starsystem->p4y,
    	                        ship->starsystem->p4z );
		for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
		{
			if ( target->hangar > 0 && target != ship)
				ch_printf(ch, "%s    %.0f %.0f %.0f\n\r         ",
                           	target->name,
                           	target->vx,
                           	target->vy,
                           	target->vz);
		}
		ch_printf(ch, "\n\rYour Coordinates: %.0f %.0f %.0f\n\r" , ship->vx , ship->vy, ship->vz);
			return;
	}

    if ( str_prefix(argument,ship->starsystem->location1a) &&
         str_prefix(argument,ship->starsystem->location2a) &&
         str_prefix(argument,ship->starsystem->location3a) &&
         str_prefix(argument,ship->starsystem->location4a) &&
         str_prefix(argument,ship->starsystem->location1b) &&
         str_prefix(argument,ship->starsystem->location2b) &&
         str_prefix(argument,ship->starsystem->location3b) &&
         str_prefix(argument,ship->starsystem->location4b) &&
         str_prefix(argument,ship->starsystem->location1c) &&
         str_prefix(argument,ship->starsystem->location2c) &&
         str_prefix(argument,ship->starsystem->location3c) &&
         str_prefix(argument,ship->starsystem->location4c) )
    {
    	target = get_ship_here( argument , ship->starsystem );
        if ( target == NULL )
        {
       		send_to_char("&RI don't see that here. Type land by itself for a list\n\r",ch);
    	    	return;
    	}
    	if ( target == ship )
    	{
    	    	send_to_char("&RYou can't land your ship inside itself!\n\r",ch);
    	    	return;
    	}
    		if ( ! target->hangar )
    		{
			send_to_char("&RThat ship has no hangar for you to land in!\n\r",ch);
			return;
		}
		if ( ship->class == MIDSIZE_SHIP && target->class == MIDSIZE_SHIP )
		{
			send_to_char("&RThat ship is not big enough for your ship to land in!\n\r",ch);
			return;
		}
		if (ship->class == CAPITAL_SHIP && target->class != SHIP_PLATFORM )
		{
			send_to_char("&RCapital ships can only dock on platforms.\n\r",ch);
			return;
		}
		if ( ! target->bayopen )
		{
			send_to_char("&RTheir hangar is closed. You'll have to ask them to open it for you\n\r",ch);
			return;
		}
		if (  (target->vx > ship->vx + 200) || (target->vx < ship->vx - 200) ||
		      (target->vy > ship->vy + 200) || (target->vy < ship->vy - 200) ||
    	      (target->vz > ship->vz + 200) || (target->vz < ship->vz - 200) )
    		{
    		send_to_char("&RThat ship is too far away! You'll have to fly a little closer.\n\r",ch);
			return;
		}
	}
	else
	{
		if ( !str_prefix(argument,ship->starsystem->location4a) ||
		     !str_prefix(argument,ship->starsystem->location4b) ||
		     !str_prefix(argument,ship->starsystem->location4c) )
		{
			vx = ship->starsystem->p4x;
			vy = ship->starsystem->p4y;
			vz = ship->starsystem->p4z;
		}
		if ( !str_prefix(argument,ship->starsystem->location3a) ||
		     !str_prefix(argument,ship->starsystem->location3b) ||
		     !str_prefix(argument,ship->starsystem->location3c) )
		{
			vx = ship->starsystem->p3x;
			vy = ship->starsystem->p3y;
			vz = ship->starsystem->p3z;
		}
		if ( !str_prefix(argument,ship->starsystem->location2a) ||
    	     !str_prefix(argument,ship->starsystem->location2b) ||
    	     !str_prefix(argument,ship->starsystem->location2c) )
		{
			vx = ship->starsystem->p2x;
			vy = ship->starsystem->p2y;
			vz = ship->starsystem->p2z;
		}
		if ( !str_prefix(argument,ship->starsystem->location1a) ||
			 !str_prefix(argument,ship->starsystem->location1b) ||
			 !str_prefix(argument,ship->starsystem->location1c) )
		{
			vx = ship->starsystem->p1x;
			vy = ship->starsystem->p1y;
			vz = ship->starsystem->p1z;
		}
		if (  (vx > ship->vx + 200) || (vx < ship->vx - 200) ||
		      (vy > ship->vy + 200) || (vy < ship->vy - 200) ||
			  (vz > ship->vz + 200) || (vz < ship->vz - 200) )
		{
			send_to_char("&R That platform is too far away! You'll have to fly a little closer.\n\r",ch);
			return;
		}
	}

	if ( ship->class == FIGHTER_SHIP )
	    chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
	if ( ship->class == MIDSIZE_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_midships]) ;
	if ( number_percent( ) < chance )
	{
		set_char_color( AT_GREEN, ch );
		send_to_char( "Landing sequence initiated.\n\r", ch);
		act( AT_PLAIN, "$n begins the landing sequence.", ch, NULL, argument , TO_ROOM );
		echo_to_ship( AT_YELLOW , ship , "The ship slowly begins its landing approach.");
		ship->dest = STRALLOC(arg);
		ship->shipstate = SHIP_LAND;

		calc_ship_speed( ship_from_hangar( ship->location ) ); /* Dynamically calculate speed - Funf */

		ship->currspeed = 0;
		if ( ship->class == FIGHTER_SHIP )
			learn_from_success( ch, gsn_starfighters );
		if ( ship->class == MIDSIZE_SHIP )
			learn_from_success( ch, gsn_midships );
		if ( starsystem_from_vnum(ship->lastdoc) != ship->starsystem )
		{
			if(ch->skill_level[PILOTING_ABILITY] < 10)
			{
			int xp =  (exp_level( ch->skill_level[PILOTING_ABILITY]+1) - exp_level( ch->skill_level[PILOTING_ABILITY])) ;
			xp = UMIN( get_ship_value( ship ) , xp );
			gain_exp( ch , xp , PILOTING_ABILITY );
			ch_printf( ch, "&WYou gain %lld points of flight experience!\n\r", UMIN( get_ship_value( ship ) , xp ) );
			}
			else
			{
			send_to_char("Your level is too high to gain experience for using this skill.\n\r", ch);
			}
		}
		return;
	}
	send_to_char("You fail to work the controls properly.\n\r",ch);
	if ( ship->class == FIGHTER_SHIP )
		learn_from_failure( ch, gsn_starfighters );
	else
		learn_from_failure( ch, gsn_midships );
	return;
}

void landship( SHIP_DATA *ship, char *arg )
{
    SHIP_DATA *target;
    char buf[MAX_STRING_LENGTH];
    int destination;

    if ( !str_prefix(arg,ship->starsystem->location4a) )
		destination = ship->starsystem->doc4a;
    if ( !str_prefix(arg,ship->starsystem->location4b) )
       	destination = ship->starsystem->doc4b;
    if ( !str_prefix(arg,ship->starsystem->location4c) )
       	destination = ship->starsystem->doc4c;
    if ( !str_prefix(arg,ship->starsystem->location3a) )
		destination = ship->starsystem->doc3a;
    if ( !str_prefix(arg,ship->starsystem->location3b) )
       	destination = ship->starsystem->doc3b;
    if ( !str_prefix(arg,ship->starsystem->location3c) )
       	destination = ship->starsystem->doc3c;
    if ( !str_prefix(arg,ship->starsystem->location2a) )
       	destination = ship->starsystem->doc2a;
    if ( !str_prefix(arg,ship->starsystem->location2b) )
       	destination = ship->starsystem->doc2b;
    if ( !str_prefix(arg,ship->starsystem->location2c) )
       	destination = ship->starsystem->doc2c;
    if ( !str_prefix(arg,ship->starsystem->location1a) )
       	destination = ship->starsystem->doc1a;
    if ( !str_prefix(arg,ship->starsystem->location1b) )
       	destination = ship->starsystem->doc1b;
    if ( !str_prefix(arg,ship->starsystem->location1c) )
       	destination = ship->starsystem->doc1c;

    target = get_ship_here( arg , ship->starsystem );
    if ( target != ship && target != NULL && target->bayopen
                        && ( ship->class != MIDSIZE_SHIP || target->class != MIDSIZE_SHIP ) )
    	destination = target->hangar;

    if ( !ship_to_room( ship , destination ) )
    {
       	echo_to_room( AT_YELLOW , get_room_index(ship->pilotseat), "Could not complete approach. Landing aborted.");
       	echo_to_ship( AT_YELLOW , ship , "The ship pulls back up out of its landing sequence.");
       	if (ship->shipstate != SHIP_DISABLED)
        	ship->shipstate = SHIP_READY;
       	return;
    }

    echo_to_room( AT_YELLOW , get_room_index(ship->pilotseat), "Landing sequence complete.");
    echo_to_ship( AT_YELLOW , ship , "You feel a slight thud as the ship sets down on the ground.");
    sprintf( buf ,"%s disappears from your scanner." , ship->name  );
    echo_to_system( AT_YELLOW, ship, buf , NULL );

    ship->location = destination;
    ship->lastdoc = ship->location;
    if (ship->shipstate != SHIP_DISABLED)
       	ship->shipstate = SHIP_DOCKED;
    ship_from_starsystem(ship, ship->starsystem);

    sprintf( buf, "%s lands on the platform.", ship->name );
    echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );

    ship->energy = ship->energy - 25 - 25*ship->class;

    if ( !str_cmp("Public",ship->owner) )
    {
       	ship->energy = ship->maxenergy;
       	ship->chaff = ship->maxchaff;
       	ship->missiles = ship->maxmissiles;
       	ship->torpedos = ship->maxtorpedos;
       	ship->rockets = ship->maxrockets;
       	ship->autorecharge = FALSE;
       	ship->autotrack = FALSE;
       	ship->autospeed = FALSE;
// Ship hull repair removed -Spike      	ship->hull = ship->maxhull;
       	ship->ammo = ship->maxammo;

       	ship->missilestate = MISSILE_READY;
       	ship->statet0 = LASER_READY;
       	ship->statet1 = LASER_READY;
       	ship->statet2 = LASER_READY;
       	ship->shipstate = SHIP_DOCKED;

       	echo_to_cockpit( AT_YELLOW , ship , "Repairing, refueling, and reloading ship..." );
    }

       	save_ship(ship);
}

void do_accelerate( CHAR_DATA *ch, char *argument )
{
    int chance;
    int change;
    SHIP_DATA *ship;
    char buf[MAX_STRING_LENGTH];

	if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
		return;
	}

	if ( ship->class > SHIP_PLATFORM )
	{
		send_to_char("&RThis isn't a spacecraft!\n\r",ch);
		return;
	}

	if (  (ship = ship_from_pilotseat(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RThe controls must be at the pilots chair...\n\r",ch);
		return;
	}

	if ( autofly(ship) )
	{
		send_to_char("&RYou'll have to turn off the ships autopilot first.\n\r",ch);
		return;
	}

	if  ( ship->class == SHIP_PLATFORM )
	{
		send_to_char( "&RPlatforms can't move!\n\r" , ch );
		return;
	}

	if (ship->shipstate == SHIP_DISABLED)
	{
		send_to_char("&RThe ships drive is disabled. Unable to accelerate.\n\r",ch);
		return;
	}

	if (ship->shipstate == SHIP_DOCKED)
	{
		send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
		return;
	}

	if ( ship->energy < abs((atoi(argument) - abs(ship->currspeed))/10) )
	{
		send_to_char("&RTheres not enough fuel!\n\r",ch);
		return;
	}

	if ( ship->class == FIGHTER_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
	if ( ship->class == MIDSIZE_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_midships]) ;
	if ( ship->class == CAPITAL_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_capitalships]);
	if ( number_percent( ) >= chance )
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

    change = atoi(argument);

    act( AT_PLAIN, "$n manipulates the ships controls.", ch,
    NULL, argument , TO_ROOM );

    if ( change < 0 )
    {
		send_to_char( "&RNo negative speeds!\n\r", ch);
		return;
    }

    if ( change > ship->realspeed )
    {
       	send_to_char( "&RYour ship can't go that fast!\n\r", ch);
	   	return;
    }

    if ( change == 0 && ship->shipstate == SHIP_HYPERSPACE)
    {
    	send_to_char("&RYou can't stop in hyperspace!\n\r",ch);
    	return;
    }

    if ( change > ship->currspeed )
    {
       send_to_char( "&GAccelerating\n\r", ch);
       echo_to_cockpit( AT_YELLOW , ship , "The ship begins to accelerate.");
       sprintf( buf, "%s begins to speed up." , ship->name );
       echo_to_system( AT_ORANGE , ship , buf , NULL );
    }

    if ( change < ship->currspeed )
    {
       send_to_char( "&GDecelerating\n\r", ch);
       echo_to_cockpit( AT_YELLOW , ship , "The ship begins to slow down.");
       sprintf( buf, "%s begins to slow down." , ship->name );
       echo_to_system( AT_ORANGE , ship , buf , NULL );
    }

    if ( change == ship->currspeed )
    {
       send_to_char( "&RYou are already going that speed.\n\r", ch);
	   return;
    }

    ship->energy -= abs((change-abs(ship->currspeed))/10);

    ship->currspeed = URANGE( 0 , change , ship->realspeed );

    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}

void do_trajectory( CHAR_DATA *ch, char *argument )
{
    char  buf[MAX_STRING_LENGTH];
    char  arg2[MAX_INPUT_LENGTH];
    char  arg3[MAX_INPUT_LENGTH];
    int chance;
    float vx,vy,vz;
    SHIP_DATA *ship;


	if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
		return;
	}

	if ( ship->class > SHIP_PLATFORM )
	{
		send_to_char("&RThis isn't a spacecraft!\n\r",ch);
		return;
	}

	if (  (ship = ship_from_pilotseat(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RYour not in the pilots seat.\n\r",ch);
		return;
	}

	if ( autofly(ship))
	{
		send_to_char("&RYou'll have to turn off the ships autopilot first.\n\r",ch);
		return;
	}

	if (ship->shipstate == SHIP_DISABLED)
	{
		send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
		return;
	}

	if  ( ship->class == SHIP_PLATFORM )
	{
		send_to_char( "&RPlatforms can't turn!\n\r" , ch );
		return;
	}

	if (ship->shipstate == SHIP_HYPERSPACE)
	{
		send_to_char("&RYou can only do that in realspace!\n\r",ch);
		return;
	}

	if (ship->shipstate == SHIP_DOCKED)
	{
		send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
		return;
	}

	if (ship->shipstate != SHIP_READY)
	{
		send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
		return;
	}

	if ( ship->energy < (ship->currspeed/10) )
	{
		send_to_char("&RTheres not enough fuel!\n\r",ch);
		return;
	}

	if ( ship->class == FIGHTER_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_starfighters]) ;
	if ( ship->class == MIDSIZE_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_midships]) ;
	if ( ship->class == CAPITAL_SHIP )
		chance = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_capitalships]);
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

    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    vx = atof( arg2 );
    vy = atof( arg3 );
    vz = atof( argument );

    if ( vx == ship->vx && vy == ship->vy && vz == ship->vz )
    {
       ch_printf( ch , "The ship is already at %.0f %.0f %.0f !" ,vx,vy,vz);
    }

    ship->hx = vx - ship->vx;
    ship->hy = vy - ship->vy;
    ship->hz = vz - ship->vz;

    ship->energy -= (ship->currspeed/10);

    ch_printf( ch ,"&GNew course set, approaching %.0f %.0f %.0f.\n\r" , vx,vy,vz );
    act( AT_PLAIN, "$n manipulates the ships controls.", ch, NULL, argument , TO_ROOM );

    echo_to_cockpit( AT_YELLOW ,ship, "The ship begins to turn.\n\r" );
    sprintf( buf, "%s turns altering its present course." , ship->name );
    echo_to_system( AT_ORANGE , ship , buf , NULL );

    if ( ship->class == FIGHTER_SHIP || ( ship->class == MIDSIZE_SHIP && ship->manuever > 50 ) )
        ship->shipstate = SHIP_BUSY_3;
    else if ( ship->class == MIDSIZE_SHIP || ( ship->class == CAPITAL_SHIP && ship->manuever > 50 ) )
        ship->shipstate = SHIP_BUSY_2;
    else
        ship->shipstate = SHIP_BUSY;

    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}


void do_buyship(CHAR_DATA *ch, char *argument )
{
    long long int     price;
    SHIP_DATA   *ship;

	if ( IS_NPC(ch) || !ch->pcdata )
   	{
   		send_to_char( "&ROnly players can do that!\n\r" ,ch );
   		return;
   	}

   	ship = ship_in_room( ch->in_room , argument );
   	if ( !ship )
   	{
    	ship = ship_from_cockpit( ch->in_room->vnum );

    	if ( !ship )
      	{
        	act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
        	return;
      	}
   	}

   	if ( str_cmp( ship->owner , "" )  || ship->type == MOB_SHIP )
   	{
   		send_to_char( "&RThat ship isn't for sale!" ,ch );
   		return;
   	}

   	price = get_ship_value( ship );

   	if ( ch->gold < price )
    {
    	ch_printf(ch, "&RThis ship costs %lld. You don't have enough wulongs!\n\r" , price );
       	return;
    }

    ch->gold -= price;
    ch_printf(ch, "&GYou pay %lld wulongs to purchace the ship.\n\r" , price );

    act( AT_PLAIN, "$n walks over to a terminal and makes a wulong transaction.", ch, NULL, argument , TO_ROOM );

    STRFREE( ship->owner );
	ship->owner = STRALLOC( ch->name );
	save_ship( ship );

}

void do_clansellship(CHAR_DATA *ch, char *argument )
{
	long long int     price;
    SHIP_DATA   *ship;
	CLAN_DATA   *clan;

	if ( IS_NPC(ch) || !ch->pcdata )
	{
		send_to_char( "&ROnly players can do that!\n\r" ,ch );
		return;
	}
	if ( !ch->pcdata->clan )
	{
		send_to_char( "&RYou aren't a member of an organization!\n\r" ,ch );
		return;
	}

	clan = ch->pcdata->clan;

	if ( ( ch->pcdata->bestowments
		&& is_name("clansellship", ch->pcdata->bestowments))
		|| !str_cmp( ch->name, clan->leader  ))
		;
	else
	{
		send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability.\n\r" ,ch );
		return;
	}

	ship = ship_in_room( ch->in_room , argument );
	if ( !ship )
	{
		ship = ship_from_cockpit( ch->in_room->vnum );
		if ( !ship )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
			return;
		}
	}

	if ( !check_pilot( ch , ship ) || !str_cmp( ship->owner , "Public" ) )
	{
		send_to_char("&RHey, thats not your ship!\n\r",ch);
		return;
	}

	price = get_ship_value( ship );
	clan->funds += ( price - price/10 );
	ch_printf(ch, "&G%s receive %lld wulongs from selling %s's ship.\n\r" , clan->name, price - price/10, clan->name );
    act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.",ch, NULL, argument , TO_ROOM );
	STRFREE( ship->owner );
	ship->owner = STRALLOC( "Unowned" );
	save_ship( ship );
	clan->spacecraft--;
}


void do_clanbuyship(CHAR_DATA *ch, char *argument )
{
    long long int     price;
    SHIP_DATA   *ship;
    CLAN_DATA   *clan;
    CLAN_DATA   *mainclan;

   	if ( IS_NPC(ch) || !ch->pcdata )
   	{
   		send_to_char( "&ROnly players can do that!\n\r" ,ch );
   		return;
   	}
   	if ( !ch->pcdata->clan )
   	{
   		send_to_char( "&RYou aren't a member of any organizations!\n\r" ,ch );
   		return;
   	}

   	clan = ch->pcdata->clan;
   	mainclan = ch->pcdata->clan->mainclan ? ch->pcdata->clan->mainclan : clan;

   	if ( ( ch->pcdata->bestowments && is_name("clanbuyship", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->leader  ))
	{
	}
   	else
   	{
   		send_to_char( "&RYour organization hasn't seen fit to bestow you with that ability.\n\r" ,ch );
   		return;
   	}

   	ship = ship_in_room( ch->in_room , argument );
   	if ( !ship )
   	{
      	ship = ship_from_cockpit( ch->in_room->vnum );

      	if ( !ship )
      	{
           act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
           return;
      	}
   	}

   	if ( str_cmp( ship->owner , "" )  || ship->type == MOB_SHIP )
   	{
   		send_to_char( "&RThat ship isn't for sale!\n\r" ,ch );
   		return;
   	}

   	price = get_ship_value( ship );

    if ( ch->pcdata->clan->funds < price )
    {
       ch_printf(ch, "&RThis ship costs %lld. You don't have enough wulongs!\n\r" , price );
       return;
    }

    clan->funds -= price;
    ch_printf(ch, "&G%s pays %lld wulongs to purchace the ship.\n\r", clan->name , price );

    act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.",ch,
       NULL, argument , TO_ROOM );

    	STRFREE( ship->owner );
	ship->owner = STRALLOC( clan->name );
	save_ship( ship );

   if ( ship->class <= SHIP_PLATFORM )
             clan->spacecraft++;
   else
             clan->vehicles++;
}

void do_sellship(CHAR_DATA *ch, char *argument )
{
    long long int     price;
    SHIP_DATA   *ship;

   	ship = ship_in_room( ch->in_room , argument );
   	if ( !ship )
   	{
    	act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
    	return;
   	}

   	if ( str_cmp( ship->owner , ch->name ) )
   	{
   		send_to_char( "&RThat isn't your ship!\n\r" ,ch );
   		return;
   	}

   	price = get_ship_value( ship );

    ch->gold += ( price - price/10 );
    ch_printf(ch, "&GYou receive %lld wulongs from selling your ship.\n\r" , price - price/10 );

    act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.",ch, NULL, argument , TO_ROOM );

	STRFREE( ship->owner );
	ship->owner = STRALLOC( "" );
	save_ship( ship );

}

void do_info(CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *ship;
    SHIP_DATA *target;
    OBJ_DATA *component;
    TRADEGOOD_DATA *tg;
    int i;


    if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    {
    	if ( argument[0] == '\0' )
        {
        	act( AT_PLAIN, "Which ship do you want info on?.", ch, NULL, NULL, TO_CHAR );
            return;
        }

        ship = ship_in_room( ch->in_room , argument );
        if ( !ship )
        {
        	act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
        	return;
        }

        target = ship;
    }
    else if (argument[0] == '\0')
    	target = ship;
    else
    	target = get_ship_here( argument , ship->starsystem );

	if( ship->mono_disabled ) /* MONO check - Funf */
	{
		send_to_char("&RYou look at a blank readout. Please enable MONO.\n\r", ch);
		return;
	}

    if ( target == NULL )
    {
    	send_to_char("&RI don't see that here.\n\rTry the radar, or type info by itself for info on this ship.\n\r",ch);
    	return;
    }

    if ( abs(target->vx - ship->vx) > 500+ship->sensor*2 ||
         abs(target->vy - ship->vy) > 500+ship->sensor*2 ||
         abs(target->vz - ship->vz) > 500+ship->sensor*2 )
    {
         send_to_char("&RThat ship is to far away to scan.\n\r",ch);
         return;
    }

    ch_printf( ch, "&Y%s %s : %s\n\r&B",
				target->type == SHIP_REPUBLIC ? "New Republic" :
		       (target->type == SHIP_IMPERIAL ? "Imperial" : "Civilian" ),
		        target->class == FIGHTER_SHIP ? "Starfighter" :
		       (target->class == MIDSIZE_SHIP ? "Midtarget" :
		       (target->class == CAPITAL_SHIP ? "Capital Ship" :
		       (ship->class == SHIP_PLATFORM ? 	"Platform" :
		       (ship->class == CLOUD_CAR ? 		"Cloudcar" :
		       (ship->class == OCEAN_SHIP ? 	"Boat" :
		       (ship->class == LAND_SPEEDER ? 	"Speeder" :
		       (ship->class == WHEELED ? 		"Wheeled Transport" :
		       (ship->class == LAND_CRAWLER ? 	"Crawler" :
		       (ship->class == WALKER ? 		"Walker" : "Unknown" ) ) ) ) ) ) ) ) ),
    			target->name,
    			target->filename);
    ch_printf( ch, "&CDescription&R: &W%s\n\r&COwner&R: &W%s   &CPilot&R: &W%s   &CCopilot&R: &W%s\n\r",
    			target->description,
    			target->owner, target->pilot,  target->copilot );
    ch_printf( ch, "&CMax Hull&R: &B(&W%d&B)  ",
                target->maxhull);
    ch_printf( ch, "&CMax Energy&c(&wfuel&c)&R: &B(&W%d&B)\n\r",
                target->maxenergy);
    ch_printf( ch, "&CMax Speed&R: &B(&W%d&B)   &CManuever&R: &B(&W%d&B)\n\r",
                target->realspeed,
                target->manuever );
    ch_printf( ch, "&CComm Range&R: &B(&W%d&B)   &CMax Chaff&R: &B(&W%d&B)&W\n\r",
				target->comm,
				target->maxchaff);
    ch_printf( ch, "&CMax Ammo&R: &B(&W%d&B)   &CTotal Weight&R: &B(&W%d&B)\n\r", target->maxammo, target->mass );
        ch_printf( ch, "&CCargo Capacity&R: &B(&W%d&B)&w\n\r", target->cargo);

	i = 0;
    for ( component = target->first_component ; component ; component = component->next_content )
    {
		++i;
		ch_printf( ch, "&gComponent %d&b:   &W%s\n\r",
			i,
			component->short_descr);
		if ( component->value[0] == COMPONENT_WEAPON )
		{
			ch_printf( ch, "&gType&b: &CWeapon   &gWeight&b: &W%d   &gStatus&b: &W%s\n\r",
				component->weight,
				component->value[3] == DISABLE_NONE ? "Working" : "Disabled" );
		}
		else
		{
			ch_printf( ch, "&gType&b: &C%s   &gWeight&b: &W%d   &gValue&b: &W%d\n\r",
				component->value[0] == COMPONENT_ENGINE ? "Engine" :
				component->value[0] == COMPONENT_HULL ? "Hull Plate" :
				component->value[0] == COMPONENT_FUEL ? "Fuel Tank" :
				component->value[0] == COMPONENT_AMMO ? "Ammo Store" :
				component->value[0] == COMPONENT_THRUSTER ? "Thruster" :
				component->value[0] == COMPONENT_COMM ? "Comm Radio" :
				component->value[0] == COMPONENT_MONO ? "MONO Computer" :
				component->value[0] == COMPONENT_SCANNER ? "Body Scanner" :
				component->value[0] == COMPONENT_CARGO ? "Cargo Hold" :	"Unknown",
				component->weight,
				component->value[1] );
		}
	}

	if ( (ship_from_cockpit(ch->in_room->vnum)  != NULL
		 || !str_cmp(ch->name, ship->pilot)
		 || !str_cmp(ch->name, ship->copilot)
		 || !str_cmp(ch->name, ship->owner)
		) && ship->cargo > 0) // If in cockpit or a pilot, show cargo --Funf 2011-02-08
	{
		ch_printf( ch, "&YCargo Contents:\n\r");
		tg = ship->first_tradegood;

		for(i = 0; i < ship->cargo; ++i)
		{
			if (tg)
			{
				ch_printf( ch, " &Y- &gOrigin&b: &C%s  &gType&b: &C%s  &gPaid&b: &C%lld\n\r",
					tg->origin,
					tg->type == TRADEGOOD_STANDARD ? "Standard" :
					tg->type == TRADEGOOD_CONTRABAND ? "Contraband" : "Unknown",
					tg->value );
				tg = tg->next;
			}
			else
			{
				ch_printf( ch, " &Y- &gEmpty\n\r");
			}
		}
	}

    /*ch_printf( ch, "&gFirstWeapon&b:   &w%s\n\r", target->firstweapon == 0 ? "Dual Machine Guns"  :
                       (target->firstweapon == 1 ? "105mm Rifle" :
                       (target->firstweapon == 2 ? "30mm Cannon" :
                       (target->firstweapon == 3 ? "Grappling Claw" :
                       (target->firstweapon == 4 ? "Machine Gun" :
                       (target->firstweapon == 5 ? "Cluster Missile Launcher" :
                       (target->firstweapon == 6 ? "Multiple Munitions Launcher" :
                       (target->firstweapon == 7 ? "Plasma Cannon" :
                       (target->firstweapon == 8 ? "Gatling Gun"  :
                       (target->firstweapon == 9 ? "Super Plasma Cannon" :"None" ) ) ) ) ) ) ) ) ) );
    ch_printf( ch, "&gSecondWeapon&b:  &w%s\n\r", target->secondweapon == 0 ? "Dual Machine Guns"  :
                       (target->secondweapon == 1 ? "105mm Rifle" :
                       (target->secondweapon == 2 ? "30mm Cannon" :
                       (target->secondweapon == 3 ? "Grappling Claw" :
                       (target->secondweapon == 4 ? "Machine Gun" :
                       (target->secondweapon == 5 ? "Cluster Missile Launcher" :
                       (target->secondweapon == 6 ? "Multiple Munitions Launcher" :
                       (target->secondweapon == 7 ? "Plasma Cannon" :
                       (target->secondweapon == 8 ? "Gatling Gun"  :
                       (target->secondweapon == 9 ? "Super Plasma Cannon" :"None" ) ) ) ) ) ) ) ) ) );
    ch_printf( ch, "&gThirdWeapon&b:   &w%s\n\r", target->thirdweapon == 0 ? "Dual Machine Guns"  :
                       (target->thirdweapon == 1 ? "105mm Rifle" :
                       (target->thirdweapon == 2 ? "30mm Cannon" :
                       (target->thirdweapon == 3 ? "Grappling Claw" :
                       (target->thirdweapon == 4 ? "Machine Gun" :
                       (target->thirdweapon == 5 ? "Cluster Missile Launcher" :
                       (target->thirdweapon == 6 ? "Multiple Munitions Launcher" :
                       (target->thirdweapon == 7 ? "Plasma Cannon" :
                       (target->thirdweapon == 8 ? "Gatling Gun"  :
                       (target->thirdweapon == 9 ? "Super Plasma Cannon" :"None" ) ) ) ) ) ) ) ) ) );*/

    act( AT_PLAIN, "$n checks various gages and displays on the control panel.", ch,
         NULL, argument , TO_ROOM );

}

void do_autopilot(CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;

    if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    {
    	send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
        return;
    }

    if (  (ship = ship_from_pilotseat(ch->in_room->vnum))  == NULL )
    {
        send_to_char("&RYou must be in the pilot's seat!\n\r",ch);
        return;
    }

    if ( ship->mono_disabled ) /* MONO support - Funf */
    {
		ship->autopilot = FALSE;
		ship->autorecharge = FALSE;
		send_to_char( "&RThe MONO computer is down!\n\r", ch);
		return;
	}

    act( AT_PLAIN, "$n flips a switch on the control panel.", ch, NULL, argument , TO_ROOM );

	if (ship->autopilot == TRUE)
	{
		ship->autopilot=FALSE;
		send_to_char( "&GYou toggle the autopilot.\n\r", ch);
		echo_to_cockpit( AT_YELLOW , ship , "Autopilot &C[&ROFF&C]&W");
	}
	else
	{
		ship->autopilot=TRUE;
		ship->autorecharge = TRUE;
		send_to_char( "&GYou toggle the autopilot.\n\r", ch);
		echo_to_cockpit( AT_YELLOW , ship , "Autopilot &C[&GON&C]&W");
	}

}

void do_openhatch(CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;
	char buf[MAX_STRING_LENGTH];

	if ( !argument || argument[0] == '\0' || !str_cmp(argument,"hatch") )
	{
		ship = ship_from_entrance( ch->in_room->vnum );
		if( ship == NULL)
		{
			send_to_char( "&ROpen what?\n\r", ch );
          	return;
       	}
       	else
       	{
			if ( !ship->hatchopen)
			{
				if  ( ship->class == SHIP_PLATFORM )
                {
					send_to_char( "&RTry one of the docking bays!\n\r" , ch );
                   	return;
                }
       	     	if ( ship->location != ship->lastdoc ||
       	           ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED ) )
       	     	{
       	       		send_to_char("&RPlease wait until the ship lands!\n\r",ch);
       	       		return;
       	     	}
       	     	ship->hatchopen = TRUE;
       	     	send_to_char("&GYou open the hatch.\n\r",ch);
       	     	act( AT_PLAIN, "$n opens the hatch.", ch, NULL, argument, TO_ROOM );
       	     	sprintf( buf , "The hatch on %s opens." , ship->name);
       	     	echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
				return;
       	  	}
       	  	else
       	  	{
       	    	send_to_char("&RIt's already open.\n\r",ch);
       	    	return;
       	  	}
       	}
   	}

   	ship = ship_in_room( ch->in_room , argument );
   	if ( !ship )
   	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
   	}

   	if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   	{
		send_to_char( "&RThat ship has already started to launch.\n\r",ch);
        return;
   	}

   	if ( ! check_pilot(ch,ship) )
	{
		send_to_char("&RHey! Thats not your ship!\n\r",ch);
		return;
	}

   	if ( !ship->hatchopen)
   	{
		ship->hatchopen = TRUE;
   		act( AT_PLAIN, "You open the hatch on $T.", ch, NULL, ship->name, TO_CHAR );
   		act( AT_PLAIN, "$n opens the hatch on $T.", ch, NULL, ship->name, TO_ROOM );
   		echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch opens from the outside." );
		return;
   	}

   	send_to_char("&GIts already open!\n\r",ch);

}


void do_closehatch(CHAR_DATA *ch, char *argument )
{
   	SHIP_DATA *ship;
   	char buf[MAX_STRING_LENGTH];

   	if ( !argument || argument[0] == '\0' || !str_cmp(argument,"hatch") )
   	{
		ship = ship_from_entrance( ch->in_room->vnum );
		if( ship == NULL)
       	{
			send_to_char( "&RClose what?\n\r", ch );
			return;
       	}
       	else
       	{
			if  ( ship->class == SHIP_PLATFORM )
			{
				send_to_char( "&RTry one of the docking bays!\n\r" , ch );
				return;
			}
          	if ( ship->hatchopen)
       	  	{
       	     	ship->hatchopen = FALSE;
       	     	send_to_char("&GYou close the hatch.\n\r",ch);
       	     	act( AT_PLAIN, "$n closes the hatch.", ch, NULL, argument, TO_ROOM );
       	     	sprintf( buf , "The hatch on %s closes." , ship->name);
       	     	echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
 	     		return;
       	  	}
       	  	else
       	  	{
       	     	send_to_char("&RIt's already closed.\n\r",ch);
       	     	return;
       	  	}
       	}
   	}

   	ship = ship_in_room( ch->in_room , argument );
   	if ( !ship )
   	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
		return;
   	}

   	if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
   	{
		send_to_char( "&RThat ship has already started to launch.\n\r",ch);
		return;
   	}
   	else
   	{
      	if(ship->hatchopen)
      	{
   		ship->hatchopen = FALSE;
   		act( AT_PLAIN, "You close the hatch on $T.", ch, NULL, ship->name, TO_CHAR );
   		act( AT_PLAIN, "$n closes the hatch on $T.", ch, NULL, ship->name, TO_ROOM );
        echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch is closed from outside.");
   		return;
    	}
    	else
      	{
      		send_to_char("&RIt's already closed.\n\r",ch);
      		return;
      	}
   	}
}

void do_status(CHAR_DATA *ch, char *argument )
{
    int chance;
    SHIP_DATA *ship;
    SHIP_DATA *target;

	if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    {
		send_to_char("&RYou must be in the cockpit, turret or engineroom of a ship to do that!\n\r",ch);
		return;
    }

    if (argument[0] == '\0')
    	target = ship;
    else
       	target = get_ship_here( argument , ship->starsystem );

    if ( target == NULL )
    {
       	send_to_char("&RI don't see that here.\n\rTry the radar, or type status by itself for your ships status.\n\r",ch);
       	return;
    }

    if ( abs(target->vx - ship->vx) > 500+ship->sensor*2 ||
         abs(target->vy - ship->vy) > 500+ship->sensor*2 ||
         abs(target->vz - ship->vz) > 500+ship->sensor*2 )
    {
       	send_to_char("&RThat ship is to far away to scan.\n\r",ch);
        return;
    }

    chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_shipsystems]) ;
    if ( number_percent( ) > chance )
    {
		send_to_char("&RYou cant figure out what the readout means.\n\r",ch);
        learn_from_failure( ch, gsn_shipsystems );
        return;
    }

    act( AT_PLAIN, "$n checks various gages and displays on the control panel.", ch,
         NULL, argument , TO_ROOM );

	calc_ship_speed( target ); /* Dynamic speed system, Do I need one here? - Funf */

    ch_printf( ch, "&C%s&R:&W\n\r",target->name);
    ch_printf( ch, "&CCurrent Coordinates&R:&W %.0f %.0f %.0f&W\n\r",
                    target->vx, target->vy, target->vz );
    ch_printf( ch, "&CCurrent Heading&R:&W %.0f %.0f %.0f&W\n\r",
                    target->hx, target->hy, target->hz );
    ch_printf( ch, "&CCurrent Speed&R:&W %d&R/%d&W\n\r",
                    get_ship_speed(target) , target->realspeed );
    ch_printf( ch, "&CHull&R:&W %d&R/&W%d  &CShip Condition&R:&W %s&W\n\r",
                    target->hull,
    		        target->maxhull,
    				target->shipstate == SHIP_DISABLED ? "Disabled" : target->mono_disabled ? "MONO Off" : "Running");
    ch_printf( ch, "&CEnergy&c(&wfuel&c)&R:&W %d&R/&W%d   &CAmmo&c&R:&W %d&R/&W%d\n\r",
    		        target->energy,
    		        target->maxenergy,
    		        target->ammo,
    		        target->maxammo);
    ch_printf( ch, "&CCurrent Target&R:&W %s\n\r",
    		        target->target0 ? target->target0->name : "&cnone&W");

    learn_from_success( ch, gsn_shipsystems );
}

/* New Hyperspace system designed to work with gates ala Cowboy Bebop -Kristen- */
void do_hyperspace( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SHIP_DATA *ship;
    int vx, vy ,vz;
    long long int fee;

    strcpy( arg, argument );

    if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    {
    	send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	return;
    }

    if ( ship->class >= SHIP_PLATFORM )
    {
    	send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	return;
    }

    if ( (ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL )
    {
    	send_to_char("&RYou need to be in the pilot seat!\n\r",ch);
    	return;
    }

    if ( autofly(ship) )
    {
    	send_to_char("&RYou'll have to turn off the ships autopilot first.\n\r",ch);
    	return;
    }

	if ( ship->mono_disabled ) /* MONO system support - Funf */
	{
		send_to_char("&RMONO system down! You can't navigate hyperspace!\n\r",ch);
		return;
	}

    if (ship->shipstate == SHIP_HYPERSPACE)
    {
    	send_to_char("&RYou are already travelling in hyperspace!\n\r",ch);
    	return;
    }

    if (ship->shipstate == SHIP_DISABLED)
    {
    	send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
    	return;
    }

    if (ship->shipstate == SHIP_DOCKED)
    {
    	send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	return;
    }

    if (ship->shipstate != SHIP_READY)
    {
    	send_to_char("&RPlease wait until the ship has finished its current manuever.\n\r",ch);
    	return;
    }

    if (!ship->currjump)
    {
    	send_to_char("&RYou need to calculate your jump first!\n\r",ch);
    	return;
    }

    if ( ship->currspeed <= 0 )
    {
    	send_to_char("&RYou need to speed up a little first!\n\r",ch);
    	return;
    }

    if ( ship->starsystem == NULL )
    {
    	send_to_char("&RThere's nothing here!\n\r",ch);
    	return;
    }

	if ( ship->currjump == ship->starsystem )
	{
		send_to_char("&RYou are already there!\n\r", ch );
		return;
	}

    vx = ship->starsystem->g1x;
    vy = ship->starsystem->g1y;
    vz = ship->starsystem->g1z;
	fee = ( abs( ship->hyperdistance * 10 * (1+ship->class) ) );
    if (  (vx > ship->vx + 200) || (vx < ship->vx - 200) ||
    	  (vy > ship->vy + 200) || (vy < ship->vy - 200) ||
    	  (vz > ship->vz + 200) || (vz < ship->vz - 200) )
    {
    	send_to_char("&RThe hyperspace gate is too far away! You'll have to fly a little closer.\n\r",ch);
    	return;
    }
    if ( ch->pcdata )
    {
		if ( ch->pcdata->bank >= fee )
		{
            sprintf( buf ,"You are charged %lld wulongs for the use of the gate." , fee );
            echo_to_room( AT_GREEN , get_room_index(ship->pilotseat),  buf );
			ch->pcdata->bank -= fee;
		}
		else
		{
            sprintf( buf ,"You don't have enough money in your account to pay the toll of %d." , fee );
            echo_to_room( AT_RED , get_room_index(ship->pilotseat),  buf );
			return;
		}
	}
    sprintf( buf ,"%s disappears into the hyperspace gate." , ship->name );
    echo_to_system( AT_YELLOW, ship, buf , NULL );

    ship_from_starsystem( ship , ship->starsystem );
    ship->shipstate = SHIP_HYPERSPACE;

    send_to_char( "&GYou guide the ship into the hyperspace gate.\n\r", ch);
    act( AT_PLAIN, "$n steers the ship into the hyperspace gate\n\r.", ch, NULL, argument , TO_ROOM );
    echo_to_ship( AT_YELLOW , ship , "The ship lurches slightly as it enters a hyperspace gate." );
    echo_to_cockpit( AT_YELLOW , ship , "The stars disappear and you enter hyperspace.");

    ship->vx = ship->jx;
    ship->vy = ship->jy;
    ship->vz = ship->jz;

}

void do_target(CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
    int chance;
    SHIP_DATA *ship;
    SHIP_DATA *target;
    char buf[MAX_STRING_LENGTH];

    strcpy( arg, argument );

    switch( ch->substate )
    {
		default:
			if (  (ship = ship_from_turret(ch->in_room->vnum))  == NULL )
			{
				send_to_char("&RYou must be in the gunners seat or turret of a ship to do that!\n\r",ch);
    	        return;
    	    }

            if ( ship->class > SHIP_PLATFORM )
    	    {
    	        send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	        return;
    	    }

            if (ship->shipstate == SHIP_HYPERSPACE)
            {
                send_to_char("&RYou can only do that in realspace!\n\r",ch);
            	return;
            }

    	    if ( ! ship->starsystem )
    	    {
    	        send_to_char("&RYou can't do that until you've finished launching!\n\r",ch);
    	        return;
    	    }

            if ( autofly(ship) )
    	    {
    	        send_to_char("&RYou'll have to turn off the ships autopilot first....\n\r",ch);
    	        return;
    	    }

            if (arg[0] == '\0')
    	    {
    	        send_to_char("&RYou need to specify a target!\n\r",ch);
    	        return;
    	    }

            if ( !str_cmp( arg, "none") )
    	    {
    	    	send_to_char("&GTarget set to none.\n\r",ch);
    	        if ( ch->in_room->vnum == ship->gunseat )
    	        	ship->target0 = NULL;
    	        if ( ch->in_room->vnum == ship->turret1 )
    	            ship->target1 = NULL;
    	        if ( ch->in_room->vnum == ship->turret2 )
    	            ship->target2 = NULL;
    	        return;
			}

            target = get_ship_here( arg, ship->starsystem );
            if (  target == NULL )
            {
                send_to_char("&RThat ship isn't here!\n\r",ch);
                return;
            }

            if (  target == ship )
            {
                send_to_char("&RYou can't target your own ship!\n\r",ch);
                return;
            }

            if ( abs(ship->vx-target->vx) > 5000 ||
                 abs(ship->vy-target->vy) > 5000 ||
                 abs(ship->vz-target->vz) > 5000 )
            {
                send_to_char("&RThat ship is too far away to target.\n\r",ch);
                return;
            }

            chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_weaponsystems]) ;
			if ( number_percent( ) < chance )
    		{
    			send_to_char( "&GTracking target.\n\r", ch);
    		   	act( AT_PLAIN, "$n makes some adjustments on the targeting computer.", ch, NULL, argument , TO_ROOM );
    		   	add_timer ( ch , TIMER_DO_FUN , ( ship->mono_disabled ? 2 : 1 ) , do_target , 1 ); /* MONO support - Funf */
    		   	ch->dest_buf = str_dup(arg);
    		   	return;
	        }
	        send_to_char("&RYou fail to work the controls properly.\n\r",ch);
	        learn_from_failure( ch, gsn_weaponsystems );
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
    		if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    		      return;
    	        send_to_char("&RYour concentration is broken. You fail to lock onto your target.\n\r", ch);
    		return;
	}

    ch->substate = SUB_NONE;

    if ( (ship = ship_from_turret(ch->in_room->vnum)) == NULL )
    {
       return;
    }

    target = get_ship_here( arg, ship->starsystem );
    if (  target == NULL || target == ship)
    {
        send_to_char("&RThe ship has left the area. Targeting aborted.\n\r",ch);
    	return;
    }

    if ( ch->in_room->vnum == ship->gunseat )
    	ship->target0 = target;

    if ( ch->in_room->vnum == ship->turret1 )
    	ship->target1 = target;

    if ( ch->in_room->vnum == ship->turret2 )
    	ship->target2 = target;
    send_to_char( "&GTarget Locked.\n\r", ch);
    //if ( has_component( ship, COMPONENT_SCANNER ) )   All ships have scanners... for now. 2006-10-16 Funf
    { /* Target shows occupants - Funf */
		int room;
		CHAR_DATA * person; //Locke
		sprintf( buf, "&GShip logs indicate the following people are aboard: ");
   		for ( room = target->firstroom ; room <= target->lastroom ;room++ )
		{
			//fix start - Locke
			for ( person = (get_room_index(room)->first_person); person ; person = person->next_in_room )
			{
				if (!IS_IMMORTAL(person) && !IS_NPC(person)) /* added imm and npc check - Funf */
				{
					strcat(buf, person->name);
					strcat(buf, " ");
				}
			}
		}	//fix end - Locke
		strcat(buf, "&w\r\n");
		send_to_char(buf, ch);
	}
    /*sprintf( buf , "You are being targeted by %s." , ship->name);
    echo_to_cockpit( AT_BLOOD , target , buf );*/

    learn_from_success( ch, gsn_weaponsystems );

    if ( autofly(target) && !target->target0)
    {
		/*sprintf( buf , "You are being targeted by %s." , target->name);
	    echo_to_cockpit( AT_BLOOD , ship , buf );*/
	    target->target0 = ship;
    }
}

/*void do_fire(CHAR_DATA *ch, char *argument ) Completely rewritten, see msweapons.c - Funf
{
    int chance;
    SHIP_DATA *ship;
    SHIP_DATA *target;
    char buf[MAX_STRING_LENGTH];

	if (  (ship = ship_from_turret(ch->in_room->vnum))  == NULL )
    {
    	send_to_char("&RYou must be in the gunners chair or turret of a ship to do that!\n\r",ch);
        return;
    }

    if ( ship->class > SHIP_PLATFORM )
    {
        send_to_char("&RThis isn't a spacecraft!\n\r",ch);
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
    	if ( ship->energy <5 )
        {
             send_to_char("&RTheres not enough energy left to fire!\n\r",ch);
             return;
        }

                if ( autofly(ship) )
    	        {
    	            send_to_char("&RYou'll have to turn off the ships autopilot first.\n\r",ch);
    	            return;
    	        }


        chance = IS_NPC(ch) ? ch->top_level
                 : (int) ( ch->perm_dex*2 + ch->pcdata->learned[gsn_spacecombat]/3
                           + ch->pcdata->learned[gsn_spacecombat2]/3 + ch->pcdata->learned[gsn_spacecombat3]/3 );

    	if ( ch->in_room->vnum == ship->gunseat && !str_prefix( argument , "lasers") )
    	{

    	     if (ship->statet0 == LASER_DAMAGED)
    	     {
    	        send_to_char("&RThe ships main laser is damaged.\n\r",ch);
    	      	return;
    	     }
             if (ship->statet0 >= ship->lasers )
    	     {
    	     	send_to_char("&RThe lasers are still recharging.\n\r",ch);
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
             if ( abs(target->vx - ship->vx) >1000 ||
                  abs(target->vy - ship->vy) >1000 ||
                  abs(target->vz - ship->vz) >1000 )
             {
                send_to_char("&RThat ship is out of laser range.\n\r",ch);
    	     	return;
             }
             if ( ship->class < 2 && !is_facing( ship, target ) )
             {
                send_to_char("&RThe main laser can only fire forward. You'll need to turn your ship!\n\r",ch);
    	     	return;
             }
             ship->statet0++;
             chance += target->class*25;
             chance -= target->manuever/10;
             chance -= target->currspeed/20;
             chance -= ( abs(target->vx - ship->vx)/70 );
             chance -= ( abs(target->vy - ship->vy)/70 );
             chance -= ( abs(target->vz - ship->vz)/70 );
             chance = URANGE( 10 , chance , 90 );
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             if ( number_percent( ) > chance )
             {
                sprintf( buf , "Lasers fire from %s at you but miss." , ship->name);
                echo_to_cockpit( AT_ORANGE , target , buf );
                sprintf( buf , "The ships lasers fire at %s but miss." , target->name);
                echo_to_cockpit( AT_ORANGE , ship , buf );
                learn_from_failure( ch, gsn_spacecombat );
    	        learn_from_failure( ch, gsn_spacecombat2 );
    	        learn_from_failure( ch, gsn_spacecombat3 );
    	        sprintf( buf, "Laserfire from %s barely misses %s." , ship->name , target->name );
                echo_to_system( AT_ORANGE , ship , buf , target );
    	        return;
             }
             sprintf( buf, "Laserfire from %s hits %s." , ship->name, target->name );
             echo_to_system( AT_ORANGE , ship , buf , target );
             sprintf( buf , "You are hit by lasers from %s!" , ship->name);
             echo_to_cockpit( AT_BLOOD , target , buf );
             sprintf( buf , "Your ships lasers hit %s!." , target->name);
             echo_to_cockpit( AT_YELLOW , ship , buf );
             learn_from_success( ch, gsn_spacecombat );
             learn_from_success( ch, gsn_spacecombat2 );
             learn_from_success( ch, gsn_spacecombat3 );
             echo_to_ship( AT_RED , target , "A small explosion vibrates through the ship." );
             damage_ship_ch( target , 5 , 10 , ch );

             if ( autofly(target) && target->target0 != ship )
             {
                target->target0 = ship;
                sprintf( buf , "You are being targetted by %s." , target->name);
                echo_to_cockpit( AT_BLOOD , ship , buf );
             }

             return;
    	}

        if ( ch->in_room->vnum == ship->gunseat && !str_prefix( argument , "missile") )
    	{
    	     if (ship->missilestate == MISSILE_DAMAGED)
    	     {
    	        send_to_char("&RThe ships missile launchers are dammaged.\n\r",ch);
    	      	return;
    	     }
             if (ship->missiles <= 0)
    	     {
    	     	send_to_char("&RYou have no missiles to fire!\n\r",ch);
    	        return;
    	     }
    	     if (ship->missilestate != MISSILE_READY )
    	     {
    	     	send_to_char("&RThe missiles are still reloading.\n\r",ch);
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
             if ( abs(target->vx - ship->vx) >1000 ||
                  abs(target->vy - ship->vy) >1000 ||
                  abs(target->vz - ship->vz) >1000 )
             {
                send_to_char("&RThat ship is out of missile range.\n\r",ch);
    	     	return;
             }
             if ( ship->class < 2 && !is_facing( ship, target ) )
             {
                send_to_char("&RMissiles can only fire in a forward. You'll need to turn your ship!\n\r",ch);
    	     	return;
             }
             chance -= target->manuever/5;
             chance -= target->currspeed/20;
             chance += target->class*target->class*25;
             chance -= ( abs(target->vx - ship->vx)/100 );
             chance -= ( abs(target->vy - ship->vy)/100 );
             chance -= ( abs(target->vz - ship->vz)/100 );
             chance += ( 30 );
             chance = URANGE( 20 , chance , 80 );
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             if ( number_percent( ) > chance )
             {
                send_to_char( "&RYou fail to lock onto your target!", ch );
                ship->missilestate = MISSILE_RELOAD_2;
    	        return;
             }
             new_missile( ship , target , ch , CONCUSSION_MISSILE );
             ship->missiles-- ;
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             echo_to_cockpit( AT_YELLOW , ship , "Missiles launched.");
             sprintf( buf , "Incoming missile from %s." , ship->name);
             echo_to_cockpit( AT_BLOOD , target , buf );
             sprintf( buf, "%s fires a missile towards %s." , ship->name, target->name );
             echo_to_system( AT_ORANGE , ship , buf , target );
             learn_from_success( ch, gsn_weaponsystems );
             if ( ship->class == CAPITAL_SHIP || ship->class == SHIP_PLATFORM )
                   ship->missilestate = MISSILE_RELOAD;
             else
                   ship->missilestate = MISSILE_FIRED;

             if ( autofly(target) && target->target0 != ship )
             {
                target->target0 = ship;
                sprintf( buf , "You are being targetted by %s." , target->name);
                echo_to_cockpit( AT_BLOOD , ship , buf );
             }

             return;
    	}
        if ( ch->in_room->vnum == ship->gunseat && !str_prefix( argument , "torpedo") )
    	{
    	     if (ship->missilestate == MISSILE_DAMAGED)
    	     {
    	        send_to_char("&RThe ships missile launchers are dammaged.\n\r",ch);
    	      	return;
    	     }
             if (ship->torpedos <= 0)
    	     {
    	     	send_to_char("&RYou have no torpedos to fire!\n\r",ch);
    	        return;
    	     }
    	     if (ship->missilestate != MISSILE_READY )
    	     {
    	     	send_to_char("&RThe torpedos are still reloading.\n\r",ch);
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
             if ( abs(target->vx - ship->vx) >1000 ||
                  abs(target->vy - ship->vy) >1000 ||
                  abs(target->vz - ship->vz) >1000 )
             {
                send_to_char("&RThat ship is out of torpedo range.\n\r",ch);
    	     	return;
             }
             if ( ship->class < 2 && !is_facing( ship, target ) )
             {
                send_to_char("&RTorpedos can only fire in a forward direction. You'll need to turn your ship!\n\r",ch);
    	     	return;
             }
             chance -= target->manuever/5;
             chance -= target->currspeed/20;
             chance += target->class*target->class*25;
             chance -= ( abs(target->vx - ship->vx)/100 );
             chance -= ( abs(target->vy - ship->vy)/100 );
             chance -= ( abs(target->vz - ship->vz)/100 );
             chance = URANGE( 20 , chance , 80 );
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             if ( number_percent( ) > chance )
             {
                send_to_char( "&RYou fail to lock onto your target!", ch );
                ship->missilestate = MISSILE_RELOAD_2;
    	        return;
             }
             new_missile( ship , target , ch , PROTON_TORPEDO );
             ship->torpedos-- ;
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             echo_to_cockpit( AT_YELLOW , ship , "Missiles launched.");
             sprintf( buf , "Incoming torpedo from %s." , ship->name);
             echo_to_cockpit( AT_BLOOD , target , buf );
             sprintf( buf, "%s fires a torpedo towards %s." , ship->name, target->name );
             echo_to_system( AT_ORANGE , ship , buf , target );
             learn_from_success( ch, gsn_weaponsystems );
             if ( ship->class == CAPITAL_SHIP || ship->class == SHIP_PLATFORM )
                   ship->missilestate = MISSILE_RELOAD;
             else
                   ship->missilestate = MISSILE_FIRED;

             if ( autofly(target) && target->target0 != ship )
             {
                target->target0 = ship;
                sprintf( buf , "You are being targetted by %s." , target->name);
                echo_to_cockpit( AT_BLOOD , ship , buf );
             }

             return;
    	}

        if ( ch->in_room->vnum == ship->gunseat && !str_prefix( argument , "rocket") )
    	{
    	     if (ship->missilestate == MISSILE_DAMAGED)
    	     {
    	        send_to_char("&RThe ships missile launchers are damaged.\n\r",ch);
    	      	return;
    	     }
             if (ship->rockets <= 0)
    	     {
    	     	send_to_char("&RYou have no rockets to fire!\n\r",ch);
    	        return;
    	     }
    	     if (ship->missilestate != MISSILE_READY )
    	     {
    	     	send_to_char("&RThe missiles are still reloading.\n\r",ch);
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
             if ( abs(target->vx - ship->vx) >800 ||
                  abs(target->vy - ship->vy) >800 ||
                  abs(target->vz - ship->vz) >800 )
             {
                send_to_char("&RThat ship is out of rocket range.\n\r",ch);
    	     	return;
             }
             if ( ship->class < 2 && !is_facing( ship, target ) )
             {
                send_to_char("&RRockets can only fire forward. You'll need to turn your ship!\n\r",ch);
    	     	return;
             }
             chance -= target->manuever/5;
             chance -= target->currspeed/20;
             chance += target->class*target->class*25;
             chance -= ( abs(target->vx - ship->vx)/100 );
             chance -= ( abs(target->vy - ship->vy)/100 );
             chance -= ( abs(target->vz - ship->vz)/100 );
             chance -= 30 ;
             chance = URANGE( 20 , chance , 80 );
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             if ( number_percent( ) > chance )
             {
                send_to_char( "&RYou fail to lock onto your target!", ch );
                ship->missilestate = MISSILE_RELOAD_2;
    	        return;
             }
             new_missile( ship , target , ch , HEAVY_ROCKET );
             ship->rockets-- ;
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             echo_to_cockpit( AT_YELLOW , ship , "Rocket launched.");
             sprintf( buf , "Incoming rocket from %s." , ship->name);
             echo_to_cockpit( AT_BLOOD , target , buf );
             sprintf( buf, "%s fires a heavy rocket towards %s." , ship->name, target->name );
             echo_to_system( AT_ORANGE , ship , buf , target );
             learn_from_success( ch, gsn_weaponsystems );
             if ( ship->class == CAPITAL_SHIP || ship->class == SHIP_PLATFORM )
                   ship->missilestate = MISSILE_RELOAD;
             else
                   ship->missilestate = MISSILE_FIRED;

             if ( autofly(target) && target->target0 != ship )
             {
                target->target0 = ship;
                sprintf( buf , "You are being targetted by %s." , target->name);
                echo_to_cockpit( AT_BLOOD , ship , buf );
             }

             return;
    	}

        if ( ch->in_room->vnum == ship->turret1 && !str_prefix( argument , "lasers") )
    	{
    	     if (ship->statet1 == LASER_DAMAGED)
    	     {
    	        send_to_char("&RThe ships turret is damaged.\n\r",ch);
    	      	return;
    	     }
             if (ship->statet1 > ship->class )
    	     {
    	     	send_to_char("&RThe turbolaser is recharging.\n\r",ch);
    	     	return;
    	     }
    	     if (ship->target1 == NULL )
    	     {
    	     	send_to_char("&RYou need to choose a target first.\n\r",ch);
    	     	return;
    	     }
    	     target = ship->target1;
             if (ship->target1->starsystem != ship->starsystem)
    	     {
    	     	send_to_char("&RYour target seems to have left.\n\r",ch);
    	        ship->target1 = NULL;
    	     	return;
    	     }
             if ( abs(target->vx - ship->vx) >1000 ||
                  abs(target->vy - ship->vy) >1000 ||
                  abs(target->vz - ship->vz) >1000 )
             {
                send_to_char("&RThat ship is out of laser range.\n\r",ch);
    	     	return;
             }
             ship->statet1++;
             chance -= target->manuever/10;
             chance += target->class*25;
             chance -= target->currspeed/20;
             chance -= ( abs(target->vx - ship->vx)/70 );
             chance -= ( abs(target->vy - ship->vy)/70 );
             chance -= ( abs(target->vz - ship->vz)/70 );
             chance = URANGE( 10 , chance , 90 );
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             if ( number_percent( ) > chance )
             {
                sprintf( buf , "Turbolasers fire from %s at you but miss." , ship->name);
                echo_to_cockpit( AT_ORANGE , target , buf );
                sprintf( buf , "Turbolasers fire from the ships turret at %s but miss." , target->name);
                echo_to_cockpit( AT_ORANGE , ship , buf );
                sprintf( buf, "%s fires at %s but misses." , ship->name, target->name );
                echo_to_system( AT_ORANGE , ship , buf , target );
                learn_from_failure( ch, gsn_spacecombat );
    	        learn_from_failure( ch, gsn_spacecombat2 );
    	        learn_from_failure( ch, gsn_spacecombat3 );
    	        return;
             }
             sprintf( buf, "Turboasers fire from %s, hitting %s." , ship->name, target->name );
             echo_to_system( AT_ORANGE , ship , buf , target );
             sprintf( buf , "You are hit by turbolasers from %s!" , ship->name);
             echo_to_cockpit( AT_BLOOD , target , buf );
             sprintf( buf , "Turbolasers fire from the turret, hitting %s!." , target->name);
             echo_to_cockpit( AT_YELLOW , ship , buf );
             learn_from_success( ch, gsn_spacecombat );
             learn_from_success( ch, gsn_spacecombat2 );
             learn_from_success( ch, gsn_spacecombat3 );
             echo_to_ship( AT_RED , target , "A small explosion vibrates through the ship." );
             damage_ship_ch( target , 10 , 25 , ch );

             if ( autofly(target) && target->target0 != ship )
             {
                target->target0 = ship;
                sprintf( buf , "You are being targetted by %s." , target->name);
                echo_to_cockpit( AT_BLOOD , ship , buf );
             }

             return;
    	}

        if ( ch->in_room->vnum == ship->turret2 && !str_prefix( argument , "lasers") )
    	{
    	     if (ship->statet2 == LASER_DAMAGED)
    	     {
    	        send_to_char("&RThe ships turret is damaged.\n\r",ch);
    	      	return;
    	     }
             if (ship->statet2 > ship->class )
    	     {
    	     	send_to_char("&RThe turbolaser is still recharging.\n\r",ch);
    	     	return;
    	     }
    	     if (ship->target2 == NULL )
    	     {
    	     	send_to_char("&RYou need to choose a target first.\n\r",ch);
    	     	return;
    	     }
    	     target = ship->target2;
             if (ship->target2->starsystem != ship->starsystem)
    	     {
    	     	send_to_char("&RYour target seems to have left.\n\r",ch);
    	        ship->target2 = NULL;
    	     	return;
    	     }
    	     if ( abs(target->vx - ship->vx) >1000 ||
                  abs(target->vy - ship->vy) >1000 ||
                  abs(target->vz - ship->vz) >1000 )
             {
                send_to_char("&RThat ship is out of laser range.\n\r",ch);
    	     	return;
             }
             ship->statet2++;
             chance -= target->manuever/10;
             chance += target->class*25;
             chance -= target->currspeed/20;
             chance -= ( abs(target->vx - ship->vx)/70 );
             chance -= ( abs(target->vy - ship->vy)/70 );
             chance -= ( abs(target->vz - ship->vz)/70 );
             chance = URANGE( 10 , chance , 90 );
             act( AT_PLAIN, "$n presses the fire button.", ch,
                  NULL, argument , TO_ROOM );
             if ( number_percent( ) > chance )
             {
                sprintf( buf, "Turbolasers fire from %s barely missing %s." , ship->name, target->name );
                echo_to_system( AT_ORANGE , ship , buf , target );
                sprintf( buf , "Turbolasers fire from %s at you but miss." , ship->name);
                echo_to_cockpit( AT_ORANGE , target , buf );
                sprintf( buf , "Turbolasers fire from the turret missing %s." , target->name);
                echo_to_cockpit( AT_ORANGE , ship , buf );
                learn_from_failure( ch, gsn_spacecombat );
    	        learn_from_failure( ch, gsn_spacecombat2 );
    	        learn_from_failure( ch, gsn_spacecombat3 );
    	        return;
             }
             sprintf( buf, "Turbolasers fire from %s, hitting %s." , ship->name, target->name );
             echo_to_system( AT_ORANGE , ship , buf , target );
             sprintf( buf , "You are hit by turbolasers from %s!" , ship->name);
             echo_to_cockpit( AT_BLOOD , target , buf );
             sprintf( buf , "turbolasers fire from the turret hitting %s!." , target->name);
             echo_to_cockpit( AT_YELLOW , ship , buf );
             learn_from_success( ch, gsn_spacecombat );
             learn_from_success( ch, gsn_spacecombat2 );
             learn_from_success( ch, gsn_spacecombat3 );
             echo_to_ship( AT_RED , target , "A small explosion vibrates through the ship." );
             damage_ship_ch( target , 10 , 25 , ch);

             if ( autofly(target) && target->target0 != ship )
             {
                target->target0 = ship;
                sprintf( buf , "You are being targetted by %s." , target->name);
                echo_to_cockpit( AT_BLOOD , ship , buf );
             }

             return;
    	}

        send_to_char( "&RYou can't fire that!\n\r" , ch);

} */

/* Modified to work with the new hyperspace -Kristen- */
void do_calculate(CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
    int chance , count;
    SHIP_DATA *ship;
    SPACE_DATA *starsystem;

    argument = one_argument( argument , arg);

    if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    {
    	send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	return;
    }

    if ( ship->class > SHIP_PLATFORM )
    {
        send_to_char("&RThis isn't a spacecraft!\n\r",ch);
 	    return;
    }

    if (  (ship = ship_from_navseat(ch->in_room->vnum))  == NULL )
    {
   		send_to_char("&RYou must be at a nav computer to calculate jumps.\n\r",ch);
        return;
    }

    if ( autofly(ship)  )
    {
        send_to_char("&RYou'll have to turn off the ships autopilot first....\n\r",ch);
        return;
    }

    if ( ship->mono_disabled ) /* MONO support added - Funf */
    {
		send_to_char("&RMONO system down! Calculations impossible.\n\r", ch);
		return;
	}

    if  ( ship->class == SHIP_PLATFORM )
    {
        send_to_char( "&RAnd what exactly are you going to calculate...?\n\r" , ch );
        return;
    }

    if (ship->shipstate == SHIP_DOCKED)
    {
        send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
        return;
    }

    if (ship->starsystem == NULL)
    {
        send_to_char("&RYou can only do that in realspace.\n\r",ch);
        return;
    }

    if (arg[0] == '\0')
    {
    	send_to_char("&WFormat: Calculate <planet>\n\r&wPossible destinations:\n\r",ch);
        for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
        {
        	set_char_color( AT_NOTE, ch );
            ch_printf(ch,"%-30s %d\n\r",starsystem->name,
              (abs(starsystem->xpos - ship->starsystem->xpos)+
               abs(starsystem->ypos - ship->starsystem->ypos))/2);
            count++;
        }
        if ( !count )
        {
        	send_to_char( "No Starsystems found.\n\r", ch );
        }
        return;
    }

    chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_navigation]) ;
    if ( number_percent( ) > chance )
    {
		send_to_char("&RYou cant seem to figure the charts out today.\n\r",ch);
	    learn_from_failure( ch, gsn_navigation );
    	return;
    }

    for ( starsystem = first_starsystem; starsystem; starsystem = starsystem->next )
    {
       	if ( !str_cmp( arg, starsystem->name ) )
	{
			ship->currjump = starsystem_from_name( arg );
			break;
	}
    }

    if ( ship->currjump != NULL )
    {
		ship->jx = ship->currjump->g1x;
		ship->jy = ship->currjump->g1y;
    	ship->jz = ship->currjump->g1z;
	}

    if ( ship->currjump == NULL || ship->currjump == ship->starsystem )
    {
        send_to_char( "&RInvalid destination.\n\r", ch);
        return;
    }
    else
    {
        SPACE_DATA * starsystem;

        starsystem = ship->currjump;
    }

    ship->hyperdistance  = abs(ship->starsystem->xpos - ship->currjump->xpos) ;
    ship->hyperdistance += abs(ship->starsystem->ypos - ship->currjump->ypos) ;
    ship->hyperdistance /= 5;

    if (ship->hyperdistance<100)
      ship->hyperdistance = 100;

    ship->hyperdistance += number_range(0, 200);

    send_to_char( "&GHyperspace course set. Ready for entry into a gate.\n\r", ch);
    act( AT_PLAIN, "$n does some calculations using the ships computer.", ch,
		        NULL, argument , TO_ROOM );

    learn_from_success( ch, gsn_navigation );

    WAIT_STATE( ch , 2*PULSE_VIOLENCE );
}

void do_repairship(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance, change;
    SHIP_DATA *ship;
    SPACE_DATA *starsystem;
    OBJ_DATA *component;
    int lag;

    strcpy( arg, argument );


    switch( ch->substate )
    {
    	default:
    	        if (  (ship = ship_from_engine(ch->in_room->vnum))  == NULL )
    	        {
    	            send_to_char("&RYou must be in the engine room of a ship to do that!\n\r",ch);
    	            return;
    	        }

		        if (ship->shipstate == SHIP_HYPERSPACE )
		        {
        			send_to_char("You can't repair your ship in hyperspace!\n\r",ch);
        			return;
    	        }


                if ( str_cmp( argument , "hull" ) && str_cmp( argument , "drive" ) &&
                     str_cmp( argument , "weapon" ) && str_cmp( argument , "mono" )  )
                {
                   send_to_char("&RYou need to specify something to repair:\n\r",ch);
                   send_to_char("&rTry: hull, drive, weapon, or mono.\n\r",ch);
                   return;
                }

                chance = 0;
                lag = 5;

                if ( !str_cmp( argument , "mono" ) ) /* MONO support - Funf */
                {
					if ( !ship->mono_disabled )
					{
						send_to_char( "&GMONO system already active.\n\r", ch);
						return;
					}
					chance -= 90;
					lag = 15;
				}
				else
                if ( !str_cmp( argument , "weapon" ) ) /* New weapon and disable support - Funf */
                {
					int found = 0;
                	for ( component = ship->first_component ; component ; component = component->first_content )
                	{

						if ( component->value[0] == COMPONENT_WEAPON && component->value[3] != DISABLE_NONE )
						{
							switch ( component->value[3] )
							{
							DISABLE_MONO:
								chance -= 60;
							DISABLE_SABOTAGE:
								chance -= 30;
								lag = 15;
								break;
							DISABLE_EMP:
								sprintf( arg, "%s mpdamage %s 100", ch->name, ch->name );
								do_mpat( supermob, arg ); /* damage char */
								strcpy( arg, argument ); /* reseting */
								send_to_char("&YAn arc of electricity strikes you from the weapon!&w\n\r", ch);
								act( AT_PLAIN, "$n gets struck by a electric discharge from a $T.", ch, NULL, argument , TO_ROOM );
							default:
								lag = 5;
							}
							found = 1;
							break;
						}
					}

					if ( found == 0 )
					{
						send_to_char("&GThis ship has no weapons in need of repair!&w\n\r", ch);
						return;
					}
				}

                chance += IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_shipmaintenance]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin your repairs\n\r", ch);
    		   act( AT_PLAIN, "$n begins repairing the ships $T.", ch,
		        NULL, argument , TO_ROOM );
    		   if ( !str_cmp(arg,"hull") )
    		     add_timer ( ch , TIMER_DO_FUN , 15 , do_repairship , 1 );
    		   else
    		     add_timer ( ch , TIMER_DO_FUN , lag , do_repairship , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou fail to locate the source of the problem.\n\r",ch);
	        learn_from_failure( ch, gsn_shipmaintenance );
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
    		if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    		      return;
    	        send_to_char("&RYou are distracted and fail to finish your repairs.\n\r", ch);
    		return;
    }

    ch->substate = SUB_NONE;

    if ( (ship = ship_from_engine(ch->in_room->vnum)) == NULL )
    {
       return;
    }

    if ( !str_cmp(arg,"hull") )
    {  /* amount upped - Funf */
		change = (int) (ship->maxhull * (ch->pcdata->learned[gsn_shipmaintenance] / 2000.) + (get_curr_int(ch) - 10) / 300.);
        change = URANGE( 0, number_range((int)(change * 0.75), (int)(change * 1.25)), ( ship->maxhull - ship->hull ) );
        ship->hull += change;
        ch_printf( ch, "&GRepair complete. Hull strength inreased by %d points.\n\r", change );
    }

    if ( !str_cmp(arg,"drive") )
    {
       if (ship->location == ship->lastdoc)
          ship->shipstate = SHIP_DOCKED;
       else
          ship->shipstate = SHIP_READY;
       send_to_char("&GShips drive repaired.\n\r", ch);
    }

    if ( !str_cmp(arg,"mono") )
    {
		if (ship->mono_disabled) {
			ship->mono_disabled = FALSE;
			calc_ship_speed( ship ); /* refresh speed */
		}
		send_to_char("&GMONO rebooted.\n\r", ch);
	}

    if ( !str_cmp(arg,"weapon") )
    {
		component->value[3] = DISABLE_NONE; /* I'm assuming component is still valid, might cause random crashes - Funf */
		send_to_char("&GWeapon repaired.\n\r", ch);
	}

    /*if ( !str_cmp(arg,"launcher") )
    {
       ship->missilestate = MISSILE_READY;
       send_to_char("&GMissile launcher repaired.\n\r", ch);
    }

    if ( !str_cmp(arg,"laser") )
    {
       ship->statet0 = LASER_READY;
       send_to_char("&GMain laser repaired.\n\r", ch);
    }

    if ( !str_cmp(arg,"turret 1") )
    {
       ship->statet1 = LASER_READY;
       send_to_char("&GLaser Turret 1 repaired.\n\r", ch);
    }

    if ( !str_cmp(arg,"turret 2") )
    {
       ship->statet2 = LASER_READY;
       send_to_char("&Laser Turret 2 repaired.\n\r", ch);
    }*/

    act( AT_PLAIN, "$n finishes the repairs.", ch,
         NULL, argument , TO_ROOM );

    learn_from_success( ch, gsn_shipmaintenance );

}


void do_refuel(CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	int value,price;
	SHIP_DATA *ship;

	argument = one_argument( argument, arg1 );
	value = atoi(arg1);
	value = abs(value);

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

	if ( ship->energy == ship->maxenergy )
	{
		send_to_char("&RYou are already at max capacity.\n\r",ch);
		return;
	}

	if ( ( value + ship->energy ) > ship->maxenergy )
	{
		send_to_char("&RYou don't have that much fuel capacity.\n\r",ch);
		return;
	}

	if ( (ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL )
	{
		send_to_char("&RYou don't seem to be in the pilot seat!\n\r",ch);
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

	if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED  )
	{
		send_to_char("The ship is not docked right now.\n\r",ch);
		return;
	}

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: refuel <amount/full>\n\r", ch );
		send_to_char( "Price: 1 wulong for 2 fuel units\n\r", ch);
		return;
	}

        if ( !str_cmp("1",arg1) )
        {
                price = value;
                if (ch->gold < price) {
					ch_printf( ch, "You don't have enough wulongs for that.\n\r" );
					return;
				}
                ch_printf( ch, "You pay %d wulongs for %d units of fuel.\n\r", price, value );
                ship->energy += value;
				ch->gold -= price;
                return;
        }

	if ( !str_cmp("full",arg1) )
	{
		value = ship->maxenergy-ship->energy;
		price = value/2;
		if (ch->gold < price) {
			ch_printf( ch, "You don't have enough wulongs for that.\n\r" );
			return;
		}
		ch_printf( ch, "You pay %d wulongs for %d units of fuel.\n\r", price, value );
		ship->energy += value;
		ch->gold -= price;
		return;
	}
	value = atoi(arg1);
	price = value/2;
	if (ch->gold < price) {
		ch_printf( ch, "You don't have enough wulongs for that.\n\r" );
		return;
	}
	ch_printf( ch, "You pay %d wulongs for %d units of fuel.\n\r", price, value );
	ship->energy += value;
	ch->gold -= price;
}

void do_addpilot(CHAR_DATA *ch, char *argument )
{
	SHIP_DATA *ship;

   	if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
	{
		send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
		return;
	}

	if  ( ship->class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't do that here.\n\r" , ch );
		return;
	}

   	if ( str_cmp( ship->owner , ch->name ) )
   	{

    	if ( !IS_NPC(ch) && ch->pcdata && ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name,ship->owner) )
        	if ( !str_cmp(ch->pcdata->clan->leader,ch->name) )
           	;
         	else if ( !str_cmp(ch->pcdata->clan->number1,ch->name) )
           	;
         	else if ( !str_cmp(ch->pcdata->clan->number2,ch->name) )
           	;
         	else
         	{
           		send_to_char( "&RThat isn't your ship!\n\r" ,ch );
   	   			return;
         	}
     	else
     	{
   			send_to_char( "&RThat isn't your ship!\n\r" ,ch );
   			return;
     	}

	}

   	if (argument[0] == '\0')
   	{
    	send_to_char( "&RAdd which pilot?\n\r" ,ch );
    	return;
   	}

   	if ( str_cmp( ship->pilot , "" ) )
   	{
        if ( str_cmp( ship->copilot , "" ) )
        {
             send_to_char( "&RYou are ready have a pilot and copilot..\n\r" ,ch );
             send_to_char( "&RTry rempilot first.\n\r" ,ch );
             return;
        }

        STRFREE( ship->copilot );
	ship->copilot = STRALLOC( argument );
	send_to_char( "Copilot Added.\n\r", ch );
	save_ship( ship );
	return;

        return;
   	}

   	STRFREE( ship->pilot );
   	ship->pilot = STRALLOC( argument );
   	send_to_char( "Pilot Added.\n\r", ch );
   	save_ship( ship );

}

void do_rempilot(CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *ship;

   if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
   {
           send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
           return;
   }

                if  ( ship->class == SHIP_PLATFORM )
                {
                   send_to_char( "&RYou can't do that here.\n\r" , ch );
                   return;
                }

   if ( str_cmp( ship->owner , ch->name ) )
   {

     if ( !IS_NPC(ch) && ch->pcdata && ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name,ship->owner) )
         if ( !str_cmp(ch->pcdata->clan->leader,ch->name) )
           ;
         else if ( !str_cmp(ch->pcdata->clan->number1,ch->name) )
           ;
         else if ( !str_cmp(ch->pcdata->clan->number2,ch->name) )
           ;
         else
         {
           send_to_char( "&RThat isn't your ship!\n\r" ,ch );
   	   return;
         }
     else
     {
   	send_to_char( "&RThat isn't your ship!\n\r" ,ch );
   	return;
     }

   }

   if (argument[0] == '\0')
   {
       send_to_char( "&RRemove which pilot?\n\r" ,ch );
       return;
   }

   if ( !str_cmp( ship->pilot , argument ) )
   {
        STRFREE( ship->pilot );
	ship->pilot = STRALLOC( "" );
	send_to_char( "Pilot Removed.\n\r", ch );
	save_ship( ship );
	return;
   }

   if ( !str_cmp( ship->copilot , argument ) )
   {
        STRFREE( ship->copilot );
	ship->copilot = STRALLOC( "" );
	send_to_char( "Copilot Removed.\n\r", ch );
	save_ship( ship );
	return;
    }

    send_to_char( "&RThat person isn't listed as one of the ships pilots.\n\r" ,ch );

}

void do_radar( CHAR_DATA *ch, char *argument )
{
    SHIP_DATA *target;
    int chance;
    SHIP_DATA *ship;
    MISSILE_DATA *missile;

        if (   (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
        {
            send_to_char("&RYou must be in the cockpit or turret of a ship to do that!\n\r",ch);
            return;
        }

        if ( ship->class > SHIP_PLATFORM )
    	{
    		send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    		return;
    	}

        if (ship->shipstate == SHIP_DOCKED)
        {
            send_to_char("&RWait until after you launch!\n\r",ch);
            return;
        }

        if (ship->shipstate == SHIP_HYPERSPACE)
        {
            send_to_char("&RYou can only do that in realspace!\n\r",ch);
            return;
        }

    	if (ship->starsystem == NULL)
    	{
    	       send_to_char("&RYou can't do that unless the ship is flying in realspace!\n\r",ch);
    	       return;
    	}

    	if (ship->mono_disabled) /* MONO support - Funf */
    	{
			send_to_char("&RMONO system down! Radar is dead!\n\r",ch);
			return;
		}

        chance = IS_NPC(ch) ? ch->top_level
             : (int)  (ch->pcdata->learned[gsn_navigation]) ;
        if ( number_percent( ) > chance )
        {
           send_to_char("&RYou fail to work the controls properly.\n\r",ch);
           learn_from_failure( ch, gsn_navigation );
    	   return;
        }


    act( AT_PLAIN, "$n checks the radar.", ch,
         NULL, argument , TO_ROOM );

    	           set_char_color(  AT_WHITE, ch );
    	           ch_printf(ch, "%s\n\r\n\r" , ship->starsystem->name );
    	           set_char_color(  AT_LBLUE, ch );
    	           if ( ship->starsystem->gate1 && str_cmp(ship->starsystem->gate1,"") )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->gate1,
    	                        ship->starsystem->g1x,
    	                        ship->starsystem->g1y,
    	                        ship->starsystem->g1z );
    	           if ( ship->starsystem->gate2 && str_cmp(ship->starsystem->gate2,"")  )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->gate2,
    	                        ship->starsystem->g2x,
    	                        ship->starsystem->g2y,
    	                        ship->starsystem->g2z );
    	           if ( ship->starsystem->gate3 && str_cmp(ship->starsystem->gate3,"") )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->gate3,
    	                        ship->starsystem->g3x,
    	                        ship->starsystem->g3y,
    	                        ship->starsystem->g3z );
    	           if ( ship->starsystem->gate4 && str_cmp(ship->starsystem->gate4,"")  )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->gate4,
    	                        ship->starsystem->g4x,
    	                        ship->starsystem->g4y,
    	                        ship->starsystem->g4z );
    	           if ( ship->starsystem->gate5 && str_cmp(ship->starsystem->gate5,"") )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->gate5,
    	                        ship->starsystem->g5x,
    	                        ship->starsystem->g5y,
    	                        ship->starsystem->g5z );
    	           if ( ship->starsystem->gate6 && str_cmp(ship->starsystem->gate6,"")  )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->gate6,
    	                        ship->starsystem->g6x,
    	                        ship->starsystem->g6y,
    	                        ship->starsystem->g6z );
    	           if ( ship->starsystem->planet1 && str_cmp(ship->starsystem->planet1,"") )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->planet1,
    	                        ship->starsystem->p1x,
    	                        ship->starsystem->p1y,
    	                        ship->starsystem->p1z );
    	           if ( ship->starsystem->planet2 && str_cmp(ship->starsystem->planet2,"")  )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->planet2,
    	                        ship->starsystem->p2x,
    	                        ship->starsystem->p2y,
    	                        ship->starsystem->p2z );
    	           if ( ship->starsystem->planet3 && str_cmp(ship->starsystem->planet3,"")  )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->planet3,
    	                        ship->starsystem->p3x,
    	                        ship->starsystem->p3y,
    	                        ship->starsystem->p3z );
    	           if ( ship->starsystem->planet4 && str_cmp(ship->starsystem->planet4,"")  )
    	                 ch_printf(ch, "%s   %d %d %d\n\r" ,
    	                        ship->starsystem->planet4,
    	                        ship->starsystem->p4x,
    	                        ship->starsystem->p4y,
    	                        ship->starsystem->p4z );
    	           ch_printf(ch,"\n\r");
    	           for ( target = ship->starsystem->first_ship; target; target = target->next_in_starsystem )
                   {
                        if ( target != ship )
                           ch_printf(ch, "%s    %.0f %.0f %.0f\n\r",
                           	target->name,
                           	target->vx,
                           	target->vy,
                           	target->vz);
                   }
                   ch_printf(ch,"\n\r");
    	           for ( missile = ship->starsystem->first_missile; missile; missile = missile->next_in_starsystem )
                   {
                           ch_printf(ch, "%s    %d %d %d\n\r",
                           	missile->missiletype == CONCUSSION_MISSILE ? "A Concusion missile" :
    			        ( missile->missiletype ==  PROTON_TORPEDO ? "A Torpedo" :
    			        ( missile->missiletype ==  HEAVY_ROCKET ? "A Heavy Rocket" : "A Heavy Bomb" ) ),
                           	missile->mx,
                           	missile->my,
                                missile->mz );
                   }

                   ch_printf(ch, "\n\r&WYour Coordinates: %.0f %.0f %.0f\n\r" ,
                             ship->vx , ship->vy, ship->vz);


        learn_from_success( ch, gsn_navigation );

}

void do_autotrack( CHAR_DATA *ch, char *argument )
{
   SHIP_DATA *ship;
   int chance;

   if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
   {
        send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
        return;
   }

   if ( ship->class > SHIP_PLATFORM )
   {
        send_to_char("&RThis isn't a spacecraft!\n\r",ch);
        return;
   }

   if ( ship->class == SHIP_PLATFORM )
   {
        send_to_char("&RPlatforms don't have autotracking systems!\n\r",ch);
        return;
   }

   if (  (ship = ship_from_pilotseat(ch->in_room->vnum))  == NULL )
   {
        send_to_char("&RYou aren't in the pilot's chair!\n\r",ch);
        return;
   }

   if ( autofly(ship)  )
   {
    	send_to_char("&RYou'll have to turn off the ships autopilot first....\n\r",ch);
    	return;
   }

   if ( ship->mono_disabled )
   {
	   send_to_char("&RMONO system down! Computerized tracking impossible!\n\r",ch);
	   return;
   }

   chance = IS_NPC(ch) ? ch->top_level
             : (int)  (ch->pcdata->learned[gsn_shipsystems]) ;
        if ( number_percent( ) > chance )
        {
           send_to_char("&RYou're not sure which switch to flip.\n\r",ch);
           learn_from_failure( ch, gsn_shipsystems );
    	   return;
        }

   act( AT_PLAIN, "$n flips a switch on the control panel.", ch,
         NULL, argument , TO_ROOM );
   if (ship->autotrack)
   {
     ship->autotrack = FALSE;
     echo_to_cockpit( AT_YELLOW , ship, "Autotracking off.");
   }
   else
   {
      ship->autotrack = TRUE;
      echo_to_cockpit( AT_YELLOW , ship, "Autotracking on.");
   }

   learn_from_success( ch, gsn_shipsystems );

}

bool ship_was_in_range( SHIP_DATA *ship, SHIP_DATA *target )
{
  if (target && ship && target != ship )
    if ( abs(target->ox - ship->vx) < 100*(ship->sensor+10)*((target->class == SHIP_READY ? 2 : target->class)+3) &&
         abs(target->oy - ship->vy) < 100*(ship->sensor+10)*((target->class == SHIP_READY ? 2 : target->class)+3) &&
         abs(target->oz - ship->vz) < 100*(ship->sensor+10)*((target->class == SHIP_READY ? 2 : target->class)+3) )
      return TRUE;
  return FALSE;
}

void do_jumpvector( CHAR_DATA *ch, char *argument )
{
    int chance, num;
    float tx, ty, tz;
    SHIP_DATA *ship;
    SHIP_DATA *target;
    char buf[MAX_STRING_LENGTH];

    num = number_range( 1, 16 );

    if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    {
    	send_to_char("&RYou must be in the cockpit, turret or engineroom of a ship to do that!\n\r",ch);
       	return;
    }

    target = get_ship( argument );

    if ( !target )
    {
		send_to_char( "No such ship.\n\r", ch );
		return;
    }

    if ( target == ship )
    {
		send_to_char( "You can figure out where you are going on your own.\n\r", ch );
		return;
    }

    if ( ship->mono_disabled )
    {
	   send_to_char("&RMONO system down! Vectors unavailable!\n\r",ch);
	   return;
    }

    if (!ship_was_in_range( ship, target ))
    {
    	send_to_char( "No log for that ship.\n\r", ch);
    	return;
    }

    if (target->shipstate == SHIP_DOCKED)
    {
    	send_to_char( "No log for that ship.\n\r", ch);
    	return;
    }

    chance = IS_NPC(ch) ? ch->top_level : (int)  (ch->pcdata->learned[gsn_jumpvector]) ;

    if ( number_percent( ) > chance )
    {
        send_to_char("&RYou cant figure out the vectors correctly.\n\r",ch);
        learn_from_failure( ch, gsn_shipsystems );
        return;
    }

    if( target->shipstate == SHIP_HYPERSPACE )
    {
    	tx = (target->currjump->g1x - num);
      	ty = (target->currjump->g1y + num);
      	tz = (target->currjump->g1z - num);

      	send_to_char("After some deliberation, you figure out its projected course.\n\r", ch);
      	sprintf(buf, "%s Heading: %.0f, %.0f, %.0f", target->name, tx, ty, tz );
      	echo_to_cockpit( AT_BLOOD, ship , buf );
      	learn_from_success( ch, gsn_jumpvector );
      	return;
    }

	tx = (target->hx + num);
    ty = (target->hy - num);
    tz = (target->hz + num);

    send_to_char("After some deliberation, you figure out its projected course.\n\r", ch);
    sprintf(buf, "%s Heading: %.0f, %.0f, %.0f", target->name, tx, ty, tz  );
    echo_to_cockpit( AT_BLOOD, ship , buf );
    learn_from_success( ch, gsn_jumpvector );
    return;

}

void do_reload( CHAR_DATA *ch, char *argument )
{

  /* Reload code added by Darrik Vequir */

  char arg[MAX_INPUT_LENGTH];
  SHIP_DATA *ship;
  long long int price = 5000;


  strcpy( arg, argument );

  if (arg[0] == '\0')
  {
    send_to_char("&RYou need to specify a target!\n\r",ch);
    return;
  }

  if ( ( ship = ship_in_room( ch->in_room , argument ) ) == NULL )
  {
    act( AT_PLAIN, "I see no $T here.", ch, NULL, argument, TO_CHAR );
    return;
  }

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

  if ( ch->pcdata && ch->pcdata->clan && !str_cmp(ch->pcdata->clan->name,ship->owner) )
  {
    if ( ch->pcdata->clan->funds < price )
    {
      ch_printf(ch, "&R%s doesn't have enough funds to prepare this ship for launch.\n\r", ch->pcdata->clan->name );
      return;
    }

    ch->pcdata->clan->funds -= price;
    ch_printf(ch, "&GIt costs %s %lld wulongs to ready this ship for launch.\n\r", ch->pcdata->clan->name, price );
  }
  else if ( str_cmp( ship->owner , "Public" ) )
  {
    if ( ch->gold < price )
    {
      ch_printf(ch, "&RYou don't have enough funds to prepare this ship for launch.\n\r");
      return;
    }

    ch->gold -= price;
    ch_printf(ch, "&GYou pay %lld wulongs to ready the ship for launch.\n\r", price );
  }

  ship->energy = ship->maxenergy;
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

  return;

 }

void do_openbay( CHAR_DATA *ch, char *argument )
{
  SHIP_DATA *ship;
  char buf[MAX_STRING_LENGTH];

   if ( ship_from_pilotseat(ch->in_room->vnum) == NULL
   && ship_from_hangar(ch->in_room->vnum) == NULL )
   {
        send_to_char("&RYou aren't in the pilots chair or hangar of a ship!\n\r",ch);
        return;
   }

   if ( ship_from_pilotseat(ch->in_room->vnum) )
      ship = ship_from_pilotseat(ch->in_room->vnum);
   else
      ship = ship_from_hangar(ch->in_room->vnum);

   if ( ship->hangar == 0 )
   {
      send_to_char("&RThis ship has no hangar!\n\r",ch);
      return;
   }

   if (ship->bayopen == TRUE)
   {
      send_to_char("Bay doors are already open!\n\r",ch);
      return;
   }

   act( AT_PLAIN, "$n flips a switch on the control panel.", ch,
         NULL, argument , TO_ROOM );
      ship->bayopen = TRUE;

      echo_to_cockpit( AT_YELLOW , ship, "Bay Doors Opened");
      send_to_char("You open the bay doors.\n\r", ch);
      sprintf( buf ,"%s's bay doors open." , ship->name );
      echo_to_system( AT_YELLOW, ship, buf , NULL );

   }

void do_closebay( CHAR_DATA *ch, char *argument )
{
  SHIP_DATA *ship;
  char buf[MAX_STRING_LENGTH];
      if ( ship_from_pilotseat(ch->in_room->vnum) == NULL
   && ship_from_hangar(ch->in_room->vnum) == NULL )
   {
        send_to_char("&RYou aren't in the pilots chair or hangar of a ship!\n\r",ch);
        return;
   }

   if ( ship_from_pilotseat(ch->in_room->vnum) )
      ship = ship_from_pilotseat(ch->in_room->vnum);
   else
      ship = ship_from_hangar(ch->in_room->vnum);

   if ( ship->hangar == 0 )
   {
      send_to_char("&RThis ship has no hangar!\n\r",ch);
      return;
   }

   if (ship->bayopen == FALSE)
   {
      send_to_char("Bay doors are already closed!\n\r", ch);
      return;
   }

   act( AT_PLAIN, "$n flips a switch on the control panel.", ch,
         NULL, argument , TO_ROOM );
      ship->bayopen = FALSE;

      echo_to_cockpit( AT_YELLOW , ship, "Bay Doors closed");
      send_to_char("You close the bay doors.\n\r", ch);
      sprintf( buf ,"%s's bay doors close." , ship->name );
      echo_to_system( AT_YELLOW, ship, buf , NULL );

   }

void do_tractorbeam( CHAR_DATA *ch, char *argument )
{

    char arg[MAX_INPUT_LENGTH];
    int chance;
    SHIP_DATA *ship;
    SHIP_DATA *target;
	char buf[MAX_STRING_LENGTH];

    strcpy( arg, argument );

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


	if ( !check_pilot( ch , ship ) )
	{
		send_to_char("This isn't your ship!\n\r" , ch );
		return;
	}

	if ( ship->tractorbeam == 0 )
	{
		send_to_char("You might want to install a tractorbeam!\n\r" , ch );
		return;
	}

	if ( ship->hangar == 0 )
	{
		send_to_char("No hangar available.\n\r",ch);
		return;
	}

	if ( !ship->bayopen )
	{
		send_to_char("Your hangar is closed.\n\r",ch);
		return;
	}


	if ( (ship = ship_from_pilotseat(ch->in_room->vnum)) == NULL )
	{
		send_to_char("&RYou need to be in the pilot seat!\n\r",ch);
		return;
	}


	if (ship->shipstate == SHIP_DISABLED)
	{
		send_to_char("&RThe ships drive is disabled. No power available.\n\r",ch);
		return;
	}

	if (ship->shipstate == SHIP_DOCKED)
	{
		send_to_char("&RYour ship is docked!\n\r",ch);
		return;
	}

	if (ship->shipstate == SHIP_HYPERSPACE)
	{

		send_to_char("&RYou can only do that in realspace!\n\r",ch);
		return;
	}

	if (ship->shipstate != SHIP_READY)
	{
		send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
		return;
	}




	if ( argument[0] == '\0' )
	{
		send_to_char("&RCapture what?\n\r",ch);
		return;
	}

	target = get_ship_here( argument , ship->starsystem );

	if ( target == NULL )
	{
		send_to_char("&RI don't see that here.\n\r",ch);
		return;
	}

	if ( target == ship )
	{
		send_to_char("&RYou can't yourself!\n\r",ch);
		return;
	}

	if ( target->shipstate == SHIP_LAND )
	{
		send_to_char("&RThat ship is already in a landing sequence.\n\r", ch);
		return;
	}

	if (  (target->vx > ship->vx + 200) || (target->vx < ship->vx - 200) ||
	   (target->vy > ship->vy + 200) || (target->vy < ship->vy - 200) ||
	   (target->vz > ship->vz + 200) || (target->vz < ship->vz - 200) )
	{
		send_to_char("&RThat ship is too far away! You'll have to fly a little closer.\n\r",ch);
		return;
	}

	if (ship->class <= target->class)
	{
		send_to_char("&RThat ship is too big for your hangar.\n\r",ch);
		return;
	}

	if  ( target->class == SHIP_PLATFORM )
	{
		send_to_char( "&RYou can't capture platforms.\n\r" , ch );
		return;
	}

	if ( target->class == CAPITAL_SHIP)
	{
		send_to_char("&RYou can't capture capital ships.\n\r",ch);
		return;
	}


	if ( ship->energy < (25 + 25*target->class) )
	{
		send_to_char("&RTheres not enough fuel!\n\r",ch);
		return;
	}




	chance = IS_NPC(ch) ? ch->top_level
	: (int)  (ch->pcdata->learned[gsn_tractorbeams]);

	/* This is just a first guess chance modifier, feel free to change if needed */

	chance = chance * ( ship->tractorbeam / (target->currspeed+1 ) );

	if ( number_percent( ) < chance )
	{
		set_char_color( AT_GREEN, ch );
		send_to_char( "Capture sequence initiated.\n\r", ch);
		act( AT_PLAIN, "$n begins the capture sequence.", ch,
			NULL, argument , TO_ROOM );
		echo_to_ship( AT_YELLOW , ship , "ALERT: Ship is being captured, all hands to docking bay." );
    	echo_to_ship( AT_YELLOW , target , "The ship shudders as a tractorbeam locks on." );
		sprintf( buf , "You are being captured by %s." , ship->name);
		echo_to_cockpit( AT_BLOOD , target , buf );

		if ( autofly(target) && !target->target0)
			target->target0 = ship;

		target->dest = STRALLOC(ship->name);
		target->shipstate = SHIP_LAND;
		target->currspeed = 0;

		learn_from_success( ch, gsn_tractorbeams );
		return;

	}
	send_to_char("You fail to work the controls properly.\n\r",ch);
   	echo_to_ship( AT_YELLOW , target , "The ship shudders and then stops as a tractorbeam attemps to lock on." );
	sprintf( buf , "The %s attempted to capture your ship!" , ship->name);
	echo_to_cockpit( AT_BLOOD , target , buf );
	if ( autofly(target) && !target->target0)
		target->target0 = ship;


	learn_from_failure( ch, gsn_tractorbeams );

   	return;
}

void do_fly( CHAR_DATA *ch, char *argument )
{}

void do_drive( CHAR_DATA *ch, char *argument )
{
    int dir;
    SHIP_DATA *ship;

        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
        {
            send_to_char("&RYou must be in the drivers seat of a land vehicle to do that!\n\r",ch);
            return;
        }

        if ( ship->class < LAND_SPEEDER )
    	{
    	      send_to_char("&RThis isn't a land vehicle!\n\r",ch);
    	      return;
    	}


        if (ship->shipstate == SHIP_DISABLED)
    	{
    	     send_to_char("&RThe drive is disabled.\n\r",ch);
    	     return;
    	}

        if ( ship->energy <1 )
        {
              send_to_char("&RTheres not enough fuel!\n\r",ch);
              return;
        }

        if ( ( dir = get_door( argument ) ) == -1 )
        {
             send_to_char( "Usage: drive <direction>\n\r", ch );
             return;
        }

        drive_ship( ch, ship, get_exit(get_room_index(ship->location), dir), 0 );

}

ch_ret drive_ship( CHAR_DATA *ch, SHIP_DATA *ship, EXIT_DATA  *pexit , int fall )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    ROOM_INDEX_DATA *original;
    char buf[MAX_STRING_LENGTH];
    char *txt;
    char *dtxt;
    ch_ret retcode;
    sh_int door, distance;
    bool drunk = FALSE;
    CHAR_DATA * rch;
    CHAR_DATA * next_rch;


    if ( !IS_NPC( ch ) )
      if ( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE )
	&& ( ch->position != POS_DRAG ) )
	drunk = TRUE;

    if ( drunk && !fall )
    {
      door = number_door();
      pexit = get_exit( get_room_index(ship->location), door );
    }

#ifdef DEBUG
    if ( pexit )
    {
	sprintf( buf, "drive_ship: %s to door %d", ch->name, pexit->vdir );
	log_string( buf );
    }
#endif

    retcode = rNONE;
    txt = NULL;

    in_room = get_room_index(ship->location);
    from_room = in_room;
    if ( !pexit || (to_room = pexit->to_room) == NULL )
    {
	if ( drunk )
	  send_to_char( "You drive into a wall in your drunken state.\n\r", ch );
	 else
	  send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return rNONE;
    }

    door = pexit->vdir;
    distance = pexit->distance;

    if ( IS_SET( pexit->exit_info, EX_WINDOW )
    &&  !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return rNONE;
    }

    if (  IS_SET(pexit->exit_info, EX_PORTAL)
       && IS_NPC(ch) )
    {
        act( AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_NOMOB)
	&& IS_NPC(ch) )
    {
	act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
    {
	if ( !IS_SET( pexit->exit_info, EX_SECRET )
	&&   !IS_SET( pexit->exit_info, EX_DIG ) )
	{
	  if ( drunk )
	  {
	    act( AT_PLAIN, "$n drives into the $d in $s drunken state.", ch,
		NULL, pexit->keyword, TO_ROOM );
	    act( AT_PLAIN, "You drive into the $d in your drunken state.", ch,
		NULL, pexit->keyword, TO_CHAR );
	  }
	 else
	  act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
       else
	{
	  if ( drunk )
	    send_to_char( "You hit a wall in your drunken state.\n\r", ch );
	   else
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );
	}

	return rNONE;
    }

/*
    if ( distance > 1 )
	if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );
*/
    if ( room_is_private( ch, to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return rNONE;
    }

    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
	if ( ch->top_level < to_room->area->low_hard_range )
	{
	    set_char_color( AT_TELL, ch );
	    switch( to_room->area->low_hard_range - ch->top_level )
	    {
		case 1:
		  send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'", ch );
		  break;
		case 2:
		  send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
		  break;
		case 3:
		  send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
		  break;
		default:
		  send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
	    }
	    return rNONE;
	}
	else
	if ( ch->top_level > to_room->area->hi_hard_range )
	{
	    set_char_color( AT_TELL, ch );
	    send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'", ch );
	    return rNONE;
	}
    }

    if ( !fall )
    {
        if ( IS_SET( to_room->room_flags, ROOM_INDOORS )
        || IS_SET( to_room->room_flags, ROOM_SPACECRAFT )
        || to_room->sector_type == SECT_INSIDE )
	{
		send_to_char( "You can't drive indoors!\n\r", ch );
		return rNONE;
	}

        if ( IS_SET( to_room->room_flags, ROOM_NO_DRIVING ) )
	{
		send_to_char( "You can't take a vehicle through there!\n\r", ch );
		return rNONE;
	}

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR
	||   IS_SET( pexit->exit_info, EX_FLY ) )
	{
            if ( ship->class > CLOUD_CAR )
	    {
		send_to_char( "You'd need to fly to go there.\n\r", ch );
		return rNONE;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_SWIM
	||   to_room->sector_type == SECT_UNDERWATER
	||   to_room->sector_type == SECT_OCEANFLOOR )
	{

	    if ( ship->class != OCEAN_SHIP )
	    {
		send_to_char( "You'd need a boat to go there.\n\r", ch );
		return rNONE;
	    }

	}

	if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
	{

	    if ( ship->class < CLOUD_CAR )
	    {
		send_to_char( "You need to fly or climb to get up there.\n\r", ch );
		return rNONE;
	    }
	}

    }

    if ( to_room->tunnel > 0 )
    {
	CHAR_DATA *ctmp;
	int count = 0;

	for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
	  if ( ++count >= to_room->tunnel )
	  {
		  send_to_char( "There is no room for you in there.\n\r", ch );
		return rNONE;
	  }
    }

      if ( fall )
        txt = "falls";
      else
      if ( !txt )
      {
	  if (  ship->class < OCEAN_SHIP )
	      txt = "fly";
	  else
	  if ( ship->class == OCEAN_SHIP  )
	  {
	      txt = "float";
	  }
	  else
	  if ( ship->class > OCEAN_SHIP  )
	  {
	      txt = "drive";
	  }
      }
      sprintf( buf, "$n %ss the vehicle $T.", txt );
      act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
      sprintf( buf, "You %s the vehicle $T.", txt );
      act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_CHAR );
      sprintf( buf, "%s %ss %s.", ship->name, txt, dir_name[door] );
      echo_to_room( AT_ACTION , get_room_index(ship->location) , buf );

      extract_ship( ship );
      ship_to_room(ship, to_room->vnum );

      ship->location = to_room->vnum;
      ship->lastdoc = ship->location;

      if ( fall )
        txt = "falls";
      else
	  if (  ship->class < OCEAN_SHIP )
	      txt = "flys in";
	  else
	  if ( ship->class == OCEAN_SHIP  )
	  {
	      txt = "floats in";
	  }
	  else
	  if ( ship->class > OCEAN_SHIP  )
	  {
	      txt = "drives in";
	  }

      switch( door )
      {
      default: dtxt = "somewhere";	break;
      case 0:  dtxt = "the south";	break;
      case 1:  dtxt = "the west";	break;
      case 2:  dtxt = "the north";	break;
      case 3:  dtxt = "the east";	break;
      case 4:  dtxt = "below";		break;
      case 5:  dtxt = "above";		break;
      case 6:  dtxt = "the south-west";	break;
      case 7:  dtxt = "the south-east";	break;
      case 8:  dtxt = "the north-west";	break;
      case 9:  dtxt = "the north-east";	break;
      }

    sprintf( buf, "%s %s from %s.", ship->name, txt, dtxt );
    echo_to_room( AT_ACTION , get_room_index(ship->location) , buf );

    for ( rch = ch->in_room->last_person ; rch ; rch = next_rch )
    {
        next_rch = rch->prev_in_room;
        original = rch->in_room;
        char_from_room( rch );
        char_to_room( rch, to_room );
        do_look( rch, "auto" );
        char_from_room( rch );
        char_to_room( rch, original );
    }

    return retcode;

}

void do_bomb( CHAR_DATA *ch, char *argument )
{}

void do_chaff( CHAR_DATA *ch, char *argument )
{
    int chance;
    SHIP_DATA *ship;


        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
        {
            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
            return;
        }

        if ( ship->class > SHIP_PLATFORM )
    	        {
    	            send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	            return;
    	        }


        if (  (ship = ship_from_coseat(ch->in_room->vnum))  == NULL )
        {
            send_to_char("&RThe controls are at the copilots seat!\n\r",ch);
            return;
        }

                if ( autofly(ship) )
    	        {
    	            send_to_char("&RYou'll have to turn the autopilot off first...\n\r",ch);
    	            return;
    	        }

                if (ship->shipstate == SHIP_HYPERSPACE)
                {
                  send_to_char("&RYou can only do that in realspace!\n\r",ch);
                  return;
                }
    	        if (ship->shipstate == SHIP_DOCKED)
    	        {
    	            send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	            return;
    	        }
                if (ship->chaff <= 0 )
    	        {
    	            send_to_char("&RYou don't have any chaff to release!\n\r",ch);
    	            return;
    	        }
                chance = IS_NPC(ch) ? ch->top_level
                 : (int)  (ch->pcdata->learned[gsn_weaponsystems]) ;
        if ( number_percent( ) > chance )
        {
            send_to_char("&RYou can't figure out which switch it is.\n\r",ch);
            learn_from_failure( ch, gsn_weaponsystems );
    	   return;
        }

    ship->chaff--;

    ship->chaff_released++;

    send_to_char( "You flip the chaff release switch.\n\r", ch);
    act( AT_PLAIN, "$n flips a switch on the control pannel", ch,
         NULL, argument , TO_ROOM );
    echo_to_cockpit( AT_YELLOW , ship , "A burst of chaff is released from the ship.");

    learn_from_success( ch, gsn_weaponsystems );

}

bool autofly( SHIP_DATA *ship )
{

     if (!ship)
        return FALSE;

     if ( ship->type == MOB_SHIP )
        return TRUE;

     if ( ship->mono_disabled )
		ship->autopilot = FALSE;

     if ( ship->autopilot )
        return TRUE;

     return FALSE;

}

sh_int get_ship_speed ( SHIP_DATA *ship )
{
	return ship->mono_disabled ? ship->currspeed / 10 : ship->currspeed;
}



void do_fueltransfer( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance, amount;
    SHIP_DATA *ship, *target;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:
    	        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }
                if (ship->shipstate == SHIP_HYPERSPACE)
                {
                  send_to_char("&RYou can only do that in realspace!\n\r",ch);
                  return;
                }
                if (ship->shipstate == SHIP_DISABLED)
    	        {
    	            send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate == SHIP_DOCKED)
    	        {
    	            send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate != SHIP_READY)
    	        {
    	            send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
    	            return;
    	        }
    	        if ((target = get_ship_here(arg, ship->starsystem)) == NULL)
    	        {
					send_to_char("&RNo such ship here.\n\r",ch);
					return;
				}

				if (  (target->vx > ship->vx + 10) || (target->vx < ship->vx - 10) ||
				      (target->vy > ship->vy + 10) || (target->vy < ship->vy - 10) ||
		    	      (target->vz > ship->vz + 10) || (target->vz < ship->vz - 10) )
		    		{
		    		send_to_char("&RThat ship is too far away! You'll have to fly a little closer.\n\r",ch);
					return;
				}

                if ( ship->energy < 1 )
    	        {
    	           send_to_char("&RTheres no fuel to transfer, bye!\n\r",ch);
    	           return;
    	        }

                if ( ship->class == FIGHTER_SHIP )
                    chance = IS_NPC(ch) ? ch->top_level
	                 : (int)  (ch->pcdata->learned[gsn_starfighters]) / ((ship->mono_disabled) ? 2 : 1);
                if ( ship->class == MIDSIZE_SHIP )
                    chance = IS_NPC(ch) ? ch->top_level
	                 : (int)  (ch->pcdata->learned[gsn_midships]) / ((ship->mono_disabled) ? 2 : 1);
                if ( ship->class == CAPITAL_SHIP )
                    chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_capitalships]) / ((ship->mono_disabled) ? 2 : 1);

	            chance -= 30;
                if ( number_percent( ) < chance )
    			{
				   sprintf(buf,"&GYou successfully initiate a fuel transfer to %s\n\r",target->name);
    			   send_to_char( buf, ch);
    			   act( AT_PLAIN, "$n initiates a fuel transfer.", ch,
		    	    NULL, argument , TO_ROOM );
		 		   //echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");
    		       add_timer ( ch , TIMER_DO_FUN , 5 , do_fueltransfer , 1 );
    			   ch->dest_buf = str_dup(arg);
    			   return;
	        	}
	        send_to_char("&RYou fail to work the controls properly.\n\r",ch);
	        if ( ship->class == FIGHTER_SHIP )
                    learn_from_failure( ch, gsn_starfighters );
                if ( ship->class == MIDSIZE_SHIP )
    	            learn_from_failure( ch, gsn_midships );
                if ( ship->class == CAPITAL_SHIP )
                    learn_from_failure( ch, gsn_capitalships );
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
    		if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    		      return;
    	        send_to_char("&You are interupted and fail the fuel transfer.\n\r", ch);
    	        sprintf(buf, "%s gets distracted and fails to transfer any fuel.", ch->name);
    	        echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , buf);
    		if (ship->shipstate != SHIP_DISABLED)
    		   ship->shipstate = SHIP_READY;
    		return;
    }

    ch->substate = SUB_NONE;

    if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    {
       return;
    }

    if ((target = get_ship_here(arg, ship->starsystem)) == NULL)
	{
		send_to_char("&RThe target ship must have left.\n\r",ch);
		return;
	}

	if (  (target->vx > ship->vx + 10) || (target->vx < ship->vx - 10) ||
	      (target->vy > ship->vy + 10) || (target->vy < ship->vy - 10) ||
  	      (target->vz > ship->vz + 10) || (target->vz < ship->vz - 10) )
  		{
   		send_to_char("&RThe ships flew too far apart! You'll have to try again.\n\r",ch);
		return;
	}

	if ( ship->class == FIGHTER_SHIP )
        amount = IS_NPC(ch) ? ch->top_level
        : (int)  (ch->pcdata->learned[gsn_starfighters]) / ((ship->mono_disabled) ? 2 : 1);
    if ( ship->class == MIDSIZE_SHIP )
        amount = IS_NPC(ch) ? ch->top_level
        : (int)  (ch->pcdata->learned[gsn_midships]) / ((ship->mono_disabled) ? 2 : 1);
    if ( ship->class == CAPITAL_SHIP )
        amount = IS_NPC(ch) ? ch->top_level
        : (int) (ch->pcdata->learned[gsn_capitalships]) / ((ship->mono_disabled) ? 2 : 1);

    amount *= 2;

	if (ship->energy - 100 < amount) amount = ship->energy - 100;
	if (target->maxenergy - target->energy < amount) amount = target->maxenergy - target->energy;

	ship->energy -= amount;
	target->energy += amount;

	sprintf(buf, "&GYou transfered %d fuel to %s.\n\r", amount, target->name);
    send_to_char( buf, ch);
    act( AT_PLAIN, "$n finishes a fuel transfer.", ch,
         NULL, argument , TO_ROOM );
    //echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");


    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}

ROOM_INDEX_DATA * find_dock ( char * dockname )
{
	SPACE_DATA *starsystem;
	ROOM_INDEX_DATA *dock;

	for ( starsystem = first_starsystem ; starsystem ; starsystem = starsystem->next )
	{
		dock = get_room_index(starsystem->doc1a);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc2a);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc3a);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc4a);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc1b);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc2b);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc3b);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc4b);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc1c);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc2c);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc3c);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
		dock = get_room_index(starsystem->doc4c);
		if (dock && nifty_is_name_prefix( dockname, dock->name ) ) return dock;
	}
	return NULL;
}


/* Generic Pilot Command To use as template

void do_hmm( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    SHIP_DATA *ship;

    strcpy( arg, argument );

    switch( ch->substate )
    {
    	default:
    	        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }
                if (ship->shipstate == SHIP_HYPERSPACE)
                {
                  send_to_char("&RYou can only do that in realspace!\n\r",ch);
                  return;
                }
                if (ship->shipstate == SHIP_DISABLED)
    	        {
    	            send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate == SHIP_DOCKED)
    	        {
    	            send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate != SHIP_READY)
    	        {
    	            send_to_char("&RPlease wait until the ship has finished its current manouver.\n\r",ch);
    	            return;
    	        }

                if ( ship->energy <1 )
    	        {
    	           send_to_char("&RTheres not enough fuel!\n\r",ch);
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
    		   send_to_char( "&G\n\r", ch);
    		   act( AT_PLAIN, "$n does  ...", ch,
		        NULL, argument , TO_ROOM );
		   echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");
    		   add_timer ( ch , TIMER_DO_FUN , 1 , do_hmm , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou fail to work the controls properly.\n\r",ch);
	        if ( ship->class == FIGHTER_SHIP )
                    learn_from_failure( ch, gsn_starfighters );
                if ( ship->class == MIDSIZE_SHIP )
    	            learn_from_failure( ch, gsn_midships );
                if ( ship->class == CAPITAL_SHIP )
                    learn_from_failure( ch, gsn_capitalships );
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
    		if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    		      return;
    	        send_to_char("&Raborted.\n\r", ch);
    	        echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");
    		if (ship->shipstate != SHIP_DISABLED)
    		   ship->shipstate = SHIP_READY;
    		return;
    }

    ch->substate = SUB_NONE;

    if ( (ship = ship_from_cockpit(ch->in_room->vnum)) == NULL )
    {
       return;
    }

    send_to_char( "&G\n\r", ch);
    act( AT_PLAIN, "$n does  ...", ch,
         NULL, argument , TO_ROOM );
    echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");


    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}

void do_hmm( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance;
    SHIP_DATA *ship;


        if (  (ship = ship_from_cockpit(ch->in_room->vnum))  == NULL )
        {
            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
            return;
        }

                if (ship->shipstate == SHIP_HYPERSPACE)
                {
                  send_to_char("&RYou can only do that in realspace!\n\r",ch);
                  return;
                }
                if (ship->shipstate == SHIP_DISABLED)
    	        {
    	            send_to_char("&RThe ships drive is disabled. Unable to manuever.\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate == SHIP_DOCKED)
    	        {
    	            send_to_char("&RYou can't do that until after you've launched!\n\r",ch);
    	            return;
    	        }
    	        if (ship->shipstate != SHIP_READY)
    	        {
    	            send_to_char("&RPlease wait until the ship has finished its current manuever.\n\r",ch);
    	            return;
    	        }

        if ( ship->energy <1 )
        {
              send_to_char("&RTheres not enough fuel!\n\r",ch);
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

    send_to_char( "&G\n\r", ch);
    act( AT_PLAIN, "$n does  ...", ch,
         NULL, argument , TO_ROOM );
    echo_to_room( AT_YELLOW , get_room_index(ship->cockpit) , "");



    if ( ship->class == FIGHTER_SHIP )
        learn_from_success( ch, gsn_starfighters );
    if ( ship->class == MIDSIZE_SHIP )
        learn_from_success( ch, gsn_midships );
    if ( ship->class == CAPITAL_SHIP )
        learn_from_success( ch, gsn_capitalships );

}

*/
