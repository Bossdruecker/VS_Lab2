#include "p2p.h"

ipLinkedList *head, *current_peer, *last_peer, *delete_peer;
chat_message send_msg, recv_msg, *point_msg;

void sendingmsg(int socked, chat_message *point_msg)
{
    struct sockaddr_in addr_send;
    current_peer = head;
    while (current_peer->nextUser != NULL)
    {
        current_peer = current_peer->nextUser;
        addr_send.sin_family = current_peer->user.sin_family;
        addr_send.sin_port = current_peer->user.sin_port;
        addr_send.sin_addr.s_addr = current_peer->user.sin_addr.s_addr;
        sendto(socked, (void *)point_msg, sizeof(chat_message), 0, (struct sockaddr *)&addr_send, sizeof(struct sockaddr_in));
    }
}

//return 0 wenn es funktioniert hat return -1 Fehler
int addPeer(struct sockaddr_in addr)
{
    last_peer->nextUser = malloc(sizeof(ipLinkedList));
    last_peer = last_peer->nextUser;
    if (last_peer != NULL)
    {
        last_peer->user.sin_family = addr.sin_family;
        last_peer->user.sin_port = addr.sin_port;
        last_peer->user.sin_addr.s_addr = addr.sin_addr.s_addr;
        last_peer->nextUser = NULL;
    }
    else
    {
        perror("Fehler bei allokieren von Speicher für head");
        return -1;
    }
    return 0;
}

//return 0 wenn es funktioniert hat return -1 Fehler
void deletePeer(struct sockaddr_in addr)
{
    ipLinkedList *peer, *delete_peer;
    peer = head;
    while (peer->nextUser != NULL)
    {
        if(peer->nextUser->user.sin_port == addr.sin_port)
        {
            delete_peer = peer->nextUser;
            peer->nextUser = peer->nextUser->nextUser;
            free(delete_peer);
        }
        peer = peer->nextUser;
    }
}

