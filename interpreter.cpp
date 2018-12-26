#include <iostream>
#include <vector>

namespace UweAsm { namespace Interpreter { 
	template <typename T> T pop(std::vector<T>& v) {
		T back = v.back();
		v.pop_back();
		return back;
	}

	struct StackItem {
		unsigned loc;
		unsigned varsSize;
		unsigned varsLoc;
		unsigned varNum;
	};

	struct State {
		bool inMainIO;
		unsigned carry, loc, varsSize, varsLoc;
		std::vector<unsigned> mainIO, vars, code;
		std::vector<StackItem> stack;
		std::vector<std::vector<unsigned>> lists;

		State(std::vector<unsigned> code): code(code) {
			inMainIO = false;
			carry = 0;
			loc = 0;
			varsSize = 0;
			varsLoc = 0;
			mainIO = {};
			vars = {};
			stack = {{(unsigned)-1,0,0,0}};
			lists = {};
		}
	};

	const unsigned beginAnd = 0xC0000000;
	const unsigned endAnd = 0x3FFFFFFF;

	#define func(a) [](State& state){ a; }
	#define efunc(a) func(return a;)
	#define numEfunc(a) func(return ((a) | beginAnd);)
	#define arg(n) (state.code[state.loc+2+n])
	#define vararg(n) (state.vars[state.varsLoc+arg(n)-1])
	#define normarg(n) ((arg(n) & beginAnd) ? arg(n) :  vararg(n))
	#define lsarg(n) state.lists[vararg(n)]
	#define numarg(n) normarg(n) & endAnd
	#define listfunc(a) func(std::vector<unsigned> ls; a; state.lists.push_back(ls); return (unsigned)state.lists.size()-1;)

	const std::vector<unsigned(*)(State&)> builtins { //TODO more of these
		numEfunc(numarg(0)+numarg(1)), // add
		numEfunc(numarg(0)-numarg(1)), // sub
		numEfunc(numarg(0)==numarg(1)), // eq
		numEfunc(numarg(0)!=numarg(1)), // neq
		numEfunc((int)numarg(0)>(int)numarg(1)), // gt
		numEfunc((int)numarg(0)<(int)numarg(1)), // lt
		numEfunc((int)numarg(0)>=(int)numarg(1)), // gteq
		numEfunc((int)numarg(0)<=(int)numarg(1)), // lteq
		numEfunc(numarg(0)>numarg(1)), // gtu
		numEfunc(numarg(0)<numarg(1)), // ltu
		numEfunc(numarg(0)>=numarg(1)), // gtequ
		numEfunc(numarg(0)<=numarg(1)), // ltequ
		numEfunc(numarg(0)<<numarg(1)), // lsh
		numEfunc(numarg(0)>>numarg(1)), // rsh
		efunc((unsigned)!numarg(0)), // not
		numEfunc(numarg(0)&&numarg(1)), // and
		numEfunc(numarg(0)||numarg(1)), // or
		numEfunc((bool)numarg(0)!=(bool)numarg(1)), // xor
		numEfunc(!(numarg(0)&&numarg(1))), // nand
		numEfunc(!(numarg(0)||numarg(1))), // nor
		numEfunc((bool)numarg(0)==(bool)numarg(1)), // xnor
		efunc(~numarg(0)), // bnot
		numEfunc(numarg(0)&numarg(1)), // band
		numEfunc(numarg(0)|numarg(1)), // bor
		numEfunc(numarg(0)^numarg(1)), // bxor
		numEfunc(~(numarg(0)&numarg(1))), // bnand
		numEfunc(~(numarg(0)|numarg(1))), // bnor
		numEfunc(~(numarg(0)^numarg(1))), // bxnor
		listfunc(
			for (int i=0; arg(i); i++)
				ls.push_back(normarg(i));
		), // ls
		listfunc(
			std::vector<unsigned> l;
			for (int i=0; arg(i); i++)
				l.push_back(normarg(i));
			state.lists.push_back(l);
			ls.push_back((unsigned) state.lists.size()-1);
		), // ls2d
		listfunc(
			int i = 0;
			for (; arg(i+1); i++)
				ls.insert(ls.begin(), normarg(i));
			ls.insert(ls.end(),lsarg(i).begin(),lsarg(i).end());
		), // insert
		listfunc(
			ls.insert(ls.begin(),lsarg(0).begin(),lsarg(0).end());
			for (int i=1; arg(i); i++)
				ls.insert(ls.end(), normarg(i));
		), // append
		efunc(lsarg(0)[numarg(1)]), // get
		listfunc(
			ls.insert(ls.begin(),lsarg(0).begin(),lsarg(0).end());
			ls[numarg(1) & endAnd] = normarg(2);
		), // set
		efunc((unsigned)lsarg(0).size()) // size
	};

	void runIO(unsigned io, State& state) {
		std::cout << std::hex << (io & endAnd) << std::endl; //TODO
	}

	void setVar(unsigned num, unsigned val, State& state) {
		if (state.vars.size() < state.varsLoc + num)
			state.vars.resize(state.varsLoc + num);
		if (num > state.varsSize) state.varsSize = num;
		state.vars[state.varsLoc + num - 1] = val;
	}

	bool mainLoop(State& state) {
		if (state.inMainIO) {
			runIO(pop(state.mainIO),state);
			return state.mainIO.size() > 0;
		} else {
			unsigned varNum = state.code[state.loc];
			unsigned cmd = state.code[state.loc+1];
			unsigned start = cmd & beginAnd; //type of cmd (first 2 bits)
			unsigned end = cmd & endAnd; //last n-2 bits
			if (start == 0)
				cmd = state.vars[state.varsLoc+cmd-1];
			unsigned result = cmd;
			if (start == 0x80000000) //builtin func
				result = builtins[end](state);
			if (start == 0x40000000) { //ptr func
				state.stack.push_back({state.loc,state.varsSize,state.varsLoc,varNum});
				state.varsLoc += state.varsSize;
				state.varsSize = 0;
				for (unsigned x = state.loc+2; state.code[x] != 0; x++)
					setVar(state.varsSize+1,state.code[x], state);
				state.loc = end;
			} else { //not ptr func, so safe to store returned value in variable/return
				if (varNum != 0) {
					setVar(varNum, result, state);
					for (unsigned x = 0; state.code[state.loc] != 0; x++) state.loc++;
					state.loc++;
				} else { //return
					StackItem s;
					while (s.varNum != 0) s = pop(state.stack);
					if (s.loc == (unsigned)-1) { //hit the bottom
						state.inMainIO = true;
						state.stack.push_back(s);
						state.mainIO.push_back(result);
					} else {
						state.loc = s.loc;
						state.varsLoc = s.varsLoc;
						state.varsSize = s.varsSize;
						setVar(s.varNum, result, state);
					}
				}
			}
			return true;
		}
	}
} }
