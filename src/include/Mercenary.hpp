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
        Mercenary::Offer *    next = nullptr;

    private:
        Offer(const Offer&);
        Offer& operator=(const Offer&);
    };

    Mercenary() {}
    virtual ~Mercenary() {}

    String  name;
    int     minimum = 0;

    Offer *    offer = nullptr;
    sh_int      num_offers = 0;
    String  employer;

    bool        valid = FALSE;
    Mercenary * previous = nullptr;
    Mercenary * next = nullptr;

private:
    Mercenary(const Mercenary&);
    Mercenary& operator=(const Mercenary&);
};
