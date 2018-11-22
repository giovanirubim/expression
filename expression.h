#ifndef EXPRESSION_H
#define EXPRESSION_H

// #include <iostream>
// using namespace std;

#include <map>
#include <cmath>
#include <string>

// ---------------------------------------------------------------------------------------------- //
// Estrutura que armazenará um bytecode para a execução da expressão                              //
// ---------------------------------------------------------------------------------------------- //
#define EXPR_BYTECODE_CONST 0x01
#define EXPR_BYTECODE_ARG   0x02
#define EXPR_BYTECODE_REF   0x03
#define EXPR_BYTECODE_ABS   0x04
#define EXPR_BYTECODE_NEG   0x05
#define EXPR_BYTECODE_ADD   0x06
#define EXPR_BYTECODE_SUB   0x07
#define EXPR_BYTECODE_MUL   0x08
#define EXPR_BYTECODE_DIV   0x09
#define EXPR_BYTECODE_POW   0x0a
#define EXPR_BYTECODE_CALL  0x0b
class ExprBytecode {
	private:
		unsigned char blob[2048];
		unsigned char* blobPtr;
		double arg[256];
		int nArgs;
		unsigned char nextByte() {
			return *blobPtr++;
		}
		double nextVal() {
			double value = *(double*)(blobPtr);
			blobPtr += sizeof(double);
			return value;
		}
		void* nextRef() {
			void* ref = *(void**)(blobPtr);
			blobPtr += sizeof(void*);
			return ref;
		}
		double run() {
			switch (nextByte()) {
				case EXPR_BYTECODE_CONST: return nextVal();
				case EXPR_BYTECODE_ARG: return arg[nextByte()];
				case EXPR_BYTECODE_REF: return *(double*)nextRef();
				case EXPR_BYTECODE_ABS: {
					double value = run();
					return value >= 0 ? value : - value;
				};
				case EXPR_BYTECODE_NEG: return - run();
				case EXPR_BYTECODE_ADD: return run() + run();
				case EXPR_BYTECODE_SUB: {
					double value = run();
					return value - run();
				}
				case EXPR_BYTECODE_MUL: return run() * run();
				case EXPR_BYTECODE_DIV: {
					double value = run();
					return value / run();
				}
				case EXPR_BYTECODE_POW: {
					double value = run();
					return pow(value, run());
				}
				case EXPR_BYTECODE_CALL: {
					double(*ref)(const double[]);
					ref = (double(*)(const double[])) nextRef();
					int n = nextByte();
					double v[n];
					for (int i=0; i<n; ++i) {
						v[i] = run();
					}
					return ref(v);
				}
			}
			return 0;
		}
	public:
		ExprBytecode() {
			blobPtr = blob;
			nArgs = 0;
		}
		void addByte(unsigned char byte) {
			*blobPtr++ = byte;
		}
		void addVal(double value) {
			*(double*)(blobPtr) = value;
			blobPtr += sizeof(double);
		}
		void addRef(void* ref) {
			*(void**)(blobPtr) = ref;
			blobPtr += sizeof(void*);
		}
		void setArg(double value, int index) {
			arg[index] = value;
		}
		double calc() {
			blobPtr = blob;
			return run();
		}
};

