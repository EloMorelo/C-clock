# PL Zegar c++
zakładamy stałą liczbę procesów i kroków symulacji (5,10)

Mutexy (clock_mutexes) do synchronizacji dostępu do zegarów \
liczby atomowe - https://stackoverflow.com/questions/31978324/what-exactly-is-stdatomic

# processFunction
processFunction jest wykonywana przez każdy proces.
Losuje losowe przyrosty do zegara, aktualizuje go, a następnie symuluje wysyłanie i odbieranie wiadomości między procesami

każdy krok reprezentuje jednostke czasu w symulacji
for (int step = 0; step < SIMULATION_STEPS; ++step)

losujemy wartość od 1 do 5 (dis(gen)) i dodajemy bazowy przyrost czasu dla danego procesu (id)
int randomIncrement = baseIncrements[id] + dis(gen);

mutex zapewnia ochrone dostepu do zegara i dodajemy losowy przyrost czasu
std::lock_guard<std::mutex > lock(clock_mutexes[id]);
clocks[id] += randomIncrement;

Pętla iteruje przez każdą parę w wektorze messageSequence (nadawca, odbiorca) 
Sprawdzamy, czy aktualny proces (id) jest nadawcą wiadomości w aktualnym kroku (step). Jeśli tak to pobieramy czas nadawcy i wysyłamy go do odbiorcy.

Czas nadawcy jest pobierany w bloku `{ std::lock_guard<std::mutex> lock(clock_mutexes[id]); sender_time = clocks[id]; }`. Chroni to dostęp do zegara nadawcy przed jednoczesnym dostępem innego wątku.

Następnie zegar odbiorcy jest aktualizowany na podstawie maksymalnej wartości pomiędzy jego obecnym czasem a czasem nadawcy.

for (const auto& pair : messageSequence) {
 if (pair.first == id && step == pair.second) {
  int recipient = pair.second;
   int sender_time;
    {
     std::lock_guard<std::mutex > lock(clock_mutexes[id]);
      sender_time = clocks[id]; }
       std::lock_guard<std::mutex > lock(clock_mutexes[recipient]);
        clocks[recipient] = std::max(clocks[recipient].load(), sender_time);  } }
