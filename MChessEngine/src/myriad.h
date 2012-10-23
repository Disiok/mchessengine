/*
 * myriad.h
 * ============================================
 * (c) Spark Team, Oct 2012.
 * The Spark Team reserves all intellectual rights to the following source code.
 * The code may not be distributed or modified for personal use, except with the
 * express permission of a team member.
 * ============================================
 * Contains all the essential declarations for use in the Myriad engine.
 */
#ifndef RULES_H
#define RULES_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <algorithm>
#include <cstdlib>
#include <cmath>

using namespace std;

namespace myriad {

// ======================Typedefs=====================
/* Past .rules class redefinitions.
 * Note: object versions replaced with bit-strings.*/
typedef unsigned short _piece;		/* piece format(LSB->MSB): 8 bits for location, 3 for type,
									   	   1 for colour */
typedef unsigned int _property;		/* connotes a descriptor of any kind. */
typedef unsigned int _move;			/* move format(LSB->MSB): 8 bits for start, 8 bits for end,
									   	   remaining bits for descriptor. */
typedef unsigned char _location;		/* standard 0x88 location square. */
typedef unsigned long _zobrist;		/* Zobrist hash key */
typedef unsigned long _zdata;			/* Data to store into Zobrist. Format (LSB-> MSB) 1 bit for exact,
 	 	 	 	 	 	 	 	 	   	   1 bit for bound, 15 bits for score, 1 bit for sign of score,
 	 	 	 	 	 	 	 	 	   	   remaining bits for cutoff move.*/
typedef signed short _score;			/* The score of a position. */

struct _stackel{						/* An stack element for hay. */
	_zobrist prev_hash;
	_move edge;							/* An edge connects two nodes, a move connects two positions. */
	_property detail;					/* Restoration of castling rights, etc. */
	_score score;
	unsigned short fullmove_clock;
};
struct _time{							/* Andy, tell me what you need to implement ID with. */
	long time_remaining;
	int depth;
};
// ======================End of Typedefs=====================

// ======================Constants=====================
/* Piece constants */
const _property PAWN = 1;
const _property KNIGHT = 2;
const _property BISHOP = 3;
const _property ROOK = 4;
const _property QUEEN = 5;
const _property KING = 6;
const _property WHITE = 0;	/* NB: previously defined as WHITE = 1, BLACK = -1 use colour ^ 1 */
const _property BLACK = 1;	/* or !colour to get the other colour, not multiply by -1 */

/* Move constants */
const _property WKS_CASTLE = 1;
const _property WQS_CASTLE = 2;
const _property BKS_CASTLE = 3;
const _property BQS_CASTLE = 4;
const _property EN_PASSANT = 5;
const _property DOUBLE_ADVANCE = 0x20;
const _property PROMOTE_OFFSET = 4; 	/* use PROMOTION_OFFSET + piece type to obtain modifier */

/* Bit manipulation special values. */
/* ----------------------------------------------
 * Note on bitshift values: the values do not explain the code that it is used in.
 * Here are common uses for mask values.
 * Last 3 bits: Detecting the file of a location value, or retrieving the piece type.
 * Last 4 bits: Most small data can be fit into 4 bits.
 * Last 8 bits: To retrieve a 0x88 square.
 */
const int FOUR_SH = 4;
const int EIGHT_SH = 8;
const int SIXTEEN_SH = 16;
const int COLOR_SH = 11;		/* used for piece colour bit */
const int EP_SH = 12; 			/* used for en passant square in _details variable */
const int LOCATION_MASK = 0xff;
const int NIBBLE_MASK = 0xf;
const int TRIPLET_MASK = 0x7;	/* used for the file of any _location */

/* 0x88 Differences */
const char UP = 0x10;
const char DOWN = -0x10;
const char RIGHT = 0x01;
const char LEFT = -0x01;
const char UP_RIGHT = 0x11;
const char UP_LEFT = 0x0f;
const char DOWN_RIGHT = -0x0f;
const char DOWN_LEFT = -0x11;
const char RADIAL [] = {UP, DOWN, LEFT, RIGHT, UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT};
const char LINEAR [] = {UP, DOWN, LEFT, RIGHT};
const char DIAGONAL [] = {UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT};
const char KNIGHT_MOVE [] = {0x21, 0x1f, -0x1f, -0x21, 0x12, -0xe, 0xe, -0x12};
const char WHITE_PAWN_ATTACK[] = {UP_RIGHT, UP_LEFT};
const char BLACK_PAWN_ATTACK[] = {DOWN_RIGHT, DOWN_LEFT};

/* Default Values  and Miscellany */
const _property start_position = 0xf00;		/* detail value at start position */
extern _piece zero_piece; 					/* WARNING: g++ will not allow this to be declared const,
 	 	 	 	 	 	 	 	 	 	 	 * but DO NOT CHANGE this value. */
const int MAX_PLY_COUNT = 300;				/* Maximum ply count. */
const signed short UNKNOWN_SCORE = 20001;
const signed short POS_INFINITY = 20000;
const signed short NEG_INFINITY = -20000;
const _move NULL_MOVE = 0;

/* Zobrist hash property values */
const int multiplier [] = {0, 2, 4, 6, 8, 10};
const int CASTLE_HASH_OFFSET = 832;
const int EPSQ_HASH_OFFSET = 768;
const int STM_INDEX = 848;
extern long xor_values [849];				/* The value to xor in for each property on the board, i.e., knight on f3 */
// ======================End of Constants======================

// ======================Classes======================
class position{
public:
	_piece* board[0x78];
	_piece white_map[16];
	_piece black_map[16];
	_property details;		/* LSB->MSB, 1 bit for stm, 7 bits for plycount, 4 bits for cstl. rights.,
							   8 bits for position of pawn capturable by en passant */
	_zobrist hash_key;
	unsigned short fullmove_clock;

