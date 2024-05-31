#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <vector>
#include <locale>

const char operation_sign = '>';
const char f_const = 'f';

class AST;

class Node {
	friend class AST;
public:
	char expression;
	Node* left;
	Node* right;
	Node() {
		expression = ' ';
		left = nullptr;
		right = nullptr;
	}
	Node(char c) {
		expression = c;
		left = nullptr;
		right = nullptr;
	}
	Node* BuildNodeFromFormula(const std::string&);
private:
	
};

class AST {
public:
	std::string name;
	Node* head;
	AST(std::string formula) {
		name = formula;
		head = new Node;
	}
	AST() {
		name = "";
		head = new Node;
	}
	~AST() {
		DeleteTree(head);
	}
private:
	void DeleteTree(Node*);
};

struct SimilarityAnswer {
	int ans;
	const Node* axiom_node;
	const Node* formula_node;
	// полное различие нод или попытка заменить на f ans = 0
	SimilarityAnswer() {
		ans = 0;
		axiom_node = nullptr;
		formula_node = nullptr;
	}
	// равенство нод считаем ans = 1
	SimilarityAnswer(int f) {
		ans = f;
		axiom_node = nullptr;
		formula_node = nullptr;
	}
	// сходство нод cчитаем ans = 2
	SimilarityAnswer(int a, const Node* axiom, const Node* formula) {
		ans = a;
		axiom_node = axiom;
		formula_node = formula;
	}
};

struct BetaAnswer {
	bool ans = 0;
	int axiom_ind = 0;
	char variable = ' ';
	std::string formula = "";
	BetaAnswer() {
		ans = 0;
	}
	BetaAnswer(bool ans_, int axiom_ind_, char variable_,
		std::string formula_) : ans(ans_), axiom_ind(axiom_ind_),
		variable(variable_), formula(formula_) {};
};

void AST::DeleteTree(Node* cur) {
	if (cur) {
		DeleteTree(cur->left);
		DeleteTree(cur->right);
		delete cur;
	}
}

Node* Node::BuildNodeFromFormula(const std::string& formula) {
	if (formula.size() == 1 && formula[0] >= 'a' && formula[0] <= 'z') {
		return new Node(formula[0]);
	}

	std::stack<Node*> st;
	for (int i = formula.size() - 1; i >= 0; --i) {
		//std::cout << "i= " << i << " : " << formula[i] << '\n';
		// balance
		if (formula[i] == ')') {
			//std::cout << ") ind = " << i << '\n';
			size_t ind = i;
			int balance = -1;
			while (balance != 0) {
				--ind;
				if (formula[ind] == ')') --balance;
				else if (formula[ind] == '(') ++balance;
			}
			st.push(BuildNodeFromFormula(formula.substr(ind + 1, i - ind - 1)));
			i = ind;
			//std::cout << "outer i = " << i << '\n';
		}
		// in bracket options
		else if (formula[i] >= 'a' && formula[i] <= 'z') {
			st.push(new Node(formula[i]));
		}
		else if (formula[i] == operation_sign) {
			Node* cur = new Node(operation_sign);
			//std::cout << formula.substr(0, i) << '\n';
			cur->left = BuildNodeFromFormula(formula.substr(0, i));
			cur->right = st.top();
			st.pop();
			return cur;
		}
	}
	return st.top();
}

AST* BuildTree(AST& cur) {
	if (cur.name.size() > 1) {
		cur.head = cur.head->BuildNodeFromFormula(cur.name);
	}
	return &cur;
}

std::string NodeToString(Node& cur) {
	if (!cur.right && !cur.left) return std::string{ cur.expression };
	std::string result;
	if (cur.left) {
		result += NodeToString(*cur.left);
	}
	result += std::string{ operation_sign };
	if (cur.right) {
		result += NodeToString(*cur.right);
	}
	return "(" + result + ")";
}

std::string NodeToString(const Node& cur) {
	if (!cur.right && !cur.left) return std::string{ cur.expression };
	std::string result;
	if (cur.left) {
		result += NodeToString(*cur.left);
	}
	result += std::string{ operation_sign };
	if (cur.right) {
		result += NodeToString(*cur.right);
	}
	return "(" + result + ")";
}

std::string AstToString(const AST& tree) {
	Node* cur = tree.head;
	return NodeToString(*cur);
}

