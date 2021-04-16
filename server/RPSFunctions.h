//
// Created by sherman on 2021-04-15.
//

typedef enum {
    ROCK = 1,
    PAPER,
    SCISSORS
} RPSMoves;

#ifndef DCFSM_RPSFUNCTIONS_H
#define DCFSM_RPSFUNCTIONS_H
#define RPS_MOVE_INDEX 7
#include "generalServerResources.h"

bool RPSMakeMove(SocketEnvironment* sockEnv);
void RPSCheckEnd(SocketEnvironment* sockEnv);
void RPSResult(SocketEnvironment* sockEnv);
void RPSNotifyEnd(SocketEnvironment* sockEnv);

bool RPSMakeMove(SocketEnvironment* sockEnv) {
    int move = *sockEnv->packet[RPS_MOVE_INDEX];
    int player = (sockEnv->gameEnvironment->playerSocket[PLAYERONE] == sockEnv->fd) ? 0 : 1;
    if(sockEnv->gameEnvironment->board[player] == -1) {
        sockEnv->gameEnvironment->board[player] = move;
        RPSCheckEnd(sockEnv);
        return true;
    }
    return false;
}

void RPSCheckEnd(SocketEnvironment* sockEnv) {
    bool end = true;
    for (int i = 0; i < 2; i++) {
        if (sockEnv->gameEnvironment->board[i] == -1) {
            end = false;
            break;
        }
    }
    sockEnv->gameEnvironment->end = end;
    return;
}

void RPSResult(SocketEnvironment* sockEnv) {
    int* board = sockEnv->gameEnvironment->board;
    if(board[0] == board[1]) {
        sockEnv->gameEnvironment->result = -1;
        return;
    }
    switch(board[0]) {
        case ROCK:
            if (board[1] == SCISSORS){
                sockEnv->gameEnvironment->result = PLAYERONE;
                return;
            }
            if (board[1] == PAPER) {
                sockEnv->gameEnvironment->result = PLAYERTWO;
                return;
            }
            break;
        case PAPER:
            if (board[1] == ROCK){
                sockEnv->gameEnvironment->result = PLAYERONE;
                return;
            }
            if (board[1] == SCISSORS) {
                sockEnv->gameEnvironment->result = PLAYERTWO;
                return;
            }
            break;
        case SCISSORS:
            if (board[1] == PAPER){
                sockEnv->gameEnvironment->result = PLAYERONE;
                return;
            }
            if (board[1] == ROCK) {
                sockEnv->gameEnvironment->result = PLAYERTWO;
                return;
            }
            break;
    }
}
void RPSNotifyEnd(SocketEnvironment* sockEnv) {
    int* board = sockEnv->gameEnvironment->board;
    uint8_t** response = malloc(sizeof(uint8_t*) * 5);
    for(int i = 0; i < 5; i++) {
        response[i] = malloc(sizeof(uint8_t) * 2);
    }
    *response[0] = (uint8_t) 20;
    *response[1] = (uint8_t) 3;
    *response[2] = (uint8_t) 2;
    switch(sockEnv->gameEnvironment->result) {
        case (PLAYERONE):
            *response[3] = WIN;
            break;
        case (PLAYERTWO):
            *response[3] = LOSS;
            break;
        default:
            *response[3] = TIE;
            break;
    }

    *response[4] = (uint8_t) board[1];
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
    *response[4] = (uint8_t) board[0];
    for (int i = 0; i < 5; i++) {
        write(sockEnv->gameEnvironment->playerSocket[1], response[i],1);
    }
    free(response);
}

#endif //DCFSM_RPSFUNCTIONS_H
