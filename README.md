# BGT-blockchain
# Blokų grandinių 2 užduotis
Centralizuota blokų grandinė
## Įdiegimo ir naudojimosi instrukcija
- git clone https://github.com/Matas-V/blockchain.git
- Atsidaryti projektą VS code aplinkoje
- Konsolėje (CMD) parašyti komandą *g++ -o main main.cpp -fopenmp*
- Konsolėje (CMD) paleisti sukompiliuotą main.exe tipo programą su komanda *./main*
## Kaip veikia programa?
- Sugeneruojama 1000 vartotojų
- Sugeneruojama 10000 transakcijų
- Išrenkama random 100 transakcijų bei sukuriamas merkle tree paprastuoju būdu (sudedant visų bloko transakcijų id ir gaunamas hash'as).
- Vyksta bloko kasimas, kol gaunamas reikalavimus atitinkantis hash'as
- Naujame bloke įvykdomos transakcijos
- Eliminuojamos įvykdytos transakcijos
- Naujai iškastas blokas pridedas prie pagrindinės grandinės (blockchain)
