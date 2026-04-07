#include "jeu.h"     // Doit être inclus en premier pour la définition d'EtatPartie
#include "plateau.h"
#include "pieces.h"
#include <stdio.h>
#include <string.h>
#include <emscripten.h>

// Définition physique du plateau global (8x8 cases)
Case plateau[8][8]; 

int ep_x = -1;
int ep_y = -1;
int roque_K = 1, roque_Q = 1, roque_k = 1, roque_q = 1;

/* ==========================================================================
   1. FONCTIONS DE BASE (Logique Interne & Console)
   ========================================================================== */

/**
 * Initialise le plateau avec les pièces à leurs positions standards.
 */
void initialiserPlateau() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == 1) {
                plateau[i][j].type = PION; plateau[i][j].couleur = NOIR; 
            } else if (i == 6) {
                plateau[i][j].type = PION; plateau[i][j].couleur = BLANC; 
            } else if (i == 0 || i == 7) {
                // Pièces lourdes
                if (j == 0 || j == 7)      plateau[i][j].type = TOUR;
                else if (j == 1 || j == 6) plateau[i][j].type = CAVALIER;
                else if (j == 2 || j == 5) plateau[i][j].type = FOU;
                else if (j == 3)           plateau[i][j].type = DAME;
                else if (j == 4)           plateau[i][j].type = ROI;
                
                plateau[i][j].couleur = (i == 0) ? NOIR : BLANC; 
            } else {
                plateau[i][j].type = VIDE; plateau[i][j].couleur = AUCUNE; 
            }
        }
    }
}

/**
 * Affiche le plateau dans la console (Utile pour le debug/mode terminal).
 */
void afficherPlateau() {
    printf("\n    a b c d e f g h\n");
    printf("  +-----------------+\n");
    for (int i = 0; i < 8; i++) {
        printf("%d | ", 8 - i); 
        for (int j = 0; j < 8; j++) {
            char c = '.';
            if (plateau[i][j].type != VIDE) {
                switch(plateau[i][j].type) {
                    case PION:     c = 'p'; break;
                    case TOUR:     c = 'r'; break;
                    case CAVALIER: c = 'n'; break;
                    case FOU:      c = 'b'; break;
                    case DAME:     c = 'q'; break;
                    case ROI:      c = 'k'; break;
                    default:       c = '?'; break;
                }
                if (plateau[i][j].couleur == BLANC) c -= 32; // Majuscule pour les blancs
            }
            printf("%c ", c);
        }
        printf("| %d\n", 8 - i); 
    }
    printf("  +-----------------+\n");
    printf("    a b c d e f g h\n\n");
}

/**
 * Copie un état de plateau vers un autre (utilisé par l'IA pour simuler des coups).
 */
void copier_plateau(Case source[8][8], Case destination[8][8]) {
    memcpy(destination, source, sizeof(Case) * 64);
}

/**
 * Applique un mouvement brut sur le plateau sans aucune vérification.
 */
void appliquer_mouvement_direct(int x1, int y1, int x2, int y2) {
    int est_pion = (plateau[x1][y1].type == PION);
    int est_roi = (plateau[x1][y1].type == ROI);
    int est_ep = (est_pion && y1 != y2 && plateau[x2][y2].type == VIDE);
    int est_roque = (est_roi && abs(y2 - y1) == 2);

    // 1. Déplacement classique
    plateau[x2][y2] = plateau[x1][y1];
    plateau[x1][y1].type = VIDE;
    plateau[x1][y1].couleur = AUCUNE;

    // 2. Destruction du pion adverse (Prise en Passant)
    if (est_ep) {
        plateau[x1][y2].type = VIDE;
        plateau[x1][y2].couleur = AUCUNE;
    }

    // 3. Déplacement de la Tour (Le Roque)
    if (est_roque) {
        if (y2 == 6) { // Petit roque
            plateau[x2][5] = plateau[x2][7];
            plateau[x2][7].type = VIDE; plateau[x2][7].couleur = AUCUNE;
        } else if (y2 == 2) { // Grand roque
            plateau[x2][3] = plateau[x2][0];
            plateau[x2][0].type = VIDE; plateau[x2][0].couleur = AUCUNE;
        }
    }

    // 4. Promotion auto
    if (est_pion && ((plateau[x2][y2].couleur == BLANC && x2 == 0) || (plateau[x2][y2].couleur == NOIR && x2 == 7))) {
        plateau[x2][y2].type = DAME;
    }

    // 5. Mise à jour Prise en Passant pour le prochain tour
    if (est_pion && abs(x2 - x1) == 2) {
        ep_x = (x1 + x2) / 2; ep_y = y1;
    } else {
        ep_x = -1; ep_y = -1;
    }

    // 6. Mise à jour des Droits de Roque
    // Si le roi bouge
    if (x1 == 7 && y1 == 4) { roque_K = 0; roque_Q = 0; } 
    if (x1 == 0 && y1 == 4) { roque_k = 0; roque_q = 0; }
    // Si une Tour bouge OU est capturée sur sa case de départ
    if ((x1 == 7 && y1 == 7) || (x2 == 7 && y2 == 7)) roque_K = 0;
    if ((x1 == 7 && y1 == 0) || (x2 == 7 && y2 == 0)) roque_Q = 0;
    if ((x1 == 0 && y1 == 7) || (x2 == 0 && y2 == 7)) roque_k = 0;
    if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) roque_q = 0;
}

/* ==========================================================================
   2. FONCTIONS POUR LE JAVASCRIPT (WebAssembly)
   ========================================================================== */

/**
 * Génère une chaîne au format FEN (Forsyth-Edwards Notation).
 * Utilisé par chessboard.js pour mettre à jour l'affichage web.
 */
EMSCRIPTEN_KEEPALIVE
const char* renvoyer_FEN(EtatPartie *partie) {
    static char fen[128]; 
    int pos = 0;

    // A. Position des pièces (Rangées 8 à 1)
    for (int i = 0; i < 8; i++) {
        int vides = 0;
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].type == VIDE) {
                vides++;
            } else {
                if (vides > 0) {
                    fen[pos++] = vides + '0';
                    vides = 0;
                }
                
                char c;
                switch(plateau[i][j].type) {
                    case ROI:      c = 'k'; break;
                    case DAME:     c = 'q'; break;
                    case PION:     c = 'p'; break;
                    case TOUR:     c = 'r'; break;
                    case CAVALIER: c = 'n'; break;
                    case FOU:      c = 'b'; break;
                    default:       c = '?'; break;
                }
                // Majuscule si BLANC, minuscule si NOIR
                if (plateau[i][j].couleur == BLANC) c -= 32; 
                fen[pos++] = c;
            }
        }
        if (vides > 0) fen[pos++] = vides + '0';
        if (i < 7) fen[pos++] = '/';
    }

    // B. Métadonnées (Tour, Roques, En Passant, Demi-coups, Numéro de tour)
    char details[64];
    sprintf(details, " %c %s %s %d %d", 
            (partie->tour_joueur == BLANC) ? 'w' : 'b',
            partie->can_castle,
            partie->en_passant,
            partie->halmoven_clock,
            partie->fullmove_number);

    // Concaténation finale
    strcpy(fen + pos, details);

    return fen;
}

/**
 * Alias Web pour permettre au JS de forcer un mouvement sur le plateau C.
 */
EMSCRIPTEN_KEEPALIVE
void jouer_coup_IA(int x1, int y1, int x2, int y2) {
    appliquer_mouvement_direct(x1, y1, x2, y2);
}