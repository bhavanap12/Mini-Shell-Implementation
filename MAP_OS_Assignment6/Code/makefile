TARGET=pastry
CC=g++
HEADERS=header.h md5.h
FLAGS=-Wall -g
OBJS=lib.o main1.o md5.o hex.o quad.o

$(TARGET) : $(OBJS) $(HEADERS)
	$(CC) $(FLAGS) -o $@ $^ -lpthread
%.o : %.c 
	$(CC) $(FLAGS)  -c -o $@ $^ -lpthread

clean : 
	rm *.o $(TARGET)
