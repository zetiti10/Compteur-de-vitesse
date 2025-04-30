# Ce petit programme Python permet de générer les variables nécessaire au programme Arduino pour estimer le pourcentage de batterie restante à partir de la tension mesurée. Le tout se base sur une décharge complète de la batterie en mesurant la tension à intervales réguliers.

# Ajout des bibliothèques au programme.
import numpy as np


def read_measures_file(fichier):
    """Lit le fichier de mesures et retourne une liste de tensions."""
    voltages = []
    with open(fichier, 'r') as f:
        for ligne in f:
            if ',' in ligne:
                _, voltage = ligne.strip().split(',')
                voltages.append(float(voltage))
    return voltages


def generate_arrays(voltages, num_values):
    """Génère les tableaux de niveaux de tension et de pourcentages de charge."""
    voltages_uniques = sorted(set(voltages))
    voltages_selectionnees = np.linspace(
        voltages_uniques[0], voltages_uniques[-1], num_values)
    percentages = np.linspace(0, 1, num_values)
    return voltages_selectionnees, percentages


def create_cpp_code(voltages, percentages):
    """Génère le code C++ pour Arduino."""
    code = "// Valeurs permettant d'estimer le pourcentage de batterie restante à partir de la tension mesurée.\n"
    code += "const float voltageLevels[] = {\n    " + \
        ", ".join(f"{v:.2f}f" for v in voltages) + "\n};\n"
    code += "const float chargePercentages[] = {\n    " + \
        ", ".join(f"{p:.2f}f" for p in percentages) + "\n};\n"
    code += f"const int numberOfVoltageValues = {len(voltages)};\n"
    return code


def main():
    # Fichier contenant les mesures de tension.
    fichier_mesures = "measures.txt"
    # Nombre de valeurs à intégrer dans le tableau.
    num_values = 20

    # Lit le fichier de mesures et génère les tableaux de tensions et de pourcentages.
    voltages = read_measures_file(fichier_mesures)
    voltages_selectionnees, percentages = generate_arrays(
        voltages, num_values)
    code_cpp = create_cpp_code(voltages_selectionnees, percentages)

    # Écrit le code généré dans un fichier.
    with open("output.cpp", "w") as f:
        f.write(code_cpp)

    print("Code C++ généré avec succès dans 'output.cpp'.")


if __name__ == "__main__":
    main()
