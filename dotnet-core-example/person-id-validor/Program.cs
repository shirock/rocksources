string[] id_list;

if (args.Length < 1) 
{
    id_list = new string[]
    {
        "A123456789", // valid
        "B123456789",
        "a123456789", // valid
        "A12345678",  // short
        "A1234567890", // long
        "A1z3456789", // invalid char
        "A12345 789"  // invalid char
    };
}
else
{
    id_list = args;
}

foreach (var id in id_list)
{
    if (PersonIdValidor.IsValid(id))
    {
        Console.WriteLine($"{id} is valid.");
    }
    else
    {
        Console.WriteLine($"{id} is invalid!");
    }
}
