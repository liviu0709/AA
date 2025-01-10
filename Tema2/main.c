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
    bool decided;
    int decisionLevel;
} Clause;

Clause *startC = NULL;
Clause *stop = NULL;

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

void analyzeConflict(Clause *c, Vars *start, int nrVars, Clause *learnedClause, int *backjump, int *ok, int maxLevel) {
    TrailElement **startCopyTrail = copyV2();
    int maxFound = 0;
    for ( int i = 0 ; i < c->nrVars ; i++ ) {
        if ( start->evaluated[abs(c->vars[i]) - 1] != -1 ) {
            if ( start->evaluated[abs(c->vars[i]) - 1] > maxFound ) {
                maxFound = start->evaluated[abs(c->vars[i]) - 1];
            }
        }
    }
    int p = -1;
    int uip = -1;
    Clause *reason = c;
    for ( int i = 0 ; i < c->nrVars ; i++ ) {
        if ( start->evaluated[abs(c->vars[i]) - 1] == maxFound ) {

            p = abs(c->vars[i]) - 1;

        }
    }
    learnedClause->watched = NULL;
    // If the last decided value provides a conflict,,
    if ( maxFound != maxLevel ) {
        *ok = 5;
        free(startCopyTrail);
        return;
    }
    int freeCnt = 0;
    memset(seen, false, nrVars * sizeof(bool));
    int cc = 0;
    int learnedClauseIndex = 0;
    learnedClause->nrVars = 0;
    int decLevel = 0;
    do {
        if ( reason == NULL ) {
            break;
        }
        for ( int i = 0 ; i < reason->nrVars ; i++ ) {
            if ( seen[abs(reason->vars[i]) - 1] == false ) {
                seen[abs(reason->vars[i]) - 1] = true;
                if ( start->evaluated[abs(reason->vars[i]) - 1] == maxFound ) {
                    cc++;
                } else {
                    learnedClause->vars[learnedClauseIndex] = start->option[abs(reason->vars[i]) - 1] ? -abs(reason->vars[i]) : +abs(reason->vars[i]);
                    learnedClauseIndex++;
                    learnedClause->nrVars++;
                    if ( learnedClause->nrVars > 10 ) {
                        *ok = 5;
                        free(startCopyTrail);
                        return;
                    }
                }
            }
        }
        do {
            freeCnt++;
            TrailElement *t = removeTrailElementV2(startCopyTrail);
            p = abs(t->var) - 1;
            uip = t->value ? t->var : -t->var;
            reason = t->cause;
        } while ( seen[p] == false && p != -1 );
        if ( p < 0 )
            break;
        cc--;
    } while ( cc );
    // Add the uip
    if ( p != -1 ) {
        learnedClause->vars[learnedClauseIndex] = -uip;
        learnedClauseIndex++;
        learnedClause->nrVars++;
        *backjump = decLevel;
        if ( learnedClause->nrVars > 90 ) {
            *ok = 5;
            free(startCopyTrail);
            return;
        }
    } else {
        printf("P is -1\n");
    }
    free(startCopyTrail);
    if ( learnedClause->nrVars == 0 ) {
        *ok = false;
        return;
    }
    learnedClause->learnedClause = true;
    // Init watched
    learnedClause->watched = malloc(2 * sizeof(int*));
    if ( learnedClause->nrVars >= 2 ) {
        learnedClause->watched[0] = learnedClause->vars;
        learnedClause->watched[1] = learnedClause->vars + 1;
    } else {
        learnedClause->watched[0] = learnedClause->vars;
        learnedClause->watched[1] = NULL;
    }
    *backjump = 0;
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ )
        if ( start->evaluated[abs(learnedClause->vars[i]) - 1] > *backjump && start->evaluated[abs(learnedClause->vars[i]) - 1] != -1 )
            *backjump = start->evaluated[abs(learnedClause->vars[i]) - 1];
    int secondMax = 0;
    for ( int i = 0 ; i < learnedClause->nrVars ; i++ )
        if ( start->evaluated[abs(learnedClause->vars[i]) - 1] > secondMax && start->evaluated[abs(learnedClause->vars[i]) - 1] != -1 && start->evaluated[abs(learnedClause->vars[i]) - 1] != *backjump )
            secondMax = start->evaluated[abs(learnedClause->vars[i]) - 1];
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

