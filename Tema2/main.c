#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
typedef struct Clause{
    int *vars;
    int nrVars;
    int **watched;
    struct Clause *next;
    struct Clause *prev;
    bool learnedClause;
    bool useMeOnce;
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

int trailSize = 0;
TrailElement *startT = NULL;
TrailElement *stopT = NULL;
int mallocLearn = 0;
int freeLearn = 0;

void addTrailElement(int value, int decisionLevel, bool decided, Clause *cause) {
    TrailElement *p = malloc(sizeof(TrailElement));
    p->var = abs(value);
    p->value = value > 0 ? true : false;
    p->decisionLevel = decisionLevel;
    p->decided = decided;
    p->cause = cause;
    trailSize++;
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
            if ( p->cause != NULL ) {
                free(p->cause->vars);
                free(p->cause);
            }
            free(p);
            trailSize--;
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
            if ( q->cause != NULL ) {
                free(q->cause->vars);
                free(q->cause);
            }
            p = p->prev;
            trailSize--;
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
    double *score;
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
int trailSizeCopy = 0;

TrailElement** copyV2() {
    TrailElement **p = malloc(trailSize * sizeof(TrailElement*));
    TrailElement *q = startT;
    trailSizeCopy = trailSize;
    for ( int i = 0 ; q ; i++, q = q->next ) {
        p[i] = q;
    }
    return p;
}

TrailElement* removeTrailElementV2(TrailElement **p) {
    TrailElement *q = p[trailSizeCopy - 1];
    trailSizeCopy--;
    return q;
}

int inc = 1;

Clause* genReason(int var, Clause *c, Vars *start, int nrVars) {
    Clause *p = malloc(sizeof(Clause));
    p->vars = malloc(nrVars * sizeof(int));
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

Clause *lastReason = NULL;

Clause* findReason(int index) {
    TrailElement *p = startT;
    for ( ; p ; p = p->next ) {
        if ( p->var == index ) {
            return p->cause;
        }
    }
    return NULL;
}

Clause* findFirstVarReason(Clause *c) {
    TrailElement *p = startT;
    for ( ; p ; p = p->next );
    for ( ; p ; p = p->prev ) {
        for ( int i = 0 ; i < c->nrVars ; i++ ) {
            if ( abs(p->var) == abs(c->vars[i]) ) {
                return p->cause;
            }}}return NULL;}

void analyzeConflict(Clause *c, Vars *start, int nrVars, Clause *learnedClause, int *backjump, int *ok, int maxLevel) {
    // printf("Conflict clause: ");
    // printf("\n");
    // printClause(c);
    // printf("Max level: %d\n", maxLevel);
    // TrailElement *stop = copy();
    TrailElement **startCopyTrail = copyV2();
    // printf("Stop: \n");
    // TrailElement *startt = NULL;
    // TrailElement *pp = stop;
    // for ( ; stop->prev ; stop = stop->prev );
        // printf("Var: %d, Value: %d, Decision level: %d, Decided: %d\n", stop->var, stop->value, stop->decisionLevel, stop->decided);
    // printf("Stop: \n");
    // startt = stop;
    // stop = pp;
    // copyStop = stop;
    // stop = NULL;
    // printTrail(startT);
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
    // GET ME THE GOOD FCKING REASON
    Clause *reason = findFirstVarReason(c);
    // Clause *reason = c;
    Clause reasonForAllCurrentLevel;
    reasonForAllCurrentLevel.vars = malloc(nrVars * sizeof(int));
    // reasonForAllCurrentLevel.nrVars = 0;

    // for ( int i = 0 ; i < c->nrVars ; i++ ) {
    //     if ( start->evaluated[abs(c->vars[i]) - 1] == maxFound ) {
    //         Clause *pp = findReason(abs(c->vars[i]));
    //         if ( pp == NULL ) {
    //             printf("No reason found for %d\n", c->vars[i]);
    //             continue;
    //         }
    //         for ( int j = 0 ; j < pp->nrVars ; j++ ) {
    //             bool skip = false;
    //             for ( int k = 0 ; k < reasonForAllCurrentLevel.nrVars ; k++ ) {
    //                 if ( reasonForAllCurrentLevel.vars[k] == pp->vars[j] )
    //                     skip = true;
    //             }
    //             if ( skip )
    //                 continue;
    //             reasonForAllCurrentLevel.vars[reasonForAllCurrentLevel.nrVars] = pp->vars[j];
    //             reasonForAllCurrentLevel.nrVars++;
    //         }
    //     }
    // }
    // reason = &reasonForAllCurrentLevel;
    // if (reason->nrVars == 0)
        // reason = NULL;
    // if ( reason ) {
    //     printf("Reason: \n");
    //     printClause(reason);
    // }
    // Clause *reason = c;
    // reason->nrVars = 0;
    // reason->vars = malloc(100 * sizeof(int));
    // int numConflicts = 0;
    // bool perfect = false;


        // for ( int i = 0 ; i < c->nrVars ; i++ ) {
        //     if ( start->evaluated[abs(c->vars[i]) - 1] == maxFound ) {
        //         // printf("Conflict literal: %d. Level: %d. Level implications: %d\n", c->vars[i], maxLevel, maxFound);
        //         p = abs(c->vars[i]) - 1;
        //         uip = c->vars[i];
        //     } else {
        //         // reason->vars[reason->nrVars] = c->vars[i];
        //         // reason->nrVars++;
        //         perfect = true;
        //     }
        // }

    // if ( perfect == false ) {
        // reason = NULL;
        // (void)0;
    // }
    learnedClause->watched = NULL;
    // If the last decided value provides a conflict,,
    if ( maxFound != maxLevel ) {
        *ok = 5;
        free(startCopyTrail);
        free(reasonForAllCurrentLevel.vars);
        return;
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
            // printf("Reason is null\n");
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
                        // if ( learnedClause->nrVars > 10 ) {
                        //     *ok = 5;
                        //     // while ( removeTrailElement(copyStop).var != 0 )
                        //         // freeCnt++;
                        //         free(startCopyTrail);
                        //     // printf("Free count: %d\n", freeCnt);
                        //     return;
                        // }
                    } //else
                    //    cc++;

                }
            }

            do {
                freeCnt++;
                // TrailElement t = removeTrailElement(copyStop);
                TrailElement *t = removeTrailElementV2(startCopyTrail);
                // printf("Trail size: %d\n", trailSizeCopy);
                p = abs(t->var) - 1;
                uip = t->value ? t->var : -t->var;
                reason = t->cause;
                // decLevel = t.decisionLevel;
                // printf("P is %d. cc %d.\n", p + 1, cc);
            } while ( seen[p] == false && p != -1 );
            if ( p < 0 )
                break;
            // printf("Current node: %d\n", p + 1);
            // if ( reason ) {
            //     printClause(reason);
            //     printf("Propagated reasons.. with cc : %d\n", cc);
            // }
            cc--;

    } while ( cc );
    // } while ( cc > 0 );
    // Add the uip
    if ( p != -1 ) {
        learnedClause->vars[learnedClauseIndex] = -uip;
        learnedClauseIndex++;
        learnedClause->nrVars++;
        *backjump = decLevel;
        // if ( learnedClause->nrVars > 90 ) {
        //     *ok = 5;
        //     // while ( removeTrailElement(copyStop).var != 0 )
        //         // freeCnt++;
        //     // printf("Free count: %d\n", freeCnt);
        //     free(startCopyTrail);
        //     return;
        // }
    } else {
        printf("P is -1\n");
    }

    // Empty stack
    // while ( removeTrailElement(copyStop).var != 0 )
        // freeCnt++;
    free(startCopyTrail);
    free(reasonForAllCurrentLevel.vars);
    // printf("Free count: %d\n", freeCnt);
    if ( learnedClause->nrVars == 0 ) {
        *ok = false;
        // printf("Learned clause is empty\n");
        return;
    }
    // if ( learnedClause->vars[0] == -4 )
        // printf("Found -4\n");
    // if ( learnedClause->nrVars == 0 ) {
    //     *ok = false;
    //     return;
    // }
    learnedClause->learnedClause = true;
    learnedClause->useMeOnce = false;
    // Sort the learned clause
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ ) {
        for ( int j = i + 1 ; j < learnedClause->nrVars ; j++ ) {
            if ( abs(learnedClause->vars[i]) > abs(learnedClause->vars[j]) ) {
                int aux = learnedClause->vars[i];
                learnedClause->vars[i] = learnedClause->vars[j];
                learnedClause->vars[j] = aux;
            }
        }
    }
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ ) {
        for ( int j = i + 1 ; j < learnedClause->nrVars ; j++ ) {
            if ( learnedClause->vars[i] ==  learnedClause->vars[j] ) {
                for ( int k = j ; k < learnedClause->nrVars - 1 ; k++ ) {
                    learnedClause->vars[k] = learnedClause->vars[k + 1];
                }
                learnedClause->nrVars--;
            }
        }
    }

    // Init watched
    learnedClause->watched = malloc(2 * sizeof(int*));
    if ( learnedClause->nrVars >= 2 ) {
        learnedClause->watched[0] = learnedClause->vars;
        learnedClause->watched[1] = learnedClause->vars + 1;
    } else {
        learnedClause->watched[0] = learnedClause->vars;
        learnedClause->watched[1] = NULL;
    }

    // printClause(learnedClause);
    *backjump = 0;
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ )
        if ( start->evaluated[abs(learnedClause->vars[i]) - 1] > *backjump && start->evaluated[abs(learnedClause->vars[i]) - 1] != -1 )
            *backjump = start->evaluated[abs(learnedClause->vars[i]) - 1];
    // printf("Backjump first lvl: %d", *backjump);
    int secondMax = 0;
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ )
        if ( start->evaluated[abs(learnedClause->vars[i]) - 1] > secondMax && start->evaluated[abs(learnedClause->vars[i]) - 1] != -1 && start->evaluated[abs(learnedClause->vars[i]) - 1] != *backjump )
            secondMax = start->evaluated[abs(learnedClause->vars[i]) - 1];
    // printf("Backjump second lvl: %d. Backjump diff: %d\n", secondMax, *backjump - secondMax);
    *backjump = secondMax;


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



