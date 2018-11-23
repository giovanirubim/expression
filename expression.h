#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <map>
#include <cmath>
#include <vector>
#include <string>

typedef double (*TExprFunction) (const double[]);

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
	const double* vArgs;
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
			case EXPR_BYTECODE_ARG: return vArgs[nextByte()];
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
				TExprFunction ref;
				ref = (TExprFunction) nextRef();
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
	void updateNArgs(int nArgs) {
		if (nArgs > this->nArgs) this->nArgs = nArgs;
	}
	double calc() {
		blobPtr = blob;
		return run();
	}
	double calc(const double* vArgs) {
		this->vArgs = vArgs;
		return calc();
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
	virtual void setCall(std::string id, TExprFunction ref) {}
	virtual void addVarsToMap(std::map <std::string, bool> &map) {}
	virtual void addCallsToMap(std::map <std::string, bool> &map) {}
	virtual int countArgs() {return 0;}
	virtual void addToBytecode(ExprBytecode &bytecode) = 0;
	// variável
	virtual double calc() = 0; // (temporário) Calcula a sub-árvore que tem este nó como raiz
	virtual std::string toString() = 0; // (temporário) Formato textual da sub-árvore que tem
	// este nó como raiz
};
class ExprNodeConst: public ExprNode {
private:
	double value;
public:
	ExprNodeConst(double value) {
		this->value = value;
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
private:
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
	void setCall(std::string id, TExprFunction ref) {
		if (tree) tree->setCall(id, ref);
	}
	void addVarsToMap(std::map <std::string, bool> &map) {
		if (tree) tree->addVarsToMap(map);
	}
	void addCallsToMap(std::map <std::string, bool> &map) {
		if (tree) tree->addCallsToMap(map);
	}
	int countArgs() {
		if (tree) return tree->countArgs();
		return 0;
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
private:
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
	void setCall(std::string id, TExprFunction ref) {
		if (tree) tree->setCall(id, ref);
	}
	void addVarsToMap(std::map <std::string, bool> &map) {
		if (tree) tree->addVarsToMap(map);
	}
	void addCallsToMap(std::map <std::string, bool> &map) {
		if (tree) tree->addCallsToMap(map);
	}
	int countArgs() {
		if (tree) return tree->countArgs();
		return 0;
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
private:
	std::string id;
	int index;
	double value;
	const double* ref;
	char type;
public:
	ExprNodeVar(std::string id) {
		this->id = id;
		index = -1;
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
	void addVarsToMap(std::map <std::string, bool> &map) {
		map[id] = type != '\0';
	}
	int countArgs() {
		return type == 'a' ? index + 1 : 0;
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
				bytecode.updateNArgs(index + 1);
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
private:
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
	void setCall(std::string id, TExprFunction ref) {
		if (a) a->setCall(id, ref);
		if (b) b->setCall(id, ref);
	}
	void addVarsToMap(std::map <std::string, bool> &map) {
		if (a) a->addVarsToMap(map);
		if (b) b->addVarsToMap(map);
	}
	void addCallsToMap(std::map <std::string, bool> &map) {
		if (a) a->addCallsToMap(map);
		if (b) b->addCallsToMap(map);
	}
	int countArgs() {
		int nA = a ? a->countArgs() : 0;
		int nB = b ? b->countArgs() : 0;
		return nA > nB ? nA : nB;
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
		return str + ")";
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
	TExprFunction ref;
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
	void setCall(std::string id, TExprFunction ref) {
		if (this->id == id) {
			this->ref = ref;
		}
		ExprArgNode* node = list.head;
		while (node) {
			if (node->tree) node->tree->setCall(id, ref);
			node = node->next;
		}
	}
	void addVarsToMap(std::map <std::string, bool> &map) {
		ExprArgNode* node = list.head;
		while (node) {
			if (node->tree) node->tree->addVarsToMap(map);
			node = node->next;
		}			
	}
	void addCallsToMap(std::map <std::string, bool> &map) {
		map[id] = this->ref != nullptr;
		ExprArgNode* node = list.head;
		while (node) {
			if (node->tree) node->tree->addCallsToMap(map);
			node = node->next;
		}			
	}
	int countArgs() {
		int res = 0;
		ExprArgNode* node = list.head;
		while (node) {
			int x = node->tree ? node->tree->countArgs() : 0;
			if (x > res) res = x;
			node = node->next;
		}		
		return res;	
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

// ---------------------------------------------------------------------------------------------- //
// Objeto que carrega um bytecode para a execução de uma expressão                                //
// ---------------------------------------------------------------------------------------------- //
class Expr {
	private:
		ExprBytecode bytecode;
		bool validFlag;
	public:
		Expr (ExprNode* tree) {
			if (validFlag = tree != nullptr) tree->addToBytecode(bytecode);
		}
		bool valid() {
			return validFlag;
		}
		double calc() {
			if (!validFlag) return 0;
			return bytecode.calc();
		}
		double calc(const double args[]) {
			if (!validFlag) return 0;
			return bytecode.calc(args);
		}
		double calc(double first) {
			if (!validFlag) return 0;
			return calc(&first);
		}
		double calc(double x, double y) {
			if (!validFlag) return 0;
			double args[2] = {x, y};
			return calc(args);
		}
};

// ---------------------------------------------------------------------------------------------- //
// Objeto que faz o parser de uma string contendo uma expressão para uma árvore de operações      //
// ---------------------------------------------------------------------------------------------- //
class ExprParser {
private:
	std::string src;
	int length;
	int index;
	int errorIndex;
	ExprNode* parsedTree;
	static double call_ln(const double args[]) {
		return log(args[0]);
	}
	static double call_log(const double args[]) {
		return log10(args[0]);
	}
	static double call_exp(const double args[]) {
		return exp(args[0]);
	}
	static double call_sin(const double args[]) {
		return sin(args[0]);
	}
	static double call_cos(const double args[]) {
		return cos(args[0]);
	}
	static double call_tan(const double args[]) {
		return tan(args[0]);
	}
	static double call_asin(const double args[]) {
		return asin(args[0]);
	}
	static double call_acos(const double args[]) {
		return acos(args[0]);
	}
	static double call_atan(const double args[]) {
		return atan(args[0]);
	}
	void catchError() {
		if (errorIndex == -1) errorIndex = index;
	}
	bool hasError() {
		return errorIndex != -1;
	}
	bool end() {
		return index >= length;
	}
	bool isOver() {
		return end() || hasError();
	}
	char nextChar() {
		if (isOver()) return '\0';
		return src[index];
	}
	char consumeChar() {
		if (isOver()) return '\0';
		return src[index++];
	}
	bool consumeChar(char chr) {
		if (nextChar() != chr) return false;
		consumeChar();
		return true;
	}
	char consumeTokenChar() {
		char chr = consumeChar();
		consumeSpaces();
		return chr;
	}
	bool consumeToken(char chr) {
		if (!consumeChar(chr)) return false;
		consumeSpaces();
		return true;
	}
	bool isSpace(char chr) {
		return chr == ' ' || chr == '\t' || chr == '\n';
	}
	bool isDigit(char chr) {
		return chr >= '0' && chr <= '9';
	}
	bool isIdHead(char chr) {
		return chr == '_' || (chr|=32) >= 'a' && chr <= 'z';
	}
	bool isIdBody(char chr) {
		return isIdHead(chr) || isDigit(chr);
	}
	void consumeSpaces() {
		while (isSpace(nextChar())) consumeChar();
	}
	std::string consumeId() {
		std::string id;
		while (isIdBody(nextChar())) id += consumeChar();
		consumeSpaces();
		return id;
	}
	double consumeValue() {
		std::string str;
		while (isDigit(nextChar())) {
			str += consumeChar();
		}
		if (nextChar()!='.') {
			consumeSpaces();
			return std::stod(str);
		}
		str += consumeChar();
		if (!isDigit(nextChar())) {
			catchError();
			return 0;
		}
		while (isDigit(nextChar())) str += consumeChar();
		consumeSpaces();
		return std::stod(str);
	}
	ExprNode* parseConst() {
		double value = consumeValue();
		if (hasError()) return nullptr;
		return new ExprNodeConst(value);
	}
	ExprNode* parseCall(std::string id) {
		if (consumeToken(')')) return new ExprNodeCall(id);
		ExprNode* arg = parseExpr();
		if (!arg) {
			catchError();
			return nullptr;
		}
		ExprNodeCall* call = new ExprNodeCall(id);
		call->addArg(arg);
		while (consumeToken(',')) {
			ExprNode* arg = parseExpr();
			if (!arg) {
				catchError();
				delete call;
				return nullptr;
			}
		}
		if (!consumeToken(')')) {
			catchError();
			delete call;
			return nullptr;
		}
		return call;
	}
	ExprNode* parseAbs() {
		ExprNode* tree = parseExpr();
		if (consumeToken('|')) return tree;
		catchError();
		delete tree;
		return nullptr;
	}
	ExprNode* parseOpr1() {
		if (isDigit(nextChar())) {
			return parseConst();
		}
		if (isIdHead(nextChar())) {
			std::string id = consumeId();
			if (consumeToken('(')) {
				return parseCall(id);
			}
			return new ExprNodeVar(id);
		}
		if (consumeToken('(')) {
			ExprNode* tree = parseExpr();
			if (!tree) return nullptr;
			if (!consumeToken(')')) {
				delete tree;
				catchError();
				return nullptr;
			}
			return tree;
		}
		if (consumeToken('|')) {
			ExprNode* tree = parseExpr();
			if (!tree) return nullptr;
			if (!consumeToken('|')) {
				delete tree;
				catchError();
				return nullptr;
			}
			return new ExprNodeAbs(tree);
		}
		return nullptr;
	}
	ExprNode* parseOpr2() {
		bool neg = consumeToken('-');
		ExprNode* tree = parseOpr1();
		if (!tree) {
			catchError();
			return nullptr;
		}
		while (consumeToken('^')) {
			bool neg = consumeToken('-');
			ExprNode* right = parseOpr1();
			if (!right) {
				catchError();
				delete tree;
				return nullptr;
			}
			if (neg) right = new ExprNodeNeg(right);
			tree = new ExprNodeOpr('^', tree, right);
		}
		if (neg) tree = new ExprNodeNeg(tree);
		return tree;
	}
	ExprNode* parseOpr3() {
		ExprNode* tree = parseOpr2();
		if (!tree) {
			catchError();
			return nullptr;
		}
		while (nextChar() == '*' || nextChar() == '/') {
			char opr = consumeTokenChar();
			ExprNode* right = parseOpr2();
			if (!right) {
				catchError();
				delete tree;
				return nullptr;
			}
			tree = new ExprNodeOpr(opr, tree, right);
		}
		return tree;
	}
	ExprNode* parseOpr4() {
		ExprNode* tree = parseOpr3();
		if (!tree) {
			catchError();
			return nullptr;
		}
		while (nextChar() == '+' || nextChar() == '-') {
			char opr = consumeTokenChar();
			ExprNode* right = parseOpr3();
			if (!right) {
				catchError();
				delete tree;
				return nullptr;
			}
			tree = new ExprNodeOpr(opr, tree, right);
		}
		return tree;
	}
	ExprNode* parseExpr() {
		return parseOpr4();
	}
public:
	ExprParser() {
		length = 0;
		index = 0;
		errorIndex = -1;
		parsedTree = nullptr;
	}
	bool parse(std::string expr) {
		src = expr;
		length = expr.length();
		index = 0;
		errorIndex = -1;
		consumeSpaces();
		parsedTree = parseExpr();
		if (!parsedTree) return false;
		if (!end()) catchError();
		if (hasError()) {
			delete parsedTree;
			parsedTree = nullptr;
			return false;
		}
		return true;
	}
	bool parse(const char str[]) {
		return parse(std::string(str));
	}
	bool success() {
		return !hasError() && parsedTree!=nullptr;
	}
	int error() {
		return errorIndex;
	}
	void setArg(std::string id, int index) {
		if (parsedTree) parsedTree->setArg(id, index);
	}
	void setVar(std::string id, double value) {
		if (parsedTree) parsedTree->setVar(id, value);
	}
	void setVar(std::string id, double* ref) {
		if (parsedTree) parsedTree->setVar(id, ref);
	}
	void setCall(std::string id, TExprFunction ref) {
		if (parsedTree) parsedTree->setCall(id, ref);
	}
	void std() {
		setVar("PI", (double) 3.1415926535897932384626433832795028841972);
		setVar("E",  (double) 2.7182818284590452353602874713526624977572);
		setCall("ln",   call_ln);
		setCall("log",  call_log);
		setCall("exp",  call_exp);
		setCall("sin",  call_sin);
		setCall("cos",  call_cos);
		setCall("tan",  call_tan);
		setCall("asin", call_asin);
		setCall("acos", call_acos);
		setCall("atan", call_atan);
	}
	std::vector <std::string> nullVars() {
		std::map <std::string, bool> map;
		std::vector <std::string> array;
		if (parsedTree) parsedTree->addVarsToMap(map);
		for (auto it=map.begin(), end=map.end(); it!=end; ++it) {
			if (!it->second) array.push_back(it->first);
		}
		return array;
	}
	std::vector <std::string> nullCalls() {
		std::map <std::string, bool> map;
		std::vector <std::string> array;
		if (parsedTree) parsedTree->addCallsToMap(map);
		for (auto it=map.begin(), end=map.end(); it!=end; ++it) {
			if (!it->second) array.push_back(it->first);
		}
		return array;
	}
	int countNullVars() {
		std::map <std::string, bool> map;
		if (parsedTree) parsedTree->addVarsToMap(map);
		int n = 0;
		for (auto it=map.begin(), end=map.end(); it!=end; ++it) {
			n += !it->second;
		}
		return n;
	}
	double calc() {
		return parsedTree ? parsedTree->calc() : 0;
	}
	std::string toString() {
		if (success()) return parsedTree->toString();
		return "error!";
	}
	Expr toExpr() {
		std::map<std::string, bool> map;
		parsedTree->addVarsToMap(map);
		int n = parsedTree->countArgs();
		for (auto it=map.begin(), end=map.end(); it!=end; ++it) {
			if (!it->second) parsedTree->setArg(it->first, n++);
		}
		return Expr(parsedTree);
	}
	~ExprParser() {
		if (parsedTree) delete parsedTree;
	}
};
