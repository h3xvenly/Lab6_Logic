/*
 * Лабораторная работа №6. Унарные и бинарные операции над графами.
 *
 * Граф неориентированный, без петель и кратных рёбер.
 * Вершины для пользователя нумеруются с 1, внутри программы - с 0.
 *
 * Представления графа:
 *   Graph - матрица смежности  (a[i][j] = 1, если вершины i и j соединены ребром)
 *   List  - списки смежности   (для каждой вершины - массив её соседей)
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS   /* чтобы компилятор Visual Studio не ругался на scanf */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>              /* Windows: настройка русского языка в консоли */
#else
#include <locale.h>               /* Linux / macOS */
#endif

#define MAXN 6            /* максимум вершин в исходном графе */
#define MAX  (MAXN * MAXN) /* максимум вершин вообще (декартово произведение: 6*6 = 36) */

/* ======================= СТРУКТУРЫ ======================= */

typedef struct {
    int n;                 /* число вершин */
    int a[MAX][MAX];       /* матрица смежности */
} Graph;

typedef struct {
    int n;                 /* число вершин */
    int cnt[MAX];          /* cnt[i] - сколько соседей у вершины i */
    int nb[MAX][MAX];      /* nb[i][0..cnt[i]-1] - номера соседей вершины i */
} List;

/* ======================= ВВОД ======================= */

/* Просит ввести число от lo до hi, пока не введут правильно */
int readInt(const char *text, int lo, int hi) {
    int x;
    while (1) {
        printf("%s", text);
        int r = scanf("%d", &x);
        if (r == EOF) exit(0);                       /* ввод закончился */
        if (r == 1 && x >= lo && x <= hi) return x;
        printf("  Нужно число от %d до %d, попробуйте ещё раз.\n", lo, hi);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);   /* выбрасываем неверный ввод */
    }
}

/* ======================= ЗАДАНИЕ 1: ГЕНЕРАЦИЯ И ВЫВОД ======================= */

/* Случайный неориентированный граф: заполняем верхний треугольник
   нулями и единицами и копируем его в нижний (матрица симметричная).
   Диагональ остаётся нулевой - петель нет. */
void generate(Graph *g, int n) {
    g->n = n;
    for (int i = 0; i < MAX; i++)
        for (int j = 0; j < MAX; j++) g->a[i][j] = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            g->a[i][j] = g->a[j][i] = rand() % 2;
}

/* Печать матрицы смежности.
   hideIsolated = 1 - не печатать изолированные вершины (нужно для кольцевой суммы).
   Сверху и слева подписаны номера вершин. */
void printGraph(const Graph *g, int hideIsolated) {
    int idx[MAX], k = 0;                    /* idx - какие вершины печатаем */
    for (int i = 0; i < g->n; i++) {
        int deg = 0;
        for (int j = 0; j < g->n; j++) deg += g->a[i][j];
        if (!hideIsolated || deg > 0) idx[k++] = i;
    }
    if (k == 0) { printf("  (вершин нет)\n"); return; }

    printf("     ");
    for (int c = 0; c < k; c++) printf("%3d", idx[c] + 1);
    printf("\n");
    for (int r = 0; r < k; r++) {
        printf("%3d: ", idx[r] + 1);
        for (int c = 0; c < k; c++) printf("%3d", g->a[idx[r]][idx[c]]);
        printf("\n");
    }
}

/* ======================= СПИСКИ СМЕЖНОСТИ ======================= */

/* Проверка: есть ли ребро u-v */
int hasEdgeL(const List *l, int u, int v) {
    for (int i = 0; i < l->cnt[u]; i++)
        if (l->nb[u][i] == v) return 1;
    return 0;
}

/* Вставить v в список соседей u так, чтобы список остался по возрастанию */
void insertNb(List *l, int u, int v) {
    int pos = l->cnt[u];
    while (pos > 0 && l->nb[u][pos - 1] > v) {
        l->nb[u][pos] = l->nb[u][pos - 1];
        pos--;
    }
    l->nb[u][pos] = v;
    l->cnt[u]++;
}

/* Убрать v из списка соседей u */
void removeNb(List *l, int u, int v) {
    int i = 0;
    while (i < l->cnt[u] && l->nb[u][i] != v) i++;
    if (i == l->cnt[u]) return;                 /* такого соседа нет */
    for (; i < l->cnt[u] - 1; i++) l->nb[u][i] = l->nb[u][i + 1];
    l->cnt[u]--;
}