// ---------------------------------------------------------------------------------------------- //
// Estruturas da árvore de operações, usada como estrutura auxiliar para o parsing de uma         //
// expressão                                                                                      //
// ---------------------------------------------------------------------------------------------- //
class ExprNode {
	public:
		virtual void setArg(std::string id, int index) {} // Define uma variável como argumento
		virtual void setVar(std::string id, double value) {} // Define um valor para uma variável
		virtual void setVar(std::string id, double* ref) {} // Define uma referência para uma
		virtual void setCall(std::string id, double(*ref)(const double[])) {}
		virtual void addToBytecode(ExprBytecode &bytecode) = 0;
		// variável
		virtual double calc() = 0; // (temporário) Calcula a sub-árvore que tem este nó como raiz
		virtual std::string toString() = 0; // (temporário) Formato textual da sub-árvore que tem
		// este nó como raiz
};
class ExprNodeConst: public ExprNode {
	double value;
	public:
		ExprNodeConst(double value) {
			this->value = value;
		}
		ExprNodeConst(std::string value) {
			this->value = std::stof(value);
		}
		void addToBytecode(ExprBytecode &bytecode) {
			bytecode.addByte(EXPR_BYTECODE_CONST);
			bytecode.addVal(value);
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
		void setArg(std::string id, int index) {
			if (tree) tree->setArg(id, index);
		}
		void setVar(std::string id, double value) {
			if (tree) tree->setVar(id, value);
		}
		void setVar(std::string id, double* ref) {
			if (tree) tree->setVar(id, ref);
		}
		void setCall(std::string id, double(*ref)(const double[])) {
			if (tree) tree->setCall(id, ref);
		}
		void addToBytecode(ExprBytecode &bytecode) {
			bytecode.addByte(EXPR_BYTECODE_NEG);
			tree->addToBytecode(bytecode);
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
		void setArg(std::string id, int index) {
			if (tree) tree->setArg(id, index);
		}
		void setVar(std::string id, double value) {
			if (tree) tree->setVar(id, value);
		}
		void setVar(std::string id, double* ref) {
			if (tree) tree->setVar(id, ref);
		}
		void setCall(std::string id, double(*ref)(const double[])) {
			if (tree) tree->setCall(id, ref);
		}
		void addToBytecode(ExprBytecode &bytecode) {
			bytecode.addByte(EXPR_BYTECODE_ABS);
			tree->addToBytecode(bytecode);
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
	int index;
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
		void setArg(std::string id, int index) {
			if (this->id == id) {
				this->index = index;
				type = 'a';
			}
		}
		void setVar(std::string id, double value) {
			if (this->id == id) {
				this->value = value;
				type = 'v';
			}
		}
		void setVar(std::string id, double* ref) {
			if (this->id == id) {
				this->ref = ref;
				type = 'r';
			}
		}
		void addToBytecode(ExprBytecode &bytecode) {
			switch (type) {
				case 'v':
					bytecode.addByte(EXPR_BYTECODE_CONST);
					bytecode.addVal(value);
				break;
				case 'r':
					bytecode.addByte(EXPR_BYTECODE_REF);
					bytecode.addRef((void*)ref);
				break;
				case 'a':
					bytecode.addByte(EXPR_BYTECODE_ARG);
					bytecode.addByte(index);
				break;
			}
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
class ExprNodeOpr: public ExprNode {
	char chr;
	ExprNode* a;
	ExprNode* b;
	public:
		ExprNodeOpr(char chr, ExprNode* a, ExprNode* b) {
			this->chr = chr;
			this->a   = a;
			this->b   = b;
		}
		void setArg(std::string id, int index) {
			if (a) a->setArg(id, index);
			if (b) b->setArg(id, index);
		}
		void setVar(std::string id, double value) {
			if (a) a->setVar(id, value);
			if (b) b->setVar(id, value);
		}
		void setVar(std::string id, double* ref) {
			if (a) a->setVar(id, ref);
			if (b) b->setVar(id, ref);
		}
		void setCall(std::string id, double(*ref)(const double[])) {
			if (a) a->setCall(id, ref);
			if (b) b->setCall(id, ref);
		}
		void addToBytecode(ExprBytecode &bytecode) {
			switch (chr) {
				case '+':
					bytecode.addByte(EXPR_BYTECODE_ADD);
				break;
				case '-':
					bytecode.addByte(EXPR_BYTECODE_SUB);
				break;
				case '*':
					bytecode.addByte(EXPR_BYTECODE_MUL);
				break;
				case '/':
					bytecode.addByte(EXPR_BYTECODE_DIV);
				break;
				case '^':
					bytecode.addByte(EXPR_BYTECODE_POW);
				break;
			}
			a->addToBytecode(bytecode);
			b->addToBytecode(bytecode);
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
			return 0;
		}
		std::string toString() {
			std::string str = "(";
			str += a ? a->toString() : "#";
			str += chr;
			str += b ? b->toString() : "#";
			return str;
		}
		~ExprNodeOpr() {
			if (a) delete a;
			if (b) delete b;
		}
};
class ExprArgNode {
	public:
		ExprNode* tree;
		ExprArgNode* next;
		ExprArgNode(ExprNode* tree) {
			this->tree = tree;
			next = nullptr;
		}
		~ExprArgNode() {
			if (next) delete next;
			if (tree) delete tree;
		}
};
class ExprNodeCall: public ExprNode {
	private:
		std::string id;
		struct {
			ExprArgNode* head;
			ExprArgNode* tail;
			int size;
		} list;
		double (*ref)(const double[]);
	public:
		ExprNodeCall(std::string id) {
			this->id = id;
			list.head = nullptr;
			list.tail = nullptr;
			list.size = 0;
			ref = nullptr;
		}
		void addArg(ExprNode* tree) {
			ExprArgNode* node = new ExprArgNode(tree);
			if (list.tail) {
				list.tail->next = node;
			} else {
				list.head = node;
			}
			list.tail = node;
			++ list.size;
		}
		void setArg(std::string id, int index) {
			ExprArgNode* node = list.head;
			while (node) {
				if (node->tree) node->tree->setArg(id, index);
				node = node->next;
			}
		}
		void setVar(std::string id, double value) {
			ExprArgNode* node = list.head;
			while (node) {
				if (node->tree) node->tree->setVar(id, value);
				node = node->next;
			}
		}
		void setVar(std::string id, double* ref) {
			ExprArgNode* node = list.head;
			while (node) {
				if (node->tree) node->tree->setVar(id, ref);
				node = node->next;
			}
		}
		void setCall(std::string id, double(*ref)(const double[])) {
			if (this->id == id) {
				this->ref = ref;
			}
			ExprArgNode* node = list.head;
			while (node) {
				if (node->tree) node->tree->setCall(id, ref);
				node = node->next;
			}
		}
		void addToBytecode(ExprBytecode &bytecode) {
			bytecode.addByte(EXPR_BYTECODE_CALL);
			bytecode.addRef((void*)ref);
			bytecode.addByte(list.size);
			ExprArgNode* node = list.head;
			while (node) {
				node->tree->addToBytecode(bytecode);
				node = node->next;
			}
		}
		double calc() {
			if (!ref) return 0;
			double v[list.size], *arg = v;
			ExprArgNode* node = list.head;
			while (node) {
				double val = node->tree ? node->tree->calc() : 0;
				*arg++ = val;
				node = node->next;
			}
			return ref(v);
		}
		std::string toString() {
			std::string str = id + "(";
			ExprArgNode* node = list.head;
			while (node) {
				if (node != list.head) str += ",";
				str += node->tree ? node->tree->toString() : "#";
				node = node->next;
			}
			return str + ")";
		}
		~ExprNodeCall() {
			if (list.head) delete list.head;
		}
};
#endif
