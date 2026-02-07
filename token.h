/*************************************************************************
	> File Name: token.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 00:52:36 2026
 ************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

#include<string>

enum TokenType {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_EQUAL,
    TOK_SEMICOLON,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LET,
    TOK_INPUT,
    TOK_PRINT,
    TOK_IF,
    TOK_ELSE,
    TOK_FOR,
    TOK_WHILE,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_STRING,
    TOK_TRUE,
    TOK_FALSE,
    TOK_COMMA,
    TOK_GREATER,
    TOK_GREATEREQUAL,
    TOK_LESS,
    TOK_LESSEQUAL,
    TOK_EQUALEQUAL,
    TOK_BANG,
    TOK_NOTEQUAL,
    TOK_FUNC,
    TOK_RETURN,
    TOK_BREAK,
    TOK_CONTINUE,
    TOK_UNKNOWN,
};

struct Token {
    TokenType type;
    std::string lexeme;

    Token() : type(TOK_UNKNOWN), lexeme("\0") {}
    Token(TokenType t, const std::string &lex) : type(t), lexeme(lex) {}

    Token& operator=(const Token& t) {
        if (&t == this) return *this;
        type = t.type;
        lexeme = t.lexeme;
        return *this;
    }
};

#endif
