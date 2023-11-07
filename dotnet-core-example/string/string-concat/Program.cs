using System.Text;
/*
PHP:
$ss = [];
$ss[] = "abc";
$ss[] = "DEF";
echo implode('', $ss), "\n";
echo implode(',', $ss), "\n";

Python:
ss = []
ss.append("abc")
ss.append("DEF")
print(''.join(ss))
print(','.join(ss))

JavaScript:
const ss = [];
ss.push("abc")
ss.push("DEF")
console.log(ss.join(''));
console.log(ss.join(','));
console.log(''.concat(ss));
*/

// StringBuilder does not implement IEnumerable interface.
var sb = new StringBuilder();
sb.Append("abc");
sb.Append("DEF");
sb.Append("xyz");

Console.WriteLine(string.Concat(sb));
// print: abcDEFxyz
// string.Concat(sb) 等同 sb.ToString()
// 慣例是用 sb.ToString()

Console.WriteLine(string.Join(",", sb)); //! 沒有 join 效果
// print: abcDEFxyz
// 雖然 StringBuilder 行為接近 list ，但它沒有實作 IEnumerable。
// 故這一行被當成 string.Join(",", sb.ToString())，沒有效果。

var ls = new List<string>();
ls.Add("abc");
ls.Add("DEF");
ls.Add("xyz");

Console.WriteLine(string.Concat(ls));
// print: abcDEFxyz
Console.WriteLine(string.Join("", ls));
// print: abcDEFxyz

Console.WriteLine(string.Join(",", ls));
// print: abc,DEF,xyz
