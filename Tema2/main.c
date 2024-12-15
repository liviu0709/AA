#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
typedef struct {
    int *vars;
    int nrVars;
    Clause* next;
    Clause* prev;
} Clause;

Clause* start = NULL;
Clause* stop = NULL;

void addClauseEnd(Clause* c) {
    if ( start == NULL ) {
        start = c;
        stop = c;
        return;
    }
    stop->next = c;
    c->prev = stop;
    stop = c;
}

void addClauseStart(Clause* c) {
    if ( start == NULL ) {
        start = c;
        stop = c;
        return;
    }
    start->prev = c;
    c->next = start;
    start = c;
}

typedef struct {
    int *vars;
    bool *option;
    int *evaluated;
    int *decisionLevel;
    int *previous;
} Vars;

static int goingDEEP;

void printClause(Clause *c) {
    printf("Nr vars in clause: %d...", c->nrVars);
    for ( int i = 0 ; i < c->nrVars ; i++ )
        printf("%d ", c->vars[i]);
    printf("\n");
}

bool *seen;
typedef struct {
    int decisionList[100];
    int decisionLevel;
    int index;
} LevelData;
LevelData levelData[100];
static int decisionListIndex;
int swapped[1000];

void printList() {
    printf("====================================\n");
    for ( int i = 0 ; i < decisionListIndex ; i++ ) {
        printf("Level: %d ", i);
        for ( int j = 0 ; j < levelData[i].index ; j++ )
            printf(" %d", levelData[i].decisionList[j]);
        printf("\n");
    }
    printf("====================================\n");
}

void addToList(int level, int var) {
    // levelData[level].decisionList[levelData[level].index] = var;
    // levelData[level].index++;
    for ( int i = 0 ; i < levelData[level].index ; i++ ) {
        if ( levelData[level].decisionList[i] == var )
            return;
        if ( levelData[level].decisionList[i] == -var ) {
            levelData[level].decisionList[i] = var;
            return;
        }
    }
    levelData[level].decisionList[levelData[level].index] = var;
    levelData[level].index++;
}

void removeFromList(int level, int var) {
    for ( int i = 0 ; i < levelData[level].index ; i++ ) {
        if ( levelData[level].decisionList[i] == var ) {
            for ( int j = i ; j < levelData[level].index - 1 ; j++ )
                levelData[level].decisionList[j] = levelData[level].decisionList[j + 1];
            levelData[level].index--;
            // printf("Removed %d from level %d\n", var, level);
            break;
        }
    }
}

void analyzeConflict(Clause *c, Vars *start, int nrVars, Clause *learnedClause, int *backjump, int *ok, int maxLevel) {
    int minLevel = __INT32_MAX__;
    // printf("Conflict clause: ");
    // printClause(c);
    for ( int i = 0 ; i < c->nrVars ; i++ ) {
        if ( start->evaluated[abs(c->vars[i]) - 1] != -1 ) {
                if ( start->evaluated[abs(c->vars[i]) - 1] < minLevel ) {
                minLevel = start->evaluated[abs(c->vars[i]) - 1];
            }
        }
    }

    if ( minLevel == __INT32_MAX__ ) {
        // printf("No decision level found\n");
        backjump = 0;
        *ok = false;
        return;
    }

    for ( int i = 0 ; i < nrVars ; i++ )
        seen[i] = false;

    memset(seen, false, nrVars * sizeof(bool));

    // printf("Decision levels: ");
    int learnedClauseIndex = 0;
    learnedClause->nrVars = 0;
    for ( int i = 0 ; i < nrVars ; i++ ) {
        if ( start->evaluated[i] != -1 && start->evaluated[i] >= minLevel && seen[i] == false && start->evaluated[i] <= maxLevel ) {
        // if ( decisionLevel[i] >= minLevel && seen[abs(decisionList[i]) - 1] == false && decisionLevel[i] <= maxLevel ) {
            learnedClause->vars[learnedClauseIndex] = start->option[i] ? -(i + 1) : +(i + 1);
            seen[i] = true;
            learnedClauseIndex++;
            learnedClause->nrVars++;
            if ( learnedClause->nrVars > 9 ) {
                // printf("Learned clause too long\n");
                *ok = 5;
                break;
            }
        }
        // printf(" %d", start->decisionLevel[i]);
    }
    // printf("\n");
    // printClause(learnedClause);
    learnedClause->vars[learnedClauseIndex] = 0;
    *backjump = minLevel;
    // printf("Min: %d, Max: %d\n", minLevel, maxLevel);

}

