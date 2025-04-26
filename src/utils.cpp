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

/// @brief Méthode permettant de mapper une valeur flottante d'un intervalle à un autre.
/// @param x La valeur à mapper.
/// @param inMin La valeur minimale de l'intervalle d'entrée.
/// @param inMax La valeur maximale de l'intervalle d'entrée.
/// @param outMin La valeur minimale de l'intervalle de sortie.
/// @param outMax La valeur maximale de l'intervalle de sortie.
/// @return La valeur mappée.
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax)
{
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}