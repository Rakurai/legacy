INCDIR  = ./include
OBJDIR  = obj
BINDIR  = ../bin
DEPDIR  = deps
EXEFILE = legacy

CC      = gcc
WARNING = -Wall -Winline
DEBUG   = -ggdb
DEFINE  = -Dlinux -Dunix -DSQL -DDEBUG -DIPV6
INCLUDE = -I/usr/include $(shell mysql_config --cflags) -I$(INCDIR)
C_FLAGS = -O ${WARNING} ${DEBUG} ${DEFINE} ${INCLUDE}
L_FLAGS = $(shell mysql_config --libs)

DATE=`date +%Y-%m-%d`

_H_DEPS = cJSON/cJSON.h
H_DEPS  = $(patsubst %,$(DEPDIR)/%,$(_H_DEPS))
_O_DEPS = cJSON/cJSON.o
O_DEPS  = $(patsubst %,$(DEPDIR)/%,$(_O_DEPS))

_H_FILES = ansi_compat.h bet.h chit.h db.h departed.h interp.h lookup.h  \
        magic.h malloc.h memory.h merc.h music.h mysql.h newline.h phone.h ports.h  \
        recycle.h sand.h tables.h telnet.h vt100.h
H_FILES = $(patsubst %,$(INCDIR)/%,$(_H_FILES))

_O_FILES = act_comm.o act_info.o act_move.o act_obj.o help.o memory.o    \
        phone.o chit.o wiz_build.o wiz_coder.o wiz_gen.o wiz_quest.o wiz_secure.o	\
		alias.o comm.o const.o db.o edit.o effects.o fight.o flags.o handler.o		\
		mobiles.o interp.o note.o lookup.o magic.o rmagic.o music.o recycle.o		\
		save.o scan.o skills.o special.o ignore.o social.o tables.o update.o		\
		bank.o hunt.o string.o mob_commands.o mob_prog.o quest.o marry.o sand.o		\
		tour.o clan-edit.o storage.o departed.o remort.o set-stat.o config.o war.o	\
		duel.o channels.o debug.o find.o objstate.o nanny.o mysql.o random.o		\
		area_handler.o newline.o xsocial.o load_config.o file.o
O_FILES = $(patsubst %,$(OBJDIR)/%,$(_O_FILES))

$(OBJDIR)/%.o: %.c $(H_DEPS) $(H_FILES)
	$(CC) -c -o $@ $< $(C_FLAGS)

legacy: $(O_DEPS) $(O_FILES)
	$(CC) -o $@ $^ $(C_FLAGS) $(L_FLAGS)

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~

cleaner: clean
	rm -f ${EXEFILE}
	cd sand && make cleaner

install:
	mv ${BINDIR}/${EXEFILE} ${BINDIR}/${EXEFILE}.bak
	cp ${EXEFILE} ${BINDIR}