bool bktUnitPropagation(Vars *start, Clause *c, int nrClauses, int nrVars, int level, int *nrLearnedClauses, int goingUp) {
    int freeVarIndex = -1;
    // printList();
    // Check learned clauses
    if ( level < 0 )
        return false;

    // if ( goingUp )
        // printf("Going up\n");
    goingUp = 0;
    if ( !goingUp ) {
        // Unit clause propagation
        for ( bool unitClause = true ; unitClause ; ) {
            unitClause = false;
            for ( int i = 0 ; i < nrClauses + *nrLearnedClauses; i++ ) {
                bool clauseChecksOut = false;
                int freeVar = 0;
                int freeVarValue = 0;
                for ( int j = 0 ; j < c[i].nrVars ; j++ ) {
                    // Count free vars
                    if ( start->evaluated[abs(c[i].vars[j]) - 1] == -1 ) {
                        freeVar++;
                        freeVarIndex = abs(c[i].vars[j]) - 1;
                        freeVarValue = c[i].vars[j];
                    } else {
                        // Check if clause is already good
                        if ( c[i].vars[j] > 0 && start->option[c[i].vars[j] - 1] == true ) {
                            clauseChecksOut = true;
                            break;
                        }
                        if ( c[i].vars[j] < 0 && start->option[-c[i].vars[j] - 1] == false ) {
                            clauseChecksOut = true;
                            break;
                        }
                    }
                }
                if ( clauseChecksOut == false && freeVar == 0 ) {
                    // Conflict detected!
                    if ( *nrLearnedClauses > nrClauses )
                    // if ( *nrLearnedClauses > 5 )
                        return false;
                    // return false;
                    // if ( goingUp == 2 )
                        // return false;
                    int ok = true;
                    int backjump = 0;
                    analyzeConflict(&c[i], start, nrVars, &c[nrClauses + *nrLearnedClauses], &backjump, &ok, level);
                    // backjump--;
                    if ( ok == 5 ) {
                        // printf("Learned clause too long!\n");
                        // for ( int i  = 0 ; i < nrVars ; i++ ) {
                        //         if (start->evaluated[i] > backjump) {
                        //             start->evaluated[i] = -1;
                        //     }
                        // }
                        // return bktUnitPropagation(start, c, nrClauses, nrVars, backjump, nrLearnedClauses, 1);
                        return false;
                    }
                    if ( ok == false ) {
                        return false;
                        // printf("Conflict detected\n");
                        // return bktUnitPropagation(start, c, nrClauses, nrVars, backjump, nrLearnedClauses, 1);
                    }
                    if ( *nrLearnedClauses < nrClauses ) {
                        if ( swapped[abs(c[i].vars[0]) - 1] == 1 )
                            return false;
                        (*nrLearnedClauses)++;
                        // return false;
                        int oldLevelDec = start->evaluated[abs(c[i].vars[0]) - 1];
                        // printf("Evaluated level: %d\n", start->evaluated[abs(c[i].vars[0]) - 1]);
                        for ( int i  = 0 ; i < nrVars ; i++ ) {
                                if (start->evaluated[i] >= backjump) {
                                    start->evaluated[i] = -1;
                            }
                        }
                        // printf("Learned clause: \n");
                        // printf("Backjump: %d. Current level: %d\n", backjump, level);
                        // printClause(&c[i]);
                        // printClause(&c[nrClauses + *nrLearnedClauses - 1]);
                        // printList();
                        // start->evaluated[abs(c[i].vars[0]) - 1] = oldLevelDec;
                        // printf("Variable initial value: %d.", start->option[abs(c[i].vars[0]) - 1]);
                        // start->option[abs(c[i].vars[0]) - 1] = start->option[abs(c[i].vars[0]) - 1] ? false : true;
                        // swapped[abs(c[i].vars[0]) - 1] = 1;
                        // printf("Variable to modify: %d. Value: %d\n", c[i].vars[0], start->option[abs(c[i].vars[0]) - 1]);
                        goingDEEP++;
                        return bktUnitPropagation(start, c, nrClauses, nrVars, backjump, nrLearnedClauses, true);
                    }
                    return false;
                }
                // We can do propagation :)
                if ( clauseChecksOut == false && freeVar == 1 ) {
                    start->evaluated[freeVarIndex] = level;
                    // start->decisionLevel[freeVarIndex] = level;
                    // start->previous[freeVarIndex] = i;
                    if ( freeVarValue > 0 )
                        start->option[freeVarIndex] = true;
                    else
                        start->option[freeVarIndex] = false;
                    unitClause = true;
                    break;
                }
            }
        }
        // Check if all vars have values
        bool solution = true;
        for ( int i = 0 ; i < nrVars ; i++ ) {
            if ( start->evaluated[i] == -1 ) {
                solution = false;
                break;
            }
        }
        if ( solution == true )
            return true;
    }
    // Keep going!
    // Find free var
    for ( int i = 0 ; i < nrVars ; i++ ) {
        if ( start->evaluated[i] == -1 ) {
            freeVarIndex = i;
            break;
        }
    }

    if ( start->evaluated[freeVarIndex] == -1 ) {
        start->evaluated[freeVarIndex] = level;
        start->option[freeVarIndex] = true;
        // int lastDecision = decisionListIndex;
        // decisionList[decisionListIndex] = freeVarIndex + 1;
        // decisionLevel[decisionListIndex] = level;
        // decisionListIndex++;
        addToList(level, freeVarIndex + 1);
        // if ( level > decisionListIndex )
            decisionListIndex = level;
        // start->decisionLevel[freeVarIndex] = level;
        // if ( goingUp == 1 || goingUp == 0 ) {
        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1, nrLearnedClauses, false) )
            return true;

        // Reset modified vars by recursion
        for ( int i  = 0 ; i < nrVars ; i++ ) {
                if (start->evaluated[i] > level) {
                    start->evaluated[i] = -1;
            }
        }

        // }
        // if ( goingUp == 2 || goingUp == 0 ) {
            start->option[freeVarIndex] = false;
            // decisionListIndex--;
            // decisionList[lastDecision] = -(freeVarIndex + 1);
            // decisionListIndex++;
            removeFromList(level, freeVarIndex + 1);
            decisionListIndex = level;
            addToList(level, -(freeVarIndex + 1));
            if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1, nrLearnedClauses, 0) )
                return true;
            start->evaluated[freeVarIndex] = -1;
        // }
    }
    return false;
}

