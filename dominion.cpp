#include "dominion.hpp"

/* ***** constructors ***** */

Game* newGame(int desiredNumPlayers, Set* desiredSet)
{
	/* validation */
	if (desiredSet == NULL
		|| desiredNumPlayers < MIN_NUM_PLAYERS
		|| desiredNumPlayers > MAX_NUM_PLAYERS)
		return NULL;

	Game* newGame = new Game;

	/* players */
	newGame->numPlayers = desiredNumPlayers;
	newGame->players = new Player[newGame->numPlayers];
	newGame->currPlayer = &newGame->players[0];
	newGame->nextPlayer = &newGame->players[1 % newGame->numPlayers];
	newGame->prevPlayer = &newGame->players[newGame->numPlayers - 1];

	/* player 1 begins the game */
	newGame->players[0].numTurnsTaken = 1;

	/* player ids and piles */
	for (int i = 0; i < newGame->numPlayers; i++)
	{
		newGame->players[i].id = i;

		for (int j = 0; j < 7; j++)
			newGame->players[i].deck[j] = Cards::copper;
		for (int j = 7; j < 10; j++)
			newGame->players[i].deck[j] = Cards::estate;

		newGame->players[i].deckSize = 10;

		shuffleDeck(&newGame->players[i]);
	}

	/* supply */
	newGame->set = desiredSet;
	for (int i = 0; i < MAX_NUM_SUPPLY_CARDS; i++)
		newGame->supply[i] = -1;

	/* basic treasure cards */
	newGame->supply[(int)Cards::copper] = 60 - (7 * newGame->numPlayers);
	newGame->supply[(int)Cards::silver] = 40;
	newGame->supply[(int)Cards::gold] = 30;

	/* basic victory cards */
	newGame->supply[(int)Cards::estate]
		= newGame->supply[(int)Cards::duchy]
		= newGame->supply[(int)Cards::province]
		= (newGame->numPlayers == 2 ? 8 : 12);

	/* curse cards */
	newGame->supply[(int)Cards::curse] = 10 * (newGame->numPlayers - 1);

	/* kingdom cards */
	for (int i = 0; i < newGame->set->numKingdomCards; i++)
	{
		if (isCardType(newGame->set->kingdomCards[i], Types::victory))
			newGame->supply[(int)newGame->set->kingdomCards[i]] = (newGame->numPlayers == 2 ? 8 : 12);
		else
			newGame->supply[(int)newGame->set->kingdomCards[i]] = 10;
	}

	return newGame;
}

Set* newRecommendedSet(Sets set)
{
	/* validation */
	if (set < (Sets)0 || set > Sets::NUM_SETS)
		return NULL;

	Set* newSet = new Set;
	newSet->name = getSetName(set);
	newSet->numKingdomCards = 10;
	newSet->kingdomCards = new Cards[newSet->numKingdomCards];

	switch (set)
	{
	case Sets::first_game:
		newSet->kingdomCards[0] = Cards::cellar;
		newSet->kingdomCards[1] = Cards::market;
		newSet->kingdomCards[2] = Cards::merchant;
		newSet->kingdomCards[3] = Cards::militia;
		newSet->kingdomCards[4] = Cards::mine;
		newSet->kingdomCards[5] = Cards::moat;
		newSet->kingdomCards[6] = Cards::remodel;
		newSet->kingdomCards[7] = Cards::smithy;
		newSet->kingdomCards[8] = Cards::village;
		newSet->kingdomCards[9] = Cards::workshop;
		break;
	case Sets::size_distortion:
		newSet->kingdomCards[0] = Cards::artisan;
		newSet->kingdomCards[1] = Cards::bandit;
		newSet->kingdomCards[2] = Cards::bureaucrat;
		newSet->kingdomCards[3] = Cards::chapel;
		newSet->kingdomCards[4] = Cards::festival;
		newSet->kingdomCards[5] = Cards::gardens;
		newSet->kingdomCards[6] = Cards::sentry;
		newSet->kingdomCards[7] = Cards::throne_room;
		newSet->kingdomCards[8] = Cards::witch;
		newSet->kingdomCards[9] = Cards::workshop;
		break;
	case Sets::deck_top:
		newSet->kingdomCards[0] = Cards::artisan;
		newSet->kingdomCards[1] = Cards::bureaucrat;
		newSet->kingdomCards[2] = Cards::council_room;
		newSet->kingdomCards[3] = Cards::festival;
		newSet->kingdomCards[4] = Cards::harbringer;
		newSet->kingdomCards[5] = Cards::laboratory;
		newSet->kingdomCards[6] = Cards::moneylender;
		newSet->kingdomCards[7] = Cards::sentry;
		newSet->kingdomCards[8] = Cards::vassal;
		newSet->kingdomCards[9] = Cards::village;
		break;
	case Sets::sleight_of_hand:
		newSet->kingdomCards[0] = Cards::cellar;
		newSet->kingdomCards[1] = Cards::council_room;
		newSet->kingdomCards[2] = Cards::festival;
		newSet->kingdomCards[3] = Cards::gardens;
		newSet->kingdomCards[4] = Cards::library;
		newSet->kingdomCards[5] = Cards::harbringer;
		newSet->kingdomCards[6] = Cards::militia;
		newSet->kingdomCards[7] = Cards::poacher;
		newSet->kingdomCards[8] = Cards::smithy;
		newSet->kingdomCards[9] = Cards::throne_room;
		break;
	case Sets::improvements:
		newSet->kingdomCards[0] = Cards::artisan;
		newSet->kingdomCards[1] = Cards::cellar;
		newSet->kingdomCards[2] = Cards::market;
		newSet->kingdomCards[3] = Cards::merchant;
		newSet->kingdomCards[4] = Cards::mine;
		newSet->kingdomCards[5] = Cards::moat;
		newSet->kingdomCards[6] = Cards::moneylender;
		newSet->kingdomCards[7] = Cards::poacher;
		newSet->kingdomCards[8] = Cards::remodel;
		newSet->kingdomCards[9] = Cards::witch;
		break;
	case Sets::silver_and_gold:
		newSet->kingdomCards[0] = Cards::bandit;
		newSet->kingdomCards[1] = Cards::bureaucrat;
		newSet->kingdomCards[2] = Cards::chapel;
		newSet->kingdomCards[3] = Cards::harbringer;
		newSet->kingdomCards[4] = Cards::laboratory;
		newSet->kingdomCards[5] = Cards::merchant;
		newSet->kingdomCards[6] = Cards::mine;
		newSet->kingdomCards[7] = Cards::moneylender;
		newSet->kingdomCards[8] = Cards::throne_room;
		newSet->kingdomCards[9] = Cards::vassal;
		break;
	}

	return newSet;
}

Set* newRandomSet()
{
	Set* newSet = new Set;
	newSet->name = "Random";
	newSet->numKingdomCards = 10;
	newSet->kingdomCards = new Cards[newSet->numKingdomCards];

	for (int i = 0; i < newSet->numKingdomCards; i++)
	{
		bool duplicate = true;
		while (duplicate == true)
		{
			duplicate = false;
			/* get random card */
			newSet->kingdomCards[i] = (Cards) (rand() % (int) Cards::NUM_CARDS);
			for (int j = 0; j < i; j++)
				if (newSet->kingdomCards[i] == newSet->kingdomCards[j])
					duplicate = true;
		}
	}

	return newSet;
}

Set* newCustomSet(Cards desiredKingdomCards[MAX_NUM_KINGDOM_CARDS], int desiredNumKingdomCards)
{
	/* validation */
	if (desiredKingdomCards == NULL
		|| desiredNumKingdomCards < MIN_NUM_KINGDOM_CARDS
		|| desiredNumKingdomCards > MAX_NUM_KINGDOM_CARDS)
		return NULL;

	Set* newSet = new Set;
	newSet->name = "Custom";
	newSet->kingdomCards = desiredKingdomCards;
	newSet->numKingdomCards = desiredNumKingdomCards;

	return newSet;
}

