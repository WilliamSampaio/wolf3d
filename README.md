# Wolfenstein 3D — port educacional

Port progressivo do código-fonte original de Wolfenstein 3D para plataformas
modernas. O objetivo inicial é aprendizado: compreender o motor DOS de 1992 e
substituir, em etapas verificáveis, suas dependências de Borland C++, VGA,
interrupções e assembly de 16 bits.

O Linux é a primeira plataforma. O suporte a Windows virá depois que o núcleo
portável estiver funcional.

## Estado atual

- build nativo em C com CMake;
- janela SDL2 e framebuffer de 320×200 escalado;
- loop de eventos com saída por `Esc`;
- smoke check executável sem interface gráfica;
- validação dos conjuntos de dados shareware (`WL1`) e completo (`WL6`);
- leitor seguro de `VSWAP` e teste automatizado com arquivo sintético;
- paleta VGA original e renderização da primeira textura de parede;
- renderização validada com os dados shareware v1.4 incluídos em `data/`;
- descompressão Carmack/RLEW e carregamento dos dois planos do primeiro mapa;
- visão superior estática com paredes, portas e orientação do jogador;
- carregamento das 106 páginas de parede, incluindo páginas esparsas;
- raycasting DDA com paredes texturizadas e câmera inicial estática;
- código histórico preservado em `WOLFSRC/`.

O jogo original ainda não é jogável. A primeira cena 3D estática foi validada;
movimento e colisão pertencem ao próximo marco.

## Compilar no Linux

Requisitos: compilador C, CMake 3.16+ e headers de desenvolvimento do SDL2.

```sh
cmake -S . -B build
cmake --build build
./build/wolf3d --data data/shareware-v1.4
```

Verificação rápida:

```sh
./build/wolf3d --check
./build/wolf3d --check --data data/shareware-v1.4
ctest --test-dir build --output-on-failure
git diff --check
```

O diretório deve conter os oito arquivos originais não vazios: `VSWAP`,
`GAMEMAPS`, `MAPHEAD`, `VGADICT`, `VGAHEAD`, `VGAGRAPH`, `AUDIOHED` e
`AUDIOT`, todos com extensão `.WL1` ou `.WL6`.

O pacote shareware v1.4 usado no desenvolvimento veio do
[Internet Archive](https://archive.org/details/wolf3dsw). O ZIP original, sua
origem, checksum e organização estão documentados em
[`data/README.md`](data/README.md). Dados comerciais `.WL6` não fazem parte do
repositório.

Ao executar, a janela mostra a visão 3D do início de `Wolf1 Map1`, com teto,
chão e paredes do `VSWAP` usando a paleta original.

## Desenvolvimento orientado por especificações

O projeto usa SDD (*Spec-Driven Development*). Antes de implementar um marco:

1. crie ou atualize sua especificação em `docs/specs/`;
2. defina escopo, não objetivos e critérios de aceitação observáveis;
3. implemente apenas o necessário para satisfazer esses critérios;
4. execute as verificações e registre o estado na documentação.

A especificação vigente do port é
[`docs/specs/0001-linux-port.md`](docs/specs/0001-linux-port.md). Toda alteração
funcional deve manter sincronizados este README, a especificação aplicável e as
orientações de contribuição em `AGENTS.md` quando estrutura, comandos ou fluxo
de trabalho forem afetados.

## Licença e código original

`README.rst` e `WOLFSRC/README/LICENSE.DOC` registram a distribuição e a licença
originais. `README.rst` é histórico e não deve ser editado pelo port.
