/*************************************************************************
	> File Name: test_lexer.cpp
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 01:11:49 2026
 ************************************************************************/

#include "../lexer.h"
#include "../token.h"
#include<iostream>
#include<vector>
#include<fstream>

int main(int argc, char** argv) {
    
    std::ifstream inFile(argv[1]);
    std::vector<Token> tokens;

    std::string line;
    while(std::getline(inFile, line)) {
        Lexer lexer(line);
        Token token;

        do {
            token = lexer.next();
            std::cout<<"Received Token: "<<token.lexeme<<std::endl;
            if (token.type != TOK_EOF && token.type != TOK_UNKNOWN) tokens.push_back(token);
        } while (token.type != TOK_EOF && token.type != TOK_UNKNOWN);
    }
    Token eof(TOK_EOF, "\0");
    tokens.push_back(eof);


    for (size_t i = 0; i < tokens.size(); i++) {
        std::cout<<"Token : ";
        switch(tokens[i].type) {
            case TOK_LET:        std::cout << "LET"; break;
            case TOK_PRINT:      std::cout << "PRINT"; break;
            case TOK_IDENTIFIER: std::cout << "IDENT(" << tokens[i].lexeme << ")"; break;
            case TOK_NUMBER:     std::cout << "NUMBER(" << tokens[i].lexeme << ")"; break;
            case TOK_PLUS:       std::cout << "PLUS"; break;
            case TOK_EQUAL:      std::cout << "EQUAL"; break;
            case TOK_SEMICOLON:  std::cout << "SEMICOLON"; break;
            case TOK_LPAREN:     std::cout << "LPAREN"; break;
            case TOK_RPAREN:     std::cout << "RPAREN"; break;
            case TOK_EOF:        std::cout << "EOF"; break;
            case TOK_IF:         std::cout << "IF"; break;
            case TOK_LBRACE:     std::cout << "LBRACE"; break;
            case TOK_RBRACE:     std::cout << "RBRACE"; break;
            case TOK_INPUT:      std::cout << "INPUT"; break;
            case TOK_LESSEQUAL:  std::cout << "LESSQUEAL"; break;
            default:             std::cout << "UNKNOWN"; break;
        }
        std::cout<<std::endl;
    }

    return 0;
}


