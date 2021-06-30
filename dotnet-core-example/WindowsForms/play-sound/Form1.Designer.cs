using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace Example
{
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
            this.ClientSize = new Size(320, 240);
            this.Text = "Form1";

            this.Shown += new EventHandler(this.Form1_Shown);

            this.Btn1 = new Button()
            {
                Location = new Point(10, 10),
                AutoSize = true,
                Text = "Play 1"
            };
            Btn1.Click += new EventHandler(this.BtnPlay_Click);
            this.Controls.Add(Btn1);

            this.Btn2 = new Button()
            {
                Location = new Point(10, 40),
                AutoSize = true,
                Text = "Play 2"
            };
            Btn2.Click += new EventHandler(this.BtnPlay_Click);
            this.Controls.Add(Btn2);

            this.Btn3 = new Button()
            {
                Location = new Point(10, 70),
                AutoSize = true,
                Text = "Play 3"
            };
            Btn3.Click += new EventHandler(this.BtnPlay_Click);
            this.Controls.Add(Btn3);

        }

        #endregion
    }
}

