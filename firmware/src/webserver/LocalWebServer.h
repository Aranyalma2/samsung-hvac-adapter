#ifndef LOCALWEBSERVER_H
#define LOCALWEBSERVER_H

#include <ESPAsyncWebServer.h>

class LocalWebServer {
public:
    static void start();

private:
    static AsyncWebServer server;
};
#endif