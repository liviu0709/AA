/* STOICA Liviu-Gabriel - 311CB */
#ifndef FUNCTII_H
#define FUNCTII_H
#include <stdio.h>

typedef struct celarc
{ int d;           /* destinatie arc */
  int c;            /* 0 -> nemonitorizat ; 1 -> monitorizat */
  struct celarc *urm; /* adresa urmatorul adiacent */
} TCelArc, *AArc;

typedef struct
{ int n;     /* numar noduri */
  AArc* x;    /* vector adrese liste arce */
} TGL;

void DistrG(TGL** ag); /* distruge graf -> punctaj valgrind */
TGL* AlocG(int nr); /* aloca memorie pentru un graf */
TGL* CitGraf(FILE*);  /* citeste descriere graf */

#endif
