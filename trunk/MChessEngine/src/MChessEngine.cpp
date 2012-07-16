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
		cout << moves[i] << endl;
	}
}
