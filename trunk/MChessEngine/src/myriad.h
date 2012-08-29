/*
 * myriad.h
 * ============================================
 * (c) Spark Team, July 2012.
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
#undef RAND_MAX
#define RAND_MAX 4294967295

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
typedef unsigned char _location;	/* standard 0x88 location square. */
typedef unsigned long _zobrist;		/* Zobrist hash key */
typedef unsigned long _zdata;		/* Data to store into Zobrist, XXX: organization unknown */


typedef pair<_move,_property> _u; //"undo pair", used with operator-, operator-=
// ======================End of Typedefs=====================
// ======================Classes======================
class position{
public:
	_piece* board[136];
	_piece white_map[16];
	_piece black_map[16];
	_property details;		/* LSB->MSB, 1 bit for stm, 7 bits for plycount, 4 bits for cstl. rights.,
							   8 bits for position of pawn capturable by en passant */
	unsigned short halfmove_clock;

	position();
	position(string fen);
	//Copy constructor

	virtual ~position() {}

	bool is_in_check ();
	void make_move(_move);
	_piece& piece_search(_location);					/* When search map is unknown */
	_piece& piece_search(_location, _property);			/* WHITE for white, BLACK for black */
	vector<_move>* move_gen();
	void unmake_move (_move previous_move, _property prev_details);

	//creates a fen string
	operator string ();
	//creates a graphical representation
	string display_board();
	bool operator<(const position& rhs) const { return halfmove_clock < rhs.halfmove_clock; };
	bool operator==(const position& rhs) const
	{	// XXX: Replace with Zobrist comparison
		return equal(black_map, black_map + 15*sizeof(_piece), rhs.black_map)
				&& equal(white_map, white_map + 15*sizeof(_piece), rhs.white_map)
				&& (details&1) == (rhs.details&1)
				&& (details >> 8) == (rhs.details >> 8);
				//&& halfmove_clock == rhs.halfmove_clock
				//&& is_in_check() == rhs.is_in_check();
	}
	position& operator+=(const _move& m) {
		this->make_move(m);
		return *this;
	}

	//Call thus: position -= pair<_move, _property>(m,d)
	position& operator-=(const _u& thePair) {
		this->unmake_move(thePair.first, thePair.second);
		return *this;
	}
	position operator+(const _move& m) const {
		position p = *this;
		p += m;
		return p;
	}
	position operator-(const _u& thePair) const {
		position p = *this;
		p -= thePair;
		return p;
	}
	_piece& operator[](_location l) {
		return piece_search(l);
	}

private:
	inline void continuous_gen (_property, _location, vector<_move> &, _property, char);
	inline _piece* create_guardian_map (_property, _property);
	inline char get_difference(_location, _location);
	inline int get_last_index(_piece*);
	inline void kill(_piece& victim, _property map);
	inline void king_gen(_location, _piece &, vector <_move> &, _property, char);
	inline bool is_guardian (_piece*, _piece);
	inline void pawn_capture_reach (vector <_piece> &, _location, _property);
	inline vector<_piece> reachable_pieces(_location, _property);
	inline void single_gen (_property, _location, vector<_move> &, _property, char);
	string get_2d();
};

class Round{
public:
	const int size;
	static const long SCORE_RSH = 23;
	static const long EXACT_RSH = 22;
	static const long BOUND_RSH = 21;
	static const long STARTSQ_RSH = 13;
	static const long ENDSQ_RSH = 5;
	static const long MODIFIER_RSH = 1;
	static const int MASK_BIT = 1;
	static const int MASK_4BIT = 0xf;
	static const int MASK_BYTE = 0xff;
	inline int getSize();
	inline long get(long);
	bool set(long, long, char, bool, bool, _move, bool);
private:
	const int MASK_INDEX;
	vector<long> bitstring_descript;
	vector<long> hashes;
	vector<char> depth;
};

namespace Zobrist{
	/*long hash_values[836];
	int multiplier[7];
	long base_hash;
	int CASTLING_HASHES;
	char EN_PASSENT_ID;*/
	extern long hash_values[836];
	extern long multiplier[7];
	extern long base_hash;
	extern int CASTLING_HASHES;
	extern char EN_PASSANT_ID;
	void init ();
	long createinitialhash (vector<_piece>, vector<_piece>, vector<bool>, char);
	long xorinout (long, char, char, char, char);
	long xorout (long, char, char, char);
	long xorcastling (long, vector<bool>, vector<bool>);
	long xorepsq (long, char, char);
	long xorpromotion (long, char, char, char);
	int getIndex(char, char, char);
};
// ======================End of Classes======================
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

/* Default Values */
const _property start_position = 0xf00;		/* detail value at startposition */
extern _piece zero_piece; 					/* WARNING: g++ will not allow this to be declared const,
 	 	 	 	 	 	 	 	 	 	 	 * but DO NOT CHANGE this value. */
// ======================End of Constants======================
// ======================Functions======================
/* _piece accessors and mutators */
inline _location get_piece_location(_piece p){ return p & LOCATION_MASK; }
inline _property get_piece_color(_piece p){ return p >> COLOR_SH; }
inline _property get_piece_type(_piece p){ return (p >> EIGHT_SH) & TRIPLET_MASK; }
inline void move_piece(_piece& p, _location start, _location end, _piece** board){
	p = (p ^ start) ^ end;
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

/* detail _property accessors and mutators */
inline bool is_black_to_move (_property detail) {	return detail & 1;	}
inline _property get_epsq (_property detail) {	return detail >> EP_SH;	}
inline _property clear_epsq (_property detail) {	return detail & 0xfff;	}
inline _property set_epsq (_property detail, _location epsq) /* assuming clear_epsq has been called */
	{	return detail + (epsq << EP_SH);	}
inline _property increase_ply_count (_property detail) {	return (detail ^ 1) + 2;	}
inline _property reset_ply_count (_property detail) {	return detail & (~0xfe);	}
inline _property revoke_castle_right (_property detail, _property modifier)
	{	return detail & (~(1 << (modifier + 7)));	}
inline _property get_castle_right (_property detail, _property modifier)
	{	return (detail >> (modifier + 7)) & 1; }
inline int get_plycount(_property detail) {	return (detail >> 1) & 0x7f;	}

/* utility / debug functions */
string piece_to_string (_piece p);
string move_to_string (_move m, position &p);
string piecetype_to_string (_property type);
string piecetype_to_string_figurine(_property type, _property color);
inline string location_to_string (_location sq){
	stringstream ss;
	ss << (1 + (sq>> FOUR_SH));
	return string() + (char)('a' + (sq & TRIPLET_MASK)) + ss.str();
}
inline _location string_to_location(string location){
	return (((location[1] - '1') << 4) + (location[0] - 'a'));
}


/* Round functions */
inline int Round::getSize(){
	return size;
}
inline long Round::get(long hash){
	int index = (int)(hash & (MASK_INDEX));
	if(hashes[index] == hash) return bitstring_descript [index];
	return -1;
}
// ======================End of Functions======================
}

#endif