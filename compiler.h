/*************************************************************************
	> File Name: compiler.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 18:15:05 2026
 ************************************************************************/

#ifndef COMPILER_H
#define COMPILER_H

#include<vector>
#include<stack>
#include<unordered_map>
#include<string>
#include<iostream>
#include "ast.h"
#include "instruction.h"

class Func {
public:
    std::string name;
    std::vector<std::string> params;
    Chunk __chunk__;

    Func(const std::string &n, std::vector<std::string> p) : name(n), params(p) {}
    Func& operator=(const Func &right) {
        if (&right == this) return *this;
        name = right.name;
        params = right.params;
        __chunk__ = right.__chunk__;
        return *this;
    }
};

struct Loop {
    int start;
    std::vector<int> _break_jump_, _continue_jump_;

    Loop(int s) : start(s) {}
};

enum CompilerType {
    FunctionCompiler,
    MainCompiler,
};

class Compiler {

    Chunk __chunk__; // Global Chunk
    int __tmp_counter__; // 这里的 __tmp_counter__ 是作为寄存器下标存在
    int __max_reg_count__; // 这里记录编译中存在最高的 __tmp_counter__

    std::stack<std::unordered_map<std::string, int>> __scope__;
    std::stack<Loop *> __loop__;
    std::unordered_map<std::string, Func> __user_def_func__;

    CompilerType __type__;

    // Expression Compile
    int compile_expr(Expr *expr);
    int compile_binary_expr(BinaryExpr *expr);
    int compile_unary_expr(UnaryExpr *expr);
    int compile_call_expr(CallExpr *expr);
    int compile_assign_expr(AssignExpr *expr);
    
    // Statement Compile
    void compile_stmt(Stmt *stmt);
    void compile_if_stmt(IfStmt *stmt);
    void compile_while_stmt(WhileStmt *stmt);
    void compile_for_stmt(ForStmt *stmt);
    void compile_let_stmt(LetStmt *stmt);
    void compile_return_stmt(ReturnStmt *stmt);
    void compile_break_stmt(BreakStmt *stmt);
    void compile_continue_stmt(ContinueStmt *stmt);
    void compile_func_stmt(FuncStmt *stmt);
    void compile_expr_stmt(ExprStmt *stmt);
    void compile_block(Block* block);

public:

    Compiler() : __tmp_counter__(0), __max_reg_count__(0), __type__(MainCompiler) {
        // std::cout << "Created Main Compiler (Default)" << std::endl;
        std::unordered_map<std::string, int> base_scope;
        __scope__.push(base_scope);
    }

    Compiler(CompilerType type) : __tmp_counter__(0), __max_reg_count__(0), __type__(type) {
        // std::cout << "Created Specific Type Compiler" << std::endl;
        std::unordered_map<std::string, int> base_scope;
        __scope__.push(base_scope);
    }

    Compiler(CompilerType type, std::vector<std::string> params) : __type__(type) {
        std::unordered_map<std::string, int> base_scope;
        __scope__.push(base_scope);
        for (size_t i = 0; i < params.size(); i++) {
            __scope__.top()[params[i]] = i;
        }
        __tmp_counter__ = static_cast<int> (params.size());
        __max_reg_count__ = __tmp_counter__;
        // std::cout<<"Compiling Function with params size " << __max_reg_count__ << std::endl;
    }

    void compile(Block *block) {
        for(size_t i = 0; i < block->statements.size(); i++) {
            compile_stmt(block->statements[i]);
        }
        __chunk__.__reg_count__ = std::max(__tmp_counter__, __max_reg_count__);
        if (__type__ == MainCompiler) __chunk__.write(OP_HALT, 0, 0, 0);
    }

    Chunk get_chunk() {
        // std::cout << "Returning Chunk with reg count: "<<__chunk__.get_reg_count() << std::endl;
        return __chunk__;
    }

    std::unordered_map<std::string, Func> get_user_func() {
        return __user_def_func__;
    }
};

