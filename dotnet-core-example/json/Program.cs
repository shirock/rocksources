/**
.NET Core 3 起，內建 System.Text.Json ，取代第三方的 Newtonsoft.Json.

雖然 .NET 5 的 System.Text.Json 功能比較完整，
但此範例只用 .NET Core 3 可支援的內容。

See https://docs.microsoft.com/zh-tw/dotnet/standard/serialization/system-text-json-how-to
 */
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace Example
{
    /*
    .NET Core 3 Json 預設只處理公開屬性(public property)
    .NET 5 Json 可處理公開欄位(public field), 
    step 1. 資料欄位 attribute 加上 [JsonInclude]
    step 2. 序列方法加選項 new JsonSerializerOptions {IncludeFields = true};
    */
    public class MyType
    {
        public int Id { get; set; }
        public string Name { get; set; }
        public bool Flag { get; set; }
        public IList<int> NumberList { get; set; }
        public Dictionary<string, int> KeyTable { get; set; }
    }

    class Program
    {
        static void Main(string[] args)
        {
            var a = new 
            {
                id = 1,
                name = "你好 hello",
                flag = true,
                numberList = new []{1, 2, 3},
                keyTable = new {
                    A = 100,
                    B = 200,
                    C = 300
                }
            };

            // Reuse it to avoid performance problem.
            var options = new JsonSerializerOptions { 
                WriteIndented = true, // Json內容縮排
                PropertyNameCaseInsensitive = true // 忽視屬性名稱大小寫
            };

            // 可將匿名類別序列化
            // json_encode
            string jsonString = JsonSerializer.Serialize(a, options);

            Console.WriteLine(jsonString);

            var jsonFile = "r:/test.json";
            // file_put_contents
            File.WriteAllText(jsonFile, jsonString);

            // file_get_contents
            string jsonString2 = File.ReadAllText(jsonFile);

            // 但不能還原匿名類別。必須告知類別
            // json_decode
            var b = JsonSerializer.Deserialize<MyType>(jsonString2, options);
            // 來源對象的屬性名稱規則是CamelCase，和目標對象的屬性名稱規則不同。
            // 此時可用 options PropertyNameCaseInsensitive

            Console.WriteLine($"id: {b.Id}");
            Console.WriteLine($"name: {b.Name}");
            Console.WriteLine($"flag: {b.Flag}");
            Console.WriteLine($"numberlist[2]: {b.NumberList[2]}");
            Console.WriteLine($"keytable[a]: {b.KeyTable["A"]}");
            Console.WriteLine($"keytable[b]: {b.KeyTable.GetValueOrDefault("B")}");
        }
    }
}
