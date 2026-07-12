# Dados shareware

Este diretório preserva o pacote **Wolfenstein 3D Shareware v1.4** usado para
validar o port.

## Proveniência

- origem: [Internet Archive — Wolfenstein 3D (Shareware, v1.4)](https://archive.org/details/wolf3dsw);
- publicação indicada: id Software, 1992;
- arquivo original: `wolf3dsw.zip`;
- SHA-256: `76ee5e73e7d6341aefff620989bb5f828e9d295982afd5415b62dee7fe54eb64`.

O ZIP é mantido intacto. `shareware-v1.4/` contém somente os oito arquivos
`.WL1` consumidos pelo port. O `WOLF3D.EXE` DOS continua disponível dentro do
arquivo original, mas não é usado nem extraído pelo build Linux.

Execute a validação visual com:

```sh
./build/wolf3d --data data/shareware-v1.4
```

Não substitua esses arquivos por dados da edição comercial `.WL6`.

Dados comerciais próprios podem ser mantidos localmente em `full-v1.4/`, que é
ignorado pelo Git. Execute-os separadamente com
`./build/wolf3d --data data/full-v1.4`; eles não fazem parte da validação M5.
