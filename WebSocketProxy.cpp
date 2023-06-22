#include "PSERVER.h"
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    PSERVER::serviceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    PSERVER server;
    server.startServer();

}

