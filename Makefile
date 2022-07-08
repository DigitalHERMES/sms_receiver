all: http_server https_server

http_server: server.c
	gcc -Wall -O3 server.c -o http_server

https_server: tls_server.c
	gcc -Wall -O3 tls_server.c -o https_server -lssl -lcrypto

.PHONY: clean
clean:
	rm -f http_server https_server
