#!/usr/bin/python

from sys import argv
import os.path
import glob
import re

import Tkinter as tk
import ttk
import tkMessageBox
import tkFileDialog
import PCOCreator

class PCOFront():

    CAPTION_QHG_DIR  = "QHG Directory"
    CAPTION_QDF_DIR  = "QDF Directory"
    CAPTION_LOG_DIR  = "Log Directory"
    CAPTION_POP_PAT  = "Pop Pattern"
    CAPTION_ZIPPED   = "Zipped"
    CAPTION_STAT_PAT = "Stat Pattern"
    CAPTION_TIMES    = "Times"
    CAPTION_OUT_DIR  = "Out Directory"
    CAPTION_OUT_PREF = "Out Prefix"

    CAPTION_BUT_BROWSE = "browse"
    CAPTION_BUT_FIND   = "find all"
    CAPTION_BUT_START  = "start"
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, defQHG, defQDF, defLog, defPopPat, defStatPat, defOutDir, defOutPrefix):
        self.root = tk.Tk()
        self.sQHGDir    = tk.StringVar()
        self.sQDFDir    = tk.StringVar()
        self.sLogDir    = tk.StringVar()
        self.sPopPat    = tk.StringVar()
        self.sStatPat   = tk.StringVar()
        self.sTimes     = tk.StringVar()
        self.sOutDir    = tk.StringVar()
        self.sOutPrefix = tk.StringVar()
        self.bZipped    = tk.IntVar()

        self.sStatus = tk.StringVar()

        #-- default valus for the Entry elements
        self.sQHGDir.set(defQHG)
        self.sQDFDir.set(defQDF)
        self.sLogDir.set(defLog)
        self.sPopPat.set(defPopPat)
        self.sStatPat.set(defStatPat)
        self.sOutDir.set(defOutDir)
        self.sOutPrefix.set(defOutPrefix)
        self.bZipped.set(0)
        self.sStatus.set("  Right-click on any label for help   ---   press ESC to exit")


        #-- adding all widgets
        rownum = 0
        #--- QHG Dir
        self.txtQHGDir = self.addEntryLine(self.sQHGDir, self.CAPTION_QHG_DIR, self.helpQHGDir, rownum, self.browseQHG)

        
        rownum = rownum + 1
        #--- QDF Dir
        self.txtQDFDir = self.addEntryLine(self.sQDFDir, self.CAPTION_QDF_DIR, self.helpQDFDir, rownum, self.browseQDF)
        
        rownum = rownum + 1
        #--- Log Dir
        self.txtLogDir = self.addEntryLine(self.sLogDir, self.CAPTION_LOG_DIR, self.helpLogDir, rownum, self.browseLog)

        rownum = rownum + 1
        #-- pop pattern
        self.txtPopPat = self.addEntryLine(self.sPopPat, self.CAPTION_POP_PAT, self.helpPopPat, rownum)

        #-- zipped
        self.chkZip = ttk.Checkbutton(self.root, text=self.CAPTION_ZIPPED, variable=self.bZipped)
        self.chkZip.grid(column=4, columnspan=1, row=rownum, rowspan=1,sticky=tk.W)
        self.chkZip.bind('<Button-3>', self.helpZip)

        rownum = rownum + 1
        #-- stat pattern
        self.txtStatPat = self.addEntryLine(self.sStatPat, self.CAPTION_STAT_PAT, self.helpStatPat, rownum)

        rownum = rownum + 1
        #-- times
        self.txtTimes = self.addEntryLine(self.sTimes, self.CAPTION_TIMES, self.helpTimes, rownum)

        self.butLoadTimes = ttk.Button(self.root, text=self.CAPTION_BUT_FIND, command = self.readTimes)
        self.butLoadTimes.grid(column=4, columnspan=1, row=rownum, rowspan=1,sticky=tk.E)

        rownum = rownum + 1
        #-- output dir
        self.txtOutDir = self.addEntryLine(self.sOutDir, self.CAPTION_OUT_DIR, self.helpOutDir, rownum, self.browseOutDir)

        rownum = rownum + 1
        #-- output prefix
        self.txtOutPrefix = self.addEntryLine(self.sOutPrefix, self.CAPTION_OUT_PREF, self.helpOutPrefix, rownum)

        
        rownum = rownum + 1
        #-- separator
        sep1 = ttk.Separator(self.root, orient=tk.HORIZONTAL)
        sep1.grid(column=0, columnspan=5, row=rownum, rowspan=1, sticky=tk.W+tk.E,pady=5)

        #-- Start button
        rownum = rownum + 1
        self.butOK = ttk.Button(self.root, text=self.CAPTION_BUT_START, command = self.startAction)
        self.butOK.grid(column=4, columnspan=1, row=rownum, rowspan=1, pady=8,sticky=tk.W+tk.E)

        #-- status label
        lblTemp = ttk.Label(self.root, text=self.sStatus.get(), textvariable=self.sStatus, relief="sunken")
        lblTemp.grid(column=0, columnspan=4, row=rownum, rowspan=1,sticky=tk.W+tk.E+tk.N+tk.S,pady=5,padx=5)
        lblTemp.bind_all('<Key-Escape>', self.closeApp)

        self.root.wm_title("Create PCO")
        self.root.mainloop()

    #-- ende def


    #---------------------------------------------------------------------
    #-- addEntryLine
    #-- 
    def addEntryLine(self, variable, caption, help_callback, rownum, browse_callback=None):
        lblTemp = ttk.Label(self.root, text=caption)
        lblTemp.grid(column=0, columnspan=1, row=rownum, rowspan=1,sticky=tk.W)
        lblTemp.bind('<Button-3>', help_callback)

        txtTemp = ttk.Entry(self.root, textvariable=variable, width=40)
        txtTemp.grid(column=1, columnspan=3, row=rownum, rowspan=1, sticky=tk.W+tk.E,padx=2,pady=2)

        if not browse_callback is None:
            butTemp = ttk.Button(self.root, text=self.CAPTION_BUT_BROWSE, command=browse_callback)
            butTemp.grid(column=4, columnspan=3, row=rownum, rowspan=1,sticky=tk.W+tk.E,pady=1)
        #-- end def
        return txtTemp
    #-- end def
    
        
    #---------------------------------------------------------------------
    #-- browseQDF
    #-- 
    def browseQDF(self):
        s0=self.sQDFDir.get()
        if (s0  == ""):
            s0 = os.getcwd()
        #-- end if
        s=tkFileDialog.askdirectory(title="select QDF directory", initialdir=s0)
        self.sQDFDir.set(s)
    #-- end def

    
    #---------------------------------------------------------------------
    #-- helpQDFDir
    #-- 
    def helpQDFDir(self, event):
        tkMessageBox.showinfo(self.CAPTION_QDF_DIR, "Select the directory containing theQDF files to be analyzed");
    #-- end def

    
    #---------------------------------------------------------------------
    #-- browseQHG
    #--
    def browseQHG(self):
        s0=self.sQHGDir.get()
        if (s0  == ""):
            s0 = os.getcwd()
        #-- end if
        s=tkFileDialog.askdirectory(title="select QHG directory", initialdir=s0)
        self.sQHGDir.set(s)
    #-- end def


    #---------------------------------------------------------------------
    #-- helpQHGDir
    #-- 
    def helpQHGDir(self, event):
        tkMessageBox.showinfo(self.CAPTION_QHG_DIR, "Select the root directory of your QHG installation");
    #-- end def

    
    #---------------------------------------------------------------------
    #-- browseLog
    #--
    def browseLog(self):
        s0=self.sLogDir.get()
        if (s0  == ""):
            s0 = os.getcwd()
        #-- end if
        s=tkFileDialog.askdirectory(title="select directory containing log files", initialdir=s0)
        self.sLogDir.set(s)
    #-- end def


    #---------------------------------------------------------------------
    #-- helpLogDir
    #-- 
    def helpLogDir(self, event):
        tkMessageBox.showinfo(self.CAPTION_LOG_DIR, "Select the directory into which the simulation's .log and .out files have been written")
    #-- end def
    

    #---------------------------------------------------------------------
    #-- browseOutDir
    #--
    def browseOutDir(self):
        s0=self.sOutDir.get()
        if (s0  == ""):
            s0 = os.getcwd()
        #-- end if
        s=tkFileDialog.askdirectory(title="select Output directory", initialdir=s0)
        self.sOutDir.set(s)
    #-- end def


    #---------------------------------------------------------------------
    #-- helpPopPat
    #-- 
    def helpPopPat(self, event):
        tkMessageBox.showinfo("PopPat", "Enter the pattern of the population qdf files.\nUse '#' as wildcard for a digit")
    #-- end def


    #---------------------------------------------------------------------
    #-- helpStatPat
    #-- 
    def helpStatPat(self, event):
        tkMessageBox.showinfo(self.CAPTION_POP_PAT, "Enter the pattern of the qdf files containing move statistics.\nUse '#' as wildcard for a digit")
    #-- end def


    #---------------------------------------------------------------------
    #-- helpZip
    #-- 
    def helpZip(self, event):
        tkMessageBox.showinfo(self.CAPTION_ZIPPED, "Click this checkbox if the qdf files are zipped")
    #-- end def
    

    #---------------------------------------------------------------------
    #-- helpTimes
    #-- 
    def helpTimes(self, event):
        tkMessageBox.showinfo(self.CAPTION_TIMES, "Enter the space separateed number values for the files to be analyzed,\nor press 'find all' to enter all available number values.\nThe numbers should correspond to the wildcard pattern entered under 'PopPattern'")
    #-- end def
    

    #---------------------------------------------------------------------
    #-- helpOutDir
    #-- 
    def helpOutDir(self, event):
        tkMessageBox.showinfo(self.CAPTION_OUT_DIR, "Select the directory into which the analysis results should be written. To create a new directory, select the parent directory and add the name in the dialog's text entry")
    #-- end def
    

    #---------------------------------------------------------------------
    #-- helpOutPrefix
    #-- 
    def helpOutPrefix(self, event):
        tkMessageBox.showinfo(self.CAPTION_OUT_PREF, "Enter a prefix for the output file names")
    #-- end def
    

    #---------------------------------------------------------------------
    #-- closeApp
    #-- 
    def closeApp(self, event):
        self.root.quit()
    #-- end def
    

    #---------------------------------------------------------------------
    #-- readTimes
    #--
    def readTimes(self):
        sTimes = self.getTimes()
        self.sTimes.set(sTimes)
    #-- end def


    #---------------------------------------------------------------------
    #-- getTimes
    #--
    def getTimes(self):
        sTimes = ""
        suffix = ".gz" if self.bZipped.get() else ""
        sQDFDir = self.sQDFDir.get()
        if os.path.exists(sQDFDir) and os.path.isdir(sQDFDir):
            i0Pop=self.sPopPat.get().find('#')                  
            i1Pop=self.sPopPat.get().rfind('#')                  

            sPopPat  = self.sPopPat.get().replace("#", "[0-9]")+suffix
            print("PopPat:[%s]" % (sPopPat))
            poplist = glob.glob(sQDFDir + '/' + sPopPat)
            poplist.sort()
            print("Found %d matches" % (len(poplist)))

            times = []
            p=re.compile(sPopPat)
            for n in poplist:
                
                s=n.split('/')[-1]
                times.append(s[i0Pop:i1Pop+1])
            #-- end for
            sTimes = " ".join(times)

        else:
            if os.path.exists(sQHGDir):
                tkMessageBox.showerror("Error ", "Path doesn't exist:\n%s"%(sQDFDir))
            elif os.path.exists(sQDFDir):
                tkMessageBox.showerror("Error ", "Path doesn't exist:\n%s"%(sQHGDir))
            else:
                tkMessageBox.showerror("Error ", "None of the paths exists")
            #-- end if
        #-- end if
        return sTimes
    #-- end def


    #---------------------------------------------------------------------
    #-- updateStatus
    #--
    def updateStatus(self, sNewStatus):
        self.sStatus.set("  "+sNewStatus)
        self.root.update_idletasks()
        self.root.update()
    #-- end def

    
    #---------------------------------------------------------------------
    #-- startAction
    #--
    def startAction(self):
       
        sQHGDir = self.sQHGDir.get()
        sQDFDir = self.sQDFDir.get()
        sTimes  = self.sTimes.get()
        if os.path.exists(sQHGDir) and os.path.isdir(sQDFDir):
            t = [x for x in sTimes.split(' ') if x.strip()]
            bOK = True
            for x in t:
                bOK = bOK and unicode(x, 'utf-8').isnumeric()
            #-- end if

            if bOK:
                i0Pop=self.sPopPat.get().find('#')                  
                i1Pop=self.sPopPat.get().rfind('#')                  
                n = i1Pop - i0Pop +1
                pat=""
                for i in range(n):
                    pat = pat + '#'
                #-- end for
                print("pat [%s]" %(pat))

                # do something
                try:
                                    
                    PCC = PCOCreator.PCOCreator(self.sQHGDir.get(),
                                                self.sQDFDir.get(),
                                                self.sPopPat.get(),
                                                self.sStatPat.get(),
                                                self.bZipped.get(),
                                                self.sLogDir.get(),  
                                                sTimes,
                                                self.sOutDir.get(),
                                                self.sOutPrefix.get())

                    self.updateStatus("creating output dirs")
                    PCC.createOuputDir()

                    if PCC.zipped:
                        self.updateStatus("unzipping qdfs")
                        PCC.gunzipFiles()
                    #-- end if

                    self.updateStatus("QDFSampler")
                    PCC.doQDFSampler()

                    self.updateStatus("BinGeneMerge2")
                    PCC.doBinGeneMerge2()

                    self.updateStatus("GeneDist")
                    PCC.doGeneDist()

                    self.updateStatus("CalcPCO")
                    PCC.doCalcPCO()

                    self.updateStatus("ColumnMerger")
                    PCC.doColumnMerger()

                    self.updateStatus("Attributes")
                    PCC.doAttributes()

                    self.updateStatus("Arrivals")
                    PCC.doArrivals()

                    self.updateStatus("copying logs")
                    PCC.doCopyLogs()

                    if PCC.zipped:
                        self.updateStatus("zipping qdfs")
                        PCC.gzipFiles()
                    #-- end if

                    self.updateStatus("success")

                except PCOCreator.QHGError as e:
                    print("\n%sQHGError: %s%s" % (COL_RED, e, COL_OFF))
                    self.updateStatus("error")
                except Exception as e:
                    print("exception: %s" %(e))
                    self.updateStatus("exception")
                #-- end try
            else:
                self.updateStatus("error")
                tkMessageBox.showerror("Error ", "'Times' contains non-numeric entries")
            #-- end if
            
        else:
            if os.path.exists(sQHGDir):
                tkMessageBox.showerror("Error ", "Path doesn't exist:\n%s"%(sQDFDir))
            elif os.path.exists(sQDFDir):
                tkMessageBox.showerror("Error ", "Path doesn't exist:\n%s"%(sQHGDir))
            else:
                tkMessageBox.showerror("Error ", "None of the paths exists")
            #-- end if
        #-- end if
        

#-- end class

#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':

    if len(argv) > 1:
        if argv[1]=="def":
        
            pf = PCOFront("/home/jody/progs/QHG3/trunk",
                          "/home/jody/progs/QHG3/trunk/data/africa3/muttest_b3_var",
                          "/home/jody/progs/QHG3/trunk/app",
                          "ooa_pop-Sapiens_ooa__###000.qdf",
                          "ooa_SGCVNM_###000.qdf",
                          "",
                          "analysis")
        else:
            pf = PCOFront("", "", "", "", "", "", "")

