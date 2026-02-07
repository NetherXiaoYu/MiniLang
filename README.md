# MiniLang

MiniLang 是一门由 C++ 编写用于学习的“**玩具语言**”，你可以通过修改 MiniLang 的源代码为这门语言添加更多新的元素（或者修改我没发现的 BUG）！

通过学习如何编写，改进这门语言可以帮助快速学习 C++ 各种概念以及复习编译原理，虽然这门语言中部分地方并不符合我们在编译原理课程或者书本中学习到的那么规范。

## 安装

我是用 G++ 13.3.0 在 Ubuntu 24.04 系统上对这个项目进行测试，我的编译指令如下：

```bash
g++ --std=c++11 main.cpp -o minilang
```

在编译成功后您可以尝试运行我编写的 5 个示范程序：

```bash
./minilang program/program1.ml
```

## 进阶

为了更仔细的学习，我提供了 lexer 提取和 compiler 编译 opcode 的单独输出文件在 `test` 目录中，但我再测试这两份代码的时候并没有传递 `--std=c++11`，所以并不保证一定能够编译成功。

这门语言主要分为四个部分：

1. Lexer：用于提取代码文件中的 Token
2. Parser： 用于将 Token 组合并生成 AST
3. Compiler： 将 AST 转化为 Opcode，这一步更多是为了参考 PHP 的 Opcode 而进行，但是我发现 PHP 的 Opcode 好像用的是 3 地址码，而我用的是 4 地址码
4. VM： 虚拟机用于运行 Opcode

## 改进

欢迎大家发起各种 Pull Request 或者在 Issue 中提出可以改进的部分！
