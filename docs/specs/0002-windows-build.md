# 0002 - Build nativo para Windows

**Status:** Build Windows validado
**Plataforma:** Windows 10/11 x64  
**Referencia:** nucleo portatil definido em `0001-linux-port.md`

## Problema

O build atual assume opcoes de compilacao GCC/Clang, vincula `libm`
incondicionalmente e usa um caminho de inclusao do SDL2 que nao e uniforme entre
Linux e os pacotes CMake para Windows. Essas hipoteses impedem a configuracao
direta com MSVC.

## Objetivo

Permitir configurar, compilar e testar o port com MSVC e SDL2 fornecido pelo
vcpkg, preservando o build Linux e o mesmo codigo portatil do jogo.

## Escopo

- selecionar avisos equivalentes para MSVC e compiladores GCC/Clang;
- vincular a biblioteca matematica somente nas plataformas que a exigem;
- consumir os targets CMake exportados pelo SDL2, incluindo `SDL2main` quando
  ele estiver disponivel;
- manter as assercoes habilitadas nos executaveis de teste em configuracoes
  `Release`;
- reservar pilha suficiente para os testes que mantem varios estados completos
  do jogo como variaveis locais no MSVC;
- documentar configuracao, build, smoke check e testes no Windows.

## Nao objetivos

- suportar o codigo DOS historico com MSVC;
- empacotar SDL2 ou gerar um instalador do jogo;
- instalar Visual Studio, CMake ou vcpkg automaticamente;
- adicionar suporte a DirectX ou substituir SDL2.

## Criterios de aceitacao

1. [x] CMake configura com Visual Studio 2022 ou mais recente, x64, e o
   toolchain do vcpkg.
2. [x] O build MSVC termina sem erros e usa `/W4` nos targets do projeto.
3. [x] `wolf3d.exe --check` imprime `SDL2 OK` e retorna zero.
4. [x] CTest executa todos os testes portateis no Windows.
5. [ ] O build Linux continua usando `-Wall -Wextra -Wpedantic` e `libm`.
6. [x] `README.rst` e `WOLFSRC/` permanecem intactos.

## Estado

A configuracao e o build `Release` foram validados no Windows x64 com MSVC
19.51, CMake 4.3 e SDL2 2.32.10 instalado pelo vcpkg. O smoke check retornou
`SDL2 OK` e os seis testes portateis passaram. A regressao do build Linux ainda
precisa ser executada em um ambiente Linux.
