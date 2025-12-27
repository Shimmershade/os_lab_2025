all: tcp udp

tcp:
	gcc -o tcpclient tcpclient.c
	gcc -o tcpserver tcpserver.c

udp:
	gcc -o udpclient udpclient.c
	gcc -o udpserver udpserver.c

clean:
	rm tcpclient
	rm tcpserver
	rm udpclient
	rm udpserver

.phony: all clean