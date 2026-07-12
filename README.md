# Wolfenstein 3D — port educacional

Port progressivo do código-fonte original de Wolfenstein 3D para plataformas
modernas. O objetivo inicial é aprendizado: compreender o motor DOS de 1992 e
substituir, em etapas verificáveis, suas dependências de Borland C++, VGA,
interrupções e assembly de 16 bits.

O Linux é a primeira plataforma. O suporte a Windows virá depois que o núcleo
portável estiver funcional.

O suporte Windows descrito abaixo sucede o marco inicial Linux agora que o
núcleo portátil está funcional.

## Estado atual

- build nativo em C com CMake;
- build configurado para Linux e validado no Windows x64 com CMake, MSVC e vcpkg;
- janela SDL2 e framebuffer de 320×200 escalado;
- loop de eventos com saída por `Esc`;
- smoke check executável sem interface gráfica;
- validação dos conjuntos de dados shareware (`WL1`) e completo (`WL6`);
- leitor seguro de `VSWAP` e teste automatizado com arquivo sintético;
- paleta VGA original extraída do payload OMF e renderização de paredes;
- renderização validada com os dados shareware v1.4 incluídos em `data/`;
- descompressão Carmack/RLEW e carregamento dos dois planos do primeiro mapa;
- visão superior estática com paredes, portas e orientação do jogador;
- carregamento das 106 páginas de parede, incluindo páginas esparsas;
- raycasting DDA com paredes texturizadas e câmera inicial estática;
- movimento e rotação independentes da taxa de quadros;
- colisão com raio contra paredes, portas fechadas e limites do mapa;
- controle horizontal da câmera com mouse em modo relativo;
- estado portátil de jogo e comandos de entrada independentes de SDL2;
- carregamento de mapas por índice e RNG determinístico para a simulação;
- módulos portáveis reunidos na biblioteca `wolf3d_core`;
- catálogo completo do `VSWAP` e decodificação segura de sprites compilados;
- objetos estáticos renderizados com transparência e oclusão por paredes;
- objetos sólidos participam da colisão e pickups atualizam o estado do jogador;
- vida, munição, armas, chaves, vidas, score e tesouros mantidos na simulação;
- portas móveis com abertura parcial, fechaduras, colisão e fechamento automático;
- guardas comuns patrulham por células, perseguem e atravessam portas normais;
- disparos atingem guardas, consomem munição e deixam cadáveres;
- pistola inicial exibida em primeira pessoa com animação de disparo;
- guardas próximos atiram e causam dano com feedback visual;
- morte escurece a tela e permite reiniciar o mapa consumindo uma vida;
- guardas mortos deixam pentes de munição coletáveis;
- HUD compacto mostra vida, munição, vidas e pontuação;
- faca funciona em curta distância e é selecionada quando a munição termina;
- interruptor do elevador conclui o nível atual;
- elevador carrega o mapa seguinte preservando o estado da campanha;
- oficiais usam sprites, resistência, velocidade e pontuação próprias;
- cães patrulham, perseguem, mordem e usam animações próprias;
- código histórico preservado em `WOLFSRC/`.

O primeiro mapa pode ser explorado com teclado e mouse. Portas abrem com `Space`,
fecham automaticamente e respeitam as chaves coletadas. Guardas patrulham,
percebem, perseguem e atiram no jogador, abrindo portas normais quando necessário;
ainda não há HUD, morte do jogador ou áudio. Objetos sólidos bloqueiam movimento;
itens aplicáveis são coletados e desaparecem do cenário.

## Compilar no Linux

Requisitos: compilador C, CMake 3.16+ e headers de desenvolvimento do SDL2.

Em Debian ou Ubuntu, instale e valide essas dependências com:

```sh
./scripts/bootstrap.sh --install
./scripts/bootstrap.sh --check
```

O modo `--install` usa `apt-get` e solicita `sudo` quando necessário. O modo
`--check` apenas informa dependências ausentes e não modifica o sistema. Em
outras distribuições, instale os pacotes equivalentes pelo gerenciador local.

```sh
cmake -S . -B build
cmake --build build
./build/wolf3d --data data/shareware-v1.4
```

Controles: `W`/`↑` avança, `S`/`↓` recua, `A`/`←` e `D`/`→` giram, o mouse
controla a direção horizontal, `Space` opera a porta à frente, `Ctrl` ou botão
esquerdo dispara, `1–4` seleciona armas e `Esc` encerra. Metralhadora e chaingun
disparam continuamente enquanto o controle permanece pressionado.
Após morrer, `Space` reinicia o mapa enquanto ainda houver vidas.

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

## Compilar no Windows

Requisitos: Visual Studio 2022 com o workload **Desktop development with C++**,
CMake 3.16+ e vcpkg. No PowerShell, instale SDL2 para x64 e configure usando o
toolchain do vcpkg (substitua `C:\src\vcpkg` pelo caminho local):

```powershell
C:\src\vcpkg\vcpkg.exe install sdl2:x64-windows
cmake -S . -B build-windows -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build-windows --config Release
```

Execute a partir da raiz do repositório:

```powershell
.\build-windows\Release\wolf3d.exe --check
.\build-windows\Release\wolf3d.exe --data .\data\shareware-v1.4
ctest --test-dir build-windows -C Release --output-on-failure
git diff --check
```

O vcpkg copia as DLLs necessárias do SDL2 para o diretório do executável durante
o build. O contrato desse build está em
[`docs/specs/0002-windows-build.md`](docs/specs/0002-windows-build.md).

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
