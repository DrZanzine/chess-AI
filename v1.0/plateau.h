#ifndef PLATEAU_H  // Doit correspondre au nom du fichier
#define PLATEAU_H

#include "pieces.h" // Pour connaître le type 'Case' et 'Couleur'

// On déclare que le tableau existe ailleurs (dans plateau.c)
extern Case plateau[8][8];

// Les prototypes des fonctions
void initialiserPlateau();
void afficherPlateau();
void copier_plateau(Case source[8][8], Case destination[8][8]);
void jouer_coup_IA(int x1, int y1, int x2, int y2);

#endif