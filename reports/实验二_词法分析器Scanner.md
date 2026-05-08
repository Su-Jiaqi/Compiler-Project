# 编译器设计专题实验二：词法分析器 Scanner 实现

**计算机2203 苏佳琪 2226124199**



## 一、实验目的

本实验在实验一 DFA 思想的基础上，实现一个类 C 语言子集的词法分析器。词法分析器能够读取源程序文件，对输入字符流进行扫描，识别关键字、标识符、数字常量、运算符和界符，并输出 Token 序列，为后续语法分析提供输入。

## 二、实验内容

### 1. 必做内容

本实验实现以下词法类别：

| Token 类型 | 含义 |
|---|---|
| ID | 标识符 |
| NUM | 整数常量 |
| FLO | 浮点常量 |
| INT/FLOAT/VOID | 类型关键字 |
| IF/ELSE/WHILE/RETURN | 控制流关键字 |
| INPUT/PRINT | 输入输出关键字 |
| ADD/SUB/MUL/DIV | 算术运算符 |
| LT/LE/EQ/GT/GE/NE | 关系运算符 |
| ASG/AAS/AAA | 赋值、加等、自增 |
| AND/OR/NOT | 逻辑运算符 |
| LPA/RPA/LBK/RBK/LBR/RBR/CMA/SCO | 界符 |

词法分析器支持：

1. 跳过空格、换行符和制表符；
2. 跳过 `//` 单行注释和 `/* ... */` 多行注释；
3. 识别整数、小数和科学计数法；
4. 区分关键字和普通标识符；
5. 输出 Token 类型、词素和行列位置。

## 三、核心设计

词法分析器使用手写扫描方式实现，核心类为 `Lexer`。

主要接口：

```cpp
std::vector<Token> scan(const std::string& source);
```

Token 结构定义为：

```cpp
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;
};
```

扫描过程采用从左到右的线性扫描方式。对于每个字符，根据其类别进入不同处理分支：

- 字母或下划线开头：识别标识符或关键字；
- 数字开头：识别整数或浮点数；
- 运算符开头：判断单字符或双字符运算符；
- 界符：直接生成对应 Token；
- 空白字符和注释：跳过。

## 四、测试输入

测试源程序为 `input/test.src`：

```c
int a = 3;
int b = 5;
float c = 2.5;
a = a + b * 2;
if (a < 20) {
    print a;
} else {
    print b;
}
while (a < 25) {
    a += 1;
}
return a;
```

![2](E:\course\4.2\CompilerProj\compiler_project\reports\pic\proj2\2.png)



## 五、运行方式

```bash
make
./compiler lexer input/test.src output/tokens.txt
```

运行后，Token 序列会输出到终端，并保存到：

```text
output/tokens.txt
```

## 六、实验结果

部分输出结果如下：

```text
INT     int     (1,1)
ID      a       (1,5)
ASG     =       (1,7)
NUM     3       (1,9)
SCO     ;       (1,10)
INT     int     (2,1)
ID      b       (2,5)
ASG     =       (2,7)
NUM     5       (2,9)
SCO     ;       (2,10)
FLOAT   float   (3,1)
ID      c       (3,7)
ASG     =       (3,9)
FLO     2.5     (3,11)
SCO     ;       (3,14)
```

可以看到，程序正确识别了类型关键字、标识符、赋值符、整数常量、浮点常量和分号。

![1](E:\course\4.2\CompilerProj\compiler_project\reports\pic\proj2\1.png)

## 七、实验总结

本实验完成了一个可用的词法分析器。相比实验一中通用 DFA 的模拟，本实验面向具体程序设计语言的词法规则进行扫描，实现了更完整的 Token 输出。该输出可作为后续 LR(0)、SLR(1) 语法分析和语义分析的输入。
