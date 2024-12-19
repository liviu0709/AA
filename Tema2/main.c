#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
typedef struct Clause{
    int *vars;
    int nrVars;
    struct Clause *next;
    struct Clause *prev;
    bool learnedClause;
} Clause;

Clause *startC = NULL;
Clause *stop = NULL;

static int noLearn, learning;

typedef struct TrailElement {
    int var;
    bool value;
    int decisionLevel;
    bool decided;
    Clause *cause;
    struct TrailElement *next;
    struct TrailElement *prev;
} TrailElement;

TrailElement *startT = NULL;
TrailElement *stopT = NULL;

void addTrailElement(int value, int decisionLevel, bool decided, Clause *cause) {
    TrailElement *p = malloc(sizeof(TrailElement));
    p->var = abs(value);
    p->value = value > 0 ? true : false;
    p->decisionLevel = decisionLevel;
    p->decided = decided;
    p->cause = cause;
    if ( startT == NULL ) {
        startT = p;
        stopT = p;
        p->prev = NULL;
        p->next = NULL;
        return;
    }
    stopT->next = p;
    p->prev = stopT;
    p->next = NULL;


    stopT = p;
}

bool decidedTrailElement(int var) {
    TrailElement *p = startT;
    for ( ; p ; p = p->next ) {
        if ( p->var == var )
            return p->decided;
    }
    return false;
}



TrailElement *copyStop;

TrailElement removeTrailElement() {
    TrailElement *p = copyStop;
    if ( p == NULL )
        return (TrailElement) {0, 0, 0, 0, NULL, NULL, NULL};
    TrailElement solution;
    solution.cause = p->cause;
    solution.decided = p->decided;
    solution.decisionLevel = p->decisionLevel;
    solution.value = p->value;
    solution.var = p->var;
    if ( p->prev )
        p->prev->next = NULL;
    // else
        // startT = NULL;
    copyStop = p->prev;
    // WHY SEG FAULT ?!
    free(p);

    return solution;
}

void removeTrailElementBy(int var) {
    TrailElement *p = startT;
    for ( ; p ; p = p->next ) {
        if ( p->var == var ) {
            if ( p->prev )
                p->prev->next = p->next;
            else
                startT = p->next;
            if ( p->next )
                p->next->prev = p->prev;
            else
                stopT = p->prev;
            free(p);
            return;
        }
    }
}

void removeTrailElemets(int decisionLevel) {
    TrailElement *p = stopT;
    for ( ; p ; ) {
        if ( p->decisionLevel > decisionLevel ) {
            if ( p->prev )
                p->prev->next = NULL;
            else
                startT = NULL;
            stopT = p->prev;
            TrailElement *q = p;
            p = p->prev;
            free(q);

        } else {
            p = p->prev;
        }
    }

}

void addClauseEnd(Clause* c) {
    if ( startC == NULL ) {
        startC = c;
        stop = c;
        c->next = NULL;
        c->prev = NULL;
        return;
    }
    stop->next = c;
    c->prev = stop;
    c->next = NULL;
    stop = c;
}

void addClauseStart(Clause* c) {
    if ( startC == NULL ) {
        startC = c;
        c->next = NULL;
        c->prev = NULL;
        stop = c;
        return;
    }
    startC->prev = c;
    c->next = startC;
    c->prev = NULL;
    startC = c;
}

