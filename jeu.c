#include "jeu.h"
#include <stdio.h>
#include <string.h>
#include <emscripten.h>

/**
 * Initialise l'état de la structure de jeu et place les pièces sur le plateau.
 */
EMSCRIPTEN_KEEPALIVE
void initialiser_partie(EtatPartie *partie) {
    partie->tour_joueur = BLANC;
    partie->est_fini = 0;
    partie->en_echec = 0;
    partie->can_castle = "KQkq"; // Toutes les possibilités de roque au début
    partie->en_passant = -1; // Pas de prise en passant possible au début
    partie->halmoven_clock = 0;
    partie->fullmove_number = 1;
    initialiserPlateau(); 
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
    if (!est_mouvement_valide(x1, y1, x2, y2, partie->tour_joueur)) {
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
 * vérifie le coup joué par le web (x1,y1) -> (x2,y2) et met à jour l'état de la partie.
 * Contrairement à executer_tour, on suppose que la validation du format de la chaîne
 * renvoie 0 si le coup est invalide, 1 sinon. (ex: "e2 e4" -> x1=6,y1=4,x2=4,y2=4)
 * n'affiche jamais de message d'erreur, c'est à la partie JS de gérer ça.
 */
EMSCRIPTEN_KEEPALIVE
int jouer_coup_web(int x1, int y1, int x2, int y2, char p) {
    // 1. Appliquer le coup de l'humain
    // Note : On utilise ta logique existante tour_web
    // Assure-toi que 'partie_globale' est bien ton pointeur d'état
    int valide = tour_web(&partie_globale, x1, y1, x2, y2);
    
    if (!valide) return 0; // Le JS saura que le coup était illégal

    // 2. Si le coup est valide, c'est au tour de l'IA
    // On appelle ta fonction de recherche (ex: minimax)
    Coup meilleur_coup = calculer_meilleur_coup(&partie_globale);
    
    // 3. Appliquer le coup de l'IA sur le plateau
    appliquer_coup(&partie_globale, meilleur_coup);
    
    // 4. On change le tour pour revenir à l'humain
    partie_globale.tour_joueur = BLANC; 
    
    return 1;
}

/**
 * Vérifie si le Roi de la couleur 'c' est menacé par une pièce adverse.
 */
EMSCRIPTEN_KEEPALIVE
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
                if (est_mouvement_valide(x, y, roi_x, roi_y, c)) {
                    return 1; 
                }
            }
        }
    }
    return 0;
}

int est_mat(EtatPartie *partie) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].couleur == partie->tour_joueur) {
                for (int x2 = 0; x2 < 8; x2++) {
                    for (int y2 = 0; y2 < 8; y2++) {
                        if (est_mouvement_valide(i, j, x2, y2, partie->tour_joueur)) {
                            // Simuler le coup
                            Case piece_depart = plateau[i][j];
                            Case piece_dest_svg = plateau[x2][y2];
                            plateau[x2][y2] = piece_depart;
                            plateau[i][j].type = VIDE;
                            plateau[i][j].couleur = AUCUNE;

                            int en_echec = est_echec(partie->tour_joueur);

                            // Annuler la simulation
                            plateau[i][j] = piece_depart;
                            plateau[x2][y2] = piece_dest_svg;

                            if (!en_echec) return 0; // Il existe au moins un coup légal pour sortir de l'échec
                        }
                    }
                }
            }
        }
    }
    if (est_echec(partie->tour_joueur)) return 1; // MAT
    else return 2; // PAT}
}