using System;
using System.Threading;
using System.Net;
using System.Net.Sockets;

// See 
// https://docs.microsoft.com/zh-tw/dotnet/api/system.net.sockets.tcplistener
public class Server
{
    static int IncomingPort;

    static void Main(string[] args)
    {
        TcpListener server = null;

        if (args.Length < 1)
        {
            Console.WriteLine($"Usage: server <port>");
            return;
        }

        IncomingPort = Int32.Parse(args[0]);

        try
        {
            server = new TcpListener(new IPEndPoint(IPAddress.Any, IncomingPort));
            server.Start();

            Console.WriteLine("[{0}] Working... ", DateTime.Now.ToString("s"));
            Console.WriteLine("press Ctrl+C to exit");

            // Enter the listening loop.
            while (true)
            {
                TcpClient client = server.AcceptTcpClient();
                client.ReceiveTimeout = 60000; // 1 minute

                var waiter = new Waiter(client);
                var waiter_thread = new Thread(new ThreadStart(waiter.Serve))
                {
                    Name = client.Client.RemoteEndPoint.ToString(),
                    IsBackground = true
                };
                waiter_thread.Start();
            }
        }
        catch (SocketException e)
        {
            Console.WriteLine("SocketException: {0}", e);
        }
        finally
        {
            server.Stop();
        }
    }
} // end class Server