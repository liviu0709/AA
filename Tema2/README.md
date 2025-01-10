# SAT solver

## Solutia initiala trimisa la tema:
    Acest sat solver este implementat in c si foloseste algoritmul DPLL, explicat in tema. In esenta, este un algoritm de backtracking mai eficient. Astfel, nu atribuim toate valorile posibile pentru fiecare variabila, ci doar variabilelor care nu influenteaza o clauza unitara. Deci, intai verificam toate clauzele si verificam daca avem clauze unitare. Daca exista clauze nesatisfacute compuse doar din variabile cu valori atribuite, ne oprim. Daca gasim clauze care depind de o singura variabila, atribuim valoarea corespunzatoare variabilei. Cand nu mai gasim clauze unitare, verificam daca toate variabilele au primit o valoare. Daca au primit o valoare, inseamna ca am gasit o solutie. In caz contrar, cautam o variabila fara valoare atribuita si ii aplicam ambele valori posibile, folosind clasicul algoritm de backtracking. Intre cele 2 apeluri recursive de backtracking, refacem starea initiala a variabilelor.

## Modificari aduse ulterior pentru imbunatatirea performantei:
S-a incercat implementarea unui algoritm pe baza *conflict driven clause learning* (CDCL), care este un algoritm mai eficient decat DPLL. Acesta stie sa invete din greseli, generand noi clauze pentru a preveni intalnirea unui conflict identic in viitor. Din anumite motive, functia utilizata pentru a genera noi clauze nu functioneaza exact cum trebuie. Astfel, aceasta functie genereaza niste clauze care duc la gasirea solutiei *unsatisfabile* mai repede decat daca nu ar fi utilizat.
In concluzie, tentativa de algoritm CDCL ofera performante mai bune, dar nu este perfecta.

### analyzeConflict
Aceasta functie este baza unui tipic algoritm CDCL, generand noi clauze, ce duc la rezolvarea problemei mai repede(in mod normal). Functia implementeaza pseudo-codul gasit aici [UIP](https://efforeffort.wordpress.com/2009/03/09/linear-time-first-uip-calculation/). Acest pseudocod parcurge o parte din graful de implicatii. Pentru a parcurge graful de implicatii cat mai rapid, acesta nu este construit efectiv in memorie, ci ne folosim de o structura de date similara cu stiva, care tine minte nodurile parcurse si cauzele care au dus la acestea. In final, se genereaza o clauza noua, care contine toate variabilele care au dus la conflict. Aceasta clauza este adaugata in lista de clauze si se reia algoritmul DPLL.

### Unit Propagation
Fata de metoda simpla din tema, cand la intalnirea unui conflict pur si simplu ne intoarcem un nivel mai sus in arborele de decizii, se genereaza o noua clauza care *ar trebui* sa previna intalnirea aceluiasi conflict in viitor. Aceasta clauza este adaugata in lista de clauze si abia dupa aceea ne intoarcem un nivel mai sus in arborele de decizii.

### Watched literals
Unele teste contin clauze cu relativ multe variabile. Pentru a obtine solutia mai rapid, nu verificam valoarea de adevar a fiecarei variabile dintr-o clauza. Urmarim doar 2 variabile din clauza, pe care le numim *watched literals*. Daca una dintre aceste variabile devine falsa, cautam o alta variabila din clauza care sa fie adevarata. Daca nu gasim nicio variabila adevarata, inseamna ca am ajuns la un conflict.

### Learnt clauses
Clauzele invatate se adauga in lista de clauze (inaintea celor initiate pentru a duce la propagarea unitara cat mai rapid). Din motive de performante, am ales sa folosesc doar clauzele cu maxim 10 variabile. In plus, nu tin minte decat ultimele 200 de clauze invatate. Cand se ajunge la 200 de clauze invatate, se sterg primele 100 clauze invatate.

### Euristici
-   Aparent, daca o clauza este validata pe un nivel de decizie si o ignoram pe urmatoarele niveluri, se observa o usoara imbunatatire a performantei. Teoretic, diferenta nu ar trebui sa fie semnificativa, deoarece sistemul de *watched literals* ar duce la doar 1/2 verificari per clauza.
-   Pentru ghicirea valorii unei variabile, am ales o euristica simplista: variabila care apare in cele mai multe clauze are prioritate. Am ales sa ignor valoarea acesteia (nu am observat imbunatatiri semnificative daca as fi tinut cont de semnul cu care apare si sa atribui intai valoarea 0 sau 1, in functie de care apare mai des). Ca si alte tentative de rezolvare, am incercat sa implementez si partea de decay in scorul variabilelor, dar din nou, pe implementarea care o am, nu am vazut diferente si am renuntat la ea.
