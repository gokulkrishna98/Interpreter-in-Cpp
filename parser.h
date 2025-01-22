#include <cstdint>
#include <string>
#include <any>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
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

struct IntegerLiteral : Expression {
    lexer::Token token;
    int64_t val;

    IntegerLiteral(lexer::Token token, int64_t val) : token(token), val(val){}
    void expression_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct Boolean : Expression {
    lexer::Token token;
    bool val;

    Boolean(lexer::Token token, bool val) : token(token), val(val) {};
    void expression_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct PrefixExpression : Expression {
    lexer::Token token;
    std::string op;
    std::unique_ptr<Expression> right;

    void expression_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct InfixExpression : Expression {
    lexer::Token token;
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;

    void expression_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct BlockStatement : Statement {
    lexer::Token token;
    vector<std::unique_ptr<Statement>> statements;
    void statement_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct IfExpression : Expression {
    lexer::Token token; // if token
    std::unique_ptr<Expression> cond;
    std::unique_ptr<BlockStatement> consequence;
    std::unique_ptr<BlockStatement> alternative;

    void expression_node() const {}
    std::string token_literal() const;
    std::string string() const;
};

struct CallExpression : Expression {
    lexer::Token token;
    std::unique_ptr<Expression> function;
    std::vector<std::unique_ptr<Expression>> arguments;

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

struct Program : Node {
    vector<unique_ptr<Statement>> statements;
    std::string token_literal() const {
        return (!statements.empty()) ? statements[0]->token_literal() : "";
    }
    std::string string() const;
};

std::unique_ptr<Expression> prefix_parse_fn();
std::unique_ptr<Expression> infix_parse_fn(std::unique_ptr<Expression>);

struct Parser {
    lexer::Lexer l;
    lexer::Token cur_token;
    lexer::Token peek_token;
    vector<string> errors;

    // two maps for infix and prefix parse fns
    unordered_map<lexer::TokenType, 
        std::function<std::unique_ptr<Expression>()>> prefix_parse_fns;
    unordered_map<lexer::TokenType, 
        std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)>> infix_parse_fns;

    Parser(lexer::Lexer l) : l (l) {
        next_token();
        next_token();
        register_prefix(lexer::TokenType::ID, std::bind(&Parser::parse_identifier, this));
        register_prefix(lexer::TokenType::INT, std::bind(&Parser::parse_integer_literal, this));
        register_prefix(lexer::TokenType::BANG, std::bind(&Parser::parse_prefix_expression, this));
        register_prefix(lexer::TokenType::MINUS, std::bind(&Parser::parse_prefix_expression, this));
        register_prefix(lexer::TokenType::TRUE, std::bind(&Parser::parse_boolean, this));
        register_prefix(lexer::TokenType::FALSE, std::bind(&Parser::parse_boolean, this));
        register_prefix(lexer::TokenType::LPAREN, std::bind(&Parser::parse_grouped_expression, this));
        register_prefix(lexer::TokenType::IF, std::bind(&Parser::parse_if_expression, this));


        register_infix(lexer::TokenType::PLUS, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::MINUS, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::FSLASH, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::ASTERISK, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::EQ, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::NEQ, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::LT, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::GT, 
            std::bind(&Parser::parse_infix_expression, this, std::placeholders::_1));
        register_infix(lexer::TokenType::LPAREN,
            std::bind(&Parser::parse_call_expression, this, std::placeholders::_1));
    };

    void next_token(){
        cur_token = peek_token;
        peek_token = l.next_token();
    }

    bool _cur_tok_is(lexer::TokenType t);
    bool _peek_tok_is(lexer::TokenType t); 
    bool _expect_peek(lexer::TokenType t);
    void _peek_error(lexer::TokenType t);
    int _peek_precedence();
    int _cur_precedence();

    vector<string> get_errors();

    std::unique_ptr<LetStatement> parse_let_statement();
    std::unique_ptr<ReturnStatement> parse_ret_statement();
    std::unique_ptr<ExpressionStatement> parse_expression_statement();
    std::unique_ptr<BlockStatement> parse_block_statement();

    std::unique_ptr<Program> parse_program();
    std::unique_ptr<Statement> parse_statement();

    std::unique_ptr<Expression> parse_boolean();
    std::unique_ptr<Expression> parse_expression(int precedence);
    std::unique_ptr<Expression> parse_grouped_expression();
    std::unique_ptr<Expression> parse_identifier();
    std::unique_ptr<Expression> parse_if_expression();
    std::unique_ptr<Expression> parse_infix_expression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parse_integer_literal();
    std::unique_ptr<Expression> parse_prefix_expression();
    std::unique_ptr<Expression> parse_call_expression(std::unique_ptr<Expression> function);

    std::vector<std::unique_ptr<Expression>> parse_call_arguments();

    void register_prefix(lexer::TokenType token_type, 
        std::function<std::unique_ptr<Expression>()> fn);
    void register_infix(lexer::TokenType token_type, 
        std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)>);
};

void test_let_statements();
void test_ret_statements();
void test_string();
void test_identifier_expression();
void test_integer_literal_expression();
void test_parsing_prefix_expression();
void test_parsing_infix_expression();
void test_operator_precedence_parsing();
void test_infix_expression(std::unique_ptr<Expression> exp, const std::any& expected);
void test_if_expression();
void test_call_expression();
}