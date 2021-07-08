/**
Example for decode "48656C6C6F20576F726C6421" to "Hello World!";
 */
using System;
using System.Diagnostics;
using System.Text;

namespace Example
{
    class Program
    {
        public static string Decode(string hex)
        {
            var sb = new StringBuilder();

            try
            {
                for (var i = 0; i < hex.Length; i += 2)
                {
                    var hs = hex.Substring(i, 2);
                    sb.Append(Convert.ToChar(Convert.ToUInt32(hs, 16)));
                }
            }
            catch (Exception e)
            {
                throw new FormatException(e.Message);
            }
            
            return sb.ToString();
        }

        public static string EncodeToHex(string str)
        {
            var sb = new StringBuilder();
            
            foreach (var c in Encoding.UTF8.GetBytes(str))
            {
                sb.Append(String.Format("{0:X2}", c));
            }

            return sb.ToString();
        }

        static void Main(string[] args)
        {
            var str = "Hello World!";
            var hex = "48656C6C6F20576F726C6421";

            var decoded = Decode(hex);
            Debug.Assert(decoded == str);

            var encoded = EncodeToHex(str);
            Debug.Assert(encoded == hex);

            Console.WriteLine($"Decode Hex string {hex}: {str}");
            Console.WriteLine($"Encode {str} to Hex string: {hex}");
        }
    }
}
