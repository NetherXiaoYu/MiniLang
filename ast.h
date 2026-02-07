/*************************************************************************
	> File Name: ast.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 00:43:49 2026
 ************************************************************************/

#ifndef AST_H
#define AST_H

#include<string>
#include<vector>
#include<iostream>
#include<cstring>

class Expr {
public:
    virtual ~Expr() {}
};

class Stmt {
public:
    virtual ~Stmt() {}
};

class Block{
public:
    std::vector<Stmt*> statements;
    ~Block() {
        for(size_t i = 0; i < statements.size(); i++) {
            delete statements[i];
        }
    }
};

class LiteralExpr : public Expr {
public:
    double value;

    LiteralExpr(double v) : value(v) {}
};

class VariableExpr : public Expr {
public:
    std::string name;

    VariableExpr(const std::string &n) : name(n) {}
};

class StringExpr : public Expr{
public:
    std::string str;
    
    StringExpr(const std::string &s) : str(s) {}
};

class BinaryExpr : public Expr {
public:
    Expr *left, *right;
    std::string op;
    BinaryExpr(Expr *l, std::string o, Expr *r) : left(l), op(o), right(r) {};
    ~BinaryExpr() {
        delete left;
        delete right;
    }
};

class CallExpr : public Expr {
public:
    Expr *callee;
    std::vector<Expr *> arguments;
    CallExpr(Expr* call, std::vector<Expr *> args) : callee(call), arguments(args) {}
    ~CallExpr() {
        delete callee;
        for(size_t i = 0; i < arguments.size(); i++) delete arguments[i];
    }
};

class UnaryExpr : public Expr {
public:
    Expr *right;
    std::string op;

    UnaryExpr(std::string o, Expr *r) : op(o), right(r) {};
    ~UnaryExpr() {
        delete right;
    }
};

class AssignExpr : public Expr {
public:
    std::string var_name;
    Expr *value;
    AssignExpr(std::string n, Expr *v) : var_name(n), value(v) {};
    ~AssignExpr() { delete value; }
};

class IfStmt : public Stmt {
public:
    Expr *condition;
    Block *thenBranch, *elseBranch;

    IfStmt(Expr *cond, Block *t, Block *e) : condition(cond), thenBranch(t), elseBranch(e) {}
    ~IfStmt() {
        delete condition;
        delete thenBranch;
        delete elseBranch;
    }
};

class ForStmt : public Stmt {
public:
    Stmt *initializer;
    Expr *condition, *increment;
    Block *body;

    ForStmt(Stmt *init, Expr *cond, Expr *incr, Block *body) : initializer(init), condition(cond), increment(incr), body(body) {};
    ~ForStmt() {
        delete initializer;
        delete condition;
        delete increment;
        delete body;
    }
};

class WhileStmt : public Stmt {
public:
    Expr *condition;
    Block *body;

    WhileStmt(Expr *cond, Block *body) : condition(cond), body(body) {};
    ~WhileStmt() {
        delete condition;
        delete body;
    }
};

class LetStmt : public Stmt {
public:
    std::string name;
    Expr *initializer;

    LetStmt(std::string n, Expr *init) : name(n), initializer(init) {};
    ~LetStmt() {
        delete initializer;
    }
};

class FuncStmt : public Stmt{
public:
    std::string name;
    std::vector<std::string> params;
    Block *body;

    FuncStmt(std::string n, std::vector<std::string> p, Block *body) : name(n), params(p), body(body) {}
    ~FuncStmt() {
        delete body;
    }
};

class ReturnStmt : public Stmt {
public:
    Expr *expr;

    ReturnStmt(Expr *e) : expr(e) {}
    ~ReturnStmt() {
        delete expr;
    }
};

class ExprStmt : public Stmt {
public:
    Expr *expr;

    ExprStmt(Expr *e) : expr(e) {};
    ~ExprStmt() {
        delete expr;
    }
};

class BreakStmt : public Stmt {
public:
    BreakStmt() {}
    ~BreakStmt() {}
};

class ContinueStmt : public Stmt {
public:
    ContinueStmt() {}
    ~ContinueStmt() {}
};

#endif