/* Добавить ребро u-v (в оба списка). Петли и повторы игнорируем. */
void addEdgeL(List *l, int u, int v) {
    if (u == v || hasEdgeL(l, u, v)) return;
    insertNb(l, u, v);
    insertNb(l, v, u);
}

/* Удалить ребро u-v */
void delEdgeL(List *l, int u, int v) {
    removeNb(l, u, v);
    removeNb(l, v, u);
}

/* Задание 1*: матрица смежности -> списки смежности */
void toList(const Graph *g, List *l) {
    l->n = g->n;
    for (int i = 0; i < g->n; i++) {
        l->cnt[i] = 0;
        for (int j = 0; j < g->n; j++)
            if (g->a[i][j]) l->nb[i][l->cnt[i]++] = j;
    }
}

void printList(const List *l) {
    for (int i = 0; i < l->n; i++) {
        printf("%3d: ", i + 1);
        if (l->cnt[i] == 0) printf("-");
        for (int k = 0; k < l->cnt[i]; k++) printf("%d ", l->nb[i][k] + 1);
        printf("\n");
    }
}

/* ======================= ЗАДАНИЕ 2: УНАРНЫЕ ОПЕРАЦИИ (МАТРИЦЫ) ======================= */

/* Удалить вершину v: убираем строку v и столбец v, остальное сдвигаем */
void removeVertex(Graph *g, int v) {
    for (int i = v; i < g->n - 1; i++)           /* сдвиг строк вверх */
        for (int j = 0; j < g->n; j++) g->a[i][j] = g->a[i + 1][j];
    for (int j = v; j < g->n - 1; j++)           /* сдвиг столбцов влево */
        for (int i = 0; i < g->n - 1; i++) g->a[i][j] = g->a[i][j + 1];
    g->n--;
}

/* Отождествление вершин u и v (u < v).
   Новая вершина z соединяется со всеми из Q = Q1 U Q2 (соседи u и соседи v).
   Реализация: строку u заменяем на "строка u ИЛИ строка v" - это и есть Q,
   затем удаляем v. Вершина z занимает место u. */
void identify(Graph *g, int u, int v) {
    for (int i = 0; i < g->n; i++) {
        g->a[u][i] = g->a[u][i] || g->a[v][i];
        g->a[i][u] = g->a[u][i];                 /* матрица остаётся симметричной */
    }
    g->a[u][u] = 0;                              /* z не соединена сама с собой */
    removeVertex(g, v);
}

/* Расщепление вершины v: появляется новая вершина (номер n).
   Соседей v делим: каждый второй уходит к новой вершине.
   Затем v и новая вершина соединяются ребром. */
void split(Graph *g, int v) {
    int n = g->n, k = 0;
    g->n = n + 1;
    for (int i = 0; i <= n; i++) g->a[n][i] = g->a[i][n] = 0;   /* чистая строка/столбец */
    for (int w = 0; w < n; w++) {
        if (!g->a[v][w]) continue;               /* w не сосед v */
        if (k % 2 == 1) {                        /* каждый второй сосед */
            g->a[v][w] = g->a[w][v] = 0;
            g->a[n][w] = g->a[w][n] = 1;
        }
        k++;
    }
    g->a[v][n] = g->a[n][v] = 1;
}

/* ======================= ЗАДАНИЕ 2*: УНАРНЫЕ ОПЕРАЦИИ (СПИСКИ) ======================= */

/* Отождествление u и v (u < v) на списках: строим новый список res.
   Вершина v исчезает, номера больших вершин уменьшаются на 1, z получает номер u. */
void identifyL(const List *l, int u, int v, List *res) {
    int map[MAX];                                /* map[старый номер] = новый номер */
    for (int i = 0; i < l->n; i++) map[i] = (i < v) ? i : i - 1;

    res->n = l->n - 1;
    for (int i = 0; i < res->n; i++) res->cnt[i] = 0;

    for (int i = 0; i < l->n; i++) {
        if (i == u || i == v) continue;
        for (int k = 0; k < l->cnt[i]; k++) {
            int j = l->nb[i][k];
            if (j == u || j == v) addEdgeL(res, map[i], u);   /* ребро к z */
            else                  addEdgeL(res, map[i], map[j]);
        }
    }
}

