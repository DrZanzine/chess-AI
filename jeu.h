#ifndef JEU_H
#define JEU_H

#include "pieces.h"
#include "plateau.h"

// --- Fonctions standards ---
void initialiser_partie(EtatPartie *partie);
int executer_tour(EtatPartie *partie);
int est_echec(Couleur c);
int est_mat(EtatPartie *partie);

// --- Fonctions spécifiques au Web (Wasm) ---
// On les déclare ici pour la cohérence, même si Emscripten les trouve via KEEPALIVE
void initialiser_partie_web();
int valider_coup_humain(int x1, int y1, int x2, int y2);
int executer_ia();

#endif