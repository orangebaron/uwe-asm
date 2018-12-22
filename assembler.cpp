#include <iostream>
#include <string>
#include <map>
#include <vector>

using std::pair;
using std::map;
using std::vector;
using std::string;

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
	{ "+", 5 }
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

int main() {
	string code = "v1 i2 @c b+ rc cA ;";
	for (unsigned i: strToCode(code)) std::cout<<std::hex<<i<<' ';
	std::cout << std::endl;
	return 0;
}
