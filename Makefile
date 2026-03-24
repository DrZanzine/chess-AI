# Nom de la sortie
TARGET = echecs.js

# Compilateur
CC = emcc

# Options de compilation
CFLAGS = -O2

# Options de liaison (JS/WASM)
# Suppression des crochets complexes qui font planter PowerShell
LDFLAGS = -s WASM=1 \
          -s MODULARIZE=1 \
          -s EXPORT_NAME="Echecs" \
          -s ALLOW_MEMORY_GROWTH=1 \
          -s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
          -s EXPORTED_FUNCTIONS=_initialiserPlateau,_initialiser_partie_web,_jouer_coup_web,_renvoyer_FEN

# Fichiers sources et objets
SRCS = jeu.c plateau.c pieces.c ia.c
OBJS = $(SRCS:.c=.o)

# Règle principale
all: $(TARGET)

# Liaison finale
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Compilation des objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	@if exist *.o del /f *.o
	@if exist $(TARGET) del /f $(TARGET)
	@if exist echecs.wasm del /f echecs.wasm

.PHONY: all clean