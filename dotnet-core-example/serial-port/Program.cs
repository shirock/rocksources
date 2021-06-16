/*
Example for COM/Serial Port.

$ dotnet add package System.IO.Ports

System.IO.Ports 的微軟官方文件沒有對應到 .NET Core ，
自然不會提到 System.IO.Ports 對 .NET Core 而言是外部套件，必須將它加入專案才能引入。

See https://docs.microsoft.com/zh-tw/dotnet/api/system.io.ports?view=netframework-4.8
*/
using System;
using System.IO.Ports;
using System.Text;
using System.Threading;

namespace Example
{
    class Program
    {
        public static string ReaderPath = "com3";
        // path of device (linux) or serial port (windows).
        public static SerialPort sp = null;

        // 這是某個條碼機的讀取控制指令
        public static string TRIGGER_COMMAND = "\x16\x54\x0d";
        public static string DEACTIVATE_COMMAND= "\x16\x55\x0d";

        static void Main(string[] args)
        {
            sp = new SerialPort();
            sp.PortName = ReaderPath;
            sp.BaudRate = 9600;
            sp.Parity = Parity.None;
            sp.DataBits = 8;
            sp.StopBits = StopBits.One;
            sp.ReadTimeout = 3000;

            sp.Open();

            sp.Write(TRIGGER_COMMAND);

            Read();
            // Thread readThread = new Thread(Read);
            // readThread.Start();
            // readThread.Join();
            // Console.WriteLine("Ready. Press any key to end.");
            // Console.Read();

            sp.Write(DEACTIVATE_COMMAND);

            sp.Close();
            return;
        }

        public static void Read()
        {
            string barcode = "";

            // 掃描時間 3 秒，每 0.25 秒檢查一次讀取緩衝區。
            int count = (int)(3 / 0.25);
            for (int i = 0; i < count; ++i) 
            {
                if (sp.BytesToRead > 0) 
                {
                    try
                    {
                        barcode = sp.ReadExisting();
                        // 目標 barcode 不含 newline ，用 ReadLine() 將等到 timeout 才回傳，
                        // 回應太慢。
                        break;
                    }
                    catch (TimeoutException) 
                    { }
                }
                else 
                {
                    Console.Write(".");
                    Thread.Sleep(250);
                }
            }
            Console.WriteLine($"barcode: [{barcode}]");
        }
    }
}
