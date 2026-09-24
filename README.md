# Laboratório 4 — Inserção sem duplicidade em CSV

Programa em C que insere os registros de um CSV ao final de outro, sem duplicar a coluna-chave.

- **Base:** `obras-nao-pub-brasileiras-2026.csv` (ANCINE / dados.gov.br): 2.015 registros, 18 colunas, separador `;`.
- **Chave de unicidade:** coluna `CPB` (2ª coluna): valores únicos e fora de ordem no arquivo.

## Compilação e execução

```bash
cc -O0 -o programa codigo_fonte.c
./programa <novos_registros.csv> <destino.csv> [metodo]
```

| metodo | Estratégia |
|---|---|
| 1 | Solução 1: busca sequencial iterativa |
| 2 | Solução 1: busca sequencial recursiva |
| 3 | Solução 2: MergeSort iterativo + busca binária iterativa (padrão) |
| 4 | Solução 2: MergeSort recursivo + busca binária recursiva |

Exemplo com os arquivos de teste (300 registros duplicados e 200 inéditos):

```bash
cp obras-nao-pub-brasileiras-2026.csv destino.csv
./programa novos_registros.csv destino.csv 3
```

## Experimentos

`benchmark.c` inclui `codigo_fonte.c` (mede exatamente as mesmas funções) e conta as comparações `strcmp`.

```bash
cc -O0 -o benchmark benchmark.c
./benchmark all > resultados/resultados.csv   # ~1 min
./benchmark pilha 200000                      # teste de estouro de pilha da busca sequencial recursiva
pip3 install matplotlib
python3 gerar_graficos.py                     # gera graficos/*.png e resultados/ajustes.csv
```

`resultados/pilha.csv` contém o maior N suportado pela busca sequencial recursiva para cada tamanho de pilha
(`ulimit -s`), obtido por bissecção com `./benchmark pilha N`.

Compile com `-O0`: com `-O2` o compilador elimina a recursão de cauda da busca sequencial recursiva, e o
estouro de pilha deixa de ocorrer.
