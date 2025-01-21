#include "parser.h"
#include <any>
#include <cstdint>
#include <cstdio>
#include <fmt/format.h>
#include <ios>
#include <memory>
#include <typeinfo>
#include <iostream>

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

unordered_map<lexer::TokenType, int> precedences = {
    {lexer::TokenType::EQ, operation_prec::EQUALS},    
    {lexer::TokenType::NEQ, operation_prec::EQUALS},    
    {lexer::TokenType::LT, operation_prec::LESSGREATER},    
    {lexer::TokenType::GT, operation_prec::LESSGREATER},    
    {lexer::TokenType::PLUS, operation_prec::SUM},    
    {lexer::TokenType::MINUS, operation_prec::SUM},    
    {lexer::TokenType::FSLASH, operation_prec::PRODUCT},    
    {lexer::TokenType::ASTERISK, operation_prec::PRODUCT},    
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

std::string IntegerLiteral::token_literal() const {
    return token.val;
}

std::string Boolean::token_literal() const {
    return token.val;
}

std::string PrefixExpression::token_literal() const {
    return token.val;
}

std::string InfixExpression::token_literal() const {
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

int Parser::_peek_precedence(){
    if(precedences.find(peek_token.type) != precedences.end()){
        return precedences[peek_token.type]; 
    }
    return operation_prec::LOWEST;
}

int Parser::_cur_precedence(){
    if(precedences.find(cur_token.type) != precedences.end()){
        return precedences[cur_token.type]; 
    }
    return operation_prec::LOWEST;
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

unique_ptr<Expression> Parser::parse_boolean(){
    return std::make_unique<Boolean>(cur_token, _cur_tok_is(lexer::TokenType::TRUE));
}

unique_ptr<Expression> Parser::parse_expression(int precedence = 0){
    auto prefix = prefix_parse_fns[cur_token.type];
    if(prefix == nullptr){
        errors.push_back(fmt::format("no prefix parse function found for {} found\n",
            lexer::enum_to_string(cur_token.type)));
        return nullptr;
    }
    auto left_expr = prefix();

    while(!_peek_tok_is(lexer::TokenType::SEMICOLON) && precedence < _peek_precedence()){
        auto infix = infix_parse_fns[peek_token.type];
        if(infix == nullptr){
            return left_expr;
        }

        next_token();
        left_expr = infix(std::move(left_expr));
    }
    return left_expr;
}

unique_ptr<Expression> Parser::parse_identifier(){
    return std::make_unique<Identifier>(cur_token, cur_token.val);
}

unique_ptr<Expression> Parser::parse_integer_literal(){
    auto val = cur_token.val;
    return std::make_unique<IntegerLiteral>(cur_token, stoi(val));
}

unique_ptr<Expression> Parser::parse_prefix_expression(){
    auto expression = std::make_unique<PrefixExpression>();
    expression->token = cur_token;
    expression->op = cur_token.val;
    next_token();
    expression->right = parse_expression(operation_prec::PREFIX);
    return expression;
}

unique_ptr<Expression> Parser::parse_infix_expression(unique_ptr<Expression> left){
    auto expression = std::make_unique<InfixExpression>();
    expression->token = cur_token;
    expression->left = std::move(left);
    expression->op = cur_token.val;

    int prec = _cur_precedence();
    next_token();
    expression->right = parse_expression(prec);
    return expression;
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

std::string IntegerLiteral::string() const {
    return token.val;
}

std::string Boolean::string() const {
    return token.val;
}

std::string PrefixExpression::string() const {
    std::string pref_expr_string = "";
    pref_expr_string += "(";
    // pref_expr_string += " " + op + " ";
    pref_expr_string += op;
    pref_expr_string += right->string();
    pref_expr_string += ")";
    return pref_expr_string;
}

std::string InfixExpression::string() const {
    std::string inf_expr_string = "";
    inf_expr_string += "(";
    inf_expr_string += left->string();
    inf_expr_string += " " + op + " ";
    inf_expr_string += right->string();
    inf_expr_string += ")";
    return inf_expr_string;
}

void Parser::register_prefix(lexer::TokenType token_type,
    std::function<std::unique_ptr<Expression>()> fn){
    prefix_parse_fns[token_type] = fn;
}

void Parser::register_infix(lexer::TokenType token_type,
    std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)> fn){
    infix_parse_fns[token_type] = fn;
}

bool test_integer_integral(std::unique_ptr<Expression> il, int64_t value){
    const IntegerLiteral* integ = dynamic_cast<IntegerLiteral*>(il.get());
    if(integ == nullptr){
        printf("[error] did not recieve integer literal expression\n");
        return false;
    }
    if(integ->val != value){
        printf("[error] integer value mismatch\n");
        return false;
    }
    if(integ->token_literal() != fmt::to_string(value)){
        printf("[error] integer.token_literal() value mismatch\n");
        return false;
    }
    return true;
}

bool test_identifier(std::unique_ptr<Expression> exp, std::string value){
    const Identifier* ident = dynamic_cast<Identifier*>(exp.get());
    if(ident == nullptr){
        printf("[error] did not recieve identifier expression\n");
        return false;
    }
    if(ident->value != value){
        printf("[error] id value mismatch\n");
        return false;
    }
    if(ident->token_literal() != value){
        printf("[error] ident.token_literal() value mismatch\n");
        return false;
    }
    return true;
}

bool test_boolean_literal(std::unique_ptr<Expression> exp, bool value){
    const Boolean* bo = dynamic_cast<Boolean*>(exp.get());
    if(bo == nullptr){
        printf("[error] expected boolean expression but go something else\n");
        return false;
    }

    if(bo->val != value){
        printf("[error] boolen value mismatch\n");
        return false;
    }

    auto bool_to_string = [](bool b){
        return b ? "true" : "false";
    };
    if(bo->token_literal() != bool_to_string(value)){
        printf("[error] bo.token_literal() value mismatch\n");
        return false;
    }

    return true;
}

bool test_literal_expression(std::unique_ptr<Expression> exp, const std::any& expected){
    if(expected.type() == typeid(int64_t)){
        return test_integer_integral(std::move(exp), std::any_cast<int64_t>(expected));
    }else if(expected.type() == typeid(std::string)){
        return test_identifier(std::move(exp), std::any_cast<std::string>(expected));
    }else if(expected.type() == typeid(bool)){
        return test_boolean_literal(std::move(exp), std::any_cast<bool>(expected));
    }else{
        printf("[error] expected type not handled\n");
    }
    return false;
}

bool test_infix_expression(std::unique_ptr<Expression> exp, 
    const std::any &left, std::string op, const std::any &right){
    const InfixExpression* op_exp = dynamic_cast<InfixExpression*>(exp.get());
    if(op_exp == nullptr){
        printf("[error] the experession is not infix expression\n");
        return false;
    }

    std::unique_ptr<InfixExpression> inf_expr = 
        std::unique_ptr<InfixExpression>(dynamic_cast<InfixExpression*>(exp.release()));

    if(!test_literal_expression(std::move(inf_expr->left), left)){
        return false;
    }

    if(inf_expr->op != op){
        printf("[error] operator mismatch \n");
        return false;
    }

    if(!test_literal_expression(std::move(inf_expr->right), right)){
        return false;
    }

    return true;
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
    exit(1);
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


void test_integer_literal_expression(){
    std::string input = "5;";
    auto l = lexer::Lexer(input);
    auto p = Parser(l);
    auto program = p.parse_program();

    check_parse_errors(p.get_errors());

    if(program->statements.size() != 1){
        printf("[error] program has no enough statements %ld\n", program->statements.size());
        return;
    }

    const ExpressionStatement* stmt = 
        dynamic_cast<ExpressionStatement*>(program->statements[0].get());

    if(stmt == nullptr){
        printf("[error] first statement is not expression statement \n");
        return;
    }

    const IntegerLiteral* literal = dynamic_cast<IntegerLiteral*>(stmt->expr.get());
    if(literal == nullptr){
        printf("[error] exp not integer literal\n");
        return;
    }

    if(literal->val != 5){
        printf("[error] literal value is not %ld, got %ld\n", 5l, literal->val);
        return;
    }

    if(literal->token_literal() != "5"){
        printf("[error] literal.token_literal() is not %s, got %s\n", 
            "5", literal->token_literal().c_str());
        return;
    }
    return;
}

void test_parsing_prefix_expression(){
    struct PrefixTest{
        std::string input;
        std::string op;
        std::any val;
    };

    std::vector<PrefixTest> prefix_tests = {
        {"!5;", "!" , (int64_t)5},
        {"-15", "-", (int64_t)15},
        {"!true;", "!", true},
        {"!false;", "!", false},
    };

    for(int i=0; i<prefix_tests.size(); i++){
        auto l = lexer::Lexer(prefix_tests[i].input);
        auto p = Parser(l);
        auto program = p.parse_program();
        check_parse_errors(p.get_errors());

        if(program->statements.size() != 1){
            printf("[error] program does not contain %d statements, got %ld\n", 
                1, program->statements.size());
            return;
        }

        const ExpressionStatement* stmt = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
        if(stmt == nullptr){
            printf("[error] statement is not expression statement\n");
            return;
        }

        // TODO: make it const and implement clone
        PrefixExpression* exp = dynamic_cast<PrefixExpression*>(stmt->expr.get());
        if(exp == nullptr){
            printf("[error] statement is not prefix expression\n");
            return;
        }
        if(exp->op != prefix_tests[i].op){
            printf("[error] operator mismatch actual is '%s' but got %s\n", prefix_tests[i].op.c_str(), 
                exp->op.c_str());
            return;
        }
        std::unique_ptr<Expression> right = std::move(exp->right);

        if(!test_literal_expression(std::move(right), prefix_tests[i].val)){
            return;
        }
    }

    return;
}

void test_parsing_infix_expression(){
    struct InfixTest{
        std::string input;
        std::any left_value;
        std::string op;
        std::any right_value;
    };

    vector<InfixTest> infix_tests = {
    {"5+5;", (int64_t)5, "+", (int64_t)5},
    {"5+5;", (int64_t)5, "+", (int64_t)5},
    {"5-5;", (int64_t)5, "-", (int64_t)5},
    {"5*5;", (int64_t)5, "*", (int64_t)5},
    {"5/5;", (int64_t)5, "/", (int64_t)5},
    {"5>5;", (int64_t)5, ">", (int64_t)5},
    {"5<5;", (int64_t)5, "<", (int64_t)5},
    {"5==5;", (int64_t)5, "==", (int64_t)5},
    {"5!=5;", (int64_t)5, "!=", (int64_t)5},
    {"true == true", true, "==", true},
    {"true != false", true, "!=", false},
    {"false == false", false, "==", false}
    };

    for(int i=0; i<infix_tests.size(); i++){
        auto l = lexer::Lexer(infix_tests[i].input);
        auto p = Parser(l);
        auto program = p.parse_program();
        check_parse_errors(p.get_errors());

        if(program->statements.size() != 1){
            printf("[error] program does not contain %d statements, got %ld\n", 
                1, program->statements.size());
            return;
        }

        const ExpressionStatement* stmt = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
        if(stmt == nullptr){
            printf("[error] statement is not expression statement\n");
            return;
        }

        InfixExpression* inf_expr = dynamic_cast<InfixExpression*>(stmt->expr.get());
        if(inf_expr == nullptr){
            printf("[error] the given statement does not have infix expr\n");
            return;
        }

        std::unique_ptr<Expression> left_expr = std::move(inf_expr->left);
        std::unique_ptr<Expression> right_expr = std::move(inf_expr->right);
        if(!test_literal_expression(std::move(left_expr), infix_tests[i].left_value)){
            return;
        }

        if(inf_expr->op != infix_tests[i].op){
            printf("[error] infix operator mismatch\n");
            return;
        }

        if(!test_literal_expression(std::move(right_expr), infix_tests[i].right_value)){
            return;
        }
    }
    return;
}

void test_operator_precedence_parsing(){
    struct Test{
        std::string input;
        std::string expected;
    };

    vector<Test> tests = {
        {"-a * b", "((-a) * b)"},
        {"!-a", "(!(-a))"},
        {"a + b + c", "((a + b) + c)"},
        {"a + b - c", "((a + b) - c)"},
        {"a * b * c", "((a * b) * c)"},
        {"a * b / c", "((a * b) / c)"},
        {"a + b / c", "(a + (b / c))"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
        {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
        {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5",
         "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5",
         "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"true", "true"},
        {"false", "false"},
        {"3 > 5 == false","((3 > 5) == false)"},
        {"3 < 5 == true","((3 < 5) == true)"},
    };

    for(int i=0; i<tests.size(); i++){
        auto l = lexer::Lexer(tests[i].input);
        auto p = Parser(l);
        auto program = p.parse_program();
        check_parse_errors(p.get_errors());

        auto actual = program->string();
        if(actual != tests[i].expected){
            printf("[error] expected %s, got %s\n", 
                tests[i].expected.c_str(), actual.c_str());
        }
    }

    return;
}

}