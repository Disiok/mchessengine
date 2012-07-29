/*
 * rules.cpp
 * ============================================
 * (c) Spark Team, July 2012.
 * The Spark Team reserves all intellectual rights to the following source code.
 * The code may not be distributed or modified for personal use, except with the
 * express permission of a team member.
 * ============================================
 * Contains the implementations functions present in previous .rules package.
 */

#include "myriad.h"

namespace myriad {
	position::position() : details(start_position), zobrist(0) {
		// king is always first
		white_map[0] = create_piece(0x04, KING, WHITE);
		black_map[0] = create_piece(0x74, KING, BLACK);
		// initialize other pieces
		white_map[1] = create_piece(0x00, ROOK, WHITE);
		black_map[1] = create_piece(0x70, ROOK, BLACK);
		white_map[2] = create_piece(0x01, KNIGHT, WHITE);
		black_map[2] = create_piece(0x71, KNIGHT, BLACK);
		white_map[3] = create_piece(0x02, BISHOP, WHITE);
		black_map[3] = create_piece(0x72, BISHOP, BLACK);
		white_map[4] = create_piece(0x03, QUEEN, WHITE);
		black_map[4] = create_piece(0x73, QUEEN, BLACK);
		white_map[5] = create_piece(0x05, BISHOP, WHITE);
		black_map[5] = create_piece(0x75, BISHOP, BLACK);
		white_map[6] = create_piece(0x06, KNIGHT, WHITE);
		black_map[6] = create_piece(0x76, KNIGHT, BLACK);
		white_map[7] = create_piece(0x07, ROOK, WHITE);
		black_map[7] = create_piece(0x77, ROOK, BLACK);
		// initialize second rank with pawns
		for(int i = 0; i < 8; i++) {
			white_map [i + 8] = create_piece(i + 0x10, PAWN, WHITE);
			black_map [i + 8] = create_piece(i + 0x60, PAWN, BLACK);
		}
	}
	vector <_move> position::move_gen() {
		vector <_move> to_return;
		_property turn_col = details % 2, opp_col = turn_col ^ 1;
		_piece* turn_map = turn_col ? black_map : white_map;

		/* Useful pawn constants */
		const char* pawn_atk = turn_col ? BLACK_PAWN_ATTACK : WHITE_PAWN_ATTACK;
		signed char pawn_direction = (turn_col + 1) * 0x10;
		_location promotion_row = (opp_col * 0x70), start_row = 0x10 + 0x50 * turn_col,
				  epsq = details >> EP_SH;

		if(is_in_check()) {
			// TODO: lots more of stuff
		} else {
			// TODO: guardian maps
			for(int i = 0; i < 16; i++) {
				_piece current_piece = turn_map [i];
				_property c_type = get_piece_type(current_piece), type;
				_location c_loc = get_piece_location(current_piece), next_loc, right, left;
				switch(c_type) {
				case PAWN:
					/* Pawn advances */
					next_loc = c_loc + pawn_direction;
					if(piece_search(next_loc) == zero_piece) {
						// in case of direct adv., sq & 0x88 always == 0, provided only legal positions
						_location row = c_loc & 0x70;
						if (row == start_row){
							next_loc += pawn_direction;
							if (piece_search(next_loc) == zero_piece)
								to_return.push_back(create_move(c_loc, next_loc, DOUBLE_ADVANCE));
						} else if (row == promotion_row){
							for (_property i = KNIGHT; i < KING; i++)
								to_return.push_back(create_move(c_loc, next_loc, PROMOTE_OFFSET + i));
						} else to_return.push_back(create_move(c_loc, next_loc));
					}

					/* En-passant */
					right = c_loc + RIGHT;
					left = c_loc + LEFT;
					if (right == epsq || left == epsq){
						//TODO: handle tricky ep case
					}

					/* Pawn captures */
					next_loc = c_loc + pawn_atk[0];
					// note: type of zero_piece is 0 (or zero_piece itself).
					if ((type = get_piece_type(piece_search(next_loc, opp_col))) != zero_piece){
						// truth of search(next_loc) != 0 implies truth of sq & 0x88 == 0, given legal
						if (next_loc & 0x70 == promotion_row){
							for (_property i = KNIGHT; i < KING; i++){
								_property capture_mod = create_capture_mod(PAWN, type, i);
								to_return.push_back(create_move(c_loc, next_loc, capture_mod));
							}
						} else {
							_property capture_mod = create_capture_mod(PAWN, type, i);
							to_return.push_back(create_move(c_loc, next_loc, capture_mod));
						}
					}
					break;
				case ROOK:
					for(int i = 0; i < 4; i++)
						continuous_gen(c_type, c_loc, to_return, turn_col, LINEAR[i]);
					break;
				case QUEEN:
					for(int i = 0; i < 8; i++)
						continuous_gen(c_type, c_loc, to_return, turn_col, RADIAL[i]);
					break;
				case BISHOP:
					for(int i = 0; i < 4; i++)
						continuous_gen(c_type, c_loc, to_return, turn_col, DIAGONAL[i]);
					break;
				case KNIGHT:
					for(int i = 0; i < 8; i++)
						single_gen(c_type, c_loc, to_return, opp_col, KNIGHT_MOVE[i]);
					break;
				case KING:
					// XXX fix king walking into check
					for(int i = 0; i < 8; i++)
						single_gen(c_type, c_loc, to_return, opp_col, RADIAL[i]);
					break;
				}
			}
		}
		return to_return;
	}
	bool position::is_in_check() {
		_property turn_col = details % 2, opp_col = (turn_col ^ 1), attacker_type;
		_piece* turn_map = turn_col ? black_map : white_map;
		_piece obstruct;
		_location king = get_piece_location(turn_map[0]), current = king;
		bool melee;	// flag if pawn attacks available
		int pawn_dir = 3 + opp_col * 2;
		for(int i = 0; i < 8; i++) {
			melee = true;
			current = king + RADIAL[i];
			while((current & 0x88) == 0) {
				obstruct = piece_search(current);
				if(obstruct != 0) {
					if(get_piece_color(obstruct) == turn_col) break;
					switch(attacker_type) {
					case PAWN:
						if(melee && ((pawn_dir < i) && i < (pawn_dir + 3))) return true;
						break;
					case ROOK  : if(i < 4) return true; break;
					case BISHOP: if(i > 3) return true; break;
					case QUEEN : return true;
					}
					break;
				}
				current += RADIAL[i];
				melee = false;
			}
		}
		for(int i = 0; i < 8; i++)
			if(piece_search(current + KNIGHT_MOVE[i], opp_col) != zero_piece) return true;
		return false;
	}
	_piece& position::piece_search(_location square, _property map) {
		_piece* search_map = (map == WHITE ? white_map : black_map);   // search one map only
		for(int i = 0; i < 16; i++) {
			if(get_piece_location(search_map[i]) == square) return search_map[i];
			if(search_map[i] == zero_piece) return zero_piece;
		}
		return zero_piece;
	}
	_piece& position::piece_search(_location square) {
		// search white's then black's map
		for(int i = 0; i < 16; i++) {
			if(get_piece_location(white_map[i]) == square) return white_map [i];
			if(white_map[i] == zero_piece) break;
		}
		for(int i = 0; i < 16; i++) {
			if(get_piece_location(black_map[i]) == square) return black_map [i];
			if(black_map[i] == zero_piece) break;
		}
		return zero_piece;
	}
	void position::make_move(_move m) {
		bool is_black = details & 1;
		_piece(& map)[16] = is_black ? black_map : white_map;
		details &= 0xff00fff;		// erase ep value, use only 28 bits to avoid cast to long
		details ^= 1;				// switch side to move
		_property modifier = get_move_modifier(m);
		_location start = get_move_start(m), end = get_move_end(m);
		_piece& moving = piece_search (start, is_black);

		switch (modifier){
		case 0:
			moving ^= end ^ start;
			if (get_piece_type(moving) == ROOK){
				switch (start){
				case 0x00: details &= 0xffdff; break;	/* revoke appropriate cstl. rights */
				case 0x07: details &= 0xffeff; break;
				case 0x70: details &= 0xff7ff; break;
				case 0x77: details &= 0xffbff; break;
				}
			}
			return;
		case WKS_CASTLE: case BKS_CASTLE:
			map[0] += 0x02;
			moving -= 0x02;
			details &= is_black ? 0x003ff : 0x00cff;
			return;
		case WQS_CASTLE: case BQS_CASTLE:
			map[0] -= 0x02;
			moving += 0x03;
			details &= is_black ? 0x03ff : 0xc0ff;
			return;
		case DOUBLE_ADVANCE:
			start ^= end ^ start;
			details += (end << EP_SH);
			return;
		case EN_PASSANT:
			moving = (moving & 0xf00) + end + (is_black ? DOWN : UP);
			kill (piece_search(end, !is_black), !is_black); // assuming the end square is the capturable pawn
			break;
		default:
			if (modifier < 10) moving = create_piece (end, modifier - PROMOTE_OFFSET, is_black);
			else {
				kill(piece_search(end, !is_black), !is_black);
				_property promote_to = modifier >> 6;
				if (promote_to == 0){
					if (get_piece_type(moving) == ROOK){
						switch (start){
						case 0x00: details &= 0xffdff; break; /* revoke appropriate cstl. rights */
						case 0x07: details &= 0xffeff; break;
						case 0x70: details &= 0xff7ff; break;
						case 0x77: details &= 0xffbff; break;
						}
					}
				} else moving = create_piece (end, promote_to, is_black);
			}
			return;
		}
	}
	// Implementations of helper methods
	void position::continuous_gen(_property type, _location start, vector<_move> &v,
	                              _property col, char diff) {
		_location current = start + diff;
		while((current & 0x88) == 0) {
			_piece obstruct = piece_search(current);
			if(obstruct == zero_piece) v.push_back(create_move(start, current));
			else {
				if(get_piece_color(obstruct) == col) break;
				else {
					_property capture_mod = create_capture_mod (type, get_piece_type(obstruct));
					v.push_back(create_move(start, current, capture_mod));
					break;
				}
			}
			current += diff;
		}
	}
	inline void position::single_gen(_property type, _location start, vector<_move> &v,
									 _property opp_col, char diff) {
		_location target = start + diff;
		if((target & 0x88) == 0) {
			_piece obstruct = piece_search(target);
			if(obstruct == zero_piece) v.push_back(create_move(start, target));
			else if(get_piece_color(obstruct) == opp_col) {
				_property capture_mod = create_capture_mod(type, get_piece_type(obstruct));
				v.push_back(create_move(start, target, capture_mod));
			}
		}
	}
	_piece** position::create_guardian_map (_property col, _property opp_col){
		_piece *turn_map = opp_col ? white_map : black_map;
		_location king = get_piece_location(turn_map[0]), current;
		_piece **guardian_map = new _piece* [8], obstruct;
		_property obstruct_col, obstruct_type;

		for (int i = 0; i < 8; i ++){
			current = king + RADIAL[i];
			bool guardian = false;
			while ((current & 0x88) == 0){
				obstruct = piece_search(current);
				if (obstruct != zero_piece){
					if (!guardian) guardian_map[0][i] = obstruct;
					else {
						if ((obstruct_col = get_piece_color(obstruct)) == col) {
							guardian_map[0][i] = zero_piece; // relieve guardian
							break;
						} else {
							obstruct_type = get_piece_type(obstruct);
							//TODO: what's going on here? Dk how to translate over Simon's code.
							guardian_map[1][i] = obstruct;
							break;
						}
					}
				}
				current += RADIAL[i];
			}
		}
		return guardian_map;
	}
	inline void position::kill(_piece& p, _property victim_map) {
		_piece* search_map = victim_map ? black_map : white_map;
		for (int i = 15; i > 0; i--){	// king never subject to kill
			if (search_map [i] != 0){
				p = search_map[i];
				search_map[i] = zero_piece;
				return;
			}
		}
	}
}
