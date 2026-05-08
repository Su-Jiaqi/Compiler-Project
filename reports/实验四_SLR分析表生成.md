# 编译器设计专题实验四：SLR(1) 分析表生成

**计算机2203 苏佳琪 2226124199**



## 一、实验目的

本实验在实验三 LR(0) 项目集规范族的基础上，进一步计算 FOLLOW 集，并构造 SLR(1) 语法分析表。SLR(1) 分析表包括 ACTION 表和 GOTO 表，用于指导自底向上的移进-规约分析过程。

## 二、实验内容

本实验完成以下功能：

1. 读取上下文无关文法；
2. 自动增广文法；
3. 构造 LR(0) 项目集规范族；
4. 计算 FIRST 集和 FOLLOW 集；
5. 根据 LR(0) 项目和 FOLLOW 集生成 SLR(1) ACTION/GOTO 表；
6. 检测移进-规约冲突和规约-规约冲突；
7. 输出完整分析表。

## 三、核心设计

### 1. ACTION 表

ACTION 表用于处理终结符输入。主要有三类动作：

- `s j`：移进到状态 `j`；
- `r k`：按照第 `k` 条产生式规约；
- `acc`：接受输入。

如果项目集中存在：

```text
A -> α · a β
```

其中 `a` 是终结符，则设置：

```text
ACTION[i, a] = shift j
```

如果项目集中存在归约项目：

```text
A -> α ·
```

则对 `FOLLOW(A)` 中每个终结符 `a` 设置：

```text
ACTION[i, a] = reduce A -> α
```

### 2. GOTO 表

GOTO 表用于处理非终结符转移。如果项目集 `Ii` 对非终结符 `A` 的 Goto 结果为 `Ij`，则设置：

```text
GOTO[i, A] = j
```

### 3. 冲突检测

如果同一个表项需要填入两个不同动作，则说明存在冲突：

- 移进-规约冲突；
- 规约-规约冲突。

程序会将冲突写入输出文件的 `CONFLICTS` 部分。

## 四、运行方式

```bash
make
./compiler slr input/grammar_expr.txt output/slr_table.txt
```

输出文件为：

```text
output/slr_table.txt
```

## 五、实验结果

部分输出如下：

```text
ACTION:
  [0, (] = s1
  [0, id] = s5
  [1, (] = s1
  [1, id] = s5
  [2, $] = acc
  [2, +] = s6
GOTO:
  [0, E] = 2
  [0, F] = 3
  [0, T] = 4
CONFLICTS: none
```

![1](E:\course\4.2\CompilerProj\compiler_project\reports\pic\proj4\1.png)



输出结果说明该表达式文法可以构造出无冲突的 SLR(1) 分析表。

## 六、实验总结

本实验完成了从 LR(0) 项目集规范族到 SLR(1) 分析表的转换。SLR(1) 相比 LR(0) 的关键改进在于使用 FOLLOW 集限制规约动作的触发范围，从而解决部分 LR(0) 中存在的冲突。该分析表为后续语义动作嵌入和语义分析提供了基础。
