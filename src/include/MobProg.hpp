#pragma once

class MobProg
{
public:
	MobProg() {}
	virtual ~MobProg() {}

    MobProg *next = NULL;
    int         type = 0;
    String      arglist;
    String      comlist;

private:
	MobProg(const MobProg&);
	MobProg& operator=(const MobProg&);
};

extern bool    MOBtrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
#define BUY_PROG        2048
#define TICK_PROG       4096
#define BOOT_PROG	8192
