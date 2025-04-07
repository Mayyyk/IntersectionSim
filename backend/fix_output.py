import json
import re

with open("output.json", "r") as f:
    content = f.read()

content = re.sub(r",\s*\n\s*]", "\n  ]", content)

if not content.strip().endswith("}"):
    content += "\n}"

try:
    parsed = json.loads(content)
    print("Plik JSON naprawiony i poprawny.")
except json.JSONDecodeError as e:
    print("Błąd składni JSON:", e)

# Zapisz jako finalny output
with open("fixed_output.json", "w") as f:
    f.write(content)