typedef struct {
    int *vars;
    bool *option;
    int *score;
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

void printTrail(TrailElement *start) {
    TrailElement *p = start;
    printf("Trail: \n");
    for ( ; p ; p = p->next ) {
        printf("Var: %d, Value: %d, Decision level: %d, Decided: %d\n", p->var, p->value, p->decisionLevel, p->decided);
        if ( p->cause )
            printClause(p->cause);
    }
    printf("End of trail\n");
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

TrailElement* copy() {
    TrailElement *p = startT;
    if ( p == NULL )
        return NULL;
    TrailElement *q = NULL;
    TrailElement *start = NULL;
    int mallocCnt = 0;
    for ( ; p ; p = p->next ) {
        TrailElement *r = malloc(sizeof(TrailElement));
        mallocCnt++;
        r->var = p->var;
        r->value = p->value;
        r->decisionLevel = p->decisionLevel;
        r->decided = p->decided;
        r->cause = p->cause;
        if ( start == NULL ) {
            start = r;
            start->prev = NULL;
            start->next = NULL;
            q = r;
        } else {
            q->next = r;
            r->prev = q;
            r->next = NULL;
            q = r;
        }
    }
    printf("Malloc cnt: %d\n", mallocCnt);
    for( q = start ; q->next ; q = q->next );
    return q;
}

void analyzeConflict(Clause *c, Vars *start, int nrVars, Clause *learnedClause, int *backjump, int *ok, int maxLevel) {
    // int minLevel = __INT32_MAX__;
    printf("Conflict clause: ");
    // for ( int i = 0 ; i < nrVars ; i++) {
        // printf(" |||%d evaluated to %d |||", i + 1, start->evaluated[i]);
    // }
    // printf("\n");
    printClause(c);
    TrailElement *stop = copy();
    // printf("Stop: \n");
    // TrailElement *startt = NULL;
    // TrailElement *pp = stop;
    // for ( ; stop->prev ; stop = stop->prev );
        // printf("Var: %d, Value: %d, Decision level: %d, Decided: %d\n", stop->var, stop->value, stop->decisionLevel, stop->decided);
    // printf("Stop: \n");
    // startt = stop;
    // stop = pp;
    copyStop = stop;
    stop = NULL;
    printTrail(startT);
    int maxFound = 0;
    for ( int i = 0 ; i < c->nrVars ; i++ ) {
        if ( start->evaluated[abs(c->vars[i]) - 1] != -1 ) {
            if ( start->evaluated[abs(c->vars[i]) - 1] > maxFound ) {
                maxFound = start->evaluated[abs(c->vars[i]) - 1];
                // printf("Conflict literal: %d. Level: %d. Level implications: %d(FAKE)\n", c->vars[i], maxLevel, maxFound);
            }
        }
    }
    // printf("Max found: %d\n", maxFound);
    int p = -1;
    int uip = -1;
    Clause *reason = c;
    // int numConflicts = 0;
    for ( int i = 0 ; i < c->nrVars ; i++ ) {
        if ( start->evaluated[abs(c->vars[i]) - 1] == maxFound ) {
            printf("Conflict literal: %d. Level: %d. Level implications: %d\n", c->vars[i], maxLevel, maxFound);
            p = abs(c->vars[i]) - 1;
            // numConflicts++;
        }
    }
    // Clause *reasonNop = malloc(sizeof(Clause));
    // reasonNop->vars = malloc(100 * sizeof(int));
    // reasonNop->nrVars = 0;
    // for ( int i = 0 ; i < c->nrVars ; i++ ) {
    //     if ( abs(c->vars[i]) - 1 != p ) {
    //         reasonNop->vars[reasonNop->nrVars] = c->vars[i];
    //         reasonNop->nrVars++;
    //     }
    // }
    // reason = reasonNop;
    int freeCnt = 0;

    // if ( maxFound == 0 ) {
    //     *ok = false;
    //     while( removeTrailElement(copyStop).var != 0 )
    //         freeCnt++;
    //     printf("Free count: %d\n", freeCnt);
    //     return;
    // }
    memset(seen, false, nrVars * sizeof(bool));
    int cc = 0;
    // int cc = 0;
    int learnedClauseIndex = 0;
    learnedClause->nrVars = 0;
    int decLevel = 0;
    // printf("Starting farming learnt clauses with: \n");
    // printClause(reason);
    do {
        // cc++;
        if ( reason == NULL ) {
            printf("Reason is null\n");
            break;
        }

            for ( int i = 0 ; i < reason->nrVars ; i++ ) {
                if ( seen[abs(reason->vars[i]) - 1] == false ) {
                    seen[abs(reason->vars[i]) - 1] = true;
                    if ( start->evaluated[abs(reason->vars[i]) - 1] == maxFound ) {
                    // if (decidedTrailElement(abs(reason->vars[i]))) {
                        cc++;
                    } else {
                        // learnedClause->vars[learnedClauseIndex] = -reason->vars[i];
                        learnedClause->vars[learnedClauseIndex] = start->option[abs(reason->vars[i]) - 1] ? -abs(reason->vars[i]) : +abs(reason->vars[i]);
                        learnedClauseIndex++;
                        learnedClause->nrVars++;
                        if ( learnedClause->nrVars > 5 ) {
                            *ok = 5;
                            while ( removeTrailElement(copyStop).var != 0 )
                                freeCnt++;
                            // printf("Free count: %d\n", freeCnt);
                            return;
                        }
                    } //else
                    //    cc++;

                }
            }

            do {
                freeCnt++;
                TrailElement t = removeTrailElement(copyStop);
                p = abs(t.var) - 1;
                uip = t.value ? t.var : -t.var;
                reason = t.cause;
                // decLevel = t.decisionLevel;
                // printf("P is %d\n", p);
            } while ( seen[p] == false && p != -1 );
            if ( p < 0 )
                break;
            printf("Current node: %d\n", p + 1);
            if ( reason ) {
                printClause(reason);
                printf("Propagated reasons.. with cc : %d\n", cc);
            }
            cc--;

    } while ( cc );
    // } while ( cc > 0 );
    // Add the uip
    if ( p != -1 ) {
        learnedClause->vars[learnedClauseIndex] = -uip;
        learnedClauseIndex++;
        learnedClause->nrVars++;
        *backjump = decLevel;
        if ( learnedClause->nrVars > 9 ) {
            *ok = 5;
            while ( removeTrailElement(copyStop).var != 0 )
                freeCnt++;
            // printf("Free count: %d\n", freeCnt);
            return;
        }
    } else {
        printf("P is -1\n");
    }

    // Empty stack
    while ( removeTrailElement(copyStop).var != 0 )
        freeCnt++;
    // printf("Free count: %d\n", freeCnt);
    if ( learnedClause->nrVars == 0 ) {
        *ok = false;
        printf("Learned clause is empty\n");
        return;
    }
    if ( learnedClause->vars[0] == -4 )
        printf("Found -4\n");
    // if ( learnedClause->nrVars == 0 ) {
    //     *ok = false;
    //     return;
    // }
    learnedClause->learnedClause = true;
    // printClause(learnedClause);
    *backjump = maxFound;
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ )
        if ( start->evaluated[abs(learnedClause->vars[i]) - 1] < *backjump && start->evaluated[abs(learnedClause->vars[i]) - 1] != -1 )
            *backjump = start->evaluated[abs(learnedClause->vars[i]) - 1];
    int secondMin = *backjump;
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ )
        if ( start->evaluated[abs(learnedClause->vars[i]) - 1] < secondMin && start->evaluated[abs(learnedClause->vars[i]) - 1] != -1 && start->evaluated[abs(learnedClause->vars[i]) - 1] != *backjump )
            secondMin = start->evaluated[abs(learnedClause->vars[i]) - 1];
    *backjump = secondMin;
    printf("Backjump diff: %d\n", *backjump - maxLevel);



}