std::string CorrectParsed(const std::string& formula) {
	std::stack<char> psp;
	std::vector<bool> is_used(formula.size(), 1);
	std::stack<char> form_stack;
	bool l_param_exists = 0;
	bool r_param_required = 0;
	bool correction_required = 0;
	bool operation_required = 0;
	for (size_t i = 0; i < formula.size(); ++i) {
		if (formula[i] == '(') {
			//
			if (operation_required) {
				throw std::string{ "operation is required between two variables" };
			}
			l_param_exists = 0;
			//
			psp.push('(');
		}
		else if (formula[i] == ')') {
			if (r_param_required) {
				throw std::string{ "operation is required between two variables" };
			}
			if (!psp.empty() && psp.top() == '(') {
				psp.pop();
			}
			else {
				throw std::string{ "it's not a correct bracket sequence" };
			}
			
		}
		else if (formula[i] == operation_sign) {
			if (l_param_exists==0) {
				throw std::string{ "there isn't variable before operation" };
			}
			else {
				l_param_exists = 0;
				r_param_required = 1;
				operation_required = 0;
			}
		}
		else if (formula[i] >= 'a' && formula[i] <= 'z') {
			if (l_param_exists) {
				throw std::string{ "operation is required between two variables" };
			}
			else {
				if (r_param_required) {
					r_param_required = 0;
				}
			}
			l_param_exists = 1;
			operation_required = 1;
		}
		else if (formula[i] == ' ') {
			is_used[i] = 0;
			correction_required = 1;
		}
		else {
			throw std::string{ "illegal sign" };
		}
	}
	if (!psp.empty()) {
		throw std::string{ "it's not a correct bracket sequence" };
	}
	if (r_param_required) {
		throw std::string{ "there isn't r_param for operation" };
	}
	std::stack<size_t> open_bracket;
	std::stack<size_t> operation_ind;
	for (size_t i = 0; i < formula.size(); ++i) {
		if (is_used[i]) {
			if (formula[i] == '(') {
				open_bracket.push(i);
			}
			else if (formula[i] == operation_sign && !open_bracket.empty()) {
				operation_ind.push(i);
			}
			else if (formula[i] == ')') {
				if (open_bracket.empty()) {
					throw std::string{ "not a psp" };
				}
				size_t open_ind = open_bracket.top();
				open_bracket.pop();
				if ((!operation_ind.empty()) && (operation_ind.top() > open_ind)) {
					operation_ind.pop();
				}
				else {
					correction_required = 1;
					is_used[open_ind] = 0;
					is_used[i] = 0;
				}
			}
		}
	}
	std::string correct_formula = formula;
	size_t j = 0;
	if (correction_required) {
		for (size_t i = 0; i < formula.size(); ++i) {
			if (is_used[i]) {
				correct_formula[j++] = formula[i];
			}
		}
		correct_formula.resize(j);
	}
	if (correct_formula.size() == 0) throw std::string{ "Pared formula is nan" };
	return correct_formula;
}

std::string Parser(const std::string& formula) {
	std::string new_formula = "";
	try {
		new_formula = CorrectParsed(formula);
	}
	catch (std::string error_msg) {
		std::cout << "Формула содержит ошибку.\n\n";
		//std::cout << error_msg << '\n';
	}
	//std::cout << "new formula: " << new_formula << '\n';
	return new_formula;
}

AST* BuildAstFromFormula(std::string& formula) {
	std::string correct_formula = Parser(formula);
	AST* cur = new AST(correct_formula);
	return BuildTree(*cur);
}

bool EqualityOfTrees(const Node& axiom, const Node& formula) {
	bool res = 1;
	if (axiom.left && formula.left) {
		res &= EqualityOfTrees(*axiom.left, *formula.left);
	}
	else if (axiom.left || formula.left) return 0;

	if (axiom.right && formula.right) {
		res &= EqualityOfTrees(*axiom.right, *formula.right);
	}
	else if (axiom.right || formula.right) return 0;

	res &= (axiom.expression == formula.expression);
	return res;
}

