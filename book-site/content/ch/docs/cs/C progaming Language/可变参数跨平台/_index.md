---
weight: 1
title: "可变参数跨的平台"
---

111

# 0.问题引入

　　在做一生一芯的过程中，需要用到 abstract-machine ，其中需要实现可以跨平台的 `vspinrtf` ，这需要用到头文件 `<stdarg.h>`， 以及声明在这里面的一些宏以及函数。可以通过 `man 3 va_start` 查看他们的手册。

　　我的实现大概如下：

```C
staitc int handle_fmt(char ** pout, char ** pp_fmt, va_list sp) {
  ...
 switch(**pp_fmt) {
  case 's':
         ...
  case 'd':
         ...
         int num = va_arg(sp, int);
         ...
 }
  ...
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  ...
  handle_fmt(&out, &p_fmt, ap);
  ...
}

int sprintf(char *out, const char *fmt, ...) {
    va_list vlist;
    va_start(vlist, fmt);
    int r = vsprintf(out, fmt,vlist);
    va_end(vlist);
}
```

　　对于上面这种实现，在x86 linux环境下可以正常运行，而在riscv32-nemu 和 riscv32 linux环境下，`sprintf(buf, "%d %d", 1, 3);` 会将` "1 1"` 复制到 `buf`。并且：

- 在 x86-linux 可以通过各种测试用例，而在 riscv32-nemu 无法通过；
-  riscv32-nemu 与 spike 进行差分测试无报错。



　　对于编译*linux x86* 架构程序，可以通过给 *x86_64-linux-gnu-gcc* 传递 `-m32` 和 `-m64` (System V AMD64 ABI) 选择相应的 *ABI*， 而对于 *riscv* ，则可以传递 `-mabi=ilp32/lp64/...` 来选择程序遵循的 *ABI*；此外，都可以通过 `-march` 来选择支持的扩展指令集。例如，`-march=rv64im` 表明支持乘法扩展的 64 位指令集。

# 1. 阅读手册

---

　　阅读 C99 标准手册 `7.15 Variable arguments` 部分，发现上述宏都是  *implementation-defined* 的， 因此可以查询相应的 *ABI* 手册。

##1.1 RISC-V ABI 对可变参数传参的描述

---

这里只是对ABI手册做一个个简单的概述，具体可参考 [RISCV ABI](https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-abi.adoc) . 

- 可变参数优先通过寄存器传递：
  - 该参数对齐到 *2 × XLEN* 且该参数大小最大为 *2 × XLEN* (例如，XLEN = 32， 那么1，2，4，8字节的可变参数都可以这种方式传递)，此时还需要找到一个寄存器 $a_i$ ，其中 $i$ 为偶数， 在 $a_i$​ 和 $a_{i+1} 中存放这个参数。
  - 遇到第一个需要放到栈上的可变参数后，后面的可变参数都需要放到栈上。
- *va_list* 类型上是一个 `void *` 。
- 无论可变参数是否通过栈传递，最终都要放到栈上，并且由一个 `va_list` 类型的指针指向其第一个可变参数。 
- `va_arg` 取出一个可变参数，然后自增 `va_list` .

## 1.2 System V AMD64 ABI

---

　　首先引入一个 *System V AMD64 ABI* 对函数传递的参数分类中的一个类型, **INTEGER** 类型 : *This class consists of integral types that fit into one of the general purpose registers.*

阅读 *System V AMD64 ABI* 手册 **3.5.7 Variable Argument Lists** , 重点关注该类型的参数在可变的情况下是如何传递的：

- 依旧优先使用 %RDI, %RSI, %RDX, %RCX, %R8, %R9 来传递，这 6 个寄存器用完了再放在栈上(这里和普通参数规则一样)；
- 在确定函数具有可变参数列表且调用 `va_start` 的情况下，**一定会在这个函数 *prolgue* 部分将寄存器中的参数复制到在 *register save area* 内存区域**，对于这6个寄存器来说，其位置有固定的从 *register save area* 开始的偏移 *%rdi 0，%rsi 8,...,%r9 40*

- `va_list` 类型是只有一个元素的结构体数组：
  - `reg_save_area` 指向 *reg_save_area* 区域；
  - `reg_save_area` 初始时指向第一个由栈传递的参数（不包括寄存器保存在 *register save area* 的），每取一个由栈传递参数，自增该指针。
  - `gp_offset` 以字节为单位，从 *register save area*  到下一个要取的存放在 *register save area*  区域的寄存器参数的距离。假设 6 个通用目的寄存器中的参数都放在了该区域，且都被取完时，该值为 `48`。

```c
typedef struct {
   unsigned int gp_offset;
   unsigned int fp_offset;
   void *overflow_arg_area;
   void *reg_save_area;
} va_list[1];
```

　　调用 `va_args` 取参数的算法大致先用 `gp_offset` 判断是否还能在  *register save area*  区域取到参数，如果可以，就在该区域取，取完一次后将 `gp_offset += 8` . 如果该区域已经没有可以取的参数了，那么就通过 `overflow_arg_area`指针取出参数。

### 

# 2. 问题的分析与解决

---

　　现在原因已经很明显了，在 *RISCV ABI* 定义下， `vsprintf()` 向 `handle_fmt()` 传递了下一个要取的参数的地址，对于 `handle_fmt()`来说，这个地址就是一个局部变量，因此 `va_args()` 对其的改变并不会反映在 `vsprintf()`中。 这意味着，无论有多少个可变参数， `va_list` 永远都指向第一个可变参数。

　　这可以通过一个简单的方法去验证，调用 `sprintf(buf, "%d %d", 1, 3)` ，去检查 `buf` 区域的前三个字节，会发现依次为 `0x31` `0x20` `0x31`。 当然，这要保证函数 `handle_fmt()` 不被内联，在内联的情况下函数的行为也不会正确。

　　可以通过给 *gcc* 传递`-fopt-info-optimized-inline=inline.log ` ，将内联日志保存下来，去查看是否内联。也可以通过 *objdump* 去验证。

要想解决这个问题，只需要**将 `va_list ap`值传递改为引用传递**。

