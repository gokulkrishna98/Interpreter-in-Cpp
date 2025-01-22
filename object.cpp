#include "object.h"
#include <fmt/format.h>


namespace object {

const std::string BOOLEAN_OBJ = "BOOLEAN";
const std::string INTEGER_OBJ  = "INTEGER";
const std::string NULL_OBJ = "NULL";

ObjectType Boolean::type() const {
    return BOOLEAN_OBJ;
}

ObjectType Integer::type() const {
    return INTEGER_OBJ;
}

ObjectType Null::type() const {
    return NULL_OBJ;
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


}