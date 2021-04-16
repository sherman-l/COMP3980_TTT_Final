//
// Created by sherman on 2021-04-15.
//
#include <dc_fsm/fsm.h>
#include "TTTFunctions.h"
#include "RPSFunctions.h"

#ifndef DCFSM_GENERALSERVERFUNCTIONS_H
#define DCFSM_GENERALSERVERFUNCTIONS_H
GameEnvironment* createGameEnvironment(int gameType);

void validateGameId(uint8_t gameId, uint8_t *response, int* gameType);
void confirmRuleset(SocketEnvironment *socketEnv);
void confFunction(SocketEnvironment* socketEnv);
void infoFunction(SocketEnvironment* socketEnv);
void metaFunction(SocketEnvironment* socketEnv);
void gameFunction(SocketEnvironment* socketEnv);
void makeMove(SocketEnvironment* socketEnv);
void invalidRequest(SocketEnvironment* socketEnv);
void joinGame(SocketEnvironment* socketEnv, GameEnvironment** lobby);
void startGame(GameEnvironment* gameEnv);
void clearPacket(SocketEnvironment* socketEnv);
void acknowledgeRequest(SocketEnvironment* socketEnv) {
    uint8_t** response = malloc(sizeof(uint8_t*) * 3);
    for(int i = 0; i < 3; i++) {
        response[i] = malloc(sizeof(uint8_t) * 1);
    }
    *response[0] = (uint8_t) 10;
    *response[1] = (uint8_t) 1;
    *response[2] = (uint8_t) 0;
    for (int i = 0; i < 3; i++) {
        write(socketEnv->fd, response[i], 1);
    }
    free(response);
}

void confFunction(SocketEnvironment* socketEnv) {
    switch (socketEnv->packetOptions.reqContext) {
        case CONRULE:
            confirmRuleset(socketEnv);
            break;
        default:
            invalidRequest(socketEnv);
    }
}

void infoFunction(SocketEnvironment* socketEnv) {

}

void metaFunction(SocketEnvironment* socketEnv) {

}

void gameFunction(SocketEnvironment* socketEnv) {
    printf("in Game Function\n");
    switch (socketEnv->packetOptions.reqContext) {
        case MAKMOV:
            makeMove(socketEnv);
            break;
        default:
            invalidRequest(socketEnv);
    }
}

void invalidRequest(SocketEnvironment* socketEnv) {

}

void makeMove(SocketEnvironment* socketEnv) {
    printf("In make Move\n");
    if(socketEnv->gameEnvironment->gameType == TTT) {
        if(TTTMakeMove(socketEnv)) {
            acknowledgeRequest(socketEnv);
            if(socketEnv->gameEnvironment->end) {
                TTTnotifyEnd(socketEnv);
            } else {
                TTTnotifyOpponentMove(socketEnv);
            }

        } else {
            invalidRequest(socketEnv);
        }
    }
    if(socketEnv->gameEnvironment->gameType == RPS) {
        if(RPSMakeMove(socketEnv)) {
            acknowledgeRequest(socketEnv);
            if(socketEnv->gameEnvironment->end) {
                RPSResult(socketEnv);
                RPSNotifyEnd(socketEnv);
            }
        } else {
            invalidRequest(socketEnv);
        }
    }
}

void joinGame(SocketEnvironment* socketEnv, GameEnvironment** lobby) {
    printf("in join game, fd: %d\n", socketEnv->fd);
    socketEnv->gameEnvironment = *lobby;
    socketEnv->gameEnvironment->playerSocket[socketEnv->gameEnvironment->players] = socketEnv->fd;
    socketEnv->gameEnvironment->players++;
    socketEnv->generalState = PLAYGAME;
    if (socketEnv->gameEnvironment->players == 2) {
        GameEnvironment* newLobby = createGameEnvironment(socketEnv->gameEnvironment->gameType);
        *lobby = newLobby;
        startGame(socketEnv->gameEnvironment);
    }
}

