**Nume: Alexandru Alin-Ioan**

**Grupă: 312CA**

## Tema 1: Virtual Memory Allocator

### Descriere:

* Tema consta in implementarea unei memorii virtuale, cu toate operatiile
specifice acesteia.
* Parsarea comenzilor este realizata putin neortodox. Pentru toate comenzile
in afara de WRITE si MPROTECT parametrii sunt preluati direct din linia citita,
avand un maxim de 2 parametrii.
Pentru WRITE si MPROTECT restul cuvintelor de dupa primii doi parametrii sunt
stocati intr-un singur buffer.
* Daca o comanda este invalida pentru fiecare parametru (pot fi mai mult de 2)
se afisieaza mesajul de eroare. Mi se pare ciudat ca trebuie facut acest lucru.
* Pentru majoritatea operatiior este parcursa lista de blocuri cat timp adresa
primita ca parametru este mai mare decat adresa blocului. In acelasi mod se
parcurg si listele de miniblocuri.
* Fisierul utils.h contine macrouri ajutatoare pentru tratarea erorilor
* Implementarea listei dublu inlantuite foloseste scheletul din laborator

### Comentarii asupra temei:

* Crezi că ai fi putut realiza o implementare mai bună?
* Ce ai invățat din realizarea acestei teme?
* Alte comentarii

### (Opțional) Resurse / Bibliografie:

1. [Așa se adaugă un link în Markdown](https://youtu.be/dQw4w9WgXcQ)

PS: OCW strică link-ul, trebuie să puneți ***doar*** https-ul intre ().