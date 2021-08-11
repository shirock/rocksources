using System;
using System.Diagnostics;

namespace Example
{
    class Program
    {
        static Process ExecProgram(string command)
        {
            string program;
            string arg = "";
            var pa = command.Split(' ', 2);
            program = pa[0];
            if (pa.Length > 1) 
            {
                arg = pa[1];
            }

            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = program,
                    Arguments = arg,
                    UseShellExecute = false,
                    CreateNoWindow = true
                }
            };
            process.Start();
            return process;
        }

        static void Main(string[] args)
        {
            var p1 = ExecProgram("xcopy /y Program.cs r:\\");
            // var p1 = ExecCommand("cmd /C copy /y Program.cs r:\\");

            // 非同步式
            p1.Exited += (sender, ev) => {
                System.Console.WriteLine("ok", p1.ExitCode);
            };

            // 同步式
            p1.WaitForExit();
            Console.WriteLine(p1.ExitCode);

            ExecProgram("notepad");
        }
    }
}
