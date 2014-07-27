CC      = gcc
WARNING = -Wall -Winline
DEBUG   = -ggdb
DEFINE  = -Dlinux -DSQL -DDEBUG
#INCLUDE = -I/usr/include -I/usr/include/mysql
INCLUDE = -I/usr/include $(shell mysql_config --cflags)
C_FLAGS = -O ${WARNING} ${DEBUG} ${DEFINE} ${INCLUDE}
#L_FLAGS = -lmysqlclient -lcrypt
#L_FLAGS = -L/usr/lib/mysql -lmysqlclient -lcrypt
L_FLAGS = $(shell mysql_config --libs) -lcrypt

DATE=`date +%Y-%m-%d`
BINDIR=../bin

O_FILES =	act_comm.o act_info.o act_move.o act_obj.o help.o memory.o file.o		\
		phone.o chit.o wiz_build.o wiz_coder.o wiz_gen.o wiz_quest.o wiz_secure.o	\
		alias.o comm.o const.o db.o edit.o effects.o fight.o flags.o handler.o		\
		mobiles.o interp.o note.o lookup.o magic.o rmagic.o music.o recycle.o		\
		save.o scan.o skills.o special.o ignore.o social-edit.o tables.o update.o	\
		bank.o hunt.o string.o mob_commands.o mob_prog.o quest.o marry.o sand.o		\
		tour.o clan-edit.o storage.o departed.o remort.o set-stat.o config.o war.o	\
		duel.o channels.o debug.o find.o objstate.o nanny.o mysql.o random.o		\
		area_handler.o newline.o

legacy: $(O_FILES)
	$(CC) -o legacy $(O_FILES) $(L_FLAGS)

.c.o:
	$(CC) -c $(C_FLAGS) $<

show:
	@echo "C_FLAGS are $(C_FLAGS)"
	@echo "L_FLAGS are $(L_FLAGS)"

clean:
	rm -rf *.o *~

cleaner: clean
	rm -f legacy
	cd sand && make cleaner

install:
	mv ${BINDIR}/legacy ${BINDIR}/legacy.bak
	cp legacy ${BINDIR}

