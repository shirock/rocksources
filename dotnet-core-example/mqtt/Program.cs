using System;
using System.Net.Mqtt;
using System.Reactive.Linq;
// 加上這個 Linq 套件， MessageStream 才能接 Where 方法。
using System.Text;

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            var broker = "127.0.0.1";
            var clientId = "jlfdalkfd";
            var username = "test";
            var password = "123";

            var mqClient = new SimpleMqttClient(
                    broker, 
                    1883,
                    username,
                    password,
                    clientId);

            mqClient.Connect();
            // 此處的 Main() 沒有宣告 async ，所以 connect 不用 await 的寫法。
            // var sessionState = await mqClient.ConnectAsync();

            _ = mqClient.PublishAsync("tw/test", Encoding.UTF8.GetBytes("hello"));

            _ = mqClient.SubscribeAsync("tw/test/#");

            // observer 設計模式
            // Subscribe 可以串好幾個。
            // 如果想要每個訂閱方法負責處理不同的主題，
            // 就要在方法中加上主題判斷。
            // 或是使用 System.Reactive.Linq ，
            // 你就可以用 Where 。
            mqClient.Client.MessageStream
                .Subscribe(MessageArrived);

            // MessageArrived1 只想處理 tw/test/1 的主題，
            // 所以其方法開頭會判斷這個訊息的主題符不符合。
            mqClient.Client.MessageStream
                .Subscribe(MessageArrived1);

            // 或者像下面這樣用 Where 。
            // 只處理 topic 為 "tw/test/2" 的訊息。
            mqClient.Client.MessageStream
                .Where(msg => msg.Topic == "tw/test/2")
                .Subscribe(msg => {
                    Console.WriteLine(Encoding.UTF8.GetString(msg.Payload));
                });

            Console.WriteLine("press any key to end");
            Console.Read();

            mqClient.DisconnectAsync();
            return;
        }

        public static void MessageArrived(MqttApplicationMessage msg)
        {
            var msg_text = Encoding.UTF8.GetString(msg.Payload);
            Console.WriteLine(String.Format("topic {0}\n{1}", 
                msg.Topic,
                Encoding.UTF8.GetString(msg.Payload)));
            return;
        }

        public static void MessageArrived1(MqttApplicationMessage msg)
        {
            if (msg.Topic != "tw/test/1")
                return;

            var msg_text = Encoding.UTF8.GetString(msg.Payload);
            Console.WriteLine(String.Format("topic {0}\n{1}", 
                msg.Topic,
                Encoding.UTF8.GetString(msg.Payload)));
            return;
        }
    }
}
