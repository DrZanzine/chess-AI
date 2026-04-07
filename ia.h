#ifndef IA_H
#define IA_H

#include "pieces.h"
#include "plateau.h"
#include <stdint.h>

#define TAILLE_TT 1000003

typedef struct {
    uint64_t cle_zobrist;
    int profondeur;
    int score;
    int flag;
} EntreeTT;

typedef struct {
    int x1, y1, x2, y2;
    int score_tri;
} Coup;

typedef struct {
    Coup coups[256];
    int nb;
} ListeCoups;


int evaluer_plateau();
int minimax(int profondeur, int alpha, int beta, int est_max);
void jouer_meilleur_coup_IA(int profondeur);

void generer_tous_les_coups(Couleur couleur, ListeCoups *liste);

void initialiser_zobrist();
void vider_table_transposition();

#endif