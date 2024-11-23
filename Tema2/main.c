#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int *vars;
    int nrVars;
} Clause;

typedef struct {
    int *vars;
    bool *option;
    int *evaluated;
} Vars;

bool bktUnitPropagation(Vars *start, Clause *c, int nrClauses, int nrVars, int level) {

    int freeVarIndex = -1;
    // Unit clause propagation
    for ( bool unitClause = true ; unitClause ; ) {
        unitClause = false;
        for ( int i = 0 ; i < nrClauses ; i++ ) {
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
            if ( clauseChecksOut == false && freeVar == 0 )
                return false;
            // We can do propagation :)
            if ( clauseChecksOut == false && freeVar == 1 ) {
                start->evaluated[freeVarIndex] = level + 1;
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
    // Keep going!
    // Find free var
    for ( int i = 0 ; i < nrVars ; i++ ) {
        if ( start->evaluated[i] == -1 ) {
            freeVarIndex = i;
            break;
        }
    }
    if ( start->evaluated[freeVarIndex] == -1 ) {
        start->evaluated[freeVarIndex] = level + 1;
        start->option[freeVarIndex] = true;
        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1) )
            return true;

        // Reset modified vars by recursion
        for ( int i  = 0 ; i < nrVars ; i++ ) {
                if (start->evaluated[i] > level + 1) {
                    start->evaluated[i] = -1;
            }
        }

        start->option[freeVarIndex] = false;
        if ( bktUnitPropagation(start, c, nrClauses, nrVars, level + 1) )
            return true;
        start->evaluated[freeVarIndex] = -1;
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
            clauses = malloc(nrClauses * sizeof(Clause));
            break;
        } if ( word[0] == 'c' )
            fgets(word, 100, f1);
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
    }

    vars = malloc(sizeof(Vars));
    vars->vars = malloc(nrVars * sizeof(int));
    vars->option = malloc(nrVars * sizeof(bool));
    vars->evaluated = malloc(nrVars * sizeof(int));
    for ( int i = 0 ; i < nrVars ; i++ ) {
        vars->vars[i] = i + 1;
        vars->option[i] = false;
        vars->evaluated[i] = -1;
    }

    int ignoreMe = 0;

    if ( bktUnitPropagation(vars, clauses, nrClauses, nrVars, ignoreMe) ) {
        fprintf(out, "s SATISFIABLE\n");
        for ( int i = 0 ; i < nrVars ; i++ )
            fprintf(out, "v %d\n", vars->option[i] ? i + 1 : -(i + 1));
    } else
        fprintf(out, "s UNSATISFIABLE\n");

    fclose(f1);
    fclose(out);
    for ( int i = 0 ; i < nrClauses ; i++ )
        free(clauses[i].vars);
    free(clauses);
    free(vars->vars);
    free(vars->option);
    free(vars->evaluated);
    free(vars);
}