#include "object.h"
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

}