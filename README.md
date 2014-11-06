udtwrapper
==========


A warpper from TCP to UDT.

You need udt library be installed.

Usage:

    ./server.out listen_port connect_ip_address connect_ip_port
    ./client.out listen_port connect_ip_address connect_ip_port
    
Example:

Server: Have a socks5 server listen on port 5555. so run `./server.out 10000 127.0.0.1 5555` on server.

Client: run `./client.out 10000 server_ip 10000`. Then set broswer's socks5 to 127.0.0.1 10000


TODO List:

* -IPv6 Support.- (Worked. add -6 parameter follow client.out and server.out)
* Client Auto Reconnect.
* Encrypt Support.

Bug To Fix:

* Sometime server create socket use socket(XXX,XXX,0) failed. No idea why.
