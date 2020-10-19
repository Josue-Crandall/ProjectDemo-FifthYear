## Note spell check later - I do not know what needs explaining anymore

# ProjectDemo-FourthYear

<a name="top"></a>
# Alice v.07

This project is a chat client and server designed with the goal of \
leaking as little information as practical.\
\
This project has had many iterations. It ran the gambit from discord self-botting with a\
javafx GUI, javascript/nodejs web deployments on AWS with whonix VM frontends, \
to openBSD only iterations with an ncurses tui.\
\
This iteration has been an exploration of the C programming language, and \
of posix operating systems. As such many of the design chocies were made purely as\
an opportunity to explore various styles of programming.\
\
Note: This is a students learning project, please don't take anything I do here as secure.\

# Table of contents

1. [Introduction](#1)
2. [Transport](#2)
3. [Server](#3)
4. [Server Protocol](#4)
5. [Client](#5)
6. [Client Protocols](#6)
7. [cTools](#7)
8. [Misc.](#8)

<a name="1"></a>
## Introduction
This README.md hopes to give a brief summary of the design considerations that went\
into the development of this chat system.\
\
Then a short description of related tools used with the chat system.

<a name="2"></a>
### Transport
\
Since the goal of the project is to leak as little information to the network as possible\
the choice of transport and constraints on what is allowed to touch the network are critical.\
\
Constraints:
1) All network connections and packets are over tor onion service
2) All network packet payloads are indistinguishable from CPRNG output
3) All network packet payloads are of a fixed size\
\
Allowances:
1) Only so much can be done to efficiently hide total network traffic, no such attempt is made here.
2) The system does not define what other processess on the same computer system do with the network.\
   // Prior iterations were intended to be used in isolation with whonix, or specific configurations of openBSD.\
   // It was found that it was practically inconvenient and users were finding insecure workarounds.

<a name="3"></a>
### Server

The server in this design is assumed compromised.\
Clients and the server may only communicate via [transport](#2) and [server protocol](#4) restrictions.\
As such the server does itself provide a layer of security if not compromised via the server protocol.\
\
The server is strictly intended to transport messages, it does not directly handle business logic related to the clients chatting.\
The server keeps message logs, splits users into broadcast channels.

<a name="4"></a>
### Server Protocol

The server and client do a public key exchange with ephermal keys authenticated by a pair of identity keys.\
The goal here is to wind up with two new symmetric keys which have forward secrecy if the idenity keys are compromised.\
 // The client identity key is intended to be shared amongst all valid clients.\
 // This leaks slightly less information about clients to the server.\
 \
Then the new symmetric keys are used as ratcheting keys.\
 // Ratcheting in this context meaning the keys are one way hashed after each message sent.\
 // In essence this destroys the keys, meaning an eve's dropper who is recorded network traffic cannot\
 // find a way to decrypt the messages by breaking into the server or client afterwards.\
 // And because the hash is deterministic, both server and client still have a shared secret.

<a name="5"></a>
### Client

The client is a set of microservices: an encryption proxy, a chat logic process, and a UI.\
\
A pecularity with the chat logic implementation is that it maintains files at runtime which\
consitute the state of the chat program decoupling it pretty heavily from the UI.\
\
The encryption proxy has three end to end encrypting modes that it can perform, and it can\
perform any number of them at the same time depending on configuration.

<a name="6"></a>
### Client Protocols

Three currently exist: Simple, Double Ratcheting, One Time Padding\
\
Simple protocol simply uses nacl backed symmetric encryption with a given key. Any message\
send through the proxy to the server is encrypted symmetrically, and any message from the \
server is decrypted symmetrically before being passed on.\
\
Double Ratcheting protocol uses a ratcheting pair of symmetric keys much like Server Protocol\
only on every exchange back and forth there is also an ephermeral asymmetric exchange which is mixed into\
the key state.\
   // The purpose of this is best illustrated by an example. Lets say Alice is having a conversation with\
   // her friend Bob. We have a shared secret K1 a double ratcheting key which we use to communicate.\
   // Lets say Bob's sister Mallory sneaks into his room while hes at lunch and copies the secret K1.\
   // With most encryption protocols, mallory can now read all the messages Bob sends me.\
   // However lets say before each message we do an asymmetric exchange generating a new secret K2 which\
   // we mix with K1 creating K3 which we use to talk. Then Mallory who only has K1 can't actually read the messages without\
   // stealing K2 or K3. Forcing anybody who wants to read our messages to have continuous access rather than access once.\
   //\
   // The key mixing is also one way, if Mallory records our communications, then later steals K3, she can't\
   // go back and read what we had said using K1.\
   //\
   // Signal does a more robust version of this protocol: https://signal.org/docs/specifications/doubleratchet/ \
\
One Time Padding uses the one time pad wrapped in ratcheting symmetric encryption.\
   // The purpose of this protocol is increased security at the expense of awkward key management and performance.\
   // OTP is however information theoretically secure.

<a name="7"></a>
### cTools

CTools is a command line interface and a GUI frontend which performs mundane cryptographic tasks.\
\
Base 64 encoding / decoding.\
Generation of symmetric keys, asymmetric keys, double ratcheting keys, one time padding keys.\
Encryption and decryption of the above.\
\
All output is in base64 (or whatever format you fed it in the first place in the case of decryption).

<a name="8"></a>
### Misc

CodeBook program that spits out a codebook for non electronic one time padding (done symbol by symbol instead of bit by bit).\
Uses only easily electronically conveyable symbols.\
\
FileTransferServer simple node-express webserver with a chat program in it (simple sjcl symmetric encryption). \
Convenient for bootstrapping. Not clean was scavenged from other projects quickly.\
\
BrowserBasedChat distinct UI and chat logic plugs into the standard encryption proxy. Forces one to trust browser implementation,\
but is nicer as an experience UI wise. Not clean was scavenged from other projects quickly.

[to top](#top)
