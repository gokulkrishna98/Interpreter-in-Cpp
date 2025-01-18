#include "parser.h"
#include <cstdio>
#include <memory>
#include <fmt/format.h>
#include <unordered_map>

namespace parser {

enum operation_prec {
    LOWEST,
    EQUALS,
    LESSGREATER,
    SUM,
    PRODUCT,
    PREFIX,
    CALL
};

std::string LetStatement::token_literal() const {
    return let_token.val;
}

std::string ReturnStatement::token_literal() const {
    return ret_token.val;
}

std::string ExpressionStatement::token_literal() const {
    return expr_token.val;
}

std::string Identifier::token_literal() const {
    return token.val;
}

bool Parser::_cur_tok_is(lexer::TokenType t) {
    return cur_token.type == t;
}

bool Parser::_peek_tok_is(lexer::TokenType t){
    return peek_token.type == t;
}

bool Parser::_expect_peek(lexer::TokenType t){
    if(_peek_tok_is(t)){
        next_token();
        return true;
    }else{
        _peek_error(t);
        return false;
    }
}

vector<std::string> Parser::get_errors(){
    return errors;
}

// TODO: write helper function to conver enum to string
void Parser::_peek_error(lexer::TokenType t){
    std::string error_msg = fmt::format("expected next token to be {}, got {} instead",
        lexer::enum_to_string(t), 
        lexer::enum_to_string(peek_token.type)
    );
    errors.push_back(error_msg);
    return;
}

unique_ptr<LetStatement> Parser::parse_let_statement(){
    auto stmt = make_unique<LetStatement>();  
    stmt->let_token = cur_token;

    if(!_expect_peek(lexer::TokenType::ID)){
        return nullptr;
    }

    stmt->name = std::make_unique<Identifier>(cur_token, cur_token.val);
    if(!_expect_peek(lexer::TokenType::ASSIGN)){
        return nullptr;
    }

    // skipping parsing expr
    while(!_cur_tok_is(lexer::TokenType::SEMICOLON)){
        next_token();
    }

    return stmt;
}

unique_ptr<ReturnStatement> Parser::parse_ret_statement(){
    auto stmt = std::make_unique<ReturnStatement>();
    stmt->ret_token = cur_token;
    next_token();
    // skipping parsing expr
    while(!_cur_tok_is(lexer::TokenType::SEMICOLON)){
        next_token();
    }
    return stmt;
}

unique_ptr<Expression> Parser::parse_expression(){
    auto prefix = prefix_parse_fns[cur_token.type];
    if(prefix == nullptr){
        return nullptr;
    }

    auto left_expr = prefix();
    return left_expr;
}

unique_ptr<Expression> Parser::parse_identifier(){
    return std::make_unique<Identifier>(cur_token, cur_token.val);
}

unique_ptr<ExpressionStatement> Parser::parse_expression_statement(){
    auto stmt = std::make_unique<ExpressionStatement>();
    stmt->expr_token = cur_token;
    stmt->expr = parse_expression();
    if(_peek_tok_is(lexer::TokenType::SEMICOLON)){
        next_token();
    }
    return stmt;
}

unique_ptr<Statement> Parser::parse_statement(){
    switch(cur_token.type){
        case lexer::TokenType::LET: return parse_let_statement();
        case lexer::TokenType::RETURN: return parse_ret_statement();
        default: return parse_expression_statement();
    }
}

unique_ptr<Program> Parser::parse_program(){
    auto program = std::make_unique<Program>();
    while(cur_token.type != lexer::TokenType::ENDOF){
        auto stmt = parse_statement();
        if(stmt != nullptr){
            program->statements.push_back(std::move(stmt));
        }
        next_token();
    }
    return program;
}


std::string Program::string() const{
    std::string program_string = "";
    for(auto &stmt : statements){
        program_string += stmt->string();
    }
    return program_string;
}

std::string LetStatement::string() const {
    std::string let_stmt_string = "";
    let_stmt_string += (token_literal() + " ");
    let_stmt_string += (name->string());
    let_stmt_string += " = ";
    if(value != nullptr){
        let_stmt_string += value->string();
    }
    let_stmt_string += ";";
    return let_stmt_string;
}

std::string ReturnStatement::string() const {
    std::string ret_stmt_string = ""; 
    ret_stmt_string += (token_literal() + " ");
    if(return_value != nullptr){
        ret_stmt_string += return_value->string();
    }
    ret_stmt_string += ";";
    return ret_stmt_string;
}

std::string ExpressionStatement::string() const {
    if(expr != nullptr){
        return expr->string();
    }
    return "";
}

std::string Identifier::string() const {
    return value;
}

void Parser::register_prefix(lexer::TokenType token_type,
    std::function<std::unique_ptr<Expression>()> fn){
    prefix_parse_fns[token_type] = fn;
}

void Parser::register_infix(lexer::TokenType token_type,
    std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)> fn){
    infix_parse_fns[token_type] = fn;
}



