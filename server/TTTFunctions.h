//
// Created by sherman on 2021-04-15.
//
#include "generalServerResources.h"
#ifndef DCFSM_TTTFUNCTIONS_H
#define DCFSM_TTTFUNCTIONS_H
typedef enum {
     X = 1,
     O
} Team;
bool TTTMakeMove(SocketEnvironment* sockEnv);
void TTTCheckEnd(SocketEnvironment* sockEnv);
void TTTnotifyOpponent(SocketEnvironment* sockEnv);

bool TTTMakeMove(SocketEnvironment* sockEnv) {
    int move = *sockEnv->packet[7];
    if(sockEnv->gameEnvironment->board[move] != -1) {
        sockEnv->gameEnvironment->board[move] = (sockEnv->gameEnvironment->playerSocket[PLAYERONE] == sockEnv->fd) ?
        PLAYERONE : PLAYERTWO;
    } else {
        return false;
    }
    TTTCheckEnd(sockEnv);
    return true;
}

void TTTCheckEnd(SocketEnvironment* sockEnv) {
    int* board = sockEnv->gameEnvironment->board;
    for(int i = 0; i < 3; i++) {
        //check horizontal
        if((board[i] != -1) && (board[i*3] == board[(i*3)+1]) && (board[(i*3)+1] == board[(i*3)+2])) {
            sockEnv->gameEnvironment->result = board[i];
            sockEnv->gameEnvironment->end = true;
            return;
        }
        //check vertical
        if((board[i] != -1) && (board[i] == board[i+3]) && (board[i+3] == board[i+6])) {
            sockEnv->gameEnvironment-> result = board[i];
            sockEnv->gameEnvironment->end = true;
            return;
        }
    }
    //checkDiagonal
    if((board[0] != -1) && (board[0] == board[4]) && (board[4] == board[8])){
        sockEnv->gameEnvironment-> result = board[0];
        sockEnv->gameEnvironment->end = true;
        return;
    }
    if((board[2] != -1) && (board[2] == board[4]) && (board[4] == board[6])){
        sockEnv->gameEnvironment-> result = board[2];
        sockEnv->gameEnvironment->end = true;
        return;
    }
    for(int i = 0; i < TTT_BOARD_SIZE; i++) {
        if(board[i] == -1) {
            return;
        }
    }
    sockEnv->gameEnvironment->result = -1;
    sockEnv->gameEnvironment->end = true;
}

void TTTnotifyOpponent(SocketEnvironment* sockEnv) {
    printf("in start game\n");
    uint8_t** response = malloc(sizeof(uint8_t*) * 5);
    for(int i = 0; i < 7; i++) {
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
        *response[3] = (uint8_t)0;
        for(int i = 0; i < 3; i++) {
            write(gameEnv->playerSocket[0], response[i], 1);
            write(gameEnv->playerSocket[1], response[i], 1);
        }
    }
}
}

#endif //DCFSM_TTTFUNCTIONS_H
