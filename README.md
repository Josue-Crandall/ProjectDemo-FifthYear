## Note spell check later

# ProjectDemo-FourthYear

<a name="top"></a>
# Example project.

This project is a chat client and server designed with the goal of 
leaking as little information as practical.

This project has had many iterations. It ran the gambit from discord self-botting with a
javafx GUI, javascript/nodejs web deployments on AWS with whonix VM frontends, 
to openBSD only iterations with an ncurses tui.

This iteration has been an exploration of the C programming language, and 
of posix operating systems. As such many of the design chocies were made purely as
an opportunity to explore various styles of programming.

Note: This is a students learning project, please don't take anything I do here as secure.

# Table of contents

1. [Introduction](#1)
2. [Transport](#2)
3. [Server](#3)

/*

4. [Server Protocol](#4)
5. [Client](#5)
6. [Client Protocols](#6)
7. [Basic Usage](#7)
8. [Demo](#8)
*/

10 [Misc.](#9)

<a name="1"></a>
## Introduction
This README.md hopes to give a brief summary of the design considerations that went
into the development of this chat system.

<a name="2"></a>
### Transport

Since the goal of the project is to leak as little information to the network as possible
the choice of transport and constraints on what is allowed to touch the network are critical.

Constraints:
1) All network connections and packets are over tor onion service
2) All network packet payloads are indistinguishable from CPRNG output 
3) All network packet payloads are of a fixed size

Allowances:
1) Only so much can be done to efficiently hide total network traffic, no such attempt is made here.
2) The system does not define what other processess on the same computer system do with the network.
   // Prior iterations were intended to be used in isolation with whonix, or specific configurations of openBSD.
   // It was found that it was practically inconvenient and users were finding insecure workarounds.

<a name="3"></a>
### Server

The server in this design is assumed compromised.
Clients and the server may only communicate via [transport](#2) and [server protocol](#4) restrictions.
As such the server does itself provide a layer of security if not compromised via the server protocol.

Originally all memory was page locked, and all messages were unlogged and broadcast to all connected users.
This meant the server did not have to know who connecting was talking to who, and as little trace of messages
were left on the server as possible.

However it was too inconvient to not be able to leave messages when using cryptographic protocols with ratcheting keys
so logs were added. Once logs were added page locking all memory was impractical (large media file sends that had
to stay around for weeks). Then once the broadcast server was being used for more than simple chat server 
purposes (video game networking) it was convient to split clients up into channels.

The end result being for practical purposes the server keeps message logs, splits users into broadcast channels, and only
page locks data related to the server protocol keys.





[to top](#top)
