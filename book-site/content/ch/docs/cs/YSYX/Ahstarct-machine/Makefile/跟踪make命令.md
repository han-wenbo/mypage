# 跟踪make命令

## 1.从make ARCH=riscv32-nemu run开始

首先，进入`ysyx-workbench/am-kernels/kernels/hello`目录，运行`make ARCH=riscv32-nemu run -nB` 命令。其中

- `ARCH=riscv32-nemu` 定义了一个变量，在 Makefile 中即便没有 `ARCH = ...`这样的定义，也可以直接通过`$(ARCH)`使用这个变量。
- `run` 意味着会执行名为 `run`的规则。
- `-n`表明要只输出执行的规则，而不真正的执行。
- `-B` 不论新旧，强制执行所有依赖的规则。

将该命令的结果输出到vim中，简单浏览一遍，发现有如下规律：

- 有的地方执行了 `make` 命令，因此可以暂时先用 `:%!grep "^make"` 过滤出来简单查看。 但是发现有形如 `make[1]`的make输出，因此需要只保留make作为命令执行的行。
- 以 `gcc`,`g++`以及`riscv` 开头的命令大概率是编译、链接的命令。
- 在最底下，能看到 nemu 被执行的命令，这一行可以先不保留。

因此，可以通过这样的思路来清洗输出：保留上述列出的三项内容的行，筛选掉其他的行，可命令 `%!grep -E "^make *-|^riscv|gcc|g\+\+"` 实现，加 `-E` 是因为 grep 默认不支持或运算符。执行完该命令后，可以发现，编译一些单独的`.c`源文件就有大量重复的内容：

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

这两部分通过如下这条make命令连接，这是调用nemu的Makefile，以刚刚完成编译的hello作为参数，运行nemu的命令：

```
make -C /home/ubuntu/ysyx-workbench/nemu 
	ISA=riscv32 
	run 
	ARGS="-l /home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/nemu-log.txt" 
	IMG=/home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/hello-riscv32-nemu.bin
```

以及最后一行的命令输出：

```
/home/ubuntu/ysyx-workbench/nemu/build/riscv32-nemu-interpreter
-l /home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/nemu-log.txt  /home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/hello-riscv32-nemu.bin
```

## 2.  分析hello是如何被编译、链接的

---

对于linux下的C程序，它的入口点是 `_start` , 这个符号（Symbol）是由 `ctr1.o` 提供的, 并且在链接脚本里通过`ENTRY(_start)` 设置了程序入口点，这个关键词会让链接器将elf头中的程序入口点字段设置为 `_start`的地址。 

对于命令 `gcc a.c` 来说，它会自用调用链接器，而当我们手动调用链接器链接目标文件时候，需要我们手动提供 `ctri.o` 。当然， gcc 实际上也是调用 ld，并提供了相应的参数，用如下命令可以追踪 gcc 是如何调用 ld的：

```
strace -f -s 10000 -e execve gcc a.c 2>&1 | grep /ld
```

`strace`用于追踪系统调用， `-e  execve ` 指明了只追踪 `execve`， `-f` 表明要追踪 gcc 创建的子进程的系统调用。

执行该命令，并检查`execve`的第二个参数，可以看到如下内容：

```
/usr/bin/ld
  -plugin /usr/libexec/gcc/x86_64-linux-gnu/13/liblto_plugin.so
....
  --build-id
  --eh-frame-hdr
  -m elf_x86_64
  --hash-style=gnu
  --as-needed
  -dynamic-linker /lib64/ld-linux-x86-64.so.2
  -pie
  -z now
  -z relro
  -o a.out

  /usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/Scrt1.o
  /usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crti.o
  /usr/lib/gcc/x86_64-linux-gnu/13/crtbeginS.o

  -L/usr/lib/gcc/x86_64-linux-gnu/13
  -L/usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu
  -L/usr/lib/gcc/x86_64-linux-gnu/13/../../../../lib
  -L/lib/x86_64-linux-gnu
  -L/lib/../lib
  -L/usr/lib/x86_64-linux-gnu
  -L/usr/lib/../lib
  -L/usr/lib/gcc/x86_64-linux-gnu/13/../../..

  /tmp/ccszOHmP.o
...
```

可以看到里面就有 ` /usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crti.o`.

