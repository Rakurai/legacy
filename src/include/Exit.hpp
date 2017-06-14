/*
 * Exit data.
 */
class Exit
{
public:
	Exit() {}
	virtual ~Exit() {}

    union
    {
        RoomPrototype *       to_room;
        sh_int                  vnum;
    } u1 = {0};
    Flags               exit_flags;
    sh_int              key = 0;
    String              keyword;
    String              description;

    static const String& dir_name(unsigned short dir, bool reverse = false);
    static unsigned short rev_dir(unsigned short dir);

private:
	Exit(const Exit&);
	Exit& operator=(const Exit&);
};

inline unsigned short Exit::
rev_dir(unsigned short dir) {
    static const short rdir[] = {
        2, 3, 0, 1, 5, 4
    };

    return rdir[dir % 6];
}

inline const String& Exit::
dir_name(unsigned short dir, bool reverse) {
    static const String dir_name[] = {
        "north", "east", "south", "west", "up", "down"
    };

    dir = dir % 6;

    if (reverse)
        dir = rev_dir(dir);

    return dir_name[dir % 6];
}
