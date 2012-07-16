/*
 * rules.cpp
 */
#include "rules.h"

namespace std{
	string piece_to_string (_piece p){
		if (p == 0) return "Null";
		_location sq = get_piece_location(p);
		_property color = get_piece_color(p);
		_property type = get_piece_type(p);
		string s("");
		s = s + piecetype_to_string (type) + location_to_string(sq);
		s += (color == WHITE) ? "(w)" : "(b)";
		return s;
	}
	string move_to_string (_move m, position &p){
		if (m == 0) return "Null";
		_location start = get_move_start(m);
		_location end = get_move_end(m);
		_property modifier = get_move_modifier(m);
		string p_type = piecetype_to_string(get_piece_type(p.piece_search(start)));
		switch (modifier){
		case 0: return p_type + location_to_string(start) + "-" + location_to_string(end);
		case 1: return "0-0 (w)";
		case 2: return "0-0-0 (w)";
		case 3: return "0-0 (b)";
		case 4: return "0-0-0 (b)";
		case 5: return location_to_string(start) + ":" + location_to_string(end) + " e.p.";
		default: return p_type + location_to_string(start) + ":" + location_to_string(end);
		}
	}
	// TODO: Zobrist
	position::position() : details(start_position), zobrist(0){
		// king is always first
		white_map[0] = create_piece (0x04, KING, WHITE);
		black_map[0] = create_piece (0x74, KING, BLACK);
		// initialize other pieces
		white_map[1] = create_piece (0x00, ROOK, WHITE);
		black_map[1] = create_piece (0x70, ROOK, BLACK);
		white_map[2] = create_piece (0x01, KNIGHT, WHITE);
		black_map[2] = create_piece (0x71, KNIGHT, BLACK);
		white_map[3] = create_piece (0x02, BISHOP, WHITE);
		black_map[3] = create_piece (0x72, BISHOP, BLACK);
		white_map[4] = create_piece (0x03, QUEEN, WHITE);
		black_map[4] = create_piece (0x73, QUEEN, BLACK);
		white_map[5] = create_piece (0x05, BISHOP, WHITE);
		black_map[5] = create_piece (0x75, BISHOP, BLACK);
		white_map[6] = create_piece (0x06, KNIGHT, WHITE);
		black_map[6] = create_piece (0x76, KNIGHT, BLACK);
		white_map[7] = create_piece (0x07, ROOK, WHITE);
		black_map[7] = create_piece (0x77, ROOK, BLACK);
		// initialize second rank with pawns
		for (int i = 0; i < 8; i++){
			white_map [i + 8] = create_piece (i + 0x10, PAWN, WHITE);
			black_map [i + 8] = create_piece (i + 0x60, PAWN, BLACK);
		}
	}
	vector <_move> position::move_gen (){
		vector <_move> to_return;
		_property turn_col = details % 2, opp_col = turn_col ^ 1;
		_piece *turn_map = (turn_col == 0) ? white_map : black_map;

		/* Useful pawn constants */
		char *pawn_atk = (turn_col == 0) ? WHITE_PAWN_ATTACK : BLACK_PAWN_ATTACK;
		char pawn_direction = (turn_col + 1) * 0x10;
		int promotion_row = (opp_col * 0x70), start_row = 0x10 + 0x50 * turn_col;
		if (is_in_check()){
			// TODO: lots more of stuff
		} else {
			// TODO: guardian maps
			for (int i = 0; i < 16; i++){
				_piece current_piece = turn_map [i];
				_property c_type = get_piece_type(current_piece);
				_location c_loc = get_piece_location(current_piece);
				switch (c_type){
				case PAWN:
					_location next_loc = c_loc + pawn_direction;
					if (piece_search(next_loc) == 0 && (next_loc & 0x88) == 0){

					}
					break;
				case ROOK:
					for (int i = 0; i < 4; i++)
						continuous_gen(c_type, c_loc, to_return, turn_col, opp_col, LINEAR[i]);
					break;
				case QUEEN:
					for (int i = 0; i < 8; i++)
						continuous_gen(c_type, c_loc, to_return, turn_col, opp_col, RADIAL[i]);
					break;
				case BISHOP:
					for (int i = 0; i < 4; i++)
						continuous_gen(c_type, c_loc, to_return, turn_col, opp_col, DIAGONAL[i]);
					break;
				case KNIGHT:
					for (int i = 0; i < 8; i++)
						single_gen (c_type, c_loc, to_return, turn_col, opp_col, KNIGHT_MOVE[i]);
					break;
				case KING:
					for (int i = 0; i < 8; i++)
						single_gen (c_type, c_loc, to_return, turn_col, opp_col, RADIAL[i]);
					break;
				}
			}
		}
		return to_return;
	}
	bool position::is_in_check (){
		_property turn_col = details % 2, opp_col = (turn_col ^ 1), attacker_type;
		_piece *turn_map = (turn_col == 0) ? white_map : black_map;
		_piece obstruct;
		_location king = get_piece_location(turn_map[0]), current = king;
		bool melee;	// flag if pawn attacks available
		int pawn_dir = 3 + opp_col*2;
		for (int i = 0; i < 8; i++){
			melee = true;
			current = king + RADIAL[i];
			while ((current & 0x88) == 0){
				obstruct = piece_search (current);
				if (obstruct != 0){
					if (get_piece_color(obstruct) == turn_col) break;
					attacker_type = get_piece_type (obstruct);
					switch (attacker_type){
					case PAWN:
						if (melee && ((pawn_dir < i) && i < (pawn_dir + 3))) return true;
						break;
					case ROOK: if (i < 4) return true; break;
					case BISHOP: if (i > 3) return true; break;
					case QUEEN: return true;
					}
					break;
				}
				current += RADIAL[i];
				melee = false;
			}
		}
		for (int i = 0; i < 8; i++)
			if (piece_search (current + KNIGHT_MOVE[i], opp_col) != 0) return true;
		return false;
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
	void position::continuous_gen (_property type, _location start, vector<_move> &v,
								   _property map, _property opp_map, char diff){
		_location current = start + diff;
		while ((current & 0x88) == 0){
			_piece obstruct = piece_search(current);
			if (obstruct != 0){
				if (get_piece_color(obstruct) == map) break;
				else {
					v.push_back(create_move(start, current, create_capture_mod(type,
							get_piece_type(obstruct))));
					break;
				}
			}
			current += diff;
			v.push_back(create_move(start, current));
		}
	}
	inline void position::single_gen (_property type, _location start, vector<_move> &v, _property map,
									  _property opp_map, char diff){
		_location target = start + diff;
		if ((target & 0x88) == 0){
			_piece obstruct = piece_search (target);
			if (obstruct == 0) v.push_back(create_move(start, target));
			else if (get_piece_color(obstruct) == opp_map){
				_property capture_mod = create_capture_mod(type, get_piece_type(obstruct));
				v.push_back(create_move(start, target, capture_mod));
			}
		}
	}
}