/* Расщепление вершины v на списках (правило то же, что и для матриц) */
void splitL(List *l, int v) {
    int n = l->n, tmp[MAX], c = l->cnt[v];
    for (int i = 0; i < c; i++) tmp[i] = l->nb[v][i];   /* запоминаем соседей v */
    l->n = n + 1;
    l->cnt[n] = 0;
    for (int i = 1; i < c; i += 2) {                    /* каждый второй сосед */
        delEdgeL(l, v, tmp[i]);
        addEdgeL(l, n, tmp[i]);
    }
    addEdgeL(l, v, n);
}

/* ======================= ЗАДАНИЕ 3: БИНАРНЫЕ ОПЕРАЦИИ ======================= */

/* Элемент матрицы, а если вершины в графе нет - 0 */
int get(const Graph *g, int i, int j) {
    return (i < g->n && j < g->n) ? g->a[i][j] : 0;
}

/* Объединение: рёбра из G1 ИЛИ из G2. Вершин столько, сколько в большем графе
   (вершины с одинаковыми номерами считаются общими). */
void unite(const Graph *x, const Graph *y, Graph *r) {
    int n = x->n > y->n ? x->n : y->n;
    generate(r, 0);
    r->n = n;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) r->a[i][j] = get(x, i, j) || get(y, i, j);
}

/* Пересечение: рёбра и из G1, И из G2. Вершины - только общие (меньший граф). */
void intersect(const Graph *x, const Graph *y, Graph *r) {
    int n = x->n < y->n ? x->n : y->n;
    generate(r, 0);
    r->n = n;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) r->a[i][j] = x->a[i][j] && y->a[i][j];
}

/* Кольцевая сумма: ребро есть ровно в одном из графов (исключающее ИЛИ).
   Изолированные вершины потом не печатаем (printGraph(r, 1)). */
void ringSum(const Graph *x, const Graph *y, Graph *r) {
    int n = x->n > y->n ? x->n : y->n;
    generate(r, 0);
    r->n = n;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) r->a[i][j] = get(x, i, j) != get(y, i, j);
}

/* ======================= ЗАДАНИЕ 4: ДЕКАРТОВО ПРОИЗВЕДЕНИЕ ======================= */

/* Вершина (z_i, v_k) получает номер i*n2 + k.
   Две вершины (z_i, v_k) и (z_j, v_l) соединены, если
     i == j и v_k, v_l соединены в G2     (двигаемся по G2, оставаясь в "строке" i)
     или
     k == l и z_i, z_j соединены в G1     (двигаемся по G1, оставаясь в "столбце" k) */
void product(const Graph *x, const Graph *y, Graph *r) {
    int n2 = y->n;
    generate(r, 0);
    r->n = x->n * y->n;
    for (int i = 0; i < x->n; i++)
        for (int k = 0; k < y->n; k++)
            for (int j = 0; j < x->n; j++)
                for (int l = 0; l < y->n; l++)
                    if ((i == j && y->a[k][l]) || (k == l && x->a[i][j]))
                        r->a[i * n2 + k][j * n2 + l] = 1;
}

/* ======================= МЕНЮ ======================= */

void showGraphs(const Graph *g1, const Graph *g2) {
    List l;
    printf("\nG1 - матрица смежности:\n");  printGraph(g1, 0);
    printf("\nG2 - матрица смежности:\n");  printGraph(g2, 0);
    toList(g1, &l);
    printf("\nG1 - списки смежности:\n");   printList(&l);
    toList(g2, &l);
    printf("\nG2 - списки смежности:\n");   printList(&l);
}

