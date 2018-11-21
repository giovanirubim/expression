#include "expression.h"
#include <string>
#include <iostream>
using namespace std;
int main() {
	ExprNode* var = new ExprNodeVar("x");
	ExprNode* abs = new ExprNodeAbs(var);
	ExprNode* neg = new ExprNodeAbs(abs);
	neg->setVar("x", 70);
	cout << neg->calc();
}
