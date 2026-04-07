#include "jeu.h"
#include "ia.h"
#include "ia.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <emscripten.h>

// Instance globale pour maintenir l'état entre les appels JavaScript
EtatPartie partie_globale;

/**
 * Initialise une structure EtatPartie et le plateau de jeu.
 */
EMSCRIPTEN_KEEPALIVE
void initialiser_partie(EtatPartie *partie) {
    partie->tour_joueur = BLANC;
    partie->est_fini = 0;
    partie->en_echec = 0;
    strcpy(partie->can_castle, "KQkq");
    strcpy(partie->en_passant, "-");
    partie->halmoven_clock = 0;
    partie->fullmove_number = 1;
    initialiserPlateau();
    initialiser_zobrist();
    vider_table_transposition();
}

/**
 * Point d'entrée pour le Web : initialise l'instance globale.
 */
EMSCRIPTEN_KEEPALIVE
void initialiser_partie_web() {
    initialiser_partie(&partie_globale);
}

/**
 * Vérifie si le Roi de la couleur 'c' est attaqué.
 */
EMSCRIPTEN_KEEPALIVE
int est_echec(Couleur c) {
    int roi_x = -1, roi_y = -1;

    // 1. Localiser le Roi
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].type == ROI && plateau[i][j].couleur == c) {
                roi_x = i; roi_y = j;
                break;
            }
        }
        if (roi_x != -1) break;
    }

    // 2. Vérifier si une pièce adverse peut atteindre cette case
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (plateau[x][y].couleur != c && plateau[x][y].couleur != AUCUNE) {
                // On utilise la couleur de l'attaquant pour la validation
                if (est_mouvement_valide(x, y, roi_x, roi_y, plateau[x][y].couleur)) {
                    return 1; 
                }
            }
        }
    }
    return 0;
}

/**
 * Détermine si le joueur au trait est Mat (1), Pat (2) ou peut encore jouer (0).
 */
EMSCRIPTEN_KEEPALIVE
int est_mat(EtatPartie *partie) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].couleur == partie->tour_joueur) {
                for (int x2 = 0; x2 < 8; x2++) {
                    for (int y2 = 0; y2 < 8; y2++) {
                        if (est_mouvement_valide(i, j, x2, y2, partie->tour_joueur)) {
                            // Simulation du coup
                            Case piece_depart = plateau[i][j];
                            Case piece_dest_svg = plateau[x2][y2];
                            
                            appliquer_mouvement_direct(i, j, x2, y2);

                            int toujours_echec = est_echec(partie->tour_joueur);

                            // Annulation
                            plateau[i][j] = piece_depart;
                            plateau[x2][y2] = piece_dest_svg;

                            if (!toujours_echec) return 0; // Un coup légal existe
                        }
                    }
                }
            }
        }
    }
    return est_echec(partie->tour_joueur) ? 1 : 2; 
}

/**
 * Fonction appelée par le JavaScript lors d'un déplacement sur l'interface.
 */
// ÉTAPE 1 : Valider et appliquer uniquement le coup humain
EMSCRIPTEN_KEEPALIVE
int valider_coup_humain(int x1, int y1, int x2, int y2) {
    if (plateau[x1][y1].couleur != partie_globale.tour_joueur) return 0;
    if (!est_mouvement_valide(x1, y1, x2, y2, partie_globale.tour_joueur)) return 0;

    // Simulation d'échec
    Case piece_depart = plateau[x1][y1];
    Case piece_dest_svg = plateau[x2][y2];

    appliquer_mouvement_direct(x1, y1, x2, y2);

    if (est_echec(partie_globale.tour_joueur)) {
        plateau[x1][y1] = piece_depart;
        plateau[x2][y2] = piece_dest_svg;
        return 0;
    }

    partie_globale.tour_joueur = (partie_globale.tour_joueur == BLANC) ? NOIR : BLANC;
    return 1; 
}

// ÉTAPE 2 : Lancer le calcul de l'IA séparément
EMSCRIPTEN_KEEPALIVE
int executer_ia() {
    if (est_mat(&partie_globale) == 0) {
        jouer_meilleur_coup_IA(5); 
        partie_globale.tour_joueur = BLANC;
        return est_mat(&partie_globale); // Renvoie l'état après le coup IA (0, 1=Mat, 2=Pat)
    }
    return 0;
}
/**
 * Boucle de tour pour le mode console.
 */
int executer_tour(EtatPartie *partie) {
    char dep[10], arr[10];
    int x1, y1, x2, y2;

    printf("Tour des %s (ex: e2 e4) : ", (partie->tour_joueur == BLANC) ? "BLANCS" : "NOIRS");
    if (scanf("%s %s", dep, arr) != 2) return 1;

    y1 = dep[0] - 'a';
    y2 = arr[0] - 'a';
    x1 = 8 - (dep[1] - '0');
    x2 = 8 - (arr[1] - '0');

    if (x1 < 0 || x1 > 7 || y1 < 0 || y1 > 7 || x2 < 0 || x2 > 7 || y2 < 0 || y2 > 7) return 1;
    if (plateau[x1][y1].couleur != partie->tour_joueur) return 1;

    if (!est_mouvement_valide(x1, y1, x2, y2, partie->tour_joueur)) return 1;

    // Simulation et validation finale
    Case piece_depart = plateau[x1][y1];
    Case piece_dest_svg = plateau[x2][y2];
    
    appliquer_mouvement_direct(x1, y1, x2, y2);

    if (est_echec(partie->tour_joueur)) {
        plateau[x1][y1] = piece_depart;
        plateau[x2][y2] = piece_dest_svg;
        return 1;
    }

    partie->tour_joueur = (partie->tour_joueur == BLANC) ? NOIR : BLANC;
    return 0; 
}

// --- PASSERELLE POUR LA BARRE D'AVANTAGE WEB ---
EMSCRIPTEN_KEEPALIVE
int evaluer_plateau_web() {
    return evaluer_plateau(); 
}