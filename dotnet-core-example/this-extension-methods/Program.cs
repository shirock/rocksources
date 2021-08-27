// C# Extension Methods (擴充方法)
// https://docs.microsoft.com/zh-tw/dotnet/csharp/programming-guide/classes-and-structs/extension-methods
using System;

namespace StringExt
{
    public static class MyString
    {
        //                          VVVV
        public static string concat(this string src1, string src2)
        {
            return src1 + src2;
        }
    }
}

namespace Example
{
    using StringExt;

    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello ".concat("World!"));
            // C# 內建 string 類別並沒有 concat 這個方法。我從外面(MyString)掛一個上去。

            // 如果第一個引數宣告時沒有加上 this 修飾字。那呼叫時只能這樣寫:
            Console.WriteLine(MyString.concat("Hello ", "World!"));
        }
    }
}
