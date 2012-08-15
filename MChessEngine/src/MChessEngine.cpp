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
#include<set>

using namespace myriad;
using namespace std;

position current_position;
position perft_position;
void Perft(int depth, bool serial, bool debug);
long Perft(int depth);

set<std::pair<position, int> > checkedPositions;


int main() {
	cout << "Welcome to Myriad Standalone Utility" << endl;
	cout << "~~Myriad (c) Team Spark~~" << endl;
	cout << "**Utility last updated: 7 Aug. 2012**" << endl << endl;
	cout << "<<Input 'help' for help menu." << endl;
	cout << "-------------------------------" << endl;
	cout << ">> ";

	string input;
	do {
		getline(cin, input);
		stringstream ss (input);
		string command_name, arguments;
		getline(ss, command_name, ' ');
		if (!command_name.compare("set_board")){
			string fen;
			while (!ss.eof()){
				getline (ss, arguments);
				fen += arguments;
			}
			position current_position(fen);
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

			cout << "---------------Perf. Test Start---------------" << endl;
			if (debug) cout << "<< Depth\tNodes\tCaptures\tMates\tEp\tChecks" << endl;
			else cout << "<< Depth\tNodes\tTime(s)\tkN/s" << endl;
			Perft(depth, serial, debug);
			cout << "----------------Perf. Test End----------------" << endl;
			cout << ">> ";


		} else if (!command_name.compare("make_move")){
			getline(ss, arguments, ' ');
			_move move;
			stringstream(arguments) >> move;
			cout << "<< Move Made: " << move_to_string(move, current_position) << endl;
			current_position.make_move(move);
			cout << "<< Resultant Position: " << endl;
			cout << current_position.get_graphical();
			cout << ">> ";
		} else if (!command_name.compare("help")){
			cout << "<< Currently implemented features include: " << endl;
			cout << "<< perft <depth>" << endl;
			cout << "<< \t perft <depth>-> calls perft for the current position to a specified depth." << endl;
			cout << "<< \t perft <depth> (+s)-> calls perft for all depths up to the specified depth." << endl;
			cout << "<< \t perft <depth> (+d)-> calls perft and displays debug numbers." << endl;
			cout << "<< make_move <move>-> makes a move and displays the resultant position." << endl;
			cout << ">> ";
		} else cout << "<< Input not recognized. Input 'help' for the help menu.";
	} while (input.compare("exit"));

	position p2(string("rnbqkbnr/pp3ppp/8/2p1p3/3pP3/3P2P1/PPP1NPBP/R1BQK1NR b KQkq - 1 6"));
	cout << endl << (string)p2 << endl;
} /*
int main (){
	position p;
	p.make_move(0x3212);
	vector <_move> moves = p.move_gen(), new_moves;
	for (unsigned int i = 0; i < moves.size(); i++){
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
void Perft (int depth, bool serial, bool debug){
	if (debug) {
		// XXX: Implement!
	}
	int start = serial ? 1 : depth;
	for (int i = start; i <= depth; i++){
		clock_t start = time(0);
		long nodes = Perft(i);
		clock_t end = time(0);
		double diff = difftime(start, end);
		cout << "   " << depth << "\t" << nodes << "\t" << diff << "\t" << nodes / diff << endl;
	}

	checkedPositions.erase(checkedPositions.begin(), checkedPositions.end());
}
long Perft(int depth){

	//returns pair of iterators where the first one leads to the first element
	//not greater than the argument
	//do not continue if a search at like or higher depth has been done or started
	if ( (*checkedPositions.equal_range(
					pair<position, int>(perft_position, depth)
			).first).first == perft_position) {
		return 0;
	}
	//insert returns a pair of iterator and bool, the bool which indicates
	//the success of the insertion
	//do not continue if this search at this depth has been done
	/*if ((checkedPositions.insert(
			std::pair<position, int>(perft_position, depth))).second
			== false) {
		return 0;
	}*/
	checkedPositions.insert(pair<position, int>(perft_position, depth));

	if (depth == 0) {

		return 1;
	}


	int nodes = 0, n_moves;
	_property details = perft_position.details;
	vector <_move> moves = perft_position.move_gen();
	n_moves = moves.size();
	for (int i = 0; i < n_moves; i++){
		perft_position.make_move(moves[i]);
		nodes += Perft(depth - 1);

		//cout << "depth: " << depth << " / nodes: " << nodes << endl;

		perft_position.unmake_move(moves[i], details);
	}
	return nodes;
}
