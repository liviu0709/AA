#include <stdlib.h>
#include <string.h>
#include "functii.h"

TGL* AlocG(int nr) /* aloca spatiu pentru descriptor graf si
	                      vector de (n+1) adrese liste arce */
{
  TGL* g = (TGL*)malloc(sizeof(TGL));    /* spatiu descriptor */
  if (!g) return NULL;
  g->x = (AArc*)calloc((nr+1), sizeof(AArc)); /* spatiu vector adr.liste arce */
  if(!g->x) { free(g); return NULL; }
  g->n = nr;
  return g;
}

void DistrG(TGL** ag)
{
    for ( int i = 0 ; i <= (*ag)->n ; i++ ) {
        AArc p = (*ag)->x[i];
        while ( p != NULL ) {
            AArc aux = p;
            p = p->urm;
            free(aux);
        }
    }

    free((*ag)->x);
    free(*ag);
    *ag = NULL;
}

TGL* CitGraf(FILE *in)
{
    TGL* g = NULL;
    int s, d;

    AArc *p, aux;
    int n = 0, m;
    fscanf(in, "%d %d", &n, &m);
    g = AlocG(n + 1); // nu stim cate noduri are graful
    // aflam dupa ce citim tot
    if (!g) return 0;
    while (fscanf(in, "%d %d", &s, &d) == 2)
    {


        p = g->x + s;
        while (*p) p = &(*p)->urm;
        aux = (TCelArc*)calloc(sizeof(TCelArc),1);
        if (!aux) {
        DistrG(&g); return NULL;
        }
        aux->urm = *p; *p = aux;
        aux->d = d;
        aux->c = 0;
        // Duplicam arcul din lista de adicenta
        p = g->x + d;
        while (*p) p = &(*p)->urm;
        AArc aux2 = (TCelArc*)calloc(sizeof(TCelArc),1);
        if (!aux2) {
        DistrG(&g); return NULL;
        }
        aux2->urm = *p; *p = aux2;
        aux2->d = s;
        aux2->c = 0;
    }
    // Poate?
    // g->x = realloc(g->x, n * sizeof(AArc));
    g->n = n;

    return g;
}
