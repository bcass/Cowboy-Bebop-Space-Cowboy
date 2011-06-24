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
 *                       Slay V2.0 header information			          *
 ****************************************************************************/

/* Capability to create, edit and delete slaytypes added to original code
   by Samson 8-3-98 */

#ifndef FCLOSE
   #define FCLOSE(fp)  fclose(fp); fp=NULL;
#endif

typedef struct	slay_data	SLAY_DATA;

extern	SLAY_DATA	*	first_slay;
extern	SLAY_DATA	*	last_slay;

/* Improved data structure for online slay editing - Samson 8-3-98 */
struct slay_data
{
   SLAY_DATA *	next;
   SLAY_DATA *	prev;
   char *		owner;
   char *		type;
   int 		color;
   char *		cmsg;
   char *		vmsg;
   char *		rmsg;
};

#define SLAY_FILE		"slay.dat" /* Slay data file for online editing - Samson 8-3-98 */

/* Maxslaytypes variable - 50 should be WAY more than enough :P */
#define MAX_SLAY_TYPES		50

DECLARE_DO_FUN( do_destroyslay ); /* New destroyslay command - Samson 8-3-98 */
DECLARE_DO_FUN( do_makeslay	);  /* New makeslay command - Samson 8-3-98 */
DECLARE_DO_FUN( do_setslay	);  /* New setslay command - Samson 8-3-98 */
DECLARE_DO_FUN( do_showslay	);  /* New showslay command - Samson 8-3-98 */

void load_slays		args( ( void ) );

