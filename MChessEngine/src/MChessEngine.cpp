//============================================================================
// Name        : MChessEngine.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <ctime>
#include "myriad.h"
#include <set>

using namespace myriad;
using namespace std;

position current_position;

int captures = 0, mates = 0, checks = 0, ep = 0, castle = 0, promo = 0;

void divide(int depth);
void perft(int depth, bool serial, bool debug);
long perft_benchmark(int depth);
long perft_debug (int depth, _move previous);

int main() {
	cout << "Welcome to Myriad Standalone Utility" << endl;
	cout << "~~Myriad (c) Team Spark~~" << endl;
	cout << "**Utility last updated: 7 Aug. 2012**" << endl << endl;
	cout << "Input 'help' for help menu." << endl;
	cout << "-------------------------------" << endl;

	string input;
	do {
		cout << ">> ";
		getline(cin, input);
		stringstream ss (input);
		string command_name, arguments;
		getline(ss, command_name, ' ');

		clock_t startTime = clock();

		if (!command_name.compare("set_board")){
			string fen;
			while (!ss.eof()){
				getline (ss, arguments);
				fen += arguments;
			}
			current_position = fen;
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
			if (debug) cout << "<< Depth\t\tNodes\tKill\tMates\tEp\tCheck\tPromo\tCastles" << endl;
			else cout << "<< Depth\tNodes\t\tTime(ms)\tkN/s" << endl;
			perft(depth, serial, debug);
			cout << "<< ----------------Perf. Test End----------------" << endl;
		} else if (!command_name.compare("make_move")){
			getline(ss, arguments, ' ');
			_move move;
			stringstream(arguments) >> move;
			cout << "<< Move Made: " << move_to_string(move, current_position) << endl;
			current_position.make_move(move);
			cout << "<< Resultant Position: " << endl;
			cout << current_position.get_graphical();
		} else if (!command_name.compare("help")){
			cout << "<< Currently implemented features include: " << endl;
			cout << "<< divide <depth>-> calls divide for the current position to a specified depth.";
			cout << "<< perft <depth>" << endl;
			cout << "<< \t perft <depth>-> calls perft for the current position to a specified depth." << endl;
			cout << "<< \t perft <depth> (+s)-> calls perft for all depths up to the specified depth." << endl;
			cout << "<< \t perft <depth> (+d)-> calls perft and displays debug numbers." << endl;
			cout << "<< make_move <move>-> makes a move and displays the resultant position." << endl;
		} else if (!command_name.compare("divide")){
			getline(ss, arguments, ' ');
			int depth;
			stringstream(arguments) >> depth;
			if (depth >= 2){
				cout << "<< ---------------Divide Test Start---------------" << endl;
				cout << "<< Depth\t\tNodes\tResulting FEN" << endl;
				divide(depth);
				cout << "<< ----------------Divide Test End----------------" << endl;
			} else cout << "<< Cannot perform divide with a depth lower than 2!" << endl;
		} else if (!command_name.compare("move_gen")){
			cout << "<< ---------------Movegen Start---------------" << endl;
			cout << "<< The legal move(s) in the current position are: ";
			vector <_move>* moves = current_position.move_gen();
			int size = moves->size();
			for (int i = 0; i < size; i++) {
				if (i % 8 == 0) cout << endl << "<< ";
				//Or we could be sane and just call ->at(i)
				cout << move_to_string(moves->operator[](i), current_position) << ". ";
			}
			delete moves;
			cout << endl << "<< ----------------Movegen End----------------" << endl;
		} else if (!command_name.compare("display")){
			cout << endl << current_position.get_graphical() << endl;
		} else if (!command_name.compare("exit")){
			cout << "Debug Utility Closing..." << endl;
		} else cout << "<< Input not recognized. Input 'help' for the help menu." << endl;

		std::cout << "Looped in " << fixed
			<< 1000.*(clock() - startTime)/CLOCKS_PER_SEC << "ms." << endl;
	} while (input.compare("exit"));
}
/*
int main (){
	position p;
	p.make_move(0x3212);
	vector <_move> moves = p.move_gen(), new_moves;
	for (unsigned int i = 0; i < moves->size(); i++){
		_property details = p.details;
		cout << move_to_string(moves[i], p) << endl;
		p.make_move(moves[i]);
		new_moves = p.move_gen();
		for (unsigned int j = 0; j < new_moves.size(); j++){
			_property new_details = p.details;
			cout << "\t" <<  move_to_string(new_moves[j], p) << endl;
			p.make_move(new_moves[j]);
			p.unmake_move(new_moves[j], new_details);
		}
		p.unmake_move(moves[i], details);
	}
} */
void divide(int depth){
	vector <_move>* moves = current_position.move_gen();
	int n_moves = moves->size();
	long total = 0;
	_property detail = current_position.details;
	for (int i = 0; i < n_moves; i++){
		cout << "<< " << move_to_string(moves->operator[](i), current_position) << "\t\t";
		current_position.make_move(moves->operator[](i));
		long nodes = perft_benchmark(depth - 1);
		cout << nodes << "\t" << (string) current_position << endl;
		total += nodes;
		current_position.unmake_move(moves->operator[](i), detail);
	}
	delete moves;
	cout << "<< Total number of positions: " << total << endl;
}
void perft (int depth, bool serial, bool debug){
	int start = serial ? 1 : depth;
	long nodes;
	for (int i = start; i <= depth; i++){
		if (debug) {
			captures = 0;
			mates = 0;
			ep = 0;
			checks = 0;
			castle = 0;
			promo = 0;
			nodes = perft_debug (i, 0);
			cout << "<< " << i << "\t\t" << nodes << "\t\t" << captures << "\t" <<  mates << "\t" << ep
				 << "\t" << checks << "\t" << promo << "\t" << castle << endl;
		}
		else {
			clock_t start = clock();
			nodes = perft_benchmark(i);
			double diff = (1000.0 * (clock() - start)) / CLOCKS_PER_SEC;
			cout << "<< " << i << "\t\t" << nodes << "\t\t" << diff
					<< "\t" << fixed <<  nodes / (diff) << endl;
		}
	}
}
long perft_benchmark(int depth){
	if (depth == 1) {
		vector<_move>* moves = current_position.move_gen();
		long size = moves->size();
		delete moves;
		return size;
	}
	int nodes = 0, n_moves;
	_property details = current_position.details;
	vector <_move>* moves = current_position.move_gen();
	n_moves = moves->size();
	for (int i = 0; i < n_moves; i++){
		current_position.make_move(moves->operator[](i));
		nodes += perft_benchmark(depth - 1);
		current_position.unmake_move(moves->operator[](i), details);
	}
	delete moves;
	return nodes;
}
long perft_debug (int depth, _move prev_move){
	if (depth == 0){
		_property modifier = get_move_modifier(prev_move);
		if (current_position.is_in_check()) checks ++;
		vector<_move>* moves = current_position.move_gen();
		if (moves->size() == 0) {
			mates++;
		}
		delete moves;
		switch (modifier){
		case WKS_CASTLE: case BKS_CASTLE: case WQS_CASTLE: case BQS_CASTLE: castle++; break;
		case EN_PASSANT: captures++; ep++; break;
		case DOUBLE_ADVANCE: case 0: break;
		default:
			if (modifier < 10) promo++;
			else {
				if ((modifier >> EIGHT_SH) != 0) promo++;
				captures++;
			}
			break;
		}
		return 1;
	}
	int nodes = 0, n_moves;
	_property details = current_position.details;
	vector <_move>* moves = current_position.move_gen();
	n_moves = moves->size();
	for (int i = 0; i < n_moves; i++){
		current_position.make_move(moves->operator[](i));
		nodes += perft_debug(depth - 1, moves->operator[](i));
		current_position.unmake_move(moves->operator[](i), details);
	}
	delete moves;
	return nodes;
}