int Compiler::compile_expr(Expr *expr) {
    if (BinaryExpr *e = dynamic_cast<BinaryExpr *>(expr)) {
        return compile_binary_expr(e);
    } 
    if (UnaryExpr *e = dynamic_cast<UnaryExpr *>(expr)) {
        return compile_unary_expr(e);
    }
    if (CallExpr *e = dynamic_cast<CallExpr *>(expr)) {
        return compile_call_expr(e);
    }

    if (AssignExpr *e = dynamic_cast<AssignExpr *>(expr)) {
        return compile_assign_expr(e);
    }

    if (LiteralExpr *e = dynamic_cast<LiteralExpr *>(expr)) {
        size_t idx = __chunk__.add_const_number(e->value);
        __chunk__.write(OP_CONSTANT, idx, 0, __tmp_counter__++);
        return __tmp_counter__ - 1;
    }

    if (StringExpr *e = dynamic_cast<StringExpr *>(expr)) {
        int idx = __chunk__.add_const_str(e->str);
        __chunk__.write(OP_CONSTANT, ~idx, 0, __tmp_counter__++);
        return __tmp_counter__ - 1;
    }

    if (VariableExpr *e = dynamic_cast<VariableExpr *>(expr)) {
        std::unordered_map<std::string, int>::iterator it = __scope__.top().find(e->name);
        if(it != __scope__.top().end()) {
            __chunk__.write(OP_GET_LOCAL, it->second, 0, __tmp_counter__++);
            return __tmp_counter__ - 1;
        } else {
            std::cerr << "Undefined variable "<<e->name<<std::endl;
            exit(1);
        }
    }

    std::cerr << "Unknown expression "<<std::endl;
    exit(1);
}

int Compiler::compile_binary_expr(BinaryExpr *expr) {
    int left_reg = compile_expr(expr->left);
    int right_reg = compile_expr(expr->right);
    int result_reg = __tmp_counter__++;

    if (expr->op == "+") {
        __chunk__.write(OP_ADD, left_reg, right_reg, result_reg);
    } else if (expr->op == "-") {
        __chunk__.write(OP_SUB, left_reg, right_reg, result_reg);
    } else if (expr->op == "*") {
        __chunk__.write(OP_MUL, left_reg, right_reg, result_reg);
    } else if (expr->op == "/") {
        __chunk__.write(OP_DIV, left_reg, right_reg, result_reg);
    } else if (expr->op == "<") {
        __chunk__.write(OP_LESS, left_reg, right_reg, result_reg);
    } else if (expr->op == ">") {
        __chunk__.write(OP_GREATER, left_reg, right_reg, result_reg);
    } else if (expr->op == "==") {
        __chunk__.write(OP_EQUAL, left_reg, right_reg, result_reg);
    } else if (expr->op == ">=") {
        __chunk__.write(OP_GREATER_EQUAL, left_reg, right_reg, result_reg);
    } else if (expr->op == "<=") {
        __chunk__.write(OP_LESS_EQUAL, left_reg, right_reg, result_reg);
    } else if (expr->op == "!=") {
        __chunk__.write(OP_EQUAL, left_reg, right_reg, result_reg);
        __chunk__.write(OP_NOT, result_reg, 0, __tmp_counter__++);
        result_reg = __tmp_counter__;
    } else {
        std::cerr << "Unsupported binary operator "<<expr->op<<std::endl;
        exit(1);
    }

    return result_reg;
}

int Compiler::compile_unary_expr(UnaryExpr *expr) {
    int src = compile_expr(expr->right);
    int dst = __tmp_counter__++;
    if (expr->op == "!") {
        __chunk__.write(OP_NOT, src, 0, dst);
    } else if(expr->op == "-") {
        int zero_idx = __chunk__.add_const_number(0.0);
        int zero_reg = __tmp_counter__++;
        __chunk__.write(OP_CONSTANT, zero_idx, 0, zero_reg);
        __chunk__.write(OP_SUB, zero_reg, src, dst);
    } else {
        std::cerr<<"Unsupported unary operator "<<expr->op<<std::endl;
        exit(1);
    }
    return dst;
}

int Compiler::compile_call_expr(CallExpr *expr) {
    VariableExpr *callee = dynamic_cast<VariableExpr *>(expr->callee);
    if (!callee) {
        std::cerr << "Function name must be VariableExpr" << std::endl;
        exit(1);
    }
    int fn_idx = __chunk__.add_const_str(callee->name);
    int fn_reg = __tmp_counter__++;
    __chunk__.write(OP_CONSTANT, ~fn_idx, 0, fn_reg);

    std::vector<int> arg_regs;
    for (size_t i = 0; i < expr->arguments.size(); i++) {
        arg_regs.push_back(compile_expr(expr->arguments[i]));
    }

    // 这里一般的做法是需要通过扫描那些寄存器是真的存放着重要的数据
    // 然后把这些数据移到后面的寄存器当中，但是这个方案实现方面太复杂了不够 "mini"
    // 所以我们这里只能用空间换时间了，我们这里虽然空间不 "mini" 但是逻辑和实现很 "mini"

    for (size_t i = 0; i < arg_regs.size(); i++) {
        __chunk__.write(OP_SET_LOCAL, arg_regs[i], 0, __tmp_counter__++);
    }
    
    int result_reg = __tmp_counter__++;

    __chunk__.write(OP_CALL, fn_reg, static_cast<int>(arg_regs.size()), result_reg);
    return result_reg;
}

