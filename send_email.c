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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#include "process_sms.h"
#include "send_email.h"


bool send_email(char *from, char *dest, char *timestamp, int argc, char **argv, char *body)
{
    char rmail_cmd[BUFSIZE];
    char tmp_mail[BUFSIZE];
    FILE *email_body;

    sprintf(tmp_mail, "/tmp/smsreceiver.%d", getpid ());

    if (argc <= 2)
      return false;

    // write the email...
    email_body = fopen(tmp_mail, "w");
    fprintf(email_body, "Subject: HERMES SMS Sytem\r\n");
    fprintf(email_body, "From: rafael2k@hermes.radio\r\n");
    fprintf(email_body, "To: rafael@riseup.net\r\n");
    fprintf(email_body, "To: rafael@riseup.net\r\n");
    fprintf(email_body, "\r\n");

    fprintf(email_body, "SMS de %s, em %s\n", from, timestamp);
    fprintf(email_body, "%s\n", body);

    fclose(email_body);

    sprintf(rmail_cmd, "(rmail ");


    for (int i = 2; i < argc; i++)
    {
        strcat (rmail_cmd, argv[i]);
        strcat (rmail_cmd, " ");
    }
    sprintf (rmail_cmd+strlen(rmail_cmd), "< %s)", tmp_mail);

    printf("Running: %s\n", rmail_cmd);

    system(rmail_cmd);

    unlink(tmp_mail);

    return true;
}
