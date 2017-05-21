INCDIR  = ./include
OBJDIR  = obj
BINDIR  = ../bin
DEPDIR  = deps
EXEFILE = legacy

CC      = g++
#STD     = -std=gnu99
STD     = -std=c++11
WARNING = -Wall -Winline -Wno-write-strings
DEBUG   = -ggdb
DEFINE  = -Dunix -DSQL -DDEBUG -DIPV6
INCLUDE = -I$(INCDIR) -I/usr/include -I/usr/local/include
C_FLAGS = ${STD} ${WARNING} ${DEBUG} ${DEFINE} ${INCLUDE}
L_FLAGS = -lsqlite3 -lm

DATE=`date +%Y-%m-%d`

_H_DEPS = cJSON/cJSON.h # cprops/splay.h
H_DEPS  = $(patsubst %,$(DEPDIR)/%,$(_H_DEPS))
_O_DEPS = cJSON/cJSON.o # cprops/splay.o cprops/mempool.o cprops/rb.o cprops/vector.o
O_DEPS  = $(patsubst %,$(DEPDIR)/%,$(_O_DEPS))

_H_FILES = db.h interp.h lookup.h magic.h memory.h merc.h music.h sql.h recycle.h tables.h	\
	telnet.h vt100.h affect.h affect_list.h
H_FILES = $(patsubst %,$(INCDIR)/%,$(_H_FILES))

_O_FILES = act_comm.o act_info.o act_move.o act_obj.o \
	affect.o affect_char.o affect_obj.o affect_room.o affect_list.o affect_cache_array.o attribute.o \
	alias.o area_handler.o bank.o channels.o clan-edit.o comm.o config.o const.o db.o \
	debug.o departed.o dispel.o duel.o edit.o effects.o	\
	fight.o file.o find.o flags.o handler.o help.o hunt.o ignore.o interp.o load_config.o	\
	lookup.o magic.o marry.o memory.o mob_commands.o mob_prog.o mobiles.o music.o sqlite.o	\
	nanny.o note.o objstate.o quest.o random.o recycle.o remort.o rmagic.o save.o scan.o	\
	set-stat.o skills.o social.o special.o storage.o c_string.o tables.o update.o war.o	\
	wiz_admin.o wiz_build.o wiz_coder.o wiz_gen.o wiz_quest.o wiz_secure.o gem.o paint.o \
	String.o

O_FILES = $(patsubst %,$(OBJDIR)/%,$(_O_FILES))

$(OBJDIR)/%.o: %.c $(H_DEPS) $(H_FILES)
	$(CC) -c -o $@ $< $(C_FLAGS)

$(OBJDIR)/%.o: %.cpp $(H_DEPS) $(H_FILES)
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
