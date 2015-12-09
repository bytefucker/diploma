all: info hdd memSeq memRand memMonitor serialTest

info:
	$(CC) info.c -o ../bin/info
	
hdd:
	$(CC) hdd.c -o ../bin/hdd	

memSeq:
	$(CC) -pthread memSeq.c -o ../bin/memSeq
	
memRand:
	$(CC) -pthread memRand.c -o ../bin/memRand

memMonitor:
	$(CC) memMonitor.c -o ../bin/memMonitor

serialTest:
	$(CC) -o ../bin/serialTest -lpthread -Wall -g serialTest.c

clean:
	rm -rf *.o
