def replace_dollar_with_katex(input_file, output_file):
    with open(input_file, 'r', encoding='utf-8') as f:
        content = f.read()

    result = []
    open_katex = True
    i = 0
    while i < len(content):
        if content[i] == '$':
            if open_katex:
                result.append('{{< katex >}}')
            else:
                result.append('{{< /katex >}}')
            open_katex = not open_katex
        else:
            result.append(content[i])
        i += 1

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(''.join(result))

# 用法示例
replace_dollar_with_katex('t.md', 'out.md')

