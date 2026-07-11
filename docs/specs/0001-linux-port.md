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
- [ ] M5 — campanha shareware com atores, interface, áudio PCM e fluxo completo.
  - [x] M5A — fundação portátil da simulação.
  - [x] M5B — catálogo VSWAP e sprites estáticos.
  - [ ] M5C — objetos estáticos, bloqueios e pickups.
  - [ ] M5D — portas móveis e interação.

## Critérios de aceitação atuais

1. `cmake -S . -B build && cmake --build build` termina sem avisos.
2. `./build/wolf3d --check` imprime `SDL2 OK` e retorna zero.
3. `./build/wolf3d` abre o framebuffer e encerra por `Esc` ou pelo fechamento da janela.
4. `README.rst` e os arquivos históricos em `WOLFSRC/` permanecem intactos.

### Bootstrap do ambiente Linux

- fornecer um script versionado para instalar a toolchain C, CMake e os headers
  do SDL2 em distribuições Debian e Ubuntu;
- detectar a distribuição antes de executar o gerenciador de pacotes;
- permitir verificar as dependências sem modificar o sistema;
- falhar com uma mensagem clara em sistemas não suportados ou quando uma
  dependência continuar ausente.

Critérios de aceitação do bootstrap:

1. [x] `./scripts/bootstrap.sh --check` informa todas as dependências ausentes;
2. [x] `./scripts/bootstrap.sh --install` usa `apt-get` somente em Debian/Ubuntu;
3. [x] após a instalação, compilador C, CMake e SDL2 são encontrados;
4. [x] o procedimento e os comandos seguintes estão documentados no `README.md`.

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

### M5A — contrato da fundação portátil da simulação

- representar mapa, jogador e estado determinístico do RNG em um `GameState`;
- representar ações por frame em um `PlayerCommand` independente de SDL2;
- manter movimento, rotação por teclado e mouse e colisão observavelmente iguais;
- carregar um mapa por índice, preparando a progressão sem implementá-la;
- compilar os módulos portáveis uma única vez em uma biblioteca usada pelo jogo e testes.

Critérios de aceitação do M5A:

1. [x] a simulação do jogador pode ser atualizada por teste sem inicializar SDL2;
2. [x] duas sequências com a mesma semente produzem os mesmos valores aleatórios;
3. [x] índices de mapa ausentes ou fora do cabeçalho são rejeitados com segurança;
4. [x] o mapa 0 shareware continua carregando e jogável com os controles do M4;
5. [x] build, `--check` e todos os testes anteriores continuam passando.

Portas, sprites, objetos, inventário, atores, combate, HUD, áudio e progressão
permanecem fora do M5A. O alvo posterior do M5 é a campanha `WL1`; suporte
validado a `WL6`, música AdLib/OPL, saves e menus completos não fazem parte deste
marco.

### M5B — contrato do catálogo VSWAP e sprites estáticos

- ler e validar o diretório completo de chunks do `VSWAP` uma única vez;
- preservar os limites entre paredes, sprites e sons para os próximos marcos;
- decodificar sprites compilados para pixels indexados e máscara transparente;
- rejeitar limites, offsets, posts e fontes de pixels fora da página;
- manter profundidade por coluna para ocultar sprites atrás de paredes;
- projetar os objetos estáticos 23–70 do plano de objetos no cenário 3D.

Critérios de aceitação do M5B:

1. [x] testes sintéticos comprovam posts, transparência e orientação do sprite;
2. [x] páginas truncadas e comandos fora dos limites são rejeitados;
3. [x] todas as páginas de sprite do conjunto shareware são decodificadas;
4. [x] sprites atrás de paredes são ocultados pelo depth buffer;
5. [x] objetos estáticos do primeiro mapa são validados visualmente;
6. [x] build, `--check` e todos os testes anteriores continuam passando.

Coleta, bloqueio por objetos, portas móveis, atores e animações permanecem fora
do M5B.

### M5C — contrato de objetos estáticos, bloqueios e pickups

- instanciar códigos 23–70 do plano de objetos como estado explícito do nível;
- preservar o sprite e classificar decoração, bloqueio e item coletável;
- impedir que o jogador atravesse objetos marcados como bloqueantes;
- coletar itens ao entrar em sua célula e removê-los da simulação e renderização;
- manter vida, munição, armas, chaves, vidas, pontuação e tesouros no estado;
- respeitar limites e condições do original, incluindo itens não consumidos
  quando vida ou munição já estiverem no máximo.

