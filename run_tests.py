import os
import subprocess
import json
import difflib
import sys

def run_test(test_dir):
    print(f"\n🔍 Running test: {test_dir}")

    # Uruchom symulację z input/output
    subprocess.run([
        "python", "run_simulation.py",
        os.path.join(test_dir, "commands.json"),
        os.path.join(test_dir, "result.json"),
        "--deterministic"
    ], check=True)

    # Wczytaj wynik i oczekiwany output
    with open(os.path.join(test_dir, "result.json")) as f:
        actual = json.load(f)
    with open(os.path.join(test_dir, "expected_output.json")) as f:
        expected = json.load(f)

    # Porównaj
    if actual == expected:
        print("✅ PASSED")
        return True
    else:
        print("❌ FAILED (data mismatch)")
        print("🔍 JSON diff:")
        expected_str = json.dumps(expected, indent=2, sort_keys=True)
        actual_str = json.dumps(actual, indent=2, sort_keys=True)
        diff = difflib.unified_diff(
            expected_str.splitlines(),
            actual_str.splitlines(),
            fromfile="expected_output.json",
            tofile="result.json",
            lineterm=""
        )
        print("\n".join(diff))
        return False


# Przejdź przez wszystkie testy
def main():
    all_passed = True
    for test_name in sorted(os.listdir("tests")):
        path = os.path.join("tests", test_name)
        if os.path.isdir(path):
            passed = run_test(path)
            all_passed = all_passed and passed

    if all_passed:
        print("\n✅✅✅ ALL TESTS PASSED ✅✅✅")
        sys.exit(0)
    else:
        print("\n❌❌ Some tests failed ❌❌")
        sys.exit(1)

if __name__ == "__main__":
    main()