int Compiler::compile_assign_expr(AssignExpr *expr) {
    std::unordered_map<std::string, int>::iterator it = __scope__.top().find(expr->var_name);
    if (it != __scope__.top().end()) {
        int src = compile_expr(expr->value);
        __chunk__.write(OP_SET_LOCAL, src, 0, it->second);
    } else {
        std::cerr << "Undefined variable "<<expr->var_name<<std::endl;
        exit(1);
    }
    return it->second;
}

void Compiler::compile_stmt(Stmt* stmt) {
    if (IfStmt *s = dynamic_cast<IfStmt *>(stmt))           compile_if_stmt(s);
    if (WhileStmt *s = dynamic_cast<WhileStmt *>(stmt))     compile_while_stmt(s);
    if (ForStmt *s = dynamic_cast<ForStmt *>(stmt))         compile_for_stmt(s);
    if (LetStmt *s = dynamic_cast<LetStmt *>(stmt))         compile_let_stmt(s);
    if (ExprStmt *s = dynamic_cast<ExprStmt *>(stmt))       compile_expr_stmt(s);
    if (BreakStmt *s = dynamic_cast<BreakStmt *>(stmt))     compile_break_stmt(s);
    if (ContinueStmt *s = dynamic_cast<ContinueStmt *>(stmt)) compile_continue_stmt(s);
    if (FuncStmt *s = dynamic_cast<FuncStmt *>(stmt))       compile_func_stmt(s);
    if (ReturnStmt *s = dynamic_cast<ReturnStmt *>(stmt))   compile_return_stmt(s);
    if (Block *s = dynamic_cast<Block *>(stmt))             compile_block(s);
}

void Compiler::compile_block(Block *block) {
    // 往栈中压入一个 scope ，可以保留变量 reg 的同时，更改的时候也会在 top 的 scope 更改；
    __scope__.push(__scope__.top());

    // 处理 block
    for (size_t i = 0; i < block->statements.size(); i++) {
        compile_stmt(block->statements[i]);
    }

    // 弹出最顶的 scope，回到原本的 scope 当中
    __scope__.pop();
}

void Compiler::compile_if_stmt(IfStmt *stmt) {
    int cond_reg = compile_expr(stmt->condition);
    int then_line = static_cast<int>(__chunk__.__code__.size());
    __chunk__.write(OP_JUMP_IF_FALSE, cond_reg, 0, 0); // result 在后面会修改

    compile_block(stmt->thenBranch);

    int end_line = -1;
    if (stmt->elseBranch) {
        end_line = static_cast<int> (__chunk__.__code__.size());
        __chunk__.write(OP_JUMP, 0, 0, 0); // 同样在后面会修改
    }

    __chunk__.__code__[then_line].result = static_cast<int>(__chunk__.__code__.size());

    if (stmt->elseBranch) {
        compile_block(stmt->elseBranch);
        __chunk__.__code__[end_line].arg1 = static_cast<int> (__chunk__.__code__.size());
    }
}

void Compiler::compile_while_stmt(WhileStmt *stmt) {
    int loop_start = static_cast<int> (__chunk__.__code__.size());

    int cond_reg = compile_expr(stmt->condition);
    int exit_line = static_cast<int> (__chunk__.__code__.size());
    __chunk__.write(OP_JUMP_IF_FALSE, cond_reg, 0, 0);

    __loop__.push(new Loop(loop_start));
    int _origin_next_reg_ = __tmp_counter__;
    compile_block(stmt->body);
    __max_reg_count__ = std::max(__tmp_counter__, __max_reg_count__);
    __tmp_counter__ = _origin_next_reg_;
    Loop *l = __loop__.top(); __loop__.pop();

    __chunk__.write(OP_JUMP, loop_start, 0, 0);

    int after_loop = static_cast<int> (__chunk__.__code__.size());
    for (int pc : l->_break_jump_) {
        __chunk__.__code__[pc].arg1 = after_loop;
    }
    __chunk__.__code__[exit_line].result = after_loop;

    delete l;
}

