#include "PSERVER.h"
#include "ServException.h"
#include "writeLog.h"
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    PSERVER server;
    server.serviceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    try
    {
        server.serverInitialization();
       	while (true) {
       		try
       		{
                server.serverHandler();
       		}
       		catch (const ServException& ex)
       		{
       			std::string ErrorCode = std::to_string(ex.GetErrorCode());
       			writeLog(ex.GetErrorType() + ErrorCode);
       		}
       	}
    }
    catch (const ServException& ex)
    {
    	std::string ErrorCode = std::to_string(ex.GetErrorCode());
    	writeLog(ex.GetErrorType() + ErrorCode);
    }
}

