#include "expression.h"
#include <string>
#include <iostream>
using namespace std;

int main() {

	std::string str;
	ExprParser parser;

	cout << "Expressao: ";
	getline(cin, str);

	parser.parse(str);
	parser.std();

	Expr expr = parser.toExpr();
	cout << expr.calc(5) << "\n";

}
