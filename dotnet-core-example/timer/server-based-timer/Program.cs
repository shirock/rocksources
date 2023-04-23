/*
Server-based Timer: System.Timers.Timer
https://docs.microsoft.com/en-us/dotnet/api/system.timers.timer

System.Timers.Timer fires an event at regular intervals. 
The class is intended for use as a server-based or service component in a 
multithreaded environment.
It has no user interface and is not visible at runtime.
*/
using System;

namespace Example
{
    class Program
    {
        public static System.Timers.Timer HeartBeatTimer;
        public static System.Timers.Timer TriggerOnceTimer;
        public static System.Timers.Timer StandbyTimer;

        public static void StartOnceTimer()
        {
            /*
            計時器設置:
            1. 建立後立即計時: 接著呼叫 Start()
            2. 計時後2秒執行內容: interval = 2000
            3. 只執行一次: AutoReset = false
            */
            var onceTimer = new System.Timers.Timer(2000);
            onceTimer.AutoReset = false;
            onceTimer.Elapsed += (s, e)=>{
                Console.WriteLine($"計時器2結束 {DateTime.Now}");
            };
            onceTimer.Start();
        }

        static void Main(string[] args)
        {
            /*
            計時器設置:
            1. 建立後立即計時: 接著呼叫 Start()
            2. 計時後1秒執行內容: interval = 1000
            3. 定期重複執行: AutoReset = true
            */
            HeartBeatTimer =  new System.Timers.Timer(1000)
            {
                AutoReset = true
            };
            HeartBeatTimer.Elapsed += (s, e)=>{
                Console.WriteLine(".");
            };
            HeartBeatTimer.Start();

            /*
            計時器設置:
            1. 建立後先擱置: 等要用時才呼叫 Start()
            2. 計時後3秒執行內容: interval = 3000
            3. 只執行一次: AutoReset = false
            */
            TriggerOnceTimer = new System.Timers.Timer(3000)
            {
                AutoReset = false
            };
            TriggerOnceTimer.Elapsed += (s, e)=>{
                Console.WriteLine($"計時器1結束 {DateTime.Now}");

                Console.WriteLine("2秒後再做另一件事... 。示範用 StandbyTimer 計時器取代sleep()");
                StandbyTimer.Start(); // async
                // Thread.Sleep(2000); // sync
            };

            /*
            計時器設置:
            1. 建立後先擱置: 等要用時才呼叫 Start()
            2. 計時後2秒執行內容: interval = 2000
            3. 只執行一次: AutoReset = false
            */
            StandbyTimer = new System.Timers.Timer(2000)
            {
                AutoReset = false
            };
            StandbyTimer.Elapsed += (s, e)=>{
                Console.WriteLine("StandbyTimer is elapsed.");
            };

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
                    TriggerOnceTimer.Start();
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
