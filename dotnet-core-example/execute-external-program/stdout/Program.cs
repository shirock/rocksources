using System;
using System.Diagnostics;

namespace Example
{
    class Program
    {
        static void Main()
        {
            var program = "cmd";
            var args = "/C dir C:\\";

            var p1 = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = program,
                    Arguments = args,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardOutput = true
                }
            };

            // eventhandler(object, DataReceivedEventArgs)
            p1.OutputDataReceived += (sender, ev) => {
                Console.WriteLine($"read: {ev.Data}");
            };

            p1.Start();

            // raise OutputDataReceived
            p1.BeginOutputReadLine(); 

            p1.WaitForExit();
            Console.WriteLine($"exit: {p1.ExitCode}");
        }
    }
}
