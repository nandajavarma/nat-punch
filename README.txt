
This folder contains a simple implementation of an introducer 
based scheme to let two peers, each behind NAT, talk to each 
other. There is a server, publicly available on the internet, 
which serves as introducer, telling the peers what the public 
address for each of them is; the peers will then talk to each 
other. The first such message in each direction may be 
discarded, but acts to set up a virtual channel between the 
two peers through their NAT.

For full description, see:
http://www.mindcontrol.org/~hplus/nat-punch.html

NOTE: This code is intended only to show the punch-through 
technique. It's not a sockets tutorial. It's not intended to 
be secure code. Please do not use this code in a production 
environment (although AFAIK, you can use the technique freely).


To build on a UNIX with GCC, type "make" which builds two 
executables: nat-client, and nat-server.

Start nat-server on a host that has a public internet address 
(which you should previously have set up in nat-reg.h).

Start nat-client, giving it some text-based name on the command 
line. The client will send a message to the server, and get a 
list of clients back, which should include itself. It will then 
send a message to itself.

Start another client, behind another NAT gateway, and point it 
at the same server (when building). Give it another text-based 
name on the command line. If all goes well, these two clients 
should be able to send each other messages, after the introducer 
has introduced them and gotten out of the way.

It's not necessary to re-register with the introducer every 5 
seconds, like this implementation does.

The folder "msvc71" contains a project for Microsoft Visual 
Studio .NET 2003 to build a Win32 version of the client only. 
It will have a hard-coded client name (no command line).

