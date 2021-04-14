//
// Created by Sherman Lok and Jeffrey Huynh on 2021-02-10.
// Server represented by a Finite State Machine to run a Tic Tac Toe game
//

#include <sys/types.h>
#include <dc_fsm/fsm.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <err.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <ctype.h>
#include "socketReadWrite.h"

#define BACKLOG 4
#define ROWS 3
#define COLUMNS 3

#define MAX_CLIENTS 10
#define UID_BYTE_LENGTH 4
#define PAYLOAD_BYTE_LENGTH 1
#define PAYLOAD_LENGTH_INDEX 6
#define PAYLOAD_START_INDEX 7
#define MAX_MSG_SIZE 11


typedef struct PacketOptions{
    uint32_t uid;
    uint8_t reqType;
    uint8_t reqContext;
    uint8_t payloadLength;
} PacketOptions;

/**
 * Struct holding result and state of board
 */
typedef struct GameEnvironment {
    struct dc_fsm_environment common;
    int players;
    int playerSocket[2];
    uint8_t** packet;
    int byteCount;
    struct PacketOptions packetOptions;
} GameEnvironment;

typedef struct SocketEnvironment {
    int fd;
    struct GameEnvironment* gameEnvironment;
} SocketEnvironment;

SocketEnvironment* addressBook;

typedef struct GameType {
    uint8_t code;
    struct dc_fsm_environment fsm;
} GameType;

int serverConnections = 0;

/** Constructors **/
GameEnvironment* createGameEnvironment(int fd);

/** Getter **/
GameEnvironment* getGameEnvironment(int fd);

/** Helper Functions **/
int prompt_port();
GameEnvironment* getGameEnvironment(int fd);
void storeData(GameEnvironment* gameEnv, uint8_t* data, int fd);
void fillOptions(GameEnvironment* gameEnv);
void printPacket(GameEnvironment* gameEnv);
void clearData(GameEnvironment* gameEnv);
void parseRequest(GameEnvironment* gameEnv);
void sendResponse(GameEnvironment* gameEnv, int fd);

void sendResponse(GameEnvironment* gameEnv, int fd) {
    uint8_t** sendToJeff = malloc(sizeof(uint8_t*) * 7);
    for(int i = 0; i < 7; i++) {
        sendToJeff[i] = malloc(sizeof(uint8_t));
    }
    *sendToJeff[0] = (uint8_t) 10;
    *sendToJeff[1] = (uint8_t) 1;
    *sendToJeff[2] = (uint8_t) 4;
    for(int i = 3; i < 6; i++){
        *sendToJeff[i] = 0;
    }
    *sendToJeff[6] = (uint8_t) fd;
    for (int i = 0; i < 7; i++) {
        write(fd, sendToJeff[i], 1);
    }
}

void clearData(GameEnvironment* gameEnv) {
    for(int i = 0; i < gameEnv->byteCount; i++) {
        gameEnv->packet[i] = 0;
    }
    gameEnv->byteCount = 0;
    gameEnv->packetOptions.uid = 0;
    gameEnv->packetOptions.payloadLength = 0;
    gameEnv->packetOptions.reqContext = 0;
    gameEnv->packetOptions.reqType = 0;
}

GameEnvironment* createGameEnvironment(int fd) {
    printf("inside createGameEnvironment\n");
    GameEnvironment* gameEnvironment = malloc(sizeof(struct GameEnvironment));
    gameEnvironment->playerSocket[0] = fd;
    gameEnvironment->players = 1;
    gameEnvironment->byteCount = 0;
    gameEnvironment->packet = malloc(sizeof(uint8_t*) * MAX_MSG_SIZE);
    gameEnvironment->packetOptions.uid = 0;
    gameEnvironment->packetOptions.reqType = 0;
    gameEnvironment->packetOptions.reqContext = 0;
    gameEnvironment->packetOptions.payloadLength = 0;
    return gameEnvironment;
}

void printPacket(GameEnvironment* gameEnv) {

}

void fillOptions(GameEnvironment* gameEnv) {
    int trackIndex = 0;
    for(int i = 0; i < UID_BYTE_LENGTH; i++, trackIndex++) {
        gameEnv->packetOptions.uid += *gameEnv->packet[trackIndex];
        if (i < UID_BYTE_LENGTH-1) {
            gameEnv->packetOptions.uid <<= 8;
        }
    }
    gameEnv->packetOptions.reqType = *gameEnv->packet[trackIndex++];
    gameEnv->packetOptions.reqContext = *gameEnv->packet[trackIndex++];
    gameEnv->packetOptions.payloadLength = *gameEnv->packet[trackIndex];
}

void storeData(GameEnvironment* gameEnv, uint8_t* data, int fd) {
 //   printf("Beginning of storeData: %d, ByteCount: %d\n", *data, gameEnv->byteCount);
    if (gameEnv->byteCount < (PAYLOAD_LENGTH_INDEX + gameEnv->packetOptions.payloadLength + 1)) {
        gameEnv->packet[gameEnv->byteCount] = data;
        printf("In storeData, storing: %d\n", *gameEnv->packet[gameEnv->byteCount]);
        (gameEnv->byteCount)++;
    }
    if(gameEnv->byteCount == (PAYLOAD_LENGTH_INDEX + 1)) {
        fillOptions(gameEnv);
    }
    if(gameEnv->byteCount == (PAYLOAD_LENGTH_INDEX + gameEnv->packetOptions.payloadLength + 1)) {
        sendResponse(gameEnv, fd);
        clearData(gameEnv);
    }
}


GameEnvironment* getGameEnvironment(int fd) {
    printf("inside getGameEnvironment\n");
    for(int i = 0; i < serverConnections; i++) {
        if(addressBook[i].fd == fd) {
            printf("gameEnvironment found for fd: %d", fd);
            return addressBook[i].gameEnvironment;
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


    int one = 1;
    struct sockaddr_in svr_addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        err(1, "can't open socket");

    int port = prompt_port();
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
        close(sock);
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
        fdmax = sock;

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
                    addressBook[i].fd = cfd;
                    if (i % 2 == 0) {
                        addressBook[i].gameEnvironment = createGameEnvironment(cfd);
                    } else {
                        addressBook[i].gameEnvironment = addressBook[i-1].gameEnvironment;
                        addressBook[i].gameEnvironment->players += 1;
                        addressBook[i].gameEnvironment->playerSocket[1] = cfd;
                    }
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
                    printf("Receiving buf: %d\n", *buf);
                    GameEnvironment* tempEnv = getGameEnvironment(temp);
                    printf("outside getGameEnvironment\n");
                    storeData(tempEnv, buf, temp);
                    printf("Outside storeData inserted data: %d\n", *tempEnv->packet[tempEnv->byteCount-1]);
                    printf("Bytecount: %d \n", tempEnv->byteCount);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
