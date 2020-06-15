#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <ctime>
#include <string>

const int	MAX_NUM_HAND_CARDS = 100;
const int	MAX_NUM_DECK_CARDS = 100;
const int	MAX_NUM_DISCARD_CARDS = 100;
const int	MAX_NUM_PLAYED_CARDS = 100;
const int	MAX_NUM_PEEKED_CARDS = 100;
const int	MAX_NUM_REVEALED_CARDS = 100;
const int	MAX_NUM_SUPPLY_CARDS = 1000; // number of unique cards in the game, see (int) Cards::NUM_CARDS

const int	MAX_NUM_TYPES = 3;
const int	MAX_NUM_TYPE_CHARS = 100;

const int	MAX_NUM_CHOICES = MAX_NUM_HAND_CARDS + 1; // action cards like cellar mandate this

const int	MIN_NUM_KINGDOM_CARDS = 10;
const int	MAX_NUM_KINGDOM_CARDS = 10; // in a set, subject to change based off expansions
const int	MAX_NUM_TRASH_CARDS = 100;

const int	MIN_NUM_PLAYERS = 2;
const int	MAX_NUM_PLAYERS = 4;


enum class Cards
{
	/* DOMINION EXPANSION */
	/* basic supply cards */

	// curse
	curse,
	
	// treasures
	copper,
	silver,
	gold,
	
	// victory cards
	estate,
	duchy,
	province,

	/* kingdom cards */

	// 2-cost
	cellar,
	chapel,
	moat,

	// 3-cost
	harbringer,
	merchant,
	vassal,
	village,
	workshop,
	
	// 4-cost
	bureaucrat,
	gardens,
	militia,
	moneylender,
	poacher,
	remodel,
	smithy,
	throne_room,

	// 5-cost
	bandit,
	council_room,
	festival,
	laboratory,
	library,
	market,
	mine,
	sentry,
	witch,

	// 6-cost
	artisan,

	/* number of cards */
	NUM_CARDS
};

enum class Expansions
{
	dominion,
	intrigue,
	seaside,
	alchemy,
	propserity,
	cornucopia,
	hinterlands,
	dark_ages,
	guilds,
	adventures,
	empires,
	nocturne,
	renaissance,
	promo,

	/* number of expansions */
	NUM_EXPANSIONS
};

enum class Locations
{
	hand,
	deck,
	discard,
	played,
	peeked,
	revealed,
	supply,
	trash,

	/* number of locations */
	NUM_LOCATIONS
};

enum class Phases
{
	action,
	buy,
	clean_up,

	/* number of phases */
	NUM_PHASES
};

enum class Sets
{
	first_game,
	size_distortion,
	deck_top,
	sleight_of_hand,
	improvements,
	silver_and_gold,

	/* number of sets */
	NUM_SETS
};

enum class Types
{
	action,
	treasure,
	victory,
	curse,

	attack,
	duration,
	reaction,
	command,
	
	castle,
	doom,
	fate,
	gathering,
	heirloom,
	knight,
	looter,
	night,
	prize,
	reserve,
	ruins,
	shelter,
	spirit,
	traveller,
	zombie,

	/* "non-card types" */
	artifact,
	boon,
	event,
	hex,
	landmark,
	project,
	state,

	/* number of types */
	NUM_TYPES
};

/* ***** structs ***** */

struct Choice
{
	int*	choices = NULL;
	int		numChoices = 0;
};

struct Player
{
	int		id = 0;

	/* counters */
	int		victoryPoints	= 0; // see cards like Monument that allot victory tokens
	int		score			= 0; // for determining the winner, can change with getScore() through end of game
	int		numTurnsTaken	= 0; // incremented at turn start in case abrupt game over

	/* flags */
	bool	moatFlag = false;

	/* piles */
	Cards	hand[MAX_NUM_HAND_CARDS] = { Cards::NUM_CARDS };
	int		handSize = 0;
	Cards	deck[MAX_NUM_DECK_CARDS] = { Cards::NUM_CARDS };
	int		deckSize = 0;
	Cards	discard[MAX_NUM_DISCARD_CARDS] = { Cards::NUM_CARDS };
	int		discardSize = 0;
	Cards	played[MAX_NUM_PLAYED_CARDS] = { Cards::NUM_CARDS };
	int		playedSize = 0;
	Cards	peeked[MAX_NUM_PEEKED_CARDS] = { Cards::NUM_CARDS };
	int		peekedSize = 0;
	Cards	revealed[MAX_NUM_REVEALED_CARDS] = { Cards::NUM_CARDS };
	int		revealedSize = 0;
};

struct Set
{
	const char* name = NULL;
	Cards*		kingdomCards = NULL;
	int			numKingdomCards = 0;
};

