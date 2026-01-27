<h1 style="text-align:center;">
Implementazione di una libreria C++<br> per la modellizzazione dell'interazione tra due specie<br> mediante le equazioni di Lotka-Volterra
</h1>

<p style="text-align:center;">
<strong>Arturo Busetto (0000925476)</strong><br>
DIFA, UniBo<br>
<em>Data di svolgimento: 23/06/2025</em>
</p>

## Scopo del progetto

Durante la progettazione del codice, l'obiettivo principale è stato lo sviluppo di una libreria essenziale, ma sufficientemente completa, in grado di simulare, attraverso la discretizzazione delle [equazioni di Lotka-Volterra](https://github.com/Programmazione-per-la-Fisica/progetto2024/blob/main/volterra.md#le-equazioni-di-lotka-volterra), l'evoluzione temporale di un ecosistema costituito da una popolazione di prede e una di predatori. Vengono inoltre forniti alcuni esempi di utilizzo dei metodi offerti dalla libreria, ritenuti rappresentativi delle esigenze di un'ampia gamma di livelli di competenza da parte dell'utente.

## Struttura del progetto

- [Presentazione della libreria](#presentazione-della-libreria)
    - [Attributi](#attributi)
    - [Invariante di classe](#invariante-di-classe)
    - [Costruttori](#costruttori)
    - [Metodi di calcolo](#metodi-di-calcolo)
    - [Metodi di output](#metodi-di-output)
- [Strategia di test](#strategia-di-test)
- [Esempi di utilizzo](#esempi-di-utilizzo)
    - [`beta.main.cpp`](#betamaincpp)
    - [`alpha.main.cpp`](#alphamaincpp)
    - [`sigma.main.cpp`](#sigmamaincpp)
- [Interpretazione dei risultati](#interpretazione-dei-risultati)
- [Istruzioni per l'esecuzione](#istruzioni-per-lesecuzione)

## Presentazione della libreria

La libreria è composta da due file principali: un file di interfaccia (`project.lv.hpp`) e un file di implementazione (`project.lv.cpp`).
Nel file di interfaccia sono definite le strutture `Parameters`, `State` e `Statistics`, utilizzate per rappresentare rispettivamente i parametri del modello, lo stato del sistema e i dati statistici raccolti durante la simulazione.
All'interno dello stesso file è inoltre definita la classe `Simulation` e sono dichiarate alcune funzioni libere e operatori.
Il file di implementazione contiene invece le definizioni dei metodi della classe, delle funzioni libere e degli operatori.

## Attributi
Gli attributi della classe `Simulation` sono stati progettati partendo dalle variabili presenti nelle equazioni del modello, dalla precisione desiderata nella discretizzazione temporale e dal numero di iterazioni previsto per l’evoluzione del sistema.
In particolare, quattro attributi rappresentano l'informazione fondamentale da fornire al costruttore:

- i parametri del modello (`par_`),
- lo stato del sistema (`state_`),
- l’intervallo temporale di discretizzazione (`dt_`),
- il numero di iterazioni da eseguire (`iterations_`).

Oltre a questi, la classe contiene un oggetto `evolution_` di tipo `std::vector<State>`, utilizzato per memorizzare l’evoluzione temporale del sistema, e due oggetti di tipo `Statistics` per raccogliere le statistiche relative alle popolazioni di prede (`prey_stat_`) e predatori (`pred_stat_`).

```c++ 
class Simulation {
  Parameters const par_;
  double const dt_;
  std::size_t const iterations_;
  State state_;
  std::vector<State> evolution_;
  Statistics prey_stat_;
  Statistics pred_stat_;
    . . .
 public:
    . . .
};
```
Nel seguente frammento di codice viene approfondita, con ovvio significato delle variabili, la struttura interna di `Parameters`, di `State` e di `Statistics`.

```c++
struct Parameters {
  double A;  // prey birth rate
  double B;  // prey death rate
  double C;  // predator birth rate
  double D;  // predator death rate
};

struct State {
  double prey;
  double predator;
  double H;
};

struct Statistics {
  double mean;
  double sigma;
  double maximum;
  double minimum;
};
```

## Invariante di classe

Al fine di garantire la coerenza e la correttezza dello stato interno della classe `Simulation`, viene definito un invariante di classe che riflette le condizioni imposte dal modello matematico e dalla strategia risolutiva adottata.
In particolare, si richiede che le seguenti proprietà siano sempre soddisfatte:

1. I parametri contenuti in `par_` devono essere numeri reali strettamente maggiori di zero;
2. Le popolazioni `state_.prey` e `state_.predator` devono assumere valori reali strettamente maggiori di zero;
3. L’intervallo temporale `dt_` deve essere un numero reale strettamente maggiore di zero;
4. Il numero di iterazioni `iterations_` deve essere un numero naturale.

Come conseguenza diretta, si assume che ogni elemento (`State`) memorizzato nel vettore `evolution_` rispetti le stesse condizioni imposte alle popolazioni rappresentate nell'attributo `state_`.

## Costruttori

Per garantire che vengano create unicamente istanze coerenti con l'invariante di classe, il [costruttore principale](#costruttore-principale) è stato reso privato; l'interfaccia pubblica prevede tre costruttori distinti che implementano una logica di costruzione delegata.

#### Costruttore con argomenti espliciti:
```c++
  Simulation(Parameters p, double prey, double pred, double t, double n) :
    Simulation(is_good(p), is_good(prey), is_good(pred), dt_good(t), it_good(n)) {}
```
#### Costruttore con inserimento manuale:
```c++
  Simulation() : Simulation(set_simulation()) {}
```
#### Costruttore con file di inizializzazione:
```c++
  Simulation(std::string const& name) : Simulation(set_simulation(name)) {}
```

Prendendo in esame il [primo costruttore](#costruttore-con-argomenti-espliciti), si osserva che, al momento della delega, gli argomenti forniti vengono passati al [costruttore principale](#costruttore-principale) esclusivamente tramite funzioni di validazione. Tali funzioni hanno il compito di verificare il rispetto dei vincoli imposti dall’invariante di classe e, in caso di valori non conformi, sollevano un’eccezione, impedendo l’inizializzazione dell’oggetto.<br>
Il [secondo](#costruttore-con-inserimento-manuale) e il [terzo](#costruttore-con-file-di-inizializzazione) costruttore si basano invece su una funzione `set_simulation`, la quale consente di inserire manualmente i parametri della simulazione oppure di fornire un percorso al file di inizializzazione. Entrambi gli overload restituiscono un oggetto soltanto qualora i controlli effettuati tramite i metodi di validazione sopra citati risultino superati con successo.<br>
Il [costruttore principale](#costruttore-principale), riporato nel seguente frammento di codice, esegue inizialmente il calcolo dell’integrale primo (`state_.H`) e provvede a salvare lo stato iniziale completo all’interno del vettore `evolution_`. Successivamente, converte i valori delle popolazioni in frazioni del [secondo punto di equilibrio](https://github.com/Programmazione-per-la-Fisica/progetto2024/blob/main/volterra.md#note-sulla-dinamica-del-sistema) del sistema.

#### costruttore principale:
```c++ 
  explicit Simulation(Parameters par, double prey, double pred, double dt, std::size_t it)
      : par_(par), dt_(dt), iterations_(it) {
    // DEFINING INITIAL STATE
    state_.H = (par_.C * prey) + (par_.B * pred) - ((par_.D * std::log(prey)) + (par_.A * std::log(pred)));

    evolution_.push_back(State{prey, pred, state_.H});

    // SWITCHING TO RELATIVE COORDINATES FOR COMPUTATIONS
    state_.prey = prey * par_.C / par_.D;
    state_.predator = pred * par_.B / par_.A;
  }
```

## Metodi di calcolo

L’evoluzione temporale del sistema è affidata al metodo `evolve()`, in cui viene implementata la [versione discretizzata delle equazioni di Lotka-Volterra](https://github.com/Programmazione-per-la-Fisica/progetto2024/blob/main/volterra.md#versione-discretizzata-delle-equazioni).Il metodo applica un singolo passo evolutivo aggiornando lo stato relativo delle popolazioni, a partire dallo stato corrente.
Una volta eseguito il calcolo, il metodo verifica che il nuovo stato sia conforme ai vincoli imposti dall’invariante di classe. In caso di violazione, viene ripristinato lo stato precedente e viene sollevata un’eccezione di tipo `std::runtime_error`, segnalando un’anomalia nel processo evolutivo. Diversamente, lo stato appena calcolato viene convertito nella rappresentazione assoluta e aggiunto al vettore `evolution_`.

```c++
void Simulation::evolve() {
  ...
  if (!(state_.prey > 0) || !(state_.predator > 0)) {
    state_.prey = x_rel;
    state_.predator = y_rel;
    throw std::runtime_error{"WWF joined the simulation.\n"}; 
  } else {
    evolution_.push_back(State{ ... }); // stato contenente il valore assoluto degli individui
  }
}
```
Il metodo `compute()` gestisce l’esecuzione dell’intero processo evolutivo, chiamando ripetutamente il metodo `evolve()` fino al raggiungimento del numero di iterazioni specificato in fase di inizializzazione, oppure fino a quando quest’ultimo non sollevi un’eccezione. In tal caso, l’eccezione viene intercettata e il relativo messaggio d’errore viene stampato su `std::cerr`, interrompendo il calcolo dell’evoluzione.


L’analisi statistica delle popolazioni simulate è affidata al metodo `statistics()`, che calcola gli indicatori ritenuti più significativi (_media, deviazione standard, massimo e minimo_) per descrivere l’andamento temporale delle popolazioni. I risultati dei calcoli vengono memorizzati all'interno degli [attributi](#attributi) `prey_stat_` e `pred_stat_`. 


## Metodi di output

La classe mette a disposizione diversi metodi per la visualizzazione dei risultati della simulazione, consentendo la stampa sia su [schermo](#metodi-di-stampa-su-schermo) sia su [file](#metodi-di-stampa-su-file), con la possibilità di personalizzare il numero di cifre significative. In particolare, la generazione dei grafici avviene tramite l’invocazione automatica di `gnuplot`, con la creazione e successiva eliminazione di file temporanei necessari alla graficazione.


#### [Metodi di stampa su schermo](#metodi-di-stampa-su-schermo):

- `show_evolution(int p)`, permette di stampare a schermo l’evoluzione temporale delle popolazioni e del corrispondente valore dell'integrale primo;

- `show_statistics(int p)`, consente di visualizzare i principali indicatori statistici e i grafici relativi all’andamento delle popolazioni e dell'integrale primo nel tempo;
- `show_trajectory()`, mostra la traiettoria dell’ecosistema nello spazio delle fasi (predatori vs. prede).

#### [Metodi di stampa su file](#metodi-di-stampa-su-file)
Per ognuno dei metodi sopra menzionati è disponibile una corrispondente funzione di salvataggio (`save_evolution`, `save_statistics`, `save_trajectory`) che richiede, al momento della chiamata, anche il path del file da utilizzare per l’output.

## Strategia di test

La correttezza e la robustezza della classe `Simulation` vengono verificate, mediante gli strumenti forniti da [**doctest**](https://github.com/doctest), nel file `project.test.cpp`. Le suite di test sono articolate in più casi e coprono la validazione delle eccezioni sollevate dai costruttori in presenza di parametri non validi o file di inizializzazione errati (contenuti in `.bad-init-files`), oltre a verificare la corretta costruzione degli oggetti in presenza di dati coerenti. Vengono inoltre testati i [metodi di calcolo](#metodi-di-calcolo): tramite confronti approssimati — imposti dalla rappresentazione `double` dei valori numerici — viene verificato che lo stato interno, l’evoluzione temporale e le statistiche della simulazione siano compatibili con i risultati ottenibili da un calcolo manuale.

## Esempi di utilizzo

Gli esempi di `main` sono elencati  di seguente, corredati da una breve descrizione, e ordinati secondo una stima qualitativa dello sviluppo osseo della regione mascello-mandibolare di un possibile utente (_criterio di [Gigs Chadmann](figs/gigschadmann.jpg)_).

### `beta.main.cpp`

Dopo il breve ed entusiasmante inserimento dei parametri richiesto dal [costruttore di default personalizzato](#costruttore-con-inserimento-manuale), viene stampato sul terminale un menù interattivo(`std::string const menu`). Questo consente all’utente di eseguire sull’istanza le operazioni offerte dai [metodi di calcolo](#metodi-di-calcolo) e dai [metodi di output](#metodi-di-output).

```c++
int main() {
  try {
    ...
    std::string cmd{};

    while (std::cin >> cmd) {
      if (cmd == /*opzione*/ ) {
        ...
      } else if (cmd == /*altra opzione*/ ) {
        ...
      }
      .
      .
      . 
        else {
        std::cout << "Bad format, insert a new command\n";
        ...
      }
    }

    return EXIT_SUCCESS;

  } catch ( /*oggetto sollevato*/ ) {
    std::cerr << /*messaggio*/ ;
    return EXIT_FAILURE;
  }
}
```

### `alpha.main.cpp`


Questo esempio mostra l'utilizzo del [costruttore parametrico](#costruttore-con-argomenti-espliciti), inizializzato con valori numerici espliciti dell'oggetto `Parameters`, delle popolazioni e dei parametri dell'evoluzione temporale. Dopo l'invocazione del metodo `compute()`, viene verificata creata, se necessario,la directory `results` tramite il metodo [`std::system`](https://en.cppreference.com/w/cpp/utility/program/system). I risultati della simulazione vengono quindi salvati nei file `results/alpha.sim.statistics.txt`, `results/alpha.sim.evolution.csv` e `results/alpha.sim.trajectory.svg`. Eventuali eccezioni sollevate durante l’esecuzione del blocco `try` vengono catturate, causando la terminazione dell’esecuzione con la stampa del messaggio di errore corrispondente.

```c++
int main() {
  try {
    lv::Simulation s(lv::Parameters{2000., 4., 8., 1000.}, 200., 100., 0.00001, 100000.);

    s.compute();

    if (system("mkdir -p results") != 0) {
      throw std::runtime_error{"Unable to create results directory"};
    }

    s.save_evolution(6, "results/alpha.sim");
    s.save_trajectory("results/alpha.sim");
    s.save_statistics(6, "results/alpha.sim");
    
    return EXIT_SUCCESS;

  } catch ( /*oggetto sollevato*/ ) {
    std::cerr << /*messaggio*/ ;
    return EXIT_FAILURE;
  }
}
```

### `sigma.main.cpp`

Il seguente codice mostra un utilizzo avanzato della libreria, eseguendo automaticamente una collezione di simulazioni. In fase iniziale, il programma genera un elenco dei file presenti nella directory `simulations`, salvandolo nel file temporaneo `simulations_list.txt`. Per ciascun file elencato, viene tentata la costruzione di un oggetto `Simulation`, inizializzato tramite il corrispondente file di configurazione, fornito al [costruttore apposito](#costruttore-con-file-di-inizializzazione). L’istanza così creata, insieme a una stringa che ne rappresenta il nome identificativo, viene incapsulata nella struttura `Simulations`, andando a costituire un alamento del vettore `S`. Ciascuna simulazione viene eseguita autonomamente e i risultati vengono salvati con la stessa procedura seguita nel codice di [`alpha.main.cpp`](#alphamaincpp). Eventuali errori di inizializzazione o di calcolo relativi a singole simulazioni non interrompono l'esecuzione complessiva, ma vengono notificati all'utente tramite messaggi di errore dedicati. La terminazione avviene con valore di uscita `EXIT_SUCCESS`, salvo intercettazione di eccezioni inaspettate.

```c++
int main() {
  try {
    struct Simulations {  //struct utilizzato per associare un nome a ogni oggetto
      lv::Simulation sim;
      std::string name;
    };

    std::vector<Simulations> S{};

    if (system("ls simulations >> simulations_list.txt") != 0) {
      throw std::runtime_error{"Unable to access setting files"};
    }

    std::ifstream sim_list{"simulations_list.txt"};
    ...

    while (lv::read(sim_list, file_name, '\n')) {
      ... ciclo di istanziazione degli oggetti lv::Simulation ...
    }
    ...

    for (auto& s : S) {
      try {
        ... calcolo e salvataggio risultati ...
      } catch (const std::exception& e) {
        std::cerr << "Simulation " << s.name << " failed" << e.what() << '\n';
      }
    }

    return EXIT_SUCCESS;

  } catch ( /*oggetto sollevato*/ ) {
    std::cerr << /*messaggio*/ ;
    return EXIT_FAILURE;
  }
}
```

## Interpretazione dei risultati

L’approccio adottato per risolvere le equazioni differenziali che descrivono l’evoluzione del sistema genera orbite qualitativamente chiuse quando la discretizzazione è impostata con una finezza dell’ordine di 10<sup>−7</sup> secondi o inferiore. In caso contrario, si osserva un progressivo allontanamento dal secondo punto di equilibrio, che comporta la violazione della costanza dell’integrale primo nel tempo.

## Istruzioni per l'esecuzione


Il progetto utilizza [**CMake**](https://cmake.org/) come sistema di build. Nel file di configurazione `CMakeLists.txt` viene richiesta una versione minima di CMake (3.28) e la presenza di `gnuplot`[*](#installazione-di-gnuplot) installato. Il file imposta lo standard C++20 e abilita una serie di flag di compilazione per garantire massima attenzione ai warning e alla sicurezza, inclusi i sanitizer per il debug e flag di hardening specifici del compilatore utilizzato. Il progetto definisce tre eseguibili (`sigma`, `alpha`, `beta`), ciascuno composto da un file `main` differente e il file sorgente principale `project.lv.cpp`. Il sistema di testing, abilitato di default, compila l'eseguibile `project.t` e lo registra come test eseguibile, permettendo così l’esecuzione automatizzata dei casi di test definiti.

#### Installazione di gnuplot
Qualora sulla macchina dell’utente non fosse presente `gnuplot`, procedere con l’installazione tramite i seguenti comandi da terminale:

```bash
$ sudo apt update
$ sudo apt install gnuplot 
```

Per assicurarsi che i file `.gp` temporanei vengano eseguiti correttamente, assicurarsi che la versione installata sia la `5.4.2` o superiore eseguendo:

```bash
$ gnuplot --version
```


<p style="text-align:center;">
  <strong>Dichiarazione sull'utilizzo di IA generativa:</strong><br>
  <em>
  La stesura della presente relazione è satata supportata dalla versione per utenti non registrati di 
  <a href="https://chatgpt.com" target="_blank" rel="noopener noreferrer">ChatGPT</a> per migliorare la fluidità dei periodi e uniformare lo stile di scrittura. La progettazione e l'implementazione del codice riflettono scelte prese unicamente e interamente dal Mastro Sviluppatore (Arturo Busetto).
  All'intelligenza artificiale sono state riservate mansioni principalmente didaticche, come chiarimenti sulle funzioni offerte dalla <a href="https://en.cppreference.com/w/cpp/standard_library.html" target="_blank" rel="noopener noreferrer">Standard Library</a> e la traduzione di messaggi di errore scoraggiantemente lunghi. In aggiunta, come risorse per apprendere nuove strategie implementative, sono state utilizzate le soluzioni ai laboratori del corso di <a href="https://github.com/Programmazione-per-la-Fisica" target="_blank" rel="noopener noreferrer">Programmazione per la Fisica</a> e gli esempi degli utenti di <a href="https://stackoverflow.com" target="_blank" rel="noopener noreferrer">Stack Overflow</a>.
  </em>
</p>
