#include "ia.h"
#include "pieces.h"
#include "plateau.h"
#include <limits.h>
#include <stdio.h>

// Tables de positionnement (Square-Table)
const int table_pion[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5,  5, 10, 25, 25, 10,  5,  5},
    {0,  0,  0, 20, 20,  0,  0,  0},
    {5, -5,-10,  0,  0,-10, -5,  5},
    {5, 10, 10,-20,-20, 10, 10,  5},
    {0,  0,  0,  0,  0,  0,  0,  0}
};

const int table_cavalier[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

int evaluer_plateau() {
    int score = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Case p = plateau[i][j];
            if (p.type == VIDE) continue;

            int valeur = 0;
            int pos_i = (p.couleur == NOIR) ? (7 - i) : i; // Inverse la table pour les Noirs

            switch(p.type) {
                case PION:     valeur = 100 + table_pion[pos_i][j]; break;
                case CAVALIER: valeur = 320 + table_cavalier[pos_i][j]; break;
                case FOU:      valeur = 330; break;
                case TOUR:     valeur = 500; break;
                case DAME:     valeur = 900; break;
                case ROI:      valeur = 20000; break;
                default: break;
            }

            if (p.couleur == NOIR) score += valeur;
            else score -= valeur;
        }
    }
    return score;
}

int minimax(int profondeur, int alpha, int beta, int est_max) {
    if (profondeur == 0) return evaluer_plateau();

    if (est_max) {
        int max_eval = -30000;
        for (int x1=0; x1<8; x1++) {
            for (int y1=0; y1<8; y1++) {
                if (plateau[x1][y1].couleur == NOIR) {
                    for (int x2=0; x2<8; x2++) {
                        for (int y2=0; y2<8; y2++) {
                            if (est_mouvement_valide(x1, y1, x2, y2, NOIR)) {
                                Case svg_dest = plateau[x2][y2];
                                Case svg_dep = plateau[x1][y1];
                                jouer_coup_IA(x1, y1, x2, y2);
                                int eval = minimax(profondeur - 1, alpha, beta, 0);
                                // Annulation manuelle précise
                                plateau[x1][y1] = svg_dep;
                                plateau[x2][y2] = svg_dest;

                                if (eval > max_eval) max_eval = eval;
                                if (eval > alpha) alpha = eval;
                                if (beta <= alpha) return max_eval; 
                            }
                        }
                    }
                }
            }
        }
        return max_eval;
    } else {
        int min_eval = 30000;
        for (int x1=0; x1<8; x1++) {
            for (int y1=0; y1<8; y1++) {
                if (plateau[x1][y1].couleur == BLANC) {
                    for (int x2=0; x2<8; x2++) {
                        for (int y2=0; y2<8; y2++) {
                            if (est_mouvement_valide(x1, y1, x2, y2, BLANC)) {
                                Case svg_dest = plateau[x2][y2];
                                Case svg_dep = plateau[x1][y1];
                                jouer_coup_IA(x1, y1, x2, y2);
                                int eval = minimax(profondeur - 1, alpha, beta, 1);
                                plateau[x1][y1] = svg_dep;
                                plateau[x2][y2] = svg_dest;

                                if (eval < min_eval) min_eval = eval;
                                if (eval < beta) beta = eval;
                                if (beta <= alpha) return min_eval;
                            }
                        }
                    }
                }
            }
        }
        return min_eval;
    }
}

void jouer_meilleur_coup_IA(int profondeur) {
    int meilleur_score = -40000;
    int m_x1 = -1, m_y1 = -1, m_x2 = -1, m_y2 = -1;

    for (int x1=0; x1<8; x1++) {
        for (int y1=0; y1<8; y1++) {
            if (plateau[x1][y1].couleur == NOIR) {
                for (int x2=0; x2<8; x2++) {
                    for (int y2=0; y2<8; y2++) {
                        if (est_mouvement_valide(x1, y1, x2, y2, NOIR)) {
                            Case svg_dest = plateau[x2][y2];
                            Case svg_dep = plateau[x1][y1];
                            jouer_coup_IA(x1, y1, x2, y2);
                            
                            // On utilise le paramètre profondeur ici
                            int score = minimax(profondeur - 1, -40000, 40000, 0); 
                            
                            plateau[x1][y1] = svg_dep;
                            plateau[x2][y2] = svg_dest;

                            if (score > meilleur_score) {
                                meilleur_score = score;
                                m_x1 = x1; m_y1 = y1; m_x2 = x2; m_y2 = y2;
                            }
                        }
                    }
                }
            }
        }
    }
    // ... reste de la fonction (affichage et coup réel)
    if (m_x1 != -1) {
        printf("L'IA joue : %c%d -> %c%d\n", m_y1+'a', 8-m_x1, m_y2+'a', 8-m_x2);
        jouer_coup_IA(m_x1, m_y1, m_x2, m_y2);
    }
}