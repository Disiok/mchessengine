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
long  xor_values[849];

position::position() : details(start_position), fullmove_clock(0) {
	for (int i = 0; i < 0x78 ; ++i)	board[i] = &zero_piece;
	/* king is always first */
	white_map[0] = create_piece(0x04, KING, WHITE);
	black_map[0] = create_piece(0x74, KING, BLACK);
	/* Initialize other pieces */
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

	/* Put it all into the board */
	board[0x04]= &white_map[0];
	board[0x74]= &black_map[0];
	for (unsigned char i = 1; i < 8; ++i) {
		if (i <= 4){
			board[0 + i - 1] = &white_map[i];
			board[0x70 + i - 1] = &black_map[i];
		}
		else if (i > 4) {
			board[0 + i] = &white_map[i];
			board[0x70 + i] = &black_map[i];
		}
	}
	/* Fill second rank with pawns */
	for(int i = 0; i < 8; ++i) {
		white_map [i + 8] = create_piece(i + 0x10, PAWN, WHITE);
		board[i+0x10]= &white_map[i+8];
		black_map [i + 8] = create_piece(i + 0x60, PAWN, BLACK);
		board[i+0x60]= &black_map[i+8];
	}
	hash_key = create_initial_hash(*this);
}
void position::make_move(_move m) {
	bool is_black = is_black_to_move(details);
	_piece *map = is_black ? black_map : white_map;
	_property modifier = get_move_modifier(m), castl_rights = get_castle_nibble(details), type;
	_location start = get_move_start(m), end = get_move_end(m), temp;
	_piece& moving = piece_search (start, is_black);

	hash_key = xor_epsq(hash_key, get_epsq(details), 0x00);
	hash_key ^= xor_values[STM_INDEX];
	details = clear_epsq (details);
	details = increase_ply_count (details);		/* switch side to move, ++ plycount */
	++fullmove_clock;

	switch (modifier){
	case 0:
		move_piece(moving, start, end, board);
		if ((type = get_piece_type(moving)) == ROOK){
			switch (start){
			case 0x00: details = revoke_castle_right(details, WQS_CASTLE); break;
			case 0x07: details = revoke_castle_right(details, WKS_CASTLE); break;
			case 0x70: details = revoke_castle_right(details, BQS_CASTLE); break;
			case 0x77: details = revoke_castle_right(details, BKS_CASTLE); break;
			}
		}
		/*Revoke castling rights due to a king move */
		else if (type == KING) details &= is_black ? 0x003ff : 0x00cff;
		else if (type == PAWN) details = reset_ply_count (details);
		switch (end){
		case 0x00: details = revoke_castle_right(details, WQS_CASTLE); break;
		case 0x07: details = revoke_castle_right(details, WKS_CASTLE); break;
		case 0x70: details = revoke_castle_right(details, BQS_CASTLE); break;
		case 0x77: details = revoke_castle_right(details, BKS_CASTLE); break;
		}
		hash_key = xor_in_out (hash_key, start, end, is_black, type);
		break;
		case WKS_CASTLE: case BKS_CASTLE:
			move_piece(moving, start, end, board);
			temp = get_piece_location(map[0]);
			move_piece(map[0], temp, temp + 0x02, board);
			details &= is_black ? 0x003ff : 0x00cff;		/* revoke both castling rights */
			break;
		case WQS_CASTLE: case BQS_CASTLE:
			move_piece(moving, start, end, board);
			temp = get_piece_location(map[0]);
			move_piece(map[0],temp, temp - 0x02, board);
			details &= is_black ? 0x003ff : 0x00cff;		/* revoke both castling rights */
			break;
		case DOUBLE_ADVANCE:
			move_piece(moving, start, end, board);
			details = set_epsq(details, end);
			hash_key = xor_epsq(hash_key, 0x00, end);
			hash_key = xor_in_out(hash_key, start, end, is_black, PAWN);
			break;
		case EN_PASSANT:
			temp = end + (is_black ? DOWN : UP);
			move_piece(moving, start, temp, board);
			kill (piece_search(end, !is_black), !is_black); /* assuming the end square is the capturable pawn */
			board[end] = &zero_piece;
			hash_key = xor_out(hash_key, end, !is_black, PAWN);
			hash_key = xor_in_out(hash_key, start, temp, is_black, PAWN);
			break;
		default:
			details = reset_ply_count (details);
			if (modifier < 10){
				_property promote_to = modifier - PROMOTE_OFFSET;
				moving = create_piece (end, promote_to, is_black);
				board[start] = &zero_piece;
				board[end] = &moving;
				hash_key = xor_promotion(hash_key, start, end, is_black, promote_to);
			} else {
				_piece &captured = piece_search(end, !is_black);
				hash_key = xor_out(hash_key, end, !is_black, get_piece_type(captured));
				kill(captured, !is_black);
				move_piece(moving, start, end, board);
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
					hash_key = xor_in_out(hash_key, start, end, is_black, type);
				} else {
					moving = moving ^ 0x100 ^ (promote_to << 8);
					board[end] = &moving;
					hash_key = xor_promotion(hash_key, start, end, is_black, promote_to);
				}
				switch (end){
				case 0x00: details = revoke_castle_right(details, WQS_CASTLE); break;
				case 0x07: details = revoke_castle_right(details, WKS_CASTLE); break;
				case 0x70: details = revoke_castle_right(details, BQS_CASTLE); break;
				case 0x77: details = revoke_castle_right(details, BKS_CASTLE); break;
				}
			}
			break;
	}
	hash_key = xor_castling(hash_key, get_castle_nibble(details), castl_rights);
}
vector <_move>* position::move_gen() {
	_property turn_col = is_black_to_move(details), opp_col = turn_col ^ 1;
	_piece* turn_map = turn_col ? black_map : white_map;
	_location king = get_piece_location(turn_map[0]);
	_piece obstruct;
	/*Reserving for 35 moves, the average branching factor, see Shannon Number */
	vector <_move>* moves = new vector<_move>;
	moves->reserve(35);
	vector<_piece> threats = reachable_pieces(get_piece_location(turn_map[0]), turn_col);
	pawn_capture_reach(threats, king, opp_col);	/* Consider pawns that can reach the king */
	int threats_size = threats.size();
	/* Useful pawn constants */
	const char* pawn_atk = turn_col ? BLACK_PAWN_ATTACK : WHITE_PAWN_ATTACK;
	signed char pawn_direction = turn_col ? DOWN : UP;
	_location promotion_row = (opp_col * 0x70), start_row = 0x10 + 0x50 * turn_col, epsq = details >> EP_SH;

	if(threats_size != 0) {
		/* if king is currently in check */
		/* Consider moving the king */
		for (int i = 0; i < 8; ++i) king_gen(king, turn_map[0], *moves, opp_col, RADIAL[i]);

		if (threats_size >= 2) return moves;	/* Only moving the king is possible on double check. */
		else{
			_piece threat = threats[0];
			_property threat_type = get_piece_type(threat);
			_location threat_loc = get_piece_location(threat), next_loc = king;
			_piece *guardian_map = create_guardian_map(turn_col, opp_col);
			unsigned int size;

			/* Attempt to kill the checking piece. (Use opp_col, looking for pieces that can reach opp piece.) */
			vector <_piece> assassins = reachable_pieces(threat_loc, opp_col);
			/* Consider pawns only in capturing the checking piece, not to interpose. */
			pawn_capture_reach(assassins, threat_loc, turn_col);
			size = assassins.size();
			for (unsigned int i = 0; i < size; ++i){
				if (!get_assailant (guardian_map, assassins[i])){
					_location start = get_piece_location(assassins[i]);
					_property type = get_piece_type(assassins[i]);
					if (type == PAWN && (threat_loc & 0x70) == promotion_row){
						for (_property j = KNIGHT; j < KING; ++j){
							_property capture_mod = create_capture_mod(PAWN, threat_type, j);
							moves->push_back(create_move(start, threat_loc, capture_mod));
						}
					} else moves->push_back(create_move(start, threat_loc, create_capture_mod(type, threat_type)));
				}
			}
			/* Attempt to block threat by interposing a piece. Possible only with ranged threats. */
			_location double_adv_row = start_row + 2 * (turn_col ? DOWN : UP);
			if (threat_type != PAWN && threat_type != KNIGHT){
				char diff = get_difference(threat_loc, king);
				_location start;
				_piece obstruct;

				/**
				 * Note: The piece is checked for guardian duties before it is added to the returned result.
				 * This is to avoid ConcurrentModificationExceptions (<- that's what is known as in java)
				 * so elements reachable vector is not erased, which then messes up the iterating loop.
				 */
				next_loc = king + diff;
				while (next_loc != threat_loc){
					/* Attempt to interpose with a non-guardian pawn, since only pawn captures (and not
					 * advances) are covered in reachable_pieces */
					start = next_loc - pawn_direction;
					obstruct = piece_search (start);
					if (obstruct == zero_piece && ((next_loc & 0x70) == double_adv_row)){
						start -= pawn_direction;	/* Check if double advance is possible. */
						obstruct = piece_search (start, turn_col);
						/* Pieces are not guardians if they don't have an assailant pinning them
						 * to the king's defence. */
						if (get_piece_type(obstruct) == PAWN && get_assailant(guardian_map, obstruct) == zero_piece)
							moves->push_back(create_move(start, next_loc, DOUBLE_ADVANCE));
					} else if (get_piece_color(obstruct) == turn_col){
						if (get_piece_type(obstruct) == PAWN && get_assailant(guardian_map, obstruct) == zero_piece){
							if ((next_loc & 0x70) == promotion_row){
								for (_property i = KNIGHT; i < KING; ++i)
									moves->push_back(create_move(start, next_loc, PROMOTE_OFFSET + i));
							}
							else moves->push_back(create_move(start, next_loc));
						}
					}
					/* Attempt to interpose other pieces which are not guardians. */
					vector <_piece> savior = reachable_pieces(next_loc, opp_col);
					int savior_size = savior.size();
					for (int i = 0; i < savior_size; ++i){
						if (get_assailant(guardian_map, savior[i]) == zero_piece)
							moves->push_back(create_move(get_piece_location(savior[i]), next_loc));
					}
					next_loc += diff;
				}
			} else if (threat_type == PAWN && threat_loc == epsq) {
				/* Use en passant to kill meelee checking pawn. */
				_location left = epsq + LEFT, right = epsq + RIGHT;
				_piece left_piece = piece_search (left, turn_col);
				if (get_piece_type(left_piece) == PAWN && get_assailant(guardian_map, left_piece) == zero_piece)
					moves->push_back(create_move(left, epsq, EN_PASSANT));
				_piece right_piece = piece_search (right, turn_col);
				if (get_piece_type(right_piece) == PAWN && get_assailant(guardian_map, right_piece) == zero_piece)
					moves->push_back(create_move(right, epsq, EN_PASSANT));
			}
			/*Only killing the threat is plausible: Special en-passant killing is possible*/
			delete [] guardian_map;
		}
	} else {
		/*if the king is currently not in check */
		_piece *guardian_map = create_guardian_map (turn_col, opp_col);
		for(int current_index = 0; current_index < 16; ++current_index) {
			_piece current_piece = turn_map [current_index];
			if (current_piece == zero_piece) break;
			_property c_type = get_piece_type(current_piece), type;
			_location c_loc = get_piece_location(current_piece), next_loc, right, left;
			_piece assailant = get_assailant(guardian_map, current_piece);

			/* Piece is not guardian, therefore has no duties to king. */
			if (assailant == zero_piece){
				switch(c_type) {
				case KING:
					/* Normal king moves */
					for (int i = 0; i < 4; ++i) {
						king_gen(c_loc, turn_map[0], *moves, opp_col, DIAGONAL[i]);
					}
					/* If king is not on starting location, then castling is not available. */
					if (c_loc != 0x04 && c_loc != 0x74){
						for (int i = 0; i < 4; ++i) king_gen(c_loc, turn_map[0], *moves, opp_col, LINEAR[i]);
					} else {
						/**
						 * If castling is possible, then take special considerations. Use a modified version
						 * of king_gen to stop redundant checking of squares to immediate left and right of
						 * king.
						 */
						bool ks_avail = true, qs_avail = true;
						for (int i = 0; i < 4; ++i){
							next_loc = c_loc + LINEAR[i];
							if ((next_loc & 0x88) == 0){
								obstruct = piece_search (next_loc);
								if (obstruct == zero_piece){
									move_piece (turn_map[0], c_loc, next_loc, board);
									if (!is_in_check()) moves->push_back(create_move(c_loc, next_loc));
									else {
										if (i == 2) qs_avail = false;
										if (i == 3) ks_avail = false;
									}
									move_piece (turn_map[0], next_loc, c_loc, board);
								} else {
									if (get_piece_color(obstruct) == opp_col){
										_piece *temp = board[next_loc];
										move_piece (turn_map[0], c_loc, next_loc, board);
										if (!is_in_check()) {
											_property capture_mod = create_capture_mod(KING,
													get_piece_type(obstruct));
											moves->push_back(create_move(c_loc, next_loc, capture_mod));
										}
										move_piece (turn_map[0], next_loc, c_loc, board);
										board[next_loc] = temp;
									}
									if (i == 2) qs_avail = false;
									if (i == 3) ks_avail = false;
								}
							}
						}
						/* Castling */
						if (opp_col){	/* If wtm */
							if (get_castle_right (details, WKS_CASTLE)){
								next_loc = c_loc + 0x02;
								if (ks_avail && (piece_search (next_loc) == zero_piece)){
									_piece *temp = board[next_loc];
									move_piece(turn_map[0], c_loc, next_loc, board);
									if (!is_in_check()) moves->push_back(create_move(0x07, 0x05, WKS_CASTLE));
									move_piece(turn_map[0], next_loc, c_loc, board);
									board[next_loc] = temp;

								}
							}
							if (get_castle_right(details, WQS_CASTLE)){
								next_loc = c_loc - 0x02;
								_piece obstruct1 = piece_search(next_loc);
								_piece obstruct2 = piece_search(next_loc + LEFT);
								if (qs_avail && (obstruct1 == zero_piece) && (obstruct2 == zero_piece)){
									_piece *temp = board[next_loc];
									move_piece(turn_map[0], c_loc, next_loc, board);
									if (!is_in_check()) moves->push_back(create_move(0x00, 0x03, WQS_CASTLE));
									move_piece(turn_map[0], next_loc, c_loc, board);
									board[next_loc] = temp;
								}
							}
						} else {		/* If btm */
							if (get_castle_right(details, BKS_CASTLE)){
								next_loc = c_loc + 0x02;
								if (ks_avail && (piece_search(next_loc) == zero_piece)){
									_piece *temp = board[next_loc];
									move_piece(turn_map[0], c_loc, next_loc, board);
									if (!is_in_check()) moves->push_back(create_move(0x77, 0x75, BKS_CASTLE));
									move_piece(turn_map[0], next_loc, c_loc, board);
									board[next_loc] = temp;
								}
							}
							if (get_castle_right(details, BQS_CASTLE)){
								next_loc = c_loc - 0x02;
								_piece obstruct1 = piece_search(next_loc);
								_piece obstruct2 = piece_search(next_loc + LEFT);
								if (qs_avail && (obstruct1 == zero_piece) && (obstruct2 == zero_piece)){
									_piece *temp = board[next_loc];
									move_piece(turn_map[0], c_loc, next_loc, board);
									if (!is_in_check()) moves->push_back(create_move(0x70, 0x73, BQS_CASTLE));
									move_piece(turn_map[0], next_loc, c_loc, board);
									board[next_loc] = temp;
								}
							}
						}
					}

					break;
				case PAWN:
					/* Pawn advances */
					next_loc = c_loc + pawn_direction;
					if(piece_search(next_loc) == zero_piece) {
						/* in case of direct adv., sq & 0x88 always == 0, provided only legal positions */
						if ((c_loc & 0x70) == start_row){
							moves->push_back(create_move(c_loc, next_loc));
							next_loc += pawn_direction;
							if (piece_search(next_loc) == zero_piece)
								moves->push_back(create_move(c_loc, next_loc, DOUBLE_ADVANCE));
						} else if ((next_loc & 0x70) == promotion_row){
							for (_property i = KNIGHT; i < KING; ++i)
								moves->push_back(create_move(c_loc, next_loc, PROMOTE_OFFSET + i));
						} else moves->push_back(create_move(c_loc, next_loc));
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
						move_piece(ep_pawn, epsq, 0x09, board);
						_location temp = epsq + pawn_direction;
						move_piece(turn_map[current_index], c_loc ,temp, board);
						if (!is_in_check()) moves->push_back(create_move(c_loc, epsq, EN_PASSANT));
						move_piece(turn_map[current_index], temp ,c_loc, board);
						move_piece(ep_pawn, 0x09, epsq, board);
					}

					/* Pawn captures */
					for (int pawn_atk_index = 0; pawn_atk_index < 2; ++pawn_atk_index){
						next_loc = c_loc + pawn_atk[pawn_atk_index];
						/* note: type of zero_piece is 0 (or zero_piece itself). */
						if ((type = get_piece_type(piece_search(next_loc, opp_col))) != zero_piece){
							/* truth of search(next_loc) != 0 implies truth of sq & 0x88 == 0, given legal */
							if ((next_loc & 0x70) == promotion_row){
								for (_property i = KNIGHT; i < KING; ++i){
									_property capture_mod = create_capture_mod(PAWN, type, i);
									moves->push_back(create_move(c_loc, next_loc, capture_mod));
								}
							} else {
								_property capture_mod = create_capture_mod(PAWN, type);
								moves->push_back(create_move(c_loc, next_loc, capture_mod));
							}
						}
					}
					break;
				case ROOK:
					for(int i = 0; i < 4; ++i) continuous_gen(c_type, c_loc, *moves, turn_col, LINEAR[i]);
					break;
				case QUEEN:
					for(int i = 0; i < 8; ++i) continuous_gen(c_type, c_loc, *moves, turn_col, RADIAL[i]);
					break;
				case BISHOP:
					for(int i = 0; i < 4; ++i) continuous_gen(c_type, c_loc, *moves, turn_col, DIAGONAL[i]);
					break;
				case KNIGHT:
					for(int i = 0; i < 8; ++i) single_gen(c_type, c_loc, *moves, opp_col, KNIGHT_MOVE[i]);
					break;
				}
			} else {
				/* Guardians must move in such away that they do not hurt the king. */
				_location attacker_loc = get_piece_location (assailant);
				signed char diff = get_difference(c_loc, attacker_loc);
				_property attacker_type;
				switch (c_type){
				case PAWN:
					if (diff == UP || diff == DOWN){
						next_loc = c_loc + pawn_direction;
						if (piece_search(next_loc) == zero_piece) {
							moves->push_back(create_move(c_loc, next_loc));
							next_loc += pawn_direction;
							if ((c_loc & 0x70) == start_row && piece_search(next_loc) == zero_piece)
								moves->push_back(create_move(c_loc, next_loc, DOUBLE_ADVANCE));
						}
					} else {
						attacker_type = get_piece_type (assailant);
						char close_diff = attacker_loc - c_loc;
						if (close_diff == pawn_atk [0] || close_diff == pawn_atk[1]){
							if ((attacker_loc & 0x70) == promotion_row){
								/* promotion with capture */
								for (_property k = KNIGHT; k < KING; ++k){
									_property capture_mod = create_capture_mod (c_type, attacker_type, k);
									moves->push_back(create_move(c_loc, attacker_loc, capture_mod));
								}
							} else {
								_property capture_mod = create_capture_mod(c_type, attacker_type);
								moves->push_back(create_move(c_loc, attacker_loc, capture_mod));
							}
						}
						/* permit en_passant if along guardian direction. */
						if (epsq != 0 && (epsq + pawn_direction - c_loc) == diff)
								moves->push_back(create_move(c_loc, epsq, EN_PASSANT));
					}
					break;
					/* only permit moves along guardian direction */
				case ROOK:
					if (diff == UP || diff == DOWN || diff == RIGHT || diff == LEFT){
						continuous_gen(c_type, c_loc, *moves, turn_col, diff);
						continuous_gen(c_type, c_loc, *moves, turn_col, -diff);
					}
					break;
				case BISHOP:
					if (diff != UP && diff != DOWN && diff != RIGHT && diff != LEFT){
						continuous_gen(c_type, c_loc, *moves, turn_col, diff);
						continuous_gen(c_type, c_loc, *moves, turn_col, -diff);
					}
					break;
				case QUEEN:
					continuous_gen(c_type, c_loc, *moves, turn_col, diff);
					continuous_gen(c_type, c_loc, *moves, turn_col, -diff);
					break;
				}
			}
		}
		delete[] guardian_map;
	}
	return moves;
}
_piece& position::piece_search(_location square, _property map) {
	_piece *p;
	if ((square & 0x88) == 0) p = board[square];
	else return zero_piece;
	return get_piece_color(*p) == map ? *p : zero_piece;
}
_piece& position::piece_search(_location square) {
	if ((square & 0x88) == 0) return *(board[square]);
	else return zero_piece;
}
bool position::is_in_check()  {
	bool turn_col = is_black_to_move(details), opp_col = !turn_col, melee;
	_piece* turn_map = turn_col ? black_map : white_map;
	_piece obstruct;
	_location king = get_piece_location(turn_map[0]), current = king;
	/**
	 * pawn_index explanation (decl. may look like wtf!?!?, so explanation is provided):
	 * 		When wtm, turn_col = 1, differences from king to rel. square are indices 6 & 7 (of RADIAL),
	 * 		so use comparison 5 > i > 8. When btm, opp_col = 0, differences from king to rel. square
	 * 		are indices 4 & 5, so use comparison 3 > i > 6.
	 *
	 * 		pawn_index determines lowest acceptable index, higher index is just pawn_index + 3.
	 */
	int pawn_index = 3 + turn_col * 2;

	for(int i = 0; i < 8; ++i) {
		melee = true;	/* pawn checks possible on first iteration. */
		current = king + RADIAL[i];
		while((current & 0x88) == 0) {
			obstruct = piece_search(current);
			if(obstruct != 0) {
				if(get_piece_color(obstruct) == turn_col) break;
				switch(get_piece_type(obstruct)) {
				case PAWN  :
					if(melee && ((pawn_index < i) && i < (pawn_index + 3))) return true; break;
				case ROOK  : if(i < 4) return true; break;
				case BISHOP: if(i > 3) return true; break;
				case QUEEN : return true;
				case KING : if (melee) return true; break;
				}
				break;
			}
			current += RADIAL[i];
			melee = false;	/* revoke pawn check after 1st iteration. */
		}
	}
	for(int i = 0; i < 8; ++i){
		obstruct = piece_search(king + KNIGHT_MOVE[i], opp_col);
		if (get_piece_type(obstruct) == KNIGHT) return true;
	}
	return false;
}
void position::unmake_move (_move previous_move, _property prev_details, _zobrist prev_hash){
	details = prev_details;
	hash_key = prev_hash;
	_property modifier = get_move_modifier (previous_move), turn_col = details & 1, opp_col = turn_col ^ 1,
			victim_type;
	_location start = get_move_start (previous_move), end = get_move_end(previous_move), temp;
	_piece* turn_map = turn_col ? black_map : white_map, *opp_map = turn_col ? white_map : black_map, *captured,
			*ep_moved;
	_piece& moved = piece_search(end, turn_col);

	--fullmove_clock;
	switch (modifier){
	case 0:	case DOUBLE_ADVANCE: move_piece(moved, end, start, board); break;
	case WKS_CASTLE: case BKS_CASTLE:
		move_piece(moved, end, start, board);
		temp = get_piece_location(turn_map[0]);
		move_piece(turn_map[0], temp, temp - 0x02, board);
		break;
	case WQS_CASTLE: case BQS_CASTLE:
		move_piece(moved, end, start, board);
		temp = get_piece_location(turn_map[0]);
		move_piece(turn_map[0],temp, temp + 0x02, board);
		break;
	case EN_PASSANT:
		assert(get_last_index(opp_map) + 1 < 16 && get_last_index(opp_map) + 1 >= 0);
		captured = &opp_map[get_last_index(opp_map) + 1];
		*captured = create_piece(end, PAWN, opp_col);
		board[end] = captured;
		temp = end + (turn_col ? DOWN : UP);
		ep_moved = &piece_search(temp, turn_col);
		*ep_moved = create_piece(start, PAWN, turn_col);
		board[temp] = &zero_piece;
		board[start]= ep_moved;
		break;
	default:
		if (modifier < 10){
			moved = create_piece (start, PAWN, turn_col);
			board[end] = &zero_piece;
			board[start] = &moved;
		} else {
			if ((modifier >> EIGHT_SH) == 0) move_piece (moved, end, start, board);
			else {
				moved = create_piece(start, PAWN, turn_col);
				board[start] = &moved;
			}
			victim_type = (modifier >> FOUR_SH) & NIBBLE_MASK;
			assert(get_last_index(opp_map) + 1 >= 0 && get_last_index(opp_map) + 1 < 16);
			captured = &opp_map[get_last_index(opp_map) + 1];
			*captured = create_piece(end, victim_type, opp_col);
			board[end] = captured;
		}
		break;
	}
}
// Implementations of helper methods
inline void position::continuous_gen(_property type, _location start, vector<_move> &v, _property col, char diff){
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
inline void position::single_gen(_property type, _location start, vector<_move> &v, _property opp_col, char diff){
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
inline void position::king_gen (_location start, _piece &king, vector <_move> &v, _property opp_col, char diff){
	_piece obstruct_ref;
	_location next_loc = start + diff;
	if ((next_loc & 0x88) == 0){
		obstruct_ref = piece_search (next_loc);
		if (obstruct_ref == zero_piece){
			move_piece (king, start, next_loc,board);
			if (!is_in_check()) v.push_back(create_move(start, next_loc));
			move_piece (king, next_loc, start,board);
		} else {
			if (get_piece_color(obstruct_ref) == opp_col){
				_piece *temp = board[next_loc];
				move_piece (king, start, next_loc, board);
				if (!is_in_check()) {
					_property capture_mod = create_capture_mod(KING, get_piece_type(obstruct_ref));
					v.push_back(create_move(start, next_loc, capture_mod));
				}
				move_piece (king, next_loc, start,board);
				board[next_loc] = temp;
			}
		}
	}
}
/**
 * Note: this method is a copy of is_in_check(), using the same "octopus" idea. This method does not return
 * pawns that can reach the square via capture or advance.
 */
inline vector <_piece> position::reachable_pieces (_location square, _property map){
	vector<_piece> to_return;
	_piece obstruct;
	_location current = square;

	for(int i = 0; i < 8; ++i) {
		current = square + RADIAL[i];
		while((current & 0x88) == 0) {
			obstruct = piece_search(current);
			if(obstruct != 0) {
				if(get_piece_color(obstruct) == map) break;
				switch(get_piece_type(obstruct)) {
				case ROOK  : if(i < 4) to_return.push_back(obstruct); break;
				case BISHOP: if(i > 3) to_return.push_back(obstruct); break;
				case QUEEN : to_return.push_back(obstruct); break;
				}
				break;
			}
			current += RADIAL[i];
		}
	}
	for(int i = 0; i < 8; ++i){
		obstruct = piece_search(square + KNIGHT_MOVE[i], !map);
		if (get_piece_type(obstruct) == KNIGHT) to_return.push_back(obstruct);
	}
	return to_return;
}
inline _piece* position::create_guardian_map (_property col, _property opp_col){
	_piece *turn_map = opp_col ? white_map : black_map;
	_piece *guardian_map = new _piece [16];	/* initialises all indices to zero_piece */
	_piece obstruct, guardian = zero_piece, attacker = zero_piece;
	_property obstruct_type;
	_location king = get_piece_location(turn_map[0]), current;

	for (int i = 0; i < 8; ++i){
		current = king + RADIAL[i];
		bool guardian_found = false, set = false;
		while ((current & 0x88) == 0){
			obstruct = piece_search(current);
			if (obstruct != zero_piece){
				if (!guardian_found){
					if (get_piece_color(obstruct) == opp_col) break;
					guardian = obstruct;
					guardian_found = true;
				} else {
					if (get_piece_color(obstruct) == col) break;
					else {
						obstruct_type = get_piece_type(obstruct);
						switch (obstruct_type){
						case ROOK: if (i < 4) {	attacker = obstruct; set = true;	} break;
						case BISHOP: if (i > 3) {	attacker = obstruct; set = true;	} break;
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
inline char position::get_difference(_location loc, _location k_loc) {
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
	board[get_piece_location(to_swap)] = &victim;
	to_swap = zero_piece;
}
inline int position::get_last_index (_piece* map){
	for (int i = 15; i > 0; --i) if (map[i] != zero_piece) return i;
	return -1;
}
inline _piece position::get_assailant (_piece *guardian_map, _piece piece_in_question){
	for (int i = 0; i < 8; ++i) if (guardian_map[i] == piece_in_question) return guardian_map[i+8];
	return zero_piece;
}
inline void position::pawn_capture_reach (vector <_piece> &v, _location target, _property threat_map){
	const char* opp_pawn_atk = threat_map ? BLACK_PAWN_ATTACK : WHITE_PAWN_ATTACK;
	_piece obstruct1 = piece_search(target - opp_pawn_atk[0], threat_map);
	_piece obstruct2 = piece_search(target - opp_pawn_atk[1], threat_map);
	if ((obstruct1 != 0) && (get_piece_type(obstruct1) == PAWN)) v.push_back(obstruct1);
	if ((obstruct2 != 0) && (get_piece_type(obstruct2) == PAWN)) v.push_back(obstruct2);
}
}
