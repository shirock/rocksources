using System;

/*
https://docs.microsoft.com/zh-tw/dotnet/api/system.timespan?view=netcore-2.2
 */
namespace datetime_timespan
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

            Console.WriteLine((DateTime.Now - DateTime.UtcNow).Hours);

            datetime = DateTime.Now - new TimeSpan(0, 10, 0);
            Console.WriteLine(datetime);
        }
    }
}
