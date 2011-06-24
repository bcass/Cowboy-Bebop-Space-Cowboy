CC      = gcc
PROF    = 

#Uncomment to compile in Cygwin
#CYGWIN = -DCYGWIN

NOCRYPT =
#Uncomment the next line if you want request support
#DBUGFLG = -DREQUESTS
C_FLAGS = -I/usr/lib/gcc-lib/i586-pc-linux-gnu/2.95.3/include -ggdb3
L_FLAGS = $(PROF)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	  build.o clans.o comm.o comments.o const.o db.o fight.o \
	  handler.o hashstr.o id.o interp.o magic.o makeobjs.o \
	  misc.o mud_comm.o mud_prog.o player.o requests.o \
	  reset.o save.o shops.o skills.o special.o tables.o track.o update.o \
	  space.o bounty.o swskills.o quest.o whisper.o slay.o restore.o \
	  finger.o hail.o msweapons.o board.o shipbuild.o trading.o

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c \
	  build.c clans.c comm.c comments.c const.c db.c fight.c \
	  handler.c hashstr.c id.c interp.c magic.c makeobjs.c \
	  misc.c mud_comm.c mud_prog.c player.c requests.c \
	  reset.c save.c shops.c skills.c special.c tables.c track.c update.c \
	  space.c bounty.c swskills.c quest.c whisper.c slay.c restore.c \
	  finger.c hail.c msweapons.c board.c shipbuild.c trading.c

H_FILES = mud.h bet.h slay.h restore.h finger.h board.h


all:
#	co $(H_FILES)
	make swreality
#	rm -f $(H_FILES)

ifdef CYGWIN
swreality: $(O_FILES)
	rm -f swreality.exe
	$(CC) $(L_FLAGS) -o swreality.exe $(O_FILES) -lm -lcrypt
	chmod g+w swreality.exe
	chmod g+w $(O_FILES)

else
swreality: $(O_FILES)
	rm -f swreality
	$(CC) $(L_FLAGS) -o swreality $(O_FILES) -lm  -lcrypt
	chmod g+w swreality
	chmod g+w $(O_FILES)

endif
.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f $(O_FILES)
