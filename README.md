# TCP Log System
* Created by Bernardo Meneghini, Lucas Rocha and Thiago Alexandre in 09/21/2017.
* This program sends local logs from a client to the server based of some given expression.
You can use and modify the source code as you pleased.

# Compile
* gcc server-side.c -o server
* gcc client-side.c -o client

# Running
* ./server
* ./client -i 127.0.0.1 -f logs.txt -e expression
