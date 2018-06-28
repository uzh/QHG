#!/usr/bin/python

from sys import argv, exit
import os
import subprocess
import re

#-----------------------------------------------------------------------------
#--
#--
class PCAMaker:

    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, datadir, filebody, eigendir):
        self.datadir  = datadir
        self.filebody = filebody
        self.eigendir = eigendir
        self.outbody  = self.datadir + "/" + filebody
        self.outbody  = re.sub(("//"), "/", self.outbody)
        self.loclist  = []
        self.locstring = ""
        self.logfile  = open(self.outbody+".log", "wt")
        self.pcarr    = []
        self.sEvec2   = ""
    #-- end def


    #-------------------------------------------------------------------------
    #-- term
    #--
    def term(self):
        self.logfile.flush()
        self.logfile.close()
    #-- end def



    #-------------------------------------------------------------------------
    #-- writeParConvert
    #--
    def writeParConvert(self, sParConvert):
        iResult = 0
    
        try:
            f = open(sParConvert, "wt")
            f.write("genotypename:    %s.ped\n"  % (self.outbody)) 
            f.write("snpname:         %s.map\n"  % (self.outbody))
            f.write("indivname:       %s.ped\n"  % (self.outbody))
            f.write("outputformat:    EIGENSTRAT\n")
            f.write("genotypeoutname: %s_geno\n" % (self.outbody))
            f.write("snpoutname:      %s_snp\n"  % (self.outbody))
            f.write("indivoutname:    %s_ind\n"  % (self.outbody))
            f.write("familynames:     YES\n")
            f.close()
        except:
            print("Couldn't open [%s] for writing" % (sParConvert));
            iResult = -1;
        #-- end try
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- writeParSmartPCA
    #--
    def writeParSmartPCA(self, sParSmartPCA):
        iResult = 0

        try:
            f = open(sParSmartPCA, "wt")
            f.write("genotypename:    %s_geno\n" % (self.outbody))
            f.write("snpname:         %s_snp\n"  % (self.outbody)) 
            f.write("indivname:       %s_ind\n"  % (self.outbody)) 
            f.write("evecoutname:     %s_evec\n" % (self.outbody))
            f.write("evaloutname:     %s_eval\n" % (self.outbody))
            f.close()
        except:
            print("Couldn't open [%s] for writing" % (sParSmartPCA));
            iResult = -1;
        #-- end try
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- extractLocList
    #--
    def extractLocList(self):
        iResult = 0
        
        sPed = self.outbody+".ped"
        sPedTemp = self.outbody+".ped_temp"
        os.rename(sPed, sPedTemp)
        
        self.loclist=[]

        try:
            f1 = open(sPedTemp, "rt")
            f2 = open(sPed, "wt")
            for line in f1:
                line = re.sub(("_[0-9][0-9][0-9]"), "", line)
                f2.write(line)
                k = line.split()[0]
                
                if not k in self.loclist:
                    self.loclist.append(k)
                #-- end if
            #-- end for
            f1.close()
            f2.close()
            
            self.locstring = ':'.join(self.loclist)
            print("loclist:     [%s]" %(self.locstring))
            os.remove(sPedTemp)
        except:
            print("Couldn't open ped file [%s] or temp file [%s]" % (sPed, sPedTemp))
            iResult = -1
        #-- end try
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- fixEvec
    #--
    def fixEvec(self):
        iResult = 0
        sEvec  = self.outbody+"_evec"
        self.sEvec2 = self.outbody+"_evec2"

        try:
            f1 = open(sEvec,  "r")
            f2 = open(self.sEvec2, "w")

            for line in f1:
                name = line.split(":")[0]
                line = re.sub(("Control"), name, line)
                line = re.sub(("^ *"), "", line)

                f2.write(line)
                
            #-- end for
            f1.close()
            f2.close()
        except:
            print("troubles while correcting [%s]" % (self.sEvec2))
            iResult = -1
        #-- end try
        return iResult
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- jplotEig
    #--
    def jplotEig(self, bvisual):
        iResult = 0
        stem = self.sEvec2+self.pcarr[0]+":"+self.pcarr[1]
        d1 = str(1+int(self.pcarr[0]))
        d2 = str(1+int(self.pcarr[1]))
        self.gnuplotfile=self.outbody+"_"+"_".join(self.pcarr)+".gpl"
        sPNGFile = self.outbody+".pca.png"


        try:
            f = open(self.gnuplotfile, "wt")
            f.write("#\n")
            f.write("set terminal png size 1200,1000\n")
            f.write("set output \"%s\"\n" %(sPNGFile))
            f.write("set title \"%s\"\n" % (self.sEvec2))
            f.write("set key outside\n")
            f.write("set xlabel  \"PCA %s\"\n" % (self.pcarr[0]))
            f.write("set ylabel  \"PCA %s\"\n" % (self.pcarr[1]))

            iC = 0
            f.write("plot  ")
            for x in self.loclist:
                if (iC == 1):
                    f.write(", \\\n")
                else:
                    iC = 1
                #-- end if
                
                f.write(" \"%s:%s\" using %s:%s title \"%s\"" % (stem, x, d1, d2, x))
            #-- end for
            f.write("\n")
            if bvisual:
                f.write("\n")
                f.write("set terminal wxt size 1200,1000\n")
                f.write("replot")

                f.write("\n")
                f.write("print \"press 'Ctrl-C' to exit gnuplot\"\n")
            #-- end if
            f.close()
        except Exception as e:
            print("troubles while creating [%s]: %s" % (self.gnuplotfile, e))
            iResult = -1
        #-- end try
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- extractEvec
    #--
    def extractEvec(self):
        iResult = 0
        for x in self.loclist:
            sOutput = self.sEvec2+self.pcarr[0]+":"+self.pcarr[1]+":"+x
            try:
                fIn  = open(self.sEvec2, "rt")
                fOut = open(sOutput, "wt")
            except:
                print("Couldn't open [%s] or [%s]" % (self.sEvec2, sOutput))
                iResult = -1
                break
            else:
                for line in fIn:
                    if not re.match("^"+x+":", line) is None:
                        fOut.write(line)
                    #-- end if
                #-- end for
                fOut.close()
                fIn.close()
            #-- end try
        #-- end for
        return iResult
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- callConvert
    #--
    def callConvert(self):
        iResult = 0
        print("calling convertf ...")
        sParConvert =  self.datadir+"/par.convert"
        iResult = self.writeParConvert(sParConvert)
        if (iResult == 0):
            arglist=[self.eigendir+"/convertf", "-p", sParConvert]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist, stdout=self.logfile)
            #-- end if
        #-- end if
        return iResult
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- callSmartPCA
    #--
    def callSmartPCA(self):
        iResult = 0
        print("calling smartpca ...")
        sParSmartPCA =  self.datadir+"/par.smartpca"
        iResult = self.writeParSmartPCA(sParSmartPCA)
        if (iResult == 0):
            arglist=[self.eigendir+"/smartpca", "-p", sParSmartPCA]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist, stdout=self.logfile)
        #-- end if
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- makePCA
    #--
    def makePCA(self, pc1, pc2, bvisual):
        iResult = 0
        try:
            # they must be ints
            ip1= int(pc1)
            ip2= int(pc2)
            # but we use them as strings
            self.pcarr=[str(pc1), str(pc2)]
        except:
            print("pc1 [%s] and pc2 [%s] must be numbers" % (pc1, pc2))
            
            iResult = -1
        else:
            # get the list of locations from the ped file
            if iResult == 0:
                iResult = self.extractLocList()
            #-- end if

            # convert ped file to eigen format
            if iResult == 0:
                iResult = self.callConvert()
            #-- end if
            
            # call smart pca to calculate pca
            if iResult == 0:
                iResult = self.callSmartPCA()
            #-- end if
            
            # fix the entries in the evec file: add locations to end of line
            if iResult == 0:
                iResult = self.fixEvec()
            #-- end if

            # create one evec file for each location
            if iResult == 0:
                iResult = self.extractEvec()
            #-- end if

            # create a gnuplot file (creates a png file)
            if iResult == 0:
                iResult = self.jplotEig(bvisual)
            #-- end if
        #-- end try
        return iResult
    #-- end def

#-- end class
