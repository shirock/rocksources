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
            Console.WriteLine(String.Format("固定6位寬度數字，前置0:\n{0,6:D6}", n));
            Console.WriteLine(String.Format("固定6位寬度數字，前置空白:\n{0,6}", n));

            float fv = 12.34f;
            Console.WriteLine(String.Format("固定3.1位寬度浮點數，前置0:\n{0,5:000.0}", fv));
            Console.WriteLine(String.Format("固定3.1位寬度浮點數:\n{0,5:.0}", fv));
        }
    }
}
