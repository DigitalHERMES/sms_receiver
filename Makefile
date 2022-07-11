all: http_server https_server

http_server: server.c process_sms.c process_sms.h
	gcc -Wall -O3 server.c process_sms.c -o http_server

https_server: tls_server.c process_sms.c process_sms.h
	gcc -Wall -O3 tls_server.c process_sms.c -o https_server -lssl -lcrypto

.PHONY: clean
clean:
	rm -f http_server https_server
