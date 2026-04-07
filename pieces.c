#include "pieces.h"
#include "plateau.h"
#include <stdlib.h>
#include <emscripten.h>


// Fonction "Radar" : Vérifie si une case (x, y) est attaquée par une couleur
int est_case_attaquee(int x, int y, Couleur attaquant) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].couleur == attaquant) {
                // Pour le Roi adverse, on check la distance (évite une boucle infinie de roque)
                if (plateau[i][j].type == ROI) {
                    if (abs(x - i) <= 1 && abs(y - j) <= 1) return 1;
                } else if (est_mouvement_valide(i, j, x, y, attaquant)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

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
    // Captures (Diagonales)
    if (abs(y2 - y1) == 1 && x2 == x1 + direction) {
        // Prise classique
        if (plateau[x2][y2].type != VIDE && plateau[x2][y2].couleur != couleur) {
            return 1;
        }
        // NOUVEAU : Prise en passant
        if (x2 == ep_x && y2 == ep_y) {
            return 1;
        }
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
    // 1. Déplacement normal (1 case)
    if (abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1) return 1; 

    // 2. LE ROQUE (Déplacement de 2 cases horizontalement)
    Couleur c = plateau[x1][y1].couleur;
    Couleur adv = (c == BLANC) ? NOIR : BLANC;

    if (x1 == x2 && abs(y2 - y1) == 2) {
        // Le roi doit être sur sa ligne de départ
        if ((c == BLANC && x1 != 7) || (c == NOIR && x1 != 0)) return 0;
        
        // Règle 1 : Le Roi ne doit pas être en échec actuellement
        if (est_case_attaquee(x1, y1, adv)) return 0;

        // Petit Roque (vers la colonne 'g' / y2 == 6)
        if (y2 == 6) {
            if ((c == BLANC && !roque_K) || (c == NOIR && !roque_k)) return 0;
            // Vérifier que les cases f1/g1 (ou f8/g8) sont vides
            if (plateau[x1][5].type != VIDE || plateau[x1][6].type != VIDE) return 0;
            // Règle 2 : Le Roi ne doit pas traverser ni arriver sur une case attaquée
            if (est_case_attaquee(x1, 5, adv) || est_case_attaquee(x1, 6, adv)) return 0;
            return 1;
        }
        
        // Grand Roque (vers la colonne 'c' / y2 == 2)
        if (y2 == 2) {
            if ((c == BLANC && !roque_Q) || (c == NOIR && !roque_q)) return 0;
            // Vérifier que les cases b1/c1/d1 sont vides
            if (plateau[x1][1].type != VIDE || plateau[x1][2].type != VIDE || plateau[x1][3].type != VIDE) return 0;
            // Règle 2 : Le Roi ne doit pas traverser ni arriver sur une case attaquée
            if (est_case_attaquee(x1, 3, adv) || est_case_attaquee(x1, 2, adv)) return 0;
            return 1;
        }
    }
    return 0;
}

// LA FONCTION MAÎTRESSE
EMSCRIPTEN_KEEPALIVE
int est_mouvement_valide(int x1, int y1, int x2, int y2, Couleur joueur_actuel) {
    /*
    est_mouvement_valide vérifie si le déplacement de (x1,y1) à (x2,y2) est légal pour le joueur actuel.
     - Vérifie que la pièce déplacée appartient au joueur
     - Vérifie que la destination n'est pas occupée par une pièce alliée
     - Délègue aux fonctions spécifiques selon le type de pièce pour vérifier les règles de déplacement
    */
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