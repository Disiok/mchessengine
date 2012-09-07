/*
 * utils.cpp
 *
 * (c) Spark Team, July 2012
 */
#include "myriad.h"

namespace myriad {
string piece_to_string(_piece p) {
	if(p == zero_piece) return "Null";

	_location sq = get_piece_location(p);
	_property color = get_piece_color(p);
	_property type = get_piece_type(p);
	string s("");
	s += piecetype_to_string(type) + location_to_string(sq);
	s += (color == WHITE) ? "(w)" : "(b)";
	return s;
}
string move_to_string(_move m, position& p) {
	if(m == zero_piece) return "Null";

	_location start = get_move_start(m);
	_location end = get_move_end(m);
	_property modifier = get_move_modifier(m);
	string p_type = piecetype_to_string(get_piece_type(p.piece_search(start)));

	switch(modifier) {
	case 0: return p_type + location_to_string(start) + "-" + location_to_string(end);
	case WKS_CASTLE: return "0-0 (w)";
	case WQS_CASTLE: return "0-0-0 (w)";
	case BKS_CASTLE: return "0-0 (b)";
	case BQS_CASTLE: return "0-0-0 (b)";
	case EN_PASSANT: return location_to_string(start) + ":" + location_to_string(end) + " e.p.";
	case DOUBLE_ADVANCE: return location_to_string(start) + "-" + location_to_string(end);
	default:
		if(modifier < 10) {
			return location_to_string(start) + "-" + location_to_string(end) + "=" +
					piecetype_to_string(modifier - PROMOTE_OFFSET);
		} else {
			string s = p_type + location_to_string(start) + ":" + location_to_string(end);
			_property promote = modifier >> EIGHT_SH;
			if(promote != 0) s += "=" + piecetype_to_string(promote);
		}
		return p_type + location_to_string(start) + ":" + location_to_string(end);
	}
}
string piecetype_to_string(_property type) {
	switch(type) {
	case ROOK: return "R";
	case KNIGHT: return "N";
	case BISHOP: return "B";
	case QUEEN: return "Q";
	case KING: return "K";
	case PAWN: return "";
	}
	return "Invalid Type";
}
string position::display_board(){
	string to_return = "<<     a   b   c   d   e   f   g   h \n";
	to_return += "<<   ----+---+---+---+---+---+---+----\n";
	bool stm = is_black_to_move(details);
	for (int i = 7; i >= 0; i--){
		stringstream ss;
		ss << (i + 1);
		to_return += "<< " + ss.str() + " |";
		for (int j = 0; j < 8; j++){
			_piece occupant = piece_search((i << 4) + j);
			if (get_piece_type(occupant) == 0) cout << occupant << endl;
			if (occupant == zero_piece) to_return += "   |";
			else {
				_property col = get_piece_color(occupant);
				string type = piecetype_to_string(get_piece_type(occupant));
				if (type == "") type = col ? "p" : "P";
				else if (col) type[0] = type[0] - 'A' + 'a';
				to_return += " " + type + " |";
			}
		}
		if ((i == 8) && stm) to_return += " o";
		if ((i == 0) && !stm) to_return += " o";
		to_return += "\n<<  ";
		to_return += " ----+---+---+---+---+---+---+----\n";
	}
	stringstream ss1;
	ss1 << get_plycount(details);
	to_return += "<< plycount = " + ss1.str() + ", ";
	unsigned char original_length = to_return.size();
	if(get_castle_right(details, WKS_CASTLE)) to_return += 'K';
	if(get_castle_right(details, WQS_CASTLE)) to_return += 'Q';
	if(get_castle_right(details, BKS_CASTLE)) to_return += 'k';
	if(get_castle_right(details, BQS_CASTLE)) to_return += 'q';
	if(to_return.size() == original_length) to_return += "-";
	ss1.clear();
	ss1 << fullmove_clock;
	to_return += ", halfmove clock = " + ss1.str();
	_location epsq = get_epsq(details);
	if (epsq != 0) to_return += ", epsq = " + location_to_string(epsq);
	return to_return;
}
position::position(string fen){
	for(int i = 0; i < 16; ++i) black_map[i] = white_map[i] = 0;
	for(int i = 0; i < 136; ++i) board[i] = &zero_piece;
	istringstream ss;
	ss.str(fen);
	string row;
	unsigned int insertion_index[2] = {1,1};	/* Stores index to insert at */
	for(int rank = 7; rank >= 0; --rank) {
		if(rank > 0) getline(ss, row, '/');				/* '/' is the delimiter for ranks */
		else ss >> row;									/* No delimiter for first rank */
		/* Note: two counters: x is location in string, file is location in chess board */
		const size_t rowSize = row.size();
		for(unsigned int x = 0, file = 0; x < rowSize; ++x, ++file) {
			/* Handle empty space by advancing file counter. */
			if(row[x] >= '1' && row[x] <= '8') {
				stringstream ss2;
				ss2 << row[x];
				short number_of_spaces;
				ss2 >> number_of_spaces;
				file += number_of_spaces - 1;
				continue;
			}
			const bool is_black = (row[x] >= 'b' && row[x] <= 'r');
			_piece* map = is_black ? black_map : white_map;
			if(row[x] == 'K' + ((is_black) * 32)) { 						/* Lower/upper case conversion, +32 */
				map[0] = create_piece((rank << 4) + file, KING, is_black);	/* King must be at index 0 */
				continue;
			}
			else if(row[x] ==  'Q' + ((is_black) * 32))
				map[insertion_index[is_black]] = create_piece((rank << FOUR_SH) + file, QUEEN, is_black);
			else if(row[x] ==  'B' + ((is_black) * 32))
				map[insertion_index[is_black]] = create_piece((rank << FOUR_SH) + file, BISHOP, is_black);
			else if (row[x] == 'N' + ((is_black) * 32))
				map[insertion_index[is_black]] = create_piece((rank << FOUR_SH) + file, KNIGHT, is_black);
			else if(row[x] == 'R' + ((is_black) * 32))
				map[insertion_index[is_black]] = create_piece((rank << FOUR_SH) + file, ROOK, is_black);
			else if(row[x] ==  'P' + ((is_black) * 32))
				map[insertion_index[is_black]] = create_piece((rank << FOUR_SH) + file, PAWN, is_black);
			++insertion_index[is_black];
		}
	}
	/* Side to Move */
	char stm;
	ss >> stm;
	details ^= (stm == 'b');
	/* Castling Rights */
	string castling_rights;
	ss >> castling_rights;
	if(castling_rights[0] != '-') {
		for(unsigned char i = 0; i < castling_rights.size(); ++i) {
			/* Grant castling rights, +7 since CASTLE modifiers start from 1. */
			switch(castling_rights[i]) {
			case 'K': details ^= 1 << (WKS_CASTLE + 7); break;
			case 'Q': details ^= 1 << (WQS_CASTLE + 7); break;
			case 'k': details ^= 1 << (BKS_CASTLE + 7); break;
			case 'q': details ^= 1 << (BQS_CASTLE + 7); break;
			}
		}
	}
	/* En passant suqare */
	string epsq;
	ss >> epsq;
	if (epsq[0] != '-') details ^= string_to_location(epsq) << EP_SH;
	/* Halfmove Clock */
	short plycount;
	ss >> plycount;
	details ^= plycount << 1;
	/* Fullmove Clock */
	ss >> fullmove_clock;
	/* Set board array. */
	for (int i = 0; i < 16; i++) board[get_piece_location(white_map[i])] = &white_map[i];
	for (int i = 0; i < 16; i++) board[get_piece_location(black_map[i])] = &black_map[i];
	hash_key = create_initial_hash(*this);
}
position::operator string() {
	string fen;
	stringstream ss;
	/* Board */
	for(int i = 0; i < 8; ++i) {
		string rank;
		int n_blank = 0;
		for (int j = 0; j < 8; ++j){
			_location sq = (i << FOUR_SH) + j;
			_piece occupier = piece_search(sq);
			if (occupier == zero_piece) n_blank++;
			else{
				if (n_blank != 0){
					ss << n_blank;
					rank += ss.str();
					n_blank = 0;
					ss.clear();
				}
				string piece_type = piecetype_to_string(get_piece_type(occupier));
				if (get_piece_color(occupier)) piece_type[0] = piece_type[0] - 'A' + 'a';	/* To lower case */
				rank += piece_type;
			}
		}
		if (n_blank == 8) fen += "8";
		else fen += rank;
		if(i != 7) fen += '/';
	}
	fen += ' ';
	/* Side to move */
	fen += is_black_to_move(details) ? 'b' : 'w';
	fen += ' ';
	/* Castling Rights */
	unsigned char originalFenLength = fen.size();
	if(get_castle_right(details, WKS_CASTLE)) fen += 'K';
	if(get_castle_right(details, WQS_CASTLE)) fen += 'Q';
	if(get_castle_right(details, BKS_CASTLE)) fen += 'k';
	if(get_castle_right(details, BQS_CASTLE)) fen += 'q';
	if(fen.size() == originalFenLength) fen += '-'; /* No castling available -> add "-" */
	fen += ' ';
	/* En passant */
	_location epsq;
	if((epsq = get_epsq(details))) fen += location_to_string(epsq);
	else fen += '-';
	fen += ' ';
	/* Half move clock, i.e., number of half moves since last capture or pawn move. */
	ss.clear();
	ss.str(string());
	ss << get_plycount(details);
	fen += ss.str() + ' ';
	/* Full move clock, i.e., number of moves made in the game. */
	ss.clear();
	ss.str(string());
	ss << fullmove_clock;
	fen += ss.str();
	return fen;
}
}