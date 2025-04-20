/**
 * @file display.cpp
 * @author Louis L
 * @brief Fichier contenant diverses fonctions utiles au programme.
 * @version 1.0
 * @date 2025-04-07
 */

// Ajout des bibilothèques au programme.
#include <Arduino.h>

// Autres fichiers du programme
#include "utils.hpp"

/// @brief Méthode permettant de convertir un entier en une chaîne de caractère à longueur fixe, complétée de `0` si nécessaire.
/// @param number L'entier à convertir.
/// @param length La longueur de la chaîne de caractères désirée.
/// @return L'entier formaté.
String addZeros(int number, int length)
{
    String result = String(number);
    while (result.length() < (unsigned int)length)
        result = "0" + result;

    return result;
}