void Compiler::compile_for_stmt(ForStmt *stmt) {
    
    if (stmt->initializer) {
        compile_stmt(stmt->initializer);
    }

    // std::cout << "Compile init statement."<<std::endl;

    int loop_start = static_cast<int> (__chunk__.__code__.size());

    int cond_reg = -1;
    if (stmt->condition) {
        cond_reg = compile_expr(stmt->condition);
    }
    // std::cout << "Compile cond expr."<<std::endl;

    int exit_line = -1;
    if (stmt->condition) {
        exit_line = static_cast<int> (__chunk__.__code__.size());
        __chunk__.write(OP_JUMP_IF_FALSE, cond_reg, 0, 0);
    }

    __loop__.push(new Loop(-1));
    int _origin_next_reg_ = __tmp_counter__;
    
    // std::cout << "Compile for body with loop stack size "<<__loop__.size()<<std::endl;
    compile_block(stmt->body);
    // std::cout << "For body compile success, restoring scope with loop stack size "<< __loop__.size() <<std::endl;
    
    __max_reg_count__ = std::max(__tmp_counter__, __max_reg_count__);
    __tmp_counter__ = _origin_next_reg_;

    int after_body = static_cast<int> (__chunk__.__code__.size());
    if (stmt->increment) {
        compile_expr(stmt->increment);
    }

    __chunk__.write(OP_JUMP, loop_start, 0, 0);

    Loop *l = __loop__.top(); __loop__.pop();
    // std::cout<<"Popped out loop context "<<l<<std::endl;
    // std::cout<<"Processing continue jumps , there are "<< l->_continue_jump_.size()<<" continue"<<std::endl;
    for (int pc : l->_continue_jump_) {
        __chunk__.__code__[pc].arg1 = after_body;
    }

    int after_loop = static_cast<int> (__chunk__.__code__.size());
    // std::cout<<"Processing break jumps , there are "<< l->_break_jump_.size()<<" breaks"<<std::endl;
    for (int pc : l->_break_jump_) {
        __chunk__.__code__[pc].arg1 = after_loop;
    }
    if (stmt->condition) {
        __chunk__.__code__[exit_line].result = after_loop;
    }

    delete l;
}

void Compiler::compile_continue_stmt(ContinueStmt *stmt) {
    if (__loop__.empty()) {
        std::cerr << "Continue outside loop."<<std::endl;
        exit(1);
    }

    Loop *l = __loop__.top();
    if (l->start != -1) {
        __chunk__.write(OP_JUMP, l->start, 0, 0);
    } else {
        l->_continue_jump_.push_back(static_cast<int> (__chunk__.__code__.size()));
        __chunk__.write(OP_JUMP, 0, 0, 0);
    }
}

void Compiler::compile_break_stmt(BreakStmt *stmt) {
    // std::cout<<"Compiling break statement..."<<std::endl;
    if (__loop__.empty()) {
        std::cerr << "Break outside loop" <<std::endl;
        exit(1);
    }

    Loop *l = __loop__.top();
    l->_break_jump_.push_back(static_cast<int> (__chunk__.__code__.size()));
    // std::cout<<"Pushed break to loop stack top, now have breaks: "<<l->_break_jump_.size()<<" written in "<<&l<<std::endl;
    __chunk__.write(OP_JUMP, 0, 0, 0);
}

void Compiler::compile_let_stmt(LetStmt *stmt) {
    int reg = -1;
    if (stmt->initializer) {
        reg = compile_expr(stmt->initializer);
    } else {
        int zero_idx = __chunk__.add_const_number(0.0);
        reg = __tmp_counter__++;
        __chunk__.write(OP_CONSTANT, zero_idx, 0, reg);
    }
    __chunk__.write(OP_REGISTER_LOCAL, reg, 0, __tmp_counter__++);
    __scope__.top()[stmt->name] = __tmp_counter__ - 1;
}

void Compiler::compile_expr_stmt(ExprStmt *stmt) {
    compile_expr(stmt->expr); // 历史遗留问题，其实不应该用 AssignExpr 应该用这个的.
}

void Compiler::compile_func_stmt(FuncStmt *stmt) {
    // std::cout<<"Compiling function " << stmt->name <<std::endl;
    if (__user_def_func__.find(stmt->name) != __user_def_func__.end()) {
        std::cerr << "Redeclare of function " << stmt->name << std::endl;
        exit(1);
    }

    if (__type__ == FunctionCompiler) {
        std::cerr << "You cannot declare a function within a function" << std::endl;
        exit(1);
    }

    Func fn(stmt->name, stmt->params);
    Compiler* fn_compiler = new Compiler(FunctionCompiler, stmt->params);
    
    fn_compiler->compile(stmt->body);

    fn.__chunk__ = fn_compiler->get_chunk();
    __user_def_func__.insert({stmt->name, fn});

}

void Compiler::compile_return_stmt(ReturnStmt *stmt) {
    if (__type__ != FunctionCompiler) {
        std::cerr << "Return outside function" << std::endl;
        return ;
    }

    int reg = -1;
    if (stmt->expr) {
        reg = compile_expr(stmt->expr);
    } else {
        int zero_idx = __chunk__.add_const_number(0.0);
        reg = __tmp_counter__++;
        __chunk__.write(OP_CONSTANT, zero_idx, 0, reg);
    }
    __chunk__.write(OP_RETURN_VAL, reg, 0, 0);
}


#endif
