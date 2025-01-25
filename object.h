#include "parser.h"
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <vector>


namespace object {
using ObjectType = std::string;

const std::string BOOLEAN_OBJ = "BOOLEAN";
const std::string INTEGER_OBJ  = "INTEGER";
const std::string NULL_OBJ = "NULL";
const std::string RETURN_VALUE_OBJ = "RETURN_VALUE";
const std::string ERROR_OBJ = "ERROR";
const std::string FUNCTION_OBJ = "FUNCTION";

struct Object {
    virtual ObjectType type() const = 0;
    virtual std::string inspect() const = 0;
    virtual std::unique_ptr<Object> clone() const = 0;
    virtual ~Object() {};
};

struct Error : Object {
    std::string message;
    ObjectType type() const override;
    std::string inspect() const override;
    std::unique_ptr<Object> clone() const override;
    Error(std::string message) : message(message) {};
};

struct ReturnValue : Object {
    std::unique_ptr<Object> value;
    ObjectType type() const override;
    std::string inspect() const override; 
    std::unique_ptr<Object> clone() const override;
    ReturnValue(std::unique_ptr<Object> value) : value(std::move(value)) {};
};

struct Integer : Object {
    int64_t value;
    ObjectType type() const override;
    std::string inspect() const override;
    std::unique_ptr<Object> clone() const override;
    Integer(int64_t value) : value(value) {};
};

struct Boolean : Object {
    bool value;
    ObjectType type() const override;
    std::string inspect() const override;
    std::unique_ptr<Object> clone() const override;
    Boolean(bool value) : value(value) {};
};

struct Null : Object {
    ObjectType type() const override;
    std::string inspect() const override;
    std::unique_ptr<Object> clone() const override;
};

struct Environment {
    std::unique_ptr<Environment> *outer;
    std::unordered_map<std::string, std::unique_ptr<object::Object>> store;
    std::tuple<std::unique_ptr<object::Object>, bool> get(std::string name);
    std::unique_ptr<object::Object> set(std::string name, std::unique_ptr<object::Object>val);
    Environment(std::unique_ptr<Environment> *outer) : outer(outer) {}
};

struct Function : Object {
    std::vector<std::unique_ptr<parser::Identifier>> parameters;
    std::unique_ptr<parser::BlockStatement> body;
    std::unique_ptr<Environment> &environment;
    ObjectType type() const override;
    std::string inspect() const override;
    std::unique_ptr<Object> clone() const override;

    Function(std::vector<std::unique_ptr<parser::Identifier>> params,
        std::unique_ptr<parser::BlockStatement> body,
        std::unique_ptr<Environment> &environment): 
        parameters(std::move(params)), body(std::move(body)), 
        environment(environment){};
};

std::unique_ptr<Environment> new_enclosed_environment(std::unique_ptr<Environment> &outer);


} 