/* ***** name getters ***** */

const char* getCardName(Cards card)
{
	switch (card)
	{
		/* dominion :: basic supply cards */
		case Cards::curse:
			return "Curse";

		case Cards::copper:
			return "Copper";
		case Cards::silver:
			return "Silver";
		case Cards::gold:
			return "Gold";

		case Cards::estate:
			return "Estate";
		case Cards::duchy:
			return "Duchy";
		case Cards::province:
			return "Province";

		/* dominion :: 2-cost kingdom cards */
		case Cards::cellar:
			return "Cellar";
		case Cards::chapel:
			return "Chapel";
		case Cards::moat:
			return "Moat";

		/* dominion :: 3-cost kingdom cards */
		case Cards::harbringer:
			return "Harbringer";
		case Cards::merchant:
			return "Merchant";
		case Cards::vassal:
			return "Vassal";
		case Cards::village:
			return "Village";
		case Cards::workshop:
			return "Workshop";

		/* dominion :: 4-cost kingdom cards */
		case Cards::bureaucrat:
			return "Bureaucrat";
		case Cards::gardens:
			return "Gardens";
		case Cards::militia:
			return "Militia";
		case Cards::moneylender:
			return "Moneylender";
		case Cards::poacher:
			return "Poacher";
		case Cards::remodel:
			return "Remodel";
		case Cards::smithy:
			return "Smithy";
		case Cards::throne_room:
			return "Throne Room";

		/* dominion :: 5-cost kingdom cards */
		case Cards::bandit:
			return "Bandit";
		case Cards::council_room:
			return "Council Room";
		case Cards::festival:
			return "Festival";
		case Cards::laboratory:
			return "Laboratory";
		case Cards::library:
			return "Library";
		case Cards::market:
			return "Market";
		case Cards::mine:
			return "Mine";
		case Cards::sentry:
			return "Sentry";
		case Cards::witch:
			return "Witch";

		/* dominion :: 6-cost kingdom cards */
		case Cards::artisan:
			return "Artisan";

		/* default error */
		default:
			fprintf(stderr, "ERROR: Name not found for card %d.\n", card);
			return "ERROR";
	}
}

const char* getCardTypeName(Cards card)
{
	Type type = getCardType(card);

	char* cardTypeName = new char[MAX_NUM_TYPE_CHARS];
	strcpy(cardTypeName, getTypeName(type.type[0]));

	for (int i = 1; i < type.numTypes; i++)
	{
		strcat(cardTypeName, " - ");
		strcat(cardTypeName, getTypeName(type.type[i]));
	}

	return cardTypeName;
}

const char* getLocationName(Locations location)
{
	switch (location)
	{
		case Locations::hand:
			return "Hand";
		case Locations::deck:
			return "Deck";
		case Locations::discard:
			return "Discard";
		case Locations::played:
			return "Played";
		case Locations::peeked:
			return "Peeked";
		case Locations::revealed:
			return "Revealed";
		case Locations::supply:
			return "Supply";
		case Locations::trash:
			return "Trash";
		case Locations::NUM_LOCATIONS:
			return "NUM_LOCATIONS";
		default:
			fprintf(stderr, "ERROR: getLocationName() could not resolve locaton %d.\n", (int) location);
			return "ERROR";
	}
}

const char* getSetName(Sets set)
{
	switch (set)
	{
		case Sets::first_game:
			return "First Game";
		case Sets::size_distortion:
			return "Size Distortion";
		case Sets::deck_top:
			return "Deck Top";
		case Sets::sleight_of_hand:
			return "Sleight of Hand";
		case Sets::improvements:
			return "Improvements";
		case Sets::silver_and_gold:
			return "Silver & Gold";
		default:
			fprintf(stderr, "ERROR: getSetName() could not resolve set %d.\n", (int) set);
			return "ERROR";
	}
}

const char* getPhaseName(Phases phase)
{
	switch (phase)
	{
		case Phases::action:
			return "Action";
		case Phases::buy:
			return "Buy";
		case Phases::clean_up:
			return "Clean-Up";
		default:
			fprintf(stderr, "ERROR: getPhaseName(): Cannot resolve phase %d.\n", (int) phase);
			return "ERROR";
	}
}

const char* getTypeName(Types type)
{
	switch (type)
	{
		case Types::action:
			return "Action";
		case Types::treasure:
			return "Treasure";
		case Types::victory:
			return "Victory";
		case Types::curse:
			return "Curse";
		case Types::attack:
			return "Attack";
		case Types::duration:
			return "Duration";
		case Types::reaction:
			return "Reaction";
		case Types::command:
			return "Command";
		case Types::castle:
			return "Castle";
		case Types::doom:
			return "Doom";
		case Types::fate:
			return "Fate";
		case Types::gathering:
			return "Gathering";
		case Types::heirloom:
			return "Heirloom";
		case Types::knight:
			return "Knight";
		case Types::looter:
			return "Looter";
		case Types::night:
			return "Night";
		case Types::prize:
			return "Prize";
		case Types::reserve:
			return "Reserve";
		case Types::ruins:
			return "Ruins";
		case Types::shelter:
			return "Shelter";
		case Types::spirit:
			return "Spirit";
		case Types::traveller:
			return "Traveller";
		case Types::zombie:
			return "Zombie";
		case Types::artifact:
			return "Artifact";
		case Types::boon:
			return "Boon";
		case Types::event:
			return "Event";
		case Types::hex:
			return "Hex";
		case Types::landmark:
			return "Landmark";
		case Types::project:
			return "Project";
		case Types::state:
			return "State";

		/* default error */
		default:
			fprintf(stderr, "ERROR: Name not found for type %d.\n", type);
			return "ERROR";
	}
}

/* ***** card interaction ***** */

int getCardCoin(Cards card)
{
	switch (card)
	{
		case Cards::copper:
			return 1;
		case Cards::silver:
			return 2;
		case Cards::gold:
			return 3;
		default:
			return 0;
	}
}

int getCardCost(Cards card)
{
	switch (card)
	{
		case Cards::curse:
		case Cards::copper:
			return 0;
		case Cards::estate:
		case Cards::cellar:
		case Cards::chapel:
		case Cards::moat:
			return 2;
		case Cards::silver:
		case Cards::harbringer:
		case Cards::merchant:
		case Cards::vassal:
		case Cards::village:
		case Cards::workshop:
			return 3;
		case Cards::bureaucrat:
		case Cards::gardens:
		case Cards::militia:
		case Cards::moneylender:
		case Cards::poacher:
		case Cards::remodel:
		case Cards::smithy:
		case Cards::throne_room:
			return 4;
		case Cards::duchy:
		case Cards::bandit:
		case Cards::council_room:
		case Cards::festival:
		case Cards::laboratory:
		case Cards::library:
		case Cards::market:
		case Cards::mine:
		case Cards::sentry:
		case Cards::witch:
			return 5;
		case Cards::gold:
		case Cards::artisan:
			return 6;
		case Cards::province:
			return 8;

		/* default error */
		default:
			fprintf(stderr, "ERROR: Cost not found for %s.\n", getCardName(card));
			return -1;
	}
}

