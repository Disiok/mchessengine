/*
 * rules.h
 * Successor to the previous rules package.
 * Implements the rules of a chess game under a name-space.
 *
 * (c) Spark Team, July 2012.
 */
#ifndef RULES_H
#define RULES_H

#include <string>
#include <vector>
#include <iostream>

namespace std {
	// ======================Typedefs=====================
	/* Past .rules class redefinitions.
	 * Note: object versions replaced with bit-strings.*/
	typedef short _piece;				/* piece format(LSB->MSB): 8 bits for location, 3 for type,
										   1 for colour */
	typedef int _property;				/* connotes a descriptor of any kind. */
	typedef int _move;					/* move format(LSB->MSB): 8 bits for start, 8 bits for end,
										   remaining bits for descriptor. */
	typedef unsigned char _location;	/* standard 0x88 location square. */
	typedef short _position_detail;		/* details format(LSB->MSB): 1 bit for side to move
										   7 bits plycount, 4 bits for castling rights. */
	typedef long _zobrist;				/* Zobrist hash key */
	// ======================End of Typedefs=====================

	// ======================Constants=====================
	/* Piece constants */
	const _property PAWN = 1;
	const _property ROOK = 2;
	const _property KNIGHT = 3;
	const _property BISHOP = 4;
	const _property QUEEN = 5;
	const _property KING = 6;
	const _property WHITE = 0;
	const _property BLACK = 1;
	/* Bit manipulation special values. */
	const int FOUR_SH = 4;				/* shift by 4 bits, retrieve row in _location, or
										   other 4 bit data, see typedefs */
	const int EIGHT_SH = 8;				/* shift by 8 bits */
	const int SIXTEEN_SH = 16;			/* shift by 16 bits */
	const int COLOR_SH = 11;			/* shift to piece colour. */
	const int LOCATION_MASK = 0xff;		/* masks last 8 bits */
	const int NIBBLE_MASK = 0xf;		/* masks last 4 bits */
	const int TRIPLET_MASK = 0x7;		/* masks last 3 bits, used to det. piece type. or
	 	 	 	 	 	 	 	 	 	   file name. */
	/* Default Values */
	const _position_detail start_position = 0xf00;	/* detail value at startposition */
	// ======================End of Constants======================

	// ======================Functions======================
	/* _piece accessors and mutators */
	inline _location get_piece_location(_piece p){ return p & LOCATION_MASK; }
	inline _property get_piece_color(_piece p){ return p >> COLOR_SH; }
	inline _property get_piece_type(_piece p){ return (p >> EIGHT_SH) & TRIPLET_MASK; }
	inline _piece create_piece (_location loc, _property type, _property color){
		return (color << COLOR_SH) + (type << EIGHT_SH) + loc;
	}
	inline _location get_move_start(_move m){ return m & LOCATION_MASK; }
	inline _location get_move_end (_move m) { return (m >> EIGHT_SH) & LOCATION_MASK; }
	inline _property get_move_modifier (_move m) { return m >> SIXTEEN_SH; }
	inline _move create_move (_location start, _location end, _property modifier = 0){
		return (modifier << SIXTEEN_SH) + (end << EIGHT_SH) + start;
	}
	string string_descriptor (_piece p);
	// ======================End of Functions======================

	// ======================Classes======================
	class position{
	public:
		_piece white_map[16];
		_piece black_map[16];
		_position_detail details;
		_zobrist zobrist;			// TODO: implement this!
		position();
		_piece piece_search(_location square);					/* When search map is unknown */
		_piece piece_search(_location square, _property map);	/* WHITE for white, BLACK for black */
		vector<_move> move_gen();
	private:
		void continuous_gen (_location start, vector<_move> v, _property map, _location direction);
	};
	// ======================End of Classes======================
}
#endif
