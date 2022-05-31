/**
  delegate 語法範例
 */
using System;

namespace Example
{
    class Program
    {

        public static void Cout(string s)
        {
            Console.WriteLine($"cout: {s}");
        }

        static void Main(string[] args)
        {
            // 使用 MyClass 預設的 Print 方法
            MyClass.Method1();

            // 用例1: 委派對象是類別或個體成員。
            // MyClass.Print 是一個可委派成員。
            // 委派 Cout() 作為 MyClass.Print 的方法。
            MyClass.Print = Cout;
            MyClass.Method1();

            // 用例2: 委派對象是方法的參數。
            // MyClass.Method2() 方法的 print 參數必須是可呼叫的方法。

            // 將具名方法 Cout 委派給 print 參數。
            MyClass.Method2(Cout);

            // 將不具名方法(lambda)委派給 print 參數。
            MyClass.Method2(
                (s)=>{
                    Console.WriteLine($"lambda: {s}");
                }
            );
        }
    }
}
