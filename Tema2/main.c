#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int *vars;
    int nrVars;
} Clause;

typedef struct {
    int *vars;
    int *nrOptions;
    bool *option;
    int *evaluated;
} Vars;

FILE *debug;

int ModifyOrder[100];

void printClause(Clause c) {
    for ( int i = 0 ; i < c.nrVars ; i++ )
        fprintf(debug, "%d ", c.vars[i]);
    fprintf(debug, "| nrvar: %d \n", c.nrVars);
}

void printVars(Vars v, int nrVars) {
    for ( int i = 0 ; i < nrVars ; i++ )
        fprintf(debug, "Var: %d, Opt: %d, Eval: %d |||", i + 1, v.option[i], v.evaluated[i]);
    fprintf(debug, "\n");
}

bool checkSat(Clause *c, int nrClauses, Vars *v, int nrVars) {
    bool sat = true;
    for ( int i = 0 ; i < nrClauses ; i++ ) {
        bool satClause = false;
        for ( int j = 0 ; j < c[i].nrVars ; j++ ) {
            if ( c[i].vars[j] > 0 && v->option[c[i].vars[j] - 1] == true ) {
                satClause = true;
                break;
            }
            if ( c[i].vars[j] < 0 && v->option[-c[i].vars[j] - 1] == false ) {
                satClause = true;
                break;
            }
        }
        if ( satClause == false ) {
            sat = false;
            break;
        }
    }
    return sat;
}

// 1 -> ok, 0 -> not ok, 2 -> impossible
int checkSatDetect(Clause *c, int nrClauses, Vars *v, int nrVars, int level) {
    bool sat = true;
    for ( int i = 0 ; i < nrClauses ; i++ ) {
        bool satClause = false;
        bool conflict = true;
        for ( int j = 0 ; j < c[i].nrVars ; j++ ) {
            if (v->evaluated[abs(c[i].vars[j]) - 1] >= level || v->evaluated[abs(c[i].vars[j]) - 1] == -1 ) {
                 conflict = false;
            }
            if ( c[i].vars[j] > 0 && v->option[c[i].vars[j] - 1] == true ) {
                satClause = true;
                break;
            }
            if ( c[i].vars[j] < 0 && v->option[-c[i].vars[j] - 1] == false ) {
                satClause = true;
                break;
            }
        }
        if ( conflict == true && satClause == false )
            return 2;
        if ( satClause == false ) {
            sat = false;
            break;
        }
    }
    return sat;
}

// Classic backtracking
bool bktBasic(Vars *start, Clause *c, int nrClauses, int nrVars, int level) {
    if ( checkSat(c, nrClauses, start, nrVars) == true )
        return true;
    if ( level == nrVars )
        return false;
    if ( bktBasic(start, c, nrClauses, nrVars, level + 1) )
        return true;
    if ( start->option[level] == false ) {
        start->option[level] = true;
    } else {
        start->option[level] = false;
    }
    if ( bktBasic(start, c, nrClauses, nrVars, level + 1) )
            return true;
    return false;
}

