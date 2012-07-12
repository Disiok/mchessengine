/*
 * rules.cpp
 */
#include "rules.h"

namespace std{
	string string_descriptor (_piece p){
		if (p == 0) return "Null";
		_location sq = get_piece_location(p);
		_property color = get_piece_color(p);
		_property type = get_piece_type(p);
		string s("");
		switch (type){
		case ROOK: s += "R"; break;
		case KNIGHT: s += "N"; break;
		case BISHOP: s += "B"; break;
		case QUEEN: s += "Q"; break;
		case KING: s += "K"; break;
		case PAWN: break;
		default: s += "Invalid Type"; break;
		}
		s = s + (char)('a' + (sq & TRIPLET_MASK)) + (char)('1' + (sq >> FOUR_SH));
		s += (color == WHITE) ? "(w)" : "(b)";
		return s;
	}
	// TODO: Zobrist
	position::position() : details(start_position), zobrist(0){
		// king is always first
		white_map[0] = create_piece (0x04, KING, WHITE);
		black_map[0] = create_piece (0x74, KING, WHITE);
		// initialize first rank
		for (int i = 0; i < 4; i++){
			white_map [i + 1] = create_piece (i, i + 2, WHITE);
			black_map [i + 1] = create_piece (i + 0x70, i + 2, BLACK);
		}
		for (int i = 0; i < 3; i++){
			white_map [i + 5] = create_piece (i + 0x05, 4 - i, WHITE);
			black_map [i + 5] = create_piece (i + 0x75, 4 - i, BLACK);
		}
		// initialize second rank with pawns
		for (int i = 0; i < 8; i++){
			white_map [i + 8] = create_piece (i + 0x10, PAWN, WHITE);
			black_map [i + 8] = create_piece (i + 0x60, PAWN, BLACK);
		}
	}
	vector <_move> position::move_gen (){
		vector <_move> to_return;
		// TODO: tons of stuff
		return to_return;
	}
	_piece position::piece_search (_location square, _property map){
		_piece* search_map = (map == WHITE ? white_map : black_map); // search one map only
		for (int i = 0; i < 16; i++){
			if (search_map[i] == 0) return 0;
			if (get_piece_location(search_map[i]) == square) return search_map[i];
		}
		return 0;
	}
	_piece position::piece_search (_location square){
		// search both maps
		for (int i = 0; i < 16; i++){
			if (white_map[i] == 0) break;
			if (get_piece_location(white_map[i]) == square) return white_map [i];
		}
		for (int i = 0; i < 16; i++){
			if (black_map[i] == 0) break;
			if (get_piece_location(black_map[i]) == square) return black_map [i];
		}
		return 0;
	}
	void position::continuous_gen (_location start, vector<_move> v, _property map,
															_location difference){
		_location current = start + difference;
		while ((current & 0x88) == 0){
			_piece obstruct = piece_search(current);
			if (obstruct != 0){
				if (get_piece_color(obstruct) == map) break;
				else {
					v.push_back(create_move(start, current)); // TODO: handle captures
					break;
				}
			}
			current += difference;
			v.push_back(create_move(start, current));
		}
	}
}
