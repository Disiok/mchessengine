/*
 * tree.cpp
 * ============================================
 * (c) Spark Team, Oct 2012.
 * The Spark Team reserves all intellectual rights to the following source code.
 * The code may not be distributed or modified for personal use, except with the
 * express permission of a team member.
 * ============================================
 * Successor to .tree package.
 */

#include "myriad.h"

namespace myriad{
hay:: hay (position & base) : ind_top (0), current(base) {
	_stackel element = {base.hash_key, NULL_MOVE, base.details, UNKNOWN_SCORE, base.fullmove_clock};
	variation [ind_top] = element;
}
void hay::push (_move m){
	_stackel element = {current.hash_key, m, current.details, UNKNOWN_SCORE, current.fullmove_clock};
	variation [++ind_top] = element;
	current.make_move(m);
}
_stackel hay::pop (){
	_stackel to_pop = variation[ind_top--];
	current.unmake_move(to_pop.edge, to_pop.detail, to_pop.prev_hash);
	return to_pop;
}
bool hay::repetition (){
	_zobrist root_hash = current.hash_key;
	for (int i = 0; i < ind_top; ++i) if (root_hash == variation[i].prev_hash) return true;
	return false;
}
inline _stackel hay::top () {	return variation[ind_top];	}
inline _stackel hay::get (int index){ return variation[index];	}
inline void hay :: parent_score (_score sc){ variation[ind_top].score = sc;	}

_score minimax (hay &stack, bool color, int depth){
	if (depth == 0) return eval(stack.current);
	vector <_move> all_moves = *stack.current.move_gen();
	_score sc;
	for (unsigned int i = 0; i < all_moves.size(); i++){
		stack.push(all_moves[i]);
		sc = minimax(stack, !color, depth-1);
		if (i == 0) stack.parent_score (sc);
		else if (color && sc > stack.top().score) stack.parent_score(sc);
		else if (!color && sc < stack.top().score) stack.parent_score(sc);
		stack.pop();
	}
	return stack.top().score;
}
_score eval (position &p){
	return 10;					// XXX: Dummy function
}
}
