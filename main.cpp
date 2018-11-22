#include "expression.h"
#include <string>
#include <iostream>
using namespace std;
double temp(const double args[]) {
	return pow(args[0], 0.5);
}
double temp2(const double args[]) {
	return pow(args[0], args[1]);
}
int main() {
	ExprNode* a = new ExprNodeConst(2);
	ExprNode* b = new ExprNodeConst(10);
	ExprNode* c = new ExprNodeOpr('+', a, b);
	c = new ExprNodeOpr('^', a, c);
	ExprNode* d = new ExprNodeNeg(c);
	ExprNode* e = new ExprNodeAbs(d);
	ExprNodeCall* f = new ExprNodeCall("sqrt");
	f->addArg(e);
	ExprNodeCall* g = new ExprNodeCall("pow");
	g->addArg(f);
	g->addArg(a);
	g->setCall("sqrt", temp);
	g->setCall("pow", temp2);
	ExprBytecode bytecode;
	g->addToBytecode(bytecode);
	cout << g->toString() << "\n";
	cout << g->calc() << "\n";
	cout << bytecode.calc() << "\n";
}
