# Nom de la sortie (doit finir par .js pour Emscripten)
TARGET = echecs.js

# Compilateur
CC = emcc

# Options de compilation
# -O2 : Optimisation pour la vitesse
# -s WASM=1 : Force la génération de WebAssembly
# -s MODULARIZE=1 : Permet d'utiliser .then() dans le JS
# -s EXPORT_NAME="'Echecs'" : Nom de la fonction d'initialisation
CFLAGS = -O2 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="'Echecs'"

# Fonctions à exporter (AJOUTE ICI TOUTES TES FONCTIONS C UTILISÉES EN JS)
# Note : On ajoute un '_' devant le nom des fonctions C
EXPORTS = -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
          -s EXPORTED_FUNCTIONS='["_initialiserPlateau", "_initialiser_partie", "_jouer_coup_web", "_renvoyer_FEN", "_main"]'

SRCS = jeu.c plateau.c pieces.c ia.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

# L'édition de liens doit inclure les EXPORTS
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(EXPORTS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /f *.o echecs.js echecs.wasm

re: clean all

.PHONY: all clean re