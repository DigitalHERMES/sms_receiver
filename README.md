# sms_receiver


Simple SMS receiver (Nexmo provider supported) and e-mail sender which uses SMS
data.

Both binaries are created with make: http_server and https_server, which
have roughly the same functionality, and the following parameters:

 $ ./https_server <port> <dest_mail1> <dest_mail2> ...

Add cert.pem and key.pem in the local directory for the https_server to work.



rafael@rhizomatica.org
