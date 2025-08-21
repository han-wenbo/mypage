# 1. 配置菜单

查看 `$NEMU_HOME/Kconfig`文件，检查 *TRACE* 相关宏：

```c
config TRACE
  bool "Enable tracer"
  default y

...

config ITRACE
  depends on TRACE && TARGET_NATIVE_ELF && ENGINE_INTERPRETER
  bool "Enable instruction tracer"
  default y

config ITRACE_COND
  depends on ITRACE
  string "Only trace instructions when the condition is true"
  default "true"
```

在配置菜单了相关选项后，会在 `$NEMU_HOME/.config` 中生成相应的配置。例如, 在配置菜单中` [y] TRACE`，就会在该文件中生成 `CONFIG_ITRACE=y` ，最终会生成一个`$NEME_HOME/include/generated/autoconf.h` 头文件，其中包括 `#define CONFIG_TRACE y`，而这个头文件在 `common.h` 中被引用。



# 2.关于TRACE的宏依赖

---

如果要通过配置菜单来配置 *TRACE* 相关的宏，会存在一个依赖链：

```
CONFIG_TRACE            CONFIG_TARGET_NATIVE_ELF            CONFIG_ENGINE_INTERPRETER
      \                         |                                   /
       \                        |                                  /
        +-----------------------+---------------------------------+
                                |
                          +---------------+
                          | CONFIG_ITRACE |
                          +---------------+
                                |
                       +-------------------+
                       | CONFIG_ITRACE_COND |
                        +-------------------+
                                |
                           +-------------+
                           | ITRACE_COND |
                           +-------------+

```

前三行是通过配置菜单生成的，而最后一行是通过 `$NEMU_HOME/Makefile` 生成的，具体如下：

```makefile
CFLAGS_TRACE += -DITRACE_COND= $(if $(CONFIG_ITRACE_COND),$(call remove_quote,$(CONFIG_ITRACE_COND)),true)
```

意思是，如果定义了`CONFIG_ITRACE_COND` 变量，则增加一个宏定义，其值是`CONFIG_ITRACE_COND`的值去掉引号。例如，`$(CONFIG_ITRACE_COND)="ture"`

那么相当于引入了如下宏定义：`#define ITRACE_COND ture`



# 3. CONFIG_ITRACE

---

该宏定义只会影响三件事：

- 每条指令执行完后，对该执行反汇编，写入 `logbug[128]`

- 启用 `logbug[128]`
- 将执行的指令的反汇编输出到屏幕上

## 3.1 

---

```c
typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode
```



## 3.2

---

```c
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
    
    
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst;
#ifdef CONFIG_ISA_x86
  for (i = 0; i < ilen; i ++) {
#else
  for (i = ilen - 1; i >= 0; i --) {
#endif
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
#endif
      
}
```



## 3.3 

---



```c
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
  ...
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  ...

}
```



# 4 CONFIG_ITRACE_COND



```c
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
...
}
```

- 定义 `CONFIG_ITRACE_COND` 后不一定保存日志，必须要将 `CONFIG_ITRACE_COND` 定义为 `ture` 才会将执行记录输出日志文件中。

