using System;
using System.Text;
/*
.NET Core C# example: byte[], char, string convert
 */
namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("\nASCII TABLE");
            Console.WriteLine("    0 1 2 3 4 5 6 7 8 9 a b c d e f");
            Console.WriteLine("--+--------------------------------");
            for (byte n = 32, t = 0; n < 127; ++n) {
                if (t == 0) {
                    Console.Write($"{n:x2}| ");
                }
                ++t;

                Console.Write(String.Format("{0} ",Convert.ToChar(n)));

                if (t >= 16) {
                    Console.WriteLine();
                    t = 0;
                }
            }
            Console.WriteLine();

            var s1 = "hello";
            var bs1 = Encoding.ASCII.GetBytes(s1);
            var s1r = Encoding.ASCII.GetString(bs1);
            Console.WriteLine($"string \"{s1}\" encode to byte[]:");
            foreach (var b in bs1) {
                Console.WriteLine(String.Format("byte {0,3} = 0x{1:x} = {2}", b, b, Convert.ToChar(b)));
            }
            Console.WriteLine(String.Format("byte[] decode to string: {0}", s1r));

            var s2 = "中日韓文CJK";
            var bs2 = Encoding.UTF8.GetBytes(s2);
            var s2r = Encoding.UTF8.GetString(bs2);
            Console.WriteLine($"string \"{s2}\" encode to byte[]:");
            foreach (var b in bs2) {
                Console.WriteLine(String.Format("byte {0,3} = 0x{1:x} = {2}", b, b, Convert.ToChar(b)));
            }
            Console.WriteLine(String.Format("byte[] decode to string: {0}", s2r));

            var bs3 = new byte[] {0xe4, 0xb8, 0xad, 0xe6, 0x96, 0x87};
            var s3 = Encoding.UTF8.GetString(bs3);
            Console.WriteLine(s3);
        }
    }
}
