/*
分組求和。
SELECT name, SUM(qty) FROM items GROUP BY name;
*/
using System;
using System.Collections.Generic;
using System.Linq;

class Item
{
    public string Name;
    public int Qty;

    public Item(string n, int c)
    {
        Name = n;
        Qty =  c;
    }
}

class Program
{
    static void Main()
    {
        var items = new List<Item>
        {
            new Item("a", 1),
            new Item("b", 2),
            new Item("c", 3),
        };
        // Add method
        items.Add(new Item("a", 4));

        // SELECT name, SUM(qty) FROM items GROUP BY name;
        var gsum = 
            from item in items
            group item by item.Name into g
            select new
            {
                Name = g.Key,
                TotalQty = g.Sum(item => item.Qty)
            };

        // 函式敘述
        var gs2 = items.
            GroupBy(item => item.Name).
            Select(g => new
            {
                Name = g.Key,
                TotalQty = g.Sum(item => item.Qty)
            });

        foreach (var name_qty in gsum)
            Console.WriteLine($"{name_qty.Name} total {name_qty.TotalQty}");
    }
}
