// Benchmark experimental das Soluções 1 e 2.
// Reutiliza as MESMAS funções de codigo_fonte.c (incluído abaixo) e conta
// o número de comparações de chaves (chamadas a strcmp), que é a operação
// básica usada nas demonstrações de complexidade do relatório.
//
// Compilação: cc -O0 -o benchmark benchmark.c
// Uso:        ./benchmark all            -> gera resultados em CSV na saída padrão
//             ./benchmark pilha <N>      -> executa só a busca sequencial recursiva
//                                           com N chaves (teste de estouro de pilha)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long long comparacoes = 0;
static int strcmp_contado(const char *a, const char *b) {
    comparacoes++;
    return strcmp(a, b);
}

#define strcmp strcmp_contado
#define main main_programa
#include "codigo_fonte.c"
#undef main
#undef strcmp

static double agora(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

// Gera chaves distintas no formato do CPB (ex.: B2612345678901).
// i * 2654435761 mod 10^11 é injetivo porque o multiplicador é coprimo com 10.
static char *gerar_chave(long long i) {
    char buf[32];
    unsigned long long v = ((unsigned long long)i * 2654435761ULL) % 100000000000ULL;
    snprintf(buf, sizeof buf, "B26%011llu", v);
    return strdup(buf);
}

static char **gerar_vetor(int n, long long base) {
    char **v = malloc(n * sizeof(char *));
    for (int i = 0; i < n; i++) v[i] = gerar_chave(base + i);
    return v;
}

static void embaralhar(char **v, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char *t = v[i]; v[i] = v[j]; v[j] = t;
    }
}

static void liberar(char **v, int n) {
    for (int i = 0; i < n; i++) free(v[i]);
    free(v);
}

// Consultas: metade presente no vetor, metade ausente (pior caso).
static char **gerar_consultas(char **vet, int n, int m) {
    char **q = malloc(m * sizeof(char *));
    for (int i = 0; i < m; i++)
        q[i] = (i % 2 == 0) ? strdup(vet[rand() % n]) : gerar_chave(10000000LL + i);
    return q;
}

typedef int (*busca_fn)(char **, int, const char *);
static int bin_rec_wrapper(char **v, int n, const char *k) { return busca_binaria_recursiva(v, 0, n - 1, k); }
static int seq_rec_wrapper(char **v, int n, const char *k) { return busca_sequencial_recursiva(v, n, k, 0); }

// Mede M buscas; imprime comparações médias (presentes/ausentes), máximo e tempo por busca.
static void medir_busca(const char *nome, busca_fn f, char **vet, int n, char **q, int m, int repeticoes) {
    long long comp_pres = 0, comp_aus = 0, comp_max = 0;
    for (int i = 0; i < m; i++) {
        comparacoes = 0;
        f(vet, n, q[i]);
        if (i % 2 == 0) comp_pres += comparacoes; else comp_aus += comparacoes;
        if (comparacoes > comp_max) comp_max = comparacoes;
    }
    double t0 = agora();
    for (int r = 0; r < repeticoes; r++)
        for (int i = 0; i < m; i++) f(vet, n, q[i]);
    double t = (agora() - t0) / ((double)repeticoes * m);
    printf("%s,%d,%d,%.3f,%.3f,%lld,%.9e\n", nome, n, m,
           comp_pres / (double)((m + 1) / 2), comp_aus / (double)(m / 2), comp_max, t);
    fflush(stdout);
}

typedef void (*sort_fn)(char **, int);
static void ms_rec_wrapper(char **v, int n) { mergesort_recursivo(v, 0, n - 1); }

static void medir_sort(const char *nome, sort_fn f, int n, int repeticoes) {
    char **orig = gerar_vetor(n, 0);
    embaralhar(orig, n);
    char **v = malloc(n * sizeof(char *));
    double total = 0;
    long long comp = 0;
    for (int r = 0; r < repeticoes; r++) {
        memcpy(v, orig, n * sizeof(char *));
        comparacoes = 0;
        double t0 = agora();
        f(v, n);
        total += agora() - t0;
        comp = comparacoes;
    }
    printf("%s,%d,0,%lld,0,0,%.9e\n", nome, n, comp, total / repeticoes);
    fflush(stdout);
    free(v);
    liberar(orig, n);
}

