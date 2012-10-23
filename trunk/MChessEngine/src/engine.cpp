/*
 * engine.cpp
 * ============================================
 * (c) Spark Team, Oct 2012.
 * The Spark Team reserves all intellectual rights to the following source code.
 * The code may not be distributed or modified for personal use, except with the
 * express permission of a team member.
 * ============================================
 * The main class.
 */
#include <iostream>
#include <ctime>
#include "myriad.h"
#include <set>

using namespace myriad;
using namespace std;

myriad::round table(1);					/* Do not take up so much memory! */
position current_position;

unsigned int captures = 0, mates = 0, checks = 0, ep = 0, castle = 0, promo = 0;

void divide(int depth);
void perft(int depth, bool serial, bool debug);
unsigned long perft_benchmark(int depth);
unsigned long perft_debug (int depth, _move previous);

int main() {
	cout << "Welcome to Myriad Standalone Utility" << endl;
	cout << "~~Myriad (c) Team Spark~~" << endl;
	cout << "**Utility last updated: 10 Sep. 2012**" << endl << endl;
	cout << "Input 'help' for help menu." << endl;
	cout << "-------------------------------" << endl;
	cout << left;	/* left align formatting */

	string input;
	do {
		cout << endl <<  ">> ";
		getline(cin, input);
		stringstream ss (input);
		string command_name, arguments;
		getline(ss, command_name, ' ');
		cout << endl;
		if (!command_name.compare("set_board")){
			string fen;
			while (!ss.eof()){
				getline (ss, arguments);
				fen += arguments;
			}
			current_position.fromFen(fen);
			cout << "<< The position has been set to the following position: " << endl;
			cout << current_position.display_board() << endl;
		} else if (!command_name.compare("perft")){
			getline(ss, arguments, ' ');
			int depth;
			stringstream(arguments) >> depth;
			bool serial = false, debug = false;
			while (!ss.eof()){
				getline(ss, arguments, ' ');
				if (!arguments.compare("s") || !arguments.compare("ser") || !arguments.compare("+s") ||
					!arguments.compare("serial")) serial = true;
				else if (!arguments.compare("d") || !arguments.compare("debug") || !arguments.compare("+d") ||
					!arguments.compare("deb")) debug = true;
			}
			cout << "<< ---------------Perf. Test Start---------------" << endl;
			if (debug) cout << "<< Depth\t" << setw(12) << "Nodes" << setw(7) << "Kill" << setw(7) << "Mates"
							<< setw(7) << "Ep" << setw(7) << "Checks" << setw(7) << "Promo" << setw(7) << "Castle"
							<< endl;
			else cout << "<< Depth\t" << setw(12) << "Nodes" << setw(7) << "Time(ms)" << "\tkN/s" << endl;
			perft(depth, serial, debug);
			cout << "<< ----------------Perf. Test End----------------" << endl;
		} else if (!command_name.compare("make_move")){
			getline(ss, arguments, ' ');
			_move move;
			bool legal = false;
			if (arguments[0] == 'x')
				istringstream(arguments.substr(1,string::npos)) >> std::hex >> move >> std::dec;
			else if (arguments[1] == 'x')
				istringstream(arguments.substr(2,string::npos)) >> std::hex >> move >> std::dec;
			else istringstream(arguments) >> move;
			vector <_move> *all_moves = current_position.move_gen();
			for (unsigned int i = 0; i < all_moves->size(); ++i){
				if (all_moves->operator[](i) == move){
					legal = true;
					break;
				}
			}
			delete all_moves;
			if (legal){
				cout << "<< Move Made: " << move_to_string(move, current_position) << endl;
				current_position.make_move(move);
				cout << "<< Resultant Position: " << endl << endl;
				cout << current_position.display_board() << endl;
			} else cout << "<< An illegal move was entered!" << endl;
		} else if (!command_name.compare("help")){
			cout << "<< Currently implemented features include: " << endl;
			cout << "<< " << endl;
			cout << "<< display-> displays the current position." << endl;
			cout << "<< divide <depth>-> calls divide for the current position to a specified depth." << endl;
			cout << "<< exit-> closes this debug utility." << endl;
			cout << "<< hash_sz <size>-> Sets the size of the hash table." << endl;
			cout << "<< make_move <move>-> makes a move and displays the resultant position." << endl;
			cout << "<< \t make_move x<moveh>-> move specified in hexadecimal." << endl;
			cout << "<< move_gen-> calls the move_gen function against the current position. " << endl;
			cout << "<< perft <depth>" << endl;
			cout << "<< \t perft <depth>-> calls perft to a specified depth." << endl;
			cout << "<< \t perft <depth> (+s)-> all depths up to the specified depth." << endl;
			cout << "<< \t perft <depth> (+d)-> displays debug numbers instead of time." << endl;
			cout << "<< \t piece_at <location>-> prints the type of the piece at a location." << endl;
			cout << "<< rel_hashes-> displays all hashes relevant to the current position." << endl;
			cout << "<< set_board <fenstring>-> sets the current position." << endl;
		} else if (!command_name.compare("divide")){
			getline(ss, arguments, ' ');
			int depth;
			stringstream(arguments) >> depth;
			if (depth >= 2){
				cout << "<< ---------------Divide Test Start---------------" << endl;
				cout << "<< " << setw(10) << "Move" << setw(12) << "Nodes" << "\tResulting FEN" << endl;
				divide(depth);
				cout << "<< ----------------Divide Test End----------------" << endl;
			} else cout << "<< Cannot perform divide with a depth lower than 2!" << endl;
		} else if (!command_name.compare("move_gen")){
			cout << "<< ---------------move_gen Start---------------" << endl;
			cout << "<< The legal move(s) in the current position are: ";
			vector <_move>* moves = current_position.move_gen();
			int size = moves->size();
			for (int i = 0; i < size; ++i) {
				if (i % 8 == 0) cout << endl << "<< ";
				cout << move_to_string(moves->operator[](i), current_position) << ". ";
			}
			delete moves;
			cout << endl << "<< ----------------move_gen End----------------" << endl;
		} else if (!command_name.compare("rel_hashes")){
			cout << "<< ----------------Relevant Hash Numbers----------------" << endl;
			for (int i = 0; i < 16; ++i){
				_piece r = current_position.white_map[i];
				if (r == zero_piece) break;
				int index = get_index(get_piece_location(r), get_piece_type(r), WHITE);
				cout << "<< " + piece_to_string(r) << setw (15) << ": xor index = " << index <<
						", xor value = " << xor_values[index] << endl;
			}
			for (int i = 0; i < 16; ++i){
				_piece r = current_position.black_map[i];
				if (r == zero_piece) break;
				int index = get_index(get_piece_location(r), get_piece_type(r), BLACK);
				cout << "<< " + piece_to_string(r) << setw (15) << ": xor index = " << index <<
						", xor value = " << xor_values[index] << endl;
			}
			_location epsq = get_epsq(current_position.details);
			assert(EPSQ_HASH_OFFSET + x88to64(epsq) < 849 && EPSQ_HASH_OFFSET + x88to64(epsq) >= 0);
			cout << "<< epsq = " << location_to_string(epsq) << ", xor value = "
				 << xor_values[EPSQ_HASH_OFFSET + x88to64(epsq)] << endl;
			_property castle_four = get_epsq(current_position.details);
			assert(CASTLE_HASH_OFFSET + castle_four < 849 && CASTLE_HASH_OFFSET + castle_four>= 0);
			cout << "<< castle rights = " << get_castle_nibble (castle_four) << setw(15) << ": xor index = "
				 << CASTLE_HASH_OFFSET + castle_four << ", xor value = "
				 << xor_values[CASTLE_HASH_OFFSET + castle_four] << endl;
			if (is_black_to_move(current_position.details))
				cout << "<< Black to move, xor value = " << xor_values[STM_INDEX] << endl;
			cout << endl << "<< --------------Relevant Hash Numbers End--------------" << endl;
		} else if (!command_name.compare("hash_sz")){
			getline(ss, arguments, ' ');
			int tb_size;
			stringstream(arguments) >> tb_size;
			table = tb_size;
			cout << "Hash table set to " << tb_size << " bits per entry." << endl;
		} else if (!command_name.compare("display")){
			cout << current_position.display_board() << endl;
		} else if (!command_name.compare("exit")){
			cout << "Debug Utility Closing..." << endl;
		} else if (!command_name.compare("fen")){
			cout << "<< FEN = " << (string) current_position << endl;
		} else cout << "<< Input not recognized. Input 'help' for the help menu." << endl;
	} while (input.compare("exit"));
}
/* Perft and divide utilities */
void divide(int depth){
	vector <_move>* moves = current_position.move_gen();
	int n_moves = moves->size();
	unsigned long total = 0;
	_property detail = current_position.details;
	_zobrist hash = current_position.hash_key;
	for (int i = 0; i < n_moves; ++i){
		cout << "<< " << setw(9) << move_to_string(moves->operator[](i), current_position);
		current_position.make_move(moves->operator[](i));
		unsigned long nodes = perft_benchmark(depth - 1);
		cout << setw(12) << nodes << "\t" << (string) current_position << endl;
		total += nodes;
		current_position.unmake_move(moves->operator[](i), detail, hash);
	}
	delete moves;
	cout << "<< Number of branches divided: " << n_moves << endl;
	cout << "<< Total number of positions: " << total << endl;
}
void perft (int depth, bool serial, bool debug){
	int start = serial ? 1 : depth;
	unsigned long nodes;
	for (int i = start; i <= depth; ++i){
		if (debug) {
			captures = 0;
			mates = 0;
			ep = 0;
			checks = 0;
			castle = 0;
			promo = 0;
			nodes = perft_debug (i, 0);
			cout << "<< " << i << "\t\t" << setw(12) << nodes << setw(7) << captures << setw(7) <<  mates
				 << setw(7) << ep << setw(7) << checks << setw(7) << promo << setw(7) << castle << endl;
		} else {
			clock_t start = clock();
			nodes = perft_benchmark(i);
			int diff = (1000 * (clock() - start)) / CLOCKS_PER_SEC;
			cout << "<< " << i << "\t\t" << setw(12) << nodes << setw (7) << diff << "\t";
			if (diff == 0) cout << "?" << endl;
			else cout << nodes / (double) diff << endl;
		}
	}
}
unsigned long perft_benchmark(int depth){
	if (depth == 1) {
		vector<_move>* moves = current_position.move_gen();
		unsigned long size = moves->size();
		delete moves;
		return size;
	}
	unsigned long nodes = 0, n_moves;
	_property details = current_position.details;
	_zobrist hash = current_position.hash_key;
	vector <_move>* moves = current_position.move_gen();
	n_moves = moves->size();
	for (unsigned int i = 0; i < n_moves; ++i){
		current_position.make_move(moves->operator[](i));
		nodes += perft_benchmark(depth - 1);
		current_position.unmake_move(moves->operator[](i), details, hash);
	}
	delete moves;
	return nodes;
}
unsigned long perft_debug (int depth, _move prev_move){
	if (depth == 0){
		_property modifier = get_move_modifier(prev_move);
		if (current_position.is_in_check()) ++checks;
		vector<_move>* moves = current_position.move_gen();
		if (moves->size() == 0) ++mates;
		delete moves;
		switch (modifier){
		case WKS_CASTLE: case BKS_CASTLE: case WQS_CASTLE: case BQS_CASTLE: ++castle; break;
		case EN_PASSANT: ++captures; ++ep; break;
		case DOUBLE_ADVANCE: case 0: break;
		default:
			if (modifier < 10) ++promo;
			else {
				if ((modifier >> EIGHT_SH) != 0) ++promo;
				++captures;
			}
			break;
		}
		return 1;
	}
	unsigned long nodes = 0, n_moves;
	_property details = current_position.details;
	_zobrist hash = current_position.hash_key;
	vector <_move>* moves = current_position.move_gen();
	n_moves = moves->size();
	for (unsigned int i = 0; i < n_moves; ++i){
		current_position.make_move(moves->operator[](i));
		nodes += perft_debug(depth - 1, moves->operator[](i));
		current_position.unmake_move(moves->operator[](i), details, hash);
	}
	delete moves;
	return nodes;
}
