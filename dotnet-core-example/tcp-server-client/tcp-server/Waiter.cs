using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

public class Waiter
{
    const int BUF_LEN = 8192;

    TcpClient client;
    string addr;

    public Waiter(TcpClient client)
    {
        this.client = client;
    }

    public void Serve()
    {
        addr = ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString();
        Console.WriteLine($"[{addr}] client connected");

        var stream = client.GetStream();
        byte[] data = new byte[BUF_LEN];
        int data_len = 0;

        while (true) 
        {
            try
            {
                data_len = stream.Read(data, 0, BUF_LEN);
                if (data_len <= 0)
                {
                    Console.WriteLine($"[{addr}] end of stream (client closed)");
                    break;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                break;
            }

            var text = Encoding.UTF8.GetString(data, 0, data_len);
            Console.WriteLine($"[{addr}] recv {text}");

            text = text.ToUpper();
            try
            {
                stream.Write(Encoding.UTF8.GetBytes(text));
                stream.Flush();
            }
            catch
            {
                Console.WriteLine($"[{addr}] connection break");
                break;
            }
        }

        client.Close();
        Console.WriteLine($"[{addr}] client disconnected");
        return;
    }
} // end class Waiter