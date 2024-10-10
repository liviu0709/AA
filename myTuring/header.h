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

void destroyState(State*, int);
void destroyStateMain(stateMain*, int);
void initTape(turingMachine*, char*);
void resizeTape(turingMachine*);
void moveHead(turingMachine*, char);
void printTape(turingMachine*, char*);