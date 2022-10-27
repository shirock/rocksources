/**
注意，自 .NET6 起，使用 Unix Domain Socket 實作 NamedPipe Class ，而不是用 FIFO 。
Windwos 10 18H3 (Insider Build 17063) 起支援 Unix Domain Socket ，
故 .NET6 的 NamedPipe Class 自此可跨 Linux, MacOS, Windows 10 平台使用。
 */
using System;
using System.IO.Pipes;
using System.Text;

public class Waiter
{
    const int BUF_LEN = 8192;

    NamedPipeServerStream server;

    public Waiter(string pipePath, int n)
    {
        this.server = new NamedPipeServerStream(pipePath, PipeDirection.InOut, n);
    }

    public void Serve()
    {
        var tid = System.Threading.Thread.CurrentThread.ManagedThreadId;
        byte[] data = new byte[BUF_LEN];
        int data_len = 0;

        try
        {
            // Enter the listening loop.
            while (true)
            {
                server.WaitForConnection(); // blocked

                if (server.IsConnected)
                {
                    Console.WriteLine($"[{tid}] accept client");

                    while (true) 
                    {
                        try
                        {
                            data_len = server.Read(data, 0, BUF_LEN);
                            if (data_len <= 0)
                            {
                                Console.WriteLine($"[{tid}] end of stream (client closed)");
                                break;
                            }
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.Message);
                            break;
                        }

                        var text = Encoding.UTF8.GetString(data, 0, data_len);
                        Console.WriteLine($"[{tid}] recv {text}");

                        text = text.ToUpper();
                        try
                        {
                            server.Write(Encoding.UTF8.GetBytes(text));
                            server.Flush();
                        }
                        catch
                        {
                            Console.WriteLine($"[{tid}] connection break");
                            break;
                        }
                    }
                }

                Console.WriteLine($"[{tid}] connection disconnect");
                server.Disconnect();
            } // listening loop.
        }
        catch (Exception e)
        {
            Console.WriteLine("Exception: {0}", e);
        }
        finally
        {
            server.Close();
        }

        Console.WriteLine($"{tid} thread end");
        return;
    }
} // end class Waiter