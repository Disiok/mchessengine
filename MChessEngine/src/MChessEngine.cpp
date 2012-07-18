//============================================================================
// Name        : MChessEngine.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "rules.h"

using namespace std;

int main() {
	position p;
	vector <_move> moves = p.move_gen();
	int size = moves.size();
	for (int i = 0; i < size; i++){
		cout << move_to_string(moves[i], p) << endl;
	}
	cout<< "side to move: "<< (p.details & 1) << endl;
	std::cout << (string)p;
	p.make_move(0x2414);
	p.make_move(0x5464);
	p.make_move(0x3424);
	p.make_move(0x4434);//piece of other colour
	p.make_move(0x5363);
	p.make_move(0x3424);//nonexistent
	p.make_move(0x5434);//flying pawn capture time
	p.make_move(0x3375);//move out black bishop
	p.make_move(0x5554); //some nonsense moves while I move out black's kingside pieces
	p.make_move(0x2376);
	p.make_move(0x5455);
	p.make_move(BKS_CASTLE << 16);//castle!!!!
	p.make_move(WKS_CASTLE << 16); //forget clearing the path...
	p.make_move(0x7674); //reset the king.
	p.make_move(0x0604);
	p.make_move(BQS_CASTLE << 16);
	p.make_move(WQS_CASTLE << 16);
	p.make_move( (EN_PASSANT<<16) + 0x4060);//this should reset by next turn
	p.make_move(0x3010);//derp move

	return 0;
}
