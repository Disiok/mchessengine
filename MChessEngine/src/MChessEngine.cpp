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
	position *p = new position();
	cout << "White Pieces" << endl;
	for (int i = 0; i < 8; i++) cout << string_descriptor(p->piece_search(i, WHITE)) << endl;
	cout << "Black Pieces" << endl;
	for (int i = 0; i < 8; i++) cout << string_descriptor(p->piece_search(i + 0x70, BLACK)) << endl;
	return 0;
}
