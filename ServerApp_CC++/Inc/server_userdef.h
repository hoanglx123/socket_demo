#ifndef _SERVER_USERDEF_H_
#define _SERVER_USERDEF_H_

/* Port */
#define LISTEN_PORT     8888
/* Ip address */
#define IP_ADDRESS      "127.0.0.5"

#define MAX_LENGTH      30 

//#define DEBUG_LOG_ENABLE

#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...) ((void)0)
#endif

#endif /* _SERVER_USERDEF_H_ */