bool checkDuplicate(Clause *p) {
    Clause *q = startC;
    for ( ; q ; q = q->next ) {
        if ( p->nrVars != q->nrVars )
            continue;
        bool found = true;
        for ( int i = 0 ; i < p->nrVars ; i++ ) {
            if ( p->vars[i] != q->vars[i] ) {
                found = false;
                break;
            }
        }
        if ( found == true )
            return true;
    }
    return false;
}

bool mallocYes = false;

int mallocLearn = 0;
int freeLearn = 0;

Clause* genReason(int var, Clause *c, Vars *start, int nrVars) {
    Clause *p = malloc(sizeof(Clause));
    p->vars = malloc(100 * sizeof(int));
    mallocLearn++;
    p->nrVars = 0;
    p->learnedClause = true;
    for ( int i = 0 ; i < c->nrVars ; i++ ) {
        if ( abs(c->vars[i]) == abs(var) ) {
            continue;
        }
        p->vars[p->nrVars] = c->vars[i];
        p->nrVars++;
    }
    return p;
}

bool bktUnitPropagation(Vars *start, Clause *c, int nrClauses, int nrVars, int level, int *nrLearnedClauses, int goingUp) {
    int freeVarIndex = -1;
    // printList();
    printTrail(startT);

    // Check learned clauses
    if ( level < 0 )
        return false;

    // if ( goingUp )
        printf("Going up, %d\n", level);
    // if ( startC->learnedClause ) {
    //     printClause(startC);
    //     printf("^^^^ LEARNED!\n");
    // }
    bool efficientVarSearch = false;
    // int varIndex = 0;
    goingUp = 0;
    if ( !goingUp ) {
        // Unit clause propagation
        for ( bool unitClause = true ; unitClause ; ) {
            efficientVarSearch = false;
            unitClause = false;
            Clause *p = startC;

            for ( int i = 0 ; p ; i++, p=p->next ) {
                // printClause(p);

                // if ( i > 90 )
                    // printf("Too many clauses\n");

                // if ( p->nrVars == 1 && p->vars[0] == 17 )
                    // printf("Found 17\n");

                // printf("Clause: %d\n", i);
                bool clauseChecksOut = false;
                int freeVar = 0;
                int freeVarValue = 0;
                for ( int j = 0 ; j < p->nrVars ; j++ ) {
                    // Count free vars

                    if ( start->evaluated[abs(p->vars[j]) - 1] == -1 ) {
                        freeVar++;
                        freeVarIndex = abs(p->vars[j]) - 1;
                        freeVarValue = p->vars[j];
                    } else {
                        // Check if clause is already good
                        if ( p->vars[j] > 0 && start->option[p->vars[j] - 1] == true ) {
                            clauseChecksOut = true;
                            break;
                        }
                        if ( p->vars[j] < 0 && start->option[-p->vars[j] - 1] == false ) {
                            clauseChecksOut = true;
                            break;
                        }
                    }
                }
                if ( clauseChecksOut == false && freeVar == 0 ) {
                    // Conflict detected!
                    printf("Conflict detected\n");
                    if ( *nrLearnedClauses > nrClauses )
                    // if ( *nrLearnedClauses > 5 )
                        return false;
                    // return false;
                    // if ( goingUp == 2 )
                        // return false;
                    int ok = true;
                    int backjump = 0;
                    Clause *learn = NULL;

                    // if ( learn == NULL ) {
                        learn = malloc(sizeof(Clause));
                        learn->vars = malloc(100 * sizeof(int));
                        mallocLearn++;

                    // }
                    printf("Conflict clause: \n");
                    analyzeConflict(p, start, nrVars, learn, &backjump, &ok, level);
                    printf("Backjump: %d\n", backjump);
                    if ( ok == 5 || ok == false || *nrLearnedClauses >= nrClauses) {
                        free(learn->vars);
                        free(learn);
                        freeLearn++;
                        return false;
                    }
                    // return false;
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
                    // if ( *nrLearnedClauses < 4 ) {
                    // if ( *nrLearnedClauses < 20 ) {
                        // if ( swapped[abs(p->vars[0]) - 1] == 1 )
                            // return false;
                        // if( backjump == 0 )
                            // return false;
                        printf("Backjump: %d. Current level: %d\n", backjump, level);
                        goingDEEP++;
                        if ( checkDuplicate(learn) == true ) {
                            printClause(learn);
                            printf("^^^Duplicate clause\n");
                            free(learn->vars);
                            free(learn);
                            return false;
                        }

                        // return false;

                        (*nrLearnedClauses)++;
                        learn->learnedClause = true;
                        addClauseStart(learn);
                        printClause(startC);
                        printf("ADDED CLAuse^^^\n");
                        mallocYes = false;
                        // learn = NULL;
                        return false;
                        // backjump++;
                        // if ( backjump == 0 ) {
                            // return false;
                        // }

                        // int oldLevelDec = start->evaluated[abs(p->vars[0]) - 1];
                        // printf("Evaluated level: %d\n", start->evaluated[abs(p->vars[0]) - 1]);
                        for ( int i  = 0 ; i < nrVars ; i++ ) {
                                if (start->evaluated[i] >= backjump) {
                                    start->evaluated[i] = -1;
                            }
                        }
                        // printf("Learned clause: \n");
                        // printClause(&c[i]);
                        // printClause(&c[nrClauses + *nrLearnedClauses - 1]);
                        // printList();
                        // start->evaluated[abs(p->vars[0]) - 1] = oldLevelDec;
                        // printf("Variable initial value: %d.", start->option[abs(p->vars[0]) - 1]);
                        // start->option[abs(p->vars[0]) - 1] = start->option[abs(p->vars[0]) - 1] ? false : true;
                        // swapped[abs(p->vars[0]) - 1] = 1;
                        // printf("Variable to modify: %d. Value: %d\n", p->vars[0], start->option[abs(p->vars[0]) - 1]);
                        removeTrailElemets(backjump - 1);
                        // printTrail(startT);
                        return bktUnitPropagation(start, c, nrClauses, nrVars, backjump, nrLearnedClauses, true);
                    }
                    return false;
                }
                // We can do propagation :)
                // printf("Clause checks out\n");
                if ( clauseChecksOut == false && freeVar == 1 ) {
                    start->evaluated[freeVarIndex] = level;
                    // not the clause is da reason|||||||||||||||||||||
                    Clause *hm = genReason(freeVarValue, p, start, nrVars);
                    if ( hm->nrVars == 0 )
                        addTrailElement(freeVarValue, level, false, NULL);
                    else
                        addTrailElement(freeVarValue, level, false, hm);
                    // start->decisionLevel[freeVarIndex] = level;
                    // start->previous[freeVarIndex] = i;
                    if (p->learnedClause )
                        learning++;
                    else
                        noLearn++;
                    if ( freeVarValue > 0 )
                        start->option[freeVarIndex] = true;
                    else
                        start->option[freeVarIndex] = false;
                    unitClause = true;
                    break;
                }
                if ( freeVar >= 2 && efficientVarSearch == false && p->learnedClause == true ) {
                    efficientVarSearch = true;
                    // varIndex = freeVarIndex;
                }
                // printf("Clause checks out\n");
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
    // if ( efficientVarSearch == true && start->evaluated[varIndex] == -1 ) {
        // freeVarIndex = varIndex;
    // } else {
    int maxScore = 0;
        for ( int i = 0 ; i < nrVars ; i++ ) {
            if ( start->evaluated[i] == -1 && start->score[i] > maxScore ) {
                // freeVarIndex = i;
                // break;
                maxScore = start->score[i];
            }
        }
        for ( int i = 0 ; i < nrVars ; i++ ) {
            if ( start->evaluated[i] == -1 && start->score[i] == maxScore ) {
                freeVarIndex = i;
                break;
            }
        }
    // }

    if ( start->evaluated[freeVarIndex] == -1 ) {
        start->evaluated[freeVarIndex] = level;
        start->option[freeVarIndex] = true;
        printf("Free var: %d. ADDED MANUALLY\n", freeVarIndex + 1);
        addTrailElement(freeVarIndex + 1, level, true, NULL);
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
            // Before changing the decided var the value.. check the learned clauses so no conflict is generated
            for ( Clause *generated = startC ; generated ; generated = generated->next) {
                if ( generated->learnedClause == false )
                    break;
                if ( generated->nrVars == 1 && abs(generated->vars[0]) == freeVarIndex + 1 ) {
                    if ( generated->vars[0] > 0 )
                        start->option[freeVarIndex] = true;
                    else
                        start->option[freeVarIndex] = false;
                }
            }
            // decisionListIndex--;
            // decisionList[lastDecision] = -(freeVarIndex + 1);
            // decisionListIndex++;
            removeFromList(level, freeVarIndex + 1);
            removeTrailElemets(level);
            // TO DO -> Done
            // REMOVE OLD DECIDED VAR VALUE
            removeTrailElementBy(freeVarIndex + 1);
            addTrailElement( start->option[freeVarIndex] ? freeVarIndex + 1 : -(freeVarIndex + 1), level, true, NULL);
            decisionListIndex = level;
            addToList(level, -(freeVarIndex + 1));
            if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1, nrLearnedClauses, 0) )
                return true;
            start->evaluated[freeVarIndex] = -1;
        // }
    }
    return false;
}

