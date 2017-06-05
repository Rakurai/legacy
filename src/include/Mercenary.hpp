#pragma once

class Mercenary
{
public:
    class Offer
    {
    public:
        Offer() {}
        virtual ~Offer() {}

        String  name;
        int     amount = 0;
        bool        valid = FALSE;
        Mercenary::Offer *    next = NULL;

    private:
        Offer(const Offer&);
        Offer& operator=(const Offer&);
    };

    Mercenary() {}
    virtual ~Mercenary() {}

    String  name;
    int     minimum = 0;

    Offer *    offer = NULL;
    sh_int      num_offers = 0;
    String  employer;

    bool        valid = FALSE;
    Mercenary * previous = NULL;
    Mercenary * next = NULL;

private:
    Mercenary(const Mercenary&);
    Mercenary& operator=(const Mercenary&);
};
