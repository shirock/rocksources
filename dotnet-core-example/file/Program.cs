using System;
using System.Collections.Generic;

/**
See also: 
+ https://docs.microsoft.com/zh-tw/dotnet/csharp/programming-guide/file-system/how-to-read-a-text-file-one-line-at-a-time
+ https://docs.microsoft.com/zh-tw/dotnet/api/system.io.filestream
 */
namespace file
{
    class Program
    {
        static void Main(string[] args)
        {
            var filepath = @"lines.txt";
            var filepath2 = @"lines2.txt";

            var lines = new List<string>();
            lines.Add("line1");
            lines.Add("line2");
            lines.Add("line3");

            // Write text file
            var file_sw = new System.IO.StreamWriter(filepath);
            foreach (var line in lines)
            {
                file_sw.WriteLine(line);
            }
            file_sw.Close();

            // Read text file. Write binary file.
            byte[] data;
            string text;
            var file_sr = new System.IO.StreamReader(filepath);
            // var file_bw = new System.IO.FileStream(filepath2, System.IO.FileMode.Create);
            var file_bw = System.IO.File.Create(filepath2);
            while ((text = file_sr.ReadLine()) != null)
            {
                data = System.Text.Encoding.UTF8.GetBytes(text + "\n");
                file_bw.Write(data, 0, data.Length);
            }
            file_sr.Close();
            file_bw.Close();

            Console.WriteLine("==== file get contents ====");
            text = System.IO.File.ReadAllText(filepath);
            Console.Write(text);
            Console.WriteLine("====");

            System.IO.File.Delete(filepath);
            System.IO.File.Delete(filepath2);
        }
    }
}