	position();
	void fromFen(string fen);

	bool is_in_check ();
	void make_move(_move);
	_piece& piece_search(_location);						/* When search map is unknown */
	_piece& piece_search(_location, _property);			/* WHITE for white, BLACK for black */
	vector<_move>* move_gen();
	void unmake_move (_move previous_move, _property prev_details, _zobrist prev_hash);
	operator string ();									/* Converts to FEN String */
	string display_board();								/* Converts to a string for use in console gfx */
	bool operator<(const position& rhs) const { return fullmove_clock < rhs.fullmove_clock; };
	bool operator==(const position& rhs) const{	return hash_key == rhs.hash_key;	}
	_piece& operator[](_location l) { 	return piece_search(l); }
private:
	inline void continuous_gen (_property, _location, vector<_move> &, _property, char);
	inline _piece* create_guardian_map (_property, _property);
	inline char get_difference(_location, _location);
	inline int get_last_index(_piece*);
	inline void kill(_piece& victim, _property map);
	inline void king_gen(_location, _piece &, vector <_move> &, _property, char);
	inline _piece get_assailant (_piece*, _piece);
	inline void pawn_capture_reach (vector <_piece> &, _location, _property);
	inline vector<_piece> reachable_pieces(_location, _property);
	inline void single_gen (_property, _location, vector<_move> &, _property, char);
};
class round {
public:
	static const int KILLER_SIZE = 2;
	round (int bit_size);
	~round();
	_move killer_moves[KILLER_SIZE];

	long get(_zobrist);
	bool set(_zobrist, short, short, bool, bool, _move);
private:
	int MASK_INDEX;
	_zdata *information;
	_zobrist *hashes;
	short *depth;
};
class hay {
public:
	_stackel variation [MAX_PLY_COUNT];
	int ind_top;
	position &current;

	hay (position &);

