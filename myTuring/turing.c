#include "header.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

void destroyState(State *states, int statesCount) {
    for ( int i = 0 ; i < statesCount ; i++ ) {
        free(states[i].state);
        free(states[i].newState);
    }
    free(states);
}

void destroyStateMain(stateMain *mainState, int mainStatesCount) {
    for ( int i = 0 ; i < mainStatesCount ; i++ ) {
        free(mainState[i].shifts);
    }
    free(mainState);
}

void initTape(turingMachine *tm, char *input) {
    tm->tape = (char*)malloc(100 * sizeof(char));
    memset(tm->tape, '_', 100);
    tm->tapeSize = strlen(input);
    strcpy(tm->tape, input);
    tm->head = 0;
}

void resizeTape(turingMachine *tm) {
    char *newTape = (char*)malloc(3 * tm->tapeSize*sizeof(char));
    memset(newTape, '_', 3 * tm->tapeSize);
    strncpy(newTape + tm->tapeSize, tm->tape, tm->tapeSize);
    free(tm->tape);
    tm->tape = newTape;
    tm->head += tm->tapeSize;
    tm->tapeSize *= 3;

}

void moveHead(turingMachine *tm, char direction) {
    if (direction == '<') {
        if (tm->head == 0) {
            resizeTape(tm);
        }
        tm->head--;
    } else {
        if (tm->head == tm->tapeSize-1) {
            resizeTape(tm);
        }
        tm->head++;
    }
}

void printTape(turingMachine *tm, char *nextState) {
    printf("State: %s -> %s\n", tm->currentState, nextState);
    for (int i = 0; i < tm->tapeSize; i++) {
        if (i == tm->head) {
            printf("[%c] ", tm->tape[i]);
        } else {
            printf("%c ", tm->tape[i]);
        }
    }
    printf("\n\n");
}
