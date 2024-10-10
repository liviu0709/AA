#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "header.h"

// Turing machine state structure
typedef struct {
    char *state;
    char symbol;
    char newSymbol;
    char direction;
    char *newState;
} State;

typedef struct {
    char symbol;
    char newSymbol;
    char direction;
    char* newState;
} Shift;

typedef struct {
    char *state;
    Shift *shifts;
    int shiftCount;
    int maxShifts;
} stateMain;

typedef struct {
    char *tape;
    int head, tapeSize;
    char *currentState;
} turingMachine;

void destroyState(State *states, int statesCount) {
    for ( int i = 0 ; i < statesCount ; i++ ) {
        free(states[i].state);
        free(states[i].newState);
    }
    free(states);
}

void printStateMain(stateMain *mainState, int mainStatesCount) {
    for ( int j = 0 ; j < mainStatesCount ; j++ ) {
        printf("State: %s\n", mainState[j].state);
        for (int i = 0; i < mainState[j].shiftCount; i++) {
            printf("Symbol: %c\n", mainState[j].shifts[i].symbol);
            printf("New symbol: %c\n", mainState[j].shifts[i].newSymbol);
            printf("Direction: %c\n", mainState[j].shifts[i].direction);
            printf("New state: %s\n", mainState[j].shifts[i].newState);
        }
    }
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

void showState(State state) {
    printf("State: %s\n", state.state);
    printf("Symbol: %c\n", state.symbol);
    printf("New symbol: %c\n", state.newSymbol);
    printf("Direction: %c\n", state.direction);
    printf("New state: %s\n", state.newState);
    printf("\n");
}

int main() {
    char *name, *init, *accept;
    State *states = (State*)malloc(100*sizeof(State));
    int statesCount = 0;
    int maxStates = 5;
    name = (char*)malloc(100*sizeof(char));
    init = (char*)malloc(100*sizeof(char));
    accept = (char*)malloc(100*sizeof(char));
    FILE *in = fopen("in", "r");
    // Check for potential comments
    while (true) {
        fgets(name, 100, in);
        if (name[0] != '\n' ) {
            if ( name[0] != '/' && name[1] != '/') {
                break;
            }
        }
    }
    fgets(init, 100, in);
    fgets(accept, 100, in);
    // Parse data
    char *aux = (char*)malloc(100*sizeof(char));
    strcpy(aux, name + 6);
    strcpy(name, aux);
    // strcpy(name, name+6);
    strcpy(aux, init+6);
    // strcpy(init, init+6);
    strcpy(init, aux);
    strcpy(aux, accept+8);
    // strcpy(accept, accept+8);
    strcpy(accept, aux);
    free(aux);
    name[strlen(name)-1] = '\0';
    init[strlen(init)-1] = '\0';
    accept[strlen(accept)-1] = '\0';

    char *emptyLine = (char*)malloc(100*sizeof(char));
    char *line1 = (char*)malloc(100*sizeof(char));
    char *line2 = (char*)malloc(100*sizeof(char));

    // Read states
    while (true) {
        fgets(emptyLine, 100, in);
        if ( fgets(line1, 100, in) == NULL ) {
            break;
        }
        fgets(line2, 100, in);

        // if line2 is the last line from the file ...
        // add \n to it so parse works correctly
        if ( feof(in) ) {
            line2[strlen(line2)] = '\n';
            line2[strlen(line2)+1] = '\0';
        }

        // Parse data
        char *stateName = (char*)malloc(100*sizeof(char));
        char *newStateName = (char*)malloc(100*sizeof(char));
        states[statesCount].symbol = line1[strlen(line1)-2];
        states[statesCount].newSymbol = line2[strlen(line2)-4];
        states[statesCount].direction = line2[strlen(line2)-2];
        strchr(line1, ',')[0] = '\0';
        strcpy(stateName, line1);
        states[statesCount].state = stateName;
        strchr(line2, ',')[0] = '\0';
        strcpy(newStateName, line2);
        states[statesCount].newState = newStateName;
        statesCount++;
        // showState(states[statesCount-1]);
        if (statesCount == maxStates) {
            states = realloc(states, 2*maxStates*sizeof(State));
            maxStates *= 2;
        }
    }

    // Reorganize states
    // To improve searching efficiency
    int mainStatesCount = 0;
    int maxMainStates = 5;
    char **mainStates = (char**)malloc(maxMainStates*sizeof(char*));

    for ( int i = 0 ; i < statesCount ; i++ ) {
        bool found = false;
        for ( int j = 0 ; j < mainStatesCount ; j++ ) {
            if ( strcmp(mainStates[j], states[i].state) == 0 ) {
                found = true;
                break;
            }
        }
        if ( !found ) {
            mainStates[mainStatesCount] = states[i].state;
            mainStatesCount++;
            if ( mainStatesCount == maxMainStates ) {
                mainStates = realloc(mainStates, 2*maxMainStates*sizeof(char*));
                maxMainStates *= 2;
            }
        }
    }

    // Print vector pointer array
    for ( int i = 0 ; i < mainStatesCount ; i++ ) {
        printf("mainStates[%d] = %s\n", i, mainStates[i]);
    }

    // Create main states
    stateMain *mainStatesArray = (stateMain*)malloc(mainStatesCount*sizeof(stateMain));
    for ( int i = 0 ; i < mainStatesCount ; i++ ) {
        mainStatesArray[i].state = mainStates[i];
        mainStatesArray[i].shifts = (Shift*)malloc(5*sizeof(Shift));
        mainStatesArray[i].shiftCount = 0;
        mainStatesArray[i].maxShifts = 5;
        // Link shifts to main states
        for ( int j = 0 ; j < statesCount ; j++ ) {
            if ( strcmp(states[j].state, mainStates[i]) == 0 ) {
                mainStatesArray[i].shifts[mainStatesArray[i].shiftCount].symbol = states[j].symbol;
                mainStatesArray[i].shifts[mainStatesArray[i].shiftCount].newSymbol = states[j].newSymbol;
                mainStatesArray[i].shifts[mainStatesArray[i].shiftCount].direction = states[j].direction;

                mainStatesArray[i].shifts[mainStatesArray[i].shiftCount].newState = states[j].newState;

                mainStatesArray[i].shiftCount++;
                if ( mainStatesArray[i].shiftCount == mainStatesArray[i].maxShifts ) {
                    mainStatesArray[i].shifts = realloc(mainStatesArray[i].shifts, 2 * mainStatesArray[i].maxShifts * sizeof(Shift));
                    mainStatesArray[i].maxShifts *= 2;
                }
            }
        }
    }

    // Print main states
    // printStateMain(mainStatesArray, mainStatesCount);

    // Read input string
    char *input = (char*)malloc(100*sizeof(char));
    printf("Enter input string: ");
    fgets(input, 100, stdin);
    // remove \n
    input[strlen(input)-1] = '\0';

    fclose(in);

    printf("input ... : %s \n", input);

    turingMachine tm;
    tm.currentState = init;
    int stepCnt = 0;

    initTape(&tm, input);

    // Run the Turing machine
    bool letMeOut = false;
    while (true) {
        bool found = false;
        if (letMeOut) {
            break;
        }
        // Optimised search
        // Search for the current state in the main states
        for ( int i = 0 ; i < mainStatesCount ; i++ ) {
            if ( strcmp(mainStatesArray[i].state, tm.currentState) == 0 ) {
                // Search for the current symbol in the current state
                for ( int j = 0 ; j < mainStatesArray[i].shiftCount ; j++ ) {
                    if ( mainStatesArray[i].shifts[j].symbol == tm.tape[tm.head] ) {
                        stepCnt++;
                        found = true;
                        printTape(&tm, mainStatesArray[i].shifts[j].newState);
                        tm.tape[tm.head] = mainStatesArray[i].shifts[j].newSymbol;
                        tm.currentState = mainStatesArray[i].shifts[j].newState;
                        if ( strcmp(tm.currentState, accept) == 0 ) {
                            printf("Accepted\n");
                            letMeOut = true;
                            break;
                        }
                        if ( strcmp(tm.currentState, "qReject") == 0 ) {
                            printf("Rejected\n");
                            letMeOut = true;
                            break;
                        }
                        if ( strcmp(tm.currentState, "H") == 0 ) {
                            printf("Halted\n");
                            letMeOut = true;
                            break;
                        }
                        if ( mainStatesArray[i].shifts[j].direction != '-' ) {
                            moveHead(&tm, mainStatesArray[i].shifts[j].direction);
                        }
                        break;
                    }
                }
            }
        }
        if (!found) {
            printf("No transition found for state %s and symbol %c\n", tm.currentState, tm.tape[tm.head]);
            break;
        }
    }

    // printTape(&tm, NULL);
    printf("Steps: %d\n", stepCnt);
    printf("name: %s\n", name);
    printf("init: %s\n", init);
    printf("accept: %s\n", accept);
    destroyStateMain(mainStatesArray, mainStatesCount);
    destroyState(states, statesCount);
    free(tm.tape);
    free(input);
    free(name);
    free(init);
    free(accept);
    free(emptyLine);
    free(line1);
    free(line2);
    free(mainStates);

    return 0;
}