#include <stdio.h>
#include <stdlib.h>

#include "functii.h"

int maximL(int *x, int n) {
    int sol = 0;
    for ( int i = 1 ; i <= n ; i++ ) {
        if ( x[i] > sol )
            sol = x[i];
    }
    return sol;
}

void marcheazaNod(TGL *x, int nod) {
    AArc p = x->x[nod];
    while ( p ) {
        p->c = 1;
        // Marcheaza si celalalt sens al legaturii...
        AArc pp = x->x[p->d];
        while( pp ) {
            if ( pp->d == nod ) {
                pp->c = 1;
                break;
            }
            pp = pp->urm;
        }

        p = p->urm;
    }
}

int nrLegaturi(TGL *x, int nod) {
    AArc p = x->x[nod];
    int nrLegg = 0;
    while ( p ) {
        if ( p->c == 0 ) // nu e vizitat
            nrLegg++;
        p = p->urm;
    }
    return nrLegg;
}

int main(void) {
    // Fisier intrare
    FILE *in = fopen("2.in", "r");
    TGL *x = CitGraf(in);
    int *nrLeg = malloc(sizeof(int) * ( x->n + 1 ));

    int nuAmTerminat = 1, solutie = 0;

    while ( nuAmTerminat ) {

        for ( int i = 1 ; i <= x->n ; i++ ) {
            nrLeg[i] = nrLegaturi(x, i);
        }
        int max = maximL(nrLeg, x->n);
        // Am vizitat tot!
        if ( max == 0 ) break;
        for ( int i = 1 ; i <= x->n ; i++ ) {
            if ( nrLeg[i] == max ) {
                marcheazaNod(x, i);
                solutie++;
                break;
            }
        }
    }

    printf("Solutie: %d", solutie);
    free(nrLeg);
    DistrG(&x);
    fclose(in);
}