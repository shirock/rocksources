using System;

namespace Example
{
    public class MyClass
    {
        public delegate void PrintMethod(string s);

        // 定義一個可委派方法的成員。預設委派 Console.WriteLine
        public static PrintMethod Print = Console.WriteLine;

        public static void Method1()
        {
            Print("method1 called.");
        }

        // 宣告一個要委派方法的參數。
        public static void Method2(PrintMethod print)
        {
            print("method2 called.");
        }
    }
}