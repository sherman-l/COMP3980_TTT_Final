//
// Created by Sherman Lok and Jeffrey Huynh on 2021-02-10.
// Server represented by a Finite State Machine to run a Tic Tac Toe game
//

#include <sys/types.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <err.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <ctype.h>
#include "generalServerResources.h"
#include "generalServerFunctions.h"

#define BACKLOG 4
#define DATAGRAM_SIZE 5008

#define MAX_CLIENTS 40
#define UID_BYTE_LENGTH 4
#define PAYLOAD_BYTE_LENGTH 1
#define PAYLOAD_LENGTH_INDEX 6
#define PAYLOAD_START_INDEX 7

SocketEnvironment** addressBook;

int serverConnections = 0;
ssize_t numByte;

/** GameEnvironment Lobbies **/
GameEnvironment* tttLobby;
GameEnvironment* rpsLobby;

/** Constructors **/
SocketEnvironment* createSocketEnvironment(int fd);

/** Getter **/
SocketEnvironment* getAddressBookEntry(int fd);

/** Helper Functions **/
int prompt_port();
int max(int x, int y);
void storeData(SocketEnvironment *socketEnv, uint8_t *data);
void fillOptions(SocketEnvironment *socketEnv);

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}

SocketEnvironment* createSocketEnvironment(int fd) {
    SocketEnvironment* socketEnvironment = malloc(sizeof(struct SocketEnvironment));
    socketEnvironment->fd = fd;
    socketEnvironment->byteCount = 0;
    socketEnvironment->packet = malloc(sizeof(uint8_t*) * MAX_MSG_SIZE);
    for(int j = 0; j < MAX_MSG_SIZE; j++) {
        socketEnvironment->packet[j] = malloc(sizeof(uint8_t));
    }
    socketEnvironment->packetOptions.uid = 0;
    socketEnvironment->packetOptions.reqType = 0;
    socketEnvironment->packetOptions.reqContext = 0;
    socketEnvironment->packetOptions.payloadLength = 0;
    socketEnvironment->generalState = SELECTGAME;

    return socketEnvironment;
}

void fillOptions(SocketEnvironment *socketEnv) {
    int trackIndex = 0;
    for(int i = 0; i < UID_BYTE_LENGTH; i++, trackIndex++) {
        socketEnv->packetOptions.uid += *socketEnv->packet[trackIndex];
        if (i < UID_BYTE_LENGTH-1) {
            socketEnv->packetOptions.uid <<= 8;
        }
    }
    socketEnv->packetOptions.reqType = *socketEnv->packet[trackIndex++];
    socketEnv->packetOptions.reqContext = *socketEnv->packet[trackIndex++];
    socketEnv->packetOptions.payloadLength = *socketEnv->packet[trackIndex++];
    socketEnv->packetOptions.protocolVersion = *socketEnv->packet[trackIndex];
}

void storeData(SocketEnvironment *socketEnv, uint8_t *data) {
    int numBytes = socketEnv->byteCount;
    int maxLength = (socketEnv->packetOptions.payloadLength == 0) ?
            PAYLOAD_LENGTH_INDEX + 2
            : PAYLOAD_LENGTH_INDEX + socketEnv->packetOptions.payloadLength + 1;
    if (socketEnv->byteCount < maxLength) {
        *socketEnv->packet[numBytes] = *data;
        (socketEnv->byteCount)++;
    }


    if(socketEnv->byteCount == (PAYLOAD_LENGTH_INDEX + 1)) {
        fillOptions(socketEnv);
    }

    if(socketEnv->byteCount == (PAYLOAD_LENGTH_INDEX + 2)) {
        socketEnv->packetOptions.protocolVersion = *data;
    }
}


SocketEnvironment* getAddressBookEntry(int fd) {
    for(int i = 0; i < serverConnections; i++) {
        if(addressBook[i]->fd == fd) {
            return addressBook[i];
        }
    }
    return NULL;
}


/** Prompts user for port number to host server on**/
int prompt_port(){
    printf("enter port you wish to host server on: \n");
    char* buf = malloc(sizeof(char) * BUF_SIZE);
    bool invalid = false;
    while(1){
        read(STDIN_FILENO, buf, BUF_SIZE);
        for(int i = 0; i < strlen(buf)-1; i++){
            if(!isdigit(buf[i])){
                invalid = true;
                perror("Please enter valid port! \n");
                break;
            }
        }
        if (!invalid) {
            break;
        }
    }
    int port = atoi(buf);
    free(buf);
    return port;
}

