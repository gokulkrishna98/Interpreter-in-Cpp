#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "object.h"
#include <memory>


namespace eval {

std::unique_ptr<object::Object> eval(std::unique_ptr<parser::Node> node,
    std::unique_ptr<object::Environment> &environment);
std::unique_ptr<object::Object> test_eval(std::string input);

void test_integer_object();
void test_eval_integer_expression();

}

#endif