#ifndef _POSTORDER_EXPR_TREE_H_
#define _POSTORDER_EXPR_TREE_H_

#include <functional>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

static std::unordered_map<char, std::function<int(int, int)>> ops = {
	{ '+', [](int val1, int val2) { return val1 + val2; } },
	{ '-', [](int val1, int val2) { return val1 - val2; } },
	{ '*', [](int val1, int val2) { return val1 * val2; } },
	{ '/', [](int val1, int val2) { return val1 / val2; } }
};

class Node {
public:
	virtual int evaluate() = 0;
};

class OperandNode : public Node {
public:
	OperandNode(int val) :
		val_(val) {
	}

	int evaluate() override {
		return val_;
	}

private:
	int val_;
};

class OperatorNode : public Node {
public:
	OperatorNode(char op, 
			std::shared_ptr<Node> left,
			std::shared_ptr<Node> right) :
		op_(op),
		left_(left),
		right_(right) {
	}

	int evaluate() override {
		int left_val = left_->evaluate();
		int right_val = right_->evaluate();
		return ops[op_](left_val, right_val);
	} 

private:
	char op_;
	std::shared_ptr<Node> left_;
	std::shared_ptr<Node> right_;
};

class ExprTree {
public:
	ExprTree(std::vector<std::string>& expr) {
		std::stack<std::shared_ptr<Node>> stk;
		for (auto& e : expr) {
			std::shared_ptr<Node> node;
			if (ops.find(e[0]) != ops.end()) { // operator
				char op = e[0];
				std::shared_ptr<Node> right = stk.top(); stk.pop();
				std::shared_ptr<Node> left = stk.top(); stk.pop();
				node = std::make_shared<OperatorNode>(op, left, right);
			} else { // operand
				int val = std::stoi(e);
				node = std::make_shared<OperandNode>(val);
			}
			stk.push(node);
		}
		node_ = stk.top();
	}

	int evaluate() {
		return node_->evaluate();
	}

private:
	std::shared_ptr<Node> node_;
};

#endif // _POSTORDER_EXPR_TREE_H_