# Gera os gráficos do relatório a partir de resultados/resultados.csv
# Uso: python3 gerar_graficos.py
import csv, math, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker

AZUL, LARANJA, CINZA, TEXTO = "#2a78d6", "#eb6834", "#8a8984", "#52514e"
plt.rcParams.update({
    "font.size": 10, "axes.edgecolor": "#c9c8c2", "axes.labelcolor": TEXTO,
    "xtick.color": TEXTO, "ytick.color": TEXTO, "axes.grid": True,
    "grid.color": "#ecebe7", "grid.linewidth": 0.8, "axes.spines.top": False,
    "axes.spines.right": False, "legend.frameon": False, "lines.linewidth": 2,
    "savefig.dpi": 200, "savefig.bbox": "tight",
})
os.makedirs("graficos", exist_ok=True)

dados = {}
for r in csv.DictReader(open("resultados/resultados.csv")):
    dados.setdefault(r["algoritmo"], []).append(r)
col = lambda alg, c: [float(r[c]) for r in dados[alg]]

def ajuste(x, y):
    """Mínimos quadrados de y = a*x + b (forma dada pelas demonstrações) e R²."""
    k = len(x); mx = sum(x) / k; my = sum(y) / k
    a = sum((xi - mx) * (yi - my) for xi, yi in zip(x, y)) / sum((xi - mx) ** 2 for xi in x)
    b = my - a * mx
    ss_res = sum((yi - a * xi - b) ** 2 for xi, yi in zip(x, y))
    ss_tot = sum((yi - my) ** 2 for yi in y)
    return a, b, 1 - ss_res / ss_tot

resumo = []

# ---------- 1. Busca sequencial ----------
n = col("seq_iter", "n")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4))
ax1.plot(n, n, "--", color=CINZA, label="Teoria, pior caso: C(N) = N")
ax1.plot(n, col("seq_iter", "comp_media_ausente"), "o", color=AZUL, ms=7, label="Medido, chave ausente")
ax1.plot(n, [x / 2 for x in n], ":", color=CINZA, label="Teoria, caso médio: ≈ N/2")
ax1.plot(n, col("seq_iter", "comp_media_presente"), "s", color=LARANJA, ms=6, label="Medido, chave presente")
ax1.set(title="Comparações por busca", xlabel="N (chaves no destino)", ylabel="comparações (strcmp)")
ax1.legend(fontsize=8)
for alg, cor, mk, nome in [("seq_iter", AZUL, "o", "Iterativa"), ("seq_rec", LARANJA, "s", "Recursiva")]:
    t = [v * 1e6 for v in col(alg, "tempo_s")]
    a, b, r2 = ajuste(n, t)
    ax2.plot(n, t, mk, color=cor, ms=7, label=f"{nome}, medido")
    ax2.plot(n, [a * x + b for x in n], "-", color=cor, lw=1.2, alpha=.7, label=f"{nome}, ajuste {a*1e3:.2f}·10⁻³·N {b:+.2f}  (R²={r2:.4f})")
    resumo.append((f"Busca sequencial {nome.lower()}", "a·N+b", a, b, r2, "µs"))
ax2.set(title="Tempo por busca", xlabel="N (chaves no destino)", ylabel="tempo (µs)")
ax2.legend(fontsize=8)
fig.savefig("graficos/fig1_busca_sequencial.png"); plt.close(fig)

# ---------- 2. Busca binária ----------
n = col("bin_iter", "n")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4))
teo = [math.floor(math.log2(x)) + 1 for x in n]
ax1.plot(n, teo, "--", color=CINZA, drawstyle="steps-post", label="Teoria: ⌊log₂N⌋ + 1")
ax1.plot(n, col("bin_iter", "comp_max"), "o", color=AZUL, ms=8, label="Medido, máximo (iterativa)")
ax1.plot(n, col("bin_rec", "comp_max"), "x", color=LARANJA, ms=8, mew=2, label="Medido, máximo (recursiva)")
ax1.plot(n, col("bin_iter", "comp_media_ausente"), "s", color=CINZA, ms=4, label="Medido, média para chave ausente")
ax1.set_xscale("log")
ax1.set(title="Comparações por busca (= profundidade da recursão)", xlabel="N (escala log)", ylabel="comparações")
ax1.legend(fontsize=8)
lg = [math.log2(x) for x in n]
for alg, cor, mk, nome in [("bin_iter", AZUL, "o", "Iterativa"), ("bin_rec", LARANJA, "s", "Recursiva")]:
    t = [v * 1e9 for v in col(alg, "tempo_s")]
    a, b, r2 = ajuste(lg, t)
    ax2.plot(n, t, mk, color=cor, ms=7, label=f"{nome}, medido")
    ax2.plot(n, [a * x + b for x in lg], "-", color=cor, lw=1.2, alpha=.7, label=f"{nome}, ajuste {a:.1f}·log₂N {b:+.0f}  (R²={r2:.3f})")
    resumo.append((f"Busca binária {nome.lower()}", "a·log₂N+b", a, b, r2, "ns"))
ax2.set_xscale("log")
ax2.set(title="Tempo por busca", xlabel="N (escala log)", ylabel="tempo (ns)")
ax2.legend(fontsize=8)
fig.savefig("graficos/fig2_busca_binaria.png"); plt.close(fig)

