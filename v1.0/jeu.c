#include "jeu.h"
#include <stdio.h>
#include <string.h>

/**
 * Initialise l'état de la structure de jeu et place les pièces sur le plateau.
 */
void initialiser_partie(EtatPartie *partie) {
    partie->tour_joueur = BLANC;
    partie->est_fini = 0;
    partie->en_echec = 0;
    initialiserPlateau(); 
}

/**
 * Centralise la vérification des règles de mouvement selon le type de pièce.
 */
int est_coup_physiquement_valide(int x1, int y1, int x2, int y2) {
    Case p = plateau[x1][y1];
    if (p.type == VIDE) return 0;

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

/**
 * Gère le tour complet : saisie algébrique (e2 e4), validation et mise à jour.
 */
int executer_tour(EtatPartie *partie) {
    char dep[10], arr[10];
    int x1, y1, x2, y2;

    printf("Tour des %s (ex: e2 e4) : ", (partie->tour_joueur == BLANC) ? "BLANCS" : "NOIRS");
    
    // Lecture des chaînes (ex: "e2" et "e4")
    if (scanf("%s %s", dep, arr) != 2) return 1;

    // CONVERSION ALGEBRIQUE -> INDICES TABLEAU
    // Colonnes : 'a'-'h' -> 0-7
    y1 = dep[0] - 'a';
    y2 = arr[0] - 'a';
    // Lignes : '1'-'8' -> 7-0 (Inversion pour l'indexation du tableau)
    x1 = 8 - (dep[1] - '0');
    x2 = 8 - (arr[1] - '0');

    // 1. Vérification des limites et de la couleur de la pièce
    if (x1 < 0 || x1 > 7 || y1 < 0 || y1 > 7 || x2 < 0 || x2 > 7 || y2 < 0 || y2 > 7) {
        printf("Position hors du plateau !\n");
        return 1;
    }
    if (plateau[x1][y1].couleur != partie->tour_joueur) {
        printf("Ce n'est pas votre pièce !\n");
        return 1;
    }

    // 2. Validation du mouvement (géométrie et obstacles)
    if (!est_coup_physiquement_valide(x1, y1, x2, y2)) {
        printf("Mouvement invalide pour cette pièce.\n");
        return 1;
    }

    // 3. SIMULATION : On ne peut pas finir son tour en étant en échec
    Case piece_depart = plateau[x1][y1];
    Case piece_dest_svg = plateau[x2][y2];

    // Déplacement temporaire
    plateau[x2][y2] = piece_depart;
    plateau[x1][y1].type = VIDE;
    plateau[x1][y1].couleur = AUCUNE;

    if (est_echec(partie->tour_joueur)) {
        // Annulation si le mouvement met le joueur en échec
        plateau[x1][y1] = piece_depart;
        plateau[x2][y2] = piece_dest_svg;
        printf("Coup illégal : votre Roi est (ou reste) en échec !\n");
        return 1;
    }

    // 4. FINALISATION DU TOUR
    Couleur adversaire = (partie->tour_joueur == BLANC) ? NOIR : BLANC;
    if (est_echec(adversaire)) {
        printf("ECHEC AU ROI ADVERSE !\n");
    }

    partie->tour_joueur = adversaire;
    return 0; 
}

/**
 * Vérifie si le Roi de la couleur 'c' est menacé par une pièce adverse.
 */
int est_echec(Couleur c) {
    int roi_x = -1, roi_y = -1;

    // Localisation du Roi ciblé
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].type == ROI && plateau[i][j].couleur == c) {
                roi_x = i; roi_y = j;
                break;
            }
        }
    }

    // On parcourt le plateau pour voir si une pièce ennemie peut prendre le roi
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (plateau[x][y].couleur != c && plateau[x][y].couleur != AUCUNE) {
                if (est_coup_physiquement_valide(x, y, roi_x, roi_y)) {
                    return 1; 
                }
            }
        }
    }
    return 0;
}