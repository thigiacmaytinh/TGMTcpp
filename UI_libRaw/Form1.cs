using OpenCV;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace UI_libRaw
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
        }

        private void txt_inputFile_TextChanged(object sender, EventArgs e)
        {
            string filePath = txt_inputFile.Text;
            circle1.Visible = true;
            worker1.RunWorkerAsync(filePath);
        }

        private void worker1_DoWork(object sender, DoWorkEventArgs e)
        {
            string filePath = e.Argument as string;
            Bitmap bmp;
            if(ImgCodec.IsRawImage(filePath))
                bmp = ImgCodec.imread(filePath);
            else
                bmp = new Bitmap(filePath);

            e.Result = bmp;
        }

        private void worker1_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            circle1.Visible = false;

            Bitmap bmp = e.Result as Bitmap;
            if(bmp != null)
                pictureBox1.Image = bmp;
        }
    }
}
