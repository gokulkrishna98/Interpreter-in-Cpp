#include <string>
#include <vector>
#include <memory>
#include "lexer.h"


namespace parser {

struct Node {
    virtual std::string token_literal() const = 0;
    virtual std::string string() const = 0;
    virtual ~Node() {};
};


// abstract class for all the statements
struct Statement : public Node {
    virtual void statement_node() const = 0;
    virtual ~Statement() {}
};

struct Expression : public Node {
    virtual void expression_node() const = 0;
    virtual ~Expression() {}
};

struct Identifier : Expression {
    lexer::Token token;
    std::string value;

    Identifier(lexer::Token token, std::string value) : token(token), value(value){}
    void expression_node() const {}
    std::string token_literal() const;
    std::string string() const;
};


// syntax:
// let-statement := <let> <name> `=` <expression> 
struct LetStatement : Statement {
    lexer::Token let_token;
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Expression> value;

    void statement_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct ReturnStatement : Statement {
    lexer::Token ret_token;
    std::unique_ptr<Expression> return_value;

    void statement_node () const {}
    std::string token_literal() const;
    std::string string() const;
};

struct ExpressionStatement : Statement {
    lexer::Token expr_token; // first token in the expression
    std::unique_ptr<Expression> expr;

    void statement_node () const {}
    std::string token_literal() const;
    std::string string() const;
};

struct Program {
    vector<unique_ptr<Statement>> statements;
    std::string token_literal() const {
        return (!statements.empty()) ? statements[0]->token_literal() : "";
    }
    std::string string() const;
};

struct Parser {
    lexer::Lexer l;
    lexer::Token cur_token;
    lexer::Token peek_token;
    vector<string> errors;

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
    void _peek_error(lexer::TokenType t);
    vector<string> get_errors();

    unique_ptr<LetStatement> parse_let_statement();
    unique_ptr<ReturnStatement> parse_ret_statement();
    unique_ptr<Program> parse_program();
    unique_ptr<Statement> parse_statement();
};

void test_let_statements();
void test_ret_statements();
void test_string();

}