#ifndef _SERVER_H_
#define _SERVER_H_

#include <vector>
#include <pthread.h>  

using namespace std;

/* Declare server class*/
class CServer;

typedef struct
{
    CServer*  m_pServerObj;
    int       m_client_id;   /* Client socket id */
}
CLIENT_SOCKET_INFO_t;

/* Error definition */
typedef enum
{   
    SERVER_ERR_NONE = 0,
    SERVER_ERR_CREATE_SOCKET, 
    SERVER_ERR_BIND_SOCKET, 
    SERVER_ERR_LISTEN_SOCKET, 
    SERVER_ERR_RECEIVE_DATA,
    SERVER_ERR_SEND_DATA,
    SERVER_ERR_CLOSE_SOCKET,
    SERVER_ERR_DISCONNECT,

    SERVER_ERR_OTHER
}
SERVER_ERR_CODE_t;

/* Callback function to handle requests from client */
typedef SERVER_ERR_CODE_t (*CALLBACK_FUNC)(CServer *pServerObj, void *pData);

/* Class definition */
class CServer
{
private:
    int m_socket_server;

    /* Port and IP address information */
    int  m_port;
    char m_ip_address[30];  

    /* Callback funtion to handle client request */
    CALLBACK_FUNC m_client_request_handler; 

    /* Clients */
    vector<CLIENT_SOCKET_INFO_t> m_client_list;

public:
    CServer();
    CServer(int port, const char* pIPAddress, CALLBACK_FUNC p_callback_func = NULL);
    ~CServer();

private:
    SERVER_ERR_CODE_t createSocket();    
    SERVER_ERR_CODE_t bindSocket();
    SERVER_ERR_CODE_t listenSocket();   
    SERVER_ERR_CODE_t acceptSocket(int *pClientSocket); 

public:
    SERVER_ERR_CODE_t startServer();
    SERVER_ERR_CODE_t stopServer();

    SERVER_ERR_CODE_t receiveMessage(const int client_sock, char* pReceivedData, int* total_size);
    SERVER_ERR_CODE_t sendMessage(const int client_sock, const char* pSendData);

    SERVER_ERR_CODE_t addCallback(CALLBACK_FUNC p_callback_func);
    SERVER_ERR_CODE_t executeCallback(void* pData);
    SERVER_ERR_CODE_t removeCallback();

    SERVER_ERR_CODE_t broadcastAllClients(const char* Message);
    SERVER_ERR_CODE_t closeClient(int clientID);
};

#endif /* _SERVER_H_ */