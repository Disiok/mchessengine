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
hay:: hay (position & base) : current(base) {
	create();
}
hay::hay(const hay& copy) : current(copy.current) {
	create();
	ind_top = copy.ind_top;
	std::copy(copy.variation, copy.variation + ind_top, variation);
}
void hay::create() {
	ind_top = 0;
	_stackel element = {current.hash_key, NULL_MOVE, current.details, UNKNOWN_SCORE, current.fullmove_clock};
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
inline _score hay::get_parent_score() { return variation[ind_top].score; }

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

//Multithreaded minimax
_score minimaxTh (hay& stack, bool color, int depth) {
	if (depth == 0) return eval(stack.current);


	vector <_move> all_moves = *stack.current.move_gen();
	vector <boost::shared_future<_score> > futures;

	for (unsigned int i = 0; i < all_moves.size(); i++){
		/*
		stack.push(all_moves[i]);
		sc = minimax(stack, !color, depth-1);
		if (i == 0) stack.parent_score (sc);
		else if (color && sc > stack.top().score) stack.parent_score(sc);
		else if (!color && sc < stack.top().score) stack.parent_score(sc);
		stack.pop();
		*/

		stack.push(all_moves[i]);

		// Compare with std::thread:
		// bit.ly/VxffLI
		boost::packaged_task<_score> pt(boost::bind(&myriad::minimaxTh,
													hay(stack),
													!color, depth - 1));

		stack.pop();

		boost::unique_future<_score> f = pt.get_future();
		futures.push_back(boost::shared_future<_score>(boost::move(f)));
		boost::thread t(boost::move(pt));
	}
	for (vector<boost::shared_future<_score> >::iterator i = futures.begin();
			i < futures.end(); ++i) {
		if ((*i).is_ready()) {
			_score sc = (*i).get();
			if ( stack.get_parent_score() == UNKNOWN_SCORE )
				stack.parent_score (sc);
			else if (color && sc > stack.top().score) stack.parent_score(sc);
			else if (!color && sc < stack.top().score) stack.parent_score(sc);
			i = futures.erase(i);
		}
		if (i == futures.end()) {
			i = futures.begin();
		}
	}
	return stack.top().score;
}
_score eval (position &p){
	return 10;					// XXX: Dummy function
}
}