SimilarityAnswer SimilarityOfTrees(const Node& axiom, const Node& formula) {
	// ноды равны, а значит это либо листья, либо знаки операций
	//std::cout << axiom.expression << ' ' << formula.expression << '\n';
	if (axiom.expression == formula.expression) {
		// листья равны
		if (!axiom.left && !formula.left && !axiom.right && !formula.right) {
			return SimilarityAnswer(bool(1));
		}
		// знаки операций равны, а значит должны содержать детей
		else {
			SimilarityAnswer ans_left = SimilarityAnswer(bool(1));
			if (axiom.left && formula.left) {
				ans_left = SimilarityOfTrees(*axiom.left, *formula.left);
			}
			SimilarityAnswer ans_right = SimilarityAnswer(bool(1));
			if (axiom.right && formula.right) {
				ans_right = SimilarityOfTrees(*axiom.right, *formula.right);
			}
			// обработка ответа по детям
			// одни из детей требуют замены на f или просто неравны
			if (ans_left.ans == 0 || ans_right.ans == 0) {
				return SimilarityAnswer();
			}
			// дети равны значит поддеревья одинаковые
			else if (ans_left.ans == 1 && ans_right.ans == 1) {
				return SimilarityAnswer(1);
			}
			// только один из детей равен значит можем вернуть замену в другом ребенке
			else if (ans_left.ans == 1) {
				return ans_right;
			}
			else if (ans_right.ans == 1) {
				return ans_left;
			}
			// в обоих детях требуется замена, подстановки атомарны, 
			// проверяем что заменяется одна переменная 
			else {
				if (ans_left.axiom_node->expression == ans_right.axiom_node->expression) {
					return ans_left;
				}
				else {
					return SimilarityAnswer(0);
				}
			}
		}
	}
	// ноды не равны, а значит либо замена на f,
	// либо листья, либо деревья отличаются по структуре
	else {
		// замена f в аксиоме
		if (axiom.expression == f_const) {
			return SimilarityAnswer(0);
		}
		// деревья отличаются по структуре 
		else if (axiom.expression == operation_sign) {
			return SimilarityAnswer(0);
		}
		else return SimilarityAnswer(2, &axiom, &formula);
	}
}

void CopyNode(Node& cur, const Node& change_node) {
	cur.expression = change_node.expression;
	if (change_node.left) {
		cur.left = new Node;
		CopyNode(*cur.left, *change_node.left);
	}
	if (change_node.right) {
		cur.right = new Node;
		CopyNode(*cur.right, *change_node.right);
	}
}

void ChangeNode(const char expr, const Node& change_node, Node& cur) {
	if (cur.expression == expr) {
		CopyNode(cur, change_node);
	}
	else {
		if (cur.left) {
			ChangeNode(expr, change_node, *cur.left);
		}
		if (cur.right) {
			ChangeNode(expr, change_node, *cur.right);
		}
	}
}

BetaAnswer DerivableByBeta(std::vector<const AST*>& ast_axioms, const AST& formula) {
	for (int i = 0; i < ast_axioms.size(); ++i) {
		SimilarityAnswer cur = SimilarityOfTrees(*ast_axioms[i]->head, *formula.head);
		//std::cout << i << ' ' << cur.ans << '\n';
		if (cur.ans == 2) {
			AST* change_tree = new AST(ast_axioms[i]->name);
			change_tree = BuildTree(*change_tree);
			char expr_to_change = cur.axiom_node->expression;
			ChangeNode(expr_to_change, *cur.formula_node, *change_tree->head);
			bool is_equal = EqualityOfTrees(*change_tree->head, *formula.head);
			delete change_tree;
			if (is_equal) {
				std::string example_s = NodeToString(*cur.formula_node);
				if (example_s.size() >= 3) {
					example_s = example_s.substr(1);
					example_s.resize(example_s.size() - 1);
				}
				return BetaAnswer(bool(1), i, expr_to_change, example_s);
			}
		}
	}
	return BetaAnswer();
}

std::pair<int,int> DerivableByMP(std::vector<const AST*>& ast_axioms, const AST& formula) {
	std::vector<size_t> A_B_equal;
	for (size_t i = 0; i < ast_axioms.size(); ++i) {
		if (EqualityOfTrees(*ast_axioms[i]->head->right, *formula.head)) {
			A_B_equal.push_back(i);
		}
	}
	std::pair<int, int> mp_axs = {-1,-1};
	for (size_t ind : A_B_equal) {
		mp_axs.first = ind;
		for (int i = 0; i < ast_axioms.size(); ++i) {
			if (EqualityOfTrees(*ast_axioms[ind]->head->left, *ast_axioms[i]->head)) {
				mp_axs.second = i;
				break;
			}
		}
		if (mp_axs.second != -1) break;
	}
	return mp_axs;
}

