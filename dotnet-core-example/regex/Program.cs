using System;
using System.Text.RegularExpressions;
/*
.NET Core C# example
Regex match and split.
 */
namespace Example
{
    class Program
    {
        static void IsMatch(string id)
        {
            var rs = @"^[a-zA-Z][\w]+$";
            var regex = new Regex(rs);
            var m = regex.IsMatch(id);
            Console.WriteLine(String.Format("Is \"{0}\" match pattern {1}: {2}", id, rs, m));
        }

        static void Main(string[] args)
        {
            IsMatch("Rock123");
            IsMatch("123Rock");
            IsMatch("Rock 1@3");

            var s = "card-012-345-x:67";
            Regex pattern = new Regex(@"\d+");
            MatchCollection matches = pattern.Matches(s);
            Console.WriteLine($"符合數目: {matches.Count}");
            if (matches.Count > 0) {
                foreach (Match match in matches)
                    Console.WriteLine(match.Value);
            }
        }                    
    }
}
