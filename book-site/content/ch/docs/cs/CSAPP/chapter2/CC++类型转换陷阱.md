```
weight = 50
```

# C/C++ 数据类型转换陷阱

---

考虑如下代码中, result的值是什么？

```c++
unsigned char data1 = 0b1100'0000;
unsigned char data2 = 0b1000'0000;
bool result = (data2 == (data1 << 1)); 
```





引用cppreference的一段话：

{{% hint %}} 

### Integral promotion

[prvalues](https://en.cppreference.com/w/cpp/language/value_category.html#prvalue) of small integral types (such as char) and unscoped enumeration types may be converted to prvalues of larger integral types (such as int). In particular, [arithmetic operators](https://en.cppreference.com/w/cpp/language/operator_arithmetic.html) do not accept types smaller than int as arguments, and integral promotions are automatically applied after lvalue-to-rvalue conversion, if applicable. This conversion always preserves the value.

....

- if the [integer conversion rank](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions.html#Integer_conversion_rank) of `T` is lower than the rank of int:
  - val can be converted to a prvalue of type int if int can represent all the values of `T`;
  - otherwise, val can be converted to a prvalue of type unsigned int.

....

 {{% /hint %}}

根据定义， `<<` 是算术运算符， 因此 `(data1 << 1)` 的结果会提升为int类型。 而 `data2 == (data1 << 1)`中的`data2` 和后者类型不同，因此会根据 [Usual arithmetic conversions](https://en.cppreference.com/w/cpp/language/usual_arithmetic_conversions.html#Integer_conversion_rank) 提升为 `int`。 是否会扩展符号位？ 暂时不知道。



再分析这个：



```c++
  unsigned char data1 = 0b1000'0000;
  signed char data2 = 0b1000'0000;
  bool result = (data2 == (data1));
```