void Session() {
	std::setlocale(LC_ALL, "Russian");
	std::cout << "Для корректного завершения программы введите "
		<< '"' << "end" << '"' << ".\n\n";
	std::cout << "Для кастомизации набора аксиом пожалуйста "
		<< "воспользуйтесь файлом axioms.txt\n";
	std::cout << "Пожалуйста соблюдайте правила: все переменные -"
		<< " прописные буквы английского алфавита, f - выделенная константа, > - знак операции\n\n";
	std::vector<std::string> axioms;
	std::vector<std::string> ax_names;
	std::string ax;
	std::ifstream in("axioms.txt");
	std::cout << "Список аксиом, используемых в данной сессии программы:\n";
	if (in.is_open()) {
		while (std::getline(in, ax)) {
			std::cout << ax << '\n';
			ax_names.push_back(ax.substr(0, ax.find(':')));
			axioms.push_back(ax.substr(ax.find(':') + 1));
		}
		in.close();
	}
	std::cout << "Пожалуйста вводите ваши ппф\n\n";
	
	int num_of_axioms = axioms.size();
	std::vector<const AST*> ast_axioms(axioms.size());
	for (size_t i = 0; i < ast_axioms.size(); ++i) {
		ast_axioms[i] = BuildAstFromFormula(axioms[i]);
	}

	std::string query;

	while (std::getline(std::cin, query)) {
		if (query.size() == 0) continue;
		if (query == "end") break;
		std::string correct_string = Parser(query);
		if (correct_string.size()>=1) {
			AST* query_ast = BuildAstFromFormula(correct_string);
			bool f = 0;
			int eq_ind = 0;
			for (int i = 0; i < ast_axioms.size(); ++i) {
				f = EqualityOfTrees(*ast_axioms[i]->head, *query_ast->head);
				if (f) {
					eq_ind = i;
					break;
				}
			}
			if (f) {
				std::cout << "Формула " << query_ast->name << " аналогична ";
				if (eq_ind < num_of_axioms) {
					std::cout << "аксиоме " << ax_names[eq_ind];
				}
				else {
					std::cout << "формуле " << ast_axioms[eq_ind]->name;
				}				
				std::cout << ".\n\n";
				delete query_ast;
			}
			else {
				BetaAnswer ans = DerivableByBeta(ast_axioms, *query_ast);
				if (ans.ans == 1) {
					std::cout << "Формула " << query_ast->name << " выводима из ";
					if (ans.axiom_ind < num_of_axioms) {
						std::cout << "аксиомы " << ax_names[ans.axiom_ind];
					}
					else {
						std::cout << "формулы " << ast_axioms[ans.axiom_ind]->name;
					}
					std::cout << " путем замены переменной "
						<< '"' << ans.variable << '"' << " на выражение "
						<< '"' << ans.formula << '"' << ".\n\n";
					ast_axioms.push_back(query_ast);
					continue;
				}
				else {
					std::pair<int, int> ans = DerivableByMP(ast_axioms, *query_ast);
					if (ans.first != -1 && ans.second!=-1) {
						std::cout << "Формула " << query_ast->name << " выводима из ";
						if (ans.first < num_of_axioms) {
							std::cout << "аксиомы " << ax_names[ans.first];
						}
						else {
							std::cout << "формулы " << ast_axioms[ans.first]->name;
						}
						std::cout << " и ";
						if (ans.second < num_of_axioms) {
							std::cout << "аксиомы " << ax_names[ans.second];
						}
						else {
							std::cout << "формулы " << ast_axioms[ans.second]->name;
						}
						std::cout << " по правилу modus ponens.\n\n";
						ast_axioms.push_back(query_ast);
						continue;
					}
				}
				delete query_ast;
				std::cout << "Формула не выводима.\n\n";
			}
		}
	}
	if (ast_axioms.size() > num_of_axioms) {
		std::ofstream out;
		out.open("theorems.txt");
		if (out.is_open()) {
			for (int i = num_of_axioms; i < ast_axioms.size(); ++i) {
				out << ast_axioms[i]->name << '\n';
			}
			std::cout << "Теоремы, выведенные в ходе данной сессии программы "
				<< "успешно выгружены в файл theorems.txt\n";
			out.close();
		}
	}
	for (int i = 0; i < ast_axioms.size(); ++i) {
		delete ast_axioms[i];
	}
}

int main() {
	Session();
	std::cout << "Сессия программы завершена успешно.\n";
	return 0;
}
