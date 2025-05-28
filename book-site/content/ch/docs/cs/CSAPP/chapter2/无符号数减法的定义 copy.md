---
weight: 30
---



# 补码减法

---

## 1. 定义补码减法的思路

---

在定义一种新运算时，首先要考虑如下几个问题：

1. 是否会出现未被编码的结果，例如在无符号数减法中，结果为负数。
2. 是否会出现溢出，溢出的条件是什么，溢出时如何处理？

在做w位补码减法时，所有结果都被编码了，但是w位无法容纳发生溢出时的结果：一个正数减去一个负数的结果可能大于  {{< katex >}}2^{w-1} - 1{{< /katex >}} ，此时与补码加法正溢出类似；同样，一个负数减去一个正数可能会小于  {{< katex >}}-2^{w-1}{{< /katex >}}，这时称减法发生了负溢出。 

{{% hint def%}} 对于 {{< katex >}}w{{< /katex >}} 位补码 {{< katex >}}x,y{{< /katex >}}，定义补码减法为：



{{< katex >}} \\x-_w^t y =:   \begin{cases}x - y - 2^w,                   & x - y  \ge 2^{w-1}\\   x-y, & -2^w < x-y < 2^{w-1} \\x  - y  + 2^w,       & x-y  \le -2^{w-1} -1  \end{cases} {{< /katex >}}



 {{% /hint %}}

## 2.使用加法逆元定义补码减法

---

为了方便硬件实现减法，此处给出一种使用补码加法实现补码减法的定义。

{{% details "w位无符号数加法逆元" %}}

{{< katex >}}\forall x \in [-2^{w-1},2^{w-1}-1]{{< /katex >}} ，其 {{< katex >}}w{{< /katex >}} 位的补码加法逆元 {{< katex >}}-_w^t x{{< /katex >}} 由下式给出：

{{<katex display=true >}} -_w^t x =
 \begin{cases}
 x,             & x = -2^{w-1} \\
-x             & x > -2^{w-1} 
 \end{cases}
 {{< /katex >}}

{{% /details %}}

{{% hint def%}} 对于 {{< katex >}}w{{< /katex >}} 位补码 {{< katex >}}x,y{{< /katex >}}，定义{{< katex >}}x-_w^t y =: x+_w^t (-_w^t y){{< /katex >}} {{% /hint %}}

将其代入补码加法公式，可以得到

{{<katex display=true >}}

x +_w^t (-_w^t y) =
\begin{cases}
x + (-_w^t y) - 2^w, & \quad 2^{w-1} \le x + (-_w^t y) \\
x + (-_w^t y), & \quad -2^{w-1} \le x + (-_w^t y) < 2^{w-1} \\
x + (-_w^t y) + 2^w, & \quad x + (-_w^t y) < -2^{w-1}
\end{cases}

{{< /katex >}}