	inline _stackel get (int);
	inline void parent_score(_score sc);
	_stackel pop ();
	void push (_move);
	bool repetition ();
	inline _stackel top ();
};
// ======================End of Classes======================

// ======================Functions======================
/* utility / debug functions */
string piece_to_string (_piece p);
string move_to_string (_move m, position &p);
string piecetype_to_string (_property type);
string piecetype_to_string(_property type, _property col);
inline string location_to_string (_location sq){
	stringstream ss;
	ss << (1 + (sq>> FOUR_SH));
	return string() + (char)('a' + (sq & TRIPLET_MASK)) + ss.str();
}
inline _location string_to_location(string location){	return (((location[1] - '1') << 4) + (location[0] - 'a'));}
inline _location x88to64(_location loc){	return (loc >> FOUR_SH) * 8 + (loc & NIBBLE_MASK);	}
inline _location d64tox88(_location loc_64){	return ((loc_64 / 10) << FOUR_SH) + (loc_64 % 10);	}
/* Generates a 64 bit unsigned long random number. */
inline unsigned long rand64 (){
	unsigned long rand_16 = rand() & 0xffff, result;
	for (int i = 0; i < 4; i++)	 result = (result << 16) + rand_16;
	return result;
}

/* eval function(s) */
_score eval (position &p);

/* _piece accessors and mutators */
inline _location get_piece_location(_piece p){ return p & LOCATION_MASK; }
inline _property get_piece_color(_piece p){ 	return p >> COLOR_SH; }
inline _property get_piece_type(_piece p){	return (p >> EIGHT_SH) & TRIPLET_MASK;	}
inline void move_piece(_piece& p, _location start, _location end, _piece** board){
	p = (p ^ start) ^ end;
	assert(start <= 0x77);
	assert(end <= 0x77);
	board[start] = &zero_piece;
	board[end] = &p;
}
inline _piece create_piece (_location loc, _property type, _property color)
	{ return (color << COLOR_SH) + (type << EIGHT_SH) + loc; }

/* _move accessors and mutators */
inline _location get_move_start(_move m){ return m & LOCATION_MASK; }
inline _location get_move_end (_move m) { return (m >> EIGHT_SH) & LOCATION_MASK; }
inline _property get_move_modifier (_move m) { return m >> SIXTEEN_SH; }
inline _move create_move (_location start, _location end, _property modifier = 0)
	{	return (modifier << SIXTEEN_SH) + (end << EIGHT_SH) + start; }
/* Create modifiers for capture moves. Bitstring formatted: (MSB-> LSB) promotion piece type,
 * victim piece type, attacker piece type, 4 bits each. (NB: 4 bits now)
 * Organized this way so direct comparison i.e. _move a > _move b is possible in move-ordering */
inline _property create_capture_mod (_property attacker, _property victim, _property promote = 0)
	{	return (promote << EIGHT_SH) + (victim << FOUR_SH) + (attacker); }

/* Zobrist hash manipulators */
inline int get_index(_location loc, _property type, _property color){
/* Note: type - 1 since piece types start at 1, only useful for ordinary piece moves. */
	assert(type - 1 >= 0 && type - 1 < 6);
	return 64 * (multiplier[type - 1] + (color)) + x88to64(loc);
}
_zobrist create_initial_hash (position &);
inline _zobrist xor_in_out (_zobrist old, _location prev_loc, _location new_loc, _property col, _property type)
	{	return old^xor_values[get_index(prev_loc, type, col)]^xor_values[get_index(new_loc, type, col)];	}
inline _zobrist xor_out (_zobrist old, _location prev_loc, _property col, _property type)
	{	return old^xor_values[get_index(prev_loc, type, col)];	}
inline _zobrist xor_castling (_zobrist old, _property castle_nibble, _property prev_rights){
/* castle_nibble = 4 bits representing each of the castle rights. */
	assert(CASTLE_HASH_OFFSET + prev_rights < 849 && CASTLE_HASH_OFFSET + prev_rights >= 0);
	assert(CASTLE_HASH_OFFSET + castle_nibble < 849 && CASTLE_HASH_OFFSET + castle_nibble >= 0);
	return old^xor_values[CASTLE_HASH_OFFSET + prev_rights]^xor_values[CASTLE_HASH_OFFSET + castle_nibble];
}
inline _zobrist xor_epsq (_zobrist old, _location prev_epsq, _location new_epsq){
	assert(EPSQ_HASH_OFFSET + x88to64(prev_epsq) < 849 && EPSQ_HASH_OFFSET + x88to64(prev_epsq) >= 0);
	assert(EPSQ_HASH_OFFSET + x88to64(new_epsq) < 849 && EPSQ_HASH_OFFSET + x88to64(new_epsq) >= 0);
	return old^xor_values[EPSQ_HASH_OFFSET + x88to64(prev_epsq)]^xor_values[EPSQ_HASH_OFFSET + x88to64(new_epsq)];
}
inline _zobrist xor_promotion (_zobrist old, _location prev_loc, _location new_loc, _property col, _property promote){
/* Note: promote_to refers to the piece that is being promoted to, not the modifier */
	assert(get_index(prev_loc, PAWN, col) < 849);
	assert(get_index(new_loc, promote, col) < 849);
	return old^xor_values[get_index(prev_loc, PAWN, col)]^xor_values[get_index(new_loc, promote, col)];
}

/* detail _property accessors and mutators */
inline bool is_black_to_move (_property detail) {	return detail & 1;	}
inline _property get_epsq (_property detail) {	return detail >> EP_SH;	}
inline _property clear_epsq (_property detail) {	return detail & 0xfff;	}
inline _property set_epsq (_property detail, _location epsq) /* assuming clear_epsq has been called */
	{	return detail + (epsq << EP_SH);	}
inline _property increase_ply_count (_property detail) {	return (detail ^ 1) + 2;	}
inline _property reset_ply_count (_property detail) {	return detail & (~0xfe);	}
inline int get_plycount(_property detail) {	return (detail >> 1) & 0x7f;	}
inline _property get_castle_right (_property detail, _property modifier)
	{	return (detail >> (modifier + 7)) & 1; }
inline _property get_castle_nibble (_property detail) {	return (detail >> 7) & NIBBLE_MASK;	}
inline _property revoke_castle_right (_property detail, _property modifier)
	{	return detail & (~(1 << (modifier + 7)));	}
// ======================End of Functions======================
}

#endif
