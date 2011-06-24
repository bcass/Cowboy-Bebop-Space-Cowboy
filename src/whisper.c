#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

void do_whisper( CHAR_DATA *ch, char *argument ) 
{
   CHAR_DATA *victim; 
   char arg1[MAX_STRING_LENGTH]; 
   char arg2[MAX_INPUT_LENGTH]; 
   char buf[MAX_INPUT_LENGTH]; 
   argument = one_argument(argument, arg1); 
   strcpy(arg2,argument);
   if (( victim = get_char_room(ch, arg1)) == NULL) 
   {
    send_to_char("They are not in here.\n\r", ch); 
    return;
   } 
    sprintf(buf, "&B[&Cwhisper&B] &Cto %s: &W%s\n\r", victim->name, arg2);
    send_to_char(buf, ch); 
    sprintf(buf, "&B[&Cwhisper&B] &C%s whispers: &W%s\n\r", ch->name, arg2); 
    send_to_char(buf, victim); 
    act(AT_SOCIAL, "$n leans close to $N and whispers something", ch, NULL, victim, TO_NOTVICT);
}
   
