# OS laboratorinis darbas #8

Naudojant pasirinktą programavimo aplinką sukurti programą/script’ą, kuris imituotų atminties skirstymo algoritmo veikimą. Sprendimas turi tenkinti šiuos reikalavimus:

## Bendroji dalis:
•	Kintamųjų ir funkcijų pavadinimai turi būti rašomi anglų kalba;
•	Elementai turi turėti prasmingus pavadinimus (pvz. processName vs aaa);
## Specialioji dalis:
1)	Programos veikimo pradžioje vartotojas nurodo:
•	Atminties dydį e.g. 128: 
•	Atminties skirstymo būdą: 
•	Statinis vienodo dydžio blokų: papildomai nurodo bloko dydį e.g. 8;
•	Statinis skirtingo dydžio blokų (“by default” suskirstoma į blokus kurių dydis 2^n);
•	Dinaminis. Pasirinkus šį būdą papildomai reikia nurodyti bloko parinkimo algoritmą (First-Fit, Best-Fit, Worst-Fit);
2)	Programos veikimo metu vartotojas gali įvesti procesą t.y. proceso pavadinimą ir reikiamos atminties kiekį pvz Chrome 5. Priklausomai nuo pasirinkto atminties skirstymo būdo programa turi parinkti tinkamiausią bloką (kiekvienam blokui inicializavimo metu priskiriamas numeris (adresas)) arba pranešti, kad programos vykdymui nepakanka atminties.
3)	Programa turi pildyti atminties skirstymo istoriją (log‘ą) t.y. kokiai programai kokiu metu buvo priskirtas kuris atminties blokas.
4)	Vartotojas gali peržiūrėti užimtos ir laisvos atminties žemėlapį (šarašą).
5)	Vartotojas gali atlikti atminties defragmentavimą t.y. visus naudojamus atminties blogus sugruopuoti į vieną grupę taip, kad nebūtų įsiterpusių laisvos atminties blokų.
