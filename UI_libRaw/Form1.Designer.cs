namespace UI_libRaw
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if(disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.txt_inputFile = new TGMTcontrols.BrowseFile();
            this.circle1 = new TGMTcontrols.ProcessingControl();
            this.worker1 = new System.ComponentModel.BackgroundWorker();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Location = new System.Drawing.Point(32, 95);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(573, 343);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // txt_inputFile
            // 
            this.txt_inputFile.BackColor = System.Drawing.Color.Transparent;
            this.txt_inputFile.BackgroundColor = System.Drawing.Color.White;
            this.txt_inputFile.BorderColor = System.Drawing.Color.Red;
            this.txt_inputFile.Font = new System.Drawing.Font("Segoe UI", 10F);
            this.txt_inputFile.ForeColor = System.Drawing.SystemColors.WindowText;
            this.txt_inputFile.Location = new System.Drawing.Point(32, 30);
            this.txt_inputFile.Name = "txt_inputFile";
            this.txt_inputFile.Padding = new System.Windows.Forms.Padding(5);
            this.txt_inputFile.Pattern = "";
            this.txt_inputFile.Size = new System.Drawing.Size(573, 30);
            this.txt_inputFile.TabIndex = 1;
            this.txt_inputFile.TextChanged += new System.EventHandler(this.txt_inputFile_TextChanged);
            // 
            // circle1
            // 
            this.circle1.BackColor = System.Drawing.Color.Transparent;
            this.circle1.IndexColor = System.Drawing.Color.DeepSkyBlue;
            this.circle1.Interval = 50;
            this.circle1.Location = new System.Drawing.Point(612, 25);
            this.circle1.Name = "circle1";
            this.circle1.NCircle = 8;
            this.circle1.Others = System.Drawing.Color.LightGray;
            this.circle1.Radius = 4;
            this.circle1.Size = new System.Drawing.Size(40, 40);
            this.circle1.TabIndex = 2;
            this.circle1.Text = "processingControl1";
            this.circle1.Visible = false;
            // 
            // worker1
            // 
            this.worker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.worker1_DoWork);
            this.worker1.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.worker1_RunWorkerCompleted);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(686, 450);
            this.Controls.Add(this.circle1);
            this.Controls.Add(this.txt_inputFile);
            this.Controls.Add(this.pictureBox1);
            this.Name = "Form1";
            this.Text = "Example RAW image reader";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private TGMTcontrols.BrowseFile txt_inputFile;
        private TGMTcontrols.ProcessingControl circle1;
        private System.ComponentModel.BackgroundWorker worker1;
    }
}

