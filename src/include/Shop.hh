#pragma once

#include "constants.hh"
#include <cstdio>

#define MAX_TRADE        5

class Shop
{
public:
    Shop(FILE *fp);
    virtual ~Shop() {}

    bool is_questshop() const { return buy_type[0] == ITEM_QUESTSHOP; }

    int      buy_type [MAX_TRADE] = {0};   /* Item types shop will buy     */
    int      profit_buy = 0;             /* Cost multiplier for buying   */
    int      profit_sell = 0;            /* Cost multiplier for selling  */
    int      open_hour = 0;              /* First opening hour           */
    int      close_hour = 0;             /* First closing hour           */

private:
	Shop(const Shop&);
	Shop& operator=(const Shop&);
};
