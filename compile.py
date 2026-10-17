import os
import sys
import subprocess

# Limpa o terminal (Windows)
os.system("cls" if os.name == "nt" else "clear")

def main():
    all_files = []
    
    # Extensions permitidas para leitura de texto (evita ler binários como .exe ou imagens)
    text_extensions = ('.cpp', '.c', '.cc', '.h', '.hpp', '.txt')

    # 1. Encontrar todos os ficheiros corretamente
    for root, dirs, files in os.walk('.'):
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

    print("\n[Sucesso] Todo o código fonte foi copiado para o clipboard!")

    # 3. Configurar comando de compilação (Flags separadas corretamente)
    output_executable = "main.exe"
    
    # Separamos cada flag como um elemento independente da lista
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

    print("\nA executar comando de compilação:")
    command_string = " ".join(cmd)
    print(command_string)
    
    # Usar subprocess é mais seguro e moderno do que os.system
    result = subprocess.run(command_string, shell=True)

    if result.returncode == 0:
        print(f"\nRunning: .\\{output_executable}\n" + "="*30 + "\n")
        subprocess.run(f".\\{output_executable}", shell=True)
    else:
        print("\n[Erro] A compilação falhou. Verifique os erros acima.")

if __name__ == "__main__":
    main()