struct Game
{
	/* players */
	Player* players = NULL;
	Player* currPlayer = NULL;
	Player* nextPlayer = NULL;
	Player* prevPlayer = NULL;
	int		numPlayers = 0;

	/* turns */
	int		turn = 0;
	Phases	phase = Phases::action; /* TODO: phase change */
	int		actions = 0;
	int		coins = 0;
	int		buys = 0;

	/* flags and counters */
	int		numMerchants = 0;

	/* piles */
	Set*	set = NULL;
	int		supply[MAX_NUM_SUPPLY_CARDS] = { 0 }; // number of unique cards in the game
	Cards	trash[MAX_NUM_TRASH_CARDS] { Cards::NUM_CARDS };
	int		trashSize = 0;
};

struct Type
{
	Types	type[MAX_NUM_TYPES] = { Types::NUM_TYPES };
	int		numTypes = 0;
};

struct Winners
{
	Player	winners[MAX_NUM_PLAYERS];
	int		numWinners = 1;
	int		score = 0;
};

/* constructors */
Game*		newGame(int desiredNumPlayers, Set* desiredSet);
Set*		newRecommendedSet(Sets set);
Set*		newRandomSet();
Set*		newCustomSet(Cards desiredKingdomCards[MAX_NUM_KINGDOM_CARDS], int desiredNumKingdomCards);
/* name getters */
const char* getCardName(Cards card);
const char* getCardTypeName(Cards card);
const char* getLocationName(Locations location);
const char* getSetName(Sets set);
const char* getPhaseName(Phases phase);
const char* getTypeName(Types type);
/* card interaction */
int			getCardCoin(Cards card);
int			getCardCost(Cards card);
Type		getCardType(Cards card);
bool		isCardType(Cards card, Types type);
int			getCardVictoryPoints(Player* player, Cards card);
/* pile (hand, deck, discard, etc.) interaction */
bool		drawCard(Player* player);
bool		buyCard(Game* game, Cards card);
int			gainCard(Game* game, Player* player, Cards card, Locations destination);
int			moveCard(Game* game, Player* player, Locations from, Locations to, int position);
int			movePile(Game* game, Player* player, Locations source, Locations destination);
void		shuffleDeck(Player* player);
Cards*		getPile(Game* game, Player* player, Locations location);
int*		getPileSize(Game* game, Player* player, Locations location);
bool		shiftPile(Cards* pile, int* pileSize, int position);
int			getFirstCardInPile(Game* game, Player* player, Cards card, Locations location);
int			getLastCardInPile(Game* game, Player* player, Cards card, Locations location);
/* turn interaction */
void		advancePhase(Game* game);
void		endTurn(Game* game);
/* score and end game*/
int			getNumEmptySupplyPiles(Game* game);
bool		isEndOfGame(Game* game);
int			getPlayerScore(Player* player);
void		setScores(Game* game);
Winners		getWinners(Game* game);
/* printers */
void		printCardType(FILE* const stream, Cards card);
void		printDeck(FILE* const stream, Player* player);
void		printSet(FILE* const stream, Set* set);
void		printType(FILE* const stream, Types type);

/* card effects */
Choice		getChoices(Game* game, Player* player, Cards card);
Choice		getChoices(Game* game, Player* player, Cards card, int index);

bool		playCard(Game* game, Locations from, int position);
bool		reactionCard(Game* game, Player* player, Locations source, int position);

void		playCardCellar(Game* game);
void		playCardChapel(Game* game);
void		playCardMoat(Game* game, Player* player, bool trigger);
void		playCardHarbringer(Game* game);
void		playCardMerchant(Game* game);
void		playCardVassal(Game* game);
void		playCardVillage(Game* game);
void		playCardWorkshop(Game* game);
void		playCardBureaucrat(Game* game);
void		playCardMilitia(Game* game);
void		playCardMoneylender(Game* game);
void		playCardPoacher(Game* game);
void		playCardRemodel(Game* game);
void		playCardSmithy(Game* game);
void		playCardThroneRoom(Game* game);
void		playCardBandit(Game* game);
void		playCardCouncilRoom(Game* game);
void		playCardFestival(Game* game);
void		playCardLaboratory(Game* game);
void		playCardLibrary(Game* game);
void		playCardMarket(Game* game);
void		playCardMine(Game* game);
void		playCardSentry(Game* game);
void		playCardWitch(Game* game);
void		playCardArtisan(Game* game);

void		playCardCopper(Game* game);
void		playCardSilver(Game* game);
void		playCardGold(Game* game);

/* TODO: sort function for choices to put returned indices in ascending order */
/* TODO: moveCard() must allow function call when source pile is empty, and re-fill deck */
/* example: reveal top two cards of an empty deck */