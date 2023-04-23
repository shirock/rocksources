/*
Thread-based Timer: System.Threading.Timer: 
https://docs.microsoft.com/en-us/dotnet/api/system.threading.timer

System.Threading.Timer is a simple, lightweight timer that uses callback methods 
and is served by thread pool threads.
It has no user interface and is not visible at runtime.
*/
using System;
using System.Threading;

namespace Example
{
    class Program
    {
        public static System.Threading.Timer HeartBeatTimer;
        public static System.Threading.Timer TriggerOnceTimer;
        public static System.Threading.Timer StandbyTimer;

        public static void StartOnceTimer()
        {
            /*
            計時器設置:
            1. 建立後立即計時: dueTime is not Infinite
            2. 計時後2秒執行內容: dueTime = 2000
            3. 只執行一次: period = Infinite
            */
            var onceTimer = new System.Threading.Timer((state)=>{
                Console.WriteLine($"計時器2結束 {DateTime.Now}");
            }, null, 2000, Timeout.Infinite);
        }

        static void Main(string[] args)
        {
            /*
            計時器設置:
            1. 建立後立即計時: dueTime is not Infinite
            2. 計時後1秒執行內容: dueTime = 1000
            3. 定期重複執行: period = 1000
            */
            HeartBeatTimer = new System.Threading.Timer((state)=>{
                Console.WriteLine(".");
            }, null, 1000, 1000);

            /*
            計時器設置:
            1. 建立後先擱置: dueTime = Infinite, period = Infinite
            2. 計時後3秒執行內容: dueTime = 3000
            3. 只執行一次: period = Infinite
            */
            TriggerOnceTimer = new System.Threading.Timer((state)=>{
                Console.WriteLine($"計時器1結束 {DateTime.Now}");

                Console.WriteLine("2秒後再做另一件事... 。示範用 StandbyTimer 計時器取代sleep()");
                StandbyTimer.Change(2000, Timeout.Infinite);
            }, null, Timeout.Infinite, Timeout.Infinite);

            /*
            計時器設置:
            1. 建立後先擱置: dueTime = Infinite, period = Infinite
            2. 計時後2秒執行內容: dueTime = 2000
            3. 只執行一次: period = Infinite
            */
            StandbyTimer = new System.Threading.Timer((state)=>{
                Console.WriteLine("StandbyTimer is elapsed.");
            }, null, Timeout.Infinite, Timeout.Infinite);

            while (true)
            {
                Console.WriteLine("1個 dot 經過1秒.");
                Console.WriteLine("按 1 啟動計時器1，按 2 啟動計時器2，其他鍵則結束程式.");
                var ckey = Console.ReadKey();
                if (ckey.KeyChar == '1')
                {
                    // 計時器1是靜態實體，重複按t只會啟動同一個計時器
                    // 當計時器已經啟動時，再次呼叫 Start() 不會做任何事。
                    Console.WriteLine("啟動計時器1 (3秒後)。");
                    TriggerOnceTimer.Change(3000, Timeout.Infinite);
                }
                else if (ckey.KeyChar == '2')
                {
                    // 可重複建立多個計時器2
                    Console.WriteLine("啟動計時器2 (2秒後)。");
                    StartOnceTimer();
                }
                else
                {
                    break;
                }
            }
        }
    }
}
