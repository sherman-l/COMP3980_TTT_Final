//
// Created by sherman on 2021-04-14.
//
#include <dc_fsm/fsm.h>

#ifndef COMP3980_TTT_FINAL_GENERALSERVERFUNCTIONS_H
#define COMP3980_TTT_FINAL_GENERALSERVERFUNCTIONS_H

uint8_t validateGameId(uint8_t gameId);

typedef struct Game{
    uint8_t id;
//    struct state_transition transitions[];
} Game;

struct Game SupportedGames[] = {
        {1},
        {2},
        {0}
};

uint8_t validateGameId(uint8_t gameId) {
    int index = 0;
    while(SupportedGames[index].id != 0) {
        printf("\nAt index: %d, %d\n", index, SupportedGames[index].id);
        printf("gameId: %d\n", gameId);
        if(gameId == SupportedGames[index].id) {
            return 10;
        }
        index++;
    }
    return 32;
}

#endif //COMP3980_TTT_FINAL_GENERALSERVERFUNCTIONS_H
