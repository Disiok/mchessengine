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

position::position() : details(start_position), halfmove_clock(0) {
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
	_piece *map = is_black ? black_map : white_map;
	_property modifier = get_move_modifier(m), type;
	_location start = get_move_start(m), end = get_move_end(m);
	_piece& moving = piece_search (start, is_black);

	details = clear_epsq (details);
	details = increase_ply_count (details);		// switch side to move, ++ plycount
	halfmove_clock++;

	switch (modifier){
	case 0:
		moving = move_piece(moving, start, end);
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
			_property promote_to = modifier >> EIGHT_SH;
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
	_property turn_col = is_black_to_move(details), opp_col = turn_col ^ 1;
	_piece* turn_map = turn_col ? black_map : white_map;
	vector <_move> moves;
	vector<_piece> threats = reachable_pieces(get_piece_location(turn_map[0]), turn_col);
	int threats_size = threats.size();

	/* Useful pawn constants */
	const char* pawn_atk = turn_col ? BLACK_PAWN_ATTACK : WHITE_PAWN_ATTACK;
	signed char pawn_direction = turn_col ? DOWN : UP;
	_location promotion_row = (opp_col * 0x70), start_row = 0x10 + 0x50 * turn_col, epsq = details >> EP_SH;

	if(threats_size != 0) {
		/* if king is currently in check */
		_location c_loc = get_piece_location(turn_map[0]);

		/* Consider moving the king */
		for (int i = 0; i < 8; i++) king_gen(c_loc, turn_map[0], moves, opp_col, RADIAL[i]);

		if (threats_size >= 2) return moves;	/* Only moving the king is possible on double check. */
		else{
			_piece threat = threats[0];
			_property threat_type = get_piece_type(threat);
			_location threat_loc = get_piece_location(threat), next_loc = c_loc;
			_piece *guardian_map = create_guardian_map(turn_col, opp_col);
			unsigned int size;

			/* Attempt to kill the checking piece. (Use opp_col, looking for pieces that can reach opp piece.) */
			vector <_piece> assassins = reachable_pieces(threat_loc, opp_col);
			size = assassins.size();
			bool guardian = false;

			for (unsigned int i = 0; i < assassins.size(); i++){
				for (int j = 0; j < 8; j++){
					if (assassins[i] == guardian_map[j]){
						guardian = true;
						break;
					}
				}
				if (!guardian){
					_location start = get_piece_location(assassins[i]);
					_property type = get_piece_type(assassins[i]);
					if (type == PAWN && (threat_loc & 0x70) == promotion_row){
						for (_property j = KNIGHT; j < KING; j++){
							_property capture_mod = create_capture_mod(PAWN, threat_type, j);
							moves.push_back(create_move(start, threat_loc, capture_mod));
						}
					} else moves.push_back(create_move(start, threat_loc, create_capture_mod(type, threat_type)));
				}
			}

			/* Attempt to block threat by interposing a piece. Possible only with ranged threats. */
			_location double_adv_row = start_row + 2 * (turn_col ? DOWN : UP);
			if (threat_type != PAWN && threat_type != KNIGHT){
				char diff = get_difference(threat_loc, c_loc);
				_location start;
				_piece obstruct;

				/**
				 * Note: The piece is checked for guardian duties before it is added to the returned result.
				 * This is to avoid ConcurrentModificationExceptions (<- that's what is known as in java)
				 * so elements reachable vector is not erased, which then messes up the iterating loop.
				 */
				next_loc = c_loc + diff;
				while (next_loc != threat_loc){
					guardian = false;	/* Reset guardian flag */

					/* Attempt to interpose with a non-guardian pawn, since only pawn captures (and not
					 * advances) are covered in reachable_pieces */
					start = next_loc - pawn_direction;
					obstruct = piece_search (start);
					if (obstruct == zero_piece && ((next_loc & 0x70) == double_adv_row)){
						start -= pawn_direction;	/* Check if double advance is possible. */
						obstruct = piece_search (next_loc, turn_col);
						if (obstruct != zero_piece){
							for (int i = 0; i < 8; i++){
								if (guardian_map[i] == obstruct){
									guardian = true;
									break;
								}
							}
							if (!guardian) moves.push_back(create_move(start, next_loc, DOUBLE_ADVANCE));
						}
					} else if (get_piece_color(obstruct) == turn_col){
						for (int i = 0; i < 8; i++){
							if (guardian_map[i] == obstruct){
								guardian = true;
								break;
							}
						}
						if (!guardian) {
							if (next_loc & 0x70 == promotion_row){
								for (_property i = KNIGHT; i < KING; i++)
									moves.push_back(create_move(start, next_loc, PROMOTE_OFFSET + i));
							}
							else moves.push_back(create_move(start, next_loc));
						}
					}
					/* Attempt to interpose other pieces which are not guardians. */
					vector <_piece> savior = reachable_pieces(next_loc, opp_col);
					int savior_size = savior.size();
					for (int i = 0; i < savior_size; i++){
						guardian = false;
						for (int j = 0; j < 8; j++){
							if (savior[i] == guardian_map[j]){
								guardian = true;
								break;
							}
						}
						if (!guardian) moves.push_back(create_move(get_piece_location(savior[i]), next_loc));
					}
					next_loc += diff;
				}
			} else if (threat_type == PAWN && threat_loc == epsq) {
				/* Use en passant to kill meelee checking pawn. */
				_location left = epsq + LEFT, right = epsq + RIGHT;
				_piece left_piece = piece_search (left, turn_col);
				if (left_piece != zero_piece){
					guardian = false;
					for (int i = 0; i < 8; i++){
						if (guardian_map[i] == left_piece) {
							guardian = true;
							break;
						}
					}
					if (!guardian) moves.push_back(create_move(left, epsq, EN_PASSANT));
				}
				_piece right_piece = piece_search (right, turn_col);
				if (right_piece != zero_piece){
					guardian = false;
					for (int i = 0; i < 8; i++){
						if (guardian_map[i] == right_piece){
							guardian = true;
							break;
						}
					}
					if (!guardian) moves.push_back(create_move(right, epsq, EN_PASSANT));
				}
			}
			/*Only killing the threat is plausible: Special en-passant killing is possible*/
			delete [] guardian_map;
		}
	} else {
		/*if the king is currently not in check */
		_piece *guardian_map = create_guardian_map (turn_col, opp_col);
		for(int current_index = 0; current_index < 16; current_index++) {
			_piece current_piece = turn_map [current_index];
			if (current_piece == zero_piece) break;

			bool not_guardian = true;
			_property c_type = get_piece_type(current_piece), type;
			_location c_loc = get_piece_location(current_piece), next_loc, right, left;

			/* is current_piece a guardian? */
			for (int i = 0; i < 8; i++){
				if (current_piece == guardian_map[i]){
					/* if yes, guardian must remain guarding */
					_location attacker_loc;
					_property attacker_type;
					signed char diff;
					switch (c_type){
					case PAWN:
						attacker_loc = get_piece_location(guardian_map[i + 8]);
						attacker_type = get_piece_type (guardian_map[i + 8]);
						diff = attacker_loc - c_loc;
						if (diff == pawn_atk [0] || diff == pawn_atk[1]){
							if ((attacker_loc >> 4) == promotion_row){
								/* promotion with capture */
								for (_property k = KNIGHT; k < KING; k++){
									_property capture_mod = create_capture_mod (c_type, attacker_type, i);
									moves.push_back(create_move(c_loc, attacker_loc, capture_mod));
								}
							} else {
								_property capture_mod = create_capture_mod(c_type, attacker_type);
								moves.push_back(create_move(c_loc, attacker_loc, capture_mod));
							}
						}
						/* permit en_passant if along guardian direction, only permissible
						 * on diagonal indices. */
						if (epsq != 0 && i > 3){	/* Note, valid epsq is never 0 */
							_location result_loc = epsq + pawn_direction;
							if ((result_loc - c_loc) == RADIAL[i])
								moves.push_back(create_move(c_loc, epsq, EN_PASSANT));
						}
						break;
						/* only permit moves along guardian direction */
					case ROOK:
						if (i < 4) continuous_gen(c_type, c_loc, moves, turn_col, RADIAL[i]);
						break;
					case BISHOP:
						if (i > 3) continuous_gen(c_type, c_loc, moves, turn_col, RADIAL[i]);
						break;
					case QUEEN:
						continuous_gen (c_type, c_loc, moves, turn_col, RADIAL[i]);
					}
					not_guardian = false;
				}
			}
			/* Piece is not guardian, therefore has no duties to king. */
			if (not_guardian){
				_piece temp, obstruct;
				switch(c_type) {
				case KING:
					temp = turn_map[0];
					/* Normal king moves */
					for (int i = 0; i < 4; i++) king_gen(c_loc, turn_map[0], moves, opp_col, DIAGONAL[i]);

					/* If king is not on starting location, then castling is not available. */
					if (c_loc != 0x04 && c_loc != 0x74){
						for (int i = 0; i < 4; i++) king_gen(c_loc, turn_map[0], moves, opp_col, LINEAR[i]);
					} else {
						/**
						 * If castling is possible, then take special considerations. Use a modified version
						 * of king_gen to stop redundant checking of squares to immediate left and right of
						 * king.
						 */
						bool ks_avail = true, qs_avail = true;
						for (int i = 0; i < 4; i++){
							next_loc = c_loc + LINEAR[i];
							if ((next_loc & 0x88) == 0){
								obstruct = piece_search (next_loc);
								if (obstruct == zero_piece){
									turn_map[0] = move_piece (turn_map[0], c_loc, next_loc);
									if (!is_in_check()) moves.push_back(create_move(c_loc, next_loc));
									else {
										if (i == 2) qs_avail = false;	/* Indices for RIGHT and LEFT respectively. */
										if (i == 3) ks_avail = false;
									}
								} else {
									if (get_piece_color(obstruct) == opp_col){
										turn_map[0] = move_piece (turn_map[0], c_loc, next_loc);
										if (!is_in_check()) {
											_property capture_mod = create_capture_mod(KING,
													get_piece_type(obstruct));
											moves.push_back(create_move(c_loc, next_loc, capture_mod));
										}
									}
									if (i == 2) qs_avail = false;
									if (i == 3) ks_avail = false;
								}
							}
							turn_map[0] = temp;
						}

						/* Castling */
						if (opp_col){	/* If wtm */
							if (get_castle_right (details, WKS_CASTLE)){
								next_loc = c_loc + 0x02;
								if (ks_avail && (piece_search (next_loc) == zero_piece)){
									turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
									if (!is_in_check()) moves.push_back(create_move(0x07, 0x05, WKS_CASTLE));
									turn_map[0] = temp;
								}
							}
							if (get_castle_right(details, BKS_CASTLE)){
								next_loc = c_loc - 0x02;
								if (qs_avail && (piece_search(next_loc) == zero_piece)){
									turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
									if (!is_in_check()) moves.push_back(create_move(0x00, 0x03, WQS_CASTLE));
									turn_map[0] = temp;
								}
							}
						} else {		/* If btm */
							if (get_castle_right(details, BKS_CASTLE)){
								next_loc = c_loc + 0x02;
								if (ks_avail && (piece_search (next_loc) == zero_piece)){
									turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
									if (!is_in_check()) moves.push_back(create_move(0x77, 0x75, BKS_CASTLE));
									turn_map[0] = temp;
								}
							}
							if (get_castle_right(details, BQS_CASTLE)){
								next_loc = c_loc - 0x02;
								if (qs_avail && (piece_search(next_loc) == zero_piece)){
									turn_map[0] = move_piece(turn_map[0], c_loc, next_loc);
									if (!is_in_check()) moves.push_back(create_move(0x70, 0x73, BQS_CASTLE));
									turn_map[0] = temp;
								}
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
							moves.push_back(create_move(c_loc, next_loc));
							next_loc += pawn_direction;
							if (piece_search(next_loc) == zero_piece)
								moves.push_back(create_move(c_loc, next_loc, DOUBLE_ADVANCE));
						} else if (row == promotion_row){
							for (_property i = KNIGHT; i < KING; i++)
								moves.push_back(create_move(c_loc, next_loc, PROMOTE_OFFSET + i));
						} else moves.push_back(create_move(c_loc, next_loc));
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
						_piece c_temp = turn_map [current_index], opp_temp = ep_pawn;
						ep_pawn = create_piece (0x99, PAWN, opp_col);
						turn_map[current_index] = create_piece(0x99, PAWN, turn_col);
						if (!is_in_check()) moves.push_back(create_move(c_loc, epsq, EN_PASSANT));
						turn_map[current_index] = c_temp;
						ep_pawn = opp_temp;
					}

					/* Pawn captures */
					for (int pawn_atk_index = 0; pawn_atk_index < 2; pawn_atk_index++){
						next_loc = c_loc + pawn_atk[pawn_atk_index];
						/* note: type of zero_piece is 0 (or zero_piece itself). */
						if ((type = get_piece_type(piece_search(next_loc, opp_col))) != zero_piece){
							/* truth of search(next_loc) != 0 implies truth of sq & 0x88 == 0, given legal */
							if ((next_loc & 0x70) == promotion_row){
								for (_property i = KNIGHT; i < KING; i++){
									_property capture_mod = create_capture_mod(PAWN, type, i);
									moves.push_back(create_move(c_loc, next_loc, capture_mod));
								}
							} else {
								_property capture_mod = create_capture_mod(PAWN, type);
								moves.push_back(create_move(c_loc, next_loc, capture_mod));
							}
						}
					}
					break;
				case ROOK:
					for(int i = 0; i < 4; i++) continuous_gen(c_type, c_loc, moves, turn_col, LINEAR[i]);
					break;
				case QUEEN:
					for(int i = 0; i < 8; i++) continuous_gen(c_type, c_loc, moves, turn_col, RADIAL[i]);
					break;
				case BISHOP:
					for(int i = 0; i < 4; i++) continuous_gen(c_type, c_loc, moves, turn_col, DIAGONAL[i]);
					break;
				case KNIGHT:
					for(int i = 0; i < 8; i++) single_gen(c_type, c_loc, moves, opp_col, KNIGHT_MOVE[i]);
					break;
				}
			}
		}
		delete[] guardian_map;
	}
	return moves;
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
bool position::is_in_check()  {
	bool turn_col = is_black_to_move(details), opp_col = !turn_col, melee;
	_piece* turn_map = turn_col ? black_map : white_map;
	_piece obstruct;
	_property attacker_type;
	_location king = get_piece_location(turn_map[0]), current = king;
	/**
	 * pawn_index explanation (decl. may look like wtf!?!?, so explanation is provided):
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
				case PAWN  :
					if(melee && ((pawn_index < i) && i < (pawn_index + 3))) return true; break;
				case ROOK  : if(i < 4) return true; break;
				case BISHOP: if(i > 3) return true; break;
				case QUEEN : return obstruct;
				}
				break;
			}
			current += RADIAL[i];
			melee = false;	/* revoke pawn check after 1st iteration. */
		}
	}
	for(int i = 0; i < 8; i++){
		obstruct = piece_search(king + KNIGHT_MOVE[i], opp_col);
		if (get_piece_type(obstruct) == KNIGHT) return true;
	}
	return false;
}
void position::unmake_move (_move previous_move, _property prev_details){
	details = prev_details;
	_property modifier = get_move_modifier (previous_move), turn_col = details & 1, opp_col = turn_col ^ 1,
			victim_type;
	_location start = get_move_start (previous_move), end = get_move_end(previous_move);
	_piece* turn_map = turn_col ? black_map : white_map;
	_piece* opp_map = turn_col ? white_map : black_map;
	_piece* captured;
	_piece &moved = piece_search(end, turn_col);

	halfmove_clock--;
	switch (modifier){
	case 0:	case DOUBLE_ADVANCE: moved = move_piece(moved, end, start); break;
	case WKS_CASTLE: case BKS_CASTLE:
		moved += 0x02;
		turn_map[0] -= 0x02;
		break;
	case WQS_CASTLE: case BQS_CASTLE:
		moved -= 0x03;
		turn_map[0] += 0x02;
		break;
	case EN_PASSANT:
		captured = &opp_map[get_last_index(opp_map) + 1];
		*captured = create_piece(end, PAWN, opp_col);
		moved = piece_search(end + (turn_col ? UP : DOWN), turn_col);
		moved = create_piece(start, PAWN, turn_col);
		break;
	default:
		if (modifier < 10) moved = create_piece (start, PAWN, turn_col);
		else {
			victim_type = (modifier >> FOUR_SH) & NIBBLE_MASK;
			captured = &opp_map[get_last_index(opp_map) + 1];
			*captured = create_piece(end, victim_type, opp_col);
			if ((modifier >> EIGHT_SH) == 0) moved = move_piece (moved, end, start);
			else moved = create_piece(start, PAWN, turn_col);
		}
		break;
	}
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
void position::king_gen (_location start, _piece &king, vector <_move> &v, _property opp_col, char diff){
	_piece temp = king, obstruct;
	_location next_loc = start + diff;
	if ((next_loc & 0x88) == 0){
		obstruct = piece_search (next_loc);
		if (obstruct == zero_piece){
			king = move_piece (king, start, next_loc);
			if (!is_in_check()) v.push_back(create_move(start, next_loc));
		} else {
			if (get_piece_color(obstruct) == opp_col){
				king = move_piece (king, start, next_loc);
				if (!is_in_check()) {
					_property capture_mod = create_capture_mod(KING, get_piece_type(obstruct));
					v.push_back(create_move(start, next_loc, capture_mod));
				}
			}
		}
		king = temp;
	}
}
/**
 * Note: this method is a copy of is_in_check(), using the same "octopus" idea. However, this method
 * is not limited to a true/false response or is confined to any square / map.
 */
vector <_piece> position::reachable_pieces (_location square, _property map){
	vector<_piece> to_return;
	_piece obstruct;
	_location current = square;
	bool melee;
	int pawn_index = 5 - map * 2; 	/* See is_in_check() for explanation */

	for(int i = 0; i < 8; i++) {
		melee = true;				/* pawn checks possible on first iteration. */
		current = square + RADIAL[i];
		while((current & 0x88) == 0) {
			obstruct = piece_search(current);
			if(obstruct != 0) {
				if(get_piece_color(obstruct) == map) break;
				switch(get_piece_type(obstruct)) {
				case PAWN  :
					if(melee && ((pawn_index < i) && i < (pawn_index + 3))) to_return.push_back(obstruct);
					break;
				case ROOK  : if(i < 4) to_return.push_back(obstruct); break;
				case BISHOP: if(i > 3) to_return.push_back(obstruct); break;
				case QUEEN : to_return.push_back(obstruct); break;
				}
				break;
			}
			current += RADIAL[i];
			melee = false;			/* revoke pawn check after 1st iteration. */
		}
	}
	for(int i = 0; i < 8; i++){
		obstruct = piece_search(square + KNIGHT_MOVE[i], map ^ 1);
		if (get_piece_type(obstruct) == KNIGHT) to_return.push_back(obstruct);
	}
	return to_return;
}
_piece* position::create_guardian_map (_property col, _property opp_col){
	_piece *turn_map = opp_col ? white_map : black_map;
	_piece *guardian_map = new _piece [16];	/* initialises all indices to zero_piece */
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
		/**
		 * Note on guardian_map organization. Since 2-D arrays are hard to deal with in C++,
		 * the [2][8] array has been collapsed to a [16] array. With 0-7 being guardian_map[0]
		 * and 8-15 being guardian_map[1].
		 */
		guardian_map[i] = set ? guardian : zero_piece;
		guardian_map[i + 8] = set ? attacker : zero_piece;
	}
	return guardian_map;
}
char position::get_difference(_location loc, _location k_loc) {
	_location t_rank = loc >> FOUR_SH, t_file = loc & NIBBLE_MASK, k_rank = k_loc >> FOUR_SH,
			  k_file = k_loc & NIBBLE_MASK;
	if (t_rank == k_rank) return loc > k_loc ? RIGHT : LEFT;
	if (t_file == k_file) return loc > k_loc ? UP : DOWN;
	signed int rank_diff = t_rank - k_rank, file_diff = t_file - k_file;
	if (rank_diff == file_diff) return loc > k_loc ? UP_RIGHT: DOWN_LEFT;
	else return loc > k_loc ? UP_LEFT : DOWN_RIGHT;
	return 0;
}
inline void position::kill(_piece& victim, _property victim_map) {
	_piece* search_map = victim_map ? black_map : white_map;
	_piece& to_swap = search_map[get_last_index(search_map)];
	victim = to_swap;
	to_swap = zero_piece;
}
inline int position::get_last_index (_piece* map){
	for (int i = 15; i > 0; i--) if (map[i] != zero_piece) return i;
	return -1;
}
}