/** Drives the program **/
int main(int argc, char *argv[])
{
    tttLobby = createGameEnvironment(TTT);
    rpsLobby = createGameEnvironment(RPS);
    // waiting for responses + determining whether existing or old connection making a request
    int socket_list[MAX_CLIENTS];
    addressBook = malloc(MAX_CLIENTS*sizeof(SocketEnvironment));


    for(int i = 0; i < MAX_CLIENTS; i++) {
        socket_list[i] = 0;
    }
    int temp;
    fd_set rfds;
    int retval;
    int fdmax;
    int cfd;
    uint8_t* buf = malloc(sizeof(uint8_t));
    char* udpBuf = malloc(DATAGRAM_SIZE);

    int one = 1;
    struct sockaddr_in svr_addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        err(1, "can't open socket");

    int udpsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpsock < 0)
        err(1, "can't open udpsocket");

    int port = prompt_port();
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
        close(sock);
        err(1, "Can't bind");
    }
    if (bind(udpsock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
        close(udpsock);
        err(1, "Can't bind");
    }

    if(listen(sock, BACKLOG)==0){
        printf("Listening on %s...\n", inet_ntoa(svr_addr.sin_addr));
    } else {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    while(1) {

        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        FD_SET(udpsock, &rfds);
        fdmax = max(sock, udpsock);

        for (int i = 0; i < MAX_CLIENTS ; i++) {
            temp = socket_list[i];
            if(temp>0) {
                FD_SET(temp, &rfds);
            }
            if (temp > fdmax) {
                fdmax = temp;
            }
        }

        retval = select(fdmax+1, &rfds, NULL, NULL, NULL);
        if(retval == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        // TCP Socket Readable
        if(FD_ISSET(sock, &rfds)) {
            if((cfd = accept(sock, NULL, NULL)) < 0) {
                perror("accept error");
                exit(EXIT_FAILURE);
            }
            if(serverConnections == MAX_CLIENTS) {
                char* toWrite = malloc(sizeof(char)*BUF_SIZE);
                sprintf(toWrite, "ERR: Max Clients Reached\n");
                write(cfd, toWrite, strlen(toWrite));
                free(toWrite);
                continue;
            }

            for(int i = 0; i < MAX_CLIENTS; i++) {
                /** Insert into socket list since it is new connection; if n connection % 2 == 1 -> start game for that
                 *  pair of players. if n connection % 2 == 0, create new GameEnvironment for new pair of players **/
                if(socket_list[i] == 0) {
                    socket_list[i] = cfd;
                    addressBook[i] = createSocketEnvironment(cfd);
                    serverConnections++;
                    break;
                }
            }
        }
        // needs to be a real loop that stops when all fds have been processed
        for(int i = 0; i < MAX_CLIENTS; i++)
        {
            temp = socket_list[i];
            if(FD_ISSET(temp, &rfds))
            {
                if(read(temp, buf, 1) > 0) {
                    SocketEnvironment* tempSockEnv = getAddressBookEntry(temp);
                    storeData(tempSockEnv, buf);
                    if(tempSockEnv->byteCount == (PAYLOAD_LENGTH_INDEX + tempSockEnv->packetOptions.payloadLength + 1)) {
                        switch(tempSockEnv->generalState) {
                            case SELECTGAME:
                                readMessageType(tempSockEnv);
                                switch(tempSockEnv->gameType) {
                                    case TTT:
                                        joinGame(tempSockEnv, &tttLobby);
                                        break;
                                    case RPS:
                                        joinGame(tempSockEnv, &rpsLobby);
                                        break;
                                }
                                break;
                            case PLAYGAME:
                                readMessageType(tempSockEnv);

                        }
                        clearPacket(tempSockEnv);
                    }
                }
            }
            if(FD_ISSET(udpsock,&rfds)) {
                struct sockaddr_in cliaddr;
                int len = sizeof(cliaddr);
                numByte = recvfrom(udpsock, udpBuf, DATAGRAM_SIZE, 0, (struct sockaddr *) &cliaddr, &len);
                Datagram* datagram = createDatagram(udpBuf);
                SocketEnvironment* sockEnv = getAddressBookEntry(datagram->uid);
                int sourceIndex = (sockEnv->gameEnvironment->playerSocket[0] == datagram->uid) ? 0 : 1;
                int destIndex = (sourceIndex == 0) ? 1 : 0;
                if (datagram->ordering == 0) {
                    sockEnv->gameEnvironment->playerUdpSocket[sourceIndex] = udpsock;
                    sockEnv->gameEnvironment->playerUdpSockAddr[sourceIndex] = cliaddr;
                    sockEnv->gameEnvironment->udpSockAddrLen[sourceIndex] = len;
                }
                if(sockEnv->gameEnvironment->playerUdpSocket[0] == -1 || sockEnv->gameEnvironment->playerUdpSocket[1] == -1) {
                    continue;
                }
                if (datagram->ordering >= sockEnv->gameEnvironment->nextUdpDatagram[sourceIndex]) {
                    sockEnv->gameEnvironment->nextUdpDatagram[sourceIndex] = (datagram->ordering)+1;
                    sendto(sockEnv->gameEnvironment->playerUdpSocket[destIndex], (datagram->payload), 5000, 0, (struct sockaddr*) &sockEnv->gameEnvironment->playerUdpSockAddr[destIndex], sockEnv->gameEnvironment->udpSockAddrLen[destIndex]);
                }
                free(datagram);
            }
        }
    }
    return EXIT_SUCCESS;
}
