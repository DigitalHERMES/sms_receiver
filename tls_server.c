/*

  Rhizomatica SMS receiver sample

 */


// Based on: https://wiki.openssl.org/index.php/Simple_TLS_Server

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFSIZE 4096

int sock;
SSL_CTX *ctx;

static volatile bool keepRunning = false;

// Ctrl+C graceful exit

void intHandler(int dummy)
{
    if (keepRunning)
    {
        keepRunning = false;
        close(sock);
        SSL_CTX_free(ctx);
    }

    exit(EXIT_SUCCESS);
}

int create_socket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0)
    {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    char buf[BUFSIZE];
    char method[BUFSIZE];
    char uri[BUFSIZE];
    char version[BUFSIZE];

    bool is_sms = false;
    char *char_ptr = NULL;

    signal(SIGINT, intHandler);

    ctx = create_context();
    configure_context(ctx);

    sock = create_socket(12345);

    keepRunning = true;

    /* Handle connections */
    while(keepRunning)
    {
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        SSL *ssl;

        int client = accept(sock, (struct sockaddr*)&addr, &len);
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0)
        {
            ERR_print_errors_fp(stderr);
        } else
        {
            /* get the HTTP request line */
            int bytes_read = SSL_read(ssl, buf, BUFSIZE);
            if (bytes_read <= 0)
                goto bail;
            printf("%s\n", buf);
            printf("bytes read: %d\n",bytes_read);
            sscanf(buf, "%s %s %s\n", method, uri, version);

            printf("method: %s, uri: %s, version: %s\n",method, uri, version);

            if (strcasecmp(method, "GET")) {
                char *reply = "HTTP/1.1 400 Bad Request\n\r\n";
                SSL_write(ssl, reply, strlen(reply));
                printf("HERE!\n");
            }
            else
            {
                is_sms = false;

                if(!strcmp(buf, "\r\n"))
                    printf("got it!n\n");

                /* read the HTTP headers */
                int bytes_read = SSL_read(ssl, buf, BUFSIZE);
                if (bytes_read > 0)
                    printf("More bytes to read: %d\n",bytes_read);

                char *reply = "HTTP/1.1 204 No Content\n\r\n";
                SSL_write(ssl, reply, strlen(reply));

                printf("There!\n");
            }

        }

    bail:
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
}
