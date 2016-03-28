##UDTWrapper

This is a tcp wrapper of udt. It have two binary program named client and server to convert a tcp connection to udt connection.

local <- tcp -> client <- udt -> server <- tcp -> remote

udt is a reliable UDP based application level data transport protocol. You can find it in [here](http://udt.sourceforge.net/)

###Require

Linux Only. For I use epoll.

UDT compatibility required.

###How to make.

* use git submodule to get udt4 source.
* cd udt4-fork and make
* use cmake to make udtwrapper

###How to use

use ssh as example

* user : ssh 127.0.0.1 -p 10022
* client : ./client -a 127.0.0.1 -b 10022 -c 1.1.1.1 -p 10022
* server : ./server -a 1.1.1.1 -b 10022 -c 127.0.0.1 -p 22


