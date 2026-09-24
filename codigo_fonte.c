#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE 2048
#define MAX_KEYS 1000000 // Limite arbitrário para o tamanho do vetor em memória



// Busca Sequencial Iterativa
int busca_sequencial_iterativa(char **vetor, int n, const char *chave) {
    for (int i = 0; i < n; i++) {
        if (strcmp(vetor[i], chave) == 0) {
            return 1; 
        }
    }
    return 0;
}

int busca_sequencial_recursiva(char **vetor, int n, const char *chave, int indice) {
    if (indice >= n) return 0;
    if (strcmp(vetor[indice], chave) == 0) return 1;
    return busca_sequencial_recursiva(vetor, n, chave, indice + 1);
}



// Função auxiliar para mesclar (Merge)
void merge(char **vetor, int esq, int meio, int dir) {
    int n1 = meio - esq + 1;
    int n2 = dir - meio;

    char **L = (char **)malloc(n1 * sizeof(char *));
    char **R = (char **)malloc(n2 * sizeof(char *));

    for (int i = 0; i < n1; i++) L[i] = vetor[esq + i];
    for (int j = 0; j < n2; j++) R[j] = vetor[meio + 1 + j];

    int i = 0, j = 0, k = esq;
    while (i < n1 && j < n2) {
        if (strcmp(L[i], R[j]) <= 0) {
            vetor[k] = L[i];
            i++;
        } else {
            vetor[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        vetor[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        vetor[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// MergeSort Recursivo
void mergesort_recursivo(char **vetor, int esq, int dir) {
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        mergesort_recursivo(vetor, esq, meio);
        mergesort_recursivo(vetor, meio + 1, dir);
        merge(vetor, esq, meio, dir);
    }
}

// MergeSort Iterativo
// Função utilitária para obter o mínimo entre dois inteiros
int min(int x, int y) { return (x < y) ? x : y; }

void mergesort_iterativo(char **vetor, int n) {
    int tam_atual;
    int esq;
    for (tam_atual = 1; tam_atual <= n - 1; tam_atual = 2 * tam_atual) {
        for (esq = 0; esq < n - 1; esq += 2 * tam_atual) {
            int meio = min(esq + tam_atual - 1, n - 1);
            int dir = min(esq + 2 * tam_atual - 1, n - 1);
            merge(vetor, esq, meio, dir);
        }
    }
}

// Busca Binária Iterativa
int busca_binaria_iterativa(char **vetor, int n, const char *chave) {
    int esq = 0, dir = n - 1;
    while (esq <= dir) {
        int meio = esq + (dir - esq) / 2;
        int comp = strcmp(vetor[meio], chave);
        
        if (comp == 0) return 1; // Encontrado
        if (comp < 0) esq = meio + 1;
        else dir = meio - 1;
    }
    return 0; // Não encontrado
}

// Busca Binária Recursiva
int busca_binaria_recursiva(char **vetor, int esq, int dir, const char *chave) {
    if (dir >= esq) {
        int meio = esq + (dir - esq) / 2;
        int comp = strcmp(vetor[meio], chave);
        
        if (comp == 0) return 1;
        if (comp > 0) return busca_binaria_recursiva(vetor, esq, meio - 1, chave);
        return busca_binaria_recursiva(vetor, meio + 1, dir, chave);
    }
    return 0;
}



// Extrai a chave (primeira coluna) de uma linha CSV
void extrair_chave(const char *linha, char *chave_out) {
    int i = 0;
    while (linha[i] != '\0' && linha[i] != ',' && linha[i] != '\n' && linha[i] != '\r') {
        chave_out[i] = linha[i];
        i++;
    }
    chave_out[i] = '\0';
}

// Carrega as chaves do arquivo de destino para a RAM
int carregar_chaves_destino(const char *nome_arquivo, char ***vetor_chaves) {
    FILE *file = fopen(nome_arquivo, "r");
    if (!file) return 0; // Arquivo pode não existir ainda

    *vetor_chaves = (char **)malloc(MAX_KEYS * sizeof(char *));
    char linha[MAX_LINE];
    int count = 0;
    char chave_tmp[MAX_LINE];

    // Pula o cabeçalho se houver (opcional, mas recomendado dependendo do arquivo)
    // fgets(linha, MAX_LINE, file);

    while (fgets(linha, MAX_LINE, file)) {
        extrair_chave(linha, chave_tmp);
        if (strlen(chave_tmp) > 0) {
            (*vetor_chaves)[count] = strdup(chave_tmp);
            count++;
        }
    }

    fclose(file);
    return count;
}




int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <arquivo_novos_registros.csv> <arquivo_destino.csv>\n", argv[0]);
        printf("Exemplo utilizando: ./programa obras-nao-pub-brasileiras-2026.csv destino.csv\n");
        return 1;
    }

    const char *arq_novos = argv[1];
    const char *arq_destino = argv[2];

    char **chaves_destino = NULL;
    int num_chaves = carregar_chaves_destino(arq_destino, &chaves_destino);
    
    printf("Carregadas %d chaves existentes do arquivo destino.\n", num_chaves);

    // Abre o arquivo de novos registros para leitura
    FILE *f_novos = fopen(arq_novos, "r");
    if (!f_novos) {
        printf("Erro ao abrir arquivo de novos registros: %s\n", arq_novos);
        return 1;
    }

    // Abre o arquivo destino para append
    FILE *f_destino = fopen(arq_destino, "a");
    if (!f_destino) {
        printf("Erro ao abrir arquivo destino para escrita.\n");
        fclose(f_novos);
        return 1;
    }

    // EXEMPLO: Escolha da estratégia de busca (Mude para testar outras abordagens)
    // Para Solução 2, precisamos ordenar primeiro:
    
    clock_t inicio_sort = clock();
    if (num_chaves > 0) {
        mergesort_iterativo(chaves_destino, num_chaves);
        // mergesort_recursivo(chaves_destino, 0, num_chaves - 1);
    }
    clock_t fim_sort = clock();
    double tempo_sort = ((double)(fim_sort - inicio_sort)) / CLOCKS_PER_SEC;
    printf("Tempo de ordenacao (MergeSort): %f segundos\n", tempo_sort);

    char linha[MAX_LINE];
    char chave_tmp[MAX_LINE];
    int registros_inseridos = 0;
    int registros_ignorados = 0;

    clock_t inicio_busca = clock();

    // Ler cada novo registro
    while (fgets(linha, MAX_LINE, f_novos)) {
        extrair_chave(linha, chave_tmp);
        if (strlen(chave_tmp) == 0) continue;

        int duplicado = 0;

        // --- SOLUÇÃO 1: Busca Sequencial (Descomente para testar) ---
        // duplicado = busca_sequencial_iterativa(chaves_destino, num_chaves, chave_tmp);
        // duplicado = busca_sequencial_recursiva(chaves_destino, num_chaves, chave_tmp, 0);

        // --- SOLUÇÃO 2: Busca Binária (Requer vetor ordenado) ---
        duplicado = busca_binaria_iterativa(chaves_destino, num_chaves, chave_tmp);
        // duplicado = busca_binaria_recursiva(chaves_destino, 0, num_chaves - 1, chave_tmp);

        if (!duplicado) {
            fputs(linha, f_destino);
            
            // ATENÇÃO: Para manter a consistência em tempo de execução contínua com busca binária,
            // a nova chave deveria ser inserida no vetor 'chaves_destino' mantendo a ordenação.
            // Para fins de demonstração acadêmica da complexidade, a inserção aqui foca no arquivo.
            
            registros_inseridos++;
        } else {
            registros_ignorados++;
        }
    }

    clock_t fim_busca = clock();
    double tempo_busca = ((double)(fim_busca - inicio_busca)) / CLOCKS_PER_SEC;

    printf("Processamento concluido.\n");
    printf("Registros inseridos: %d\n", registros_inseridos);
    printf("Registros ignorados (duplicados): %d\n", registros_ignorados);
    printf("Tempo de processamento de busca: %f segundos\n", tempo_busca);

    fclose(f_novos);
    fclose(f_destino);

    // Liberar memória
    for (int i = 0; i < num_chaves; i++) {
        free(chaves_destino[i]);
    }
    if (chaves_destino) free(chaves_destino);

    return 0;
}
