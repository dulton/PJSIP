namespace PJSIP
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
            if (disposing && (components != null))
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
            this.btnStart = new System.Windows.Forms.Button();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnAnswer = new System.Windows.Forms.Button();
            this.btnHangUp = new System.Windows.Forms.Button();
            this.btnPlay = new System.Windows.Forms.Button();
            this.btnDial = new System.Windows.Forms.Button();
            this.lblIncomingCall = new System.Windows.Forms.Label();
            this.lblDtmf = new System.Windows.Forms.Label();
            this.btnRecordCall = new System.Windows.Forms.Button();
            this.btnCreateConference = new System.Windows.Forms.Button();
            this.btnRemoveConference = new System.Windows.Forms.Button();
            this.lblStopRecording = new System.Windows.Forms.Label();
            this.cmbStatusCodes = new System.Windows.Forms.ComboBox();
            this.btnStopRecording = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnStart
            // 
            this.btnStart.Location = new System.Drawing.Point(13, 13);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(75, 23);
            this.btnStart.TabIndex = 0;
            this.btnStart.Text = "Start";
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(12, 368);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(75, 23);
            this.btnDisconnect.TabIndex = 1;
            this.btnDisconnect.Text = "Stop";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.btnStop_Click);
            // 
            // btnAnswer
            // 
            this.btnAnswer.Location = new System.Drawing.Point(13, 74);
            this.btnAnswer.Name = "btnAnswer";
            this.btnAnswer.Size = new System.Drawing.Size(75, 23);
            this.btnAnswer.TabIndex = 2;
            this.btnAnswer.Text = "Answer";
            this.btnAnswer.UseVisualStyleBackColor = true;
            this.btnAnswer.Click += new System.EventHandler(this.btnAnswer_Click);
            // 
            // btnHangUp
            // 
            this.btnHangUp.Location = new System.Drawing.Point(13, 132);
            this.btnHangUp.Name = "btnHangUp";
            this.btnHangUp.Size = new System.Drawing.Size(75, 23);
            this.btnHangUp.TabIndex = 3;
            this.btnHangUp.Text = "HangUp";
            this.btnHangUp.UseVisualStyleBackColor = true;
            this.btnHangUp.Click += new System.EventHandler(this.btnHangUp_Click);
            // 
            // btnPlay
            // 
            this.btnPlay.Location = new System.Drawing.Point(13, 103);
            this.btnPlay.Name = "btnPlay";
            this.btnPlay.Size = new System.Drawing.Size(75, 23);
            this.btnPlay.TabIndex = 4;
            this.btnPlay.Text = "Play";
            this.btnPlay.UseVisualStyleBackColor = true;
            this.btnPlay.Click += new System.EventHandler(this.btnPlay_Click);
            // 
            // btnDial
            // 
            this.btnDial.Location = new System.Drawing.Point(13, 43);
            this.btnDial.Name = "btnDial";
            this.btnDial.Size = new System.Drawing.Size(75, 23);
            this.btnDial.TabIndex = 5;
            this.btnDial.Text = "Dial";
            this.btnDial.UseVisualStyleBackColor = true;
            this.btnDial.Click += new System.EventHandler(this.btnDial_Click);
            // 
            // lblIncomingCall
            // 
            this.lblIncomingCall.AutoSize = true;
            this.lblIncomingCall.Location = new System.Drawing.Point(94, 18);
            this.lblIncomingCall.Name = "lblIncomingCall";
            this.lblIncomingCall.Size = new System.Drawing.Size(0, 13);
            this.lblIncomingCall.TabIndex = 6;
            // 
            // lblDtmf
            // 
            this.lblDtmf.AutoSize = true;
            this.lblDtmf.Location = new System.Drawing.Point(94, 79);
            this.lblDtmf.Name = "lblDtmf";
            this.lblDtmf.Size = new System.Drawing.Size(40, 13);
            this.lblDtmf.TabIndex = 7;
            this.lblDtmf.Text = "DTMF:";
            // 
            // btnRecordCall
            // 
            this.btnRecordCall.Location = new System.Drawing.Point(13, 161);
            this.btnRecordCall.Name = "btnRecordCall";
            this.btnRecordCall.Size = new System.Drawing.Size(75, 23);
            this.btnRecordCall.TabIndex = 8;
            this.btnRecordCall.Text = "RecordCall";
            this.btnRecordCall.UseVisualStyleBackColor = true;
            this.btnRecordCall.Click += new System.EventHandler(this.btnRecordCall_Click);
            // 
            // btnCreateConference
            // 
            this.btnCreateConference.Location = new System.Drawing.Point(6, 219);
            this.btnCreateConference.Name = "btnCreateConference";
            this.btnCreateConference.Size = new System.Drawing.Size(105, 23);
            this.btnCreateConference.TabIndex = 9;
            this.btnCreateConference.Text = "CreateConference";
            this.btnCreateConference.UseVisualStyleBackColor = true;
            this.btnCreateConference.Click += new System.EventHandler(this.btnCreateConference_Click);
            // 
            // btnRemoveConference
            // 
            this.btnRemoveConference.Location = new System.Drawing.Point(6, 248);
            this.btnRemoveConference.Name = "btnRemoveConference";
            this.btnRemoveConference.Size = new System.Drawing.Size(122, 23);
            this.btnRemoveConference.TabIndex = 10;
            this.btnRemoveConference.Text = "RemoveConference";
            this.btnRemoveConference.UseVisualStyleBackColor = true;
            this.btnRemoveConference.Click += new System.EventHandler(this.btnRemoveConference_Click);
            // 
            // lblStopRecording
            // 
            this.lblStopRecording.AutoSize = true;
            this.lblStopRecording.Location = new System.Drawing.Point(95, 166);
            this.lblStopRecording.Name = "lblStopRecording";
            this.lblStopRecording.Size = new System.Drawing.Size(129, 13);
            this.lblStopRecording.TabIndex = 11;
            this.lblStopRecording.Text = "Press * to Stop Recording";
            // 
            // cmbStatusCodes
            // 
            this.cmbStatusCodes.FormattingEnabled = true;
            this.cmbStatusCodes.Items.AddRange(new object[] {
            "302",
            "486",
            "503",
            "603"});
            this.cmbStatusCodes.Location = new System.Drawing.Point(98, 132);
            this.cmbStatusCodes.Name = "cmbStatusCodes";
            this.cmbStatusCodes.Size = new System.Drawing.Size(121, 21);
            this.cmbStatusCodes.TabIndex = 12;
            // 
            // btnStopRecording
            // 
            this.btnStopRecording.Location = new System.Drawing.Point(14, 190);
            this.btnStopRecording.Name = "btnStopRecording";
            this.btnStopRecording.Size = new System.Drawing.Size(97, 23);
            this.btnStopRecording.TabIndex = 13;
            this.btnStopRecording.Text = "StopRecording";
            this.btnStopRecording.UseVisualStyleBackColor = true;
            this.btnStopRecording.Click += new System.EventHandler(this.btnStopRecording_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(292, 416);
            this.Controls.Add(this.btnStopRecording);
            this.Controls.Add(this.cmbStatusCodes);
            this.Controls.Add(this.lblStopRecording);
            this.Controls.Add(this.btnRemoveConference);
            this.Controls.Add(this.btnCreateConference);
            this.Controls.Add(this.btnRecordCall);
            this.Controls.Add(this.lblDtmf);
            this.Controls.Add(this.lblIncomingCall);
            this.Controls.Add(this.btnDial);
            this.Controls.Add(this.btnPlay);
            this.Controls.Add(this.btnHangUp);
            this.Controls.Add(this.btnAnswer);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnStart);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.Button btnAnswer;
        private System.Windows.Forms.Button btnHangUp;
        private System.Windows.Forms.Button btnPlay;
        private System.Windows.Forms.Button btnDial;
        private System.Windows.Forms.Label lblIncomingCall;
        private System.Windows.Forms.Label lblDtmf;
        private System.Windows.Forms.Button btnRecordCall;
        private System.Windows.Forms.Button btnCreateConference;
        private System.Windows.Forms.Button btnRemoveConference;
        private System.Windows.Forms.Label lblStopRecording;
        private System.Windows.Forms.ComboBox cmbStatusCodes;
        private System.Windows.Forms.Button btnStopRecording;
    }
}

