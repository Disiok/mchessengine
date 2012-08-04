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
/* Zero or null piece definition */
_piece zero_piece = 0;

position::position() : details(start_position), zobrist(0), halfmove_clock(0) {
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
void position::make_move(_move m) {
	bool is_black = is_black_to_move(details);
	_piece(& map)[16] = is_black ? black_map : white_map;
	_property modifier = get_move_modifier(m), type;
	_location start = get_move_start(m), end = get_move_end(m);
	_piece& moving = piece_search (start, is_black);

	details = clear_epsq (details);
	details = increase_ply_count (details);		// switch side to move, ++ plycount
	halfmove_clock++;

	switch (modifier){
	case 0:
		moving ^= end ^ start;
		if ((type = get_piece_type(moving)) == ROOK){
			switch (start){
			case 0x00: details = revoke_castle_right(details, WQS_CASTLE); break;
			case 0x07: details = revoke_castle_right(details, WKS_CASTLE); break;
			case 0x70: details = revoke_castle_right(details, BQS_CASTLE); break;
			case 0x77: details = revoke_castle_right(details, BKS_CASTLE); break;
			}
		}
		else if (type == KING) details &= is_black ? 0x003ff : 0x00cff;
		else if (type == PAWN) details = reset_ply_count (details);
		return;
	case WKS_CASTLE: case BKS_CASTLE:
		map[0] += 0x02;
		moving -= 0x02;
		details &= is_black ? 0x003ff : 0x00cff;	/* revoke both castling rights */
		return;
	case WQS_CASTLE: case BQS_CASTLE:
		map[0] -= 0x02;
		moving += 0x03;
		details &= is_black ? 0x03ff : 0xc0ff;		/* revoke both castling rights */
		return;
	case DOUBLE_ADVANCE:
		moving = move_piece(moving, start, end);
		details = set_epsq(details, end);
		return;
	case EN_PASSANT:
		moving = move_piece(moving, start, end + (is_black ? DOWN : UP));
		kill (piece_search(end, !is_black), !is_black); // assuming the end square is the capturable pawn
		break;
	default:
		details = reset_ply_count (details);
		if (modifier < 10) moving = create_piece (end, modifier - PROMOTE_OFFSET, is_black);
		else {
			moving = move_piece(moving, start, end);
			kill(piece_search(end, !is_black), !is_black);
			_property promote_to = modifier >> 6;
			if (promote_to == 0){
				if ((type = get_piece_type(moving)) == ROOK){
					switch (start){
					case 0x00: details = revoke_castle_right(details, WQS_CASTLE); break;
					case 0x07: details = revoke_castle_right(details, WKS_CASTLE); break;
					case 0x70: details = revoke_castle_right(details, BQS_CASTLE); break;
					case 0x77: details = revoke_castle_right(details, BKS_CASTLE); break;
					}
				} else if (type == KING) details &= is_black ? 0x003ff : 0x00cff;
			} else moving = create_piece (end, promote_to, is_black);
		}
		return;
	}
}
vector <_move> position::move_gen() {
	vector <_move> to_return;
	_property turn_col = is_black_to_move(details), opp_col = turn_col ^ 1;
	_piece* turn_map = turn_col ? black_map : white_map;

	/* Useful pawn constants */
	const char* pawn_atk = turn_col ? BLACK_PAWN_ATTACK : WHITE_PAWN_ATTACK;
	signed char pawn_direction = turn_col ? DOWN : UP;
	_location promotion_row = (opp_col * 0x70), start_row = 0x10 + 0x50 * turn_col,
			epsq = details >> EP_SH;

	if(is_in_check()) {
		// TODO: lots more of stuff
	} else {
		_piece **guardian_map = create_guardian_map (turn_col, opp_col);
		for(int i = 0; i < 16; i++) {
			_piece current_piece = turn_map [i];
			if (current_piece == zero_piece) break;

			bool not_guardian = true;
			_property c_type = get_piece_type(current_piece), type;
			_location c_loc = get_piece_location(current_piece), next_loc, right, left;

			/* is current_piece a guardian? */
			for (int i = 0; i < 8; i++){
				if (current_piece == guardian_map[i]){
					/* if yes, guardian must remain guarding */
					switch (c_type){
					case PAWN:
						_location attacker_loc = get_piece_location(guardian_map[1][i]);
						_property attacker_type = get_piece_type (guardian_map[1][i]);
						signed char diff = attacker_loc - c_loc;
						if (diff == pawn_atk [0] || diff == pawn_atk[1]){
							if ((attacker_loc >> 4) == promotion_row){
								/* promotion with capture */
								for (int i = KNIGHT; i < KING; i++){
									_property capture_mod = create_capture_mod (c_type, attacker_type, i);
									to_return.push_back(create_move(c_loc, attacker_loc, capture_mod));
								}
							} else {
								_property capture_mod = create_capture_mod(c_type, attacker_type);
								to_return.push_back(create_move(c_loc, attacker_loc, capture_mod));
							}
						}
						/* permit en_passant if along guardian direction, only permissible
						 * on diagonal indices. */
						if (epsq != 0 && i > 3){	/* Note, valid epsq is never 0 */
							_location result_loc = epsq + pawn_direction;
							if ((result_loc - c_loc) == RADIAL[i])
								to_return.push_back(create_move(c_loc, epsq, EN_PASSANT));
						}
						break;
						/* only permit moves along guardian direction */
					case ROOK:
						if (i < 4) continuous_gen(c_type, c_loc, to_return, turn_col, RADIAL[i]);
						break;
					case BISHOP:
						if (i > 3) continuous_gen(c_type, c_loc, to_return, turn_col, RADIAL[i]);
						break;
					case QUEEN:
						continuous_gen (c_type, c_loc, to_return, turn_col, RADIAL[i]);
					}
					not_guardian = false;
				}
			}

			/* Piece is not guardian, therefore has no duties to king. */
			if (not_guardian){
				switch(c_type) {
				case KING:
					_piece temp = turn_map[0], obstruct;
					bool ks_avail = true, qs_avail = true;	/* Store results to check castling. */

					/* Normal king moves */
					for (int i = 0; i < 4; i++){
						next_loc = c_loc + DIAGONAL[i];
						if ((next_loc & 0x88) == 0){
							obstruct = piece_search (next_loc);
							turn_map[0] = move_piece (turn_map[0], c_loc, next_loc);
							if (!is_in_check()) {
								if (obstruct == 0) to_return.push_back(create_move(c_loc, next_loc));
								else if (get_piece_color(obstruct) == opp_col){
									_property capture_mod = create_capture_mod(KING, get_piece_type(obstruct));
									to_return.push_back(create_move(c_loc, next_loc, capture_mod));
								}
							}
						}
						turn_map[0] = temp;
					}
					for (int i = 0; i < 4; i++){
						next_loc = c_loc + LINEAR[i];
						if ((next_loc ^ 0x88) == 0){
							obstruct = piece_search (next_loc);
							turn_map[0] = move_piece (turn_map[0], c_loc, next_loc);
							if (!is_in_check()) {
								if (obstruct == 0) to_return.push_back(create_move(c_loc, next_loc));
								else {
									if (get_piece_color(obstruct) == opp_col){
										_property capture_mod = create_capture_mod(KING,
																get_piece_type(obstruct));
										to_return.push_back(create_move(c_loc, next_loc, capture_mod));
									}
									qs_avail = i != 2;
									ks_avail = i != 3;
								}
							} else {
								qs_avail = i != 2;
								ks_avail = i != 3;
							}
						}
						turn_map[0] = temp;
					}

					/* Castling */
					if (opp_col){	/* If wtm */
						if (get_castle_right (details, WKS_CASTLE)){
							next_loc = c_loc + 0x02;
							if (ks_avail && piece_search (next_loc) == 0){
								turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
								if (!is_in_check()) to_return.push_back(create_move(0x07, 0x05, WKS_CASTLE));
								turn_map[0] = temp;
							}
						}
						if (get_castle_right(details, BKS_CASTLE)){
							next_loc = c_loc - 0x02;
							if (qs_avail && piece_search(next_loc)){
								turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
								if (!is_in_check()) to_return.push_back(create_move(0x00, 0x03, WQS_CASTLE));
								turn_map[0] = temp;
							}
						}
					} else {		/* If btm */
						if (get_castle_right(details, BKS_CASTLE)){
							next_loc = c_loc + 0x02;
							if (ks_avail && piece_search (next_loc) == 0){
								turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
								if (!is_in_check()) to_return.push_back(create_move(0x77, 0x75, BKS_CASTLE));
								turn_map[0] = temp;
							}
						}
						if (get_castle_right(details, BQS_CASTLE)){
							next_loc = c_loc - 0x02;
							if (qs_avail && piece_search(next_loc)){
								turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
								if (!is_in_check()) to_return.push_back(create_move(0x70, 0x73, BQS_CASTLE));
								turn_map[0] = temp;
							}
						}
					}
					turn_map[0] = temp;
					break;
				case PAWN:
					/* Pawn advances */
					next_loc = c_loc + pawn_direction;
					if(piece_search(next_loc) == zero_piece) {
						/* in case of direct adv., sq & 0x88 always == 0, provided only legal positions */
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
						/**
						 * Explanation of following code (may seem wtf?!?!):
						 * 	Since both pawns disappear in en passant, the en passant pawn can act as a
						 * 	pseudo-guardian piece. By teleporting both pawns to an invalid square, the
						 * 	capture is performed de facto and we check if the king is in check. If it isn't,
						 * 	then the capture is safe. The pieces are returned to their original squares
						 * 	following the procedure.
						 */
						_piece& ep_pawn = piece_search(epsq, opp_col);
						_piece c_temp = turn_map [i], opp_temp = ep_pawn;
						ep_pawn = create_piece (0x99, PAWN, opp_col);
						turn_map[i] = create_piece(0x99, PAWN, turn_col);
						if (!is_in_check()) to_return.push_back(create_move(c_loc, epsq, EN_PASSANT));
						turn_map[i] = c_temp;
						ep_pawn = opp_temp;
					}

					/* Pawn captures */
					next_loc = c_loc + pawn_atk[0];
					/* note: type of zero_piece is 0 (or zero_piece itself). */
					if ((type = get_piece_type(piece_search(next_loc, opp_col))) != zero_piece){
						/* truth of search(next_loc) != 0 implies truth of sq & 0x88 == 0, given legal */
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
				}
			}
			//XXX is this correct de-allocation of 2d array in memory?
			delete[] guardian_map[0];
			delete[] guardian_map[1];
			delete[] guardian_map;
		}
	}
	return to_return;
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
bool position::is_in_check() {
	bool turn_col = is_black_to_move(details), opp_col = !turn_col, melee;
	_piece* turn_map = turn_col ? black_map : white_map;
	_piece obstruct;
	_property attacker_type;
	_location king = get_piece_location(turn_map[0]), current = king;
	/**
	 * pawn_index explanation (decl. may look like wtf!?!?, so provided):
	 * 		When wtm, opp_col = 1 black_pawn_attacks relevant, i.e., indices 6 & 7 (of RADIAL),
	 * 		so use comparison 5 > i > 8. When btm, opp_col = 0, white_pawn_attacks relevant,
	 * 		i.e., indices 4 & 5, so use comparison 3 > i > 6.
	 *
	 * 		pawn_index determines lowest acceptable index, higher index is just pawn_index + 3.
	 */
	int pawn_index = 3 + opp_col * 2;

	for(int i = 0; i < 8; i++) {
		melee = true;	/* pawn checks possible on first iteration. */
		current = king + RADIAL[i];
		while((current & 0x88) == 0) {
			obstruct = piece_search(current);
			if(obstruct != 0) {
				if(get_piece_color(obstruct) == turn_col) break;
				switch(attacker_type) {
				case PAWN:
					if(melee && ((pawn_index < i) && i < (pawn_index + 3))) return true;
					break;
				case ROOK  : if(i < 4) return true; break;
				case BISHOP: if(i > 3) return true; break;
				case QUEEN : return true;
				}
				break;
			}
			current += RADIAL[i];
			melee = false;	/* revoke pawn check after 1st iteration. */
		}
	}
	for(int i = 0; i < 8; i++)
		if(piece_search(current + KNIGHT_MOVE[i], opp_col) != zero_piece) return true;
	return false;
}
// Implementations of helper methods
void position::continuous_gen(_property type, _location start, vector<_move> &v, _property col, char diff){
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
void position::single_gen(_property type, _location start, vector<_move> &v, _property opp_col, char diff){
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
	_piece **guardian_map = new _piece* [8];
	_piece obstruct, guardian = zero_piece, attacker = zero_piece;
	_property obstruct_type;
	_location king = get_piece_location(turn_map[0]), current;

	for (int i = 0; i < 8; i ++){
		current = king + RADIAL[i];
		bool guardian_found = false, set = false;
		while ((current & 0x88) == 0){
			obstruct = piece_search(current);
			if (obstruct != zero_piece){
				if (!guardian_found){
					if (get_piece_color(obstruct) == opp_col) break;
					guardian = obstruct;
				} else {
					if (get_piece_color(obstruct) == col) break;
					else {
						obstruct_type = get_piece_type(obstruct);
						switch (obstruct_type){
						case ROOK: if (i < 4) attacker = obstruct; set = true; break;
						case BISHOP: if (i > 3) attacker = obstruct; set = true; break;
						case QUEEN: attacker = obstruct; set = true; break;
						}
						break;
					}
				}
			}
			current += RADIAL[i];
		}
		if (set) {
			guardian_map[0][i] = guardian;
			guardian_map[1][i] = attacker;
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
