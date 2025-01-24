#include "evaluator.h"
#include <cstdint>
#include <memory>
#include <utility>
#include <fmt/format.h>

namespace eval {

std::unique_ptr<object::Error> new_error(std::string message){
    return std::make_unique<object::Error>(message);
}

bool is_error(const object::Object* obj){
    if(obj != nullptr){
        return obj->type() == object::ERROR_OBJ;
    }
    return false;
}

std::unique_ptr<object::Object> eval_minus_prefix_operator_expression(
    std::unique_ptr<object::Object> right){
    if(right->type() != object::INTEGER_OBJ){
        return new_error(fmt::format("unknown operator: -{}", right->type())); 
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
    return new_error(fmt::format("unknown operator: {} {} {}", 
        left->type(), op, right->type()));
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

    return new_error(fmt::format("unknown operator: {} {} {}", 
        left->type(), op, right->type()));
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

    return new_error(fmt::format("unknown operator: {} {}", op, right->type()));
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
    }else if(left_type != right_type){
        return new_error(fmt::format("type mismatch: {} {} {}", 
            left_type, op, right_type)); 
    }
    return new_error(fmt::format("unknown operator: {} {} {}", left_type, op, right_type));
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
    std::unique_ptr<parser::Node> node,
    std::unique_ptr<object::Environment> &environment){
    auto ptr = dynamic_cast<parser::IfExpression*>(node.get());
    auto cond = eval(std::move(ptr->cond), environment);
    if (is_error(dynamic_cast<const object::Object *>(cond.get()))) {
      return cond;
    }
    auto cond_val = is_truthy(std::move(cond)); 
    if(cond_val){
        return eval(std::move(ptr->consequence), environment);
    }else{
        if(ptr->alternative != nullptr){
            return eval(std::move(ptr->alternative), environment);
        }
    }
    return std::make_unique<object::Null>();
}

std::unique_ptr<object::Object> eval_program(std::unique_ptr<parser::Node> node,
    std::unique_ptr<object::Environment> &environment){
    auto result = std::unique_ptr<object::Object>();
    auto program_ptr = dynamic_cast<parser::Program*>(node.get());
    for(int i=0; i<program_ptr->statements.size(); i++){
        result = eval(std::move(program_ptr->statements[i]), environment); 
        if(auto return_value = dynamic_cast<object::ReturnValue*>(result.get())){
            return std::move(return_value->value);
        }
        if(auto err = dynamic_cast<object::Error*>(result.get())){
            return result;
        }
    }
    return result;
}

std::unique_ptr<object::Object> eval_identifier(std::unique_ptr<parser::Node> node,
    std::unique_ptr<object::Environment> &environment){
    auto id_ptr = dynamic_cast<parser::Identifier*>(node.get());
    auto [val, ok] = environment->get(id_ptr->value);
    if(!ok){
        return new_error(fmt::format("identifier not found: {}", id_ptr->value));
    }
    return std::move(val);
}

std::unique_ptr<object::Object> eval_block_statement(std::unique_ptr<parser::Node> node,
    std::unique_ptr<object::Environment> &environment){
    auto result = std::unique_ptr<object::Object>();
    auto block_ptr = dynamic_cast<parser::BlockStatement*>(node.get());
    for(int i=0; i<block_ptr->statements.size(); i++){
        result = eval(std::move(block_ptr->statements[i]), environment); 
        if((result != nullptr)){ 
            if(result->type() == object::RETURN_VALUE_OBJ || result->type() == object::ERROR_OBJ){
                return result;
            }
        }
    }
    return result;
}

std::unique_ptr<object::Object> eval(std::unique_ptr<parser::Node> node, 
    std::unique_ptr<object::Environment> &environment){
    if(auto ptr = dynamic_cast<parser::LetStatement*>(node.get())){
        auto val = eval(std::move(ptr->value), environment);
        if(is_error(val.get())){
            return val;
        }
        environment->set(ptr->name->value, std::move(val));
    }else if(auto ptr = dynamic_cast<parser::Identifier*>(node.get())){
        return eval_identifier(std::move(node), environment);
    }else if(auto ptr = dynamic_cast<parser::ExpressionStatement*>(node.get())){
        return eval(std::move(ptr->expr), environment);
    }else if(auto ptr = dynamic_cast<const parser::IntegerLiteral*>(node.get())){
        return std::make_unique<object::Integer>(ptr->val);
    }else if(auto ptr = dynamic_cast<const parser::Boolean*>(node.get())){
        return std::make_unique<object::Boolean>(ptr->val);
    }else if(auto ptr = dynamic_cast<parser::PrefixExpression*>(node.get())){
        auto right = eval(std::move(ptr->right), environment);
        if(is_error(dynamic_cast<const object::Object*>(right.get()))){
            return right;
        }
        return eval_prefix_expression(ptr->op, std::move(right));
    }else if(auto ptr = dynamic_cast<parser::InfixExpression*>(node.get())){
        auto left = eval(std::move(ptr->left), environment);
        if(is_error(dynamic_cast<const object::Object*>(left.get()))){
            return left;
        }
        auto right = eval(std::move(ptr->right), environment);
        if(is_error(dynamic_cast<const object::Object*>(right.get()))){
            return right;
        }
        return eval_infix_expression(ptr->op, std::move(left), std::move(right));
    }else if(auto ptr = dynamic_cast<parser::BlockStatement*>(node.get())){
        return eval_block_statement(std::move(node), environment);
    }else if(auto ptr = dynamic_cast<parser::IfExpression*>(node.get())){
        return eval_if_expression(std::move(node), environment);
    }else if(auto ptr = dynamic_cast<parser::ReturnStatement*>(node.get())){
        auto val = eval(std::move(ptr->return_value), environment);
        if(is_error(dynamic_cast<const object::Object*>(val.get()))){
            return val;
        }
        return std::make_unique<object::ReturnValue>(std::move(val));
    }else if(auto ptr = dynamic_cast<parser::Program*>(node.get())){
        return eval_program(std::move(node), environment);
    }    
    
    return nullptr;
}

std::unique_ptr<object::Object> test_eval(std::string input){
    auto l = lexer::Lexer(input);
    auto p = parser::Parser(l);
    auto environment = std::make_unique<object::Environment>();
    auto program = p.parse_program();

    return eval(std::move(program), environment);
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