﻿using System;
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
            this.ClientSize = new Size(800, 600);
            this.Text = "Form1";
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.Shown += new EventHandler(this.Form1_Shown);
            this.Closing += new CancelEventHandler(this.Form1_Closing);

            Label1 = new Label()
            {
                Text = "應用程式關閉事件。",
                Location = new Point(10, 10),
                Size = new Size(800, 30),
                Font = new Font("微軟正黑體", 20),
                BackColor = Color.White,
                AutoSize = true,
                UseMnemonic = false
            };
            Label1.Click += new EventHandler(this.Label1_Click);
            this.Controls.Add(Label1);

            Button1 = new Button()
            {
                Location = new Point(10, 100),
                AutoSize = true,
                Text = "Close application. Or click X button of this Window."
            };
            Button1.Click += new EventHandler(this.Button1_Click);
            this.Controls.Add(Button1);
        }

        #endregion
    }
}