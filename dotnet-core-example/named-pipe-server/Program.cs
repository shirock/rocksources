using System;
using System.Threading;

public class Server
{
    private static int numThreads = 4;

    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine($"Usage: server <path>");
            return;
        }

        var pipePath = args[0];
        var servers = new Thread[numThreads];

        for (var i = 0; i < numThreads; i++)
        {
            var waiter = new Waiter(pipePath, numThreads);
            servers[i] = new Thread(new ThreadStart(waiter.Serve))
            {
                IsBackground = true
            };
            servers[i].Start();
        }

        Thread.Sleep(100);
        Console.WriteLine("Working. Press any key to exit.");
        Console.ReadLine();
    }
} // end class Server