
class Tail
{
public:
    Tail() {}
    virtual ~Tail() {}

    bool        valid = FALSE;
    Character * tailed_by = nullptr;
    String      tailer_name;
    int         flags = 0;
    Tail *      next = nullptr;

private:
	Tail(const Tail&);
	Tail& operator=(const Tail&);
};

/*
 * Tail (Snoop) flags
 */
#define TAIL_NONE                        0
#define TAIL_ACT                         BIT_A
#define TAIL_ALL                         TAIL_ACT