此外，需要值得注意的是，`ENTRY(_start)` 不会告诉链接器要将 _start 部分的代码放到可执行文件的哪里，也不会告诉链接器如何设置 _strart部分代码的运行时地址。

---

## 3. am 链接自己的 _start

   首先分析一下链接 hello的参数：

```
  riscv64-linux-gnu-ld
     -z noexecstack
     -T$(A)/scripts/linker.ld
     -melf64lriscv
     --defsym=_pmem_start=0x80000000
     --defsym=_entry_offset=0x0
     --gc-sections
     -melf32lriscv
     -o /home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/hello-riscv32-nemu.elf
     --start-group /home/ubuntu/ysyx-workbench/am-kernels/kernels/hello/build/riscv32-nemu/hello.o $(A)/am/build/am-riscv32-nemu.a $(A)/klib/build/klib-riscv-32-nemu.a
     --end-group
```

重点关注，` --start-group...--end-group` 部分，这是链接器的输入，并且可以打乱输入文件的顺序也能找到相应的符号。 可以看到`hello.o` 和两个`.a` 文件链接在了一起。`.a` 一般作为linux静态链接库后缀，一个静态链接库本质上就是一组可重定位目标文件的集合，可以通过 `ar`将一组可重定位目标文件打包为一个静态链接库。

通过分析刚才make的输出，可以清楚的看到这两个静态链接库是如何被打包的：

```
  riscv64-linux-gnu-ar rcs $(A)/am/build/am-riscv32-nemu.a 
  $(A)/am/build/riscv32-nemu/src/platform/nemu/trm.o 
  $(A)/am/build/riscv32-nemu/src/platform/nemu/ioe/ioe.o      
  $(A)/am/build/riscv32-nemu/src/platform/nemu/ioe/timer.o 
  $(A)/am/build/riscv32-nemu/src/platform/nemu/ioe/input.o 
  $(A)/am/build/riscv32-nemu/src/pl  atform/nemu/ioe/gpu.o 
  $(A)/am/build/riscv32-nemu/src/platform/nemu/ioe/audio.o
  $(A)/am/build/riscv32-nemu/src/platform/nemu/ioe/disk.o 
  $(A)/am/build/riscv32-nemu/src/platform/nemu/mpe.o 
  $(A)/am/build/riscv32-nemu/src/riscv/nemu/start.o 
  $(A)/am/build/riscv32-nemu/src/riscv/nemu/cte.o 
  $(A)/am/build/riscv32-nemu/src/riscv/nemu/trap.o 
  $(A)/am/build/riscv32-nemu/src/riscv/nemu/vme.o
  
 riscv64-linux-gnu-ar rcs
$(A)/klib/build/klib-riscv32-nemu.a 
$(A)/klib/build/riscv32-nemu/src/stdio.o $(A)/klib/build/riscv32-nemu/src/int64.o
$(A)/klib/build/riscv32-nemu/src/stdlib.o $(A)/klib/build/riscv32-nemu/src/cpp.o 
$(A)/klib/build/riscv32-nemu/src/string.o
```

而riscv32-nemu的 `_start` 就定义在 `$(A)/am/build/riscv32-nemu/src/riscv/nemu/start.o `中，可以根据 make 的输出定位到在`$(A)/am/src/riscv/nemu/start.S` 中。其内容如下：

```
.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mv s0, zero
  la sp, _stack_pointer
  call _trm_init

```

现在再看一下链接脚本：

```c
ENTRY(_start)
PHDRS { text PT_LOAD; data PT_LOAD; }

SECTIONS {
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  . = _pmem_start + _entry_offset;
  .text : {
    *(entry)
    *(.text*)
  } : text
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data)
  } : data
  edata = .;
  _data = .;
  .bss : {
        _bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
}
      
```

首先，`ENTRY(_start)` 指定了程序入口点。 然后, `  . = _pmem_start + _entry_offset;` 设置了后续内容的开始的 VMA。

下一条一句指定了如何生成 elf 文件的 `.text` 段，`*` 是通配符，匹配任意字符，放在 section 名字前，代表取所有输入文件的某个 section：

- ` *(entry)` 取所有输入文件的 `entry` section，放在 elf 的`.text`section的开头，

- ` *(.text*)` 取所有输入文件中以 .text 开头的section，放在上述 section 的后面。

