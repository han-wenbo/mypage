# 一个DPI-C bug

---

先看一段 Verilog 模拟处理器在内存(0个周期读延迟)中读取指令的代码：

```verilog
import "DPI-C" pure function int  dpi_pmem_read (input int raddr, input logic reset);

module ImemDpi(
  input              reset,
  input       [31:0] addr,
  output reg  [31:0] inst
);


 always @(reset or addr) begin
    if (reset == 1'b0) begin
      //$display(" Fetch inst at %h", addr);
      inst = dpi_pmem_read(addr,reset);
     end
     else begin
      inst = 32'h0000_0013;
     end
  end

endmodule      
```

正确的行为上是应该当 `reset == 1` 的时候不执行 `dpi_pmem_read(addr,reset)` ，但是对于上述代码，无论 `reset` 取 `0` 还是 `1` 都会调用这个函数。主要的原因是在第一行中加了上 `pure` 关键词，这会让 verilog 认为这个函数无任何副作用，从而**大概率会生成一个选择器，以 `reset` 为选择信号，去选择  `dpi_pmem_read(addr,reset)` 和 `32'h0000_0013`** ，而将这个关键词改为 `context`则可以让 verilog 知道， 这个函数有副作用，不应该当 `reset == 1` 的时候执行。

此外，这个bug的有这样的意义：**不要用写代码的思路去写电路，比如这里verilog中的 `if-else` 逻辑，两个分支的组合逻辑都会计算一遍. **
