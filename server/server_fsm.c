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
#define UID_BYTE 4
#define REQ_TYPE_BYTE 5
#define REQ_CONTEXT_BYTE 6
#define PAYLOAD_BYTE 7
#define RECVD_DATA_LENGTH 4
#define BLOCK_LENGTH 4



typedef struct PacketOptions{
    uint32_t uid;
    uint8_t reqType;
    uint8_t reqContext;
    uint8_t payloadLength;
} PacketOptions;

uint8_t grabByte(uint32_t* data) {
    uint8_t result = 0;
    result = result | *data;
    *data = *data >> 8;
    return result;
}

typedef struct SocketEnvironment {
    int fd;
    GameEnvironment* gameEnvironment;
} SocketEnvironment;

SocketEnvironment addressBook[MAX_CLIENTS];

/**
 * Struct holding result and state of board
 */
typedef struct {
    struct dc_fsm_environment common;
    int playerSocket[2];
    int players;
    uint32_t packetArray[4];
    int byteCount;
} GameEnvironment;


int serverConnections = 0;

/** Helper Functions **/
int prompt_port();
GameEnvironment* getGameEnvironment(int fd);
void storeData(GameEnvironment* gameEnv, uint32_t data);

//void storeData(GameEnvironment* gameEnv, uint32_t data) {
//    for (int i = 0; i < RECVD_DATA_LENGTH; i++, (gameEnv->byteCount)++) {
//
//        gameEnv->packetArray[(gameEnv->byteCount)/BLOCK_LENGTH] +=
//    }
//}


GameEnvironment* getGameEnvironment(int fd) {
    for(int i = 0; i < serverConnections; i++) {
        if(addressBook.fd == fd) {
            return addressBook.gameEnvironment;
        }
    }
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

    for(int i = 0; i < MAX_CLIENTS; i++) {
        socket_list[i] = 0;
    }
    int temp;
    fd_set rfds;
    int retval;
    int fdmax;
    int cfd;
    uint32_t* buf = malloc(sizeof(uint32_t));


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
                        printf("%s", buf);
                        fflush(stdout);
                    }
            }
        }
    }
    return EXIT_SUCCESS;
}