void pureLiteralElimination(Vars *start, Clause *c, int nrClauses, int nrVars) {
    for ( int i = 0 ; i < nrVars ; i++ ) {
        bool found = false;
        bool positive = true;
        bool elimination = true;
        for ( Clause *p = startC ; p ; p = p->next ) {
            for ( int j = 0 ; j < p->nrVars ; j++ ) {
                if ( p->vars[j] == i + 1 ) {
                    if ( found == true && positive == false ) {
                        elimination = false;
                        break;
                    }
                    found = true;
                    positive = true;
                }
                if ( p->vars[j] == -(i + 1) ) {
                    if ( found == true && positive == true ) {
                        elimination = false;
                        break;
                    }
                    found = true;
                    positive = false;
                }
            }
            if ( elimination ) {
                printf("Pure literal elimination: %d\n", i + 1);
            }
        }


    }
}

void getVarsScore(Vars *start, int nrVars) {
    memset(start->score, 0, nrVars * sizeof(int));
    for ( Clause *p = startC ; p ; p = p->next ) {
        for ( int i = 0 ; i < p->nrVars ; i++ ) {
            if ( p->vars[i] > 0 )
                start->score[p->vars[i] - 1]++;
            else
                start->score[-p->vars[i] - 1]++;
        }
    }
}