# ---------- 3. MergeSort ----------
# Razões medido / (N·log₂N): se a complexidade é Θ(N log N), a razão tende a uma constante.
n = col("ms_iter", "n")
nlg = [x * math.log2(x) for x in n]
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4))
cota = [x * math.ceil(math.log2(x)) for x in n]
exata = [x * math.log2(x) - x + 1 for x in n]
ax1.plot(n, [c / f for c, f in zip(cota, nlg)], "--", color=CINZA, drawstyle="steps-mid", label="Cota demonstrada: N·⌈log₂N⌉")
ax1.plot(n, [c / f for c, f in zip(exata, nlg)], ":", color=CINZA, lw=1.5, label="Pior caso exato p/ N = 2ᵏ: N·log₂N − N + 1")
ax1.plot(n, [c / f for c, f in zip(col("ms_iter", "comp_media_presente"), nlg)], "o", color=AZUL, ms=7, label="Medido, iterativo")
ax1.plot(n, [c / f for c, f in zip(col("ms_rec", "comp_media_presente"), nlg)], "x", color=LARANJA, ms=8, mew=2, label="Medido, recursivo")
ax1.set_xscale("log"); ax1.set_ylim(0.8, 1.12)
ax1.set(title="Comparações ÷ (N·log₂N)", xlabel="N (escala log)", ylabel="razão")
ax1.legend(fontsize=8, loc="lower right")
for alg, cor, mk, nome in [("ms_iter", AZUL, "o", "Iterativo"), ("ms_rec", LARANJA, "s", "Recursivo")]:
    t = col(alg, "tempo_s")
    a, b, r2 = ajuste(nlg, [v * 1e3 for v in t])
    ax2.plot(n, [v * 1e9 / f for v, f in zip(t, nlg)], mk + "-", color=cor, ms=7, lw=1.2, label=f"{nome}")
    resumo.append((f"MergeSort {nome.lower()}", "a·N·log₂N+b", a, b, r2, "ms"))
ax2.set_xscale("log"); ax2.set_ylim(0, 50)
ax2.set(title="Tempo ÷ (N·log₂N)", xlabel="N (escala log)", ylabel="ns por unidade de N·log₂N")
ax2.legend(fontsize=8, loc="lower right")
fig.savefig("graficos/fig3_mergesort.png"); plt.close(fig)

# ---------- 4. Solução 1 x Solução 2 ----------
n = col("total_sol1", "n")
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4))
c1, c2 = col("total_sol1", "comp_media_presente"), col("total_sol2", "comp_media_presente")
ax1.plot(n, c1, "o-", color=AZUL, ms=7, label="Solução 1 (sequencial)")
ax1.plot(n, c2, "s-", color=LARANJA, ms=7, label="Solução 2 (MergeSort + binária)")
ax1.plot(n, [0.75 * x * x for x in n], "--", color=CINZA, lw=1.2, label="Teoria Sol. 1: M·N·(3/4)")
ax1.plot(n, [x * math.log2(x) + x * (math.log2(x) + 1) for x in n], ":", color=CINZA, lw=1.2, label="Teoria Sol. 2: N·log₂N + M·(log₂N+1)")
ax1.set(xscale="log", yscale="log", title="Comparações totais (M = N, log-log)", xlabel="N = M", ylabel="comparações")
ax1.legend(fontsize=8)
t1, t2 = col("total_sol1", "tempo_s"), col("total_sol2", "tempo_s")
ax2.plot(n, t1, "o-", color=AZUL, ms=7, label="Solução 1 (sequencial)")
ax2.plot(n, t2, "s-", color=LARANJA, ms=7, label="Solução 2 (MergeSort + binária)")
ax2.set(xscale="log", yscale="log", title="Tempo total (M = N, log-log)", xlabel="N = M", ylabel="tempo (s)")
ax2.legend(fontsize=8)
for x, a, b in zip(n, t1, t2):
    if x in (1000, 64000):
        ax2.annotate(f"Sol. 2 {a/b:.0f}× mais rápida", (x, math.sqrt(a * b)), xytext=(8 if x == 1000 else -8, 0),
                     textcoords="offset points", ha="left" if x == 1000 else "right", fontsize=8, color=TEXTO)
        ax2.plot([x, x], [b, a], ":", color=CINZA, lw=1)
fig.savefig("graficos/fig4_solucao1_vs_solucao2.png"); plt.close(fig)

# ---------- 5. Estouro de pilha (opcional: só se resultados/pilha.csv existir) ----------
if os.path.exists("resultados/pilha.csv"):
    p = list(csv.DictReader(open("resultados/pilha.csv")))
    kb = [float(r["pilha_kb"]) for r in p]
    nmax = [float(r["n_max"]) for r in p]
    a, b, r2 = ajuste(kb, nmax)
    fig, ax = plt.subplots(figsize=(6.5, 4))
    ax.plot(kb, [a * x + b for x in kb], "--", color=CINZA, label=f"Ajuste linear: N_max = {a:.2f}·pilha(KB) {b:+.0f}  (R²={r2:.5f})")
    ax.plot(kb, nmax, "o", color=LARANJA, ms=8, label="Medido: maior N sem estouro")
    ax.set(title="Busca sequencial recursiva: N máximo antes do estouro de pilha", xlabel="tamanho da pilha (KB, ulimit -s)", ylabel="N máximo")
    ax.yaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(lambda v, _: f"{v/1000:.0f} mil"))
    ax.legend(fontsize=8)
    fig.savefig("graficos/fig5_estouro_pilha.png"); plt.close(fig)
    resumo.append(("Pilha: bytes por chamada recursiva", "1024/a", 1024 / a, b, r2, "bytes"))

with open("resultados/ajustes.csv", "w") as f:
    f.write("medida,modelo,a,b,r2,unidade\n")
    for linha in resumo:
        f.write("%s,%s,%.6g,%.6g,%.6f,%s\n" % linha)
print(open("resultados/ajustes.csv").read())