Type getCardType(Cards card)
{
	Type type = Type();

	switch (card)
	{
		/* dominion :: curse */
		case Cards::curse:
			type.type[0] = Types::curse;
			type.numTypes = 1;
			return type;

		/* dominion :: treasure */
		case Cards::copper:
		case Cards::silver:
		case Cards::gold:
			type.type[0] = Types::treasure;
			type.numTypes = 1;
			return type;

		/* dominion :: victory */
		case Cards::estate:
		case Cards::duchy:
		case Cards::province:
		case Cards::gardens:
			type.type[0] = Types::victory;
			type.numTypes = 1;
			return type;

		/* dominion :: action */
		case Cards::cellar:
		case Cards::chapel:

		case Cards::harbringer:
		case Cards::merchant:
		case Cards::vassal:
		case Cards::village:
		case Cards::workshop:

		case Cards::moneylender:
		case Cards::poacher:
		case Cards::remodel:
		case Cards::smithy:
		case Cards::throne_room:

		case Cards::council_room:
		case Cards::festival:
		case Cards::laboratory:
		case Cards::library:
		case Cards::market:
		case Cards::mine:
		case Cards::sentry:

		case Cards::artisan:
			type.type[0] = Types::action;
			type.numTypes = 1;
			return type;

		/* dominion :: action - reaction */
		case Cards::moat:
			type.type[0] = Types::action;
			type.type[1] = Types::reaction;
			type.numTypes = 2;
			return type;

		/* dominion :: action - attack */
		case Cards::bureaucrat:
		case Cards::militia:

		case Cards::bandit:
		case Cards::witch:
			type.type[1] = Types::attack;
			type.type[0] = Types::action;
			type.numTypes = 2;
			return type;

		/* default error */
		default:
			fprintf(stderr, "ERROR: Type not found for card %s.\n", getCardName(card));
			type.type[0] = Types::NUM_TYPES;
			type.numTypes = 99;
			return type;
	}
}

bool isCardType(Cards card, Types type)
{
	Type cardType = getCardType(card);

	for (int i = 0; i < cardType.numTypes; i++)
		if (cardType.type[i] == type)
			return true;

	return false;
}

int getCardVictoryPoints(Player* player, Cards card)
{
	switch (card)
	{
		case Cards::curse:
			return -1;
		case Cards::estate:
			return 1;
		case Cards::duchy:
			return 3;
		case Cards::province:
			return 6;
		case Cards::gardens:
			return (player->handSize + player->deckSize + player->discardSize + player->playedSize) / 10;
		default:
			return 0;
	}
}

/* ***** pile (hand, deck, discard, etc.) interaction ***** */

/* drawCard() is an efficient alternative to moveCard(), which can perform the same task */
/* returns true if card was successfully drawn */
/* returns false if card could not be drawn, but nothing was violated */
bool drawCard(Player* player)
{
	/* case: empty deck, non-empty discard */
	if (player->deckSize == 0 && player->discardSize > 0)
	{
		while(player->discardSize > 0)
			player->deck[player->deckSize++] = player->discard[--player->discardSize];
		shuffleDeck(player);
	}

	/* case: non-empty deck, empty or non-empty discard pile */
	if (player->deckSize > 0)
	{
		player->hand[player->handSize++] = player->deck[--player->deckSize];
		return true;
	}

	/* case: empty deck, empty discard pile (nothing happens) */
	return false;
}

/* future implementation must consider location of bought card, bridges played, etc. */
bool buyCard(Game* game, Cards card)
{
	int cost = getCardCost(card);

	/* validation */
	if (cost > game->coins) // bridge
		return false;

	switch (game->supply[(int)card])
	{
	case -1:
		fprintf(stderr, "ERROR: buyCard(): Card %d (%s) is not in the set. Calling printSet()..\n",
			(int) card, getCardName(card));
		printSet(stderr, game->set);
		return false;
	case 0:
		fprintf(stderr, "ERROR: buyCard(): Supply pile of card %d (%s) is empty.\n",
			(int) card, getCardName(card));
		return false;
	default:
		if (gainCard(game, game->currPlayer, card, Locations::discard) == 1) // location
		{
			game->coins -= cost; // bridge
			return true;
		}
		return false;
	}
}

/* gains a card to a player from the supply only */
/* returns 1 if card was successfully gained */
/* returns 0 if card could not be gained, but nothing was violated */
/* returns -1 if internal values are bad */
int gainCard(Game* game, Player* player, Cards card, Locations destination)
{
	/* validation */
	switch (destination)
	{
		case Locations::hand:
		case Locations::deck:
		case Locations::discard:
			break;
		default:
			fprintf(stderr, "ERROR: gainCard() cannot gain card to location %d (%s). Please use moveCard().\n",
				(int) destination, getLocationName(destination));
			return -1;
	}

	/* if supply is empty, cannot gain card */
	if (game->supply[(int) card] == 0)
		return 0;

	/* gain card */
	switch (destination)
	{
		case Locations::hand:
			player->hand[player->handSize++] = card;
			break;
		case Locations::deck:
			player->deck[player->deckSize++] = card;
			break;
		case Locations::discard:
			player->discard[player->discardSize++] = card;
			break;
	}

	/* decrement supply */
	game->supply[(int) card]--;

	/* determine if the game has ended */
	if (isEndOfGame(game))
	{
		Winners winners = getWinners(game);
		if (winners.numWinners == 1)
		{
			fprintf(stderr, "GAME END! Player %d wins with a score of %d victory points!\n",
				winners.winners[0].id, winners.score);
			exit(0);
		}
		else
		{
			fprintf(stderr, "GAME END! Players ");
			for (int i = 0; i < winners.numWinners; i++)
				fprintf(stderr, "%d, ", winners.winners[i].id);
			fprintf(stderr, "all tie with a score of %d victory points!\n", winners.score);
			exit(0);
		}
	}

	return 1;
}

/* moves a card between piles, except from supply which should be done with gainCard() */
/* returns 1 if card was successfully moved */
/* returns 0 if card could not be moved, but nothing was violated (such as drawing on empty) */
/* returns -1 if internal values are bad */
int moveCard(Game* game, Player* player, Locations source, Locations destination, int position)
{
	/* validation */
	if (source == destination)
	{
		fprintf(stderr, "WARNING: moveCard(): locations 'from' and 'to' are the same.\n");
	}
	if (source == Locations::supply)
	{
		fprintf(stderr, "ERROR: moveCard() cannot move cards from supply. Please use gainCard().\n");
		return -1;
	}
	if (position < 0)
	{
		fprintf(stderr, "ERROR: moveCard(): negative position (%d).\n", position);
		return -1;
	}
	if (	(source == Locations::hand		&& position > player->handSize - 1		&& player->handSize != 0)
		||	(source == Locations::discard	&& position > player->discardSize - 1	&& player->discardSize != 0)
		||	(source == Locations::played	&& position > player->playedSize - 1	&& player->playedSize != 0)
		||	(source == Locations::peeked	&& position > player->peekedSize - 1	&& player->peekedSize != 0)
		||	(source == Locations::revealed	&& position > player->revealedSize - 1	&& player->revealedSize != 0)
		||	(source == Locations::trash		&& position > game->trashSize - 1		&& game->trashSize != 0) )
	{
		fprintf(stderr, "ERROR: moveCard(): position (%d) too high for location %d (%s).\n",
			position, (int) source, getLocationName(source));
		return -1;
	}

	/* prepare a non-empty deck, if possible */
	if (source == Locations::deck && player->deckSize == 0)
	{
		/* if deck and discard are empty, correctly do nothing */
		if (player->discardSize == 0)
			return 0;
		/* else move discard pile to deck, then shuffle */
		else
		{
			movePile(game, player, Locations::discard, Locations::deck);
			shuffleDeck(player);
		}
	}

	/* source and destination - piles and sizes*/
	Cards*	sourcePile		= getPile(game, player, source);
	Cards*	destinationPile	= getPile(game, player, destination);
	int*	sourceSize		= getPileSize(game, player, source);
	int*	destinationSize	= getPileSize(game, player, destination);

	/* validation */
	if (sourcePile == NULL || destinationPile == NULL
		|| sourceSize == NULL || destinationSize == NULL)
	{
		fprintf(stderr, "ERROR: moveCard(): getPile() and/or getPileSize() failed.\n");
		return -1;
	}

	/* move card from source to destination; decrement source size and shift pile (shiftPile does both) */
	if (*sourceSize > 0)
	{
		destinationPile[*destinationSize] = sourcePile[position];
		*destinationSize = *destinationSize + 1;
		shiftPile(sourcePile, sourceSize, position);
		return 1;
	}
	else
		return 0;

	/* this path should not occur */
	fprintf(stderr, "ERROR: moveCard(): Could not resolve path, reached end of function.\n");
	return -1;
}

