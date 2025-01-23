#include "object.h"
#include <fmt/format.h>


namespace object {

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


}