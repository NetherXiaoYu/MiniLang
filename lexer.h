/*************************************************************************
	> File Name: lexer.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 00:55:24 2026
 ************************************************************************/

#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include<iostream>
#include<string>
#include<cctype>

class Lexer {
private:
    std::string __source__;
    size_t __pos__; // 当前读取位置
    size_t __start__; // 当前 token 开始位置
    char __current_char__; // 当前字符

    char peek() const  {
        if (__pos__ >= __source__.size()) return '\0';
        return __source__[__pos__];
    }

    char advance() {
        if(__pos__ >= __source__.size()) {
            __current_char__ = '\0';
            return '\0';
        }
        __current_char__ = __source__[__pos__];
        __pos__++;
        return __current_char__;
    }

    bool match(char expected) {
        if (peek() == expected) {
            advance();
            return true;
        }
        return false;
    }

    std::string get_lexeme() const {
        return __source__.substr(__start__, __pos__ - __start__);
    }

    void skip_whitespace() {
        while(isspace(peek())) {
            advance();
        }
    }

    Token identifier() {
        while (isalnum(peek()) || peek() == '_') {
            advance();
        }
        std::string lexeme = get_lexeme();
        if (lexeme == "let") return Token(TOK_LET, lexeme);
        if (lexeme == "if") return Token(TOK_IF, lexeme);
        if (lexeme == "else") return Token(TOK_ELSE, lexeme);
        if (lexeme == "for") return Token(TOK_FOR, lexeme);
        if (lexeme == "while") return Token(TOK_WHILE, lexeme);
        if (lexeme == "false") return Token(TOK_FALSE, lexeme);
        if (lexeme == "true") return Token(TOK_TRUE, lexeme);
        if (lexeme == "func") return Token(TOK_FUNC, lexeme);
        if (lexeme == "break") return Token(TOK_BREAK, lexeme);
        if (lexeme == "continue") return Token(TOK_CONTINUE, lexeme);
        if (lexeme == "return") return Token(TOK_RETURN, lexeme);

        return Token(TOK_IDENTIFIER, lexeme);
    }
    
    Token number() {
        while (isdigit(peek()) || peek() == '.') {
            advance();
        }

        return Token(TOK_NUMBER, get_lexeme());
    }

public:

    explicit Lexer(const std::string &source) : __source__(source), __pos__(0), __start__(0), __current_char__('\0') {
    }

    Token next() {
        skip_whitespace();
        __start__ = __pos__;

        if (__pos__ >= __source__.size()) {
            return Token(TOK_EOF, "");
        }

        char c = advance();

        switch (c) {
            case '+': return Token(TOK_PLUS, "+");
            case '-': return Token(TOK_MINUS, "-");
            case '*': return Token(TOK_STAR, "*");
            case '/': return Token(TOK_SLASH, "/");
            case '(': return Token(TOK_LPAREN, "(");
            case ')': return Token(TOK_RPAREN, ")");
            case '{': return Token(TOK_LBRACE, "{");
            case '}': return Token(TOK_RBRACE, "}");
            case ';': return Token(TOK_SEMICOLON, ";");
            case ',': return Token(TOK_COMMA, ",");
        }

        if (c == '=') {
            if (peek() == '=') {
                advance(); // 吃掉 =
                return Token(TOK_EQUALEQUAL, "==");
            }
            return Token(TOK_EQUAL, "=");
        }

        if (c == '<') {
            if (peek() == '=') { 
                advance();
                return Token(TOK_LESSEQUAL, "<="); 
            }
            return Token(TOK_LESS, "<");
        }

        if (c == '>') {
            if (peek() == '=') { 
                advance();
                return Token(TOK_GREATEREQUAL, ">="); 
            }
            return Token(TOK_GREATER, ">");
        }

        if (c == '!') {
            if (peek() == '=') {
                advance();
                return Token(TOK_NOTEQUAL, "!=");
            }
            return Token(TOK_BANG, "!");
        }

        if (c == '"') {
            while(peek() != '"' && peek() != '\0') {
                advance();
            }

            if (peek() == '"') {
                advance();
            } else {
                std::cerr << "missing terminating \" character" <<std::endl;
                exit(1);
            }
            std::string str = __source__.substr(__start__ + 1, __pos__ - __start__ - 2);
            return Token(TOK_STRING, str);
        }

        if (isalpha(c) || c == '_') {
            return identifier();
        }

        if (isdigit(c)) {
            return number();
        }

        std::string bad(1, c);
        
        return Token(TOK_EOF, "");
    }
};

#endif
