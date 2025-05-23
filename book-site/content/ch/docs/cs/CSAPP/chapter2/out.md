

状态最小化算法一些思考（来自《Elements of the theroy of the computation》 P62）：

在考虑问题前，先考虑一些问题。首先， {{< katex >}} \equiv_n{{< /katex >}} 是什么： 这是一个**等价关系**， {{< katex >}} \equiv_n \subseteq K \times K{{< /katex >}}，其中每个元素的两个状态就是对于任意同一个长度小于等于n的串**要么接受，要么不接受**。

算法第 {{< katex >}}k{{< /katex >}} 次迭代完成，维护了等价关系：{{< katex >}} \equiv_{k}{{< /katex >}}。

{{< katex >}}\forall{p \in K} ~ \forall q \in K(p \equiv_{100}q \iff p \equiv_{99}q){{< /katex >}}  意味着什么：**这两个等价关系的元素完全相同**。

算法在进行第 {{< katex >}}k{{< /katex >}} 次迭代时候，会进行如下操作：**对 {{< katex >}}\equiv_{k}{{< /katex >}} 中每一个 {{< katex >}}(p,q){{< /katex >}} ，检测 {{< katex >}}\forall{a \in \Sigma}((\delta(p,a),\delta(q,a))\in \equiv_{k-1}){{< /katex >}} , 根据定理2.5.1， 如果在这个关系中，那么说明 {{< katex >}}p\equiv_{k}q{{< /katex >}}**。

如何通过 {{< katex >}}\forall{p \in K} ~ \forall q \in K(p \equiv_{n-1}q \iff p \equiv_{n}q){{< /katex >}}  推导出  {{< katex >}} \forall{p \in K} ~ \forall q \in K(p \equiv_{n+1}q \iff p \equiv_{n}q){{< /katex >}} ，可以通过**两个角度**来思考这个问题：

## 一、 直觉

如果 {{< katex >}}\forall{p \in K} ~ \forall q \in K(p \equiv_{n-1}q \iff p \equiv_{n}q){{< /katex >}} 成立， 说明{{< katex >}}\equiv_{n-1}~= ~\equiv_{n}{{< /katex >}}。

 考虑 {{< katex >}}n=100{{< /katex >}} 成立时，即此时要进行第 101 次迭代，会对 {{< katex >}}\equiv_{100}{{< /katex >}} 中每一个 {{< katex >}}(p,q){{< /katex >}} 检测{{< katex >}}\forall{a \in \Sigma}((\delta(p,a),\delta(q,a))\in \equiv_{100}){{< /katex >}}，而由于{{< katex >}}\equiv_{100}~= ~\equiv_{99}{{< /katex >}}，这相当于重复进行了一次 {{< katex >}}n=99{{< /katex >}} 时的迭代，因为要检测的等价关系是完全一样的。因此可以保证{{< katex >}}\equiv_{101}~= ~\equiv_{100}{{< /katex >}}。

## 二、从代数的角度考虑

为什么：如果 {{< katex >}}\forall{p \in K} ~ \forall q \in K(p \equiv_{n-1}q \iff p \equiv_{n}q){{< /katex >}} 成立，为什么{{< katex >}}\forall{a \in \Sigma}((\delta(p,a),\delta(q,a))\in \equiv_{n}){{< /katex >}}一定成立？

考虑要进行第n+1次迭代时，根据定理2.5.1的充分性，可以知道{{< katex >}}\forall{a \in \Sigma}((\delta(p,a),\delta(q,a))\in \equiv_{n-1}){{< /katex >}} ，由于 {{< katex >}}\forall{p \in K} ~ \forall q \in K(p \equiv_{n-1}q \iff p \equiv_{n}q){{< /katex >}} 成立，即 {{< katex >}}\equiv_{n-1}~= ~\equiv_{n}{{< /katex >}}， 所以{{< katex >}}\forall{a \in \Sigma}((\delta(p,a),\delta(q,a))\in \equiv_{n}){{< /katex >}}。 所以一定可以检测成功。

