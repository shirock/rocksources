using System.ComponentModel;
using System.Drawing;

partial class Form1
{
    /// <summary>
    ///  Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    ///  Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
        if (disposing && (components != null))
        {
            components.Dispose();
        }
        base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    ///  Required method for Designer support - do not modify
    ///  the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
        this.components = new System.ComponentModel.Container();
        this.AutoScaleMode = AutoScaleMode.Font;
        this.ClientSize = new Size(400, 300);
        this.Text = "按鍵事件";
        this.FormBorderStyle = FormBorderStyle.FixedSingle;
        this.Shown += new EventHandler(this.Form1_Shown);
        this.Closing += new CancelEventHandler(this.Form1_Closing);

        this.KeyPreview = true;
        this.KeyDown += new KeyEventHandler(this.Form1_KeyDown);
        this.KeyPress += new KeyPressEventHandler(this.Form1_KeyPress);

        Label1 = new Label()
        {
            Text = "隨意按鍵。Ctrl+Back 或 Ctrl+F1 會彈出視窗",
            Location = new Point(0, 10),
            Font = new Font("微軟正黑體", 14),
            BackColor = Color.White,
            AutoSize = true,
            UseMnemonic = false
        };
        this.Controls.Add(Label1);

        Label2 = new Label()
        {
            Text = "keydown",
            Location = new Point(0, 40),
            Font = new Font("微軟正黑體", 14),
            BackColor = Color.White,
            AutoSize = true,
            UseMnemonic = false
        };
        this.Controls.Add(Label2);

        Label3 = new Label()
        {
            Text = "keypress",
            Location = new Point(0, 70),
            Font = new Font("微軟正黑體", 14),
            BackColor = Color.White,
            AutoSize = true,
            UseMnemonic = false
        };
        this.Controls.Add(Label3);
    }

    #endregion
}
