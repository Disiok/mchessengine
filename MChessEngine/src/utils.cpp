/*
 * utils.cpp
 *
 * (c) Spark Team, July 2012
 */
#include "myriad.h"

namespace myriad{
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
		default:
			if (modifier < 10){
				return location_to_string (start) + "-" + location_to_string (end) + "=" +
					   piecetype_to_string(modifier - PROMOTE_OFFSET);
			} else {
				string s = p_type + location_to_string(start) + ":" + location_to_string(end);
				_property promote = modifier >> 6;
				if (promote != 0) s += "=" + piecetype_to_string (promote);
			}
			return p_type + location_to_string(start) + ":" + location_to_string(end);
		}
	}
	string piecetype_to_string (_property type){
		switch (type){
		case ROOK: return "R";
		case KNIGHT: return "N";
		case BISHOP: return "B";
		case QUEEN: return "Q";
		case KING: return "K";
		case PAWN: return "";
		default: return "Invalid Type";
		}
	}
	// XXX: (string) cast should convert to FEN String instead. Display should indicate side to move.
	position::operator string() {
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
		ss.str("");
		ss.clear();
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
			else board[row][column] =  typeString.c_str()[0] + 32;
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
