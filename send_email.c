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

// mail -s "teste" rafael2k@hermes.radio

#define SUBJECT "Sistema de SMS HERMES"
#define FROM "sms@hermes.radio"

bool send_email(char *from, char *dest, char *timestamp, int argc, char **argv, char *body)
{
    char mail_cmd[BUFSIZE];
    FILE *email_body;

    if (argc <= 2)
      return false;

    strcat (mail_cmd, "(mail -r ");
    strcat (mail_cmd, FROM);
    strcat (mail_cmd, " -s \"");
    strcat (mail_cmd, SUBJECT);
    strcat (mail_cmd, "\" -a \"Content-Type: text/plain; charset=UTF-8\" ");


    for (int i = 2; i < argc; i++)
    {
        strcat (mail_cmd, argv[i]);
        strcat (mail_cmd, " ");
    }
    mail_cmd[strlen(mail_cmd)] = ')';

    printf("Running: %s\n", mail_cmd);

    email_body = popen(mail_cmd, "w");
    fprintf(email_body, "Mensagem enviada em %s, de %s:\n", timestamp, from);
    fprintf(email_body, "%s", body);
    pclose(email_body);

    return true;
}
