import sys
import subprocess

# On utilise sys.executable pour pointer vers LE python qui lance ce script
# Cela évite de dépendre du "PATH" de Windows
subprocess.run([sys.executable, "-m", "http.server", "8000"])