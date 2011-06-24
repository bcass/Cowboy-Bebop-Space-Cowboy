/***************************************************************************
*                           STAR WARS UNKNOWN REGIONS                      *
*--------------------------------------------------------------------------*
* Star Wars Unknown Regions Code Addtions and changes from the SWR Code    *
* copyright (c) 2000 by Gavin Mogan                                        *
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
*                       Restores V1.0 header information			       *
****************************************************************************/

/* Capability to create, edit and delete restore messages added to original code
   by Gavin(ur_gavin@hotmail.com 5-26-2000 */
#define RESTORE_ADDON

typedef struct	restore_data	RESTORE_DATA;

extern	RESTORE_DATA	*	first_restore;
extern	RESTORE_DATA	*	last_restore;

/* Improved data structure for online restore editing - Gavin 5-26-2000 */
struct restore_data
{
   RESTORE_DATA *	next;
   RESTORE_DATA *	prev;
   char *		owner;
   char *		type;
   char *		cmsg;
   char *		vmsg;
   char *		rmsg;
   int 			color;
   float	 		boost;
};

#define RESTORE_FILE		"restore.dat" /* Restore data file for online editing - Gavin 5-26-2000 */
#define RESTORE_INTERVAL 21600

/* Maxsrestoretypes variable - 50 should be WAY more than enough */
#define MAX_RESTORE_TYPES		50

DECLARE_DO_FUN( do_destroyrestore ); /* New destroyrestore command - Gavin 5-26-2000 */
DECLARE_DO_FUN( do_makerestore	);  /* New makerestore command - Gavin 5-26-2000 */
DECLARE_DO_FUN( do_setrestore	);  /* New setrestore command - Gavin 5-26-2000 */
DECLARE_DO_FUN( do_showrestore	);  /* New showrestore command - Gavin 5-26-2000 */

void load_restores		args( ( void ) );
