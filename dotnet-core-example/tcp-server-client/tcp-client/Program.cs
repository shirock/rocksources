using System;
using System.IO;
using System.Net.Sockets;
using System.Text;

// See
// https://docs.microsoft.com/zh-tw/dotnet/api/system.net.sockets.tcpclient
class Program
{
    static void Main(string[] args)
    {
        TcpClient client = null;
        string ServerIP;
        int ServerPort;

        if (args.Length < 2)
        {
            Console.WriteLine($"client <server_ip> <server_port> <messages ...>");
            Console.WriteLine("ex: tcp-client localhost 4321 abc hello world!");
            return;
        }

        ServerIP = args[0];
        ServerPort = Int32.Parse(args[1]);

        try
        {
            client = new TcpClient();
            client.Connect(ServerIP, ServerPort);
        }
        catch
        {
            Console.WriteLine("連線失敗，請確認網路可用");
            return;
        }

        var stream = client.GetStream();
        var rb_stream = new StreamReader(new BufferedStream(stream));
        // var wb_stream = new StreamWriter(new BufferedStream(stream));

        byte[] data;
        string resp;
        for (var i = 2; i < args.Length; i++)
        {
            data = Encoding.UTF8.GetBytes(string.Format("{0}\n", args[i]));
            stream.Write(data, 0, data.Length); // 無緩衝區輸出。

            resp = rb_stream.ReadLine(); // 經緩衝讀取
            Console.WriteLine(string.Format("Response: {0}", resp));
        }

        client.Close();
        return;
    }
}