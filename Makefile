# Nom de l'exécutable final
TARGET = echecs

# Compilateur
CC = gcc

# Options de compilation
# -Wall : affiche tous les avertissements
# -Wextra : affiche encore plus d'avertissements pour un code propre
# -g : ajoute les infos de débogage
CFLAGS = -Wall -Wextra -g

# Liste des fichiers source (.c)
SRCS = main.c jeu.c plateau.c pieces.c ia.c

# Génère la liste des fichiers objets (.o) à partir des sources
OBJS = $(SRCS:.c=.o)

# Règle par défaut : compile le projet
all: $(TARGET)

# Règle pour lier les fichiers objets et créer l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Règle pour compiler les fichiers .c en .o
# %.o: %.c signifie "pour chaque fichier .o, regarde le .c correspondant"
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour nettoyer les fichiers temporaires
clean:
	del /f $(OBJS) $(TARGET).exe

# Règle pour tout recommencer à zéro
re: clean all

# Indique que ces règles ne sont pas des fichiers
.PHONY: all clean re