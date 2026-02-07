/*************************************************************************
	> File Name: parser.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 02:28:56 2026
 ************************************************************************/

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

class Parser {
    std::vector<Token> __tokens__;
    size_t __current__;

    bool is_at_end() { return __current__ >= __tokens__.size() || __tokens__[__current__].type == TOK_EOF; }

    Token peek() {
        if(is_at_end()) {
            Token eof(TOK_EOF, "\0");
            return eof;
        }
        return __tokens__[__current__];
    }

    Token previous() {
        if(__current__ <= 0) {
            Token dummy(TOK_EOF, "\0");
            return dummy;
        }
        return __tokens__[__current__ - 1];
    }
    
    bool match(TokenType type) { return __tokens__[__current__].type == type; }

    Token advance() {
        if(is_at_end()) {
            Token eof(TOK_EOF, "\0");
            return eof;
        }
        return __tokens__[__current__++];
    }

    // 消费 ; 使用
    Token consume(TokenType type, const char* message) {
        if(match(type)) {
            advance();
            return previous();
        }

        std::cerr << message << ", received: " << __tokens__[__current__].lexeme << std::endl;
        exit(1);
    }

    // Primary Expression
    Expr* parse_primary_expression() {
        if (match(TOK_NUMBER)) {
            return new LiteralExpr(strtod(advance().lexeme.c_str(), nullptr));
        }

        if (match(TOK_STRING)) {
            return new StringExpr(advance().lexeme);
        }

        if (match(TOK_IDENTIFIER)) {
            return new VariableExpr(advance().lexeme);
        }

        if (match(TOK_TRUE)) {
            advance();
            return new LiteralExpr(1.0);
        }

        if (match(TOK_FALSE)) {
            advance();
            return new LiteralExpr(0.0);
        }
        
        // 不用担心会跟函数调用的括号重复
        if (match(TOK_LPAREN)) {
            Expr *expr = parse_expression();
            consume(TOK_RPAREN, "Missing terminating ')' character!");
            return expr;
        }

        std::cerr << "Unknown expression for token " << peek().lexeme <<"("<< __current__ <<")"<<std::endl;
        exit(1);
    }

    Expr* parse_call_expression() {
        Expr* expr = parse_primary_expression();

        while(true) {
            if(match(TOK_LPAREN)) {
                VariableExpr *e = dynamic_cast<VariableExpr *> (expr);
                if (!e) {
                    std::cerr << "Funciton name must be VariableExpr while calling" << std::endl;
                    exit(1);
                }
                advance(); // 吃掉 (
                expr = finish_call(expr);
            } else {
                break;
            }
        }
        
        return expr;
    }

    Expr* finish_call(Expr* callee) {
        std::vector<Expr*> arguments;
        if(!match(TOK_RPAREN)) {
            do {
                if(arguments.size() > 255) {
                    std::cerr << "augments size should be be larger than 255" << std::endl;
                    exit(1);
                }
                arguments.push_back(parse_expression());
                advance();
            } while (previous().type == TOK_COMMA);
        }

        if(previous().type != TOK_RPAREN) {
            std::cerr<< "Missing terminating ')' character"<<std::endl;
            exit(1);
        }

        return new CallExpr(callee, arguments);
    }

    Expr* parse_unary_expression() {
        if (match(TOK_BANG) || match(TOK_MINUS)) {
            Token op = advance();
            Expr *right = parse_unary_expression();
            return new UnaryExpr(op.lexeme, right);
        }

        return parse_call_expression();
    }

    Expr* parse_factor_expression() {
        Expr *expr = parse_unary_expression();
        while (match(TOK_STAR) || match(TOK_SLASH)) {
            Token op = advance();
            Expr *right = parse_unary_expression();
            expr = new BinaryExpr(expr, op.lexeme, right);
        }
        return expr;
    }

    Expr* parse_term_expression() {
        Expr *expr = parse_factor_expression();
        while (match(TOK_PLUS) || match(TOK_MINUS)) {
            Token op = advance();
            Expr *right = parse_factor_expression();
            expr = new BinaryExpr(expr, op.lexeme, right);
        }
        return expr;
    }

    Expr* parse_comparison_expression() {
        Expr *expr = parse_term_expression();
        while (match(TOK_GREATER) || match(TOK_GREATEREQUAL) ||
               match(TOK_LESS) || match(TOK_LESSEQUAL)) {
            Token op = advance();
            Expr *right = parse_term_expression();
            expr = new BinaryExpr(expr, op.lexeme, right);
        }
        return expr;
    }

    Expr* parse_equality_expression() {
        Expr *expr = parse_comparison_expression();
        while (match(TOK_EQUALEQUAL) || match(TOK_NOTEQUAL)) {
            Token op = advance();
            Expr *right = parse_comparison_expression();
            expr = new BinaryExpr(expr, op.lexeme, right);
        }
        return expr;
    }

    Expr* parse_assignment_expression() {
        Expr *expr = parse_equality_expression();

        if (match(TOK_EQUAL)) {
            Token eq = advance();
            Expr *value = parse_assignment_expression();
            
            VariableExpr *var = dynamic_cast<VariableExpr*>(expr);
            if(var) {
                return new AssignExpr(var->name, value);
            } else {
                std::cerr<<"Invalid assignment target."<<std::endl;
                delete expr;
                delete value;
                exit(1);
            }
        }

        return expr;
    }

    Expr* parse_expression() {
        return parse_assignment_expression();
    }

    // 处理 Statement
        
