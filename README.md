**Nume: Alexandru Alin-Ioan**

**Grupă: 312CA**

## Tema 1: Virtual Memory Allocator

### Descriere:

* Tema consta in implementarea unei memorii virtuale, cu toate operatiile
specifice acesteia.
* Parsarea comenzilor se realizeaza astfel:
  * Se citeste initial intreaga linie, dupa care se separa cuvintele, primul
cuvant fiind cel dupa care se determina comanda, iar restul fiind parametrii.
* Pentru toate comenzile
in afara de WRITE si MPROTECT numarul maxim de parametrii este 2.
Pentru WRITE si MPROTECT restul cuvintelor de dupa primii doi parametrii sunt
stocati intr-un singur buffer.
    * In cazul MPROTECT acel buffer este separat mai tarziu in cuvinte si
    fiecare cuvant este transformat in numarul corespunzator permisiunii
* Daca o comanda este invalida pentru fiecare parametru (pot fi mai mult de 2)
se afisieaza mesajul de eroare. Mi se pare ciudat ca trebuie facut acest lucru.
* Pentru majoritatea operatiior este parcursa lista de blocuri cat timp adresa
primita ca parametru este mai mare decat adresa blocului. In acelasi mod se
parcurg si listele de miniblocuri.
* In cazul cazul in care ALLOC_BLOCK uneste doua liste operatia de append este
realizata prin schimbarea pointerului tail al listei precedente.
* In cazul in care FREE_BLOCK creeaza o noua lista de miniblocuri nu se adauga
unul cate unul nodurile in lista, ci doar pointerul de "head" este mutat catre
nodul urmator al celui care trebuie sters.
* Pentru read si write pozitia in buffer este aflata folosind aritmetica
cu pointeri
* Fisierul utils.h contine macrouri ajutatoare pentru tratarea erorilor.
* Implementarea listei dublu inlantuite foloseste scheletul din laborator.

### Comentarii asupra temei:

* Functiile de read si write sunt foarte asemanatoare si probabil se putea face
o modularizare mai buna pentru acestea.
* Nu am mai implementat macrouri pentru fiecare tip de eroare.
* Ce ai invățat din realizarea acestei teme? Am aprofundat lucrul cu liste.