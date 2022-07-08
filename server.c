/*

  Rhizomatica SMS receiver sample

 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define BUFSIZE 4096

void urldecode(char *dst, char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}


void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{    
    int parentfd;
    int childfd;
    int portno;
    unsigned int clientlen;
    char *hostaddrp;
    int optval;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    
    FILE *stream;
    char buf[BUFSIZE];
    char method[BUFSIZE];
    char uri[BUFSIZE];
    char version[BUFSIZE];

    bool is_sms = false;
    char *char_ptr = NULL;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);
    
    /* open socket descriptor */
    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0)
        error("ERROR opening socket");
    
    /* allows us to restart server immediately */
    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval , sizeof(int));
    
    /* bind port to socket */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    if (bind(parentfd, (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");
    
    /* get us ready to accept connection requests */
    if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
        error("ERROR on listen");
    
    /*
     * main loop: wait for a connection request, parse HTTP,
     * serve requested content, close connection.
     */
    clientlen = sizeof(clientaddr);
    while (1) {
        
        /* wait for a connection request */
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
        if (childfd < 0)
            error("ERROR on accept");
        
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            error("ERROR on inet_ntoa\n");
        
        /* open the child socket descriptor as a stream */
        if ((stream = fdopen(childfd, "r+")) == NULL)
            error("ERROR on fdopen");
        
        /* get the HTTP request line */
        fgets(buf, BUFSIZE, stream);
        printf("%s", buf);
        sscanf(buf, "%s %s %s\n", method, uri, version);
        
        if (strcasecmp(method, "GET")) {
            fprintf(stream, "HTTP/1.1 400 Bad Request\n");
            fprintf(stream, "\r\n");
            fclose(stream);
            close(childfd);
            continue;
        }

        is_sms = false;
        /* read the HTTP headers */
        fgets(buf, BUFSIZE, stream);
        char_ptr = strstr(buf, "Nexmo/MessagingHUB/v1.0");
        if (char_ptr != NULL)
            is_sms = true;
        printf("%s", buf);
        while(strcmp(buf, "\r\n")) {
            fgets(buf, BUFSIZE, stream);
            char_ptr = strstr(buf, "Nexmo/MessagingHUB/v1.0");
            if (char_ptr != NULL)
                is_sms = true;
            printf("%s", buf);
        }

        int index;
        if (is_sms){
            printf("Is SMS!\n");
            printf("URI = %s\n\n", uri);

            // get msisdn (aka: from)
            char_ptr =  strstr(uri, "?msisdn=") + strlen("?msisdn=");
            char from[BUFSIZE];
            index = -1;
            while (char_ptr[++index] != '&')
                from[index] = char_ptr[index];
            from[index] ='\n'; from[index + 1] = 0;

            printf("MSISDN (FROM) = %s\n", from);

            // get to
            char_ptr =  strstr(uri, "&to=") + strlen("&to=");
            char dest[BUFSIZE];
            index = -1;
            while (char_ptr[++index] != '&')
                dest[index] = char_ptr[index];
            dest[index] ='\n'; dest[index + 1] = 0;
            printf("To = %s\n", dest);

            // get messageId
            char_ptr =  strstr(uri, "&messageId=") + strlen("&messageId=");
            char messageId[BUFSIZE];
            index = -1;
            while (char_ptr[++index] != '&')
                messageId[index] = char_ptr[index];
            messageId[index] ='\n'; messageId[index + 1] = 0;

            printf("messageId = %s\n", messageId);

            // get message
            char_ptr =  strstr(uri, "&text=") + strlen("&text=");
            char message[BUFSIZE];
            index = -1;
            while (char_ptr[++index] != '&')
                message[index] = char_ptr[index];
            message[index] ='\n'; message[index + 1] = 0;
            printf("message raw = %s\n", message);

            // decode message
            char message_dec[BUFSIZE];
            urldecode(message_dec, message);
            printf("Message Decoded = %s\n", message_dec);
        }

        /* print response in case of success */
        // fprintf(stream, "HTTP/1.1 200 OK\n");
        fprintf(stream, "HTTP/1.1 204 No Content\n");
        // fprintf(stream, "Server: HERMES SMS\n");
        fprintf(stream, "\r\n");
        fflush(stream);

        fclose(stream);
        close(childfd);
    }
        
    /* clean up */
    fclose(stream);
    close(childfd);
    close(parentfd);
}

#if 0
URI = /?msisdn=5511998728111&to=5511953259200&messageId=28000000054225B9&concat=true&concat-ref=222&concat-total=3&concat-part=1&text=Siiiiiiim+senhor+meu+patr%C3%A3o+de+longa+data+vamos+fazer+uma+mensagem+&type=unicode&keyword=SIIIIIIIM&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A36

MSISDN (FROM) = 5511998728111To = 5511953259200messageId = 28000000054225B9message raw = Siiiiiiim+senhor+meu+patr%C3%A3o+de+longa+data+vamos+fazer+uma+mensagem+Message Decoded = Siiiiiiim senhor meu patrão de longa data vamos fazer uma mensagem

GET /?msisdn=5511998728111&to=5511953259200&messageId=28000000054225DB&concat=true&concat-ref=222&concat-total=3&concat-part=2&text=at%C3%A9+que+vem+longuinha+mesmo+ser%C3%A1+que+vain123445677889+aasssss+looon&type=unicode&keyword=AT%C3%89&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A40 HTTP/1.1
Accept: */*
User-Agent: Nexmo/MessagingHUB/v1.0
Host: hermes.radio:12345

Is SMS!
URI = /?msisdn=5511998728111&to=5511953259200&messageId=28000000054225DB&concat=true&concat-ref=222&concat-total=3&concat-part=2&text=at%C3%A9+que+vem+longuinha+mesmo+ser%C3%A1+que+vain123445677889+aasssss+looon&type=unicode&keyword=AT%C3%89&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A40

MSISDN (FROM) = 5511998728111To = 5511953259200messageId = 28000000054225DBmessage raw = at%C3%A9+que+vem+longuinha+mesmo+ser%C3%A1+que+vain123445677889+aasssss+looonMessage Decoded = até que vem longuinha mesmo será que vain123445677889 aasssss looon

GET /?msisdn=5511998728111&to=5511953259200&messageId=26000000012C9274&concat=true&concat-ref=222&concat-total=3&concat-part=3&text=ga%C3%A9+ajdjduejdjdjeuudjdjjdjdjdjjj+23456777553224565324664335534&type=unicode&keyword=GA%C3%89&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A43 HTTP/1.1
Accept: */*
User-Agent: Nexmo/MessagingHUB/v1.0
Host: hermes.radio:12345

Is SMS!
URI = /?msisdn=5511998728111&to=5511953259200&messageId=26000000012C9274&concat=true&concat-ref=222&concat-total=3&concat-part=3&text=ga%C3%A9+ajdjduejdjdjeuudjdjjdjdjdjjj+23456777553224565324664335534&type=unicode&keyword=GA%C3%89&api-key=cd906872&message-timestamp=2022-07-08+20%3A38%3A43

MSISDN (FROM) = 5511998728111To = 5511953259200messageId = 26000000012C9274message raw = ga%C3%A9+ajdjduejdjdjeuudjdjjdjdjdjjj+23456777553224565324664335534sMessage Decoded = gaé ajdjduejdjdjeuudjdjjdjdjdjjj 23456777553224565324664335534s


#endif
