#!/bin/sh

set -eu

usage()
{
    printf 'Uso: %s --check | --install\n' "$0"
}

check_dependencies()
{
    missing=0

    for command_name in cc cmake pkg-config; do
        if ! command -v "$command_name" >/dev/null 2>&1; then
            printf 'Dependência ausente: %s\n' "$command_name" >&2
            missing=1
        fi
    done

    if command -v pkg-config >/dev/null 2>&1 &&
       ! pkg-config --exists sdl2; then
        printf 'Dependência ausente: headers e biblioteca SDL2\n' >&2
        missing=1
    fi

    if [ "$missing" -ne 0 ]; then
        return 1
    fi

    printf 'Dependências de build disponíveis.\n'
}

install_dependencies()
{
    if [ ! -r /etc/os-release ]; then
        printf 'Não foi possível identificar a distribuição Linux.\n' >&2
        return 1
    fi

    # os-release is a system-provided shell-compatible metadata file.
    . /etc/os-release
    case "${ID:-} ${ID_LIKE:-}" in
        *debian*|*ubuntu*) ;;
        *)
            printf 'Distribuição não suportada: %s.\n' "${PRETTY_NAME:-desconhecida}" >&2
            printf 'Instale manualmente: compilador C, CMake e SDL2 de desenvolvimento.\n' >&2
            return 1
            ;;
    esac

    if [ "$(id -u)" -eq 0 ]; then
        privilege_command=
    elif command -v sudo >/dev/null 2>&1; then
        privilege_command=sudo
    else
        printf 'A instalação requer root ou o comando sudo.\n' >&2
        return 1
    fi

    printf 'Instalando dependências para %s...\n' "${PRETTY_NAME:-Debian/Ubuntu}"
    $privilege_command apt-get update
    $privilege_command apt-get install --no-install-recommends \
        build-essential \
        cmake \
        libsdl2-dev \
        pkg-config

    check_dependencies
}

if [ "$#" -ne 1 ]; then
    usage >&2
    exit 2
fi

case "$1" in
    --check) check_dependencies ;;
    --install) install_dependencies ;;
    -h|--help) usage ;;
    *)
        usage >&2
        exit 2
        ;;
esac
