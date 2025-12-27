all: client server

client:
	gcc -pthread -o client client_new.c

server:
	gcc -pthread -o server server_new.c

clean:
	rm client
	rm server

.phony: all, clean