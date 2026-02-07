/*************************************************************************
	> File Name: test_compiler.cpp
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 01:11:49 2026
 ************************************************************************/

#include "../lexer.h"
#include "../token.h"
#include "../parser.h"
#include "../instruction.h"
#include "../compiler.h"
#include<iostream>
#include<iomanip>
#include<vector>
#include<fstream>

std::string opcode_to_string(Opcode op) {
    if (op == OP_CONSTANT) return std::string("OP_CONSTANT");
    if (op == OP_GET_LOCAL) return std::string("OP_GET_LOCAL");
    if (op == OP_SET_LOCAL) return std::string("OP_SET_LOCAL");
    if (op == OP_REGISTER_LOCAL) return std::string("OP_REGISTER_LOCAL");
    if (op == OP_ADD) return std::string("OP_ADD");
    if (op == OP_SUB) return std::string("OP_SUB");
    if (op == OP_MUL) return std::string("OP_MUL");
    if (op == OP_DIV) return std::string("OP_DIV");
    if (op == OP_EQUAL) return std::string("OP_EQUAL");
    if (op == OP_GREATER) return std::string("OP_GREATER");
    if (op == OP_LESS) return std::string("OP_LESS");
    if (op == OP_NOT) return std::string("OP_NOT");
    if (op == OP_GREATER_EQUAL) return std::string("OP_GREATER_EQUAL");
    if (op == OP_LESS_EQUAL) return std::string("OP_LESS_EQUAL");
    if (op == OP_JUMP) return std::string("OP_JUMP");
    if (op == OP_JUMP_IF_FALSE) return std::string("OP_JUMP_IF_FALSE");
    if (op == OP_CALL) return std::string("OP_CALL");
    if (op == OP_DECL_FUNC) return std::string("OP_DECL_FUNC");
    if (op == OP_RETURN_VAL) return std::string("OP_RETURN_VAL");
    if (op == OP_HALT) return std::string("OP_HALT");
    return std::string("OP_UNKNOWN");
}

int main(int argc, char** argv) {
    
    std::ifstream inFile(argv[1]);
    std::vector<Token> tokens;

    std::string line;
    while(std::getline(inFile, line)) {
        Lexer lexer(line);
        Token token;

        do {
            token = lexer.next();
            if (token.type != TOK_EOF && token.type != TOK_UNKNOWN) tokens.push_back(token);
        } while (token.type != TOK_EOF && token.type != TOK_UNKNOWN);
    }
    Token eof(TOK_EOF, "\0");
    tokens.push_back(eof);

    std::cout << "Extracted Token: " << tokens.size() << std::endl;

    Parser p(tokens);
    Block* program = p.parse();

    std::cout << "Contained Statements: " << program->statements.size() << std::endl;

    Compiler c(MainCompiler);
    c.compile(program);

    Chunk chk = c.get_chunk();
    std::cout << std::setw(5) << "No." << std::setw(20) << "Opcode" << std::setw(6) << "Arg1" << std::setw(6) << "arg2" << std::setw(8) << "Result" << std::endl;
    for (int i = 0; i < chk.__code__.size(); i++) {
        Instruction inst = chk.__code__[i];
        std::cout << std::setw(5) << i << std::setw(20) << opcode_to_string(inst.op) << std::setw(6) << inst.arg1 << std::setw(6) << inst.arg2 << std::setw(8) << inst.result << std::endl;
    }

    std::cout<<std::endl;
    std::cout<<"Constant Number: ";
    for (int i = 0; i < chk.__const_num__.size(); i++) {
        std::cout<<std::setw(5)<<i<<" = "<<std::setw(5)<<chk.__const_num__[i]<<" ";
    }

    std::cout<<std::endl;
    std::cout<<"Constant Str: ";
    for (int i = 0; i < chk.__const_str__.size(); i++) {
        std::cout<<std::setw(5)<<(-i-1)<<" = "<<std::setw(5)<<chk.__const_str__[i]<<" ";
    }
    std::cout<<std::endl;
    
    std::cout<< "Register Count: " << chk.get_reg_count() << std::endl;

    std::unordered_map<std::string, Func> user_func = c.get_user_func();

    std::cout<<std::endl<<"Parsed functions: "<<user_func.size()<<std::endl;

    if (user_func.size() != 0) {
        for(auto it = user_func.begin(); it != user_func.end(); it++) {
            Func fn = it->second;
            std::cout<<std::endl;
            std::cout<<"Function name: "<<it->first<<std::endl;
            std::cout << std::setw(5) << "No." << std::setw(20) << "Opcode" << std::setw(6) << "Arg1" << std::setw(6) << "arg2" << std::setw(8) << "Result" << std::endl;
            for (int i = 0; i < fn.__chunk__.__code__.size(); i++) {
                Instruction inst = fn.__chunk__.__code__[i];
                std::cout << std::setw(5) << i << std::setw(20) << opcode_to_string(inst.op) << std::setw(6) << inst.arg1 << std::setw(6) << inst.arg2 << std::setw(8) << inst.result << std::endl;
            }

            std::cout<<std::endl;
            std::cout<<"Constant Number: ";
            for (int i = 0; i < fn.__chunk__.__const_num__.size(); i++) {
                std::cout<<std::setw(5)<<i<<" = "<<std::setw(5)<<fn.__chunk__.__const_num__[i]<<" ";
            }

            std::cout<<std::endl;
            std::cout<<"Constant Str: ";
            for (int i = 0; i < fn.__chunk__.__const_str__.size(); i++) {
                std::cout<<std::setw(5)<<(-i-1)<<" = "<<std::setw(5)<<fn.__chunk__.__const_str__[i]<<" ";
            }
            std::cout<<std::endl;

            std::cout<< "register Count: "<<fn.__chunk__.get_reg_count() << std::endl;

        }
    }

    std::cout<<std::endl;

    return 0;
}


