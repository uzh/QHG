#----------------------------------------------------
# showDendrogram
#   show dendrogram for the genomes given in 'filename'
#   this currently only works for ped files
#   nrows: number of samples
#   ncols: number of nucleotides + number of header entries (6)
#   ngroups: number of clusters (to color)
#   type: 'ped'
#   bEdgeCol: TRUE-> use edge coloring
# IMPORTANT: make sure pacckape "ape" is loaded!
# IMPORTANT: the names of the ped-file entries should all differ
#
showDendrogram<-function(filename, nrows, ncols, ngroups, tMethod="complete", format="ped", tTreeType="rectangle", bEdgeCol=TRUE) {
  df= matrix(scan(filename, what=character()), nrows, ncols, byrow=TRUE)
  # split into header part and data part (this is specific for ped)
  genomes=df[,7:ncols]
  headers=df[,1:6]
  # reorder nucleotides to vector of 2-char strings
  genomesG=matgroup(genomes, format)
  # set the names of the samples
  dimnames(genomesG)<-list(headers[,1])
  # calculate the cluster data
  kk<-dist.gene(genomesG)
  hc<-hclust(kk, tMethod)
  # interpret cluster data as dendrogram
  hcd<-as.dendrogram(hc)
  # create as many colors as needed
  labelColors<-calcColor2(0)
  for (i in (1:(ngroups-1))) {
    labelColors<-c(labelColors, calcColor2((i*1.0)/ngroups))
  }
  cat("LC ")
  print(labelColors)
  cat("NG ")
  print(ngroups)
  # create clusters
  clusMember<-cutree(hc, ngroups)
  # apply colors to labls
  clusDendro<-dendrapply(hcd, colLab, clusMember, labelColors)
  if (bEdgeCol) {
    clusDendro2<-dendrapply(clusDendro, edgeColors)
  } else {
    clusDendro2=clusDendro
  }
  # set small font size
  par(cex=0.5)
  # show it
  plot(clusDendro2, type=tTreeType)
  clusDendro2
}

#----------------------------------------------------
# sortpair
#   sort a 2-character string alphabetically
#
sortpair <- function(x) {
  if  (substring(x,1,1) > substring(x,2,2)) 
    paste(substring(x,2,2), substring(x,1,1),sep="") 
  else 
    x
}

#----------------------------------------------------
# pairgroup
#   rearranges the nucleotides such that
#   corresponding nucleotides of the two strands
#   are in the same column of the 2xN matrix
#
pairgroup <- function(m, format) {
  L=dim(m)[2]
  if (format == "std") {
    matrix(paste(m[,1:(L/2)],m[,(L/2+1):L],sep=""),nrow=dim(m)[1], ncol=L/2,byrow=FALSE)
  } else if (format == "ped") {
    matrix(paste(m[,1+2*(0:(L/2-1))],m[,2*(1:(L/2))],sep=""),nrow=dim(m)[1], ncol=L/2,byrow=FALSE)
  }
}

#----------------------------------------------------
# matgroup
#   creates a vector of 2-char strings, consisting
#   of the pair of corresponding nucleotides.
#   format can be "ped" (corresponding Nucleotides are together) 
#   or "std" (second strand follows the first strand)
#
matgroup <- function(m, format) {
  m1<-pairgroup(m, format)
  v<-as.vector(m1)
  u<-"aa"
  k=matrix(vapply(v, sortpair, u),nrow=dim(m1)[1],ncol=dim(m1)[2])
  q<-as.character(k)
  dim(q)=dim(k)
  q
}

