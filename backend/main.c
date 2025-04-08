#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== Stałe ====================
#define MAX_LANES 4            // Maksymalna liczba pasów na jednej drodze
#define MAX_CONNECTIONS 2      // Maksymalna liczba możliwych kierunków z pasa
#define MAX_LINE_LENGTH 128    // Maksymalna długość linii tekstu z pliku
#define MAX_VEHICLES 100       // Maksymalna liczba pojazdów w systemie

// ==================== Struktury danych ====================

// Struktura reprezentująca pas ruchu (dojazdowy lub wyjazdowy)
typedef struct {
    int road_id;                         // ID drogi (0-3)
    float lane_id;                       // ID pasa (np. 0.1, 2.2)
    char light_color;                   // Kolor światła ('R', 'G', 'Y')
    float connections[MAX_CONNECTIONS]; // Dozwolone wyjazdy z tego pasa
    int queue_size;
    int queue[MAX_VEHICLES]; // przechowujemy indeksy z vehicles[] co jes wydajne i proste
} Lane;

// Struktura drogi z pasami wjazdowymi i wyjazdowymi
typedef struct {
    int road_id;             // Unikalny ID drogi
    Lane inbound[MAX_LANES]; // Pasy dojazdowe
    Lane outbound[MAX_LANES]; // Pasy wyjazdowe
} Road;

// Struktura pojazdu
typedef struct {
    char id[32];        // Unikalny identyfikator pojazdu
    float from_lane;    // Pas początkowy
    float to_lane;      // Pas docelowy
    int active;         // 1 = aktywny (czeka), 0 = przejechał
} Vehicle;

Vehicle vehicles[MAX_VEHICLES]; // Tablica pojazdów
int vehicle_count = 0;          // Licznik pojazdów

// Tryby działania świateł (na przyszłość)
typedef enum {
    STANDARD_MODE,
    NIGHT_MODE,
    EMERGENCY_MODE,
    PRIORITY_MODE,
    MANUAL_MODE
} LightsMode;

LightsMode currentMode = STANDARD_MODE;

// ==================== Funkcje pomocnicze ====================

// Zmienia kolory świateł w zależności od fazy (0: 0-2, 1: 1-3)
void update_lights(int phase, Road roads[]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < MAX_LANES; j++)
            roads[i].inbound[j].light_color = 'R';

    if (phase == 0) {
        roads[0].inbound[1].light_color = 'G'; // 0.1
        roads[0].inbound[2].light_color = 'G'; // 0.2
        roads[2].inbound[2].light_color = 'G'; // 2.2
        roads[2].inbound[3].light_color = 'G'; // 2.3
    } else {
        roads[1].inbound[1].light_color = 'G'; // 1.1
        roads[1].inbound[2].light_color = 'G'; // 1.2
        roads[3].inbound[1].light_color = 'G'; // 3.1
        roads[3].inbound[2].light_color = 'G'; // 3.2
    }
}

// Sprawdza, czy pojazd może legalnie jechać z danego pasa na wskazany
int is_allowed(float from_lane, float to_lane, Road roads[]) {
    int from_road = (int)from_lane;
    for (int i = 0; i < MAX_LANES; i++) {
        if (roads[from_road].inbound[i].lane_id == from_lane) {
            for (int j = 0; j < MAX_CONNECTIONS; j++) {
                if (roads[from_road].inbound[i].connections[j] == to_lane)
                    return 1;
            }
        }
    }
    return 0;
}

// Dodaje pojazd do systemu (i przypisuje go do pasa dojazdowego)
void add_vehicle(const char *id, float from_lane, float to_lane, Road roads[]) {
    int from_road = (int)from_lane;

    int vehicle_index = vehicle_count;
    if(vehicle_index>MAX_VEHICLES) return;

    strcpy(vehicles[vehicle_count].id, id);
    vehicles[vehicle_count].from_lane = from_lane;
    vehicles[vehicle_count].to_lane = to_lane;
    vehicles[vehicle_count].active = 1;
    vehicle_count++;

    for (int i = 0; i < MAX_LANES; i++) {
        if (roads[from_road].inbound[i].lane_id == from_lane) {
            int q_index = roads[from_road].inbound[i].queue_size;
            roads[from_road].inbound[i].queue[q_index] = vehicle_index;
            roads[from_road].inbound[i].queue_size++;
            break;
        }
    }
}