int main(int argc, char *argv[]) {
    if ( argc != 3 ) {
        printf("Da date bune de input...\n");
        return 1;
    }
    FILE *f1 = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    char *word = malloc(100);
    Clause *clauses = NULL;
    Vars *vars;
    int nrVars, nrClauses;

    while ( !feof(f1) ) {
        fscanf(f1, "%s", word);
        if ( word[0] == 'p' ) {
            fscanf(f1, "%s %d %d", word, &nrVars, &nrClauses);
            // clauses = malloc(nrClauses * 3 * sizeof(Clause));
            break;
        } if ( word[0] == 'c' )
            fgets(word, 100, f1);
    }

    // // Funny malloc
    // for ( int i = 0 ; i < nrClauses * 2 ; i++ ) {
    //     clauses[i + nrClauses].vars = malloc(100 * sizeof(int));
    // }

    for ( int i = 0 ; i < nrClauses ; i++ ) {
        Clause *p = malloc(sizeof(Clause));
        p->vars = malloc((nrVars + 1) * sizeof(int));
        p->nrVars = 0;
        while ( true ) {
            fscanf(f1, "%d", &p->vars[p->nrVars]);
            if ( p->vars[p->nrVars] == 0 )
                break;
            p->nrVars++;
        }
        p->learnedClause = false;
        addClauseEnd(p);
        // printClause(&clauses[i]);
    }

    vars = malloc(sizeof(Vars));
    vars->vars = malloc(nrVars * sizeof(int));
    vars->score = malloc(nrVars * sizeof(int));
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

    getVarsScore(vars, nrVars);


    if ( bktUnitPropagation(vars, clauses, nrClauses, nrVars, ignoreMe, &nrLearnedClauses, 0) ) {
        fprintf(out, "s SATISFIABLE\n");
        for ( int i = 0 ; i < nrVars ; i++ )
            fprintf(out, "v %d\n", vars->option[i] ? i + 1 : -(i + 1));
    } else
        fprintf(out, "s UNSATISFIABLE\n");
    printf("Going deep: %d\n", goingDEEP);
    printf("Learnt clauses propagations: %d\n", learning);
    printf("Basic clauses propagations: %d\n", noLearn);
    printf("Learned clauses: %d\n", nrLearnedClauses);
    fclose(f1);
    fclose(out);
    for ( Clause *p = startC ; p ; p = p->next ) {
        if ( p->learnedClause )
            printClause(p);
    }
        int clausesFreed = 0;
    for ( Clause *p = startC ; p ; ) {
        free(p->vars);
        Clause *q = p;
        p = p->next;
        free(q);
        clausesFreed++;
    }
    printf("Clauses read: %d\n", nrClauses);
    printf("Clauses freed: %d\n", clausesFreed);
    printf("Malloc learn: %d\n", mallocLearn);
    printf("Free learn: %d\n", freeLearn);
    free(clauses);
    free(vars->vars);
    free(vars->option);
    free(vars->evaluated);
    free(vars);
    free(seen);
    free(word);
}