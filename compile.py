import os
import sys
import subprocess

# Limpa o terminal (Windows / Unix)
os.system("cls" if os.name == "nt" else "clear")

def main():
    all_files = []
    
    # Extensões permitidas para leitura de texto
    text_extensions = ('.cpp', '.c', '.cc', '.h', '.hpp', '.txt')
    
    # 1. Encontrar todos os ficheiros corretamente
    for root, dirs, files in os.walk('.'):
        # Ignora a pasta .git para não perder tempo a percorrê-la
        if '.git' in root:
            continue
            
        for file in files:
            full_path = os.path.join(root, file)
            all_files.append(full_path)


    print("Ficheiros encontrados no projeto:")
    for f in all_files:
        print(f"  {f}")
        
    # 2. Recolher ficheiros fonte para compilação
    source_files = [f for f in all_files if f.endswith(('.cpp', '.c', '.cc'))]
    if not source_files:
        print("\nNenhum ficheiro fonte (.cpp, .c, .cc) encontrado para compilar.")
        return

    # 3. Configurar comando de compilação
    output_executable = "main.exe"
    
    cmd = [
        "g++",
        *source_files,
        "-o", output_executable,
        "-IC:\\raylib\\raylib\\src",
        "-LC:\\raylib\\raylib\\src",
        "-lraylib",
        "-lopengl32",
        "-lgdi32",
        "-lwinmm"
    ]
    
    print("\nA Executing:")
    print(" ".join(cmd))
    
    # Execução segura usando a lista de argumentos diretamente (sem shell=True)
    result = subprocess.run(cmd)
    
    if result.returncode != 0:
        print("\n[Error] Compilation Failed.")

if __name__ == "__main__":
    main()