/* moves a pile on top of another pile */
/* returns 1 if pile was successfully moved */
/* returns 0 if pile could not be moved, but nothing was violated (such as moving an empty pile) */
/* returns -1 if internal values are bad */
int	movePile(Game* game, Player* player, Locations source, Locations destination)
{

	/* source size */
	int* sourceSize = getPileSize(game, player, source);

	/* validation */
	if (sourceSize == NULL)
	{
		fprintf(stderr, "ERROR: movePile(): getPileSize() failed.\n");
		return -1;
	}
	if (*sourceSize == 0)
	{
		fprintf(stderr, "WARNING: movePile(): source pile is empty.\n");
		return 0;
	}

	/* move pile */
	while (*sourceSize > 0)
		moveCard(game, player, source, destination, *sourceSize - 1);

	return 1;
}

/* will not place discard pile into deck */
void shuffleDeck(Player* player)
{
	for (int i = 0; i < player->deckSize; i++)
	{
		int tempIndex = rand() % player->deckSize;
		Cards tempCard = player->deck[i];
		player->deck[i] = player->deck[tempIndex];
		player->deck[tempIndex] = tempCard;
	}
}

Cards* getPile(Game* game, Player* player, Locations location)
{
	if (location == Locations::supply)
	{
		fprintf(stderr, "ERROR: getPile() cannot return pile for supply.\n");
		return NULL;
	}

	switch (location)
	{
	case Locations::hand:
		return player->hand;
	case Locations::deck:
		return player->deck;
	case Locations::discard:
		return player->discard;
	case Locations::played:
		return player->played;
	case Locations::peeked:
		return player->peeked;
	case Locations::revealed:
		return player->revealed;
	case Locations::trash:
		return game->trash;
	default:
		fprintf(stderr, "ERROR: getPile() cannot resolve location %d (%s).\n",
			(int)location, getLocationName(location));
		return NULL;
	}
}

int* getPileSize(Game* game, Player* player, Locations location)
{
	if (location == Locations::supply)
	{
		fprintf(stderr, "ERROR: getPileSize() cannot return size for supply.\n");
		return NULL;
	}

	switch (location)
	{
	case Locations::hand:
		return &player->handSize;
	case Locations::deck:
		return &player->deckSize;
	case Locations::discard:
		return &player->discardSize;
	case Locations::played:
		return &player->playedSize;
	case Locations::peeked:
		return &player->peekedSize;
	case Locations::revealed:
		return &player->revealedSize;
	case Locations::trash:
		return &game->trashSize;
	default:
		fprintf(stderr, "ERROR: getPileSize() cannot resolve location %d (%s).\n",
			(int)location, getLocationName(location));
		return NULL;
	}
}

/* position = index of card removed from pile */
/* warning: decrements pile size by reference */
bool shiftPile(Cards* pile, int* pileSize, int position)
{
	/* validation */
	if (position > *pileSize - 1)
	{
		fprintf(stderr, "ERROR: shiftPile(): Position (%d) is greater than pile size - 1 (%d).",
			position, *pileSize);
		return false;
	}
	
	/* shift pile */
	for (int i = position; i < *pileSize - 1; i++)
		pile[i] = pile[i + 1];
	*pileSize = *pileSize - 1;
	return true;
}

/* returns matching card index closest to 0 */
/* returns -1 if card was not found */
int	getFirstCardInPile(Game* game, Player* player, Cards card, Locations location)
{
	Cards* pile = getPile(game, player, location);
	int* pileSize = getPileSize(game, player, location);

	for (int i = 0; i < *pileSize; i++)
		if (pile[i] == card)
			return i;

	fprintf(stderr, "ERROR: getFirstCardInPile(): Card %d (%s) not found in pile %d (%s).\n",
		(int) card, getCardName(card), (int) location, getLocationName(location));
	return -1;
}

/* returns matching card index closest to pile size */
/* returns -1 if card was not found */
int	getLastCardInPile(Game* game, Player* player, Cards card, Locations location)
{
	Cards* pile = getPile(game, player, location);
	int* pileSize = getPileSize(game, player, location);

	for (int i = *pileSize; i > 0; i--)
		if (pile[i - 1] == card)
			return i;

	fprintf(stderr, "ERROR: getLastCardInPile(): Card %d (%s) not found in pile %d (%s).\n",
		(int)card, getCardName(card), (int)location, getLocationName(location));
	return -1;
}

/* ***** turn interaction ***** */
void advancePhase(Game* game)
{
	switch (game->phase)
	{
	case Phases::action:
		game->phase = Phases::buy;
		break;
	case Phases::buy:
		game->phase = Phases::clean_up;
		endTurn(game);
	default:
		fprintf(stderr, "ERROR: advancePhase(): Cannot resolve phase %d (%s).\n",
			(int) game->phase, getPhaseName(game->phase));
	}
}

void endTurn(Game* game)
{
	/* move played pile into discard pile */
	/* future implementation will disregard duration cards, if understood correctly */
	movePile(game, game->currPlayer, Locations::played, Locations::discard);

	/* reset actions and coins */
	game->actions = game->coins = 0;

	/* increment turn and next player's number of turns taken */
	game->turn = (game->turn + 1) % game->numPlayers;
	game->players[game->turn].numTurnsTaken++;

	/* update player flags */
	for (int i = 0; i < game->numPlayers; i++)
	{
		game->players[i].moatFlag = false;
	}

	/* update game flags and counters */
	game->numMerchants = 0;

	/* update player pointers */
	game->currPlayer = &game->players[game->turn];
	game->nextPlayer = &game->players[(game->turn + 1) % game->numPlayers];
	if (game->turn == 0)
		game->prevPlayer = &game->players[game->numPlayers - 1];
	else
		game->prevPlayer = &game->players[game->turn - 1];
}

/* ***** score and end game ***** */

int getNumEmptySupplyPiles(Game* game)
{
	int numEmptySupplyPiles = 0;

	/* basic supply */
	if (game->supply[(int) Cards::copper] == 0)
		numEmptySupplyPiles++;
	if (game->supply[(int) Cards::silver] == 0)
		numEmptySupplyPiles++;
	if (game->supply[(int) Cards::gold] == 0)
		numEmptySupplyPiles++;
	if (game->supply[(int) Cards::estate] == 0)
		numEmptySupplyPiles++;
	if (game->supply[(int) Cards::duchy] == 0)
		numEmptySupplyPiles++;
	if (game->supply[(int) Cards::province] == 0)
		numEmptySupplyPiles++;
	if (game->supply[(int) Cards::curse] == 0)
		numEmptySupplyPiles++;

	/* kingdom cards */
	for (int i = 0; i < game->set->numKingdomCards; i++)
		if (game->supply[(int)game->set->kingdomCards[i]] == 0)
			numEmptySupplyPiles++;

	return numEmptySupplyPiles;
}

bool isEndOfGame(Game* game)
{
	/* province cards */
	if (game->supply[(int) Cards::province] == 0)
		return true;

	/* three empty supply piles */
	if (getNumEmptySupplyPiles(game) > 2)
		return true;

	return false;
}

