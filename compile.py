import os
import sys
import subprocess

# Tenta importar o pyperclip para o clipboard. 
# Se não estiver instalado, avisa o utilizador.
try:
    import pyperclip
except ImportError:
    print("[Aviso] A biblioteca 'pyperclip' não está instalada. O código não será copiado para o clipboard.")
    print("Execute: pip install pyperclip")
    pyperclip = None

# Limpa o terminal (Windows / Unix)
os.system("cls" if os.name == "nt" else "clear")

def main():
    fullstr = ''
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
            
            # Só tenta ler o conteúdo se for um ficheiro de texto suportado
            if full_path.endswith(text_extensions):
                try:
                    with open(full_path, "r", encoding="utf-8", errors="ignore") as Cfile:
                        fullstr += Cfile.read() + "\n"
                except Exception as e:
                    print(f"[Erro ao ler] {full_path}: {e}")

    print("Ficheiros encontrados no projeto:")
    for f in all_files:
        print(f"  {f}")
        
    # 2. Recolher ficheiros fonte para compilação
    source_files = [f for f in all_files if f.endswith(('.cpp', '.c', '.cc'))]
    if not source_files:
        print("\nNenhum ficheiro fonte (.cpp, .c, .cc) encontrado para compilar.")
        return

    # Copia para o clipboard se a biblioteca estiver disponível
    if pyperclip and fullstr:
        pyperclip.copy(fullstr)
        print("\n[Sucesso] Todo o código fonte foi copiado para o clipboard!")

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
    
    print("\nA executar comando de compilação:")
    print(" ".join(cmd))
    
    # Execução segura usando a lista de argumentos diretamente (sem shell=True)
    result = subprocess.run(cmd)
    
    if result.returncode == 0:
        print(f"\nRunning: .\\{output_executable}\n" + "="*30 + "\n")
        
        # Executa o programa gerado com os argumentos específicos
        #run_cmd = [f".\\{output_executable}", "main.asmbi", "/D"]
        #subprocess.run(run_cmd)
    else:
        print("\n[Erro] A compilação falhou. Verifique os erros acima.")

if __name__ == "__main__":
    main()