int main(int argc, char *argv[]) {
    if ( argc != 3 ) {
        printf("Da date bune de input...\n");
        return 1;
    }
    FILE *f1 = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    char *word = malloc(100);
    Clause *clauses;
    Vars *vars;
    int nrVars, nrClauses;

    while ( !feof(f1) ) {
        fscanf(f1, "%s", word);
        if ( word[0] == 'p' ) {
            fscanf(f1, "%s %d %d", word, &nrVars, &nrClauses);
            clauses = malloc(nrClauses * 3 * sizeof(Clause));
            break;
        } if ( word[0] == 'c' )
            fgets(word, 100, f1);
    }

    // Funny malloc
    for ( int i = 0 ; i < nrClauses * 2 ; i++ ) {
        clauses[i + nrClauses].vars = malloc(100 * sizeof(int));
    }

    for ( int i = 0 ; i < nrClauses ; i++ ) {
        clauses[i].vars = malloc(nrVars * sizeof(int));
        clauses[i].nrVars = 0;
        while ( true ) {
            fscanf(f1, "%d", &clauses[i].vars[clauses[i].nrVars]);
            if ( clauses[i].vars[clauses[i].nrVars] == 0 )
                break;
            clauses[i].nrVars++;
        }
        // printClause(&clauses[i]);
    }

    vars = malloc(sizeof(Vars));
    vars->vars = malloc(nrVars * sizeof(int));
    vars->option = malloc(nrVars * sizeof(bool));
    vars->evaluated = malloc(nrVars * sizeof(int));
    // vars->decisionLevel = malloc(nrVars * sizeof(int));
    // vars->previous = malloc(nrVars * sizeof(int));

    seen = malloc(nrVars * sizeof(bool));
    for ( int i = 0 ; i < nrVars ; i++ ) {
        vars->vars[i] = i + 1;
        vars->option[i] = false;
        vars->evaluated[i] = -1;
    }

    int ignoreMe = 0;

    // memset(vars->decisionLevel, -1, nrVars * sizeof(int));
    // memset(vars->evaluated, -1, nrVars * sizeof(int));
    memset(swapped, 0, 1000 * sizeof(int));
    int nrLearnedClauses = 0;

    if ( bktUnitPropagation(vars, clauses, nrClauses, nrVars, ignoreMe, &nrLearnedClauses, 0) ) {
        fprintf(out, "s SATISFIABLE\n");
        for ( int i = 0 ; i < nrVars ; i++ )
            fprintf(out, "v %d\n", vars->option[i] ? i + 1 : -(i + 1));
    } else
        fprintf(out, "s UNSATISFIABLE\n");
    printf("Going deep: %d\n", goingDEEP);
    fclose(f1);
    fclose(out);
    for ( int i = 0 ; i < nrClauses * 3 ; i++ )
        free(clauses[i].vars);
    free(clauses);
    free(vars->vars);
    free(vars->option);
    free(vars->evaluated);
    free(vars);
    free(seen);
}