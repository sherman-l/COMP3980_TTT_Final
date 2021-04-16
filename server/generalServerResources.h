//
// Created by sherman on 2021-04-14.
//
#include <dc_fsm/fsm.h>

#ifndef COMP3980_TTT_FINAL_GENERALSERVERRESOURCES_H
#define COMP3980_TTT_FINAL_GENERALSERVERRESOURCES_H
#define SUCCESS_CODE 10
#define INVALID_TYPE 32
#define MAX_MSG_SIZE 18
#define BUF_SIZE 1000

typedef struct PacketOptions{
    uint32_t uid;
    uint8_t reqType;
    uint8_t reqContext;
    uint8_t payloadLength;
    uint8_t protocolVersion;
} PacketOptions;


typedef struct GameEnvironment {
    struct dc_fsm_environment common;
    int players;
    int playerSocket[2];
    int* board;
    int gameType;
    int result;
    bool end;
} GameEnvironment;

typedef struct Game{
    uint8_t id;
//    struct state_transition transitions[];
} Game;


typedef struct SocketEnvironment {
    int fd;
    uint8_t** packet;
    int byteCount;
    int generalState;
    int gameType;
    struct PacketOptions packetOptions;
    struct GameEnvironment* gameEnvironment;
} SocketEnvironment;

typedef enum {
    WIN = 1,
    LOSS,
    TIE
} Result;
typedef enum
{
    SELECTGAME = 0,
    PLAYGAME
} GeneralState;

typedef enum {
    TTT = 1,
    RPS = 2
} Games;

typedef enum {
    PLAYERONE = 0,
    PLAYERTWO
} Player;

typedef enum {
    CONFIRMATION = 1,
    INFORMATION,
    METAACTION,
    GAMEACTION
} RequestMessageType;

typedef enum {
    CONRULE = 1
} ConfirmationContext;

typedef enum {
    MAKMOV = 1
} GameActionContext;

typedef enum {
    QUIT = 1
} MetaActionContext;

struct Game SupportedGames[] = {
        {TTT},
        {RPS},
        {0}
};

#endif //COMP3980_TTT_FINAL_GENERALSERVERRESOURCES_H
