/*
本範例: 自行處理「未處理的例外狀況」事件，而不是彈出訊息視窗。

1. dotnet build 偵錯組態 (建置時的預設組態) 會彈出訊息視窗。
2. dotnet build -c Release 之類的組態不會彈出視窗。
*/
using System.Windows.Forms;

    static class Program
    {
        static int unexpected_count = 0;

        static void OnThreadException(object sender, ThreadExceptionEventArgs e)
        {
            // 忽視未處理例外，最多三次。
            if (unexpected_count < 3)
            {
                Console.WriteLine("ignore unexpected exception");
                unexpected_count++;
            }
            else
            {
                Console.WriteLine("unexpected exception");
                Environment.Exit(119);
            }
        }

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // Application.SetHighDpiMode(HighDpiMode.SystemAware);
            // Application.EnableVisualStyles();
            // Application.SetCompatibleTextRenderingDefault(false);
            // To customize application configuration such as set high DPI settings or default font,
            // see https://aka.ms/applicationconfiguration.
            ApplicationConfiguration.Initialize();

            // Application.SetUnhandledExceptionMode(UnhandledExceptionMode.ThrowException);
            /*
            ThrowException - 程式不處理未預期例外，直接往外扔。通常會是由作業系統直接結束程式。
            */

            // Application.SetUnhandledExceptionMode(UnhandledExceptionMode.Automatic);
            /*
            Automatic - 是預設處置，判斷條件如下:
            1.未指定 Application.ThreadException 事件處理方法時，跳出
              "未處理的例外狀況" 視窗，讓使用者選擇"繼續執行"或"結束程式"。
              此方法一般不適用在最終使用者的場合。
            2.指定 Application.ThreadException 事件處理方法後，則呼叫指定處理方法。
            */

            // Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
            /*
            CatchException 和 Automatic 之差別在於 CatchException 會忽視應用程式組態檔。
            */

            /*
            結論:
            通常不必使用 SetUnhandledExceptionMode() 改變處置策略，
            指派 ThreadException 事件的處理方法即可。
            或可將指派 ThreadException 事件處理方法的程式碼，放進 !DEBUG 前置區塊。
            這樣在程式開發階段，開發人員依然能查看例外內容。
            */
            #if !DEBUG
            // 不跳出"未處理的例外狀況"視窗，經由指定的事件處理方法結束程式。
            // Style 1: Inline method.
            Application.ThreadException += (O,z) =>
            {
                Console.WriteLine("unexpected exception");
                Environment.Exit(119);
            };

            // Style 2: Static method.
            // Application.ThreadException += 
            //     new ThreadExceptionEventHandler(OnThreadException);
            #endif

            Application.Run(new Form1());
        }
    }
