using System;
using System.Net.Mqtt;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

/*
package: https://www.nuget.org/packages/System.Net.Mqtt
source: https://github.com/xamarin/mqtt 
 */
namespace mqtt
{
    class Publisher
    {
        private IMqttClient client;
        public IMqttClient Client {
            get {
                return client;
            }
        }

        private MqttClientCredentials credentials;
        public MqttClientCredentials Credentials { 
            get {
                return credentials;
            }
        }

        public Publisher(string broker, int port)
        {
            var configuration = new MqttConfiguration {
                Port = port
            };
            this.client = MqttClient.CreateAsync(broker, configuration).Result;
        }

        /**
        System.Net.Mqtt 按照 MQTT 規格， client id 有下列限制: 
        1. 只允許使用 a-zA-Z0-9 字元。
        2. 早期版本限制 client id 長度 23 字元。
        但我呼叫前會把 client id 用 MD5 編碼後取固定長度內容，免除使用者限制。
         */
        public Publisher(
                string broker, 
                int port, 
                string username, 
                string password, 
                string clientId=null) 
            : this(broker, port)
        {
            if (!string.IsNullOrEmpty(clientId)) {
                var data = MD5.Create().ComputeHash(Encoding.UTF8.GetBytes(clientId));
                // 用 MD5 將使用者輸入的 client id 轉成可用字元。
                StringBuilder sbuf = new StringBuilder();
                // 從雜湊結果取22字元(11位元組)。
                for (var i = 0; i < 11; i++) {
                    sbuf.Append(data[i].ToString("x2"));
                }
                clientId = sbuf.ToString();
                // Console.WriteLine("The MD5 hash of " + clientId + " is: " + clientId + ".");
            }

            // 若省略 clientId (null or "")， MqttClientCredentials 會自己編一個
            this.credentials = new MqttClientCredentials(clientId, username, password);
        }

        public async void PublishAsync(
            string topic, 
            byte[] payload, 
            MqttQualityOfService qos = MqttQualityOfService.AtMostOnce)
        {
            var sessionState = await Client.ConnectAsync(Credentials);
            var message = new MqttApplicationMessage(topic, payload);

            await Client.PublishAsync(message, qos);
            
            await Client.DisconnectAsync();
            // 若不中斷連接，此線不會結束。
            return;
        }

        public async void PublishAsync(string topic, byte[] payload, int qos)
        {
            await Task.Run(()=>
                PublishAsync(topic, payload, (MqttQualityOfService) qos));
        }
    } // end class Publisher

    class Program
    {
        static void Main(string[] args)
        {
            var broker = "localhost";
            var clientId = "mqtt-publisher-test";
            // 我呼叫前會把 client id 用 MD5 編碼後取固定長度內容，不受規格限制。
            var username = "test";
            var password = "testtest";

            var pub = new Publisher(
                broker, 
                1883,
                username,
                password,
                clientId
            );

            pub.PublishAsync("/test", Encoding.UTF8.GetBytes("hello"));

            var t = Task.Run(()=>
                pub.PublishAsync("/test", Encoding.UTF8.GetBytes("hello run")));
            t.Wait();

            Console.WriteLine("press any key to end");
            Console.Read();
            return;
        }
    }
}
