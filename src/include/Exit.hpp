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
    sh_int              exit_info = 0;
    sh_int              key = 0;
    String              keyword;
    String              description;

private:
	Exit(const Exit&);
	Exit& operator=(const Exit&);
};
