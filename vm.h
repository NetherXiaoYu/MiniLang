/*************************************************************************
	> File Name: vm.h
	> Author: Bryan Si (SeongLam)
	> Created Time: Fri Feb  6 11:57:55 2026
 ************************************************************************/

#ifndef VM_H
#define VM_H

#include "compiler.h"
#include<vector>
#include<stack>
#include<string>
#include<unordered_map>
#include<iostream>

enum ValueType {
    VAL_NUMBER,
    VAL_STRING,
};

class Value {
public:
    ValueType __type__;
    union {
        double number;
        std::string str;
    };

    Value() : __type__(VAL_NUMBER), number(0.0) {}

    explicit Value(double n) : __type__(VAL_NUMBER), number(n) {}
    explicit Value(const std::string &s) : __type__(VAL_STRING), str(s) {}

    Value(const Value &other) : __type__(other.__type__) {
        if (other.__type__ == VAL_NUMBER) {
            number = other.number;
        } else if (other.__type__ == VAL_STRING) {
            new (&str) std::string(other.str);
        }
    }

    Value& operator=(const Value &right) {
        if(&right == this) return *this;

        __type__ = right.__type__;

        if (right.__type__ == VAL_NUMBER) {
            number = right.number;
        } else if (right.__type__ == VAL_STRING) {
            new (&str) std::string(right.str);
        } else {
            std::cerr << "Unknown value type!" << std::endl;
            exit(1);
        }

        return *this;
    }

    ~Value() {
    }
};

class VirtualMachine;

typedef void (*BuiltinFn)(VirtualMachine *vm, int argc, int* arg_regs, int result_reg);

// 函数调用帧
class CallFrame { 
public:
    const Func *fn; 
    size_t return_pc;
    const Chunk *caller_chunk;
    std::vector<Value> register_file;
    int return_reg;

    CallFrame() : fn(nullptr), return_pc(0), caller_chunk(nullptr), return_reg(0) {}

    CallFrame(const Func *fn, size_t return_pc, const Chunk *chunk) : fn(fn), return_pc(return_pc), caller_chunk(chunk), return_reg(0) {}
    CallFrame(const Func *fn, size_t return_pc, const Chunk *chunk, int return_reg) : fn(fn), return_pc(return_pc), caller_chunk(chunk), return_reg(return_reg) {}
};

// 虚拟机
class VirtualMachine {

    static const int MAX_CALL_DEPTH = 64;

    std::unordered_map<std::string, BuiltinFn> __builtin_func__;
    std::unordered_map<std::string, Func> __user_func__;


    std::stack<CallFrame *> __frame__;

    std::vector<Value> __current_reg__;
    const Chunk* __current_chunk__;
    size_t __tmp_counter__; // 这里的 __tmp_counter__ 是用于计算当下运行过的 opcode 下标

    static void builtin_print(VirtualMachine *vm, int argc, int* arg_regs, int result_reg) {
        if (argc > 0) {
            Value &v = vm->__current_reg__[arg_regs[0]];
            if (v.__type__ == VAL_STRING) {
                std::cout<< v.str;
            } else if (v.__type__ == VAL_NUMBER) {
                std::cout << v.number;
            }
        }

        std::cout<<std::endl;
        vm->__current_reg__[result_reg] = Value(0.0);
    }

    static void builtin_input(VirtualMachine *vm, int argc, int* arg_regs, int result_reg) {
        if (argc != 1) {
            std::cerr << "Runtime Error, int2str expected 1 argument, " << argc << "given. "<<std::endl;
            exit(1);
        }

        Value prompt_value = vm->__current_reg__[arg_regs[0]];

        if (prompt_value.__type__ == VAL_STRING) {
            std::cout << prompt_value.str;
            std::cout.flush();
        } else if (prompt_value.__type__ == VAL_NUMBER) {
            std::cout << prompt_value.number;
            std::cout.flush();
        }

        std::string line;
        std::getline(std::cin, line);

        if (std::cin.eof()) {
            line = "";
        }

        vm->__current_reg__[result_reg] = Value(line);
    }

