CC=g++
COPT=-shared -O3 -Wall -fPIC -o $@ $^

SRCS = $(shell ls | grep .cpp)

all: libQDFUtils.so

clean: 
	rm *.so 

libQDFUtils.so: $(SRCS)
	$(CC) $(COPT) 

