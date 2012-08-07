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
#include <cmath>
#include <sstream>

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
// ======================End of Typedefs=====================
// ======================Classes======================
class position{
public:
	_piece white_map[16];
	_piece black_map[16];
	_property details;		/* LSB->MSB, 1 bit for stm, 7 bits for plycount, 4 bits for cstl. rights.,
							   8 bits for position of pawn capturable by en passant */
	_zobrist zobrist;		// TODO: implement this!
	unsigned short halfmove_clock;

	position();

	bool is_in_check ();
	void make_move(_move m);
	_piece& piece_search(_location square);					/* When search map is unknown */
	_piece& piece_search(_location square, _property map);	/* WHITE for white, BLACK for black */
	vector<_move> move_gen();
	void unmake_move (_move previous_move, _property prev_details);

	//creates a fen string
	operator string ();
	//creates a graphical representation
	string get_graphical();
private:
	void continuous_gen (_property type, _location start, vector<_move> &v, _property col, char difference);
	_piece* create_guardian_map (_property col, _property opp_col);		/* Returns the checking piece. */
	void kill(_piece& p, _property map);
	void king_gen(_location start, _piece &king, vector <_move> &v, _property opp_col, char difference);
	int get_last_index(_piece *map);
	vector<_piece> reachable_pieces(_location sq, _property map);
	void single_gen (_property type, _location start, vector<_move> &v, _property opp_col, char difference);
	char getDifference(_location loc, _location k_loc);
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
inline _piece move_piece(_piece p, _location start, _location end){ return (p ^ start) ^ end; 	}
inline _piece create_piece (_location loc, _property type, _property color)
	{ return (color << COLOR_SH) + (type << EIGHT_SH) + loc; }

/* _move accessors and mutators */
inline _location get_move_start(_move m){ return m & LOCATION_MASK; }
inline _location get_move_end (_move m) { return (m >> EIGHT_SH) & LOCATION_MASK; }
inline _property get_move_modifier (_move m) { return m >> SIXTEEN_SH; }
inline _move create_move (_location start, _location end, _property modifier = 0)
	{	return (modifier << SIXTEEN_SH) + (end << EIGHT_SH) + start; }
/* Create modifiers for capture moves. Bitstring formatted: (MSB-> LSB) promotion piece type,
 * victim piece type, attacker piece type, 3 bits each.
 * Organized this way so direct comparison i.e. _move a > _move b is possible in move-ordering */
inline _property create_capture_mod (_property attacker, _property victim, _property promote = 0)
	{	return (promote << 6) + (victim << 3) + (attacker); }

/* detail _property accessors and mutators */
inline bool is_black_to_move (_property detail) {	return detail & 1;	}
inline _property get_epsq (_property detail) {	return detail >> EP_SH; }
inline _property clear_epsq (_property detail) {	return detail & 0xfff;	}
inline _property set_epsq (_property detail, _location epsq) /* assuming clear_epsq has been called */
	{	return detail + (epsq << EP_SH);	}
inline _property increase_ply_count (_property detail) {	return (detail ^ 1) + 2;	}
inline _property reset_ply_count (_property detail) {	return detail & (~0xfe);	}
inline _property revoke_castle_right (_property detail, _property modifier)
	{	return detail & (~(1 << (modifier + 7)));	}
inline _property get_castle_right (_property detail, _property modifier)
	{	return (detail >> (modifier + 7)) & 1; }

/* utility / debug functions */
string piece_to_string (_piece p);
string move_to_string (_move m, position &p);
string piecetype_to_string (_property type);
inline string location_to_string (_location sq){
	stringstream ss;
	ss << (1 + (sq >> FOUR_SH));
	return string() + (char)('a' + (sq & TRIPLET_MASK)) + ss.str();
}
// ======================End of Functions======================
}

#endif
