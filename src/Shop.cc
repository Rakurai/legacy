#include "Shop.hh"
#include "file.hh"

Shop::
Shop(FILE *fp) {
	for (int i = 0; i < MAX_TRADE; i++)
		buy_type[i] = fread_number(fp);

	profit_buy       = fread_number(fp);
	profit_sell      = fread_number(fp);
	open_hour        = fread_number(fp);
	close_hour       = fread_number(fp);
	fread_to_eol(fp);
}
