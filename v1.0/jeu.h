#ifndef JEU_H
#define JEU_H

#include "pieces.h"    // <--- Crucial pour Couleur
#include "plateau.h"

typedef struct {
    Couleur tour_joueur;
    int est_fini;
    int en_echec;
} EtatPartie;

void initialiser_partie(EtatPartie *partie);
int executer_tour(EtatPartie *partie);
int est_echec(Couleur c);

#endif