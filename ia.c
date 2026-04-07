#include "ia.h"
#include "jeu.h"
#include "pieces.h"
#include "plateau.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

// --- VARIABLES GLOBALES ZOBRIST ---
uint64_t pieces_zobrist[2][7][64];
uint64_t trait_noir_zobrist;
EntreeTT table_transposition[TAILLE_TT];

// Génère un grand nombre aléatoire de 64 bits
uint64_t rand64() {
    uint64_t r1 = (uint64_t)rand();
    uint64_t r2 = (uint64_t)rand();
    uint64_t r3 = (uint64_t)rand();
    uint64_t r4 = (uint64_t)rand();
    return (r1 << 48) | (r2 << 32) | (r3 << 16) | r4;
}

void initialiser_zobrist() {
    for (int c = 0; c < 2; c++) {
        for (int p = 0; p < 7; p++) {
            for (int i = 0; i < 64; i++) {
                pieces_zobrist[c][p][i] = rand64();
            }
        }
    }
    trait_noir_zobrist = rand64();
}

void vider_table_transposition() {
    for (int i = 0; i < TAILLE_TT; i++) {
        table_transposition[i].cle_zobrist = 0;
    }
}

// Calcule l'empreinte unique du plateau
uint64_t calculer_hash_plateau(int est_max) {
    uint64_t hash = 0;
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Case p = plateau[x][y];
            if (p.type != VIDE) {
                int couleur_idx = (p.couleur == NOIR) ? 1 : 0;
                hash ^= pieces_zobrist[couleur_idx][p.type][x * 8 + y];
            }
        }
    }
    if (est_max) hash ^= trait_noir_zobrist; // Si c'est aux Noirs de jouer
    return hash;
}

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

// Valeur rapide pour le tri des coups
int valeur_piece_tri(TypePiece type) {
    switch(type) {
        case PION: return 100;
        case CAVALIER: return 320;
        case FOU: return 330;
        case TOUR: return 500;
        case DAME: return 900;
        default: return 0;
    }
}

// Tri de la liste du plus grand score (meilleure capture) au plus petit (case vide)
void trier_coups(ListeCoups *liste) {
    for (int i = 0; i < liste->nb - 1; i++) {
        for (int j = i + 1; j < liste->nb; j++) {
            if (liste->coups[i].score_tri < liste->coups[j].score_tri) {
                Coup temp = liste->coups[i];
                liste->coups[i] = liste->coups[j];
                liste->coups[j] = temp;
            }
        }
    }
}

// --- GESTIONNAIRE D'ANNULATION DE COUP ---
typedef struct {
    Coup c;
    Case svg_dep;
    Case svg_dest;
    int ep_x_svg, ep_y_svg;
    int etait_ep, etait_roque;
    int rK, rQ, rk, rq; // Sauvegarde des droits de roque
} UndoInfo;

UndoInfo faire_coup_ia(Coup c) {
    UndoInfo u;
    u.c = c;
    u.svg_dep = plateau[c.x1][c.y1];
    u.svg_dest = plateau[c.x2][c.y2];
    u.ep_x_svg = ep_x; u.ep_y_svg = ep_y;
    u.etait_ep = (u.svg_dep.type == PION && c.y1 != c.y2 && u.svg_dest.type == VIDE);
    u.etait_roque = (u.svg_dep.type == ROI && abs(c.y2 - c.y1) == 2);
    u.rK = roque_K; u.rQ = roque_Q; u.rk = roque_k; u.rq = roque_q;

    appliquer_mouvement_direct(c.x1, c.y1, c.x2, c.y2);
    return u;
}

