// .NET 6
using System.Net.Mqtt;
using System.Text;

var broker = "localhost";
var topic = "my/test";

var configuration = new MqttConfiguration 
{
    Port = 1883,
    MaximumQualityOfService = MqttQualityOfService.AtMostOnce
};
var mqClient = MqttClient.CreateAsync(broker, configuration).Result;
_ = mqClient.ConnectAsync().Result;

var count = 0;

var pubTimer = new Timer(state => {
    var msg = new MqttApplicationMessage(
        topic, 
        Encoding.UTF8.GetBytes(count.ToString()));

    mqClient.PublishAsync(msg, MqttQualityOfService.AtLeastOnce);
    // 發佈訊息指定的 qos 大於 AtMostOnce (0)

    Console.WriteLine(string.Format("{0} publish {1}"), 
        DateTime.Now.ToString("s"), count);
    count++;
}, null, 500, 20000);

Console.WriteLine("press Enter to end.");
Console.ReadLine();
