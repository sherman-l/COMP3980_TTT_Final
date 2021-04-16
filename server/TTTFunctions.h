//
// Created by sherman on 2021-04-15.
//
#include "generalServerResources.h"
#define TTT_BOARD_SIZE 9
#define TTT_MOVE_INDEX 7
#ifndef DCFSM_TTTFUNCTIONS_H
#define DCFSM_TTTFUNCTIONS_H
typedef enum {
     X = 1,
     O
} Team;
bool TTTMakeMove(SocketEnvironment* sockEnv);
void TTTCheckEnd(SocketEnvironment* sockEnv);
void TTTnotifyOpponentMove(SocketEnvironment* sockEnv);
void TTTnotifyEnd(SocketEnvironment* sockEnv);

bool TTTMakeMove(SocketEnvironment* sockEnv) {
    printf("In TTT MakeMove\n");
    int move = *sockEnv->packet[TTT_MOVE_INDEX];
    if(sockEnv->gameEnvironment->board[move] == -1) {
        sockEnv->gameEnvironment->board[move] = (sockEnv->gameEnvironment->playerSocket[PLAYERONE] == sockEnv->fd) ?
        X : O;
    } else {
        return false;
    }
    TTTCheckEnd(sockEnv);
    for(int i = 0; i < TTT_BOARD_SIZE; i++) {
        printf("%d \t",sockEnv->gameEnvironment->board[i]);
        if (((i+1) % 3) == 0) {
            printf("\n");
        }
    }
    return true;
}

void TTTCheckEnd(SocketEnvironment* sockEnv) {
    int* board = sockEnv->gameEnvironment->board;
    for(int i = 0; i < 3; i++) {
        //check horizontal
        if((board[i*3] != -1) && (board[i*3] == board[(i*3)+1]) && (board[(i*3)+1] == board[(i*3)+2])) {
            printf("horizontal check clear!\n");
            sockEnv->gameEnvironment->result = board[i];
            sockEnv->gameEnvironment->end = true;
            return;
        }
        //check vertical
        if((board[i] != -1) && (board[i] == board[i+3]) && (board[i+3] == board[i+6])) {
            printf("vertical check clear!\n");
            sockEnv->gameEnvironment-> result = board[i];
            sockEnv->gameEnvironment->end = true;
            return;
        }
    }
    //checkDiagonal
    if((board[0] != -1) && (board[0] == board[4]) && (board[4] == board[8])){
        printf("diagonal check clear!\n");
        sockEnv->gameEnvironment-> result = board[0];
        sockEnv->gameEnvironment->end = true;
        return;
    }
    if((board[2] != -1) && (board[2] == board[4]) && (board[4] == board[6])){
        printf("diagonal check clear!\n");
        sockEnv->gameEnvironment-> result = board[2];
        sockEnv->gameEnvironment->end = true;
        return;
    }
    for(int i = 0; i < TTT_BOARD_SIZE; i++) {
        if(board[i] == -1) {
            printf("spaces still unfilled! no tie!\n");
            return;
        }
    }
    sockEnv->gameEnvironment->result = -1;
    sockEnv->gameEnvironment->end = true;
}

void TTTnotifyOpponentMove(SocketEnvironment* sockEnv) {
    uint8_t** response = malloc(sizeof(uint8_t*) * 4);
    for(int i = 0; i < 4; i++) {
        response[i] = malloc(sizeof(uint8_t) * 2);
    }
    *response[0] = (uint8_t) 20;
    *response[1] = (uint8_t) 2;
    *response[2] = (uint8_t) 1;
    *response[3] = (uint8_t) *sockEnv->packet[TTT_MOVE_INDEX];
    int opponentFd = (sockEnv->gameEnvironment->playerSocket[0] == sockEnv->fd) ? sockEnv->gameEnvironment->playerSocket[1] : sockEnv->gameEnvironment->playerSocket[0];
    for (int i = 0; i < 4; i++) {
        write(opponentFd, response[i], 1);
    }
    free(response);
}

void TTTnotifyEnd(SocketEnvironment* sockEnv) {
    uint8_t** response = malloc(sizeof(uint8_t*) * 5);
    for(int i = 0; i < 5; i++) {
        response[i] = malloc(sizeof(uint8_t) * 2);
    }
    *response[0] = (uint8_t) 20;
    *response[1] = (uint8_t) 3;
    *response[2] = (uint8_t) 2;
    switch(sockEnv->gameEnvironment->result) {
        case PLAYERONE:
            *response[3] = WIN;
            break;
        case PLAYERTWO:
            *response[3] = LOSS;
            break;
        default:
            *response[3] = TIE;
            break;
    }

    *response[4] = (uint8_t) *sockEnv->packet[TTT_MOVE_INDEX];
    for (int i = 0; i < 5; i++) {
        write(sockEnv->gameEnvironment->playerSocket[0], response[i], 1);
    }
    switch (*response[3]) {
        case WIN:
            *response[3] = LOSS;
            break;
        case LOSS:
            *response[3] = WIN;
            break;
        case TIE:
            break;
    }
    for (int i = 0; i < 5; i++) {
        write(sockEnv->gameEnvironment->playerSocket[1], response[i],1);
    }
    free(response);
}

#endif //DCFSM_TTTFUNCTIONS_H
