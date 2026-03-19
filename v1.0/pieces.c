#include "pieces.h"
#include "plateau.h"
#include <stdlib.h>

// Fonction utilitaire pour vérifier si le chemin est libre (Tour, Fou, Dame)
// Ne vérifie pas la case d'arrivée (qui peut être une capture)
int chemin_libre(int x1, int y1, int x2, int y2) {
    int dx = (x2 > x1) ? 1 : (x2 < x1 ? -1 : 0);
    int dy = (y2 > y1) ? 1 : (y2 < y1 ? -1 : 0);
    
    int x = x1 + dx;
    int y = y1 + dy;
    
    while (x != x2 || y != y2) {
        if (plateau[x][y].type != VIDE) return 0;
        x += dx;
        y += dy;
    }
    return 1;
}

int est_mouvement_pion_valide(int x1, int y1, int x2, int y2, Couleur couleur) {
    int direction = (couleur == BLANC) ? -1 : 1;
    int ligne_depart = (couleur == BLANC) ? 6 : 1;

    // Avancée d'une case
    if (y1 == y2 && x2 == x1 + direction && plateau[x2][y2].type == VIDE) {
        return 1;
    }
    // Premier double pas
    if (y1 == y2 && x1 == ligne_depart && x2 == x1 + 2 * direction 
        && plateau[x1 + direction][y2].type == VIDE && plateau[x2][y2].type == VIDE) {
        return 1;
    }
    // Capture en diagonale
    if (abs(y2 - y1) == 1 && x2 == x1 + direction 
        && plateau[x2][y2].type != VIDE && plateau[x2][y2].couleur != couleur) {
        return 1;
    }
    return 0;
}

int est_mouvement_cavalier_valide(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
}

int est_mouvement_fou_valide(int x1, int y1, int x2, int y2) {
    if (abs(x2 - x1) != abs(y2 - y1)) return 0;
    return chemin_libre(x1, y1, x2, y2);
}

int est_mouvement_tour_valide(int x1, int y1, int x2, int y2) {
    if (x1 != x2 && y1 != y2) return 0;
    return chemin_libre(x1, y1, x2, y2);
}

int est_mouvement_dame_valide(int x1, int y1, int x2, int y2) {
    if (!((x1 == x2 || y1 == y2) || (abs(x2 - x1) == abs(y2 - y1)))) return 0;
    return chemin_libre(x1, y1, x2, y2);
}

int est_mouvement_roi_valide(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1;
}

// LA FONCTION MAÎTRESSE
int est_mouvement_valide(int x1, int y1, int x2, int y2, Couleur joueur_actuel) {
    Case p = plateau[x1][y1];

    // 1. Vérifier si on déplace bien une pièce à nous
    if (p.type == VIDE || p.couleur != joueur_actuel) return 0;

    // 2. Vérifier qu'on ne mange pas sa propre pièce
    if (plateau[x2][y2].type != VIDE && plateau[x2][y2].couleur == joueur_actuel) return 0;

    // 3. Vérifier les règles spécifiques
    switch (p.type) {
        case PION:     return est_mouvement_pion_valide(x1, y1, x2, y2, p.couleur);
        case CAVALIER: return est_mouvement_cavalier_valide(x1, y1, x2, y2);
        case FOU:      return est_mouvement_fou_valide(x1, y1, x2, y2);
        case TOUR:     return est_mouvement_tour_valide(x1, y1, x2, y2);
        case DAME:     return est_mouvement_dame_valide(x1, y1, x2, y2);
        case ROI:      return est_mouvement_roi_valide(x1, y1, x2, y2);
        default:       return 0;
    }
}