using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Text;
using System.Net.Http;
using System.Net.Http.Headers;

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            string result;

            HttpResponseMessage response;

            result = ProcessIndex().Result;
            Console.WriteLine(result);

            result = ProcessGetId(5).Result;
            Console.WriteLine(result);

            result = ProcessPost().Result;
            Console.WriteLine(result);

            response = ProcessPut(5).Result;
            Console.WriteLine(response);

            response = ProcessDelete(5).Result;
            Console.WriteLine(
                string.Format("Status: {0} {1}", 
                (int)response.StatusCode, 
                response.StatusCode));
            // https://docs.microsoft.com/zh-tw/dotnet/api/system.net.httpstatuscode?view=netcore-3.0
        }

        private static async Task<string> ProcessIndex()
        {
            var client = new HttpClient();
            
            client.DefaultRequestHeaders.Accept.Clear();
            client.DefaultRequestHeaders.Accept.Add(
                new MediaTypeWithQualityHeaderValue("application/json"));

            var result = await client.GetStringAsync("http://localhost:5000/api/values");
            return result;
        }

        private static async Task<string> ProcessGetId(int id)
        {
            var client = new HttpClient();
            
            client.DefaultRequestHeaders.Accept.Clear();
            client.DefaultRequestHeaders.Accept.Add(
                new MediaTypeWithQualityHeaderValue("application/json"));

            // If you need to set Content-Type header, use this way:
            // var request = new HttpRequestMessage(HttpMethod.Get, "http://localhost:5000/api/values/5");
            // request.Content = new StringContent(String.Empty, Encoding.UTF8, "application/json");
            var request = new HttpRequestMessage() {
                Method = HttpMethod.Get,
                RequestUri = new Uri($"http://localhost:5000/api/values/{id}"),
                Content = new StringContent(string.Empty, Encoding.UTF8, "application/json")
            };

            var result = await client.SendAsync(request).Result.Content.ReadAsStringAsync();
            return result;
        }

        private static async Task<string> ProcessPost()
        {
            var client = new HttpClient();

            client.DefaultRequestHeaders.Accept.Clear();
            client.DefaultRequestHeaders.Accept.Add(
                new MediaTypeWithQualityHeaderValue("application/json"));

            var request = new HttpRequestMessage(HttpMethod.Post, "http://localhost:5000/api/values");
            request.Content = new StringContent(
                "\"hello world\"", Encoding.UTF8, "application/json");

            var result = await client.SendAsync(request).Result.Content.ReadAsStringAsync();
            return result;
        }

        private static async Task<HttpResponseMessage> ProcessPut(int id)
        {
            var client = new HttpClient();

            client.DefaultRequestHeaders.Accept.Clear();
            client.DefaultRequestHeaders.Accept.Add(
                new MediaTypeWithQualityHeaderValue("application/json"));

            var request = new HttpRequestMessage(HttpMethod.Put, $"http://localhost:5000/api/values/{id}");
            request.Content = new StringContent(
                "\"hello world\"", Encoding.UTF8, "application/json");

            var result = await client.SendAsync(request);
            return result;
        }

        private static async Task<HttpResponseMessage> ProcessDelete(int id)
        {
            var client = new HttpClient();
            var result = await client.DeleteAsync($"http://localhost:5000/api/values/{id}");
            return result;
        }
    }
}
