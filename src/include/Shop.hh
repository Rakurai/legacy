#pragma once

#include "declare.hh"

#define MAX_TRADE        5

class Shop
{
public:
    Shop() {}
    virtual ~Shop() {}

    Shop *      next = nullptr;                   /* Next shop in list            */
    sh_int	    version = 0;		/* Shop versioning -- Montrey */
    sh_int      keeper = 0;                 /* Vnum of shop keeper mob      */
    sh_int      buy_type [MAX_TRADE] = {0};   /* Item types shop will buy     */
    sh_int      profit_buy = 0;             /* Cost multiplier for buying   */
    sh_int      profit_sell = 0;            /* Cost multiplier for selling  */
    sh_int      open_hour = 0;              /* First opening hour           */
    sh_int      close_hour = 0;             /* First closing hour           */

private:
	Shop(const Shop&);
	Shop& operator=(const Shop&);
};

extern          Shop         *     shop_first; // db.c
