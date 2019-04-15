 #ifndef P2P
#define P2P

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define IP_A "192.168.4.2"
#define IP_B "192.168.4.6"
#define IP_C "192.168.4.14"
#define IP_INTERN "127.0.0.1"

typedef enum pdu 
{
    CON_PEER_NET,   //peer gibt einem anderen peer bescheid das er dem netwerk beitretten möchte
    CON_NEWPEER,    //information das sich bekannte peers beim new peer melden sollen
    ADD_ME,         //nachricht das Peer zu dem Netwerk dazugehören möchte
    CON,            //chat beitretten
    MSG,            //nachricht schicken
    EXT             //aus chat austretten
}msg_command;

typedef struct PDU_type
{
    msg_command msg_command;
    char nickname[25];
    char nachricht[256];
    struct sockaddr_in addr;
}chat_message;

typedef struct UserList
{
    struct sockaddr_in user;
    struct UserList *nextUser;
} ipLinkedList;


#endif
