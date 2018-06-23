#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "CServer.h"
#include "server_userdef.h"
 
/* Client request handler */
SERVER_ERR_CODE_t handleClientRequest(CServer* pObj, void* pData);
/* Signal handler */
void signal_handler(int signal_number);

/* Server */
CServer server(LISTEN_PORT, IP_ADDRESS, handleClientRequest);

int main(int argc , char *argv[])
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE;

    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    /* Start server */
    retCode = server.startServer(); 
    /* Stop server */
    retCode = server.stopServer();

    return 0;
}

SERVER_ERR_CODE_t handleClientRequest(CServer* pObj, void* pData)
{
    SERVER_ERR_CODE_t retCode  = SERVER_ERR_NONE;
    int               clientID = *(int*)pData;
    int               nSize    = 0;
    char              ReceivedMessage[MAX_LENGTH] = {'\0'}; 
    char              ResponseMessage[MAX_LENGTH] = {'\0'};

    /* Object is valid */
    if( pObj != NULL)
    {
        /* Receive data from client */
        retCode = pObj->receiveMessage(clientID, &ReceivedMessage[0], &nSize);
        /* Receive data successfully */
        if(retCode == SERVER_ERR_NONE)
        {
            printf("Client[%d]: %s\n", clientID, ReceivedMessage);

            /* Create response message */
            sprintf(&ResponseMessage[0], "Hello client[%d]", clientID);

            /* Send the message back to server */
            retCode = pObj->sendMessage(clientID, ResponseMessage);
        }
        else if(retCode == SERVER_ERR_DISCONNECT)
        {
            printf("Client[%d]: disconnected\n", clientID);

            /* Create response message */
            sprintf(&ResponseMessage[0], "Client[%d] says \"BYE BYE!!!\"\n", clientID);

            /* Close specified client */
            retCode = pObj->closeClient(clientID);

            /* Broadcast all clients */
            retCode = pObj->broadcastAllClients(&ResponseMessage[0]);

            /* Exit thread */
            retCode = SERVER_ERR_OTHER; 
        }
    }
    
    /* Print error code */
    if(retCode != SERVER_ERR_NONE)
    {
        DEBUG_LOG("ERROR!!! Error Code = 0x%04x\n", retCode);
    }

    return retCode; 
}

void signal_handler(int signal_number) 
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE;

    /* Stop server */
    retCode = server.stopServer();

    printf("Server is shutdown...\n");
    DEBUG_LOG("Error code = 0x%04x\n", retCode);

    exit(0);
}