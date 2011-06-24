#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

void do_shiphail( CHAR_DATA *ch, char *argument )
{
   
   SHIP_DATA *ship;
   SHIP_DATA *ship2 = ship_from_cockpit( ch->in_room->vnum );
   int  count;
   int top_num=15;
   int vnum[top_num];
   DESCRIPTOR_DATA *i;
   CHAR_DATA *och;
   char buf[MAX_STRING_LENGTH];
   char buf1[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   strcpy(arg2,argument);
   if(IS_NPC(ch))
	return;
   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
   	send_to_char ("Syntax: Hail <ship> <message>\n\r", ch);
   	return;
   }
   
   ship = get_ship( arg1 );
   send_to_char ("\n\r", ch);
   if (!ship)
   {
     send_to_char ("No Such Ship!\n\r", ch);
     return;
   }
   if (!ship2)
   {
   	send_to_char("But your not in a cockpit!\n\r", ch);
   	return;
   }
   if ( !IS_NPC(ch) && ( IS_SET (ch->act, PLR_SILENCE ) ) )
   {
	send_to_char("You can't do this while silenced!\n\r", ch);
	return;
   }
    
   sprintf (buf, "&B(&YShip Hail&B) &G&W%s: %s\n\r", ship2->name, arg2);
   sprintf (buf1, "&g%s adjusts some settings on the comm system and says something", ch->name, arg2);
   act( AT_GREEN, buf1,  ch, NULL, NULL, TO_NOTVICT );
   sprintf (buf1, "&B(&YShip Hail&B) &G&WYou: %s\n\r",  arg2);
   act(AT_PLAIN, buf1, ch, NULL, NULL, TO_CHAR);
   act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
/*   vnum[0] = ship->cockpit;
   vnum[1] = ship->pilotseat;
   vnum[2] = ship->gunseat;
   vnum[3] = ship->coseat;
   vnum[4] = ship->navseat;
   vnum[5] = ship->turret1;
   vnum[6] = ship->turret2;

 for (i = first_descriptor; i; i = i->next)
  {
    if (!i->connected && i->character)
    {
      och = i->original ? i->original : i->character;	
    }
   for( count=1; count <= top_num; count = count + 1 )
   {
     if (och->in_room->vnum == vnum[count] && och->desc)
     {
     	send_to_char(buf, och);
     	break;
     }
   }
  }
*/
 int highroom = 0, lowroom = 0;
 
 if(ship->firstroom <= 0 || ship->lastroom <= 0)
 {
	send_to_char("Contact Immoortal Staff, Hail target ship has vnum range problem.\r\n", ch);
	return;
 }
 highroom = ship->firstroom;
 lowroom = ship->lastroom;
 for(highroom; highroom <= lowroom; highroom++)
 {
	echo_to_room(AT_PLAIN, get_room_index(highroom), buf);
 }

}
  
