#include <iostream>
#include <string>
#include <map>
#include <vector>

using std::pair;
using std::map;
using std::vector;
using std::string;

namespace UweAsm { namespace Assembler {
	struct AsmCmd {
		enum { knownCmd, reference } type;
		union {
			unsigned cmd;
			string* ref;
		} val;
		AsmCmd(unsigned cmd) {
			type = knownCmd;
			val.cmd = cmd;
		}
		AsmCmd(string ref) {
			type = reference;
			val.ref = new string(ref);
		}
		unsigned convertToCmd(map<string, unsigned> refs) {
			if (type == reference)
				return 0x40000000 | refs.at(*val.ref);
			else
				return val.cmd;
		}
	};

	const map<string, unsigned> builtins {
		{ "add", 0 }, 
		{ "sub", 1 }, 
		{ "eq", 2 }, 
		{ "neq", 3 }, 
		{ "gt", 4 }, 
		{ "lt", 5 }, 
		{ "gteq", 6 }, 
		{ "lteq", 7 }, 
		{ "gtu", 8 }, 
		{ "ltu", 9 }, 
		{ "gtequ", 10 }, 
		{ "ltequ", 11 }, 
		{ "lsh", 12 }, 
		{ "rsh", 13 }, 
		{ "not", 14 }, 
		{ "and", 15 }, 
		{ "or", 16 }, 
		{ "xor", 17 }, 
		{ "nand", 18 }, 
		{ "nor", 19 }, 
		{ "xnor", 20 }, 
		{ "bnot", 21 }, 
		{ "band", 22 }, 
		{ "bor", 23 }, 
		{ "bxor", 24 }, 
		{ "bnand", 25 }, 
		{ "bnor", 26 }, 
		{ "bxnor", 27 }, 
		{ "ls", 28 }, 
		{ "ls2d", 29 }, 
		{ "insert", 30 }, 
		{ "append", 31 }, 
		{ "get", 32 }, 
		{ "set", 33 }, 
		{ "size", 34 }, 
	};

	AsmCmd wordToAsm(string word) {
		string rest = word.substr(1);
		switch (word.front()) {
			case ';': //semicolon -> 0
				return AsmCmd((unsigned)0);
			case 'i': //int literal
				return AsmCmd(0xC0000000 | stoi(rest));
			case 'c': //char literal (converted to int)
				return AsmCmd(0xC0000000 | rest[0]);
			case 'r': //reference
				return AsmCmd(rest);
			case 'v': //var #
				return AsmCmd(stoi(rest));
			case 'b': //built-in function
				return AsmCmd(0x80000000 | builtins.at(rest));
		}
	}

	pair<map<string, unsigned>, vector<AsmCmd>> wordsToRawAsm(vector<string> words) {
		map<string, unsigned> refs;
		vector<AsmCmd> lines;
		for (auto word: words) {
			if (word.front() == '@')
				refs[*new string(word.substr(1))] = lines.size();
			else
				lines.push_back(wordToAsm(word));
		}
		return {refs,lines};
	}

	vector<unsigned> wordsToCode(vector<string> words) {
		auto raw = wordsToRawAsm(words);
		vector<unsigned> code;
		for (auto instr: raw.second)
			code.push_back(instr.convertToCmd(raw.first));
		return code;
	}

	vector<string> strToWords(string str) {
		vector<string> words;
		bool inComment = false;
		bool inWord = false;
		for (char chr: str) {
			if (chr == ']') {
				inComment = false;
				inWord = false;
			} else if (chr == '[')
				inComment = true;
			else if (chr == ' ' || chr == '\t')
				inWord = false;
			else if (!inComment) {
				if (!inWord)
					words.push_back("");
				inWord = true;
				words.back().push_back(chr);
			}
		}
		return words;
	}

	vector<unsigned> strToCode(string str) { return wordsToCode(strToWords(str)); }
} }
