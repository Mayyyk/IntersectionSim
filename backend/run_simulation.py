import sys
import subprocess
import os
import platform

print("=== DEBUG START ===")
print("CWD:", os.getcwd())
print("FILES:", os.listdir("."))
print("ARGS:", sys.argv)
print("=== DEBUG END ===")


if len(sys.argv) < 3:
    print("Użycie: python run_simulation.py commands.json output.json [--deterministic]")
    sys.exit(1)

input_json = sys.argv[1]
output_json = sys.argv[2]
deterministic = "--deterministic" in sys.argv

python_cmd = "python3" if platform.system() != "Windows" else "python"

# 1. Wygeneruj input.txt
json_input_cmd = [python_cmd, "json_to_input.py", input_json]
if deterministic:
    json_input_cmd.append("--deterministic") # Tryb testów

subprocess.run(json_input_cmd, check=True)

executable = "./main" if platform.system() != "Windows" else "main.exe"
subprocess.run([executable], check=True)


print(f"Platforma: {platform.system()}")
print(f"Używam: {executable}")
print(f"Plik main istnieje? {os.path.exists(executable)}")

# 3. Napraw output.json → fixed_output.json
subprocess.run([python_cmd, "fix_output.py"], check=True)

# 4. Przenieś poprawiony plik do finalnej nazwy
os.replace("fixed_output.json", output_json)

print(f"Gotowe! Wynik zapisano jako: {output_json}")