// Wypisuje status wszystkich pasów w konsoli
void print_lane_status(Road roads[]) {
    printf("=== STATUS PASÓW ===\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < MAX_LANES; j++) {
            if (roads[i].inbound[j].lane_id >= 0)
                printf("Droga %d | Pas %.1f | Światło %c | Czeka: %d\n",
                       roads[i].road_id, roads[i].inbound[j].lane_id,
                       roads[i].inbound[j].light_color,
                       roads[i].inbound[j].queue_size);
        }
    }
    printf("=====================\n\n");
}

// System decyduje, dla których dwóch dróg włączyć światło zielone na podstawie ilości oczekujących samochodów
int decide_phase(Road roads[], int current_phase, int phase_step_counter) { // z licznikiem max 5
    // Zlicz pojazdy na drogach 0 i 2
    int count_0_2 = 0;
    count_0_2 += roads[0].inbound[1].queue_size;
    count_0_2 += roads[0].inbound[2].queue_size;
    count_0_2 += roads[2].inbound[2].queue_size;
    count_0_2 += roads[2].inbound[3].queue_size;

    // Zlicz pojazdy na drogach 1 i 3
    int count_1_3 = 0;
    count_1_3 += roads[1].inbound[1].queue_size;
    count_1_3 += roads[1].inbound[2].queue_size;
    count_1_3 += roads[3].inbound[1].queue_size;
    count_1_3 += roads[3].inbound[2].queue_size;

    if (phase_step_counter>=5) return (current_phase==0) ? 1 : 0;

    if ((current_phase==0 && count_0_2>count_1_3) || (current_phase==1 && count_1_3>count_0_2)) return current_phase;

    return (current_phase==0) ? 1 : 0;
}

int can_proceed(Vehicle v, Road roads[]) {
    float from = v.from_lane;

    if (from == 0.1 && (
        (roads[2].inbound[3].light_color == 'G' && roads[2].inbound[3].queue_size > 0)
    )) return 0;

    if (from == 1.1 && (
        (roads[3].inbound[1].light_color == 'G' && roads[3].inbound[1].queue_size > 0)
    )) return 0;

    if (from == 2.2 && (
        (roads[0].inbound[2].light_color == 'G' && roads[0].inbound[2].queue_size > 0) ||
        (roads[0].inbound[1].light_color == 'G' && roads[0].inbound[1].queue_size > 0)
    )) return 0;

    if (from == 3.1 && (
        (roads[1].inbound[2].light_color == 'G' && roads[1].inbound[2].queue_size > 0)
    )) return 0;

    return 1; // Nie ma kolizji — pojazd może jechać
}


// Próbuje przepuścić wszystkie pojazdy z aktywnych pasów
void move_cars(Road roads[], FILE* output) {
    fprintf(output, "    {\n      \"leftVehicles\": [\n");
    int first = 1;

    for(int r=0;r<4;r++) {
        for(int l=0;l<MAX_LANES;l++) {
            Lane* lane = &roads[r].inbound[l];
            
            if(lane->lane_id<0 || lane->light_color!='G' || lane->queue_size<=0) continue;

            int v_index = lane->queue[0];

            Vehicle* v = &vehicles[v_index];

            if(v->active && can_proceed(*v, roads)) {
                for (int i=1;i<lane->queue_size;i++) {
                    lane->queue[i-1] = lane->queue[i];
                }
                lane->queue_size--;
                v->active=0;

                if (!first) fprintf(output, ",\n");
                fprintf(output, "         \"%s\"", v->id);
                first = 0;
            }
        }
    }

    // for (int v = 0; v < vehicle_count; v++) {
    //     if (!vehicles[v].active) continue;

    //     int from_road = (int)vehicles[v].from_lane;

    //     for (int i = 0; i < MAX_LANES; i++) {
    //         Lane lane = roads[from_road].inbound[i];

    //         if (roads[from_road].inbound[i].lane_id == vehicles[v].from_lane &&
    //             roads[from_road].inbound[i].light_color == 'G' &&
    //             roads[from_road].inbound[i].queue_size) {

    //             if(!can_proceed(vehicles[v], roads)) break; // jeśli z naprzeciwka coś jedzie to ten pojazd czeka
    //             roads[from_road].inbound[i].queue_size--;
    //             vehicles[v].active = 0;

    //             if (!first) fprintf(output, ",\n");
    //             fprintf(output, "         \"%s\"", vehicles[v].id);
    //             first = 0;
    //             break;
    //         }
    //     }
    // }

    fprintf(output, "\n      ]\n    },\n");
}