void startGame(GameEnvironment* gameEnv) {
    printf("in start game\n");
    uint8_t** response = malloc(sizeof(uint8_t*) * 5);
    for(int i = 0; i < 5; i++) {
        response[i] = malloc(sizeof(uint8_t) * 2);
    }
    *response[0] = (uint8_t) 20;
    *response[1] = (uint8_t) 1;
    if(gameEnv->gameType == TTT) {
        *response[2] = (uint8_t) 1;
        *response[3] = (uint8_t) 1;
        for(int i = 0; i < 4; i++){
            write(gameEnv->playerSocket[0], response[i], 1);
        }
        *response[3] = (uint8_t) 2;
        for(int i = 0; i < 4; i++) {
            write(gameEnv->playerSocket[1], response[i], 1);
        }
    } else if (gameEnv->gameType == RPS) {
        *response[2] = (uint8_t)0;
        for(int i = 0; i < 3; i++) {
            write(gameEnv->playerSocket[0], response[i], 1);
            write(gameEnv->playerSocket[1], response[i], 1);
        }
    }
    free(response);
}

void readMessageType(SocketEnvironment* socketEnv) {
    printf("in readMessageType\n");
    switch(socketEnv->packetOptions.reqType) {
        case CONFIRMATION:
            confFunction(socketEnv);
            break;
        case INFORMATION:
            infoFunction(socketEnv);
            break;
        case METAACTION:
            metaFunction(socketEnv);
            break;
        case GAMEACTION:
            gameFunction(socketEnv);
            break;
        default:
            invalidRequest(socketEnv);
    }
}

GameEnvironment* createGameEnvironment(int gameType){
    GameEnvironment* gameEnvironment = malloc(sizeof(GameEnvironment));
    gameEnvironment->players = 0;
    gameEnvironment->gameType = gameType;
    gameEnvironment->end = false;
    switch(gameType) {
        case TTT:
            gameEnvironment->board = malloc(sizeof(int) * TTT_BOARD_SIZE);
            for(int i = 0; i < TTT_BOARD_SIZE; i++) {
                gameEnvironment->board[i] = -1;
            }
            break;
        case RPS:
            gameEnvironment->board = malloc(sizeof(int) * 2);
            for (int i = 0; i < 2; i++) {
                gameEnvironment->board[i] = -1;
            }
            break;
    }
    return gameEnvironment;
}

void validateGameId(uint8_t gameId, uint8_t *response, int* gameType) {
    int index = 0;
    while(SupportedGames[index].id != 0) {
        printf("\nAt index: %d, %d\n", index, SupportedGames[index].id);
        printf("gameId: %d\n", gameId);
        if(gameId == SupportedGames[index].id) {
            *response = (uint8_t) SUCCESS_CODE;
            *gameType = SupportedGames[index].id;
            return;
        }
        index++;
    }
    *response = (uint8_t) INVALID_TYPE;
    return;
}


void confirmRuleset(SocketEnvironment *socketEnv) {
    for (int i = 0; i < socketEnv->byteCount; i++) {
        printf("packet index: %d, value: %d\n", i, *socketEnv->packet[i]);
    }
    uint8_t** response = malloc(sizeof(uint8_t*) * 10);
    for(int i = 0; i < 7; i++) {
        response[i] = malloc(sizeof(uint8_t) * 3);
    }
    int gameType = 0;
    validateGameId(*socketEnv->packet[8], response[0], &gameType);
    if (*response[0] == SUCCESS_CODE) {
        socketEnv->gameType = gameType;
    }
    printf("response[0] : %d\n", *response[0]);
    *response[1] = 1;
    *response[2] = 4;
    for(int i = 3; i < 6; i++){
        *response[i] = 0;
    }
    *response[6] = (uint8_t) socketEnv->fd;
    for (int i = 0; i < 7; i++) {
        write(socketEnv->fd, response[i], 1);
    }
    free(response);
}

void clearPacket(SocketEnvironment* socketEnv) {
    for(int i = 0; i < MAX_MSG_SIZE; i++){
        *socketEnv->packet[i] = 0;
        socketEnv->byteCount = 0;
        socketEnv->packetOptions.reqContext = 0;
        socketEnv->packetOptions.uid = 0;
        socketEnv->packetOptions.reqType = 0;
        socketEnv->packetOptions.payloadLength = 0;
    }
}

#endif //DCFSM_GENERALSERVERFUNCTIONS_H
