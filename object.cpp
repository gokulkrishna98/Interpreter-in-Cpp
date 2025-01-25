#include "object.h"
#include <cstdio>
#include <fmt/format.h>
#include <memory>


namespace object {

ObjectType Error::type() const {
    return ERROR_OBJ;
}

ObjectType ReturnValue::type() const {
    return RETURN_VALUE_OBJ;
}

ObjectType Boolean::type() const {
    return BOOLEAN_OBJ;
}

ObjectType Integer::type() const {
    return INTEGER_OBJ;
}

ObjectType Null::type() const {
    return NULL_OBJ;
}

ObjectType Function::type() const {
    return FUNCTION_OBJ;
}

std::string Error::inspect() const {
    return fmt::format("ERROR: {}", message);
}

std::string ReturnValue::inspect() const {
    return fmt::format("{}", value->inspect());
}

std::string Boolean::inspect() const {
    return fmt::format("{}", value);
}

std::string Integer::inspect() const {
    return fmt::format("{}", value);
}

std::string Null::inspect() const {
    return "null";
}

std::string Function::inspect() const {
    std::string fn_lit_str = "";
    fn_lit_str += "fn";
    fn_lit_str += "(";
    for(int i=0; i<parameters.size(); i++){
        fn_lit_str += parameters[i]->string();
        if(i < parameters.size()-1){
            fn_lit_str += ", ";
        }
    }
    fn_lit_str += "){\n";
    fn_lit_str += body->string();
    fn_lit_str += "\n}";
    return fn_lit_str;
}

std::unique_ptr<Object> Error::clone() const {
    return std::make_unique<Error>(*this);
}

std::unique_ptr<Object> ReturnValue::clone() const {
    return std::make_unique<ReturnValue>(value->clone());
}

std::unique_ptr<Object> Boolean::clone() const {
    return std::make_unique<Boolean>(this->value);
}

std::unique_ptr<Object> Integer::clone() const {
    return std::make_unique<Integer>(this->value);
}

std::unique_ptr<Object> Null::clone() const {
    return std::make_unique<Null>();
}

// empty clone : have to check
// causes segmentation fault;
std::unique_ptr<Object> Function::clone() const {
    std::vector<std::unique_ptr<parser::Identifier>> c_parameters;
    for (int i=0; i < parameters.size(); i++) {
        auto param = dynamic_cast<parser::Identifier*>(parameters[i]->clone().release());
        c_parameters.push_back(std::unique_ptr<parser::Identifier>(param));
    }

    auto c_body = std::unique_ptr<parser::BlockStatement>(
        static_cast<parser::BlockStatement*>(body->clone().release()));

    auto c_environment = std::make_unique<Environment>(&environment);
    

    // Create and return a new Function object
    return std::make_unique<Function>(std::move(c_parameters),
                                      std::move(c_body), c_environment);
}


std::tuple<std::unique_ptr<object::Object>, bool> Environment::get(std::string name){
    if(store.find(name) != store.end()){
        return std::make_tuple(store[name]->clone(), true);
    }
    return std::make_tuple(nullptr, false);
}

std::unique_ptr<object::Object> Environment::set(std::string name, 
    std::unique_ptr<object::Object> val){
    store[name] = val->clone();
    return val; 
}

std::unique_ptr<Environment> new_enclosed_environment(
    std::unique_ptr<Environment> &outer){
    return std::make_unique<Environment>(&outer);
}

}