#----------------------------------------------------
# calcColor
#  calculate a linear color from the rainbow 
#  (0->R; 1/3->G, 2/3->B)
#
calcColor <- function(t) {
  print(t)
  if (t < 0) {
    r=0
    g=0
    b=0
  } else if ( t < 1.0/6.0) {
    r=1.0
    g=6.0*t
    b=0.0
  } else if ( t < 2.0/6.0) {
    r=1.0-6*(t-1.0/6.0)
    g=1.0
    b=0.0
  } else if ( t < 3.0/6.0) {
    r=0.0
    g=1.0
    b=6.0*(t-2.0/6.0)
  } else if ( t < 4.0/6.0) {
    r=0.0
    g=1.0-6*(t-3.0/6.0)
    b=1.0
  } else if ( t < 5.0/6.0) {
    r=6.0*(t-4.0/6.0)
    g=0.0
    b=1.0
  } else if ( t < 1.0) {
    r=1.0
    g=0.0
    b=1-6*(t-5.0/6.0)
  } else {
    r=1
    g=1
    b=1
  }
  col=sprintf("#%02X%02X%02X", floor(255*r), floor(255*g), floor(255*b))
#  col=rainbow(20)[ceiling(t*20)]
  col  
} 

#----------------------------------------------------
# calcColor
#  calculate a linear color from the rainbow 
#  (0->R; 1/3->G, 2/3->B)
#
calcColor2 <- function(t) {
  print(t)
  if (t < 0) {
    r=0
    g=0
    b=0
  } else if ( t < 1.0/6.0) {
    r=1.0
    g=6.0*t
    b=0.0
  } else if ( t < 2.0/6.0) {
    r=1.0-6*(t-1.0/6.0)
    g=1.0
    b=0.0
  } else if ( t < 3.0/6.0) {
    r=0.0
    g=1.0
    b=6.0*(t-2.0/6.0)
  } else if ( t < 4.0/6.0) {
    r=0.0
    g=1.0-6*(t-3.0/6.0)
    b=1.0
  } else if ( t < 5.0/6.0) {
    r=6.0*(t-4.0/6.0)
    g=0.0
    b=1.0
  } else if ( t < 1.0) {
    r=1.0
    g=0.0
    b=1-6*(t-5.0/6.0)
  } else {
    r=1
    g=1
    b=1
  }
  col=sprintf("#%02X%02X%02X", floor(220*r), floor(220*g), floor(220*b))
  #  col=rainbow(20)[ceiling(t*20)]
  col  
} 

#----------------------------------------------------
# colLab
#   to be used with dendrapply()
#   apply color to the labels of a cluster member
#   and add "edgePar" attributes to non-leafs
#
colLab <- function(n, cMember, labelCols) {
  if (is.leaf(n)) {
    a <- attributes(n)
    
    labCol <- labelCols[cMember[which(names(cMember) == a$label)]]
    attr(n, "nodePar") <- c(a$nodePar, lab.col = labCol)
    attr(n, "edgePar") <- c(a$edgePar, col = labCol)

  } else {
    a <- attributes(n)
    attr(n, "edgePar") <- c(a$edgePar, col = "#000000")
  }
  n
}

#----------------------------------------------------
# edgeColors
#   to be used with dendrapply()
#   color edge from node n
#   with the color of its branches (if they are equal) 
#   or with black
#   colLab must have been run beforehand
#
edgeColors<-function(n) {
  if (!is.leaf(n)) {
    c0 = edgeCheck(n)
    attr(n, "edgePar") <- c(c0)
    if (c0 == "#000000") {
      attr(n, "edgePar") <- c(c0, lty="dotted")
    }
  }
  n
}

#----------------------------------------------------
# edgeCheck
#   determine the color to be used for an edge:
#   if node is a leaf use its color for the outgoing edge
#   otherwise check colors of the two childrens' edges
#   if the y are equal use this color, otherwise black
#
edgeCheck<-function(n) {
  a<-attributes(n)
  if (is.leaf(n)) {
    c0<-a$edgePar
    
    c0
  } else {

    rn<-n[[1]]
    ln<-n[[2]]
    
    cr<-edgeCheck(rn)
    cl<-edgeCheck(ln)
    
    if (cr==cl) {
      c0 = cl
    } else {
      c0 ="#000000"
    }
    c0
  }
}
