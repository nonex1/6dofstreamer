# Introduction #

In order to install this library, make sure you have installed the Non-Commercial faceAPI License v3.1.3
It can be downloaded from here:
http://www.seeingmachines.com/product/faceapi/downloads/

After it is installed, simply run the Socket.exe file, which is in the http://6dofstreamer.googlecode.com/files/faceapistreamer0.9.zip file. The program should start streaming head position and rotation.


---

This application will open a socket connection with localhost, at port 29129 (by default), and stream the translation and rotation of the head as a string, like this:

```
    sprintf(sentbuf, "%lf %lf %lf %lf %lf %lf",head->x, head->y, head->z, head->rx, head->ry, head->rz);

    sockaddr_in RecvAddr;
    RecvAddr.sin_family = AF_INET;
    int Port = 29129;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    iResult = sendto(ListenSocket,sentbuf, strlen(sentbuf),0,(SOCKADDR *) &RecvAddr, sizeof(RecvAddr));
```

It is a udp connection. A simple python udp client/server like the one here (http://www.evolt.org/node/60276) would be able to get all the data.