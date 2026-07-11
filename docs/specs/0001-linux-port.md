# 0001 — Port inicial para Linux

**Status:** Em andamento  
**Plataforma:** Linux  
**Referência:** código DOS em `WOLFSRC/`

## Problema

O código original depende de Borland C++ 3.x, memória segmentada, acesso direto
à VGA, interrupções DOS e assembly de 16 bits. Esses recursos não estão
disponíveis em um executável Linux moderno.

## Objetivo

Produzir, de forma incremental e educacional, um executável Linux nativo que
preserve o comportamento do jogo, usando C, CMake e SDL2 como camada de
plataforma.

## Escopo inicial

- configurar um build nativo e reproduzível;
- fornecer vídeo, entrada, tempo e áudio por APIs portáveis;
- carregar os dados de uma instalação legítima do jogo;
- portar o motor e a lógica em marcos pequenos e verificáveis;
- preservar `WOLFSRC/` como referência histórica sempre que possível.

## Não objetivos

- suportar Windows antes do port Linux funcional;
- reescrever o jogo em outra linguagem;
- modernizar gráficos, conteúdo ou regras do jogo;
- distribuir dados comerciais do Wolfenstein 3D.

## Marcos

- [x] M0 — CMake, SDL2, framebuffer 320×200 e loop de eventos.
- [x] M1 — localizar, abrir e validar arquivos de dados originais.
- [x] M2 — carregar paleta e primeiro recurso gráfico.
- [ ] M3 — carregar mapa e renderizar uma cena estática.
- [ ] M4 — movimento, colisão e entrada jogável.
- [ ] M5 — atores, interface, áudio e fluxo completo do jogo.

## Critérios de aceitação atuais

1. `cmake -S . -B build && cmake --build build` termina sem avisos.
2. `./build/wolf3d --check` imprime `SDL2 OK` e retorna zero.
3. `./build/wolf3d` abre o framebuffer e encerra por `Esc` ou pelo fechamento da janela.
4. `README.rst` e os arquivos históricos em `WOLFSRC/` permanecem intactos.

### M1 — contrato dos dados

- `--data DIRETÓRIO` seleciona explicitamente a pasta dos dados;
- as edições shareware (`WL1`) e completa (`WL6`) são reconhecidas pelo sufixo;
- `VSWAP`, `GAMEMAPS`, `MAPHEAD`, `VGADICT`, `VGAHEAD`, `VGAGRAPH`,
  `AUDIOHED` e `AUDIOT` devem existir e não estar vazios;
- dados ausentes ou incompletos encerram o programa com erro antes de iniciar vídeo.

Critérios de aceitação do M1:

1. [x] um diretório temporário com os oito arquivos `.WL1` não vazios é aceito;
2. [x] a ausência de arquivos faz a validação retornar erro;
3. [x] argumento ausente ou desconhecido exibe o uso e retorna erro;
4. [x] `--check` continua validando SDL2 sem exigir dados comerciais.

### M2 — contrato do primeiro recurso gráfico

- ler o cabeçalho de `VSWAP` sem depender do tamanho dos tipos da plataforma;
- interpretar contagens, offsets e tamanhos explicitamente como little-endian;
- rejeitar cabeçalhos truncados, intervalos inválidos e primeira parede diferente de 64×64;
- converter a primeira parede, armazenada por colunas, em pixels organizados por linhas;
- aplicar a paleta VGA original de 768 bytes preservada em `GAMEPAL.OBJ`;
- exibir a parede repetida no framebuffer no lugar do gradiente provisório.

Critérios de aceitação do M2:

1. [x] um teste sintético lê uma página de parede válida e comprova sua orientação;
2. [x] o mesmo teste rejeita um `VSWAP` truncado;
3. [x] o build e `--check` continuam funcionando sem dados comerciais;
4. [x] com dados shareware v1.4, a janela exibe pixels da primeira textura usando a paleta original.

Validação visual registrada com `data/shareware-v1.4`, obtido do item
[`wolf3dsw`](https://archive.org/details/wolf3dsw) do Internet Archive. O pacote
original é preservado em `data/wolf3dsw.zip` com SHA-256
`76ee5e73e7d6341aefff620989bb5f828e9d295982afd5415b62dee7fe54eb64`.

## Regra de documentação

Cada mudança funcional deve atualizar seu marco e critérios nesta especificação,
o estado/comandos no `README.md` e, quando afetadas, as instruções do
`AGENTS.md`. A revisão deve declarar explicitamente quando um desses arquivos
foi verificado e não exigiu alteração.