    static void builtin_str2int(VirtualMachine *vm, int argc, int* arg_regs, int result_reg) {
        if (argc != 1) {
            std::cerr << "Runtime Error, int2str expected 1 argument, " << argc << "given. "<<std::endl;
            exit(1);
        }

        Value &arg = vm->__current_reg__[arg_regs[0]];
        if (arg.__type__ != VAL_STRING) {
            std::cerr << "Runtime error: str2int() argument must be a string" << std::endl;
            exit(1);
        }

        const char* str = arg.str.c_str();
        char* end;
        double num = strtod(str, &end);
    
        if (end == str || *end != '\0') {
            std::cerr << "Runtime error: str2int() invalid number format: '" << str << "'" << std::endl;
            exit(1);
        }
    
        vm->__current_reg__[result_reg] = Value(num);
    }

public:

    VirtualMachine() {
        __builtin_func__["print"] = builtin_print;
        __builtin_func__["input"] = builtin_input;
        __builtin_func__["str2int"] = builtin_str2int; // 一开始我以为 str2int 会被 Lexer 识别为三个 Token，但是后面看看用的是 isalnum 判断就没事了（希望

        // 我们这里将主函数也看成一个 Call frame
        CallFrame *main_frame = new CallFrame();
        __frame__.push(main_frame);

        __current_reg__ = __frame__.top()->register_file;
        __current_chunk__ = nullptr;
        __tmp_counter__ = 0;
    }

    ~VirtualMachine() {
        CallFrame *frame;
        while (!__frame__.empty()) {
            frame = __frame__.top();
            __frame__.pop();
            delete frame;
        }
    }

    void define_function(Func &fn) {
        __user_func__.insert({fn.name, fn});
    }

    void run(const Chunk& main_chunk) {
        __frame__.top()->caller_chunk = &main_chunk;

        int main_reg_cnt = __frame__.top()->caller_chunk->get_reg_count();
        __current_reg__ = __frame__.top()->register_file;
        __current_reg__.resize(main_reg_cnt, Value(0.0));

        __current_chunk__ = &main_chunk;
        __tmp_counter__ = 0;

        while (__tmp_counter__ < __current_chunk__->__code__.size()) {
            Instruction inst = __current_chunk__->__code__[__tmp_counter__++];

            switch (inst.op) {
                case OP_CONSTANT: {
                    if (inst.arg1 >= 0) {
                        // Number Constant
                        __current_reg__[inst.result] = Value(__current_chunk__->__const_num__[inst.arg1]);
                    } else {
                        // String Constant
                        int str_idx = ~inst.arg1;
                        __current_reg__[inst.result] = Value(__current_chunk__->__const_str__[str_idx]);
                    }
                    break;
                }

                case OP_GET_LOCAL: {
                    __current_reg__[inst.result] = __current_reg__[inst.arg1];
                    break;
                }

                case OP_SET_LOCAL: {
                    __current_reg__[inst.result] = __current_reg__[inst.arg1];
                    break;
                }

                case OP_REGISTER_LOCAL: {
                    __current_reg__[inst.result] = __current_reg__[inst.arg1];
                    break;
                }

                case OP_ADD: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number + r.number);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_SUB: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number - r.number);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_MUL: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number * r.number);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_DIV: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        if (r.number == 0) {
                            std::cerr << "Runtime error: divided by zero" << std::endl;
                            exit(1);
                        }

