using System;

/*
1. Parse() or TryParse()
字串來源與格式明確時，可用 Parse(), 不加 try ... catch 。
否則建議用 TryParse() 。因為例外的處理成本比較貴。

2. 時間字串格式 [ISO 8601](https://zh.wikipedia.org/wiki/ISO_8601)
自訂日期與時間格式字串
https://docs.microsoft.com/zh-tw/dotnet/standard/base-types/custom-date-and-time-format-strings?view=netcore-3.0

*/
/* Microsoft Docs
https://docs.microsoft.com/zh-tw/dotnet/api/system.datetime?view=netcore-3.0
You should use the TryParse or TryParseExact methods in scenarios where performance 
is important. 
The parsing operation for date and time strings tends to have a high failure rate, 
and exception handling is expensive.
*/
namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            DateTime datetime;

            // Unix紀元起始時間 1970-01-01 00:00:00
            Console.WriteLine($"Unix Epoch: {DateTime.UnixEpoch}"); 

            // 今天(不含時間)
            Console.WriteLine($"Today: {DateTime.Today}");  

            // 現在本地時間 (日期與時間)
            Console.WriteLine($"Now: {DateTime.Now}"); 

            // 現在統一標準時間 (日期與時間)
            Console.WriteLine($"UtcNow: {DateTime.UtcNow}"); 

            // field: Year, Month, Day, Hour, Minute, Second
            Console.WriteLine($"今年度: {DateTime.Now.Year}"); 

            Console.WriteLine($"上個月份: {DateTime.Now.AddMonths(-1).Month}"); 
            Console.WriteLine($"下一小時: {DateTime.Now.AddHours(1).Hour}"); 

            // 年, 月, 日, 時, 分, 秒
            datetime = new DateTime(2018, 1, 2, 11, 22, 33);
            Console.WriteLine($"new DateTime {datetime}");

            var date_str = "20190123"; // 不可解
            var date_strx = "2019-01-23"; // 可解

            if (DateTime.TryParse(date_str, out datetime) == false) {
                Console.WriteLine($"不認得 ISO 8601 基本格式 {date_str}");
            }

            if (DateTime.TryParse(date_strx, out datetime) == false) {
                Console.WriteLine("不認得 ISO 8601 擴展格式");
            }
            else {
                Console.WriteLine($"可分析 ISO 8601 擴展格式 {date_strx}");
                // 若字串不含時間部份，預設0時0分0秒。
                Console.WriteLine($"parse {date_strx} => {datetime}"); 
            }

            var time_str = "12:34:56";
            datetime = DateTime.Parse(time_str);
            // 不含日期部份，則預設為今天。
            Console.WriteLine($"parse {time_str} => {datetime}"); 

            var datetime_str  = "2019-01-23 12:34:56"; // 習慣表述。可解。
            var datetime_strx = "2019-01-23T12:34:56"; // 標準表述。用 T 結合。可解。

            try {
                datetime = DateTime.Parse(datetime_str);
                Console.WriteLine($"可分析日期與時間組合的習慣表達用法 {datetime_str}");
            }
            catch (FormatException e) {
                Console.WriteLine(e);
                Console.WriteLine("不認得日期與時間組合的習慣表達用法");
            }

            datetime = DateTime.Parse(datetime_strx);
            Console.WriteLine($"可分析日期與時間組合的標準表達用法 {datetime_strx}");
            Console.WriteLine(
                string.Format("DateTime 轉字串:\n"
                    + "預設格式採 localization 格式: {0}\n"
                    + "ISO 格式 (\"s\"): {1}", 
                datetime, 
                datetime.ToString("s")));
        }
    }
}
