Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process
cd C:\Users\Utilisateur\emsdk
./emsdk_env.ps1 
cd C:\Users\Utilisateur\Documents\GitHub\chess-AI\v1.0
emcc main.c jeu.c ia.c plateau.c pieces.c `
-o moteur.js `
-s MODULARIZE=1 `
-s EXPORT_NAME='createModule' `
-s EXPORTED_FUNCTIONS='["_initialiser_partie", "_initialiserPlateau", "_jouer_meilleur_coup_IA", "_jouer_coup_web", "_renvoyer_FEN"]' `
-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' `
-s ALLOW_MEMORY_GROWTH=1 `
-O3