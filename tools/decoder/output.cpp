// Valeurs permettant d'estimer le pourcentage de batterie restante à partir de la tension mesurée.
const float voltageLevels[] = {
    3.20f, 3.24f, 3.28f, 3.33f, 3.37f, 3.41f, 3.45f, 3.49f, 3.54f, 3.58f, 3.62f, 3.66f, 3.71f, 3.75f, 3.79f, 3.83f, 3.87f, 3.92f, 3.96f, 4.00f};
const float chargePercentages[] = {
    0.00f, 0.05f, 0.11f, 0.16f, 0.21f, 0.26f, 0.32f, 0.37f, 0.42f, 0.47f, 0.53f, 0.58f, 0.63f, 0.68f, 0.74f, 0.79f, 0.84f, 0.89f, 0.95f, 1.00f};
const int numberOfVoltageValues = 20;
