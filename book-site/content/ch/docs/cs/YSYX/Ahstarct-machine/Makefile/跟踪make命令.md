# 跟踪make命令

## 从make ARCH=riscv32-nemu run开始

首先，进入`ysyx-workbench/am-kernels/kernels/hello`目录，运行`make ARCH=riscv32-nemu run -nB` 命令。其中

- `ARCH=riscv32-nemu` 定义了一个变量，在 Makefile 中即便没有 `ARCH = ...`这样的定义，也可以直接通过`$(ARCH)`使用这个变量。
- `run` 意味着会执行名为 `run`的规则。
- `-n`表明要只输出执行的规则，而不真正的执行。
- `-B` 不论新旧，强制执行所有依赖的规则。

将该命令的结果输出到vim中，简单浏览一遍，发现有如下规律：

- 有的地方执行了 `make` 命令，因此可以暂时先用 `:%!grep "^make"` 过滤出来简单查看。 但是发现有形如 `make[1]`的make输出，因此需要只保留make作为命令执行的行。
- 以 `gcc`,`g++`以及`riscv` 开头的命令大概率是编译、链接的命令。
- 在最底下，能看到 nemu 被执行的命令，这一行可以先不保留。

因此，可以通过思路来清洗输出：保留上述列出的三项内容的行，筛选掉其他的行，可命令 `%!grep -E "^make *-|^riscv|gcc|g\+\+"` 实现，加 `-E` 是因为 grep 默认不支持或运算符。执行完该命令后，可以发现，编译一些单独的`.c`源文件就有大量重复的内容：

```
-std=gnu11 -O2 -MMD -Wall -Werror -I/home/ubuntu/ysyx-workbench/abstract-machine/am/src -I/home/ubuntu/ysyx-workbench/abstract-machine/am/include -I/home/ubuntu/ysyx-workbench/abstract-machine/am/include/ -I/home/ubuntu/ysyx-workb  ench/abstract-machine/klib/include/ -D__ISA__=\"riscv32\" -D__ISA_RISCV32__ -D__ARCH__=riscv32-nemu -D__ARCH_RISCV32_NEMU -D__PLATFORM__=nemu -D__PLATFORM_NEMU -DARCH_H=\"arch/riscv.h\" -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector 
  -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden -fno-pic -march=rv64g -mcmodel=medany -mstrict-align -march=rv32im_zicsr -mabi=ilp32    -static -fdata-sections -ffunction-sections -I/home/ubuntu/ysyx-workbench/abstract-machine/am/src/platform/nemu/incl  ude -DMAINARGS_MAX_LEN=64 -DMAINARGS_PLACEHOLDER=\""The insert-arg rule in Makefile will insert mainargs here."\" -DISA_H=\"riscv/riscv.h\"
```

可以简单的用`:%s/foo/bar/g`替换掉，这表示在整个文件中（`%`），把所有匹配 `foo` 的内容替换为 `bar`，`g` 表示每一行中所有匹配都替换。可以用正则表达式中`.`,`*`两个元字符，句号代表匹配任意字符一次，而星号表示匹配前一个字符0次或任意多次。

此时还会发现有很多  `gcc ... -E ...`的行，这仅仅进行了预编译处理，可以删掉这些行。可以使用`:g/{pattern}/[command]`,该命令对**所有匹配 `{pattern}` 的行**，执行指定的 `[command]`。 command用`d` 可以删掉匹配到的行。

最后，再用`%s/ -/\r   -/g`将每个以 `-`开头的参数换行。

现在可以看到，make执行的命令主要分为两部分：

1. 编译hello所需的源文件，然后链接在一起，这用了一个额外的链接脚本。
2. 编译并链接nemu。

这两部分通过如下这条make命令连接：

```
make -C /home/ubuntu/ysyx-workbench/nemu 
	ISA=riscv32 
	run 
	ARGS="-l /home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/nemu-log.txt" 
	IMG=/home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/hello-riscv32-nemu.bin
```

