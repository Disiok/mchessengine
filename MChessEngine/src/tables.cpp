/*
 * tables.cpp
 * ============================================
 * (c) Spark Team, Aug 2012.
 * The Spark Team reserves all intellectual rights to the following source code.
 * The code may not be distributed or modified for personal use, except with the
 * express permission of a team member.
 * ============================================
 * Successor to Round.java and Zobrist.java.
 */

#include "myriad.h"

namespace myriad{
_zobrist create_initial_hash (position& p){
	_zobrist initial_hash = 0;
	for (int i = 0; i < 16; i++){
		_piece r = p.white_map[i];
		if (r == zero_piece) break;
		/* Note: xor is it's own inverse, so xor in and xor out is the same thing. */
		initial_hash = xor_out(initial_hash, get_piece_location(r), WHITE, get_piece_type(r));
	}
	for (int i = 0; i < 16; i++){
		_piece r = p.black_map[i];
		if (r == zero_piece) break;
		/* See above note. */
		initial_hash = xor_out(initial_hash, get_piece_location(r), BLACK, get_piece_type(r));
	}
	initial_hash = xor_castling(initial_hash, get_castle_nibble (p.details), 0);
	initial_hash ^= xor_values[EPSQ_HASH_OFFSET + x88to64(get_epsq(p.details))];
	if (is_black_to_move(p.details)) initial_hash ^= xor_values[STM_INDEX];
	return initial_hash;
}
round::round(int bits){
	int size = 2 << bits;		/* 2 << bits raises 2 to the power of bits, no need for cast */
	hashes = new _zobrist[size];
	depth = new short[size];
	information = new _zdata [size];

	/* Generate the XOR bitstrings */
	srand(9001);						/* Seed rand to induce predictable behavior. */
	for(int i = 0; i < 849; ++i){
		/* Ensure each XOR value is unique */
		long data = rand();
		bool unique = false;
		while(!unique){
			unique = true;
			for(int k = 0; k < i; ++k){
				if(data == xor_values[k]){
					data = rand();
					unique = false;
				}
			}
		}
		xor_values[i] = data;
	}
	xor_values[CASTLE_HASH_OFFSET] = 0;
	/* Construct appropriate mask for index allocation. */
	int temp = 0;
	for(int i = 0; i < bits; ++i) temp |= 1 << i;
	MASK_INDEX = temp;
}
/* Round functions */
inline long round::get(_zobrist hash){
	int index = (int)(hash & (MASK_INDEX));
	if(hashes[index] == hash) return information [index];
	return -1;
}
inline bool round::set(_zobrist hash, short score, short level, bool exact, bool bound, _move cutoff){
	if(!exact && bound){
		for(int i = 0; i < KILLER_SIZE; i++){
			if(killer_moves[i] == 0){
				killer_moves[i] = cutoff;
				break;
			}
		}
		for(int i = 1; i < KILLER_SIZE; i++) killer_moves[i] = killer_moves[i-1];
		killer_moves[0] = cutoff;
	}
	int index = (int)(hash & (MASK_INDEX));
	if (hashes[index] == 0 || depth[index] < level){
		hashes[index] = hash;
		depth[index] = level;
		information[index] = exact + (bound << 1) + (score << 2) + ((score < 0) << 17) + (cutoff << 18);
		return true;
	}
	return false;
}
}
