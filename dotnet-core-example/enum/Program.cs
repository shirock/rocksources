// https://learn.microsoft.com/zh-tw/dotnet/csharp/language-reference/builtin-types/enum
var en0 = Enum1.ABC;
var en1 = (Enum1) 1;
var en2 = (Enum1) 2;
var en99 = (Enum1) int.Parse("99");
var enXyz= Enum.Parse(typeof(Enum1), "XYZ");

Console.WriteLine(en0);   // implicitly ToString().
// ABC
Console.WriteLine((int) en0);
// 0

Console.WriteLine(en1.ToString()); // print symbol
// DEF
Console.WriteLine(en2.ToString()); // no symbol, print number.
// 2

Console.WriteLine(en99);
// XYZ

Console.WriteLine(enXyz);
// XYZ

Console.WriteLine("Is defined XYZ: {0}", Enum.IsDefined(typeof(Enum1), "XYZ"));

public enum Enum1
{
    ABC  = 0,
    DEF  = 1,
    XYZ  = 99
}
