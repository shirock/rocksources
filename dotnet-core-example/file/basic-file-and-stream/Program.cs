using System;
using System.IO;

/**
See also: 
+ https://docs.microsoft.com/zh-tw/dotnet/csharp/programming-guide/file-system/how-to-read-a-text-file-one-line-at-a-time
+ https://docs.microsoft.com/zh-tw/dotnet/api/system.io.filestream
 */
namespace Examle
{
    class Program
    {
        static void Main(string[] args)
        {
            var filepath = @"lines.txt";
            var filepath2 = @"lines2.txt";

            var lines = new []
                {
                    "line1",
                    "line2",
                    "line3"
                };

            // Write text file
            using (var fw = new StreamWriter(filepath))
            {
                foreach (var line in lines)
                {
                    fw.WriteLine(line);
                }
            }

            // Read text file
            string text;
            using (var fr = new StreamReader(filepath))
            {
                while ((text = fr.ReadLine()) != null)
                {
                    Console.WriteLine(text);
                }
            }

            Console.WriteLine("==== file get contents ====");
            text = File.ReadAllText(filepath);
            Console.Write(text);

            Console.WriteLine("==== file put contents ====");
            File.WriteAllText(filepath2, "");
            Console.Write(text);

            File.Delete(filepath);
        }
    }
}
