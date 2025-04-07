#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LANES 4  // Maksymalna liczba linii na drodze
#define MAX_CONNECTIONS 2 // Maksymalna liczba kierunków dla linii dojazdowej
#define MAX_LINE_LENGTH 128
#define MAX_VEHICLES 100

// STRUKTURY -----------------------------

// Struktura reprezentująca linię drogową
typedef struct {
    int road_id;         // ID drogi (0-3)
    float lane_id;       // ID linii np. 0.1, 1.0
    char light_color;    // 'R' - czerwone, 'G' - zielone, 'Y' - żółte
    float connections[MAX_CONNECTIONS]; // ID linii, na które można jechać (tylko dla dojazdowych)
    int waiting_vehicles; // Liczba pojazdów oczekujących na tej linii
} Lane;

// Struktura reprezentująca drogę
typedef struct {
    int road_id;       // ID drogi (0 - Stachiewicza, 1 - Wybickiego, 2 - Wrocławska, 3 - Radzikowskiego)
    Lane inbound[MAX_LANES]; // Linie dojazdowe
    Lane outbound[MAX_LANES]; // Linie wyjazdowe
} Road;

// Struktura reprezentująca samochód
typedef struct 
{
    char id[32];
    float from_lane;
    float to_lane;
    int active; // 1 - czeka, 0 - przejechał
} Vehicle;

Vehicle vehicles[MAX_VEHICLES];
int vehicle_count = 0;

typedef enum {
    STANDARD_MODE,        // Tryb standardowy (dzienny)
    NIGHT_MODE,           // Tryb nocny (ekonomiczny)
    EMERGENCY_MODE,       // Tryb awaryjny
    PRIORITY_MODE,       // Tryb priorytetu
    MANUAL_MODE           // Tryb manualny
} LightsMode;

LightsMode currentMode = STANDARD_MODE;

//Aktualizacja świateł
void update_lights(int phase, Road roads[]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < MAX_LANES; j++) {
            roads[i].inbound[j].light_color = 'R';
        }
    }
    if (phase == 0) { // 0<->2
        roads[0].inbound[1].light_color = 'G'; // 0.1
        roads[0].inbound[2].light_color = 'G'; // 0.2
        roads[2].inbound[2].light_color = 'G'; // 2.2
        roads[2].inbound[3].light_color = 'G'; // 2.3

        roads[1].inbound[1].light_color = 'R'; // 1.1
        roads[1].inbound[2].light_color = 'R'; // 1.2
        roads[3].inbound[1].light_color = 'R'; // 3.1
        roads[3].inbound[2].light_color = 'R'; // 3.2
    } else if (phase == 1) { // 1<->3
        roads[0].inbound[1].light_color = 'R'; // 0.1
        roads[0].inbound[2].light_color = 'R'; // 0.2
        roads[2].inbound[2].light_color = 'R'; // 2.2
        roads[2].inbound[3].light_color = 'R'; // 2.3

        roads[1].inbound[1].light_color = 'G'; // 1.1
        roads[1].inbound[2].light_color = 'G'; // 1.2
        roads[3].inbound[1].light_color = 'G'; // 3.1
        roads[3].inbound[2].light_color = 'G'; // 3.2

    }
}

// Funkcja sprawdzająca, czy przejazd jest dozwolony
int is_allowed(float from_lane, float to_lane, Road roads[]) {
    int from_road = (int)from_lane;

    for (int i = 0; i < MAX_LANES; i++) {
        if (roads[from_road].inbound[i].lane_id == from_lane) { // Zabezpieczenie
            for (int j = 0; j < MAX_CONNECTIONS; j++) {
                if (roads[from_road].inbound[i].connections[j] == to_lane)
                    return 1; // dozwolone
            }
        }
    }

    return 0; // niedozwolone
}

// Funkcja dodająca pojazd na linię
void add_vehicle(const char *id, float from_lane, float to_lane, Road roads[]) {
    int from_road = (int)from_lane;

    for (int i = 0; i < MAX_LANES; i++) {
        if (roads[from_road].inbound[i].lane_id == from_lane) {
            roads[from_road].inbound[i].waiting_vehicles++;
            break;
        }
    }

    if (vehicle_count<MAX_VEHICLES){
        strcpy(vehicles[vehicle_count].id, id);
        vehicles[vehicle_count].from_lane = from_lane;
        vehicles[vehicle_count].to_lane = to_lane;
        vehicles[vehicle_count].active = 1;
        vehicle_count++;
    }
}

