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

                Console.Write($"{Convert.ToChar(n)} ");

                if (t >= 16) {
                    Console.WriteLine();
                    t = 0;
                }
            }
            Console.WriteLine();

            var s1 = "CJK中日韓文";
            var bs1 = Encoding.UTF8.GetBytes(s1);
            Console.WriteLine($"string \"{s1}\" encode to byte[]:");
            // foreach (var b in bs1) {
            for (var i = 0; i < bs1.Length; ++i) {
                var b = bs1[i];
                Console.WriteLine(String.Format("byte[{0}]: {1,3} (0x{1:x}) ({2})", i, b, Convert.ToChar(b)));
            }

            var bs2 = new byte[] {0xe4, 0xb8, 0xad, 0xe6, 0x96, 0x87};
            var s2 = Encoding.UTF8.GetString(bs2);
            Console.WriteLine($"byte[] decode to string: {s2}");
        }
    }
}
