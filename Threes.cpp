/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++11 -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "solver.h"

int main(int argc, const char* argv[]) {
	std::cout << "2048-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;
	std::fstream fp1, fp2;
	int tile[6];
	fp1.open(argv[1],std::ios::in);
	fp2.open(argv[2],std::ios::out);
	
	std::string solve_args;
	int precision = 10;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--solve=") == 0){
			solve_args = para.substr(para.find("=") + 1);
		} else if (para.find("--precision=") == 0) {
			precision = std::stol(para.substr(para.find("=") + 1));
		}
	}
	solver solve(solve_args);
	board state;
	state_type type;
	std::cout << std::setprecision(precision);
	state_hint hint(state);
	//while (fp1 >> type >> tile[0] >> tile[1] >> tile[2] >> tile[3] >> tile[4] >> tile[5] >> hint) {
	while(fp1 >> type >> state >> hint){
		auto value = solve.solve(state, type);
		//fp2 << type << " " << tile[0] << " " << tile[1] << " " << tile[2] << " " << tile[3] << " " << tile[4] << " " << tile[5] << " " << hint;
		fp2 << type << " " << state << " " << hint;
		fp2 << " = " << value << std::endl;
	}

	fp1.close();
	fp2.close();
	return 0;
}
