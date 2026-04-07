#include <stdlib.h>
#include <math.h>
#include <emscripten.h>

// 0: Vide, 1: Mur, 2: Joueur, 3: IA, 4: Sortie (Croix)
int grille[8][8];
int joueur_x, joueur_y;
int ia_x, ia_y;

EMSCRIPTEN_KEEPALIVE
void init_labyrinthe() {
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            // Zone de sécurité aux départs et à l'arrivée
            if ((i > 5 && j < 2) || (i < 2 && j > 5)) {
                grille[i][j] = 0;
            } else {
                grille[i][j] = (rand() % 100 < 25) ? 1 : 0; // 25% de murs
            }
        }
    }
    // Placer les acteurs et la sortie
    joueur_x = 7; joueur_y = 0;
    ia_x = 1; ia_y = 7;         // L'IA monte la garde
    
    grille[joueur_x][joueur_y] = 2;
    grille[ia_x][ia_y] = 3;
    grille[0][7] = 4;           // 4 = La Sortie (Croix)
}

EMSCRIPTEN_KEEPALIVE
int lire_case_laby(int x, int y) {
    if(x < 0 || x > 7 || y < 0 || y > 7) return 1;
    return grille[x][y];
}

EMSCRIPTEN_KEEPALIVE
int deplacer_joueur_laby(int cible_x, int cible_y) {
    if(cible_x < 0 || cible_x > 7 || cible_y < 0 || cible_y > 7) return 0;
    if(grille[cible_x][cible_y] == 1) return 0; // Mur
    
    // Déplacement d'une case max
    if(abs(cible_x - joueur_x) > 1 || abs(cible_y - joueur_y) > 1) return 0;

    // VICTOIRE : On marche sur la sortie
    if(grille[cible_x][cible_y] == 4) return 3; 

    // MORT STUPIDE : On marche délibérément sur l'IA
    if(cible_x == ia_x && cible_y == ia_y) return -1; 

    grille[joueur_x][joueur_y] = 0;
    joueur_x = cible_x; 
    joueur_y = cible_y;
    
    grille[joueur_x][joueur_y] = 2;
    return 1;
}

EMSCRIPTEN_KEEPALIVE
int jouer_ia_laby() {
    int g_score[8][8], f_score[8][8], parent_x[8][8], parent_y[8][8], open[8][8], closed[8][8];
    
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            g_score[i][j] = 9999; f_score[i][j] = 9999;
            parent_x[i][j] = -1; open[i][j] = 0; closed[i][j] = 0;
        }
    }

    g_score[ia_x][ia_y] = 0;
    f_score[ia_x][ia_y] = abs(ia_x - joueur_x) > abs(ia_y - joueur_y) ? abs(ia_x - joueur_x) : abs(ia_y - joueur_y);
    open[ia_x][ia_y] = 1;

    int current_x = ia_x, current_y = ia_y, found = 0;

    while(1) {
        int min_f = 9999, best_x = -1, best_y = -1;
        for(int i=0; i<8; i++) {
            for(int j=0; j<8; j++) {
                if(open[i][j] && f_score[i][j] < min_f) {
                    min_f = f_score[i][j]; best_x = i; best_y = j;
                }
            }
        }

        if(best_x == -1) break; 
        current_x = best_x; current_y = best_y;

        if(current_x == joueur_x && current_y == joueur_y) { found = 1; break; }

        open[current_x][current_y] = 0;
        closed[current_x][current_y] = 1;

        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                if(dx == 0 && dy == 0) continue;
                int nx = current_x + dx, ny = current_y + dy;

                if(nx >= 0 && nx < 8 && ny >= 0 && ny < 8 && !closed[nx][ny] && grille[nx][ny] != 1) {
                    int tentative_g = g_score[current_x][current_y] + 1;
                    if(!open[nx][ny] || tentative_g < g_score[nx][ny]) {
                        parent_x[nx][ny] = current_x; parent_y[nx][ny] = current_y;
                        g_score[nx][ny] = tentative_g;
                        int h = abs(nx - joueur_x) > abs(ny - joueur_y) ? abs(nx - joueur_x) : abs(ny - joueur_y);
                        f_score[nx][ny] = tentative_g + h;
                        open[nx][ny] = 1;
                    }
                }
            }
        }
    }

    if(found) {
        int step_x = joueur_x, step_y = joueur_y;
        while(parent_x[step_x][step_y] != ia_x || parent_y[step_x][step_y] != ia_y) {
            int px = parent_x[step_x][step_y];
            int py = parent_y[step_x][step_y];
            step_x = px; step_y = py;
        }
        
        // L'IA libère l'ancienne case (Sauf si c'était la sortie qu'elle gardait !)
        if (ia_x == 0 && ia_y == 7) grille[ia_x][ia_y] = 4;
        else grille[ia_x][ia_y] = 0;
        
        ia_x = step_x; ia_y = step_y;
        
        if(ia_x == joueur_x && ia_y == joueur_y) return 1; // L'IA a mangé le joueur
        grille[ia_x][ia_y] = 3;
    }
    return 0; 
}