// Inicjalizacja dróg i pasów (łącznie z wyjazdami)
void initialize_roads(Road roads[]) {
    for (int i = 0; i < 4; i++) {
        roads[i].road_id = i;
        for (int j = 0; j < MAX_LANES; j++) {
            roads[i].inbound[j].lane_id = -1.0;
            roads[i].inbound[j].queue_size = 0;
            roads[i].outbound[j].lane_id = -1.0;
            roads[i].outbound[j].queue_size = -1;
        }
    }

    // Linie dojazdowe
    roads[0].inbound[2] = (Lane){0, 0.2, 'R', {3.0, 2.0}, 0};
    roads[0].inbound[1] = (Lane){0, 0.1, 'R', {2.1, 1.0}, 0};
    roads[1].inbound[2] = (Lane){1, 1.2, 'R', {3.0, 0.0}, 0};
    roads[1].inbound[1] = (Lane){1, 1.1, 'R', {2.0, 2.1}, 0};
    roads[2].inbound[3] = (Lane){2, 2.3, 'R', {1.0, 0.0}, 0};
    roads[2].inbound[2] = (Lane){2, 2.2, 'R', {3.0, -1}, 0};
    roads[3].inbound[2] = (Lane){3, 3.2, 'R', {2.0, -1}, 0};
    roads[3].inbound[1] = (Lane){3, 3.1, 'R', {1.0, 0.0}, 0};

    // Linie wyjazdowe
    roads[3].outbound[0] = (Lane){3, 3.0, '-', {}, 0};
    roads[2].outbound[0] = (Lane){2, 2.0, '-', {}, 0};
    roads[2].outbound[1] = (Lane){2, 2.1, '-', {}, 0};
    roads[1].outbound[0] = (Lane){1, 1.0, '-', {}, 0};
    roads[0].outbound[0] = (Lane){0, 0.0, '-', {}, 0};
}

// ==================== Funkcja główna ====================

int main() {
    Road roads[4];
    initialize_roads(roads);
    int currentPhase = 0;
    int phase_step_counter = 0;

    FILE *file = fopen("input.txt", "r");
    FILE *output_file = fopen("output.json", "w");
    if (!file || !output_file) {
        printf("Błąd otwierania plików\n");
        return 1;
    }

    fprintf(output_file, "{\n  \"stepStatuses\": [\n");

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char vehicle_id[32];
        float from_lane, to_lane;

        if (sscanf(line, "ADD %s {%f, %f}", vehicle_id, &from_lane, &to_lane) == 3) {
            int allowed = is_allowed(from_lane, to_lane, roads);
            printf("Pojazd %s: %s\n", vehicle_id, allowed ? "PRZEJAZD DOZWOLONY" : "Mandat 10 pkt. karnych!");
            if (allowed) add_vehicle(vehicle_id, from_lane, to_lane, roads);
        } else if (strncmp(line, "STEP", 4) == 0) {
            currentPhase = decide_phase(roads, currentPhase, phase_step_counter);
            update_lights(currentPhase, roads);
            printf("Status po zmianie świateł. \n");
            print_lane_status(roads);

            move_cars(roads, output_file);
            printf("Status po przejeździe samochodów. \n");
            print_lane_status(roads);

            phase_step_counter++;

            int newPhase = decide_phase(roads, currentPhase, phase_step_counter);
            if(newPhase!=currentPhase){
                currentPhase = newPhase;
                phase_step_counter =0;
            }

            printf("Wykonano PEŁNY krok symulacji (STEP)\n");
        }
    }

    fprintf(output_file, "  ]\n}\n");
    fclose(file);
    fclose(output_file);

    printf("Koniec programu: \n");
    print_lane_status(roads);
    return 0;
}
