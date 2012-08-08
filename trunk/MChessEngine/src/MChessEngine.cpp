//============================================================================
// Name        : MChessEngine.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "myriad.h"

using namespace myriad;
using namespace std;

position current_position;

int main() {
	cout << "Welcome to Myriad Standalone Utility" << endl;
	cout << "~~Myriad (c) Team Spark~~" << endl;
	cout << "**Utility last updated: 7 Aug. 2012**" << endl << endl;
	cout << "<<Input 'help' for help menu." << endl;
	cout << "-------------------------------" << endl;

	string input;
	do {
		getline(cin, input);
		stringstream ss (input);
		string command_name, arguments;
		getline(ss, command_name, ' ');
		if (!command_name.compare("set_board")){

		} else if (!command_name.compare("perft")){
			getline(ss, arguments, ' ');
			int depth = atoi(arguments.c_str());
			bool serial = false, debug = false;
			while (!ss.eof()){
				getline(ss, arguments, ' ');
				if (!arguments.compare("s") || !arguments.compare("ser") || !arguments.compare("+s") ||
					!arguments.compare("serial")) serial = true;
				else if (!arguments.compare("d") || !arguments.compare("debug") || arguments.compare("+d"))
					debug = true;
			}
		} else if (!command_name.compare("make_move")){
			getline(ss, arguments, ' ');
			_move move = atoi(arguments.c_str());
			current_position.make_move(move);
		} else cout << "<< Input not recognized. Input 'help' for the help menu.";
	} while (!input.compare("exit"));

	position p2(string("rnbqkbnr/pp3ppp/8/2p1p3/3pP3/3P2P1/PPP1NPBP/R1BQK1NR b KQkq - 1 6"));
	cout << endl << (string)p2 << endl;

	//promotion to queen
	//p.make_move();
	return 0;
	return nodes;
}
void Divide (int depth, position& p){
	vector <_move> moves = p.move_gen();
	int n_moves = moves.size();
	for (int i = 0; i < n_moves; i++){
			p.make_move(moves[i]);
			cout<<Perft(depth , p);
			p.unmake_move(moves[i], p.details);
		}
}
