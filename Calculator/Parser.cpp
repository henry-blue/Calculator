#include <cassert>
#include <cstring>
#include "Parser.h"
#include "scanner.h"
#include "Node.h"
#include "Calc.h"
#include "Exception.h"
#include "DebugNew.h"

Parser::Parser(Scanner& scanner, Calc& calc) 
: scanner_(scanner), calc_(calc), tree_(0), status_(STATUS_OK)
{
}

Parser::~Parser()
{
	delete tree_;
}

STATUS Parser::Parse()
{
	tree_ = Expr();
	if (!scanner_.IsScannerDone())
	{
		status_ = STATUS_ERROR;
	}
	return status_;
}

Node* Parser::Expr()
{
	Node* node = Term();
	EToken token = scanner_.Token();
	//if (token == TOKEN_PLUS)
	//{
	//	scanner_.Accept();
	//	Node* nodeRight = Expr();
	//	node = new AddNode(node, nodeRight);
	//}
	//else if (token == TOKEN_MINUS)
	//{
	//	scanner_.Accept();
	//	Node* nodeRight = Expr();
	//	node = new SubNode(node, nodeRight);
	//}
	if (token == TOKEN_PLUS || token == TOKEN_MINUS)
	{
		MultipleNode* multipleNode = new SumNode(node);
		do
		{
			scanner_.Accept();
			Node* nextNode = Term();
			multipleNode->AppendChild(nextNode, token == TOKEN_PLUS);
			token = scanner_.Token();
		} while (token == TOKEN_PLUS || token == TOKEN_MINUS);
		node = multipleNode;
	}
	else if (token == TOKEN_ASSIGN)
	{
		// Expr := Term = Expr
		scanner_.Accept();
		Node* nodeRight = Expr();
		if (node->IsValue())
		{
			node = new AssignNode(node, nodeRight);
		}
		else
		{
			status_ = STATUS_ERROR;
			throw SyntaxError("the left-hand side of an assignment, must be a variable");
		}
	}
	return node;
}
Node* Parser::Term()
{
	Node* node = Factor();
	EToken token = scanner_.Token();
	//if (token == TOKEN_MULTIPLY)
	//{
	//	scanner_.Accept();
	//	Node* nodeRight = Term();
	//	node = new MultiplyNode(node, nodeRight);
	//}
	//else if (token == TOKEN_DIVIDE)
	//{
	//	scanner_.Accept();
	//	Node* nodeRight = Term();
	//	node = new DivideNode(node, nodeRight);
	//}
	if (token == TOKEN_MULTIPLY || token == TOKEN_DIVIDE)
	{
		MultipleNode* productNode = new ProductNode(node);
		do
		{
			scanner_.Accept();
			Node* nextNode = Factor();
			productNode->AppendChild(nextNode, token == TOKEN_MULTIPLY);
			token = scanner_.Token();
		} while (token == TOKEN_MULTIPLY || token == TOKEN_DIVIDE);
		node = productNode;
	}
	return node;
}

Node* Parser::Factor()
{
	Node* node;
	EToken token = scanner_.Token();
	if (token == TOKEN_LPARENTHESIS)
	{
		scanner_.Accept(); // accept '('
		node = Expr();
		if (scanner_.Token() == TOKEN_RPARENTHESIS)
		{
			scanner_.Accept(); // accept ')'
		}
		else
		{
			status_ = STATUS_ERROR;
			throw SyntaxError("missing parentthesis");
			node = 0;
		}
	}
	else if (token == TOKEN_NUMBER)
	{
		node = new NumberNode(scanner_.Number());
		scanner_.Accept();
	}
	else if (token == TOKEN_IDENTIFIER)
	{
		std::string symbol = scanner_.GetSymbol();
		unsigned int id = calc_.FindSymbol(symbol);
		scanner_.Accept();
		//�Ǻ���
		if (scanner_.Token() == TOKEN_LPARENTHESIS)
		{
			scanner_.Accept();  //accept '('
			node = Expr();
			if (scanner_.Token() == TOKEN_RPARENTHESIS)
			{
				scanner_.Accept();  //accept ')'
				if (id != SymbolTable::ID_NOT_FIND && calc_.IsFuncation(id))
				{
					node = new FuncationNode(node, calc_.GetFuncation(id));
				}
				else
				{
					status_ = STATUS_ERROR;
					char buf[256] = { 0 };
					sprintf_s(buf, "UnKnown funcation: \"%s\"", symbol.c_str());
					throw SyntaxError(buf);
				}
			}
			else
			{
				status_ = STATUS_ERROR;
				throw SyntaxError("missing parentthesis in a funcation call");
			}
		}
		else  //�Ǳ���
		{
			if (id == SymbolTable::ID_NOT_FIND)
			{
				id = calc_.AddSymbol(symbol);
			}
			node = new VariableNode(id, calc_.GetStorage());
		}
	}
	else if (token == TOKEN_MINUS)
	{
		scanner_.Accept();   // accept minus
		node = new UMinusNode(Factor());
	}
	else
	{
		status_ = STATUS_ERROR;
		throw SyntaxError("Not a valid expreesion");
		node = 0;
	}
	return node;
}

double Parser::Calculate() const
{
	assert(tree_ != 0);
	return tree_->Calc();
}