Clause* genReason(int var, Clause *c, Vars *start, int nrVars) {
    Clause *p = malloc(sizeof(Clause));
    p->vars = malloc(100 * sizeof(int));
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

Clause *firstProblemClause = NULL;

bool bktUnitPropagation(Vars *start, Clause *c, int nrClauses, int nrVars, int level, int *nrLearnedClauses, int goingUp) {
    int freeVarIndex = -1;
    // Check learned clauses
    if ( level < 0 )
        return false;
    // Unit clause propagation
    for ( bool unitClause = true ; unitClause ; ) {
        unitClause = false;
        Clause *p = startC;
        for ( int i = 0 ; p ; i++, p=p->next ) {
            if ( p->decided == true )
                continue;
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
                            p->decided = true;
                            p->decisionLevel = level;
                            break;
                        }
                        if ( *p->watched[j] < 0 && start->option[-*p->watched[j] - 1] == false ) {
                            clauseChecksOut = true;
                            p->decided = true;
                            p->decisionLevel = level;
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
                int ok = true;
                int backjump = 0;
                Clause *learn = NULL;
                learn = malloc(sizeof(Clause));
                learn->vars = malloc(100 * sizeof(int));
                analyzeConflict(p, start, nrVars, learn, &backjump, &ok, level);
                    if ( *nrLearnedClauses > 200 ) {
                    // Forget half of the learned clauses
                    Clause *p = startC;
                    int cnt = 0;
                    for ( ; p ; p = p->next ) {
                        if ( p->learnedClause == true ) {
                            cnt++;
                            if ( cnt > 100 ) {
                                break;
                            }
                        }
                    }
                    if ( p ) {
                        Clause *q = p->next;
                        for ( ; q && q->learnedClause; ) {
                            Clause *r = q;
                            q = q->next;
                            if ( r->watched )
                                free(r->watched);
                            free(r->vars);
                            free(r);
                        }
                        p->next = firstProblemClause;
                        firstProblemClause->prev = p;
                        *nrLearnedClauses = 100;
                    }
                }
                if ( ok == 5 || ok == false ) {
                    free(learn->vars);
                    if ( learn->watched )
                        free(learn->watched);
                    free(learn);
                    return false;
                }
                    (*nrLearnedClauses)++;
                    learn->learnedClause = true;
                    learn->decided = false;
                    addClauseStart(learn);
                    // Updating score
                    for ( int i = 0 ; i < learn->nrVars ; i++ ) {
                        if ( learn->vars[i] > 0 )
                            start->score[learn->vars[i] - 1]++;
                        else
                            start->score[-learn->vars[i] - 1]++;
                    }
                    return false;
                return false;
            }
            // We can do propagation :)
            if ( clauseChecksOut == false && freeVar == 1 ) {
                start->evaluated[freeVarIndex] = level;
                Clause *hm = genReason(freeVarValue, p, start, nrVars);
                if ( hm->nrVars == 0 ) {
                    addTrailElement(freeVarValue, level, false, NULL);
                    free(hm->vars);
                    free(hm);
                } else
                    addTrailElement(freeVarValue, level, false, hm);
                if ( freeVarValue > 0 )
                    start->option[freeVarIndex] = true;
                else
                    start->option[freeVarIndex] = false;
                unitClause = true;
                p->decided = true;
                p->decisionLevel = level;
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
    // Keep going!
    // Find free var
    double maxScore = 0;
    for ( int i = 0 ; i < nrVars ; i++ ) {
        if ( start->evaluated[i] == -1 && start->score[i] > maxScore ) {
            maxScore = start->score[i];
        }
    }
    for ( int i = 0 ; i < nrVars ; i++ ) {
        if ( start->evaluated[i] == -1 && start->score[i] == maxScore ) {
            freeVarIndex = i;
            break;
        }
    }
    if ( start->evaluated[freeVarIndex] == -1 ) {
        start->evaluated[freeVarIndex] = level;
        start->option[freeVarIndex] = true;
        addTrailElement(freeVarIndex + 1, level + 1, true, NULL);
        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1, nrLearnedClauses, false) )
            return true;
        // Reset modified vars by recursion
        for ( int i  = 0 ; i < nrVars ; i++ ) {
                if (start->evaluated[i] > level) {
                    start->evaluated[i] = -1;
            }
        }
        start->option[freeVarIndex] = false;
        // Uncheck the clauses decided on higher level
        for ( Clause *p = startC ; p ; p = p->next ) {
            if ( p->decisionLevel > level ) {
                p->decided = false;
                p->decisionLevel = 0;
            }
        }
        removeTrailElemets(level);
        removeTrailElementBy(freeVarIndex + 1);
        addTrailElement( start->option[freeVarIndex] ? freeVarIndex + 1 : -(freeVarIndex + 1), level, true, NULL);
        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1, nrLearnedClauses, 0) )
            return true;
        start->evaluated[freeVarIndex] = -1;
    }
    return false;
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
            break;
        } if ( word[0] == 'c' )
            fgets(word, 100, f1);
    }
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
        p->decided = false;
        addClauseEnd(p);
    }
    vars = malloc(sizeof(Vars));
    vars->vars = malloc(nrVars * sizeof(int));
    vars->score = malloc(nrVars * sizeof(double));
    vars->option = malloc(nrVars * sizeof(bool));
    vars->evaluated = malloc(nrVars * sizeof(int));
    seen = malloc(nrVars * sizeof(bool));
    for ( int i = 0 ; i < nrVars ; i++ ) {
        vars->vars[i] = i + 1;
        vars->option[i] = false;
        vars->evaluated[i] = -1;
    }
    int ignoreMe = 0;
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
    fclose(f1);
    fclose(out);
    for ( Clause *p = startC ; p ; ) {
        free(p->vars);
        free(p->watched);
        Clause *q = p;
        p = p->next;
        free(q);
    }
    free(clauses);
    free(vars->vars);
    free(vars->option);
    free(vars->evaluated);
    free(vars->score);
    free(vars);
    free(seen);
    free(word);
    freeTrail(startT);

}