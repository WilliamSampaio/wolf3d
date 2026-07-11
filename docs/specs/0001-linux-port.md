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
- [x] M3 — carregar mapa e renderizar uma cena estática.
- [x] M4 — movimento, colisão e entrada jogável.
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

### M3A — contrato da visão superior

- ler a tag RLEW e o offset do mapa 0 em `MAPHEAD`;
- ler o cabeçalho e os dois planos 64×64 correspondentes em `GAMEMAPS`;
- descomprimir cada plano com Carmack Expand seguido de RLEW Expand;
- rejeitar entradas truncadas, referências inválidas e saídas fora dos limites;
- localizar o único início do jogador pelos códigos 19–22 do plano de objetos;
- mostrar paredes, portas e jogador numa visão superior estática.

Critérios de aceitação do M3A:

1. [x] testes sintéticos cobrem literais e referências Carmack, sequências RLEW e erros;
2. [x] os dois planos reais do mapa 0 resultam em exatamente 64×64 células;
3. [x] o mapa real contém exatamente um início de jogador válido;
4. [x] a janela exibe uma planta reconhecível do primeiro nível e sua orientação;
5. [x] testes de `VSWAP`, build e `--check` continuam passando.

O conjunto shareware carregou `Wolf1 Map1`, localizou o jogador em `(29,57)` e
a visão superior foi validada manualmente.

Raycasting, movimento, colisões, atores, sprites e áudio não fazem parte do M3A.

### M3B — contrato da cena 3D estática

- carregar todas as páginas de parede anteriores aos sprites em `VSWAP`;
- lançar um raio DDA por coluna a partir da posição e orientação do jogador;
- limitar os raios ao mapa e calcular distância perpendicular, lado e coluna atingidos;
- mapear paredes 1–63 para as texturas horizontais/verticais do jogo;
- desenhar teto, chão e paredes texturizadas com a paleta original;
- manter a câmera estática e encerrar somente por `Esc` ou fechamento da janela.

Critérios de aceitação do M3B:

1. [x] teste sintético comprova célula, lado, distância e coordenada da textura;
2. [x] raio sem parede termina com segurança nos limites do mapa;
3. [x] as 106 páginas de parede, incluindo páginas esparsas, são validadas;
4. [x] a janela exibe uma cena 3D reconhecível no início de `Wolf1 Map1`;
5. [x] build, `--check` e todos os testes anteriores continuam passando.

Movimento, colisões, interação com portas, sprites, atores, HUD e áudio permanecem fora do M3B.

A cena 3D estática foi validada manualmente com o conjunto shareware v1.4.

### M4A — contrato de movimento e colisão

- iniciar o jogador no centro da célula e orientação definidas pelo mapa;
- avançar/recuar com `W`/`S` ou setas para cima/baixo;
- girar com `A`/`D` ou setas para esquerda/direita;
- aplicar velocidades por segundo usando o tempo real entre frames, limitado a 50 ms;
- bloquear paredes, portas fechadas e limites usando um raio de colisão;
- resolver X e Y separadamente para permitir deslizamento junto às paredes;
- recalcular e apresentar a cena 3D a cada frame.

Critérios de aceitação do M4A:

1. [x] teste sintético comprova avanço em área livre e rotação;
2. [x] jogador não entra em parede, porta fechada ou fora do mapa;
3. [x] colisão diagonal permite deslizamento por um eixo livre;
4. [x] controles respondem de forma estável em diferentes taxas de quadro;
5. [x] build, `--check` e todos os testes anteriores continuam passando;
6. [x] movimentação e colisão são validadas manualmente no mapa shareware.

Abrir portas, corrida, tiros, sprites, atores, HUD e áudio permanecem fora do M4A.

### M4B — contrato do mouse

- capturar o mouse em modo relativo enquanto a janela estiver ativa;
- converter apenas o deslocamento horizontal em rotação imediata;
- usar sensibilidade inicial fixa de `0.0025` radianos por pixel;
- normalizar o ângulo após movimentos grandes ou repetidos;
- manter teclado, colisão e `Esc` inalterados.

Critérios de aceitação do M4B:

1. [x] teste comprova rotação positiva, negativa e normalização do ângulo;
2. [x] eixo vertical do mouse não altera o jogador;
3. [x] build, `--check` e todos os testes anteriores continuam passando;
4. [x] controle do olhar com mouse é validado manualmente.

Configuração de sensibilidade, movimento vertical e menus permanecem fora do M4B.

O movimento, a colisão, o teclado e o mouse relativo foram validados manualmente
no mapa shareware.

## Regra de documentação

Cada mudança funcional deve atualizar seu marco e critérios nesta especificação,
o estado/comandos no `README.md` e, quando afetadas, as instruções do
`AGENTS.md`. A revisão deve declarar explicitamente quando um desses arquivos
foi verificado e não exigiu alteração.
