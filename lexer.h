#include <string>
#include <unordered_map>
using namespace std;

namespace lexer {

enum TokenType {
    ILLEGAL,
    ENDOF,
    // Identifier and literals
    ID,
    INT,
    // Operators
    ASSIGN,
    PLUS,
    MINUS,
    BANG,
    ASTERISK,
    FSLASH,
    LT,
    GT,
    EQ,
    NEQ,
    // Delimiters
    COMMA,
    SEMICOLON,
    // paranthesis
    LPAREN,
    RPAREN,
    // brackets
    LBRAC,
    RBRAC,
    // keywords
    FUNCTION,
    LET,
    TRUE,
    FALSE,
    IF,
    ELSE,
    RETURN
};


struct Token {
    TokenType type;
    string val;
    // loc(row, col)
    pair<int, int> loc;
    Token() = default;
    Token(TokenType type, string val) : type(type), val(val) {};
};

struct Lexer {
    string input;
    int position = 0;
    int read_position = 0;
    char ch;
    Lexer(string input) : input(input){_read_char();};
    // helper function
    void _skip_whitespace();
    void _read_char();
    char _peek_char();
    string _read_number();
    string _read_identifier();

    Token next_token();
};

string enum_to_string(TokenType t);
void test_next_token();
}