Clause *firstProblemClause = NULL;

bool bktUnitPropagation(Vars *start, Clause *c, int nrClauses, int nrVars, int level, int *nrLearnedClauses, int goingUp) {
    int freeVarIndex = -1;
    // printTrail(startT);
    // Check learned clauses
    if ( level < 0 )
        return false;

    // for ( int i = 0 ; i < nrVars ; i++ )
        // printf("Var: %d -> %d\n", i + 1, start->evaluated[i]);


    // if ( goingUp )
        // printf("Going up, %d\n", level);
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
                for ( int j = 0 ; j < 2 ; j++ ) {
                    if ( p->watched[j] == NULL )
                        continue;

                    // if the watched literal is evaluated look for another one (and false)
                    if ( start->evaluated[abs(*p->watched[j]) - 1] != -1 && start->option[abs(*p->watched[j]) - 1] != (*p->watched[j] > 0 ? true : false) ) {
                        for ( int k = 0 ; k < p->nrVars ; k++ ) {
                            // Or find one that is evaled to true in this clause
                            if ( start->evaluated[abs(p->vars[k]) - 1] != -1 && start->option[abs(p->vars[k]) - 1] == (p->vars[k] > 0 ? true : false) ) {
                                p->watched[j] = p->vars + k;
                                clauseChecksOut = true;
                                break;
                            }
                            // Look for empty var...?
                            if ( start->evaluated[abs(p->vars[k]) - 1] == -1 && p->vars[k] != *p->watched[0] && p->vars[k] != *p->watched[1] ) {
                                p->watched[j] = p->vars + k;
                                freeVar++;
                                freeVarValue = p->vars[k];
                                freeVarIndex = abs(p->vars[k]) - 1;
                                break;
                            }
                        }
                    } else {
                        if ( start->evaluated[abs(*p->watched[j]) - 1] != -1 ) {
                            // Check if clause is already good
                            if ( *p->watched[j] > 0 && start->option[*p->watched[j] - 1] == true ) {
                                clauseChecksOut = true;
                                break;
                            }
                            if ( *p->watched[j] < 0 && start->option[-*p->watched[j] - 1] == false ) {
                                clauseChecksOut = true;
                                break;
                            }
                        } else {
                            freeVar++;
                            freeVarValue = *p->watched[j];
                            freeVarIndex = abs(*p->watched[j]) - 1;
                        }
                    }
                }
                if ( clauseChecksOut == false && freeVar == 0 ) {
                    // Conflict detected!

                        // return false;
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
                    // printf("Conflict clause: \n");
                    // printTrail(startT);
                    analyzeConflict(p, start, nrVars, learn, &backjump, &ok, level);
                    if ( startC->learnedClause && startC->useMeOnce == true ) {
                        free(startC->watched);
                        free(startC->vars);
                        Clause *q = startC;
                        startC = startC->next;
                        free(q);
                    }
                     if ( *nrLearnedClauses > 100 ) {
                        Clause *delete = firstProblemClause->prev;
                        firstProblemClause->prev = delete->prev;
                        delete->prev->next = firstProblemClause;
                        free(delete->vars);
                        free(delete);
                        free(delete->watched);
                        (*nrLearnedClauses)--;
                    }

                    // printf("Backjump: %d\n", backjump);
                    if ( ok == 5 || ok == false ) {
                        free(learn->vars);
                        if ( learn->watched )
                            free(learn->watched);
                        free(learn);
                        freeLearn++;
                        return false;
                    }

                    if ( *nrLearnedClauses < nrClauses ) {

                        goingDEEP++;

                        if ( learn->nrVars > 5 )
                            learn->useMeOnce = true;
                        (*nrLearnedClauses)++;
                        learn->learnedClause = true;
                        addClauseStart(learn);

                        // Updating score
                        for ( int i = 0 ; i < learn->nrVars ; i++ ) {
                            if ( learn->vars[i] > 0 )
                                start->score[learn->vars[i] - 1]+=inc;
                            else
                                start->score[-learn->vars[i] - 1]+=inc;
                        }
                        inc++;
                        // printClause(startC);
                        // printf("ADDED CLAuse^^^\n");
                        mallocYes = false;
                        // learn = NULL;
                        // return false;
                        backjump++;
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
                        // printClause(learn);
                        // printClause(&c[nrClauses + *nrLearnedClauses - 1]);

                        removeTrailElemets(backjump - 1);
                        // printf("After wiping the stack");
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
                    if ( hm->nrVars == 0 ) {
                        addTrailElement(freeVarValue, level, false, NULL);
                        lastReason = NULL;
                        free(hm->vars);
                        free(hm);
                    } else {
                        addTrailElement(freeVarValue, level, false, hm);
                        lastReason = hm;
                    }
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
                    // printTrail(startT);
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
    double maxScore = 0;
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

        // printf("Free var: %d. ADDED MANUALLY\n", freeVarIndex + 1);
        lastReason = NULL;
        addTrailElement(freeVarIndex + 1, level, true, NULL);


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
        // start->evaluated[freeVarIndex] = level + 1;
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
            // removeFromList(level, freeVarIndex + 1);
            removeTrailElemets(level);
            // TO DO -> Done
            // REMOVE OLD DECIDED VAR VALUE
            removeTrailElementBy(freeVarIndex + 1);
            lastReason = NULL;
            addTrailElement( start->option[freeVarIndex] ? freeVarIndex + 1 : -(freeVarIndex + 1), level, true, NULL);
            // decisionListIndex = level;
            // addToList(level, -(freeVarIndex + 1));
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

void initWatched() {
    for ( Clause *p= startC ; p ; p=p->next ) {
        p->watched = malloc(2 * sizeof(int*));
        if ( p->nrVars >= 2 ) {
            p->watched[0] = p->vars;
            p->watched[1] = p->vars + 1;
        } else {
            p->watched[0] = p->vars;
            p->watched[1] = NULL;
        }
    }

}

void freeTrail() {
    TrailElement *p = startT;
    for ( ; p ; ) {
        TrailElement *q = p;
        if ( q->cause != NULL ) {
            free(q->cause->vars);
            free(q->cause);
        }


        p = p->next;
        free(q);
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
    vars->score = malloc(nrVars * sizeof(double));
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

    // Sort the vars from clauses
    for ( Clause *p = startC ; p ; p = p->next ) {
        for ( int i = 0 ; i < p->nrVars ; i++ ) {
            for ( int j = i + 1 ; j < p->nrVars ; j++ ) {
                if ( abs(p->vars[i]) > abs(p->vars[j]) ) {
                    int aux = p->vars[i];
                    p->vars[i] = p->vars[j];
                    p->vars[j] = aux;
                }
            }
        }
    }

    int ignoreMe = 0;

    // memset(vars->decisionLevel, -1, nrVars * sizeof(int));
    // memset(vars->evaluated, -1, nrVars * sizeof(int));

    int nrLearnedClauses = 0;

    getVarsScore(vars, nrVars);
    initWatched();
    firstProblemClause = startC;
    if ( bktUnitPropagation(vars, clauses, nrClauses, nrVars, ignoreMe, &nrLearnedClauses, 0) ) {
        fprintf(out, "s SATISFIABLE\n");
        for ( int i = 0 ; i < nrVars ; i++ )
            fprintf(out, "v %d\n", vars->option[i] ? i + 1 : -(i + 1));
    } else
        fprintf(out, "s UNSATISFIABLE\n");
    // printf("Going deep: %d\n", goingDEEP);
    printf("Learnt clauses propagations: %d\n", learning);
    printf("Basic clauses propagations: %d\n", noLearn);
    // printf("Learned clauses: %d\n", nrLearnedClauses);
    fclose(f1);
    fclose(out);
    int cntLearn = 0;
    for ( Clause *p = startC ; p ; p = p->next ) {
        if ( p->learnedClause ) {
            printClause(p);
            cntLearn++;
        }
    }
    printf("Learned clauses: %d\n", cntLearn);
        // int clausesFreed = 0;
    for ( Clause *p = startC ; p ; ) {
        free(p->vars);
        free(p->watched);
        Clause *q = p;
        p = p->next;
        free(q);
        // clausesFreed++;
    }
    // printf("Clauses read: %d\n", nrClauses);
    // printf("Clauses freed: %d\n", clausesFreed);
    // printf("Malloc learn: %d\n", mallocLearn);
    // printf("Free learn: %d\n", freeLearn);
    free(clauses);
    free(vars->vars);
    free(vars->option);
    free(vars->evaluated);
    free(vars->score);
    free(vars);
    free(seen);
    free(word);
    // printTrail(startT);
    freeTrail(startT);

}