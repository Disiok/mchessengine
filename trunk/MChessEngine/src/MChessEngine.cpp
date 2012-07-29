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
	//some nonsense moves while I move out black's kingside pieces
	p.make_move(0x5554);
	p.make_move(0x2376);
	p.make_move(0x5455);
	p.make_move(BKS_CASTLE << 16);//castle!!!!
	p.make_move(0x2010);
	//this should become the en passant pawn for one turn
	p.make_move( DOUBLE_ADVANCE << 16 | 0x4060);
	p.make_move(0x3020);//derp move
	p.make_move(0x3040);
	//castle tests by clearing out all other pieces
	p.make_move(0x2606);
	p.make_move(0x7476);//reset black king
	p.make_move(0x2505);
	p.make_move(0x6373);
	p.make_move(0x1403);
	p.make_move(0x5572);
	p.make_move(WKS_CASTLE << 16);
	p.make_move(0x5171);
	p.make_move(0x0406);//reset white king
	p.make_move(BQS_CASTLE << 16);
	p.make_move(0x2202);
	p.make_move(0x5363);//derp move...
	p.make_move(0x2101);
	p.make_move(0x6353);
	p.make_move(WQS_CASTLE << 16);
	 //set up this pawn for en passant death
	p.make_move(DOUBLE_ADVANCE << 16 | 0x4767);
	//not a legal passant capture, but due to the flag, the en passant pawn
	//should now be captured
	p.make_move(EN_PASSANT << 16 | 0x3725);

	//switching the en passant pawn
	p.make_move(DOUBLE_ADVANCE << 16 | 0x4666);
	p.make_move(DOUBLE_ADVANCE << 16 | 0x3717);
	//the pawn has stepped on the bishop. from the graphic, it is not clear
	//whether the pawn or bishop was captured but considering piece_search
	//runs from the beginning, probably the bishop is gone. the white pawn
	//is under the black pawn which captured the white bishop
	p.make_move(EN_PASSANT << 16 | 0x3746);

	//promotions
	p.make_move( (PROMOTE_OFFSET + ROOK) << 16|0x7112);
	p.make_move(0x2030); //black random pawn move
	p.make_move( (PROMOTE_OFFSET + KNIGHT) << 16 | 0x7071);
	p.make_move(0x1020);
	p.make_move( (PROMOTE_OFFSET + QUEEN)<<16 | 0x7170);
	p.make_move((PROMOTE_OFFSET + QUEEN) << 16 | 0x0010);
	p.make_move( (PROMOTE_OFFSET + BISHOP)<<16 | 0x7071);

	//promotion to queen
	//p.make_move();
	return 0;

	int i = 3;
	if (i){
		cout << "Hi";
	}
}
