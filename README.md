# ServerClient
C++ Server-Client application

Two C++ Server to Multi-Client applications, one TCP-based and one UDP-based.

The TCP server keeps responding with a stupid message no matter what clients send.

The UDP server also respond with stupid message upon receiving message, however clients will not receive the message unless connected to the server via calling connect() function.
