#include "lexer.h"
#include <unordered_map>
#include <vector>
#include <ctype.h>
#include <cstdio>
#include <cctype>

using namespace std;

namespace lexer {

unordered_map<TokenType, string> enum_to_string_map = {
    {TokenType::ILLEGAL, "ILLEGAL"},
    {TokenType::ENDOF, "ENDOF"},
    {TokenType::ID, "ID"},
    {TokenType::INT, "INT"},
    {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::BANG, "BANG"},
    {TokenType::ASTERISK, "ASTERISK"},
    {TokenType::FSLASH, "FSLASH"},
    {TokenType::LT, "LT"},
    {TokenType::GT, "GT"},
    {TokenType::EQ, "EQ"},
    {TokenType::NEQ, "NEQ"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::LBRAC, "LBRAC"},
    {TokenType::RBRAC, "RBRAC"},
    {TokenType::FUNCTION, "FUNCTION"},
    {TokenType::LET, "LET"},
    {TokenType::TRUE, "TRUE"},
    {TokenType::FALSE, "FALSE"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::RETURN, "RETURN"},
};

unordered_map<string, TokenType> keywords = {
    {"let", TokenType::LET},
    {"fn", TokenType::FUNCTION},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"return", TokenType::RETURN}
};

string enum_to_string(TokenType t){
    return enum_to_string_map[t];
}

TokenType lookup_id(string id){
    if(keywords.find(id) != keywords.end()){
        return keywords[id];
    }
    return TokenType::ID;
}

void Lexer::_skip_whitespace(){
    while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        _read_char();
    return;
}

char Lexer::_peek_char(){
    if(read_position >= input.size()){
        return '\0';
    }else{
        return input[read_position];
    }
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
        case '=' : {
            if(_peek_char() == '='){
                _read_char();
                tok = Token(TokenType::EQ, "==");
            }else{
                tok = Token(TokenType::ASSIGN, string(1, ch));
            }
            break;
        } 
        case ';' : {tok = Token(TokenType::SEMICOLON, string(1, ch)); break;} 
        case '(' : {tok = Token(TokenType::LPAREN, string(1, ch)); break;}
        case ')' : {tok = Token(TokenType::RPAREN, string(1, ch)); break;} 
        case ',' : {tok = Token(TokenType::COMMA, string(1, ch)); break;} 
        case '+' : {tok = Token(TokenType::PLUS, string(1, ch)); break;} 

        case '-' : {tok = Token(TokenType::MINUS, string(1, ch)); break;} 
        case '!' : {
            if(_peek_char() == '='){
                _read_char();
                tok = Token(TokenType::NEQ, "!=");
            }else{
                tok = Token(TokenType::BANG, string(1, ch));
            }
            break;
        } 
        case '*' : {tok = Token(TokenType::ASTERISK, string(1, ch)); break;} 
        case '/' : {tok = Token(TokenType::FSLASH, string(1, ch)); break;} 
        case '<' : {tok = Token(TokenType::LT, string(1, ch)); break;} 
        case '>' : {tok = Token(TokenType::GT, string(1, ch)); break;} 

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
            } else{
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
!-/*5;
5 < 10 > 5;

if (5 < 10) {
    return true;
} else {
    return false;
}

10 == 10;
10 != 9;

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
        Token(TokenType::BANG, "!"),
        Token(TokenType::MINUS, "-"),
        Token(TokenType::FSLASH, "/"),
        Token(TokenType::ASTERISK, "*"),
        Token(TokenType::INT, "5"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::INT, "5"),
        Token(TokenType::LT, "<"),
        Token(TokenType::INT, "10"),
        Token(TokenType::GT, ">"),
        Token(TokenType::INT, "5"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::IF, "if"),
        Token(TokenType::LPAREN, "("),
        Token(TokenType::INT, "5"),
        Token(TokenType::LT, "<"),
        Token(TokenType::INT, "10"),
        Token(TokenType::RPAREN, ")"),
        Token(TokenType::LBRAC, "{"),
        Token(TokenType::RETURN, "return"),
        Token(TokenType::TRUE, "true"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::RBRAC, "}"),
        Token(TokenType::ELSE, "else"),
        Token(TokenType::LBRAC, "{"),
        Token(TokenType::RETURN, "return"),
        Token(TokenType::FALSE, "false"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::RBRAC, "}"),
        Token(TokenType::INT, "10"),
        Token(TokenType::EQ, "=="),
        Token(TokenType::INT, "10"),
        Token(TokenType::SEMICOLON, ";"),
        Token(TokenType::INT, "10"),
        Token(TokenType::NEQ, "!="),
        Token(TokenType::INT, "9"),
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

}