// Funkcja wypisująca status wszystkich linii
void print_lane_status(Road roads[]){
    printf("=== STATUS PASÓW ===\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < MAX_LANES; j++) {
            if (roads[i].inbound[j].lane_id >= 0)
                printf("Droga %d | Pas %.1f | Światło %c | Czeka: %d\n",
                       roads[i].road_id, roads[i].inbound[j].lane_id,
                       roads[i].inbound[j].light_color,
                       roads[i].inbound[j].waiting_vehicles);
        }
    }
    printf("=====================\n\n");
}

void move_cars(Road roads[], FILE* output) {
    fprintf(output, "    {\n      \"leftVehicles\": [\n");
    int first = 1;


    for(int v =0; v<vehicle_count; v++) {
        if(!vehicles[v].active) continue;

        int from_road = (int)vehicles[v].from_lane;

        for (int i = 0; i<MAX_LANES;i++) {
                if (roads[from_road].inbound[i].lane_id == vehicles[v].from_lane && roads[from_road].inbound[i].light_color == 'G' && roads[from_road].inbound[i].waiting_vehicles) {
                    roads[from_road].inbound[i].waiting_vehicles --;
                    vehicles[v].active = 0;
                    if (!first) fprintf(output, ",\n");
                    fprintf(output, "         \"%s\"", vehicles[v].id);
                    first = 0;
                    break;
                }
        }
    }
    fprintf(output, "\n      ]\n    },\n");

}

void initialize_roads(Road roads[]) {
    for (int i =0; i<4; i++) {
        roads[i].road_id = i;
        for (int j=0; j<MAX_LANES;j++){
            roads[i].inbound[j].lane_id = -1.0;
            roads[i].inbound[j].waiting_vehicles = 0;
            roads[i].outbound[j].lane_id = -1.0;
            roads[i].outbound[j].waiting_vehicles = -1.0;

        }
    }
    
    // Ustawienie dróg dojazdowych i ich możliwych wyjazdów
    roads[0].inbound[2] = (Lane){0, 0.2, 'R', {3.0, 2.0}, 0};
    roads[0].inbound[1] = (Lane){0, 0.1, 'R', {2.1, 1.0}, 0};
    roads[1].inbound[2] = (Lane){1, 1.2, 'R', {3.0, 0.0}, 0};
    roads[1].inbound[1] = (Lane){1, 1.1, 'R', {2.0, 2.1}, 0};
    roads[2].inbound[3] = (Lane){2, 2.3, 'R', {1.0, 0.0}, 0};
    roads[2].inbound[2] = (Lane){2, 2.2, 'R', {3.0, -1}, 0};
    roads[3].inbound[2] = (Lane){3, 3.2, 'R', {2.0, -1}, 0};
    roads[3].inbound[1] = (Lane){3, 3.1, 'R', {1.0, 0.0}, 0};

    // Ustawienie dróg wyjazdowych
    roads[3].outbound[0] = (Lane){3, 3.0, '-', {}, 0};
    roads[2].outbound[0] = (Lane){2, 2.0, '-', {}, 0};
    roads[2].outbound[1] = (Lane){2, 2.1, '-', {}, 0};
    roads[1].outbound[0] = (Lane){1, 1.0, '-', {}, 0};
    roads[0].outbound[0] = (Lane){0, 0.0, '-', {}, 0};
}

// Main ----------------------------

int main() {
    Road roads[4]; // Tablica dróg
    initialize_roads(roads);
    int currentPhase = 1; // pierwsza faza to 0

    FILE *file = fopen("input.txt", "r");
    FILE *output_file = fopen("output.json", "w");
    if (!file  || !output_file) {
        printf("Błąd otwierania plików\n");
        return 1;
    }

    fprintf(output_file, "{\n  \"stepStatuses\": [\n");

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char vehicle_id[32];
        float from_lane, to_lane;


        if (sscanf(line, "ADD %s {%f, %f}", vehicle_id, &from_lane, &to_lane) == 3) {
            int is_vehicle_allowed = is_allowed(from_lane, to_lane, roads);
            printf("Pojazd %s: %s\n", vehicle_id, is_vehicle_allowed ? "PRZEJAZD DOZWOLONY" : "Mandat 10 pkt. karnych!");
            if(is_vehicle_allowed) add_vehicle(vehicle_id, from_lane, to_lane, roads);
        } else if (strncmp(line, "STEP", 4) == 0) {
            currentPhase = (currentPhase+1)%2;
            update_lights(currentPhase, roads); // nowa faza -> ruch 
            printf("Status po zmianie świateł. \n");
            print_lane_status(roads);
            move_cars(roads, output_file);
            printf("Status po przejeździe samochodów. \n");
            print_lane_status(roads);
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
