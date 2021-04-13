//
// Created by sherman on 2021-02-15.
//
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CODELENGTH 3
#define BUF_SIZE 1000


#ifndef DCFSM_SOCKETREADWRITE_H
#define DCFSM_SOCKETREADWRITE_H
int readClientResponse(int client_socket, char* buffer, char* code);

int readClientResponse(int client_socket, char* buffer, char* code){
    char* comparison = malloc(sizeof(char) * 5);
    read(client_socket, buffer, BUF_SIZE);
    strncpy(comparison, buffer, CODELENGTH);
    comparison[CODELENGTH+1] = '\0';
    if(strcmp(comparison, code) == 0) {
        fprintf(stdout,"comparison success! %s == %s \n\0", buffer, code);
        free(comparison);
        return 1;
    } else {
        return 0;
    }
}


#endif //DCFSM_SOCKETREADWRITE_H
