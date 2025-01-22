#include "evaluator.h"
#include <cstdint>
#include <memory>

namespace eval {

std::unique_ptr<object::Object> eval_statements(
    std::vector<std::unique_ptr<parser::Statement>> &stmts){
    std::unique_ptr<object::Object> result;
    for(int i=0; i<stmts.size(); i++){
        result = eval(std::move(stmts[i]));
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