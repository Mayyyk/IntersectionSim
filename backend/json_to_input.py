import json, random, sys

if len(sys.argv) < 2:
    print("Użycie: python json_to_input.py commands.json [--deterministic]")
    exit(1)

commands_file = sys.argv[1]
deterministic = "--deterministic" in sys.argv


# Mapowanie nazw dróg na ID
road_name_to_id = {
    "stachiewicza": 0,
    "wybickiego": 1,
    "wroclawska": 2,
    "radzikowskiego": 3
}

# Domyślne linie dojazdowe i wyjazdowe dla uproszczenia
inbound_lanes = {
    0: [0.1, 0.2],
    1: [1.1, 1.2],
    2: [2.2, 2.3],
    3: [3.1, 3.2]
}

outbound_lanes = {
    0: [0.0],
    1: [1.0],
    2: [2.0, 2.1],
    3: [3.0]
}

# Pas domyślny dla trybu deterministycznego do testów, które ciężko robić w trybie losowym
fixed_from_lanes = {
    0: 0.1,
    1: 1.1,
    2: 2.2,
    3: 3.1
}

fixed_to_lanes = {
    0: 0.0,
    1: 1.0,
    2: 2.1,
    3: 3.0
}

# Wczytanie commands.json
with open(commands_file, "r") as file:
    data = json.load(file)

lines = []
for cmd in data["commands"]:
    if cmd["type"] == "addVehicle":
        vehicle_id = cmd["vehicleId"]
        start = road_name_to_id[cmd["startRoad"].lower()]
        end = road_name_to_id[cmd["endRoad"].lower()]



        if deterministic:
            from_lane = fixed_from_lanes[start]
            to_lane = fixed_to_lanes[end]
        else: # Losowe dobranie pasa dojazdowego i wyjazdowego w celu urozmaicenia i możliwych mandatów
            from_lane = random.choice(inbound_lanes[start])
            to_lane = random.choice(outbound_lanes[end])

        lines.append(f"ADD {vehicle_id} {{{from_lane}, {to_lane}}}")

    elif cmd["type"] == "step":
        lines.append("STEP")
        
with open("input.txt", "w") as f:
    f.write("\n".join(lines))

print("Wygenerowano input.txt na podstawie commands.json")
