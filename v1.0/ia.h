#ifndef IA_H
#define IA_H

#include "pieces.h"
#include "plateau.h"

/**
 * Evalue numériquement l'état du plateau.
 * @return Un score entier (positif = avantage IA, négatif = avantage Humain).
 */
int evaluer_plateau();

/**
 * Algorithme de recherche Minimax avec élagage Alpha-Bêta.
 * @param profondeur Nombre de coups d'avance à calculer.
 * @param alpha Valeur plancher pour l'élagage.
 * @param beta Valeur plafond pour l'élagage.
 * @param est_max 1 si c'est au tour de l'IA (maximisation), 0 sinon.
 * @return Le meilleur score trouvé pour cette branche.
 */
int minimax(int profondeur, int alpha, int beta, int est_max);

/**
 * Fonction principale à appeler dans le main.c.
 * Analyse toutes les possibilités et joue physiquement le meilleur coup sur le plateau.
 */
void jouer_meilleur_coup_IA(int profondeur);

#endif