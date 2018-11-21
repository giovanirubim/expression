#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <map>
#include <cmath>
#include <string>

class ExprNode {
	public:
		virtual double calc() = 0;
		virtual std::string toString() = 0;
};

class ExprNodeConst: public ExprNode {
	double value;
	public:
		ExprNodeConst(double value) {
			this->value = value;
		}
		double calc() {
			return value;
		}
		std::string toString() {
			std::string str = std::to_string(value);
			if (str.find(".") == std::string::npos) {
				return str;
			}
			int i = str.length() - 1;
			while (str[i] == '0') --i;
			if (str[i] == '.') --i;
			return str.substr(0, i + 1);
		}
};

class ExprNodeNeg: public ExprNode {
	ExprNode* tree;
	public:
		ExprNodeNeg(ExprNode* tree) {
			this->tree = tree;
		}
		double calc() {
			if (tree) return - tree->calc();
			return 0;
		}
		std::string toString() {
			if (tree) return "(-" + tree->toString() + ")";
			return "(-#)";
		}
		~ExprNodeNeg() {
			if (tree) delete tree;
		}
};

class ExprNodeAbs: public ExprNode {
	ExprNode* tree;
	public:
		ExprNodeAbs(ExprNode* tree) {
			this->tree = tree;
		}
		double calc() {
			double value = tree ? tree->calc() : 0;
			return value >= 0 ? value : -value;
		}
		std::string toString() {
			if (tree) return "|" + tree->toString() + "|";
			return "|#|";
		}
		~ExprNodeAbs() {
			if (tree) delete tree;
		}
};

class ExprNodeVar: public ExprNode {
	std::string id;
	double value;
	const double* ref;
	char type;
	public:
		ExprNodeVar(std::string id) {
			this->id = id;
			value = 0;
			ref = nullptr;
			type = '\0';
		}
		void setVal(double value) {
			this->value = value;
			type = 'v';
		}
		void setRef(const double* ref) {
			this->ref = ref;
			type = 'r';
		}
		double calc() {
			if (type == 'v') return value;
			if (type == 'r') return *ref;
			return 0;
		}
		std::string toString() {
			return id;
		}
};

class ExprNodeOpr {
	char chr;
	ExprNode* a;
	ExprNode* b;
	public:
		ExprNodeOpr(char chr, ExprNode* a, ExprNode* b) {
			this->chr = chr;
			this->a   = a;
			this->b   = b;
		}
		double calc() {
			double val_a = a ? a->calc() : 0;
			double val_b = b ? b->calc() : 0;
			switch (chr) {
				case '+': return val_a + val_b;
				case '-': return val_a - val_b;
				case '*': return val_a * val_b;
				case '/': return val_a / val_b;
				case '^': return pow(val_a, val_b);
			}
		}
		std::string toString() {
			std::string str = "(";
			str += a ? a->toString() : "#";
			str += chr;
			str += b ? b->toString() : "#";
		}
		~ExprNodeOpr() {
			if (a) delete a;
			if (b) delete b;
		}
};

#endif
