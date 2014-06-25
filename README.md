SYE-Server
==========

Simple yet effective UDP server with custom protocol and multithreading.
Written in C++11, includes simple demo with server-client based chat.
Main target is simple multiplayer games with fast response time needed (like FPS etc.).


Requriements:
====

* Compiler support for C++11 (including std::thread) like MinGW-builds
* WinSock2 linked on Windows


Features:
====

* Client-Server based transfer
* Multithreading - for its work creates two additional threads to listen and response.
* Simple protocol for connecting, disconnecting and transfer of data
* Many "connected" clients at the same time while maintaing data about each
* Simple packet verification
* Simple expanding on other platform - just implement abstract class, include it and you're good to go
* Simple implentation to existing project - just include files, create server-object and host/connect with it to start sending data between users.
* Most of the code is in special namespace so that your code is still clean and will not create any additional problems
