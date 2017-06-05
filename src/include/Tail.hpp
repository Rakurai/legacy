
class Tail
{
public:
    Tail() {}
    virtual ~Tail() {}

    bool        valid = FALSE;
    Character * tailed_by = NULL;
    String      tailer_name;
    int         flags = 0;
    Tail *      next = NULL;

private:
	Tail(const Tail&);
	Tail& operator=(const Tail&);
};

/*
 * Tail (Snoop) flags
 */
#define TAIL_NONE                        0
#define TAIL_ACT                         A
#define TAIL_ALL                         TAIL_ACT
