// .NET 6
using System.Net.Mqtt;
using System.Text;

var broker = "localhost";
var topic = "my/test";

var configuration = new MqttConfiguration 
{
    Port = 1883,
    MaximumQualityOfService = MqttQualityOfService.ExactlyOnce
    // MaximumQualityOfService = MqttQualityOfService.AtMostOnce
    // 預設是 AtMostOnce 。
    // 如果 MaximumQualityOfService 和訂閱時的 qos 不匹配，
    // 則訂閱者最多可收到20次訊息。超過就會失聯。
    // 我也不知道為什麼定這個數目。也不知道為何不擲出例外。
};
var mqClient = MqttClient.CreateAsync(broker, configuration).Result;
_ = mqClient.ConnectAsync().Result;

mqClient.SubscribeAsync(topic, MqttQualityOfService.ExactlyOnce);

mqClient.MessageStream.Subscribe(msg => {
    var msgText = Encoding.UTF8.GetString(msg.Payload);
    var timeStamp = DateTime.Now.ToString("s");
    Console.WriteLine($"{timeStamp} receive {msgText}");
});

Console.WriteLine("press Enter to end.");
Console.ReadLine();
