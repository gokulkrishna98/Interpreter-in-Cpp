#include <cstdint>
#include <string>
#include <memory>

namespace object {
using ObjectType = std::string;

const std::string BOOLEAN_OBJ = "BOOLEAN";
const std::string INTEGER_OBJ  = "INTEGER";
const std::string NULL_OBJ = "NULL";
const std::string RETURN_VALUE_OBJ = "RETURN_VALUE";

struct Object {
    virtual ObjectType type() const = 0;
    virtual std::string inspect() const = 0;
    virtual ~Object() {};
};

struct ReturnValue : Object {
    std::unique_ptr<Object> value;
    ObjectType type() const;
    std::string inspect() const;
    ReturnValue(std::unique_ptr<Object> value) : value(std::move(value)) {};
};

struct Integer : Object {
    int64_t value;
    ObjectType type() const;
    std::string inspect() const;
    Integer(int64_t value) : value(value) {};
};

struct Boolean : Object {
    bool value;
    ObjectType type() const;
    std::string inspect() const;
    Boolean(bool value) : value(value) {};
};


struct Null : Object {
    ObjectType type() const;
    std::string inspect() const;
};


} 