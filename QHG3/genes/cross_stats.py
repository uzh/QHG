from sys import argv



f = open(argv[1], "r")
x=[]
for l in f:
    a=l.split()
    x.extend(a)
y=[float(a) for a in x]
print("Num: %f" % (len(y)))
print("Avg: %f" % (sum(y)/len(y)))
print("Min: %f" % (min(y)))
print("Max: %f" % (max(y)))
      
