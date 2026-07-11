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
- [ ] M1 — localizar, abrir e validar arquivos de dados originais.
- [ ] M2 — carregar paleta e primeiro recurso gráfico.
- [ ] M3 — carregar mapa e renderizar uma cena estática.
- [ ] M4 — movimento, colisão e entrada jogável.
- [ ] M5 — atores, interface, áudio e fluxo completo do jogo.

## Critérios de aceitação atuais

1. `cmake -S . -B build && cmake --build build` termina sem avisos.
2. `./build/wolf3d --check` imprime `SDL2 OK` e retorna zero.
3. `./build/wolf3d` abre o framebuffer e encerra por `Esc` ou pelo fechamento da janela.
4. `README.rst` e os arquivos históricos em `WOLFSRC/` permanecem intactos.

## Regra de documentação

Cada mudança funcional deve atualizar seu marco e critérios nesta especificação,
o estado/comandos no `README.md` e, quando afetadas, as instruções do
`AGENTS.md`. A revisão deve declarar explicitamente quando um desses arquivos
foi verificado e não exigiu alteração.
