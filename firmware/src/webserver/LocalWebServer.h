#ifndef LOCALWEBSERVER_H
#define LOCALWEBSERVER_H

#include <ESPAsyncWebServer.h>

class LocalWebServer {
public:
    static void start();
    static AsyncWebServer& getServer() { return server; }

private:
    static AsyncWebServer server;
};
#endif