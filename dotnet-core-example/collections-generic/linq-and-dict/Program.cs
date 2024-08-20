/*
將 Dictionary 當作有主鍵(primary key)的資料表。
並求集合:
1. SELECT * FROM items WHERE code > 1;
2. SELECT key FROM items WHERE key NOT IN selected;
*/
using System;
using System.Collections.Generic;
using System.Linq;

class Item
{
    public string name;
    public int code;

    public Item(string n, int c)
    {
        name = n;
        code = c;
    }
}

class Program
{
    static void Main()
    {
        var items = new Dictionary<string, Item>
        {
            {"a", new Item("nameA", 1)},
            {"b", new Item("nameB", 2)},
        };
        // Add method
        items.Add("c", new Item("nameC", 3));

        // SELECT * FROM items WHERE code > 1;
        var gt1 = from item in items
                    where item.Value.code > 1
                    select item;

        // 函式敘述
        var gt2 = items.
            Where(items => items.Value.code > 1).
            Select(item => item);

        Console.WriteLine("Greater Than 1:");
        foreach (var item in gt1)
            Console.WriteLine($"{item.Key}: {item.Value.name}|{item.Value.code}");

        var selected = new HashSet<string>
        {
            "a",
            "c"
        };

        // SELECT key FROM items WHERE key NOT IN selected;
        var unselected =
            from key in items.Keys
                where !selected.Contains(key)
                select key;

        // 函式敘述
        var unselected2 = items.Keys.
            Where(key => !selected.Contains(key)).
            Select(key => key);

        Console.WriteLine("Unselected:");
        foreach (var key in unselected)
            Console.WriteLine(key);
    }
}
