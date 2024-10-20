# Tema 1 - Skyscrapers

## Verificare pe linii
    - Aceasta este o operatie triviala, liniile fiind delimitate in stanga si in dreapta cu :. Se trece de la o linie la alta dupa simbolul #.
    - Pentru fiecare linie in parte, incepem cautarea tuturor cifrelor de la 1 la 4. Cautam fiecare cifra pe rand.
    - Daca nu gasim una din ele, linia nu este valida

## Verificare pe coloane
    - Pentru validarea coloanelor, dificultatea consta in gasirea cifrelor care sunt pe coloana respectiva. Folosind simbolurile din encodarea data, parcurgem banda pe coloane.
    - Cand o cifra de pe coloana cautata este gasita, aceasta este inlocuita cu litera corespunzatoare: a, b, c, d
    - Cand capul de citire ajunge la capatul benzii se cauta pe toata banda cate o litera din fiecare. Daca nu este gasita una din ele, coloana nu este valida.

## Verificare nr turnuri vizibile
    - Pentru fiecare cifra de pe marginea zonei 4x4 parcugem linia/coloana necesara pentru verificarea acesteia. Inlocuim turnurile vizibile cu litere si tinem minte inaltimea maxima intr-o stare. Dupa aceea, parcurgem iar linia/coloana si numaram literele. Comparam numarul dat cu cel calculat.
    - Validarea vizibilitatii turnurilor consta in etape:

* Validarea pe linii:
    - Reutilizam starile pentru parcurgerea pe linii (cu o simpla modificare a numelui)
    - Fata de verificarea coloanelor, aceasta este mult mai simpla si necesita un numar redus de pasi

* Validarea pe coloane:
    - Reutilizam starile pentru parcurgerea pe coloane, marcam in acelasi mod elementele unei coloane
    - Parcurgem toata banda in cautarea literelor respective si le inlocuim cu literele mari(A, B, C, D), daca sunt vizibile.

### Nota
    Numele starilor au fost gandite astfel incat sa poata fi copiate si modificate foarte usor, pentru reutilizare. Astfel, majoritatea starilor contin Lx sau Cx pentru a sti carei linii/coloana ii corespund. In urma reutilizarii coloanelor... pe langa Cx s-a ajuns si la denumiri precum Cvx si Cvvx. Cvx corespunde parcurgerii coloanei x in vederea validarii vizibilitatii de jos a coloanei. Cvvx corespunde parcurgerii coloane x in vederea validarii vizibilitatii de sus a coloanei.