/* Операции 2-4 меню: what = 1 отождествление, 2 стягивание, 3 расщепление */
void unaryOperation(const Graph *g1, const Graph *g2, int what) {
    int which = readInt("Какой граф взять (1 или 2): ", 1, 2);
    int form  = readInt("Форма представления (1 - матрица, 2 - списки): ", 1, 2);
    Graph g = (which == 1) ? *g1 : *g2;          /* работаем с копией, оригинал не портим */
    int u = 0, v;

    printf("Исходный граф G%d:\n", which);
    if (form == 1) printGraph(&g, 0);
    else { List l; toList(&g, &l); printList(&l); }

    if (what == 3) {
        v = readInt("Номер вершины для расщепления: ", 1, g.n) - 1;
        if (g.n + 1 > MAX) { printf("Слишком много вершин.\n"); return; }
        printf("\nРезультат расщепления вершины %d (новая вершина - %d):\n", v + 1, g.n + 1);
        if (form == 1) { split(&g, v); printGraph(&g, 0); }
        else { List l; toList(&g, &l); splitL(&l, v); printList(&l); }
        return;
    }

    u = readInt("Номер первой вершины: ", 1, g.n) - 1;
    v = readInt("Номер второй вершины: ", 1, g.n) - 1;
    if (u == v) { printf("Нужны две разные вершины.\n"); return; }
    if (u > v) { int t = u; u = v; v = t; }       /* чтобы всегда было u < v */

    if (what == 2 && !g.a[u][v]) {
        printf("Вершины %d и %d не смежны - ребро стянуть нельзя.\n", u + 1, v + 1);
        return;
    }
    printf("\nРезультат %s вершин %d и %d (новая вершина стоит под номером %d):\n",
           what == 1 ? "отождествления" : "стягивания ребра", u + 1, v + 1, u + 1);
    if (form == 1) { identify(&g, u, v); printGraph(&g, 0); }
    else {
        List l, res;
        toList(&g, &l);
        identifyL(&l, u, v, &res);
        printList(&res);
    }
}

void binaryOperations(const Graph *g1, const Graph *g2) {
    Graph r;
    unite(g1, g2, &r);
    printf("\nа) Объединение графов G1 и G2:\n");               printGraph(&r, 0);
    intersect(g1, g2, &r);
    printf("\nб) Пересечение графов G1 и G2:\n");               printGraph(&r, 0);
    ringSum(g1, g2, &r);
    printf("\nв) Кольцевая сумма графов G1 и G2 (без изолированных вершин):\n");
    printGraph(&r, 1);
}

void cartesian(const Graph *g1, const Graph *g2) {
    Graph r;
    product(g1, g2, &r);
    printf("\nДекартово произведение G1 x G2, %d вершин.\n", r.n);
    printf("Номер вершины (z_i, v_k) = (i-1)*%d + k, где i, k считаются с 1.\n", g2->n);
    printf("Например, (z_1, v_1) = 1, (z_1, v_2) = 2, (z_2, v_1) = %d.\n\n", g2->n + 1);
    printGraph(&r, 0);
}

int main(void) {
#ifdef _WIN32
    /* Windows: русский язык в консоли и в терминале VS Code.
       Файл должен быть сохранён в UTF-8 (справа внизу в VS Code: "UTF-8"). */
    SetConsoleOutputCP(CP_UTF8);         /* вывод в UTF-8 */
    SetConsoleCP(CP_UTF8);               /* ввод в UTF-8 */
    system("chcp 65001 > nul");          /* запасной вариант для терминалов, где API не сработал */
#else
    /* Linux / macOS: терминал уже в UTF-8, достаточно локали системы */
    setlocale(LC_ALL, "");
#endif
    srand((unsigned)time(NULL));

    Graph g1, g2;
    int choice;

    generate(&g1, readInt("Число вершин G1 (2..6): ", 2, MAXN));
    generate(&g2, readInt("Число вершин G2 (2..6): ", 2, MAXN));
    showGraphs(&g1, &g2);                /* Задание 1 */

    do {
        printf("\n========== МЕНЮ ==========\n");
        printf("1 - показать графы (матрицы и списки)\n");
        printf("2 - отождествление вершин\n");
        printf("3 - стягивание ребра\n");
        printf("4 - расщепление вершины\n");
        printf("5 - объединение, пересечение, кольцевая сумма\n");
        printf("6 - декартово произведение\n");
        printf("7 - сгенерировать графы заново\n");
        printf("0 - выход\n");
        choice = readInt("Ваш выбор: ", 0, 7);

        switch (choice) {
            case 1: showGraphs(&g1, &g2);              break;
            case 2: unaryOperation(&g1, &g2, 1);       break;
            case 3: unaryOperation(&g1, &g2, 2);       break;
            case 4: unaryOperation(&g1, &g2, 3);       break;
            case 5: binaryOperations(&g1, &g2);        break;
            case 6: cartesian(&g1, &g2);               break;
            case 7:
                generate(&g1, readInt("Число вершин G1 (2..6): ", 2, MAXN));
                generate(&g2, readInt("Число вершин G2 (2..6): ", 2, MAXN));
                showGraphs(&g1, &g2);
                break;
        }
    } while (choice != 0);

    return 0;
}
