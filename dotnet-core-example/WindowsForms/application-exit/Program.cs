/*
Windows Desktop 程式如何接收關閉程式的請求。

終止信號 (SIGTERM) 則由 AppDomain.CurrentDomain.ProcessExit 負責。
系統關機時，會對所有執行中程式發出這個信號。
參考 dotnet-core-example/sigterm-sigquit-handler
*/
using System;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace Example
{
    static class Program
    {
        static void OnApplicationExit(object sender, EventArgs e)
        {
            File.WriteAllText("log.txt", $"{DateTime.Now}: application exit");
        }

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Application.ApplicationExit += (O,z) =>
            {
                File.WriteAllText("log.txt", $"{DateTime.Now}: application exit");
            };

            // Application.ApplicationExit += 
            //     new EventHandler(OnApplicationExit);

            Application.Run(new Form1());
        }
    }
}
