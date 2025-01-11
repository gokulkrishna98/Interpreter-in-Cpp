#include <string>
#include <vector>
#include <memory>
#include "lexer.h"

using namespace std;

namespace parser {


struct Node {
    virtual string token_literal() = 0;
    virtual ~Node() {};
};


// abstract class for all the statements
struct Statement : public Node {
    virtual void statement_node() = 0;
    virtual ~Statement() {};
};

struct Expression : public Node {
    virtual void expression_node() = 0;
};

struct Identifier : Expression {
    lexer::Token token;
    string value;
    void expression_node() {};
    string token_literal();
};

// syntax:
// let-statement := <let> <name> `=` <expression> 
struct LetStatement : Statement {
    lexer::Token let_token;
    unique_ptr<Identifier> name;
    unique_ptr<Expression> value;
    void statement_node() {};
    string token_literal();
};

struct Program {
    vector<unique_ptr<Statement>> statements;
    string token_literal(){
        return (!statements.empty()) ? statements[0]->token_literal() : "";
    }
};

struct Parser {
    lexer::Lexer l;
    lexer::Token cur_token;
    lexer::Token peek_token;
    
    Parser(lexer::Lexer l) : l (l) {
        next_token();
        next_token();
    };

    void next_token(){
        cur_token = peek_token;
        peek_token = l.next_token();
    }

    unique_ptr<Program> parse_program(){
        return nullptr;
    }
};

void test_let_statements();

}