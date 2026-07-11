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
- código histórico preservado em `WOLFSRC/`.

O jogo original ainda não é jogável. O próximo marco é carregar e validar os
arquivos de dados originais.

## Compilar no Linux

Requisitos: compilador C, CMake 3.16+ e headers de desenvolvimento do SDL2.

```sh
cmake -S . -B build
cmake --build build
./build/wolf3d
```

Verificação rápida:

```sh
./build/wolf3d --check
git diff --check
```

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
