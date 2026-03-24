#ifndef JEU_H
#define JEU_H

#include "pieces.h"    // <--- Crucial pour Couleur
#include "plateau.h"

typedef struct {
    Couleur tour_joueur;
    int est_fini;
    int en_echec;
    char can_castle[5]; // Ex: "KQkq" pour les droits de roque
    char en_passant[3]; // Ex: "e3" ou "-"
    int halmoven_clock; // Nombre de demi-coups sans mouvement de pion ni capture
    int fullmove_number; // Incrémenté après le tour des Noirs
} EtatPartie;

void initialiser_partie(EtatPartie *partie);
int executer_tour(EtatPartie *partie);
int est_echec(Couleur c);

#endif