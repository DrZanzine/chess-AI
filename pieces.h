#ifndef PIECE_H
#define PIECE_H

typedef enum { VIDE, PION, CAVALIER, FOU, TOUR, DAME, ROI } TypePiece;
typedef enum { BLANC, NOIR, AUCUNE } Couleur;

typedef struct {
    TypePiece type;
    Couleur couleur;
} Case;

// Fonction principale appelée par le moteur de jeu
int est_mouvement_valide(int x1, int y1, int x2, int y2, Couleur joueur_actuel);

// Prototypes spécifiques (utiles pour les tests ou les règles complexes)
int est_mouvement_cavalier_valide(int x_depart, int y_depart, int x_arrivee, int y_arrivee);
int est_mouvement_fou_valide(int x_depart, int y_depart, int x_arrivee, int y_arrivee);
int est_mouvement_tour_valide(int x_depart, int y_depart, int x_arrivee, int y_arrivee);
int est_mouvement_dame_valide(int x_depart, int y_depart, int x_arrivee, int y_arrivee);
int est_mouvement_roi_valide(int x_depart, int y_depart, int x_arrivee, int y_arrivee);
int est_mouvement_pion_valide(int x_depart, int y_depart, int x_arrivee, int y_arrivee, Couleur couleur);

#endif