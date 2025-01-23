#include "evaluator.h"
#include <cstdint>
#include <memory>
#include <utility>

namespace eval {


std::unique_ptr<object::Object> eval_statements(
    std::vector<std::unique_ptr<parser::Statement>> &stmts){
    std::unique_ptr<object::Object> result;
    for(int i=0; i<stmts.size(); i++){
        result = eval(std::move(stmts[i]));
        if(auto return_value = dynamic_cast<object::ReturnValue*>(result.get())){
            return std::move(return_value->value);
        }
    }
    return result;
}

std::unique_ptr<object::Object> eval_minus_prefix_operator_expression(
    std::unique_ptr<object::Object> right){
    if(right->type() != object::INTEGER_OBJ){
        return std::make_unique<object::Null>();
    }

    auto ptr = dynamic_cast<object::Integer*>(right.get());
    ptr->value = -1 * ptr->value;
    return right;
}

std::unique_ptr<object::Object> eval_boolean_infix_expression(std::string op,
    std::unique_ptr<object::Object> left, std::unique_ptr<object::Object> right){
    auto left_val = dynamic_cast<object::Boolean*>(left.get())->value;
    auto right_val = dynamic_cast<object::Boolean*>(right.get())->value;

    if(op == "=="){
        return std::make_unique<object::Boolean>(left_val == right_val);
    }else if(op == "!="){
        return std::make_unique<object::Boolean>(left_val != right_val);
    }

    return std::make_unique<object::Null>();
}

std::unique_ptr<object::Object> eval_integer_infix_expression(std::string op,
    std::unique_ptr<object::Object> left, std::unique_ptr<object::Object> right){
    auto left_val = dynamic_cast<object::Integer*>(left.get())->value;
    auto right_val = dynamic_cast<object::Integer*>(right.get())->value;

    if(op == "+"){
        return std::make_unique<object::Integer>(left_val + right_val);
    }else if(op == "-"){
        return std::make_unique<object::Integer>(left_val - right_val);
    }else if(op == "*"){
        return std::make_unique<object::Integer>(left_val * right_val);
    }else if(op == "/"){
        return std::make_unique<object::Integer>(left_val / right_val);
    }else if(op == "<"){
        return std::make_unique<object::Boolean>(left_val < right_val);
    }else if(op == ">"){
        return std::make_unique<object::Boolean>(left_val > right_val);
    }else if(op == "=="){
        return std::make_unique<object::Boolean>(left_val == right_val);
    }else if(op == "!="){
        return std::make_unique<object::Boolean>(left_val != right_val);
    }

    return std::make_unique<object::Null>();
}

std::unique_ptr<object::Object> eval_bang_operator_expression(std::unique_ptr<object::Object> right){
    if(auto ptr = dynamic_cast<object::Boolean*>(right.get())){
        if(ptr->value == true){
            return std::make_unique<object::Boolean>(false);
        }else{
            return std::make_unique<object::Boolean>(true);
        }
    }else if(auto ptr = dynamic_cast<object::Null*>(right.get())){
        return std::make_unique<object::Boolean>(true);
    }else if(auto ptr = dynamic_cast<object::Integer*>(right.get())){
        if(ptr->value == 0){
            return std::make_unique<object::Boolean>(true);
        }else{
            return std::make_unique<object::Boolean>(false);
        }
    }
    return std::make_unique<object::Boolean>(false);
}

std::unique_ptr<object::Object> eval_prefix_expression(std::string op, 
    std::unique_ptr<object::Object> right){
    if(op == "!"){
        return eval_bang_operator_expression(std::move(right));
    }else if(op == "-"){
        return eval_minus_prefix_operator_expression(std::move(right));
    }

    return std::make_unique<object::Null>();
}

std::unique_ptr<object::Object> eval_infix_expression(std::string op,
    std::unique_ptr<object::Object> left,
    std::unique_ptr<object::Object> right){
    auto left_type = left->type();
    auto right_type = right->type();


    if(left_type == object::INTEGER_OBJ && right_type == object::INTEGER_OBJ){
        return eval_integer_infix_expression(op, std::move(left), std::move(right));
    }else if(left_type == object::BOOLEAN_OBJ && right_type == object::BOOLEAN_OBJ){
        return eval_boolean_infix_expression(op, std::move(left), std::move(right));
    }

    return std::make_unique<object::Null>();
}

bool is_truthy(std::unique_ptr<object::Object> obj){
    if(auto ptr = dynamic_cast<object::Null*>(obj.get())){
        return false;
    }else if(auto ptr = dynamic_cast<object::Boolean*>(obj.get())){
        return ptr->value;
    }
    return true;
}

std::unique_ptr<object::Object> eval_if_expression(
    std::unique_ptr<parser::Node> node){
    auto ptr = dynamic_cast<parser::IfExpression*>(node.get());
    auto cond = eval(std::move(ptr->cond));
    auto cond_val = is_truthy(std::move(cond)); 
    if(cond_val){
        return eval(std::move(ptr->consequence));
    }else{
        if(ptr->alternative != nullptr){
            return eval(std::move(ptr->alternative));
        }
    }
    return std::make_unique<object::Null>();
}

std::unique_ptr<object::Object> eval_program(std::unique_ptr<parser::Node> node){
    auto result = std::unique_ptr<object::Object>();
    auto program_ptr = dynamic_cast<parser::Program*>(node.get());
    for(int i=0; i<program_ptr->statements.size(); i++){
        result = eval(std::move(program_ptr->statements[i])); 
        if(auto return_value = dynamic_cast<object::ReturnValue*>(result.get())){
            return std::move(return_value->value);
        }
    }
    return result;
}

std::unique_ptr<object::Object> eval_block_statement(std::unique_ptr<parser::Node> node){
    auto result = std::unique_ptr<object::Object>();
    auto block_ptr = dynamic_cast<parser::BlockStatement*>(node.get());
    for(int i=0; i<block_ptr->statements.size(); i++){
        result = eval(std::move(block_ptr->statements[i])); 
        if((result != nullptr) && (result->type() == object::RETURN_VALUE_OBJ)){
            return result;
        }
    }
    return result;
}

std::unique_ptr<object::Object> eval(std::unique_ptr<parser::Node> node){
    if(auto ptr = dynamic_cast< parser::Program*>(node.get())){
        return eval_statements(ptr->statements);
    }else if(auto ptr = dynamic_cast<parser::ExpressionStatement*>(node.get())){
        return eval(std::move(ptr->expr));
    }else if(auto ptr = dynamic_cast<const parser::IntegerLiteral*>(node.get())){
        return std::make_unique<object::Integer>(ptr->val);
    }else if(auto ptr = dynamic_cast<const parser::Boolean*>(node.get())){
        return std::make_unique<object::Boolean>(ptr->val);
    }else if(auto ptr = dynamic_cast<parser::PrefixExpression*>(node.get())){
        auto right = eval(std::move(ptr->right));
        return eval_prefix_expression(ptr->op, std::move(right));
    }else if(auto ptr = dynamic_cast<parser::InfixExpression*>(node.get())){
        auto left = eval(std::move(ptr->left));
        auto right = eval(std::move(ptr->right));
        return eval_infix_expression(ptr->op, std::move(left), std::move(right));
    }else if(auto ptr = dynamic_cast<parser::BlockStatement*>(node.get())){
        return eval_block_statement(std::move(node));
    }else if(auto ptr = dynamic_cast<parser::IfExpression*>(node.get())){
        return eval_if_expression(std::move(node));
    }else if(auto ptr = dynamic_cast<parser::ReturnStatement*>(node.get())){
        auto val = eval(std::move(ptr->return_value));
        return std::make_unique<object::ReturnValue>(std::move(val));
    }else if(auto ptr = dynamic_cast<parser::Program*>(node.get())){
        return eval_program(std::move(node));
    }else if(auto ptr = dynamic_cast<parser::BlockStatement*>(node.get())){
        return eval_block_statement(std::move(node));
    }
    return nullptr;
}

std::unique_ptr<object::Object> test_eval(std::string input){
    auto l = lexer::Lexer(input);
    auto p = parser::Parser(l);
    auto program = p.parse_program();

    return eval(std::move(program));
}

void test_integer_object(std::unique_ptr<object::Object> evaluated, int64_t expected){
    auto result = dynamic_cast<const object::Integer*>(evaluated.get());
    if(result == nullptr){
        printf("[error] object is not integer \n");
        return;
    }
    if(result->value != expected){
        printf("[error] integer value does not match: eval: %ld, exp: %ld\n",
            result->value, expected);
        return;
    }
    return;
}

void test_eval_integer_expression(){
    struct Test {
        std::string input;
        int64_t expected;
    };

    std::vector<Test> tests = {
        {"5", 5},
        {"12", 12},
    };

    for(int i=0; i<tests.size(); i++){
        auto evaluated = test_eval(tests[i].input);
        test_integer_object(std::move(evaluated), tests[i].expected);
    }
}

}