    Stmt* parse_statement() {
        if (match(TOK_IF))          return parse_if_statement();
        if (match(TOK_FOR))         return parse_for_statement();
        if (match(TOK_WHILE))       return parse_while_statement();
        if (match(TOK_LET))         return parse_let_statement();
        if (match(TOK_FUNC))        return parse_func_statement();
        if (match(TOK_RETURN))      return parse_return_statement();
        if (match(TOK_BREAK))       return parse_break_statement();
        if (match(TOK_CONTINUE))    return parse_continue_statement();
    
        return parse_expression_statement();
    }

    Stmt* parse_if_statement() {
        advance(); // 吃掉 if
        consume(TOK_LPAREN, "Expected '(' character after if.");
        Expr *condition = parse_expression();
        consume(TOK_RPAREN, "Missing terminating ')' character");

        Block *thenBranch = parse_block();
        Block *elseBranch = nullptr;

        if (match(TOK_ELSE)) {
            advance(); // 吃掉 else
            if (match(TOK_IF)) {
                Stmt* elIf = parse_if_statement();
                elseBranch = new Block();
                elseBranch->statements.push_back(elIf);
            } else {
                elseBranch = parse_block();
            }
        }

        return new IfStmt(condition, thenBranch, elseBranch);
    }

    Stmt* parse_for_statement() {
        advance(); // 吃掉 for
        consume(TOK_LPAREN, "Expected '(' character after for");

        // 处理 initializer
        Stmt *initializer;
        if (match(TOK_LET)) {
            initializer = parse_let_statement();
        } else if (match(TOK_SEMICOLON)) {
            initializer = nullptr;
        } else {
            initializer = parse_expression_statement();
        }

        // 处理 condition
        Expr* condition = nullptr; 
        if (!match(TOK_SEMICOLON)) {
            condition = parse_expression();
            consume(TOK_SEMICOLON, "Expected ';' after for loop condition.");
        } else advance();

        // 处理 increment
        Expr *increment = nullptr;
        if (!match(TOK_RPAREN)) {
            increment = parse_expression();
        }
        consume(TOK_RPAREN, "Missing terminating ')' character for for loop statement.");

        // std::cout<<"Parsing for body"<<std::endl;
        Block *body = parse_block();
        // std::cout<<"Parsed successfully"<<std::endl;

        return new ForStmt(initializer, condition, increment, body);
    }

    Stmt* parse_while_statement() {
        advance(); // 吃掉 while
        consume(TOK_LPAREN, "Expected '(' character after while.");
        Expr *condition = parse_expression();
        consume(TOK_RPAREN, "Missing terminating ')' character.");

        Block *body = parse_block();
        return new WhileStmt(condition, body);
    }

    Stmt* parse_let_statement() {
        advance(); // 吃掉 let
        Token name = consume(TOK_IDENTIFIER, "Expected identifier as a variable name.");

        Expr* initializer = nullptr;
        if (match(TOK_EQUAL)) {
            advance(); // 吃掉 =
            initializer = parse_expression();
        }

        consume(TOK_SEMICOLON, "Expected ; after let statement.");
        return new LetStmt(name.lexeme, initializer);
    }

    Stmt* parse_func_statement() {
        std::cout << "Parsing function statement." << std::endl;
        advance(); // 吃掉 func 
        Token name = consume(TOK_IDENTIFIER, "Expected identifier as a function name.");
        consume(TOK_LPAREN, "Expected '(' character after function name.");

        std::vector<std::string> params;
        if (!match(TOK_RPAREN)) {
            bool first = true;
            do {
                if (match(TOK_COMMA) && !first) consume(TOK_COMMA, "Parameters should be seperated by , character if there exists multiple parameters.");
                first = false;
                if(params.size() > 255) {
                    std::cerr << "Parameters size must not be larger than 255.";
                    exit(1);
                }

                Token param = consume(TOK_IDENTIFIER, "Expected identifier as a parameter name.");
                params.push_back(param.lexeme);
            } while (match(TOK_COMMA));
        }
        consume(TOK_RPAREN, "Missing terminating ')' character while defining function.");
        std::cout << "Parsing function body ..." << std::endl;
        Block *body = parse_block();
        std::cout << "Parsed function body successfully."<<std::endl;
        return new FuncStmt(name.lexeme, params, body);
    }

    Stmt* parse_return_statement() {
        advance(); // 吃掉 return
        Expr *expr = parse_expression();
        consume(TOK_SEMICOLON, "Expected ; after return statement.");
        return new ReturnStmt(expr);
    }

    Stmt* parse_expression_statement() {
        Expr *expr = parse_expression();
        consume(TOK_SEMICOLON, "Expected ; after expression");
        return new ExprStmt(expr);
    }

    Stmt* parse_break_statement() {
        advance(); // 吃掉 break
        consume(TOK_SEMICOLON, "Expected ; after break");
        return new BreakStmt();
    }

    Stmt* parse_continue_statement() {
        advance(); // 吃掉 continue
        consume(TOK_SEMICOLON, "Expected ; after continue");
        return new ContinueStmt();
    }

    Block* parse_block() {
        Block *block = new Block();
        consume(TOK_LBRACE, "Expected LBRACE as start of block.");
        while (!match(TOK_RBRACE) && !is_at_end()) {
            // std::cout<<"Parsing statement starting with " << __tokens__[__current__].lexeme << std::endl;
            block->statements.push_back(parse_statement());
        }
        consume(TOK_RBRACE, "Missing terminating RBRACE after a block");
        return block;
    }

public:

    explicit Parser(const std::vector<Token> tokens) : __tokens__(tokens), __current__(0) {}

    Block* parse() {
        Block* program = new Block();
        while(!is_at_end()) {
            // std::cout << "Parsing statement starting with " << __tokens__[__current__].lexeme <<std::endl;
            program->statements.push_back(parse_statement());
        }

        return program;
    }
};

#endif
