#include "Auction.hh"

#include "act.hh"
#include "argument.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Player.hh"
#include "String.hh"
#include "typename.hh"

// global auction
Auction auction;

void Auction::
init() {
	item = nullptr;
	buyer = nullptr;
	seller = nullptr;
}

bool Auction::
is_participant(Object *obj) const {
	return item != nullptr && obj == item;
}

bool Auction::
is_participant(Character *ch) const {
	return item != nullptr && ch != nullptr && ((ch == buyer) || (ch == seller));
}

void Auction::
update() {
	if (item == nullptr) {
		/* no auction in progress -- return doing nothing */
		return;
	}

	if (--pulse <= 0) { /* decrease pulse */
		String buf;
		pulse = PULSE_AUCTION;

		switch (++going) { /* increase the going state */
		case 1 : /* going once */
		case 2 : /* going twice */
			if (bet > 0)
				Format::sprintf(buf, "%s: going %s for %d gold.\n",
				        item->short_descr,
				        ((going == 1) ? "once" : "twice"), bet);
			else
				Format::sprintf(buf, "%s: going %s (no bet received yet).\n",
				        item->short_descr,
				        ((going == 1) ? "once" : "twice"));

			talk_auction(buf);
			break;

		case 3 : /* SOLD! */
			if (bet > 0) {
				Format::sprintf(buf, "AUCTION: %s sold to $n for %d gold.\n",
				        item->short_descr, bet);
				global_act(buyer, buf, true,
				           YELLOW, COMM_NOAUCTION | COMM_QUIET);
				Format::sprintf(buf, "AUCTION: %s sold to $N for %d gold.\n",
				        item->short_descr, bet);
				wiznet(buf, buyer, nullptr, WIZ_AUCTION, 0,
				       GET_RANK(buyer));
				obj_to_char(item, buyer);
				act("The auctioneer appears before you in a puff of smoke and hands you $p.",
				    buyer, item, nullptr, TO_CHAR);
				act("The auctioneer appears before $n, and hands $m $p",
				    buyer, item, nullptr, TO_ROOM);
				/* deduct_cost(seller,-bet); */ /* give him the money */
				seller->gold += bet;
				item = nullptr; /* reset item */
			}
			else { /* not sold */
				Format::sprintf(buf,
				        "No bets received for %s - object has been removed.\n",
				        item->short_descr);
				talk_auction(buf);
				act("The auctioneer appears before you to return $p to you.",
				    seller, item, nullptr, TO_CHAR);
				act("The auctioneer appears before $n to return $p to $m.",
				    seller, item, nullptr, TO_ROOM);
				obj_to_char(item, seller);
				item = nullptr; /* clear auction */
			} /* else */
		} /* switch */
	} /* if */
} /* auction_update() */

