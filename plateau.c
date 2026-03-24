#include "plateau.h"
#include <stdio.h>
#include <string.h>

// Définition physique du plateau global
Case plateau[8][8]; 

/**
 * Initialise le plateau avec les pièces à leurs positions standards.
 */
EMSCRIPTEN_KEEPALIVE
void initialiserPlateau() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == 1) {
                plateau[i][j].type = PION; plateau[i][j].couleur = NOIR; 
            } else if (i == 6) {
                plateau[i][j].type = PION; plateau[i][j].couleur = BLANC; 
            } else if (i == 0 || i == 7) {
                if (j == 0 || j == 7) plateau[i][j].type = TOUR;
                else if (j == 1 || j == 6) plateau[i][j].type = CAVALIER;
                else if (j == 2 || j == 5) plateau[i][j].type = FOU;
                else if (j == 3) plateau[i][j].type = DAME;
                else if (j == 4) plateau[i][j].type = ROI;
                plateau[i][j].couleur = (i == 0) ? NOIR : BLANC; 
            } else {
                plateau[i][j].type = VIDE; plateau[i][j].couleur = AUCUNE; 
            }
        }
    }
}

/**
 * Affiche le plateau avec les coordonnées e4 d5 pour l'utilisateur.
 */
void afficherPlateau() {
    printf("\n    a b c d e f g h\n");
    printf("  +-----------------+\n");
    for (int i = 0; i < 8; i++) {
        printf("%d | ", 8 - i); 
        for (int j = 0; j < 8; j++) {
            char c = '.';
            if (plateau[i][j].type == PION) c = (plateau[i][j].couleur == BLANC) ? 'P' : 'p';
            else if (plateau[i][j].type == TOUR) c = (plateau[i][j].couleur == BLANC) ? 'T' : 't';
            else if (plateau[i][j].type == CAVALIER) c = (plateau[i][j].couleur == BLANC) ? 'C' : 'c';
            else if (plateau[i][j].type == FOU) c = (plateau[i][j].couleur == BLANC) ? 'F' : 'f';
            else if (plateau[i][j].type == DAME) c = (plateau[i][j].couleur == BLANC) ? 'D' : 'd';
            else if (plateau[i][j].type == ROI) c = (plateau[i][j].couleur == BLANC) ? 'R' : 'r'; 
            printf("%c ", c);
        }
        printf("| %d\n", 8 - i); 
    }
    printf("  +-----------------+\n");
    printf("    a b c d e f g h\n\n");
}

/**
 * OUTILS POUR L'IA : Copier un état de plateau pour simulation
 */
EMSCRIPTEN_KEEPALIVE
void copier_plateau(Case source[8][8], Case destination[8][8]) {
    memcpy(destination, source, sizeof(Case) * 64);
}

/**
 * OUTILS POUR L'IA : Jouer un coup sans vérification ni affichage
 */
EMSCRIPTEN_KEEPALIVE
void jouer_coup_IA(int x1, int y1, int x2, int y2) {
    plateau[x2][y2] = plateau[x1][y1];
    plateau[x1][y1].type = VIDE;
    plateau[x1][y1].couleur = AUCUNE;
}

EMSCRIPTEN_KEEPALIVE
// Dans plateau.c
EMSCRIPTEN_KEEPALIVE
const char* renvoyer_FEN() {
    static char fen[128]; 
    int pos = 0;
    for (int i = 0; i < 8; i++) {
        int vides = 0;
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].type == VIDE) {
                vides++;
            } else {
                if (vides > 0) fen[pos++] = vides + '0';
                vides = 0;
                char c = (plateau[i][j].type == ROI) ? 'k' : 
                         (plateau[i][j].type == DAME) ? 'q' :
                         (plateau[i][j].type == PION) ? 'p' : 
                         (plateau[i][j].type == TOUR) ? 'r' :
                         (plateau[i][j].type == CAVALIER) ? 'n' : 
                         (plateau[i][j].type == FOU) ? 'b';
                if (plateau[i][j].couleur == BLANC) c -= 32; // Majuscule
                fen[pos++] = c;
            }
        }
        if (vides > 0) fen[pos++] = vides + '0';
        if (i < 7) fen[pos++] = '/';
    }
    if (partie->tour_joueur == BLANC) fen[pos++] = 'w';
    else fen[++pos++] = 'b';

    fen[pos++] = partie->can_castle;
    fen[pos] = '\0';
    return fen;
}