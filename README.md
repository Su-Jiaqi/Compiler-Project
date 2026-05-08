# 编译器设计专题实验统一工程

本工程把实验一到实验六组织在同一个 compiler project 中：

- 实验一：DFA 五元组加载、合法性检查、字符串识别、可接受串生成
- 实验二：词法分析器 Lexer / Scanner
- 实验三：LR(0) 项目集规范族生成
- 实验四：SLR(1) ACTION/GOTO 分析表生成
- 实验五：SLR 引导的语义分析思想落地：AST + 符号表 + 基本语义检查
- 实验六：中间代码生成：四元式形式

## 编译

```bash
make
```

## 一键运行演示

```bash
make run-demo
```

输出文件会生成在 `output/` 目录下。

## 单独运行

```bash
./compiler dfa input/dfa_in.dfa 4 abba
./compiler lexer input/test.src output/tokens.txt
./compiler lr0 input/grammar_expr.txt output/lr0_items.txt
./compiler slr input/grammar_expr.txt output/slr_table.txt
./compiler ir input/test.src output
./compiler all input/test.src input/grammar_expr.txt output
```

## 清理

```bash
make clean
```
