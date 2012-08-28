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
string position::exemplify(){
	string to_return = "<<     a   b   c   d   e   f   g   h \n";
	to_return += "<<   ----+---+---+---+---+---+---+----\n";
	bool stm = is_black_to_move(details);
	for (int i = 7; i >= 0; i--){
		stringstream ss;
		ss << (i + 1);
		to_return += "<< " + ss.str() + " |";
		for (int j = 0; j < 8; j++){
			_piece occupant = piece_search((i << 4) + j);
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
	stringstream ss2;
	ss2 << halfmove_clock;
	to_return += ", halfmove clock = " + ss2.str();
	_location epsq = get_epsq(details);
	if (epsq != 0) to_return += ", epsq = " + location_to_string(epsq);
	return to_return;
}
position::position(string fen) : details(0), halfmove_clock(0){
	for(unsigned char i = 0; i < 16; ++i) {
		black_map[i] = white_map[i] = 0;
	}
	istringstream ss;
	ss.str(fen);
	string row;
	// What index at which to next write a character
	// in the white map and black map, in that order
	unsigned char nextMapInsertionIndices[2]
	                                      = {1,1};
	//ranks 8-2 have a / as a terminator
	//rank 1 has just a space
	for(char rank = 7; rank >= 0; --rank) {
		//The terminator is discarded.
		if(rank > 0) {
			getline(ss, row, '/');
		}
		else {
			ss >> row;
		}

		//cout << endl << row << endl;

		//Go through space numbers and chess pieces one by one
		//Note the two accumulators:
		//x is which character of the string row
		//file is which file on which we work on the actual chessboard
		const size_t rowSize = row.size();
		for(unsigned char x = 0, file = 0; x < rowSize; ++x, ++file) {
			//Empty spaces: increment the file by the number of spaces
			if(row[x] >= '1' && row[x] <= '8') {
				stringstream ss2;
				ss2 << row[x];
				short numberOfSpaces;
				ss2 >> numberOfSpaces;
				file += numberOfSpaces - 1;
				continue;
			}

			//Lowercase is black
			const bool is_black = (row[x] >= 'b' && row[x] <= 'r');

			_piece* map = is_black ? black_map : white_map;

			//relies on the fact that
			//+32 gets lowercase
			if(row[x] == 'K' + ((is_black) * 32)) {
				map[0] = create_piece((rank << 4) + file, KING, is_black);
				continue;
			}
			else if(row[x] ==  'Q' + ((is_black) * 32)) {
				map[nextMapInsertionIndices[is_black]]
				    = create_piece((rank << 4) + file, QUEEN, is_black);
			}
			else if(row[x] ==  'B' + ((is_black) * 32)) {
				map[nextMapInsertionIndices[is_black]]
				    = create_piece((rank << 4) + file, BISHOP, is_black);
			}
			else if (row[x] == 'N' + ((is_black) * 32)) {
				map[nextMapInsertionIndices[is_black]]
				    = create_piece((rank << 4) + file, KNIGHT, is_black);
			}
			else if(row[x] == 'R' + ((is_black) * 32)) {
				map[nextMapInsertionIndices[is_black]]
				    = create_piece((rank << 4) + file, ROOK, is_black);
			}
			else if(row[x] ==  'P' + ((is_black) * 32)) {
				map[nextMapInsertionIndices[is_black]]
				    = create_piece((rank << 4) + file, PAWN, is_black);
			}

			++nextMapInsertionIndices[is_black];
		}

	}

	//stm
	char stm;
	ss >> stm;
	//details &= (~((_property)(1)));
	details ^= (stm == 'b');

	string castling_rights;
	ss >> castling_rights;

	//details &= (~((_property)(15 << 8)));
	//if some player does have castling rights
	if(castling_rights[0] != '-') {
		for(unsigned char i = 0; i < castling_rights.size(); ++i) {
			//Grants the castling rights
			//Perhaps it's not worth writing a function as of course
			//actual players are never granted castling rights
			switch(castling_rights[i]) {
			case 'K':
				details ^= ((1 << 8));
				break;
			case 'Q':
				details ^= ((1 << 9));
				break;
			case 'k':
				details ^= ((1 << 10));
				break;
			case 'q':
				details ^= ((1 << 11));
				break;
			}
		}
	}

	string epsq;
	ss >> epsq;
	//details &= (~((_property)(255 << 11)));
	if (epsq[0] != '-') {
		details ^= string_to_location(epsq) << EP_SH;
	}

	//halfmove clock
	unsigned short plycount;
	ss >> plycount;
	//details &= (~((_property)(127 << 1)));
	details ^= plycount << 1;

	//fullmove clock
	ss >> halfmove_clock;
}

// Display should indicate side to move.
position::operator string() {
	//First get the graphical representation, which is already
	//pieces\npieces\n etc
	string graphical = position::get_2d();

	string fen;
	stringstream ss;
	ss.str(graphical);

	for(char i = 0; i < 8; ++i) {
		string rank;
		ss >> rank;
		//. indicates an empty square. We have to replace .s with numbers
		unsigned char first_empty_file = rank.find('.');
		string empty_square_buffer;

		//for int to string conversion
		stringstream ss2;

		//If there are indeed empty spots in the rank
		if(first_empty_file != string::npos) {
			for(unsigned char j = first_empty_file; j < rank.size(); ++j) {
				if(rank[j] == '.') {
					empty_square_buffer += '.';
				}
				else {
					ss2 << empty_square_buffer.size();

					rank.replace(rank.find(empty_square_buffer),
							empty_square_buffer.size(),
							ss2.str());
					if ((j = rank.find('.') - 1) == string::npos) {
						break;
					}
					empty_square_buffer = "";
					ss2.str("");
				}
			}
		}

		// In case file h is empty
		if(empty_square_buffer.size() > 0) {
			ss2.clear();
			ss2.str("");
			ss2 << empty_square_buffer.size();
			rank.replace(rank.find(empty_square_buffer),
					empty_square_buffer.size(),
					ss2.str());
		}

		fen += rank;

		if(i != 7) {
			fen += '/';
		}
	}

	fen += ' ';
	//side to move
	fen += is_black_to_move(details) ? 'b' : 'w';
	fen += ' ';

	const unsigned char originalFenLength = fen.size();

	if(get_castle_right(details, WKS_CASTLE)) {
		fen += 'K';
	}

	if(get_castle_right(details, WQS_CASTLE)) {
		fen += 'Q';
	}

	if(get_castle_right(details, BKS_CASTLE)) {
		fen += 'k';
	}

	if(get_castle_right(details, BQS_CASTLE)) {
		fen += 'q';
	}

	//No castling rights, so the FEN's length hasn't changed, so add hyphen
	if(fen.size() == originalFenLength) {
		fen += '-';
	}

	fen += ' ';
	//en passant
	_location epsq;

	if((epsq = get_epsq(details))) {
		fen += location_to_string(epsq);
	}
	else {
		fen += '-';
	}

	fen += ' ';

	//Half move
	ss.clear();
	ss.str(string());
	ss << ((details >> 1) & 127);
	fen += ss.str() + ' ';

	//Fullmove
	ss.clear();
	ss.str(string());
	ss << halfmove_clock;
	fen += ss.str();

	return fen;
}

std::string position::get_2d() {
	char** board = new char*[8];
	for(int i = 0; i < 8; ++i) {
		board[i] = new char[8];
		for(int j = 0; j < 8; ++j) {
			board[i][j] = '.';
		}
	}
	stringstream ss;
	//plot white pieces
	for(int i = 0; i < 16; ++i) {
		_piece p = white_map[i];
		if(p == 0) break;
		int row = (p & 255) >> 4;
		int column = (p & 15);
		string typeString = piecetype_to_string((p >> 8) & 7);
		if(typeString == "") board[row][column] = 'P';
		else board[row][column] =  typeString.c_str()[0];
	}
	ss.clear();
	ss.str("");
	//black pieces
	for(int i = 0; i < 16; ++i) {
		_piece p = black_map[i];
		if(p == 0) break;
		int row = (p & 255) >> 4;
		int column = (p & 15);
		ss << ((p>>8) & 7);
		string typeString = piecetype_to_string((p >> 8) & 7);
		//make black lowercase
		if(typeString == "") board[row][column] = 'p';
		else board[row][column]
		                = typeString.c_str()[0] + 32;
	}
	//time ta draw
	string result;
	for(int i = 7; i >= 0; --i) {
		for(int j = 0; j < 8; ++j) {
			result += board[i][j];
		}
		result += '\n';
		delete[] board[i];
	}
	delete[] board;
	return result;
}
}
