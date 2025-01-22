#include <cstdint>
#include <string>

namespace object {

using ObjectType = std::string;
struct Object {
    virtual ObjectType type() const = 0;
    virtual std::string inspect() const = 0;
    virtual ~Object() {};
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
};


struct Null : Object {
    ObjectType type() const;
    std::string inspect() const;
};


} 