int main()
{
    int exit_chat = 1;

    char buf[256];
    struct sockaddr_in addr, otheraddr;

    printf("(0) -> Eigenes PeerNetwork erstellen\n(Clinetnr: 1 höher) -> PeerNetwork beitreten/n");
    int input;
    scanf("%d", &input);
    if (input == 0)
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3322);
        addr.sin_addr.s_addr = inet_addr(IP_INTERN);

        //erster Eintrag der eigene
        head = malloc(sizeof(ipLinkedList));
        if (head != NULL)
        {
            head->user.sin_family = AF_INET;
            head->user.sin_port = htons(3322);
            head->user.sin_addr.s_addr = inet_addr(IP_INTERN);
            head->nextUser = NULL;
            current_peer = head;
            last_peer = head;
        }
        else
        {
            perror("Fehler bei allokieren von Speicher für head");
            return -1;
        }
    }
    else
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3322 + input); // port um anzahl des Client erhöhen
        addr.sin_addr.s_addr = inet_addr(IP_INTERN);

        head = malloc(sizeof(ipLinkedList));
        if (head != NULL)
        {
            head->user.sin_family = AF_INET;
            head->user.sin_port = htons(3322 + (int)input);
            head->user.sin_addr.s_addr = inet_addr(IP_INTERN);
            head->nextUser = NULL;
        }
        else
        {
            perror("Fehler bei allokieren von Speicher für head");
            return -1;
        }

        current_peer = malloc(sizeof(ipLinkedList));
        if (current_peer != NULL)
        {
            current_peer->user.sin_family = AF_INET;
            current_peer->user.sin_port = htons(3322);
            current_peer->user.sin_addr.s_addr = inet_addr(IP_INTERN);
            current_peer->nextUser = NULL;
            head->nextUser = current_peer;
            last_peer = current_peer;
        }

        else
        {
            perror("Fehler bei allokieren von Speicher für head");
            return -1;
        }
    }

    //socket erstellen
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        perror("socket error!\n");
        return -1;
    }

    //bind erstellen
    int bind_fd = bind(socket_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (bind_fd == -1)
    {
        perror("bind error!\n");
        return -1;
    }

    point_msg = &send_msg;
    printf("Geben Sie bitte ihren Nickname ein: ");
    fscanf(stdin, "%s", point_msg->nickname);

    if (input > 0)
    {
        point_msg = &send_msg;
        point_msg->msg_command = CON_PEER_NET;
        otheraddr.sin_family = current_peer->user.sin_family;
        otheraddr.sin_port = current_peer->user.sin_port;
        otheraddr.sin_addr.s_addr = current_peer->user.sin_addr.s_addr;
        int send = sendto(socket_fd, (void *)point_msg, sizeof(chat_message), 0, (struct sockaddr *)&otheraddr, sizeof(struct sockaddr_in));
        if (send == -1)
        {
            perror("Sendto CON_PEER_NET nicht erfolgreich!");
        }
    }

    fd_set my_readset;

    fflush(stdin);

    while (exit_chat)
    {
        FD_ZERO(&my_readset);
        FD_SET(0, &my_readset);
        FD_SET(socket_fd, &my_readset);

        if (select(socket_fd + 1, &my_readset, NULL, NULL, NULL) == -1)
        {
            return -1;
        }

        if (FD_ISSET(0, &my_readset))
        {
            point_msg = &send_msg;
            if (fgets(buf, 256, stdin) != NULL)
            {
                if (strcmp(buf, "!EXIT\n") == 0)
                {
                    point_msg->msg_command = EXT;
                    exit_chat = 0;

                    sendingmsg(socket_fd, point_msg);
                }
                else
                {
                    point_msg->msg_command = MSG;
                    strcpy(point_msg->nachricht, buf);
                    sendingmsg(socket_fd, point_msg);
                }
            }
        }

        if (FD_ISSET(socket_fd, &my_readset))
        {

            point_msg = &recv_msg;
            unsigned int addr_lenght = sizeof(struct sockaddr_in);
            if (recvfrom(socket_fd, point_msg, sizeof(chat_message), 0, (struct sockaddr *)&addr, &addr_lenght) != -1)
            {
                switch (point_msg->msg_command)
                {
                case MSG:
                    if (strlen(point_msg->nachricht) > 1) //sonst doppelte nachrichten
                    {
                        printf("\n%s: %s\n", point_msg->nickname, point_msg->nachricht);
                    }
                    break;
                case CON:
                    printf("%s tritt dem Chat bei!\n", point_msg->nickname);
                    break;
                case EXT:
                    deletePeer(addr);
                    printf("%s hat den Chat verlassen\n", point_msg->nickname);
                    // current_peer = head;
                    // while (current_peer->nextUser != NULL)
                    // {
                    //     if (current_peer->nextUser->user.sin_port == addr.sin_port)
                    //     {
                    //         delete_peer = current_peer->nextUser;
                    //         current_peer->nextUser = current_peer->nextUser->nextUser;
                    //         free(delete_peer);
                    //         printf("%s hat den Chat verlassen\n", point_msg->nickname);
                    //         break;
                    //     }
                    // }
                    break;
                case CON_PEER_NET:
                    //point_msg auf send buffer
                    point_msg = &send_msg;
                    point_msg->msg_command = CON_NEWPEER;
                    strcpy(point_msg->nickname, recv_msg.nickname);
                    point_msg->addr.sin_family = addr.sin_family;
                    point_msg->addr.sin_port = addr.sin_port;
                    point_msg->addr.sin_addr.s_addr = addr.sin_addr.s_addr;

                    //Nachricht an alle Peers mit IP von neuem Peer
                    sendingmsg(socket_fd, point_msg);

                    //point_msg auf recieve buffer
                    point_msg = &recv_msg;

                    //Peer zu LinkedList hinzufügen
                    last_peer->nextUser = malloc(sizeof(ipLinkedList));
                    last_peer = last_peer->nextUser;
                    if (last_peer != NULL)
                    {
                        last_peer->user.sin_family = addr.sin_family;
                        last_peer->user.sin_port = addr.sin_port;
                        last_peer->user.sin_addr.s_addr = addr.sin_addr.s_addr;
                        last_peer->nextUser = NULL;
                        printf("%s tritt dem Chat bei! CON_PEER_NET\n", point_msg->nickname);
                    }
                    else
                    {
                        perror("Fehler bei allokieren von Speicher last_Peer CON_PEER_NET");
                        return -1;
                    }

                    break;
                case CON_NEWPEER:
                    if (point_msg->addr.sin_port == 0)
                    {
                        break;
                    }
                    else
                    {
                        last_peer->nextUser = malloc(sizeof(ipLinkedList));
                        last_peer = last_peer->nextUser;
                        if (last_peer != NULL)
                        {
                            last_peer->user.sin_family = point_msg->addr.sin_family;
                            last_peer->user.sin_port = point_msg->addr.sin_port;
                            last_peer->user.sin_addr.s_addr = point_msg->addr.sin_addr.s_addr;
                            last_peer->nextUser = NULL;

                            printf("%s tritt dem Chat bei CON_NEWPEER!\n", point_msg->nickname);
                        }
                        else
                        {
                            perror("Fehler bei allokieren von Speicher last_Peer CON_PEER");
                            return -1;
                        }

                        otheraddr.sin_family = point_msg->addr.sin_family;
                        otheraddr.sin_port = point_msg->addr.sin_port;
                        otheraddr.sin_addr.s_addr = point_msg->addr.sin_addr.s_addr;

                        //point_msg auf send buffer
                        point_msg = &send_msg;
                        point_msg->msg_command = ADD_ME;
                        strcpy(point_msg->nachricht, "Ciao");
                        sendto(socket_fd, (void *)point_msg, sizeof(chat_message), 0, (struct sockaddr *)&otheraddr, sizeof(struct sockaddr_in));
                    }

                    //printf("%s tritt dem Chat bei!\n", point_msg->nickname);
                    break;
                case ADD_ME:
                    if (strlen(point_msg->nachricht) > 1) //sonst doppelte nachrichten
                    {
                        last_peer->nextUser = malloc(sizeof(ipLinkedList));
                        last_peer = last_peer->nextUser;
                        if (last_peer != NULL)
                        {
                            last_peer->user.sin_family = addr.sin_family;
                            last_peer->user.sin_port = addr.sin_port;
                            last_peer->user.sin_addr.s_addr = addr.sin_addr.s_addr;
                            last_peer->nextUser = NULL;

                            printf("%s tritt dem Chat bei!\n", point_msg->nickname);
                        }
                        else
                        {
                            perror("Fehler bei allokieren von Speicher last_Peer ADD_ME");
                            return -1;
                        }
                    }
                    break;
                default:
                    printf("Okey Houston, we've had a problem here ;D\n");
                    break;
                }
            }
        }
    }

    //schließen des Sockets
    int close_fd = close(socket_fd);
    if (close_fd == -1)
    {
        printf("konnte Socket nicht korrekt schliesen\n");
    }

    return 0;
}
