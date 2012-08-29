/*
 * tables.h
 * ============================================
 * (c) Spark Team, Aug 2012.
 * The Spark Team reserves all intellectual rights to the following source code.
 * The code may not be distributed or modified for personal use, except with the
 * express permission of a team member.
 * ============================================
 * Contains all the essential declarations for use in the Myriad engine.
 */

#include "myriad.h"

namespace myriad{

	namespace Zobrist{
		long multiplier[7] = {1, 2, 4, 6, 8, 10, 12};
		long base_hash = 0x00000000;
		int CASTLING_HASHES = 832;
		char EN_PASSANT_ID = 6;

		void init (){
			srand(9001);
			for(int i = 0; i < 836; ++i){
				long data = rand();
				bool unique = false;
				while(!unique){
					unique = true;
					for(int k = 0; k < i; ++k){
						if(data == hash_values[k]){
							data = rand();
							unique = false;
						}
					}
				}
				hash_values[i] = data;
			}
		}

		long createinitialhash (vector<_piece> white, vector<_piece> black, vector<bool> castling_rights, char epsq){
			long to_return = base_hash;
			for(int i = 0; i < white.size(); ++i)
				to_return ^= hash_values[getIndex(get_piece_location(white[i]), get_piece_type(white[i]), get_piece_color(white[i]))];
			for(int i = 0; i < black.size(); ++i)
				to_return ^= hash_values[getIndex(get_piece_location(black[i]), get_piece_type(black[i]), get_piece_color(black[i]))];
			for(int i = 0; i < 4; ++i) if (castling_rights[i]) to_return ^= hash_values[CASTLING_HASHES + i];
			if ((epsq & 0x88) == 0) to_return ^= hash_values[getIndex(epsq, EN_PASSANT_ID, WHITE)];
			return to_return;
		}

		long xorinout (long original_hash, char sq_in, char sq_out, char p_type, char color){
			return original_hash^hash_values[getIndex(sq_in, p_type, color)]^hash_values[getIndex(sq_out, p_type, color)];
		}

		long xorout (long original_hash, char sq_out, char p_type, char color){
			return original_hash^hash_values[getIndex(sq_out, p_type, color)];
		}

		long xorcastling (long original_hash, vector<bool> original_rights, vector<bool> new_rights){
			long new_hash = original_hash;
			for(int i = 0; i < 4; ++i)
				if (original_rights[i] && !new_rights[i]) new_hash ^= hash_values[CASTLING_HASHES + i];
			return new_hash;
		}

		long xorepsq (long original_hash, char original_epsq, char new_epsq){
			long new_hash = original_hash;
			if ((original_epsq & 0x88)==0) new_hash^=hash_values[getIndex(original_epsq, EN_PASSANT_ID, WHITE)];
			if ((new_epsq & 0x88)==0) new_hash^= hash_values[getIndex(original_epsq, EN_PASSANT_ID, WHITE)];
			return new_hash;
		}

		long xorpromotion (long original_hash, char position, char new_type, char color){
			long new_hash = original_hash;
			new_hash^=hash_values[getIndex(position, PAWN, color)];
			new_hash^=hash_values[getIndex(position, new_type, color)];
			return new_hash;
		}

		int getIndex(char pos, char id, char color){
			int modifier = color == WHITE ? 0 : 64;
			return modifier+multiplier[id]*(pos-((pos >> 4)*0x8));
		}
	};
//int round::multiplers[8] = {0, 2, 4, 6, 8, 10, 12, 14};	/* p, n, b, r, q, k, epsq */
}
