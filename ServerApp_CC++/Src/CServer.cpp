#include <stdio.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>  

#include "CServer.h"
#include "server_userdef.h"

/* Prototypes */
void* connection_handler(void* pData);

CServer::CServer()
{
    CLIENT_SOCKET_INFO_t client = 
    {
        .m_pServerObj = this,
        .m_client_id  = -1
    };

    /* Initialize default value */
    m_socket_server   = -1;
    m_port          = 8888; 

    /* Use local host as ip address*/
    strcpy(m_ip_address, "127.0.0.1");

    /* Initialize callback function */
    m_client_request_handler = NULL; 

    m_client_list.resize(100, client); 
}

CServer::CServer(int port, const char* pIPAddress, CALLBACK_FUNC p_callback_func /* = NULL */) : CServer()
{
    /* Get port */
    m_port = port; 

    /* IP address is valid */
    if(pIPAddress != NULL)
    {
        /* Store ip address*/
        strcpy(m_ip_address, pIPAddress); 
    }

    /* Callback function is available */
    if(p_callback_func != NULL)
    {
        m_client_request_handler = p_callback_func; 
    }
}

CServer::~CServer()
{

}

SERVER_ERR_CODE_t CServer::createSocket()
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    /* Create socket */
    m_socket_server = socket(AF_INET , SOCK_STREAM , 0);
    if (m_socket_server == -1)
    {
        retCode = SERVER_ERR_CREATE_SOCKET;
    }

    return retCode; 
} 

SERVER_ERR_CODE_t CServer::bindSocket()
{
    SERVER_ERR_CODE_t  retCode = SERVER_ERR_NONE; 
    int                nOption = 1;
    int                nErrno  = 0;
    struct sockaddr_in server; 

    /* Prepare the sockaddr_in structure */
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = inet_addr(m_ip_address);
    server.sin_port        = htons(m_port);

    /* Create socket successfully */
    if(m_socket_server != -1)
    {
        /* Reuse address and port for socket */ 
        if (setsockopt(m_socket_server, SOL_SOCKET,SO_REUSEADDR, (void*)&nOption,sizeof(nOption)) == -1) 
        {
            retCode = SERVER_ERR_BIND_SOCKET;
        }
        /* Bind socket */
        else if( bind(m_socket_server,(struct sockaddr *)&server , sizeof(server)) < 0)
        {
            retCode = SERVER_ERR_BIND_SOCKET;

            DEBUG_LOG("Cannot bind socket: [%d]:[%s]\n", nErrno, strerror(nErrno));
        }
    }
    else    
    {
        retCode = SERVER_ERR_BIND_SOCKET;
    }
    
    return retCode;
}   

