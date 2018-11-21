#include "expression.h"
#include <string>
#include <iostream>
using namespace std;
int main() {
	ExprNodeCall* node = new ExprNodeCall("pow");
	node->addArg(nullptr);
	cout << (new ExprNodeAbs(node))->toString();
}
