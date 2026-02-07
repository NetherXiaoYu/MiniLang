/*************************************************************************
	> File Name: main.cpp
	> Author: Bryan Si (SeongLam)
	> Created Time: Sun Feb  8 03:48:27 2026
 ************************************************************************/

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include<iostream>
#include<fstream>
#include<vector>

int main(int argc, char** argv) {

    // 来点版权（不是
    
    std::cout<<"                 __  __ ___ _   _ ___ _        _    _   _  ____"<<std::endl;
    std::cout<<"                |  \\/  |_ _| \\ | |_ _| |      / \\  | \\ | |/ ___|"<<std::endl;
    std::cout<<"                | |\\/| || ||  \\| || || |     / _ \\ |  \\| | |  _"<<std::endl;
    std::cout<<"                | |  | || || |\\  || || |___ / ___ \\| |\\  | |_| |"<<std::endl;
    std::cout<<"                |_|  |_|___|_| \\_|___|_____/_/   \\_\\_| \\_|\\____|"<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"This is a \"Toy\" Language written for teaching, do not use this language for production."<<std::endl;
    std::cout<<"Please don't be hasitate to report any bugs or give any advise on MiniLang Github Page."<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"                Github: https://github.com/NetherXiaoYu/MiniLang"<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<""<<std::endl;
    
    if(argc < 2) {
        std::cerr << "Please enter which program file you are going to run." << std::endl;
        exit(1);
    }

    std::ifstream inFile(argv[1]);
    std::vector<Token> tokens;

    std::cout<<std::endl<<"Extracting Token..." <<std::endl;

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

    std::cout<<"Parsing..." <<std::endl;

    Parser p(tokens);
    Block *program = p.parse();

    std::cout<<"Compiling..." <<std::endl;


    Compiler c(MainCompiler);
    c.compile(program);

    Chunk chk = c.get_chunk();

    VirtualMachine vm;
    for (auto &pair : c.get_user_func()) {
        vm.define_function(pair.second);
    }

    std::cout<<std::endl<<"Result: "<<std::endl;

    vm.run(chk);

    return 9;
}
