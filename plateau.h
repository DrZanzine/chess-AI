#ifndef PLATEAU_H
#define PLATEAU_H

#include "pieces.h" // Pour Case et Couleur

typedef struct {
    Couleur tour_joueur;
    int est_fini;
    int en_echec;
    char can_castle[5];  // Ex: "KQkq"
    char en_passant[3];  // Ex: "e3" ou "-"
    int halmoven_clock; 
    int fullmove_number; 
} EtatPartie;

// Déclaration du plateau global (défini physiquement dans plateau.c)
extern Case plateau[8][8];

/* --- Fonctions de base et Console --- */
void initialiserPlateau();
void afficherPlateau();
void copier_plateau(Case source[8][8], Case destination[8][8]);
void appliquer_mouvement_direct(int x1, int y1, int x2, int y2);

/* --- Fonctions d'interface pour le JavaScript (Wasm) --- */
const char* renvoyer_FEN(EtatPartie *partie);
void jouer_coup_IA(int x1, int y1, int x2, int y2);

#endif