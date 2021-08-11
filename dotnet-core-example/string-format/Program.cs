using System;
/*
.NET Core C# example: 
string format: fixed width number and padding with '0'.
 */
namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            int n = 123;
            
            // string.Format
            Console.WriteLine(string.Format("固定6位寬度數字:\n前置0   :{0,6:D6}\n前置空白:{0,6}", n));
            
            // interpolation 
            Console.WriteLine($"固定6位寬度數字:\n前置0   :{n,6:D6}\n前置空白:{n,6}");

            Console.WriteLine(String.Format("16進位表示文字，前置0:\n{0:x2}, {1:x2}", n, 10));

            float fv = 12.34f;
            Console.WriteLine(String.Format("固定3.1位寬度浮點數，前置0:\n{0,5:000.0}", fv));
            Console.WriteLine($"固定3.1位寬度浮點數:\n{fv,5:.0}");
        }
    }
}
