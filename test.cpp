#include <iostream>
#include "interpreter.cpp"
#include "assembler.cpp"

int main() {
	string code = "code here";
	auto assembled = UweAsm::Assembler::strToCode(code);
	UweAsm::Interpreter::State state(assembled);
	while (UweAsm::Interpreter::mainLoop(state));
	return 0;	
}