bool bktUnitPropagation(Vars *start, Clause *c, int nrClauses, int nrVars, int level) {

    // printVars(*start, nrVars);
    if ( level > nrVars )
        return false;
    int ret = checkSatDetect(c, nrClauses, start, nrVars, level);

    if ( ret == 1 )
        return true;

    if ( ret == 2 )
        return false;

    int toModify = -1;
    // bool found = false;
    // // fprintf(debug, "level %d\n", level);
    // for ( int i = 0 ; i < nrClauses && !found ; i++ ) {
    //     for ( int j = 0 ; j < c[i].nrVars && !found ; j++ ) {
    //         if ( start->evaluated[abs(c[i].vars[j]) - 1] > level || start->evaluated[abs(c[i].vars[j]) - 1] == -1 ) {
    //             toModify = abs(c[i].vars[j]) - 1;
    //             fprintf(debug, "toModify %d, level %d\n", toModify + 1, level);
    //             found = true;
    //         }
    //     }
    // }
    toModify = ModifyOrder[level];
    fprintf(debug, "toModify %d, level %d\n", toModify + 1, level);
    if ( toModify != - 1 ) {
        start->evaluated[toModify] = level + 1;

        start->option[toModify] = true;
        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1) )
            return true;

        start->option[toModify] = false;

        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1) )
            return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    debug = fopen("debug", "w");
    // debug = stdout;
    // printf("ayo\n");
    if ( argc != 3 ) {
        printf("Da date bune de input...\n");
        return 1;
    }
    FILE *f1 = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    // printf("Opened file\n");

    // fscanf(f1, "%d", &n);
    // fprintf(f2, "%d\n", n);
    // Read data from file till eof
    char *word = malloc(100);
    Clause *clauses;
    Vars *vars;
    int nrVars, nrClauses;

    while ( !feof(f1) ) {
        fscanf(f1, "%s", word);
        if ( word[0] == 'p' ) {
            fscanf(f1, "%s %d %d", word, &nrVars, &nrClauses);
            clauses = malloc(nrClauses * sizeof(Clause));
            break;
        } if ( word[0] == 'c' )
            fgets(word, 100, f1);
    }
    // printf("nrVars %d nrClauses %d\n", nrVars, nrClauses);
    for ( int i = 0 ; i < nrClauses ; i++ ) {
        clauses[i].vars = malloc(nrVars * sizeof(int));
        clauses[i].nrVars = 0;
        while ( true ) {
            fscanf(f1, "%d", &clauses[i].vars[clauses[i].nrVars]);
            if ( clauses[i].vars[clauses[i].nrVars] == 0 )
                break;
            clauses[i].nrVars++;
        }
        // printClause(clauses[i]);
    }

    // printf("bkt soon\n");

    vars = malloc(sizeof(Vars));
    vars->vars = malloc(nrVars * sizeof(int));
    vars->nrOptions = malloc(nrVars * sizeof(int));
    vars->option = malloc(nrVars * sizeof(bool));
    vars->evaluated = malloc(nrVars * sizeof(int));
    for ( int i = 0 ; i < nrVars ; i++ ) {
        vars->vars[i] = i + 1;
        vars->nrOptions[i] = 2;
        vars->option[i] = false;
        vars->evaluated[i] = -1;
    }

    int ignoreMe = 0;

    /*
    Getting the order of the bkt algorithm
    */
    int order = 0;
    for ( int i = 0 ; i < nrClauses ; i++ ) {
        for ( int j = 0 ; j < clauses[i].nrVars; j++ ) {
            if ( vars->evaluated[abs(clauses[i].vars[j]) - 1] == -1 ) {
                ModifyOrder[order] = abs(clauses[i].vars[j]) - 1;
                // maybe order + 1
                vars->evaluated[abs(clauses[i].vars[j]) - 1] = order + 1;
                order++;
            }
        }
    }
    ModifyOrder[order] = -1;
    ModifyOrder[order + 1] = -1;
    ModifyOrder[order + 2] = -1;

    // basic bkt or bkt with conflict detection offers same score :/
    // printf("f call\n");
    if ( bktUnitPropagation(vars, clauses, nrClauses, nrVars, ignoreMe) ) {
    // if ( bktBasic(vars, clauses, nrClauses, nrVars, ignoreMe) ) {
        fprintf(out, "s SATISFIABLE\n");
        for ( int i = 0 ; i < nrVars ; i++ )
            fprintf(out, "v %d\n", vars->option[i] ? i + 1 : -(i + 1));
    } else
        fprintf(out, "s UNSATISFIABLE\n");

    // Print my ordered thingy
    // for ( int i = 0 ; i < nrVars ; i++ )
    //     fprintf(debug, "Order: %d Var num: %d\n", ModifyOrder[i], vars->vars[ModifyOrder[i]]);
}