int	getPlayerScore(Player* player)
{
	// victory point tokens
	int total = player->victoryPoints;

	// hand
	for (int i = 0; i < player->handSize; i++)
		total += getCardVictoryPoints(player, player->hand[i]);

	// deck
	for (int i = 0; i < player->deckSize; i++)
		total += getCardVictoryPoints(player, player->deck[i]);

	// discard
	for (int i = 0; i < player->discardSize; i++)
		total += getCardVictoryPoints(player, player->discard[i]);

	// played
	for (int i = 0; i < player->playedSize; i++)
		total += getCardVictoryPoints(player, player->played[i]);

	return total;
}

void setScores(Game* game)
{
	for (int i = 0; i < game->numPlayers; i++)
		game->players[i].score = getPlayerScore(&game->players[i]);
}

Winners getWinners(Game* game)
{
	/* scores should be up-to-date */
	/* this responsibility may be deferred to another function, but it is safest here */
	setScores(game);

	Winners winners;
	winners.winners[0] = game->players[0];
	winners.numWinners = 1;
	winners.score = game->players[0].score;

	for (int i = 1; i < game->numPlayers; i++)
	{
		if (game->players[i].score > winners.score)
		{
			winners.winners[0] = game->players[i];
			winners.numWinners = 1;
			winners.score = game->players[i].score;
		}
		else if (game->players[i].score == winners.score)
		{
			/* player with less turns taken wins in the event of a tie */
			if (game->players[i].numTurnsTaken < winners.winners[0].numTurnsTaken)
			{
				winners.winners[0] = game->players[i];
				winners.numWinners = 1;
			}
			/* players share victory with equal score and turns taken */
			else if (game->players[i].numTurnsTaken == winners.winners[0].numTurnsTaken)
			{
				/* append the additional winner */
				winners.winners[winners.numWinners++] = game->players[i];
			}
		}
	}

	return winners;
}

/* ***** printers ***** */

void printCardType(FILE* const stream, Cards card)
{
	fprintf(stream, "printCardType(%s):\n\t%s\n", getCardName(card), getCardTypeName(card));
}

void printDeck(FILE* const stream, Player* player)
{
	fprintf(stream, "printDeck(Player %d):\n", player->id + 1);
	for (int i = 0; i < player->deckSize; i++)
		fprintf(stream, "\tCard #%d:\t%s\n", i + 1, getCardName(player->deck[i]));
}

void printSet(FILE *const stream, Set* set)
{
	fprintf(stream, "printSet(%s):\n", set->name);
	for (int i = 0; i < set->numKingdomCards; i++)
		fprintf(stream, "\tCard #%d:\t%s\n", i + 1, getCardName(set->kingdomCards[i]));
}

void printType(FILE* const stream, Types type)
{
	fprintf(stream, "printType(%d):\n\t%s\n", type, getTypeName(type));
}

/* ***** card effects ***** */

/* TODO */
Choice getChoices(Game* game, Player* player, Cards card)
{
	return getChoices(game, player, card, -1);
}
Choice getChoices(Game* game, Player* player, Cards card, int index)
{
	Choice choices;
	return choices;
}

/* only the current player can play a card; see reactionCard() for alternatives */
/* playCard() assumes that decrementing actions or buys after the card effect occurs is safe */
bool playCard(Game* game, Locations source, int position)
{
	Cards*	pile = getPile(game, game->currPlayer, source);
	Cards	card = pile[position];

	/* validation */
	if (source == Locations::hand)
	{
		if (game->phase == Phases::action && game->actions < 1)
		{
			fprintf(stderr, "ERROR: playCard(): Not enough actions.\n");
			return false;
		}
		if (game->phase == Phases::buy && game->buys < 1)
		{
			fprintf(stderr, "ERROR: playCard(): Not enough buys.\n");
			return false;
		}
		if (position < 0 || position > game->currPlayer->handSize - 1)
		{
			fprintf(stderr, "ERROR: playCard(): Invalid card position source hand.\n");
			return false;
		}
	}

	switch (game->phase)
	{
		case Phases::action:

			/* only action cards can be played during action phase */
			if (isCardType(game->currPlayer->hand[position], Types::action))
				moveCard(game, game->currPlayer, source, Locations::played, position);
			else
			{
				fprintf(stderr, "ERROR: playCard(): Cannot play [%s] card %d (%s) during Action phase.\n",
					getCardTypeName(pile[position]), (int) pile[position], getCardName(pile[position]));
				return false;
			}

			/* play the action card */
			switch (card)
			{
				case Cards::cellar:
					playCardCellar(game);
					break;
				case Cards::chapel:
					playCardChapel(game);
					break;
				case Cards::moat:
					playCardMoat(game, game->currPlayer, false);
					break;
				case Cards::harbringer:
					playCardHarbringer(game);
					break;
				case Cards::merchant:
					playCardMerchant(game);
					break;
				case Cards::vassal:
					playCardVassal(game);
					break;
				case Cards::village:
					playCardVillage(game);
					break;
				case Cards::workshop:
					playCardWorkshop(game);
					break;
				case Cards::bureaucrat:
					playCardBureaucrat(game);
					break;
				case Cards::militia:
					playCardMilitia(game);
					break;
				case Cards::moneylender:
					playCardMoneylender(game);
					break;
				case Cards::poacher:
					playCardPoacher(game);
					break;
				case Cards::remodel:
					playCardRemodel(game);
					break;
				case Cards::smithy:
					playCardSmithy(game);
					break;
				case Cards::throne_room:
					playCardThroneRoom(game);
					break;
				case Cards::bandit:
					playCardBandit(game);
					break;
				case Cards::council_room:
					playCardCouncilRoom(game);
					break;
				case Cards::festival:
					playCardFestival(game);
					break;
				case Cards::laboratory:
					playCardLaboratory(game);
					break;
				case Cards::library:
					playCardLibrary(game);
					break;
				case Cards::market:
					playCardMarket(game);
					break;
				case Cards::mine:
					playCardMine(game);
					break;
				case Cards::sentry:
					playCardSentry(game);
					break;
				case Cards::witch:
					playCardWitch(game);
					break;
				case Cards::artisan:
					playCardArtisan(game);
					break;
				default:
					fprintf(stderr, "ERROR: playCard(): Could not resolve card %d (%s) during Action phase.\n",
						(int) game->currPlayer->hand[position], getCardName(game->currPlayer->hand[position]));
					return false;
			}

			/* if played from hand, decrement actions; return */
			if (source == Locations::hand)
				game->actions--;
			return true;
		
		case Phases::buy:
			/* only treasure cards can be played during buy phase */
			if (isCardType(game->currPlayer->hand[position], Types::treasure))
				moveCard(game, game->currPlayer, Locations::hand, Locations::played, position);
			else
			{
				fprintf(stderr, "ERROR: playCard(): Cannot play [%s] card %d (%s) during Buy phase.\n",
					getCardTypeName(game->currPlayer->hand[position]),
					(int) game->currPlayer->hand[position],
					getCardName(game->currPlayer->hand[position]));
				return false;
			}

			/* play the treasure card */
			switch (game->currPlayer->hand[position])
			{
				case Cards::copper:
					playCardCopper(game);
					break;
				case Cards::silver:
					playCardSilver(game);
					break;
				case Cards::gold:
					playCardGold(game);
					break;
				default:
					fprintf(stderr, "ERROR: playCard(): Could not resolve [%s] card %d (%s) during Action phase.\n",
						getCardTypeName(pile[position]), (int)pile[position], getCardName(pile[position]));
					return false;
			}

		default:
			fprintf(stderr, "ERROR: playCard(): Could not resolve phase %d.\n", (int) game->phase);
			return false;
	}
}

