#include <string>
#include <vector>
#include <memory>
#include "lexer.h"

using namespace std;

namespace parser {


struct Node {
    virtual string token_literal() const = 0;
    virtual ~Node() {};
};


// abstract class for all the statements
struct Statement : public Node {
    virtual void statement_node() const = 0;
    virtual ~Statement() {}
};

struct Expression : public Node {
    virtual void expression_node() const = 0;
};

struct Identifier : Expression {
    lexer::Token token;
    string value;
    void expression_node() const {}
    string token_literal() const;
    Identifier(lexer::Token token, string value) : token(token), value(value){}
};

// syntax:
// let-statement := <let> <name> `=` <expression> 
struct LetStatement : Statement {
    lexer::Token let_token;
    unique_ptr<Identifier> name;
    unique_ptr<Expression> value;
    void statement_node() const {}
    string token_literal() const;
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

    bool _cur_tok_is(lexer::TokenType t);
    bool _peek_tok_is(lexer::TokenType t); 
    bool _expect_peek(lexer::TokenType t);

    unique_ptr<LetStatement> parse_let_statement();
    unique_ptr<Program> parse_program();
    unique_ptr<Statement> parse_statement();
};

void test_let_statements();

}