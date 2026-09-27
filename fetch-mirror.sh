#!/usr/bin/env bash

set -e

# URLs base de los mirrors de Alpine (ajustá la versión o arquitectura según tu target, ej: v3.19 / x86_64)
MIRROR_MAIN="https://dl-cdn.alpinelinux.org/alpine/v3.19/main/x86_64"
MIRROR_COMMUNITY="https://dl-cdn.alpinelinux.org/alpine/v3.19/community/x86_64"

mkdir -p pkgs pkgs-extra libs libs-extra temp_extract

download_repo() {
    local index_file="$1"
    local mirror_url="$2"
    local target_pkg_dir="$3"
    local target_lib_dir="$4"

    echo "[INFO] Procesando $index_file..."

    while IFS= read -r line; do
        if [[ "$line" =~ ^P:(.+) ]]; then
            pkgname="${BASH_REMATCH[1]}"
        elif [[ "$line" =~ ^V:(.+) ]]; then
            pkgver="${BASH_REMATCH[1]}"
            filename="${pkgname}-${pkgver}.apk"
            
            echo "[FETCH] Descargando ${filename}..."
            if curl -s -f "${mirror_url}/${filename}" -o "${target_pkg_dir}/${filename}"; then
                # Extraer librerías .so del paquete descargado hacia la carpeta de libs correspondiente
                tar -xzf "${target_pkg_dir}/${filename}" -C temp_extract 2>/dev/null || true
                find temp_extract -name "*.so*" -exec cp -a {} "${target_lib_dir}/" \;
                rm -rf temp_extract/*
            else
                echo "[WARN] No se pudo descargar ${filename}"
            fi
        fi
    done < "$index_file"
}

# Procesar Main -> pkgs / libs
download_repo "APKINDEX-main" "$MIRROR_MAIN" "pkgs" "libs"

# Procesar Community/Extra -> pkgs-extra / libs-extra
download_repo "APKINDEX-extra" "$MIRROR_COMMUNITY" "pkgs-extra" "libs-extra"

rm -rf temp_extract
echo "[OK] Descarga y clasificación finalizada."