bool test_let_statement(const Statement* s, std::string id){
    if(s->token_literal() != "let"){
        printf("expected to get `let` but got %s\n", s->token_literal().c_str());
        return false;
    }

    auto let_statment = dynamic_cast<const LetStatement*>(s);
    if(let_statment == nullptr){
        printf("could not cast statment to let_statement\n");
        return false;
    }

    if(let_statment->name->value != id){
        printf("letstatement id is different, expected %s, got %s\n", 
            id.c_str(),
            let_statment->name->value.c_str());
        return false;
    }
    return true;
}

void check_parse_errors(vector<std::string> errors){
    if(errors.empty()){
        return;
    }

    printf("parser has [%ld] errors\n", errors.size());
    for(auto &s: errors){
        printf("parser error: %s\n", s.c_str());
    }
    return;
}

void test_let_statements(){
    std::string input = R"(
let x = 5;
let y = 10;
let foobar = 838383;
    )";

    input = R"(
let x 5;
let = 10;
let 838383;
    )";

    auto l = lexer::Lexer(input);
    auto p = Parser(l);

    auto program = p.parse_program();
    check_parse_errors(p.get_errors());

    if(program == nullptr){
        printf("parse program returned nullptr\n");
        return;
    }

    if(program->statements.size() != 3){
        printf("program does not contain 3 statements: found %ld statements\n",
            program->statements.size());
        return;
    }

    vector<std::string> expected_id = {"x", "y", "foobar"};

    for(int i=0; i<expected_id.size(); i++){
        const Statement* stmt= program->statements[i].get();
        if(!test_let_statement(stmt, expected_id[i])){
            return;
        }
    }
    printf("[3/3] all test cases passed !!\n");
    return;
}


void test_ret_statements(){
    std::string input = R"(
return 5;
return 10;
return 993322;
    )";
    
    auto l = lexer::Lexer(input);
    auto p = Parser(l);

    auto program = p.parse_program();
    check_parse_errors(p.get_errors());

    if(program == nullptr){
        printf("parse program returned nullptr\n");
        return;
    }

    if(program->statements.size() != 3){
        printf("program does not contain 3 statements: found %ld statements\n",
            program->statements.size());
        return;
    }


    for(int i=0; i<program->statements.size(); i++){
        const ReturnStatement* stmt = 
            dynamic_cast<ReturnStatement*>(program->statements[i].get());
        if(stmt == nullptr){
            printf("got nullptr statement, not a return statement\n");
            return;
        }

        if(stmt->ret_token.val != "return"){
            printf("returnstmt.token is 'return', but got %s\n", 
                stmt->ret_token.val.c_str());
            return;
        }
    }
    printf("[3/3] all test cases passed !!\n");
    return;
}

void test_string(){
    auto stmt = std::make_unique<LetStatement>();
    stmt->let_token = lexer::Token(lexer::TokenType::LET, "let"); 
    stmt->name = std::make_unique<Identifier>(
        lexer::Token(lexer::TokenType::ID, "my_var"), 
        "my_var");
    stmt->value = std::make_unique<Identifier>(
        lexer::Token(lexer::TokenType::ID, "another_var"), 
        "another_var");
    Program program;
    program.statements.push_back(std::move(stmt));

    if(program.string() != "let my_var = another_var;"){
        printf("program.string(), is wrong. got %s\n", program.string().c_str());
        return;
    }

    printf("[success] testing string() in ast\n");
    return;
}

void test_identifier_expression(){
    std::string input = "foobar;";
    auto l = lexer::Lexer(input);
    Parser p(l);
    auto program = p.parse_program();
    check_parse_errors(p.get_errors());

    if(program->statements.size() != 1){
        printf("[error] program has no enough statements %ld\n", program->statements.size());
        return;
    }

    const ExpressionStatement* stmt = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
    if(stmt == nullptr){
        printf("[error] the statment is not a expression statement\n");
        return;
    }

    auto ident = dynamic_cast<Identifier*>(stmt->expr.get());
    if(ident == nullptr){
        printf("[error] the expression in the statement is not identifier\n");
        return;
    }

    if(ident->value != "foobar"){
        printf("ident not %s, got %s\n", "foobar", ident->value.c_str());
        return;
    }

    if(ident->token_literal() != "foobar"){
        printf("ident.token_literal not %s, got %s\n", "foobar", ident->value.c_str());
        return;
    }

}


}