                        __current_reg__[inst.result] = Value(l.number / r.number);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_EQUAL: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    bool eq;
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        eq = (l.number == r.number);
                    } else if (l.__type__ == VAL_STRING && r.__type__ == VAL_STRING) {
                        eq = (std::strcmp(l.str.c_str(), r.str.c_str()) == 0);
                    } else {
                        eq = false;
                    }
                    __current_reg__[inst.result] = Value(eq ? 1.0 : 0.0);
                    break;
                }

                case OP_GREATER: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number > r.number ? 1.0 : 0.0);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_LESS: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number < r.number ? 1.0 : 0.0);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_GREATER_EQUAL: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number >= r.number ? 1.0 : 0.0);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_LESS_EQUAL: {
                    Value &l = __current_reg__[inst.arg1];
                    Value &r = __current_reg__[inst.arg2];
                    if (l.__type__ == VAL_NUMBER && r.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value(l.number <= r.number ? 1.0 : 0.0);
                    } else {
                        std::cerr << "Type mismatch in OP_ADD" << std::endl;
                        exit(1);
                    }
                    break;
                }

                case OP_NOT: {
                    Value &v = __current_reg__[inst.arg1];
                    if (v.__type__ == VAL_NUMBER) {
                        __current_reg__[inst.result] = Value((v.number == 0.0) ? 1.0 : 0.0);
                    } else {
                        __current_reg__[inst.result] = Value(0.0);
                    }
                    break;
                }

                case OP_JUMP: {
                    __tmp_counter__ = static_cast<int> (inst.arg1);
                    break;
                }

                case OP_JUMP_IF_FALSE: {
                    Value &cond = __current_reg__[inst.arg1];
                    bool is_false = (cond.__type__ == VAL_NUMBER && cond.number == 0.0);
                    
                    if (is_false) {
                        __tmp_counter__ = static_cast<int> (inst.result);
                    }

                    break;
                }

                // 最麻烦的来了
                case OP_CALL: {
                    Value &fn_val = __current_reg__[inst.arg1];
                    
                    if (fn_val.__type__ != VAL_STRING) {
                        std::cerr << "Invalid function name, function name must be string at instruction " << __tmp_counter__ << std::endl;
                        exit(1);
                    }

                    std::string fn_name(fn_val.str);
                    int arg_count = inst.arg2;
                    int result_reg = inst.result;

                    // 这里先检查是否为 Builtin function
                    // 写到这里才发现前面没写 User Defined Function 和 Builtin Function 的重名检测
                    // 难受

                    auto builtin_it = __builtin_func__.find(fn_name);
                    if (builtin_it != __builtin_func__.end()) {
                        std::vector<int> arg_regs(arg_count);
                        for (int i = 0; i < arg_count; i++) arg_regs[i] = i + (result_reg - arg_count); // 我们这 Compiler 中如此约定

                        builtin_it->second(this, arg_count, arg_regs.data(), result_reg);
                        continue;
                    } 

                    // 再判断是否为 User Defined Function

                    auto user_def_it = __user_func__.find(fn_name);
                    if (user_def_it != __user_func__.end()) {
                        const Func &fn = user_def_it->second;

                        if (static_cast<int> (fn.params.size()) != arg_count) {
                            std::cerr << "Runtime Error: Arugment mismatch for function "<< fn.name << ", expected " << fn.params.size() << ", " << arg_count << " given" << std::endl;
                            exit(1);
                        }

                        size_t return_pc = __tmp_counter__;
                        const Chunk* caller_chunk = __current_chunk__;

                        CallFrame *frame = new CallFrame(&fn, return_pc, caller_chunk, result_reg);
                        frame->register_file.resize(fn.__chunk__.get_reg_count(), Value(0.0));
                        for (int i = 0; i < arg_count; i++) {
                            frame->register_file[i] = __current_reg__[i + (result_reg - arg_count)];
                        }

                        __frame__.top()->register_file = __current_reg__; // 先保存当下的寄存器

                        __frame__.push(frame);

                        __current_reg__ = __frame__.top()->register_file;
                        __current_chunk__ = &(fn.__chunk__);
                        __tmp_counter__ = 0;

                        continue;

                    } else {
                        std::cerr << "Runtime Error: Undefined function " << fn_name << std::endl;
                        exit(1);
                    }

                    break;
                }

                case OP_RETURN_VAL: {
                    Value ret_val = __current_reg__[inst.arg1];
                    int return_reg = __frame__.top()->return_reg;

                    CallFrame *frame = __frame__.top();
                    __frame__.pop();

                    if (__frame__.empty()) {
                        std::cerr << "Invalid return, returning value in main program." << std::endl;
                        exit(1);
                    }

                    __current_reg__ = __frame__.top()->register_file;
                    __current_chunk__ = frame->caller_chunk;
                    __tmp_counter__ = frame->return_pc;

                    __current_reg__[return_reg] = ret_val;

                    delete frame;
                    break;
                }

                case OP_HALT: {
                    return;
                }

                default: {
                    std::cerr << "Runtime Error: unknown opcode in instruction " << __tmp_counter__ << std::endl;
                    exit(1);
                }
            }
        }
    }

};

#endif