/* returns true if reaction was executed */
/* returns false if reaction cannot be executed */
bool reactionCard(Game* game, Player* player, Locations source, int position)
{
	/* move card to revealed */
	moveCard(game, player, source, Locations::revealed, position);

	/* resolve may go here if applicable */
	/* resolve(game); */

	switch (player->revealed[player->revealedSize])
	{
		case Cards::moat:
			playCardMoat(game, player, true);
			return true;
		default:
			fprintf(stderr, "ERROR: reactionCard(): Cannot resolve card %d (%s) from pile %d (%s).\n",
				(int) player->revealed[player->revealedSize], getCardName(player->revealed[player->revealedSize]),
				(int) source, getLocationName(source));
			return false;
	}
}

// Card: Cellar
// Type: Action
// Cost: 2
//
//	+1 Action
//	Discard any number of
//	cards, then draw that many.
void playCardCellar(Game* game)
{
	/* +1 Action */
	game->actions += 1;

	/* choices.choices[n] = position of each card to discard */
	/* choices.numChoices = number of cards to discard */
	Choice choices = getChoices(game, game->currPlayer, Cards::cellar);

	/* discard cards */
	for (int i = 0; i < choices.numChoices; i++)
		moveCard(game, game->currPlayer, Locations::hand, Locations::discard, choices.choices[i]);

	/* draw the same number of cards */
	for (int i = 0; i < choices.numChoices; i++)
		drawCard(game->currPlayer);
}

// Card: Chapel
// Type: Action
// Cost: 2
//
//	Trash up to 4 cards from
//	your hand.
void playCardChapel(Game* game)
{
	/* choices.choices[n] = index of cards to trash in ascending order */
	/* choices.numChoices = number of cards to trash */
	Choice choices = getChoices(game, game->currPlayer, Cards::chapel);

	/* trash cards from hand descending from highest index for the sake of hand shifting */
	for (int i = choices.numChoices; i > 0; i--)
		moveCard(game, game->currPlayer, Locations::hand, Locations::trash, choices.choices[i - 1]);
}


// Card: Moat
// Type: Action - Reaction
// Cost: 2
//
//	+2 Cards
//	-
//	When another player plays
//	an Attack card, you may first
//	reveal this from your hand, to
//	be unaffected by it.
void playCardMoat(Game* game, Player* player, bool trigger)
{
	/* played as reaction */
	if (trigger)
	{
		/* find moat and return it to the player's hand */
		player->moatFlag = true;
		int position = getLastCardInPile(game, player, Cards::moat, Locations::revealed);
		if (position < 0)
			fprintf(stderr, "ERROR: playCardMoat(): Moat not found in revealed pile.\n");
		moveCard(game, player, Locations::revealed, Locations::hand, position);
	}

	/* played as action */
	else
	{

		/* +2 Cards */
		drawCard(player);
		drawCard(player);
	}
}

// Card: Harbringer
// Type: Action
// Cost: 3
//
//	+1 Card
//	+1 Action
//	Look through your discard
//	pile. You may put a card
//	from it onto your deck.
void playCardHarbringer(Game* game)
{
	/* +1 Card */
	drawCard(game->currPlayer);

	/* +1 Action */
	game->actions += 1;

	/* choices.choices[0] = -1 if player cannot or chooses not to move card */
	/* choices.choices[0] = index of card to move otherwise */
	Choice choices = getChoices(game, game->currPlayer, Cards::harbringer);

	/* move card from dicard pile to deck */
	if (choices.choices[0] > -1)
		moveCard(game, game->currPlayer, Locations::discard, Locations::deck, choices.choices[0]);
}

// Card: Merchant
// Type: Action
// Cost: 3
//
//	+1 Card
//	+1 Action
//
//	The first time you play a
//	Silver this turn, +1 Coin.
void playCardMerchant(Game* game)
{
	/* +1 Card */
	drawCard(game->currPlayer);

	/* +1 Action */
	game->actions += 1;

	/* increment merchant counter */
	game->numMerchants++;
}

// Card: Vassal
// Type: Action
// Cost: 3
//
//	+2 Coins
//	Discard the top card of your
//	deck. If it's an Action card,
//	you may play it.
void playCardVassal(Game* game)
{
	/* +2 Coins */
	game->coins += 2;

	/* move top card of deck to revealed */
	moveCard(game, game->currPlayer, Locations::deck, Locations::revealed, game->currPlayer->deckSize - 1);

	/* only action cards may be played */
	if (isCardType(game->currPlayer->revealed[game->currPlayer->revealedSize - 1], Types::action))
	{
		/* choices.choices[0] = 1 if player wishes to play action */
		/* choices.choices[0] = 0 if player does not wish to play action */
		Choice choices = getChoices(game, game->currPlayer, Cards::vassal);

		if (choices.choices[0] == 1)
		{
			/* play the card */
			playCard(game, Locations::revealed, game->currPlayer->revealedSize - 1);
		}
		else
		{
			/* dicard the card */
			moveCard(game, game->currPlayer, Locations::revealed, Locations::discard, game->currPlayer->revealedSize - 1);
		}
	}
}

// Card: Village
// Type: Action
// Cost: 3
//
//	+1 Card
//	+2 Actions
void playCardVillage(Game* game)
{
	/* +1 Card */
	drawCard(game->currPlayer);

	/* +2 Actions */
	game->actions += 2;
}

// Card: Workshop
// Type: Action
// Cost: 3
//
//	Gain a card costing up to 4 Coins.
void playCardWorkshop(Game* game)
{
	/* choices.choices[0] = enum of card to gain */
	/* reminder: if a card can be gained, it must be gained (validate in getChoices()) */
	Choice choices = getChoices(game, game->currPlayer, Cards::workshop);

	/* gain the card */
	gainCard(game, game->currPlayer, (Cards) choices.choices[0], Locations::discard);
}

// Card: Bureaucrat
// Type: Action - Attack
// Cost: 4
//
//	Gain a Silver onto your
//	deck. Each other player
//	reveals a Victory card from
//	their hand and puts it onto
//	their deck (or reveals a hand
//	with no Victory cards).
void playCardBureaucrat(Game* game)
{
	/* gain a silver onto player's deck */
	gainCard(game, game->currPlayer, Cards::silver, Locations::deck);

	/* resolve (now is the time to play moat) */

	for (int i = (game->turn + 1) % game->numPlayers; i != game->turn; i = (i + 1) % game->numPlayers)
	{
		/* skip moat-protected players */
		if (game->players[i].moatFlag)
			continue;

		/* each other player reveals a victory card and puts it onto their deck ... */
		for (int j = 0; j < game->players[i].handSize; j++)
		{
			if (isCardType(game->players[i].hand[j], Types::victory))
			{
				/* choices.choices[0] = index of victory card to reveal */
				Choice choices = getChoices(game, &game->players[i], Cards::bureaucrat);
				moveCard(game, &game->players[i], Locations::hand, Locations::revealed, choices.choices[0]);

				/* resolve (allow card to be revealed) */

				/* place victory card onto player's deck */
				moveCard(game, &game->players[i], Locations::revealed, Locations::deck, game->players[i].revealedSize - 1);
			}
			else
			{
				/* or reveals a hand with no victory cards */
				while (game->players[i].handSize > 0)
					moveCard(game, &game->players[i], Locations::hand, Locations::revealed, 0);
			
				/* resolve (allow hand to be revealed) */

				/* return cards to player's hand (assumption: all cards in revealed were in hand) */
				while (game->players[i].revealedSize > 0)
					moveCard(game, &game->players[i], Locations::revealed, Locations::hand, 0);
			}
		}
	}
}

