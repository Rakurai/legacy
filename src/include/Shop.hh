#pragma once

#include "declare.hh"

#define MAX_TRADE        5

class Shop
{
public:
    Shop() {}
    virtual ~Shop() {}

    Shop *      next = nullptr;                   /* Next shop in list            */
    int      keeper = 0;                 /* Vnum of shop keeper mob      */
    int      buy_type [MAX_TRADE] = {0};   /* Item types shop will buy     */
    int      profit_buy = 0;             /* Cost multiplier for buying   */
    int      profit_sell = 0;            /* Cost multiplier for selling  */
    int      open_hour = 0;              /* First opening hour           */
    int      close_hour = 0;             /* First closing hour           */

private:
	Shop(const Shop&);
	Shop& operator=(const Shop&);
};

extern          Shop         *     shop_first; // db.c
