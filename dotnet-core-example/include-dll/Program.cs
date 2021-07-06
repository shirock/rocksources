using System;
using System.Threading;
using XP3620.Devices; // 來自 XP3620Library.dll ，看 README.md 了解如何加入。

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            var mReader = new XPDevice("COM3");

            try
            {
                mReader.Connect();
                Thread.Sleep(1000);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return;
            }

            
            try
            {
                mReader.ReadEPCMemory();
                mReader.Disconnect();
            }
            catch {/*pass*/}
        }
    }
}