// Card: Militia
// Type: Action - Attack
// Cost: 4
//
//	+2 Coins
//	Each other player discards
//	down to 3 cards in their hand.
void playCardMilitia(Game* game)
{
	/* +2 Coins */
	game->coins += 2;

	/* resolve (now is the time to play moat) */

	/* each other player discards down to 3 cards in their hand */
	for (int i = (game->turn + 1) % game->numPlayers; i != game->turn; i = (i + 1) % game->numPlayers)
	{
		/* skip moat-protected players */
		if (game->players[i].moatFlag)
			continue;

		/* choices.choices[n] = index of card to discard in ascending order */
		/* choices.numChoices = number of cards to discard */
		Choice choices = getChoices(game, &game->players[i], Cards::militia);

		/* discard each card, descending from the highest index for the sake of hand shifting */
		for (int j = choices.numChoices; j > 0; j--)
			moveCard(game, &game->players[i], Locations::hand, Locations::discard, choices.choices[j - 1]);
	}
}

// Card: Moneylender
// Type: Action
// Cost: 4
//
//	You may trash a Copper
//	from your hand for +3 Coins.
void playCardMoneylender(Game* game)
{
	/* choices.choices[0] = -1 if player cannot or chooses not to trash Copper */
	/* choices.choices[0] = index of Copper to trash otherwise */
	Choice choices = getChoices(game, game->currPlayer, Cards::moneylender);

	if (choices.choices[0] > -1)
	{
		/* trash Copper from hand */
		moveCard(game, game->currPlayer, Locations::hand, Locations::trash, choices.choices[0]);

		/* +3 Coins */
		game->coins += 3;
	}
}

// Card: Poacher
// Type: Action
// Cost: 4
//
//	+1 Card
//	+1 Action
//	+1 Coin
//
//	Discard a card per empty
//	Supply pile.
void playCardPoacher(Game* game)
{
	/* +1 Card */
	drawCard(game->currPlayer);

	/* +1 Action */
	game->actions += 1;

	/* +1 Coin */
	game->coins += 1;

	if (getNumEmptySupplyPiles(game) > 0)
	{
		/* choices.choices[n] = index of card to discard in ascending order */
		/* choices.numChoices = number of cards to discard (number of empty supply piles, usually) */
		/* see new function: getNumEmptySupplyPiles() */
		/* remember outlier: player may not have enough cards to discard */
		Choice choices = getChoices(game, game->currPlayer, Cards::poacher);

		/* discard a card per empty supply pile */
		for (int i = choices.numChoices; i > 0; i--)
			moveCard(game, game->currPlayer, Locations::hand, Locations::discard, choices.choices[i - 1]);
	}
}

// Card: Remodel
// Type: Action
// Cost: 4
//
//	Trash a card from your hand.
//	Gain a card costing up to
//	2 Coins more than it.
void playCardRemodel(Game* game)
{
	/* choices.choices[0] = index of card to trash */
	Choice choices = getChoices(game, game->currPlayer, Cards::remodel, 0);

	/* trash card from hand */
	moveCard(game, game->currPlayer, Locations::hand, Locations::trash, choices.choices[0]);

	/* choices.choices[0] = -1 if no card costing 2 coins more can be gained */
	/* choices.choices[0] = enum of card to gain otherwise */
	if (choices.choices[0] > -1)
		gainCard(game, game->currPlayer, (Cards) choices.choices[0], Locations::discard);
}

// Card: Smithy
// Type: Action
// Cost: 4
//
//	+3 Cards
void playCardSmithy(Game* game)
{
	/* +3 Cards */
	drawCard(game->currPlayer);
	drawCard(game->currPlayer);
	drawCard(game->currPlayer);
}

// Card: Throne Room
// Type: Action
// Cost: 4
//
//	You may play an Action
//	card from your hand twice.
void playCardThroneRoom(Game* game)
{
	/* this code currently supports the interpretation that throne room provides 2 outcomes: */
	/*		1: a chosen action card is played once, then resolved again */
	/*		2: no action card is played at all until throne room is resolved */

	/* choices.choices[0] = -1 if player cannot or chooses not to play action card */
	/* choices.choices[0] = index of action card to play twice */
	Choice choices = getChoices(game, game->currPlayer, Cards::throne_room);

	if (choices.choices[0] > -1)
	{
		/* play action card from hand */
		playCard(game, Locations::hand, choices.choices[0]);

		/* play action card from played (a.k.a. twice!) */
		playCard(game, Locations::played, game->currPlayer->playedSize - 1);
	}
}

// Card: Bandit
// Type: Action - Attack
// Cost: 5
//
//	Gain a Gold. Each other
//	player reveals the top 2
//	cards of their deck, trashes
//	a revealed Treasure other
//	than Copper, and discards
//	the rest.
void playCardBandit(Game* game)
{
	/* gain a gold */
	gainCard(game, game->currPlayer, Cards::gold, Locations::discard);

	/* resolve (now is the time to play moat) */

	for (int i = (game->turn + 1) % game->numPlayers; i != game->turn; i = (i + 1) % game->numPlayers)
	{
		/* skip moat-protected players */
		if (game->players[i].moatFlag)
			continue;

		/* reveal the top 2 cards of the player's deck */
		moveCard(game, &game->players[i], Locations::deck, Locations::revealed, game->players[i].deckSize - 1);
		moveCard(game, &game->players[i], Locations::deck, Locations::revealed, game->players[i].deckSize - 1);

		/* skip resolve if there is nothing to resolve - choice is already made */
		int numNonCopperTreasures = 0;
		for (int i = 0; i < 2; i++)
			if (isCardType(game->players[i].revealed[0], Types::treasure)
				&& game->players[i].revealed[0] != Cards::copper)
				numNonCopperTreasures++;

		if (numNonCopperTreasures > 1)
		{
			/* choices.choices[0] = index of revealed card to trash */
			Choice choices = getChoices(game, game->currPlayer, Cards::bandit);

			/* trash non-copper treasure */
			moveCard(game, &game->players[i], Locations::revealed, Locations::trash, choices.choices[0]);
		}
		else
		{
			/* trash any non-copper treasures */
			for (int i = 0; i < 2; i++)
				if (isCardType(game->players[i].revealed[0], Types::treasure)
					&& game->players[i].revealed[0] != Cards::copper)
					moveCard(game, &game->players[i], Locations::revealed, Locations::trash, i);
		}

		/* move remaining revealed cards to discard pile */
		while (game->players[i].revealedSize > 0)
			moveCard(game, &game->players[i], Locations::revealed, Locations::discard, game->players[i].revealedSize - 1);
	}
}

// Card: Council Room
// Type: Action
// Cost: 5
//
//	+4 Cards
//	+1 Buy
//
//	Each other player draws a
//	card.
void playCardCouncilRoom(Game* game)
{
	/* +4 Cards */
	for (int i = 0; i < 4; i++)
		drawCard(game->currPlayer);

	/* +1 Buy */
	game->buys += 1;

	/* each other player draws a card */
	for (int i = 0; i < game->numPlayers; i++)
	{
		/* skip the current player */
		if (&game->players[i] == game->currPlayer)
			continue;

		drawCard(&game->players[i]);
	}
}

// Card: Festival
// Type: Action
// Cost: 5
//
//	+2 Actions
//	+1 Buy
//	+2 Coins
void playCardFestival(Game* game)
{
	/* +2 Actions */
	game->actions += 2;

	/* +1 Buy */
	game->buys += 1;

	/* +2 Coins */
	game->coins += 2;
}

// Card: Laboratory
// Type: Action
// Cost: 5
//
//	+2 Cards
//	+1 Action
void playCardLaboratory(Game* game)
{
	/* +2 Cards */
	drawCard(game->currPlayer);
	drawCard(game->currPlayer);

	/* +1 Action */
	game->actions += 1;
}

