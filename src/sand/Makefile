CC      = gcc
WARNING = -Wall -Winline
DEBUG   = -ggdb
DEFINE  = -Dlinux -DSQL -DDEBUG
INCLUDE = -I/usr/include 
C_FLAGS = -O ${WARNING} ${DEBUG} ${DEFINE} ${INCLUDE}
L_FLAGS = 

DATE=`date +%Y-%m-%d`
BINDIR= ../../bin

O_FILES = sand_server.o

all: $(O_FILES)
	$(CC) $(L_FLAGS) -o sand_server $(O_FILES)

.c.o:
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -rf *.o *~

cleaner: clean
	rm -f sand_server

install:
	cp sand_server ${BINDIR}
