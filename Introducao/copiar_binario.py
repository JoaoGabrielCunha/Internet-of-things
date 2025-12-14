Import("env")
import shutil
import os

def copy_binaries(source, target, env):
    # Caminho para o diretório de destino comum
    common_build_dir = os.path.join(env.get("PROJECT_DIR"), "common_build")
    os.makedirs(common_build_dir, exist_ok=True)

    # Caminho do arquivo .bin
    bin_path = target[0].get_abspath()
    bin_destination = os.path.join(common_build_dir, "firmware.bin")
    
    # Copiar o arquivo .bin
    shutil.copy(bin_path, bin_destination)
    print(f"Binary (.bin) copied to {bin_destination}")

    # Deduzir o caminho do .elf a partir do .bin
    bin_dir = os.path.dirname(bin_path)
    elf_path = os.path.join(bin_dir, os.path.splitext(os.path.basename(bin_path))[0] + '.elf')

    if os.path.exists(elf_path):
        elf_destination = os.path.join(common_build_dir, "firmware.elf")
        shutil.copy(elf_path, elf_destination)
        print(f"Binary (.elf) copied to {elf_destination}")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_binaries)