/*
.NET 沒有 signal handler 機制。
但 .NET 將 SIGTERM, SIGINT, SIGQUIT 這三個訊號處理工作設計成處理程序事件。

SIGTERM => AppDomain.CurrentDomain.ProcessExit 。
https://docs.microsoft.com/zh-tw/dotnet/api/system.appdomain.processexit

SIGINT/SIGQUIT => Console.CancelKeyPress 。
https://docs.microsoft.com/zh-tw/dotnet/api/system.console.cancelkeypress
*/
using System;

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");

            // SIGTERM
            // 注意， ProcessExit 事件在程序正常終止的情形下也會觸發。
            // 例如 Main() 結束或呼叫 Environment.Exit() 。
            AppDomain.CurrentDomain.ProcessExit += (s, ev) =>
            {
                System.Console.WriteLine("SIGTERM received");
            };

            // SIGINT/SIGQUIT
            // 在終端機按下 Ctrl+C 就等於 kill -SIGINT
            Console.CancelKeyPress += (sender, e) =>
            {
                System.Console.WriteLine("SIGINT/SIGQUIT received");
                // 等程序跑完事件處理程序後，作業系統就會結束程序。
                // 不需要程序自行呼叫exit。
            };

            Console.WriteLine("press Ctrl+C to break program.");
            Console.WriteLine("press 'q' or 'e' to normal end.");
            while (true)
            {
                var key = Console.ReadKey();
                if (key.KeyChar == 'q')
                {
                    Console.WriteLine("quit key loop");
                    break;
                }

                if (key.KeyChar == 'e')
                {
                    Console.WriteLine("invoke Exit()");
                    Environment.Exit(0);
                }
            }

            Console.WriteLine("end of Main()");
        }
    }
}
