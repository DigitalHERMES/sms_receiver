/*
 * Copyright (C) 2022 Rhizomatica <rafael@rhizomatica.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * SMS processor for Nexmo / Vonage
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "process_sms.h"

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


bool process_sms(char *uri)
{
    char *char_ptr = NULL;

    int index;

    printf("Is SMS!\n");
    printf("URI = %s\n\n", uri);

    // get msisdn (aka: from)
    char_ptr =  strstr(uri, "?msisdn=") + strlen("?msisdn=");
    if (char_ptr)
    {
        char from[BUFSIZE];
        for(index = 0; char_ptr[index] != '&'; index++)
            from[index] = char_ptr[index];
        from[index] ='\n'; from[index + 1] = 0;
        printf("MSISDN (FROM) = %s\n", from);
    }

    // get to
    char_ptr =  strstr(uri, "&to=") + strlen("&to=");
    if (char_ptr)
    {
        char dest[BUFSIZE];
        for(index = 0; char_ptr[index] != '&'; index++)
            dest[index] = char_ptr[index];
        dest[index] ='\n'; dest[index + 1] = 0;
        printf("To = %s\n", dest);
    }

    // get messageId
    char_ptr =  strstr(uri, "&messageId=") + strlen("&messageId=");
    if (char_ptr)
    {
        char messageId[BUFSIZE];
        for(index = 0; char_ptr[index] != '&'; index++)
            messageId[index] = char_ptr[index];
        messageId[index] ='\n'; messageId[index + 1] = 0;
        printf("messageId = %s\n", messageId);
    }

    char_ptr =  strstr(uri, "&messageId=") + strlen("&messageId=");
    if (char_ptr)
    {
        // get message
        char_ptr =  strstr(uri, "&text=") + strlen("&text=");
        char message[BUFSIZE];
        for(index = 0; char_ptr[index] != '&'; index++)
            message[index] = char_ptr[index];
        message[index] ='\n'; message[index + 1] = 0;
        printf("message raw = %s\n", message);

        // decode message
        char message_dec[BUFSIZE];
        urldecode(message_dec, message);
        printf("Message Decoded = %s\n", message_dec);
    }


            // remember to apply urldecode to time stamp
            // check if we need put apply it also to other fields
            // send mail
            // record a log que the timestamp

    return true;
}