/* put an item on auction, or see the stats on the current item or bet */
void do_auction(Character *ch, String argument)
{
	Object *obj;
	char buf[MSL];
	Clan *clan;
	int min = 0;

	/* NPC can be extracted at any time and thus can't auction! */
	if (ch->is_npc()) {
		stc("Mobiles can't auction!\n", ch);
		return;
	}

	if (argument.empty()) {
		channel(ch, argument, CHAN_AUCTION);
		return;
	}

	/* at least one argument given */
	if (ch->revoke_flags.has(REVOKE_AUCTION)) {
		stc("The gods have revoked your ability to auction.\n", ch);
		return;
	}

	if (ch->comm_flags.has(COMM_QUIET)) {
		stc("You must turn off quiet mode first.\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1 == "who") {
		channel_who(ch, "Auction", COMM_NOAUCTION, CSLOT_CHAN_AUCTION);
		return;
	}

	if (arg1 == "status") {
		if (auction.item == nullptr) {
			stc("No item is up for auction.\n", ch);
			return;
		}

		ptc(ch, "Name (%s)   Type (%s)   Wear (%s)\n"
		    "Level (%d)   Cost (%d)   Condition  (%d)\n",
		    auction.item->short_descr,
		    item_type_name(auction.item),
		    wear_bit_name(auction.item->wear_flags),
		    auction.item->level,
		    auction.item->cost,
		    auction.item->condition);

		if (auction.bet > 0)
			ptc(ch, "Current bid on this item is %d gold.\n", auction.bet);
		else
			stc("No bids on this item have been received.\n", ch);

		if (auction.min > 0)
			ptc(ch, "The minimum bid on this item is %d gold.\n", auction.min);
		else
			stc("There is no minimum bid on this item.\n", ch);

		if (IS_IMMORTAL(ch))
			ptc(ch, "Seller: %s\n"
			    "Current Bidder: %s\n",
			    (auction.seller == nullptr) ? "(None)" : auction.seller->name,
			    (auction.buyer == nullptr)  ? "(None)" : auction.buyer->name);

		return;
	}

	if (IS_IMMORTAL(ch) && arg1 == "stop") {
		if (auction.item == nullptr) {
			stc("There is no auction going on you can stop.\n", ch);
			return;
		}

		Format::sprintf(buf, "Sale of %s has been stopped by the Imms. Item confiscated.\n",
		        auction.item->short_descr);
		talk_auction(buf);
		Format::sprintf(buf, "%s has stopped the auction of %s.", ch->name, auction.item->short_descr);
		wiznet(buf, ch, nullptr, WIZ_AUCTION, 0, GET_RANK(ch));
		obj_to_char(auction.item, ch);
		auction.item = nullptr;

		if (auction.buyer != nullptr) { /* return money to the buyer */
			auction.buyer->gold += auction.bet;
			stc("Your money has been returned.\n", auction.buyer);
		}

		return;
	}

	if (arg1 == "bet" || arg1 == "bid") {
		int newbet;

		if (auction.item == nullptr) {
			stc("There isn't anything being auctioned right now.\n", ch);
			return;
		}

		if (auction.seller == ch) {
			stc("You may not bid on your own items.\n", ch);
			return;
		}

		if (auction.buyer == ch) {
			stc("You have already bid on it!\n", ch);
			return;
		}

		if (IS_IMMORTAL(ch)) {
			stc("Immortals may not bid on auctions.\n", ch);
			return;
		}

		/* Took this out cause it seems to have started to malfunction - Lotus
		                newbet = parsebet (auction.bet, argument);
		*/
		if (argument.empty())
			newbet = auction.bet + std::max(auction.bet / 10, 1);
		else {
			if (!argument.is_number()) {
				stc("Your bid must be numeric.\n", ch);
				return;
			}

			newbet = atoi(argument);

			if ((newbet - auction.bet) < std::max(auction.bet / 10, 1)) {
				stc("You must bid at least 10% over the current bid.\n", ch);
				return;
			}
		}

		if ((newbet < auction.min) && (auction.min > 0)) {
			ptc(ch, "The minimum bid for that item is %d gold.\n", auction.min);
			return;
		}

		if (!deduct_cost(ch, newbet * 100)) { /* subtracts the gold */
			stc("You don't have that much money!\n", ch);
			return;
		}

		/* return the gold to the last buyer, if one exists */
		if (auction.buyer != nullptr)
			deduct_cost(auction.buyer, -(auction.bet * 100));

		/* players spam bidding without having to type their bid! :P */
		if (argument.empty())
			WAIT_STATE(ch, PULSE_PER_SECOND);

		auction.buyer = ch;
		auction.bet   = newbet;
		auction.going = 0;
		auction.pulse = PULSE_AUCTION; /* start the auction over again */
		Format::sprintf(buf, "A bid of %d gold has been received on %s.\n",
		        newbet, auction.item->short_descr);
		talk_auction(buf);
		Format::sprintf(buf, "%s has bid %d gold.", ch->name, newbet);
		wiznet(buf, ch, nullptr, WIZ_AUCTION, 0, GET_RANK(ch));
		return;
	} /* end bid */

	if (auction.item != nullptr) {
		act("Try again later - $p is being auctioned right now!", ch, auction.item, nullptr, TO_CHAR);
		return;
	}

	/* auc was not followed by an auc command keyword, so we auc an object */
	/* Added minimum bid - Lotus */
	String arg2;
	argument = one_argument(argument, arg2);

	/* changed from obj_list to obj_carry so as not to auc worn EQ -- Elrac */
	if ((obj = get_obj_carry(ch, arg1)) == nullptr) { /* does char have the item ? */
		stc("You aren't carrying that.\n", ch);
		return;
	}

	/* can't auction food made with create food */
	if (obj->pIndexData->vnum == GEN_OBJ_FOOD) {
		stc("It looks too delicious to just auction away. :)\n", ch);
		return;
	}

	/* Cannot auction any clan items */
	clan = clan_table_head->next;

	while (clan != clan_table_tail) {
		if (obj->pIndexData->vnum >= clan->area_minvnum
		    && obj->pIndexData->vnum <= clan->area_maxvnum) {
			ptc(ch, "You attempt to auction %s, which belongs to %s.\n"
			    "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n"
			    "You are lucky you weren't {Phurt!{x\n",
			    obj->short_descr,
			    clan->clanname,
			    obj->short_descr);
			obj_from_char(obj);
			extract_obj(obj);
			return;
		}

		clan = clan->next;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("Hah, you can't get rid of it that easily!\n", ch);
		return;
	}

	if (obj->pIndexData->vnum == OBJ_VNUM_NEWBIEBAG
	    || obj->pIndexData->vnum == OBJ_VNUM_TOKEN) {
		stc("I'm sorry, you can't auction newbie items.\n", ch);
		return;
	}

	if (!arg2.empty()) {
		if (!arg2.is_number()) {
			stc("Minimum bid value must be numeric.\n", ch);
			return;
		}

		min = atoi(arg2);

		if (min < 1 || min > 10000) {
			stc("The minimum bid must be between 1 and 10000 gold.\n", ch);
			return;
		}
	}

	switch (obj->item_type) {
	default:
		act("You cannot auction $Ts.", ch, nullptr, item_type_name(obj), TO_CHAR);
		return;

	case ITEM_WEAPON:
	case ITEM_LIGHT:
	case ITEM_ARMOR:
	case ITEM_WAND:
	case ITEM_FOOD:
	case ITEM_STAFF:
	case ITEM_POTION:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_BOAT:
	case ITEM_PILL:
	case ITEM_MAP:
	case ITEM_WARP_STONE:
	case ITEM_JEWELRY:
	case ITEM_SCROLL:
	case ITEM_TOKEN:
		obj_from_char(obj);
		auction.item   = obj;
		auction.bet    = 0;
		auction.buyer  = nullptr;
		auction.seller = ch;
		auction.pulse  = PULSE_AUCTION;
		auction.going  = 0;
		auction.min    = min;

		if (auction.min > 0)
			Format::sprintf(buf, "The auctioneer receives %s and announces a minimum bid of %d gold.\n",
			        obj->short_descr, auction.min);
		else
			Format::sprintf(buf, "The auctioneer receives %s and places it on the auction block.\n",
			        obj->short_descr);

		talk_auction(buf);
		Format::sprintf(buf, "%s is auctioning %s.", ch->name, obj->short_descr);
		wiznet(buf, ch, nullptr, WIZ_AUCTION, 0, GET_RANK(ch));
		return;
	}
}
