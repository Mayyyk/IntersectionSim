# Symulator Skrzyżowania

## Zasady działania systemu

- Faza świateł trwa maksymalnie 5 kroków symulacji, ale może zakończyć się wcześniej, jeśli inny kierunek ma więcej pojazdów.
- Nie używamy świateł pomarańczowych. Zakładamy uproszczenie: pojazdy zatrzymują się, gdy zapala się czerwone.
- W jednym kroku (`STEP`) z każdego pasa może przejechać tylko jeden pojazd.
- Jeśli pojazd skręca w lewo i nadjeżdża ktoś z naprzeciwka, musi przepuścić. W tym czasie blokuje pas.
- Pojazdy, które nie mają prawa jechać, bo wybrały zły zjazd otrzymują mandat i nie trafiają na pas. Zostają zatrzymane przez "policję" :)

## Tryb deterministyczny

- Do testów stosujemy opcję `--deterministic`, aby losowe elementy symulacji były przewidywalne.

## Testowanie

- Logika systemu jest zaimplementowana w C.
- Testujemy zachowanie systemu w Pythonie, bo:
  - łatwo porównać JSONy,
  - szybciej się wykonuje,
  - nie trzeba kompilować,
  - proste do integracji z CI/CD.

### Uruchamianie testów:

- **Jednorazowe uruchomienie symulacji:**

  ```bash
  cd backend
  python run_simulation.py commands.json output.json
  ```

- **Testy ręczne:**

  ```bash
  cd IntersectionSim
  python run_simulation.py tests/test_folder/commands.json tests/test_folder/result.json --deterministic
  ```

- **Testy automatyczne:**

  ```bash
  cd IntersectionSim
  python run_tests.py
  ```

## Znane problemy / ograniczenia

- Trudność z testowaniem konkretnych linii: brak predefiniowanych warunków.

## Realistyczne usprawnienia do rozważenia

- Uwzględnienie warunków pogodowych (tryb bezpieczny).
- Tryb nocny (ekonomiczny, np. tylko żółte migające).
- Tryb awaryjny: wszędzie migające żółte.
- Priorytet dla komunikacji miejskiej (KMK).

## Instrukcja uruchomienia lokalnie

1. **Zainstaluj zależności (Node.js i Python 3 muszą być zainstalowane):**

   ```bash
   cd backend
   npm install
   ```

2. **Upewnij się, że masz plik ********`main.exe`******** (lub ********`main`******** na Linux/macOS).**

3. **Uruchom serwer lokalny:**

   ```bash
   node index.js
   ```

4. **Wejdź na adres:**

   [http://localhost:3000](http://localhost:3000)

5. **Wyślij plik z komendami JSON i pobierz wynik symulacji.**

---

Działanie aplikacji backendowej polega na:

- Przyjęciu pliku `commands.json` przez `/simulate`
- Uruchomieniu pipeline (`run_simulation.py`), który:
  1. Tworzy `input.txt`
  2. Uruchamia `main.exe` (logika w C)
  3. Naprawia `output.json`
- Zwróceniu `fixed_output.json` jako pliku `result.json` do pobrania.

