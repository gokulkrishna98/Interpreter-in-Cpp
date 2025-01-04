#include <cctype>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <ctype.h>

using namespace std;

enum TokenType {
    ILLEGAL,
    ENDOF,
    // Identifier and literals
    ID,
    INT,
    // Operators
    ASSIGN,
    PLUS,
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
    LET
};

unordered_map<string, TokenType> keywords = {
    {"let", TokenType::LET},
    {"fn", TokenType::FUNCTION}
};

TokenType lookup_id(string id){
    if(keywords.find(id) != keywords.end()){
        return keywords[id];
    }
    return TokenType::ID;
}

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
    string _read_number();
    string _read_identifier();

    Token next_token();
};

void Lexer::_skip_whitespace(){
    while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        _read_char();
    return;
}

void Lexer::_read_char(){
    if(read_position >= input.size()){
        ch = '\0'; 
    }else{
        ch = input[read_position];
    }
    position = read_position;
    read_position++;
    return;
}

string Lexer::_read_number(){
    int start_pos = position;
    while(isdigit(ch)){
        _read_char();
    }
    int end_pos = position-1;
    return input.substr(start_pos, end_pos - start_pos+1);
}

string Lexer::_read_identifier(){
    int start_pos = position;
    while(isalpha(ch)){
        _read_char();
    }
    int end_pos = position-1;
    return input.substr(start_pos, end_pos-start_pos+1); 
}

Token Lexer::next_token(){
    _skip_whitespace();
    Token tok;
    switch (ch) {
        case '=' : {tok = Token(TokenType::ASSIGN, string(1, ch)); break;} 
        case ';' : {tok = Token(TokenType::SEMICOLON, string(1, ch)); break;} 
        case '(' : {tok = Token(TokenType::LPAREN, string(1, ch)); break;}
        case ')' : {tok = Token(TokenType::RPAREN, string(1, ch)); break;} 
        case ',' : {tok = Token(TokenType::COMMA, string(1, ch)); break;} 
        case '+' : {tok = Token(TokenType::PLUS, string(1, ch)); break;} 
        case '{' : {tok = Token(TokenType::LBRAC, string(1, ch)); break;} 
        case '}' : {tok = Token(TokenType::RBRAC, string(1, ch)); break;} 
        case '\0' : {tok = Token(TokenType::ENDOF, ""); break;} 
        default : {
            if(isalpha(ch)){
                tok.val = _read_identifier();
                tok.type = lookup_id(tok.val);
                return tok;
            } else if(isdigit(ch)){
                tok.type = TokenType::INT;
                tok.val = _read_number();
                return tok;
            }
            else{
                tok = Token(TokenType::ILLEGAL, string(1, ch));
            }
        }
    }
    _read_char();
    return tok;
}


void test_next_token(){
    printf("Test case #1\n");
    string input = "=+(){},;";
    vector<Token> result_token = {
        Token(TokenType::ASSIGN, "="),
        Token(TokenType::PLUS, "+"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::LBRAC, "{"),
        Token(TokenType::RBRAC, "}"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::ENDOF, ""),
    };
    Lexer l = Lexer(input);

    unsigned long correct_count = 0;
    for(int i=0; i<result_token.size(); i++){
        Token tok = l.next_token();
        if(result_token[i].type != tok.type){
            printf("[error] token mistmatch. Expected %s but found %s\n", 
                result_token[i].val.c_str(), tok.val.c_str());
        }else{
            correct_count++;
        }
    }
    printf("[%ld/%ld] test cases passed\n", correct_count, result_token.size());

    printf("Test case #2\n");
    input = R"(let five = 5;
let ten = 10;
let add = fn(x, y){
    x + y;
};
let result = add(five, ten);
)";

    result_token = {
        Token(TokenType::LET, "let"),
        Token(TokenType::ID, "five"),
        Token(TokenType::ASSIGN, "="),
        Token(TokenType::INT, "5"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::LET, "let"),
        Token(TokenType::ID, "ten"),
        Token(TokenType::ASSIGN, "="),
        Token(TokenType::INT, "10"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::LET, "let"),
        Token(TokenType::ID, "add"),
        Token(TokenType::ASSIGN, "="),
        Token(TokenType::FUNCTION, "fn"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::ID, "x"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::ID, "y"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::LBRAC, "{"),
        Token(TokenType::ID, "x"),
        Token(TokenType::PLUS, "+"),
        Token(TokenType::ID, "y"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::RBRAC, "}"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::LET, "let"),
        Token(TokenType::ID, "result"),
        Token(TokenType::ASSIGN, "="),
        Token(TokenType::ID, "add"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::ID, "five"),
        Token(TokenType::COMMA, ","),
        Token(TokenType::ID, "ten"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::ENDOF, "")
    }; 
    l = Lexer(input);
    correct_count = 0;
    for(int i=0; i<result_token.size(); i++){
        Token tok = l.next_token();
        if(result_token[i].type != tok.type){
            printf("[error] token mistmatch. Expected %s but found %s\n", 
                result_token[i].val.c_str(), tok.val.c_str());
        }else{
            correct_count++;
        }
    }
    printf("[%ld/%ld] test cases passed\n", correct_count, result_token.size());

}

int main(int argc, char** argv){
    printf("interp running\n");
    test_next_token();
    return 0;
}