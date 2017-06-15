#pragma once

/*
 * Structure for a social in the socials table.
 */
class Social
{
public:
    Social() {}
    virtual ~Social() {}

    String    name;
    String    char_no_arg;
    String    others_no_arg;
    String    char_found;
    String    others_found;
    String    vict_found;
/*
    String    char_not_found;
*/
    String      char_auto;
    String      others_auto;
    Social * previous = nullptr;
    Social * next = nullptr;

private:
    Social(const Social&);
    Social& operator=(const Social&);
};