// Card: Library
// Type: Action
// Cost: 5
//
//	Draw until you have 7 cards
//	in hand, skipping any Action
//	cards you choose to; set
//	those aside, discarding them
//	afterwards.
void playCardLibrary(Game* game)
{
	// note: current implementation will not automatically skip non-action cards
	//		due to potentially revealing too much information to other players
	while (game->currPlayer->handSize < 7)
	{
		/* if player cannot draw anymore, break */
		if (moveCard(game, game->currPlayer, Locations::deck, Locations::peeked, game->currPlayer->deckSize - 1) == 0)
			break;

		/* choices.choices[0] = 1 if player wishes to KEEP peeked action card */
		/* choices.choices[0] = 0 if player wishes to put aside (reveal) peeked action card */
		Choice choices = getChoices(game, game->currPlayer, Cards::library);

		if (choices.choices[0] == 1)
		{
			/* keep card; move from peeked to hand */
			moveCard(game, game->currPlayer, Locations::peeked, Locations::hand, game->currPlayer->peekedSize - 1);
		}
		else
		{
			/* put aside card; move from peeked to revealed */
			moveCard(game, game->currPlayer, Locations::peeked, Locations::revealed, game->currPlayer->peekedSize - 1);
		}
	}

	/* place all put aside (revealed) action cards into discard pile */
	/* this assumes that all cards in played pile need to be discarded */
	movePile(game, game->currPlayer, Locations::revealed, Locations::discard);
}

// Card: Market
// Type: Action
// Cost: 5
//
//	+1 Card
//	+1 Action
//	+1 Buy
//	+1 Coin
void playCardMarket(Game* game)
{
	/* +1 Card */
	drawCard(game->currPlayer);

	/* +1 Action */
	game->actions += 1;

	/* +1 Buy */
	game->buys += 1;

	/* +1 Coin */
	game->coins += 1;
}

// Card: Mine
// Type: Action
// Cost: 5
//
//	You may trash a Treasure
//	from your hand. Gain a
//	Treasure to your hand
//	costing up to 3 Coins more
//	than it.
void playCardMine(Game* game)
{
	/* current implementation assumes that once the player has trashed a treasure, */
	/*	they must gain a treasure to hand; because game ends at 3 empty supply piles, */
	/*	there is no need to check if player cannot gain a treasure */

	/* choices.choices[0] = -1 if player cannot or chooses not to trash treasure from hand */
	/* choices.choices[0] = index of treasure to trash from hand otherwise */
	Choice choices = getChoices(game, game->currPlayer, Cards::mine, 0);

	if (choices.choices[0] > -1)
	{
		/* trash treasure from hand */
		Cards trashedCard = (Cards)choices.choices[0];
		moveCard(game, game->currPlayer, Locations::hand, Locations::trash, choices.choices[0]);

		while (true)
		{
			/* choices.choices[0] = enum of treasure to gain from supply */
			choices = getChoices(game, game->currPlayer, Cards::mine, 1);

			if (getCardCost((Cards) choices.choices[0]) > getCardCost(trashedCard))
				fprintf(stderr, "WARNING: playCardMine(): Cannot gain %s, it costs too much more than %s.\n",
					getCardName((Cards) choices.choices[0]), getCardName(trashedCard));
			else
			{
				/* gain card to hand */
				gainCard(game, game->currPlayer, (Cards) choices.choices[0], Locations::hand);
				break;
			}
		}
	}
}

// Card: Sentry
// Type: Action
// Cost: 5
//
//	+1 Card
//	+1 Action
//
//	Look at the top 2 cards of
//	your deck. Trash and/or
//	discard any number of them.
//	Put the rest back on top in
//	any order.
void playCardSentry(Game* game)
{
	/* +1 Card */
	drawCard(game->currPlayer);

	/* +1 Action */
	game->actions += 1;

	/* peek at top two cards of deck */
	/* assumption: peeked pile is only a result of sentry */
	moveCard(game, game->currPlayer, Locations::deck, Locations::peeked, game->currPlayer->deckSize - 1);
	moveCard(game, game->currPlayer, Locations::deck, Locations::peeked, game->currPlayer->deckSize - 1);

	/* reminder: peeked size may be 0, 1, or 2*/
	
	while (game->currPlayer->peekedSize > 0)
	{
		/* choices.choices[0] = index of card to place in a pile */
		/* choices.numChoices = enum of location where card should be placed (deck, discard, trash) */
		Choice choices = getChoices(game, game->currPlayer, Cards::sentry);

		switch ((Locations)choices.numChoices)
		{
		case Locations::deck:
			moveCard(game, game->currPlayer, Locations::peeked, Locations::deck, choices.choices[0]);
			break;
		case Locations::discard:
			moveCard(game, game->currPlayer, Locations::peeked, Locations::discard, choices.choices[0]);
			break;
		case Locations::trash:
			moveCard(game, game->currPlayer, Locations::peeked, Locations::trash, choices.choices[0]);
			break;
		}
	}
}

// Card: Witch
// Type: Action - Attack
// Cost: 5
//
//	+2 Cards
//
//	Each other player gains a
//	Curse.
void playCardWitch(Game* game)
{
	/* +2 Cards */
	drawCard(game->currPlayer);
	drawCard(game->currPlayer);

	/* resolve (now is the time to play moat) */

	for (int i = (game->turn + 1) % game->numPlayers; i != game->turn; i = (i + 1) % game->numPlayers)
	{
		/* skip moat-protected players */
		if (game->players[i].moatFlag)
			continue;

		/* each other player gains a curse in turn order */
		gainCard(game, &game->players[i], Cards::curse, Locations::discard);
	}
}

// Card: Artisan
// Type: Action
// Cost: 6
//
//	Gain a card to your hand
//	costing up to 5 Coins.
//
//	Put a card from your hand
//	onto your deck.
void playCardArtisan(Game* game)
{
	/* gain a card to hand costing up to 5 coins */
	/* choices.choices[0] = enum of card to gain */
	Choice choices = getChoices(game, game->currPlayer, Cards::artisan, 0);
	gainCard(game, game->currPlayer, (Cards) choices.choices[0], Locations::hand);

	/* put a card from hand onto deck */
	if (game->currPlayer->handSize > 0)
	{
		/* choices.choices[0] = index of card to move from hand to deck */
		Choice choices = getChoices(game, game->currPlayer, Cards::artisan, 1);
		moveCard(game, game->currPlayer, Locations::hand, Locations::deck, choices.choices[0]);
	}
}

// Card: Copper
// Type: Treasure
// Cost: 0
//
//	1 Coin
void playCardCopper(Game* game)
{
	/* 1 Coin */
	game->coins += 1;
}

// Card: Silver
// Type: Treasure
// Cost: 3
//
//	2 Coins
void playCardSilver(Game* game)
{
	/* 2 Coins */
	game->coins += 2;

	/* find the number of silvers in the played pile */
	int numSilvers = 0;
	for (int i = 0; i < game->currPlayer->playedSize; i++)
	{
		if (game->currPlayer->played[i] == Cards::silver)
			numSilvers++;
	}

	/* if first silver played, add merchant bonus */
	if (numSilvers == 1)
		game->coins += game->numMerchants;
}

// Card: Gold
// Type: Treasure
// Cost: 6
//
// 3 Coins
void playCardGold(Game* game)
{
	/* 3 Coins */
	game->coins += 3;
}

int main()
{
	srand((unsigned int) time(NULL));

	Set* mySet = newRecommendedSet(Sets::first_game);
	printSet(stdout, mySet);

	Game* myGame = newGame(4, mySet);

	for (int i = 0; i < myGame->numPlayers; i++)
	{
		fprintf(stderr, "Deck of player %d:\n", i);
		printDeck(stdout, &myGame->players[i]);
	}

	endTurn(myGame);

	/* each other player gains a curse in turn order */
	fprintf(stderr, "I, player %d, will make you all say hello!\n", myGame->turn);
	for (int i = (myGame->turn + 1) % myGame->numPlayers; i != myGame->turn; i = (i + 1) % myGame->numPlayers)
	{
		fprintf(stderr, "Greetings from player %d!\n", i);
	}

	return 0;
}