/*************************************************************************
	> File Name: instruction.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Thu Feb  5 17:31:10 2026
 ************************************************************************/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include<string>

enum Opcode {
    OP_CONSTANT,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_REGISTER_LOCAL,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_NOT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_CALL,
    OP_DECL_FUNC,
    OP_RETURN_VAL,
    OP_HALT,
};

struct Instruction {
    Opcode op;
    int arg1, arg2, result;

    Instruction(Opcode op) : op(op), arg1(0), arg2(0), result(0) {}
    Instruction(Opcode op, int arg1, int arg2, int res) : op(op), arg1(arg1), arg2(arg2), result(res) {}
};

class Chunk {
public: 
    std::vector<Instruction> __code__;
    std::vector<double> __const_num__;
    std::vector<std::string> __const_str__;

    int __reg_count__ = 0;

    void write(Opcode op, int arg1, int arg2, int result) {
        __code__.push_back(Instruction(op, arg1, arg2, result));
    }

    size_t add_const_number(double val) {
        __const_num__.push_back(val);
        return __const_num__.size() - 1;
    }

    int get_reg_count() const {
        return __reg_count__;
    }

    size_t add_const_str(std::string str) {
        __const_str__.push_back(str);
        return __const_str__.size() - 1;
    }

    Chunk& operator=(const Chunk& right) {
        if (&right == this) return *this;
        __code__ = right.__code__;
        __const_num__ = right.__const_num__;
        __const_str__ = right.__const_str__;
        __reg_count__ = right.__reg_count__;
        return *this;
    }
};

#endif
