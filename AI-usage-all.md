Faza 1
    În prima fază, am apelat la AI în special pentru a genera rapid scheletele de funcții pentru operațiile standard cu fișiere în C (`open`, `read`, `write`, `mkdir`). Proiectul cerea crearea unei ierarhii stricte de directoare (pentru districte, inspectori și rapoarte), așa că am vrut să mă asigur că setez corect permisiunile din octeți (0755).
    Codul inițial generat de AI pentru crearea directoarelor crăpa sau returna eroare dacă rulam programul a doua oară (deoarece directoarele existau deja). Am modificat manual logica, adăugând verificarea lui `errno == EEXIST`, astfel încât sistemul să poată fi rulat de mai multe ori fără să șteargă datele vechi sau să se oprească cu eroare. De asemenea, logica de parsare a fișierelor de configurare am scris-o manual, deoarece AI-ul tindea să ofere soluții prea generice cu `fscanf`, care nu gestionau bine liniile libere sau formatul fix cerut.

Faza 2 
    Faza a doua a crescut complexitatea prin introducerea proceselor paralele (`fork`, `execvp`, `waitpid`) și a monitorului în background. Am folosit AI-ul pentru a-mi clarifica ciclul de viață al unui proces clonat și cum se face managementul corect al unui fișier de tip lockfile (`monitor_pid`), pentru a ști dacă monitorul rulează deja.
    Aici a apărut o problemă majoră de arhitectură. AI-ul a avut tendința inițială de a-mi genera o soluție pur comportamentală și secvențială – adică o înșiruire de funcții într-un singur flux care rezolva task-urile pe rând. 

    Conform cerințelor stricte ale proiectului, a trebuit să refuz această abordare și să restructurez totul într-un mod **strict structural**. Am spart programul în componente software clar delimitate, asemenea unor blocuri hardware, unde granițele dintre procese sunt fixe și comunicarea se face strict prin mecanisme controlate, eliminând stările globale interdependente pe care AI-ul încerca să le introducă. 

    Tot manual am rezolvat și problemele de concurență (race conditions) la verificarea PID-ului, asigurându-mă că scrierea în `monitor_pid` se face atomic.
Faza 3
    Aceasta a fost cea mai dificilă fază din punct de vedere al sincronizării. Introducerea CLI-ului interactiv în `city_hub`, pornirea monitorului prin intermediul `hub_mon` (dublu fork) și spawn-area proceselor `scorer` în paralel au generat bug-uri destul de complexe. Am folosit AI-ul pentru a înțelege exact cum se mapează descriptorii de fișiere în tabela procesului atunci când folosesc `dup2()`.
    