Critérios de aceitação do M5C:

1. [x] os 48 tipos estáticos WL1 são classificados e instanciados;
2. [x] objetos bloqueantes impedem movimento e decorações não impedem;
3. [x] comida, kit médico, munição, armas, chaves e tesouros aplicam seus efeitos;
4. [x] itens inaplicáveis permanecem no mapa e valores respeitam seus limites;
5. [x] itens coletados deixam de ser renderizados;
6. [ ] comportamento é validado manualmente no primeiro mapa;
7. [x] build, `--check` e todos os testes anteriores continuam passando.

HUD, sons de coleta, portas, atores e combate permanecem fora do M5C.

### M5D — contrato de portas móveis e interação

- instanciar tiles 90–101 com orientação, fechadura e estado explícitos;
- operar com `Space` a porta na célula atual ou cardinal adjacente, por borda de pressão;
- exigir as chaves correspondentes sem consumi-las;
- abrir e fechar progressivamente, aguardar aberta e fechar automaticamente;
- usar a mesma abertura parcial no raycasting e na colisão do jogador;
- testar o plano da porta mesmo quando a câmera já estiver dentro da célula;
- impedir fechamento sobre o jogador e reabrir quando houver obstrução;
- selecionar texturas normais, trancadas e de elevador por orientação.

Critérios de aceitação do M5D:

1. [x] testes cobrem tiles 90–101, orientação, tipo e estado inicial;
2. [x] interação adjacente, inversão e fechaduras são determinísticas;
3. [x] abertura, espera, fechamento e obstrução respeitam o tempo do original;
4. [x] raycast atravessa a fração aberta e atinge a fração sólida;
5. [x] colisão permite passagem somente quando houver espaço para o jogador;
6. [x] build, testes e smoke checks continuam passando;
7. [ ] portas, bloqueios e pickups são validados manualmente no mapa shareware.

Sons, atores, conectividade de áreas, pushwalls e conclusão por elevador
permanecem fora do M5D.

### M5E — contrato da fundação de atores

- instanciar guardas comuns dos códigos 108–115 do plano de objetos;
- preservar posição, direção cardinal e origem parada ou em patrulha;
- renderizar guardas pelo mesmo pipeline de sprites, profundidade e oclusão dos
  objetos estáticos;
- selecionar uma das oito rotações do sprite conforme a direção do guarda e a
  posição do jogador;
- ignorar com segurança códigos de dificuldade e classes ainda não suportadas.

Critérios de aceitação do M5E:

1. [x] testes sintéticos cobrem os oito códigos, direções e origem do guarda;
2. [x] guardas são ocultados por paredes como os demais sprites;
3. [x] mudar a direção do guarda seleciona outra rotação visível;
4. [x] guardas do primeiro mapa shareware são validados visualmente;
5. [x] build, testes e smoke checks continuam passando.

Movimento, animação, percepção, perseguição, ataques, dano, mortes, outras
classes de atores, HUD e áudio permanecem fora do M5E. Um estado direto para o
único ator suportado evita antecipar uma máquina de estados genérica.

### M5F — contrato de patrulha dos guardas

- mover guardas de patrulha nas quatro direções cardinais na velocidade original;
- mudar a direção em células com setas cardinais do plano de objetos;
- interromper o movimento diante de paredes, objetos bloqueantes e portas que
  ainda não estejam totalmente abertas;
- animar os quatro quadros de caminhada nos tempos do original;
- manter guardas inicialmente parados sem movimento ou animação.

Critérios de aceitação do M5F:

1. [x] patrulha avança de forma determinística a 0,546875 célula por segundo;
2. [x] setas cardinais alteram a direção e obstáculos interrompem o movimento;
3. [x] os quatro quadros de caminhada seguem o ciclo original de 80 tics;
4. [ ] patrulhas são validadas visualmente no primeiro mapa shareware;
5. [x] build, testes e smoke checks continuam passando.
6. [x] rotações laterais correspondem à direção cardinal do movimento.

Setas diagonais, abertura de portas por atores, colisão entre atores, percepção,
perseguição e combate permanecem fora do M5F.

## Regra de documentação

Cada mudança funcional deve atualizar seu marco e critérios nesta especificação,
o estado/comandos no `README.md` e, quando afetadas, as instruções do
`AGENTS.md`. A revisão deve declarar explicitamente quando um desses arquivos
foi verificado e não exigiu alteração.