// Execução completa: N chaves existentes, M novos registros (metade duplicada).
static void medir_total(int n, int m) {
    char **base = gerar_vetor(n, 0);
    embaralhar(base, n);
    char **q = gerar_consultas(base, n, m);
    char **v = malloc(n * sizeof(char *));

    // Solução 1 (iterativa)
    memcpy(v, base, n * sizeof(char *));
    comparacoes = 0;
    double t0 = agora();
    for (int i = 0; i < m; i++) busca_sequencial_iterativa(v, n, q[i]);
    double t1 = agora() - t0;
    long long c1 = comparacoes;

    // Solução 2 (MergeSort iterativo + busca binária iterativa)
    memcpy(v, base, n * sizeof(char *));
    comparacoes = 0;
    t0 = agora();
    mergesort_iterativo(v, n);
    for (int i = 0; i < m; i++) busca_binaria_iterativa(v, n, q[i]);
    double t2 = agora() - t0;
    long long c2 = comparacoes;

    printf("total_sol1,%d,%d,%lld,0,0,%.9e\n", n, m, c1, t1);
    printf("total_sol2,%d,%d,%lld,0,0,%.9e\n", n, m, c2, t2);
    fflush(stdout);
    free(v);
    liberar(q, m);
    liberar(base, n);
}

int main(int argc, char *argv[]) {
    srand(42);
    if (argc == 3 && strcmp(argv[1], "pilha") == 0) {
        int n = atoi(argv[2]);
        char **v = gerar_vetor(n, 0);
        char *ausente = gerar_chave(99999999999LL);
        int r = busca_sequencial_recursiva(v, n, ausente, 0);
        printf("ok %d %d\n", n, r);
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "all") != 0) {
        fprintf(stderr, "Uso: %s all | pilha <N>\n", argv[0]);
        return 1;
    }

    printf("algoritmo,n,m,comp_media_presente,comp_media_ausente,comp_max,tempo_s\n");

    // Buscas sequenciais (Solução 1)
    int ns_seq[] = {1000, 2000, 4000, 8000, 16000, 24000, 32000, 48000, 64000};
    for (unsigned k = 0; k < sizeof ns_seq / sizeof *ns_seq; k++) {
        int n = ns_seq[k];
        char **v = gerar_vetor(n, 0);
        embaralhar(v, n);
        char **q = gerar_consultas(v, n, 400);
        medir_busca("seq_iter", busca_sequencial_iterativa, v, n, q, 400, 3);
        medir_busca("seq_rec", seq_rec_wrapper, v, n, q, 400, 3);
        liberar(q, 400);
        liberar(v, n);
    }

    // Buscas binárias (Solução 2)
    int ns_bin[] = {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000, 256000, 512000, 1000000};
    for (unsigned k = 0; k < sizeof ns_bin / sizeof *ns_bin; k++) {
        int n = ns_bin[k];
        char **v = gerar_vetor(n, 0);
        embaralhar(v, n);
        mergesort_iterativo(v, n);
        char **q = gerar_consultas(v, n, 20000);
        medir_busca("bin_iter", busca_binaria_iterativa, v, n, q, 20000, 20);
        medir_busca("bin_rec", bin_rec_wrapper, v, n, q, 20000, 20);
        liberar(q, 20000);
        liberar(v, n);
    }

    // MergeSort (Solução 2)
    for (unsigned k = 0; k < sizeof ns_bin / sizeof *ns_bin; k++) {
        int n = ns_bin[k];
        int rep = n <= 64000 ? 20 : 3;
        medir_sort("ms_iter", mergesort_iterativo, n, rep);
        medir_sort("ms_rec", ms_rec_wrapper, n, rep);
    }

    // Solução 1 x Solução 2 (M = N)
    int ns_tot[] = {1000, 2000, 4000, 8000, 16000, 32000, 64000};
    for (unsigned k = 0; k < sizeof ns_tot / sizeof *ns_tot; k++)
        medir_total(ns_tot[k], ns_tot[k]);

    return 0;
}
