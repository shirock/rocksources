/*
dotnet add package System.Net.Mqtt --version 0.6.3-beta

See https://www.nuget.org/packages/System.Net.Mqtt
Project site: https://github.com/xamarin/mqtt
*/
using System;
using System.Net.Mqtt;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace Example
{
    public class SimpleMqttClient
    {
        public IMqttClient Client = null;
        public MqttClientCredentials Credentials = null;

        public SimpleMqttClient(string broker, int port)
        {
            var configuration = new MqttConfiguration {
                Port = port
            };
            this.Client = MqttClient.CreateAsync(broker, configuration).Result;

            this.Credentials = new MqttClientCredentials(clientId: GetAnonymousClientId());
        }

        public SimpleMqttClient(string broker, 
                                int port, string 
                                username, 
                                string password, 
                                string clientId=null) 
            : this(broker, port)
        {
            // 若省略 clientId (null or "") 就隨機編一個
            if (string.IsNullOrEmpty(clientId)) {
                clientId = GetAnonymousClientId();
            }
            else {
                var data = MD5.Create().ComputeHash(Encoding.UTF8.GetBytes(clientId));
                // client id 基本上只用 a-zA-Z0-9 字元，用 MD5 將使用者輸入的 client id 轉成可用字元。
                StringBuilder sbuf = new StringBuilder();
                // MQTT 規格早期版本限制 client id 長度 23 字元。故從雜湊結果取22 HEX 字元(11位元組)。
                for (var i = 0; i < 11; i++) {
                    sbuf.Append(data[i].ToString("x2"));
                }
                clientId = sbuf.ToString();
                // Console.WriteLine("The MD5 hash of " + clientId + " is: " + clientId + ".");
            }

            this.Credentials = new MqttClientCredentials(clientId, username, password);
        }

        internal static string GetAnonymousClientId() =>
            string.Format(
                "anonymous{0}",
                Guid.NewGuid().ToString().Replace("-", string.Empty).Substring(0, 10)
            );

        public async Task PublishAsync(string topic, byte[] payload, int qos = 0)
        {
            var message = new MqttApplicationMessage(topic, payload);

            await Client.PublishAsync(message, (MqttQualityOfService) qos);
        }

        /*
        代表 qos 的列舉符號如下，但我習慣用 0, 1, 2。
        MqttQualityOfService.AtMostOnce,
        MqttQualityOfService.AtLeastOnce,
        MqttQualityOfService.ExactlyOnce
        */
        public async Task SubscribeAsync(string topic, MqttQualityOfService qos = 0)
        {
            await Client.SubscribeAsync(topic, qos);
        }

        /*
        System.Net.Mqtt 設計的服務連接方式 (ConnectAsync) 不太好用。
        在連接 broker 之前，你不能訂閱主題。
        所以你實際上得要把 ConnectAsync() 變成同步工作。

        在非同步方法中呼叫 ConnectAsync() 時，寫法如下:
        var sessionState = await mqClient.ConnectAsync();

        在同步方法中呼叫 ConnectAsync() 時，寫法如下:
        var sessionState = Client.ConnectAsync().Result;

        所以我這裡直接設計成同步方法。

        我不知道為何 System.Net.Mqtt (~v0.6.3) 有 Disconnected 事件，卻沒有 Connected 事件。
        */
        public SessionState Connect()
        {
            // Task.Run(()=>this.Client.ConnectAsync(Credentials)).Wait();
            return Client.ConnectAsync(Credentials, cleanSession: true).Result;
       }

        public async Task<SessionState> ConnectAsync()
        {
            return await Client.ConnectAsync(Credentials);
        }

        public async void DisconnectAsync()
        {
            await Client.DisconnectAsync();
        }
    } // end class
}