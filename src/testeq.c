if (eq_type != 14){ //armor prefix will loop till hits prefix_max
	  if ((prefix_max != 0)){
		while ((i++ <= prefix_max)){
			switch(number_range(1,6)){
				case 1:{
					if ((armor_prefix != 1) && (prior_prefix != 1)){
						if (i == 1){
							sprintf(prefix1, "{WL{gu{Tn{ga{Wr{x " );	 	//+2 int/+25 mana
						}
						else	sprintf(prefix2, "{WL{gu{Tn{ga{Wr{x " );
						armor_prefix = 1;
						prior_prefix = 1;
						sprintf(prefix_name, "lunar");
						
						loc		= APPLY_INT;
						mod		= 25;
					}
					else i--;
					break;
				}
				case 2:{
					if ((armor_prefix != 2) && (prior_prefix != 2)){
						if (i == 1){
							sprintf(prefix1, "{WA{Cs{Ttr{Ca{Wl{x "); 	//+4 int/+75 mana
						}
						else	sprintf(prefix2, "{WA{Cs{Ttr{Ca{Wl{x ");
						armor_prefix = 2;
						prior_prefix = 2;
						sprintf(prefix_name, "astral");
						loc 	= APPLY_INT;
						mod 	= 4;
						loc2	= APPLY_MANA;
						mod2	= 75;
					}
					else i--;
					break;
				}						
				case 3:{
					if ((armor_prefix != 3) && (prior_prefix != 3)){
						if (i == 1){
							sprintf(prefix1,	"Celestrial ");				//+4 int&wis/ +150 mana
						}
						else 	sprintf(prefix2,	"Celestrial ");
						armor_prefix = 3;
						prior_prefix = 3;
						sprintf(prefix_name, "celestrial");
						loc 	= APPLY_INT;
						mod 	= 4;
						loc2	= APPLY_MANA;
						mod2	= 150;
						loc3	= APPLY_WIS;
						mod3	= 4;
					}
					else i--;
					break;
				}
				case 4:{
					if ((armor_prefix != 4) && (prior_prefix != 4)){
						if (i == 1){
							sprintf(prefix1, "Wind Swept ");				//+2 dex/+25 stam
						}
						else 	sprintf(prefix2, "Wind Swept ");
						armor_prefix = 4;
						prior_prefix = 4;
						sprintf(prefix_name, "wind swept");
						loc 	= APPLY_DEX;
						mod 	= 2;
						loc2	= APPLY_STAM;
						mod2	= 25;
					}
					else i--;
					break;
				}
				case 5:{
					if ((armor_prefix != 5) && (prior_prefix != 5)){
						if (i == 1){
							sprintf(prefix1, "Torrential ");				//+4 dex/+75 stam
						}
						else 	sprintf(prefix2, "Torrential ");
						armor_prefix = 5;
						prior_prefix = 5;
						sprintf(prefix_name, "torrential");
						loc 	= APPLY_DEX;
						mod 	= 4;
						loc2	= APPLY_STAM;
						mod2	= 75;
					}
					else i--;
					break;
				}
				case 6:{
					if ((armor_prefix != 6) && (prior_prefix != 6)){
						if (i == 1){
							sprintf(prefix1, "Gale ");					//+4 dex/+150 stam
						}
						else 	sprintf(prefix2, "Gale ");
						armor_prefix = 6;
						prior_prefix = 6;
						sprintf(prefix_name, "gale");
						loc 	= APPLY_DEX;
						mod 	= 4;
						loc2	= APPLY_STAM;
						mod2	= 150;
					}
					else i--;
					break;
				}
			}
		}
	  }
	}
	else {
		if ((prefix_max != 0)){
			while ((i++ <= prefix_max)){//weapon prefix will loop till hits prefix_max
				switch(number_range(1,9)){
					case 1:{
						if ((weapon_prefix != 1) && (prior_prefix !=1)){
							if (i == 1){
								sprintf(prefix1,"Jagged ");				//+1 dice sides
							}
							else 	sprintf(prefix2,"Jagged ");
							weapon_prefix = 1;
							prior_prefix = 1;
							sprintf(prefix_name, "jagged");
						}
						else i--;
						break; 
					}
					case 2:{
						if ((weapon_prefix != 2) && (prior_prefix != 2)){
							if (i == 1){
								sprintf(prefix1,"Savage ");				//+2 dice sides
							}
							else 	sprintf(prefix2,"Savage ");
							weapon_prefix = 2;
							prior_prefix = 2;
							sprintf(prefix_name, "savage");
						}
						else i--;
						break;
					}
					case 3:{
						if ((weapon_prefix != 3) && (prior_prefix != 3)){
							if (i == 1){
								sprintf(prefix1,"Cruel ");				//+3 dice sides
							}
							else 	sprintf(prefix2,"Cruel ");
							weapon_prefix = 3;
							prior_prefix = 3;
							sprintf(prefix_name, "cruel");
						}
						else i--;
						break;
					}
					case 4:{
						if ((weapon_prefix != 4) && (prior_prefix != 4)){
							if (i == 1){
								sprintf(prefix1,"Snowy ");				//frost blade
							}
							else 	sprintf(prefix2,"Snowy ");
							weapon_prefix = 4;
							prior_prefix = 4;
							sprintf(prefix_name, "snowy");
						}
						else i--;
						break;
					}
					case 5:{
						if ((weapon_prefix != 5) && (prior_prefix != 5)){
							if (i == 1){
								sprintf(prefix1,"Fiery ");				//flame blade
							}
							else 	sprintf(prefix2,"Fiery ");
							weapon_prefix = 5;
							prior_prefix = 5;
							sprintf(prefix_name, "fiery");
						}
						else i--;
						break;
					}
					case 6:{
						if ((weapon_prefix != 6) && (prior_prefix != 6)){
							if (i == 1){
								sprintf(prefix1,"Arcing ");				//shock blade
							}
							else 	sprintf(prefix2,"Arcing ");
							weapon_prefix = 6;
							prior_prefix = 6;
							sprintf(prefix_name, "arcing");
						}
						else i--;
						break;
					}
					case 7:{
						if ((weapon_prefix != 7) && (prior_prefix !=7)){
							if (i == 1){
								sprintf(prefix1,"Thirsty ");				//blood blade
							}
							else 	sprintf(prefix2,"Thirsty ");
							weapon_prefix = 7;
							prior_prefix = 7;
							sprintf(prefix_name, "thirsty");
						}
						else i--;
						break;
					}
					case 8:{
						if ((weapon_prefix != 8) && (prior_prefix != 8)){
							if (i == 1){
								sprintf(prefix1,"Sharp ");				//sharp flag
							}
							else 	sprintf(prefix2,"Sharp ");
							weapon_prefix = 8;
							prior_prefix = 8;
							sprintf(prefix_name, "sharp");
						}
						else i--;
						break;
					}
					case 9:{
						if ((weapon_prefix != 9) && (prior_prefix != 9)){
							if (i == 1){
								sprintf(prefix1,"Piercing ");			//piercing flag
							}
							else 	sprintf(prefix2,"Piercing ");
							weapon_prefix 	= 9;
							prior_prefix	= 9;
							sprintf(prefix_name, "piercing");
						}
						else i--;
						break;
					}
				}
			}
		}
	}