SERVER_ERR_CODE_t CServer::listenSocket()
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    if(m_socket_server != -1)
    {
        if( listen(m_socket_server, 10) < 0)
        {
            retCode = SERVER_ERR_LISTEN_SOCKET;            
        }             
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::acceptSocket(int *pClientSocket)
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 
    int c = 0;
    struct sockaddr_in client;

    if( (m_socket_server != -1) && (pClientSocket != NULL) )
    {
        c = sizeof(struct sockaddr_in);
        *pClientSocket = accept(m_socket_server, (struct sockaddr *)&client, (socklen_t*)&c);
    
        if(*pClientSocket < 0)
        {
            retCode = SERVER_ERR_LISTEN_SOCKET;
        }
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::receiveMessage(const int client_sock, char* pReceivedData, int* total_size)
{
    SERVER_ERR_CODE_t retCode    = SERVER_ERR_NONE; 
    int               read_size  = 0;
    int               bCompleted = false;
    const int         CHUNK_SIZE = 512;

    if( (client_sock != -1) && (pReceivedData != NULL) ) 
    {
        *total_size = 0;
        /* Read all data in buffer */
        while( (retCode == SERVER_ERR_NONE) && !bCompleted )
        {
            /* Received data */
            read_size = recv(client_sock , pReceivedData , CHUNK_SIZE, 0); 
            /* Received data failed */
            if(read_size < 0)
            {
                retCode = SERVER_ERR_RECEIVE_DATA;
            }
            /* Client is disconneted */
            else if(read_size == 0)
            {
                retCode =  SERVER_ERR_DISCONNECT; 
            }
            /* Receive data successfully */
            else 
            {
                /* Read all data in buffer */
                if(read_size < CHUNK_SIZE)
                {
                    bCompleted  = true; 
                }

                *total_size = *total_size + read_size;
            }   
        }
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::sendMessage(const int client_sock, const char* pSendData)
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    if( (client_sock != -1) && (pSendData != NULL) ) 
    {
        write(client_sock , pSendData , strlen(pSendData));
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::startServer()
{
    SERVER_ERR_CODE_t retCode   = SERVER_ERR_NONE; 
    bool              bExit     = false;  
    int               nCurIndex = 0;
    pthread_t         thread_info;
    CLIENT_SOCKET_INFO_t client_socket; 

    /* Create socket */
    retCode = createSocket();
    /* Create socket successfully */
    if(retCode == SERVER_ERR_NONE)
    {
        /* Bind socket */
        retCode = bindSocket();
        /* Bind socket successfully */
        if(retCode == SERVER_ERR_NONE)
        {
            /* Listen socket*/
            retCode = listenSocket();
        }
    }

    /* Setup successfully */
    if(retCode == SERVER_ERR_NONE)
    {
        printf("Server %s:%d has already started...\n", m_ip_address, m_port);
    
        /* Waiting for request connection */
        while(!bExit)
        {
            /* Initialzie value */
            memset(&thread_info, 0, sizeof(thread_info));
            memset(&client_socket, 0, sizeof(client_socket));

            /* Accept socket */
            retCode = acceptSocket(&client_socket.m_client_id);
            /* Accept socket successfully */
            if(retCode == SERVER_ERR_NONE)
            {
                /* Store server object */
                client_socket.m_pServerObj = this;    
                /* Store client in list */
                m_client_list.push_back(client_socket);
                nCurIndex = m_client_list.size() - 1;

                /* Create thread for each client socket */
                if( pthread_create( &thread_info , NULL ,  connection_handler , (void*)&m_client_list[nCurIndex]) < 0)
                {
                    bExit   = true;
                    retCode = SERVER_ERR_OTHER;
                }
                else
                {
                    /* Wait for all threads finished */
                    //pthread_join(thread_info, NULL); 
                }               
            }
            else
            {
                bExit = true;
            }
        }
    }
    else
    {
        printf("ERROR!!! Can't start server. Error code = 0x%04x\n", retCode);
    }

    /* Try to stop server when error occurs */
    if(retCode != SERVER_ERR_NONE)
    {
        /* Stop server */ 
        retCode = stopServer();
    }

    return retCode;
 }

SERVER_ERR_CODE_t CServer::stopServer()
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    /* Close socket when it's available */
    if(m_socket_server != -1)
    {
        /* Close all connections */
        for(int i = 0; i < m_client_list.size(); i++)
        {
            /* Socket is valid */
            if(m_client_list[i].m_client_id != -1)
            {   
                /* Close client socket */
                retCode = closeClient(m_client_list[i].m_client_id); 
                /* Close client socket failed */
                if(retCode != SERVER_ERR_NONE)
                {
                    break; 
                }
            }
        }

        /* Close server */
        if( (retCode == SERVER_ERR_NONE) && (close(m_socket_server) < 0) )
        {
            retCode = SERVER_ERR_CLOSE_SOCKET; 
        }

        /* Clear client in list */
        m_client_list.clear();
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::addCallback(CALLBACK_FUNC p_callback_func)
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    /* Callback function is available */
    if(p_callback_func != NULL)
    {
        m_client_request_handler = p_callback_func; 
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::executeCallback(void* pData)
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    /* Callback is valid */
    if(m_client_request_handler != NULL)
    {
        /* Execute callback */
        retCode = m_client_request_handler(this, pData);
    }

    return retCode;
}


SERVER_ERR_CODE_t CServer::removeCallback()
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 

    /* Callback is valid */
    if(m_client_request_handler != NULL)
    {
        m_client_request_handler = NULL;
    }

    return retCode;
}


SERVER_ERR_CODE_t CServer::closeClient(int clientID)
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 
    int               nIndex  = 0;
    bool              bFound  = false; 

    /* Find specified client */
    while( (nIndex < m_client_list.size()) && !bFound )
    {
        /* Client ID is found */
        if(clientID == m_client_list[nIndex].m_client_id)
        {
            bFound = true;
        }
        else
        {
            nIndex++;
        } 
    }

    /* Check that specified client is found */
    if( true == bFound )
    {
        /* Close client socket failed */
        if( close(clientID) < 0 )
        {
            retCode = SERVER_ERR_CLOSE_SOCKET;
        }

        /* Remove specified client from list */
        //m_client_list.erase(m_client_list.begin() + nIndex);
        m_client_list[nIndex].m_client_id = -1; 
    }

    return retCode;
}

SERVER_ERR_CODE_t CServer::broadcastAllClients(const char* Message)
{
    SERVER_ERR_CODE_t retCode = SERVER_ERR_NONE; 
    int               nIndex  = 0; 

    /* Close socket when it's available */
    if(m_socket_server != -1)
    {
        /* Send message all clients */
        while( (nIndex < m_client_list.size()) && (retCode == SERVER_ERR_NONE) )
        {
            /* Send message to each client */
            retCode = sendMessage(m_client_list[nIndex].m_client_id, Message);
            nIndex++;
        }
    }

    return retCode;
}

/* Thread handler */
void* connection_handler(void* pData)
{
    SERVER_ERR_CODE_t     retCode     = SERVER_ERR_NONE; 
    CLIENT_SOCKET_INFO_t *pClientInfo = (CLIENT_SOCKET_INFO_t*)pData; 
    bool                  bExit       = false;  

    printf("Client[%d]: connected\n", pClientInfo->m_client_id);

    /* Client is valid */
    if( pClientInfo != NULL)
    {
        while( (pClientInfo->m_pServerObj != NULL) && !bExit)
        {             
            /* Execute callback */
            retCode = pClientInfo->m_pServerObj->executeCallback((void*)&pClientInfo->m_client_id);
            /* Execute callback failed */
            if(retCode != SERVER_ERR_NONE)
            {
                bExit = true;
            }
        } 
    }

    pthread_exit(NULL); 
}
