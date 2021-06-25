// dotnet add package System.Text.Encoding.CodePages
/*
.NET Core 預設僅提供 ASCII, UTF8 文字編碼法。
其他文字編碼法移到獨立的套件 System.Text.Encoding.CodePages 。
故使用其他文字編碼法前，必須將此套件加進專案。

codepage:936,   name:gb2312
codepage:950,   name:big5
codepage:20000, name:x-Chinese-CNS, 中華民國中文標準交換碼
codepage:20002, name:x-Chinese-Eten, 倚天系統擴充碼，倚天碼
codepage:65001, name:utf-8
see: https://docs.microsoft.com/zh-tw/dotnet/api/system.text.encodinginfo.getencoding
*/
using System;
using System.IO;
using System.Text;

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            // 程式執行時，調用此方法載入其他文字編碼法。
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance); 

            // var utf8_encoding = Encoding.GetEncoding("utf-8");
            // 上行結果就是 Encoding.UTF8
            var big5_encoding = Encoding.GetEncoding("big5");
            var eten_encoding = Encoding.GetEncoding("x-Chinese-Eten");
            var gb2312_encoding = Encoding.GetEncoding("gb2312");

            var s = "許功蓋";
            // Console.WriteLine();
            File.WriteAllBytes("utf8.txt", Encoding.UTF8.GetBytes(s));
            File.WriteAllBytes("big5.txt", big5_encoding.GetBytes(s));
            File.WriteAllBytes("eten.txt", eten_encoding.GetBytes(s));
            File.WriteAllBytes("gb2312.txt", gb2312_encoding.GetBytes(s));

            var encoded_data = File.ReadAllBytes("big5.txt");
            var decoded_text = big5_encoding.GetString(encoded_data);
            // .NET 內部處理字串採用 Unicode 。故此處之意為 Big5 轉 Unicode 。
            // 將這些位元組資料，用 Big5 編碼法解讀成 Unicode 字串。
            Console.WriteLine(decoded_text);

            encoded_data = File.ReadAllBytes("utf8.txt");
            decoded_text = Encoding.UTF8.GetString(encoded_data);
            // .NET 內部處理字串採用 Unicode 。故此處之意為 UTF-8 轉 Unicode 。
            // 將這些位元組資料，用 UTF-8 編碼法解讀成 Unicode 字串。
            Console.WriteLine(decoded_text);
        }
    }
}