void defaire_coup_ia(UndoInfo u) {
    plateau[u.c.x1][u.c.y1] = u.svg_dep;
    plateau[u.c.x2][u.c.y2] = u.svg_dest;
    ep_x = u.ep_x_svg; ep_y = u.ep_y_svg;
    roque_K = u.rK; roque_Q = u.rQ; roque_k = u.rk; roque_q = u.rq;
    
    if (u.etait_ep) {
        plateau[u.c.x1][u.c.y2].type = PION;
        plateau[u.c.x1][u.c.y2].couleur = (u.svg_dep.couleur == BLANC) ? NOIR : BLANC;
    }
    
    // Le miracle : On remet la Tour à sa place si c'était un roque !
    if (u.etait_roque) {
        if (u.c.y2 == 6) { // Petit roque
            plateau[u.c.x1][7] = plateau[u.c.x1][5];
            plateau[u.c.x1][5].type = VIDE; plateau[u.c.x1][5].couleur = AUCUNE;
        } else if (u.c.y2 == 2) { // Grand roque
            plateau[u.c.x1][0] = plateau[u.c.x1][3];
            plateau[u.c.x1][3].type = VIDE; plateau[u.c.x1][3].couleur = AUCUNE;
        }
    }
}
// -----------------------------------------

void generer_tous_les_coups(Couleur couleur, ListeCoups *liste) {
    liste->nb = 0; // On initialise le compteur à 0

    // On cherche d'abord nos pièces
    for (int x1 = 0; x1 < 8; x1++) {
        for (int y1 = 0; y1 < 8; y1++) {
            if (plateau[x1][y1].couleur == couleur) {
                
                // Pour chaque pièce trouvée, on teste les destinations possibles
                for (int x2 = 0; x2 < 8; x2++) {
                    for (int y2 = 0; y2 < 8; y2++) {
                        
                        if (est_mouvement_valide(x1, y1, x2, y2, couleur)) {
                            // On simule le coup pour voir si on se met en échec
                            Coup coup_temp = {x1, y1, x2, y2, 0}; // On crée un "Coup" temporaire
                            UndoInfo u = faire_coup_ia(coup_temp); // On l'envoie à la fonction

                            int en_echec = est_echec(couleur);
                            
                            // On annule la simulation
                            defaire_coup_ia(u);
                            
                            // Si le coup est légal ET qu'il ne nous met pas en échec, on l'ajoute !
                            if (!en_echec) {
                                liste->coups[liste->nb].x1 = x1;
                                liste->coups[liste->nb].y1 = y1;
                                liste->coups[liste->nb].x2 = x2;
                                liste->coups[liste->nb].y2 = y2;
                                liste->coups[liste->nb].score_tri = valeur_piece_tri(plateau[x2][y2].type);
                                if (plateau[x1][y1].type == PION && (x2 == 0 || x2 == 7)) {
                                    liste->coups[liste->nb].score_tri += 900;
                                }
                                liste->nb++;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Générateur ultra-rapide qui ne renvoie QUE les captures
void generer_captures(Couleur couleur, ListeCoups *liste) {
    liste->nb = 0;

    for (int x1 = 0; x1 < 8; x1++) {
        for (int y1 = 0; y1 < 8; y1++) {
            if (plateau[x1][y1].couleur == couleur) {
                
                for (int x2 = 0; x2 < 8; x2++) {
                    for (int y2 = 0; y2 < 8; y2++) {
                        
                        // FILTRE : On ne teste la validité QUE si la case cible contient un ennemi
                        if (plateau[x2][y2].type != VIDE && plateau[x2][y2].couleur != couleur) {
                            
                            if (est_mouvement_valide(x1, y1, x2, y2, couleur)) {
                                Coup coup_temp = {x1, y1, x2, y2, 0}; // On crée un "Coup" temporaire
                                UndoInfo u = faire_coup_ia(coup_temp); // On l'envoie à la fonction

                                int en_echec = est_echec(couleur);
                                
                                defaire_coup_ia(u);
                                
                                if (!en_echec) {
                                    liste->coups[liste->nb].x1 = x1;
                                    liste->coups[liste->nb].y1 = y1;
                                    liste->coups[liste->nb].x2 = x2;
                                    liste->coups[liste->nb].y2 = y2;
                                    liste->coups[liste->nb].score_tri = valeur_piece_tri(plateau[x2][y2].type);
                                    liste->nb++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int quiescence(int alpha, int beta, int est_max) {
    // 1. Évaluation "Stand Pat" : On a toujours le droit de ne rien faire.
    // Si la position actuelle est déjà trop forte, on coupe.
    int eval = evaluer_plateau();

    if (est_max) {
        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    } else {
        if (eval <= alpha) return alpha;
        if (eval < beta) beta = eval;
    }

    // 2. On génère UNIQUEMENT les captures
    Couleur couleur_actuelle = est_max ? NOIR : BLANC;
    ListeCoups liste;
    generer_captures(couleur_actuelle, &liste);
    trier_coups(&liste); // Le tri est encore plus vital ici !

    // 3. Boucle de recherche simplifiée
    if (est_max) {
        int max_eval = eval;
        for (int i = 0; i < liste.nb; i++) {
            Coup c = liste.coups[i];
            UndoInfo u = faire_coup_ia(c);
            
            int score = quiescence(alpha, beta, 0); // Appel récursif
            defaire_coup_ia(u);

            if (score > max_eval) max_eval = score;
            if (score > alpha) alpha = score;
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        int min_eval = eval;
        for (int i = 0; i < liste.nb; i++) {
            Coup c = liste.coups[i];
            UndoInfo u = faire_coup_ia(c);
            
            appliquer_mouvement_direct(c.x1, c.y1, c.x2, c.y2);
            int score = quiescence(alpha, beta, 1); // Appel récursif
            defaire_coup_ia(u);

            if (score < min_eval) min_eval = score;
            if (score < beta) beta = score;
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

int minimax(int profondeur, int alpha, int beta, int est_max) {
    // 1. Zobrist : On calcule le hash et on cherche dans la table
    uint64_t hash_actuel = calculer_hash_plateau(est_max);
    int index_tt = hash_actuel % TAILLE_TT;

    if (table_transposition[index_tt].cle_zobrist == hash_actuel && table_transposition[index_tt].profondeur >= profondeur) {
        int score_tt = table_transposition[index_tt].score;
        int flag_tt = table_transposition[index_tt].flag;

        if (flag_tt == 0) return score_tt;                               // EXACT
        if (flag_tt == 1 && score_tt <= alpha) return alpha;             // ALPHA (Borne supérieure)
        if (flag_tt == 2 && score_tt >= beta) return beta;               // BETA (Borne inférieure)
    }

    // 2. Condition d'arrêt
    if (profondeur == 0) return quiescence(alpha, beta, est_max);

    int alpha_origine = alpha;
    Couleur couleur_actuelle = est_max ? NOIR : BLANC;

// ==========================================================
    // 3. NULL-MOVE PRUNING (L'Élagage du Coup Nul)
    // On ne le fait pas si on est en échec (passer son tour serait illégal)
    // On exige une profondeur >= 3 pour que la soustraction (profondeur - 3) ne bug pas.
    if (profondeur >= 3 && !est_echec(couleur_actuelle)) {
        // On "passe notre tour" en appelant minimax sur l'autre joueur, avec une profondeur réduite de 3
        int eval_nmp = minimax(profondeur - 3, alpha, beta, !est_max);
        
        // Si même en passant notre tour, on dépasse les limites de l'adversaire, on coupe !
        if (est_max && eval_nmp >= beta) return beta;
        if (!est_max && eval_nmp <= alpha) return alpha;
    }
    // ==========================================================

    ListeCoups liste;
    generer_tous_les_coups(couleur_actuelle, &liste);
    trier_coups(&liste);

    // 4. Gestion Fin de Partie
    if (liste.nb == 0) {
        if (est_echec(couleur_actuelle)) return est_max ? -30000 : 30000;
        return 0; // Pat
    }

    // 5. Recherche fusionnée (plus de duplication de code !)
    int meilleur_score = est_max ? -40000 : 40000;

    for (int i = 0; i < liste.nb; i++) {
        Coup c = liste.coups[i];
        UndoInfo u = faire_coup_ia(c);
        
        // On passe à la profondeur inférieure, en inversant "est_max" (!est_max)
        int eval = minimax(profondeur - 1, alpha, beta, !est_max); 
        
        defaire_coup_ia(u);

        // Mise à jour des bornes selon le joueur
        if (est_max) {
            if (eval > meilleur_score) meilleur_score = eval;
            if (eval > alpha) alpha = eval;
        } else {
            if (eval < meilleur_score) meilleur_score = eval;
            if (eval < beta) beta = eval;
        }

        // Élagage
        if (beta <= alpha) break; 
    }

    // 6. Zobrist : On sauvegarde notre résultat dans la table
    int flag = 0; // EXACT
    if (meilleur_score <= alpha_origine) flag = 1; // ALPHA
    else if (meilleur_score >= beta) flag = 2; // BETA

    table_transposition[index_tt].cle_zobrist = hash_actuel;
    table_transposition[index_tt].profondeur = profondeur;
    table_transposition[index_tt].score = meilleur_score;
    table_transposition[index_tt].flag = flag;

    return meilleur_score;
}

void jouer_meilleur_coup_IA(int profondeur_max) {
    ListeCoups liste;
    generer_tous_les_coups(NOIR, &liste); // L'IA joue les Noirs

    Coup meilleur_coup_global = {-1, -1, -1, -1, 0};

    // --- LA BOUCLE D'APPROFONDISSEMENT ITÉRATIF ---
    for (int prof = 1; prof <= profondeur_max; prof++) {
        int meilleur_score_iteration = -40000;
        Coup meilleur_coup_iteration = {-1, -1, -1, -1, 0};

        // 1. On trie la liste des coups avant de lancer la recherche.
        // À prof=1, c'est trié par capture. 
        // À prof>1, c'est trié grâce au vrai score de l'itération précédente !
        trier_coups(&liste);

        for (int i = 0; i < liste.nb; i++) {
            Coup *c = &liste.coups[i]; // On utilise un pointeur pour modifier le score_tri
            UndoInfo u = faire_coup_ia(*c);
            
            int score = minimax(prof - 1, -40000, 40000, 0); 
            defaire_coup_ia(u);

            // 2. On sauvegarde ce score pour trier la liste à la PROCHAINE itération
            c->score_tri = score;

            if (score > meilleur_score_iteration) {
                meilleur_score_iteration = score;
                meilleur_coup_iteration = *c;
            }
        }

        // 3. Sauvegarde du meilleur coup de cette profondeur
        meilleur_coup_global = meilleur_coup_iteration;
        printf("Profondeur %d terminee. Meilleur coup temporaire : %c%d -> %c%d (Score: %d)\n", 
               prof,
               meilleur_coup_global.y1+'a', 8-meilleur_coup_global.x1, 
               meilleur_coup_global.y2+'a', 8-meilleur_coup_global.x2, meilleur_score_iteration);

        // 4. Opti : Si on a trouvé un Mat inévitable (score extrême), on arrête de chercher plus loin !
        if (meilleur_score_iteration > 29000) {
            printf("Mat trouve ! Arrêt anticipé de l'Iterative Deepening.\n");
            break;
        }
    }

    // --- FIN DE L'ITÉRATION, ON JOUE LE COUP ---
    if (meilleur_coup_global.x1 != -1) {
        printf(">>> L'IA joue definitivement : %c%d -> %c%d\n", 
               meilleur_coup_global.y1+'a', 8-meilleur_coup_global.x1, 
               meilleur_coup_global.y2+'a', 8-meilleur_coup_global.x2);
        appliquer_mouvement_direct(meilleur_coup_global.x1, meilleur_coup_global.y1, 
                                   meilleur_coup_global.x2, meilleur_coup_global.y2);
    }
}