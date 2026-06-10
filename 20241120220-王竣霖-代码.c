#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <float.h>
#include <math.h>

/* ==================== 全局变量 ==================== */
long long compare_count;       // 比较操作计数器
int merge_sub_size[10000000];  // 合并排序子问题规模记录
int quick_sub_size[10000000];  // 快速排序子问题规模记录
int merge_sub_idx = 0;         // 合并排序子问题索引
int quick_sub_idx = 0;         // 快速排序子问题索引
long long space_used;          // 粗略空间估算(字节)

/* ==================== 排序算法 ==================== */

// 冒泡排序
void bubbleSort(int arr[], int n) {
    compare_count = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            compare_count++;
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// 合并排序 - 合并过程
void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    int *L = (int*)malloc(n1 * sizeof(int));
    int *R = (int*)malloc(n2 * sizeof(int));
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        compare_count++;
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) { arr[k++] = L[i++]; }
    while (j < n2) { arr[k++] = R[j++]; }
    free(L); free(R);
}

// 合并排序 - 递归（记录子问题规模）
void mergeSortRec(int arr[], int left, int right) {
    if (left < right) {
        int size = right - left + 1;
        if (merge_sub_idx < 10000000) {
            merge_sub_size[merge_sub_idx++] = size;
        }
        int mid = left + (right - left) / 2;
        mergeSortRec(arr, left, mid);
        mergeSortRec(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// 合并排序入口
void mergeSort(int arr[], int n) {
    compare_count = 0;
    merge_sub_idx = 0;
    mergeSortRec(arr, 0, n - 1);
}

// 快速排序 - 分区
int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        compare_count++;
        if (arr[j] < pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

// 快速排序 - 递归（记录子问题规模）
void quickSortRec(int arr[], int low, int high) {
    if (low < high) {
        int size = high - low + 1;
        if (quick_sub_idx < 10000000) {
            quick_sub_size[quick_sub_idx++] = size;
        }
        int pi = partition(arr, low, high);
        quickSortRec(arr, low, pi - 1);
        quickSortRec(arr, pi + 1, high);
    }
}

// 快速排序入口
void quickSort(int arr[], int n) {
    compare_count = 0;
    quick_sub_idx = 0;
    quickSortRec(arr, 0, n - 1);
}

// 复制数组
void copyArray(int *src, int *dest, int n) {
    for (int i = 0; i < n; i++) dest[i] = src[i];
}

/* ==================== 0-1背包问题 ==================== */

// 物品结构体
typedef struct {
    int id;
    int weight;     // 重量 (1-100整数)
    double value;   // 价值 (保留2位小数)
} Item;

double dmax(double a, double b) { return a > b ? a : b; }

// 蛮力法 - 遍历所有子集
double knapsackBruteForce(Item items[], int n, int capacity, int *selected, double *bestValue) {
    *bestValue = 0;
    space_used = (long long)sizeof(int) * n;
    long long total = 1LL << n;
    // 限制n <= 25，否则太慢
    if (n > 25) {
        *bestValue = -1;
        return -1;
    }
    for (long long mask = 0; mask < total; mask++) {
        int curWeight = 0;
        double curValue = 0;
        for (int i = 0; i < n; i++) {
            if (mask & (1LL << i)) {
                curWeight += items[i].weight;
                curValue += items[i].value;
            }
        }
        if (curWeight <= capacity && curValue > *bestValue) {
            *bestValue = curValue;
            for (int i = 0; i < n; i++) {
                selected[i] = (mask & (1LL << i)) ? 1 : 0;
            }
        }
    }
    return *bestValue;
}

// 动态规划法 (0-1背包, 整数重量, 优化空间)
double knapsackDP(Item items[], int n, int capacity, int *selected) {
    // 限制 DP 规模 N*C <= 2亿
    if ((long long)n * capacity > 200000000LL) {
        space_used = -1;
        return -1;
    }
    space_used = (long long)(capacity + 1) * sizeof(double) + (long long)n * sizeof(int);
    double *dp = (double*)malloc((capacity + 1) * sizeof(double));
    for (int j = 0; j <= capacity; j++) dp[j] = 0;
    memset(selected, 0, n * sizeof(int));
    
    // 记录每个物品是否被选（用于回溯）
    // 使用位压缩：每32个物品一个unsigned int
    int *track = NULL;
    int trackSize = 0;
    // 只在小规模时记录回溯信息
    int needTrack = ((long long)n * capacity <= 50000000LL);
    if (needTrack) {
        trackSize = (capacity + 1);
        track = (int*)malloc(trackSize * sizeof(int));
        for (int j = 0; j <= capacity; j++) track[j] = -1;
    }
    
    for (int i = 1; i <= n; i++) {
        int w = items[i-1].weight;
        double v = items[i-1].value;
        for (int j = capacity; j >= w; j--) {
            double newVal = dp[j - w] + v;
            if (newVal > dp[j]) {
                dp[j] = newVal;
                if (needTrack) track[j] = i - 1;
            }
        }
    }
    
    double bestValue = dp[capacity];
    
    // 回溯选择（仅当有track信息时）
    if (needTrack) {
        int j = capacity;
        while (j > 0 && track[j] >= 0) {
            int idx = track[j];
            if (selected[idx] == 0) {
                selected[idx] = 1;
                j -= items[idx].weight;
            } else {
                break; // 防止死循环
            }
        }
    } else {
        // 大规模情况不回溯，只输出最优值
        // 选择最优的一个物品标记（仅用于展示）
        for (int i = 0; i < n; i++) selected[i] = 0;
    }
    
    free(dp);
    if (track) free(track);
    return bestValue;
}

// 贪心法 - 按价值密度排序
int cmpByDensity(const void *a, const void *b) {
    Item *ia = (Item*)a;
    Item *ib = (Item*)b;
    double da = ia->value / (double)ia->weight;
    double db = ib->value / (double)ib->weight;
    if (db > da) return 1;
    if (db < da) return -1;
    return 0;
}

double knapsackGreedy(Item items[], int n, int capacity, int *selected) {
    space_used = (long long)sizeof(Item) * n + (long long)sizeof(int) * n;
    Item *sorted = (Item*)malloc(n * sizeof(Item));
    for (int i = 0; i < n; i++) {
        sorted[i] = items[i];
        selected[i] = 0;
    }
    qsort(sorted, n, sizeof(Item), cmpByDensity);
    
    double totalValue = 0;
    int remainCapacity = capacity;
    for (int i = 0; i < n; i++) {
        if (sorted[i].weight <= remainCapacity) {
            remainCapacity -= sorted[i].weight;
            totalValue += sorted[i].value;
            // 标记原物品编号（通过id字段）
            for (int k = 0; k < n; k++) {
                if (items[k].id == sorted[i].id && selected[k] == 0) { 
                    selected[k] = 1; 
                    break; 
                }
            }
        }
    }
    
    free(sorted);
    return totalValue;
}

// 回溯法
void knapsackBacktrackRec(Item items[], int n, int capacity, int index,
                           int curWeight, double curValue,
                           double *bestValue, int *curSelect, int *bestSelect,
                           double *remainingValue) {
    if (index >= n) {
        if (curValue > *bestValue) {
            *bestValue = curValue;
            memcpy(bestSelect, curSelect, n * sizeof(int));
        }
        return;
    }
    // 剪枝：即使全选剩余物品也无法超过最优值
    if (curValue + remainingValue[index] <= *bestValue) return;
    
    // 不选当前物品
    curSelect[index] = 0;
    knapsackBacktrackRec(items, n, capacity, index + 1, curWeight, curValue,
                         bestValue, curSelect, bestSelect, remainingValue);
    
    // 选当前物品（如果放得下）
    if (curWeight + items[index].weight <= capacity) {
        curSelect[index] = 1;
        knapsackBacktrackRec(items, n, capacity, index + 1,
                             curWeight + items[index].weight,
                             curValue + items[index].value,
                             bestValue, curSelect, bestSelect, remainingValue);
    }
}

double knapsackBacktrack(Item items[], int n, int capacity, int *selected) {
    int *curSelect = (int*)malloc(n * sizeof(int));
    double *remainingValue = (double*)malloc((n + 1) * sizeof(double));
    memset(curSelect, 0, n * sizeof(int));
    memset(selected, 0, n * sizeof(int));
    remainingValue[n] = 0;
    for (int i = n - 1; i >= 0; i--) {
        remainingValue[i] = remainingValue[i + 1] + items[i].value;
    }
    double bestValue = 0;
    space_used = (long long)sizeof(int) * n * 2 + (long long)sizeof(double) * (n + 1);
    // 限制回溯规模 <= 5000 物品，否则太慢
    if (n <= 5000) {
        knapsackBacktrackRec(items, n, capacity, 0, 0, 0, &bestValue, curSelect, selected, remainingValue);
    } else {
        bestValue = -1;
    }
    free(curSelect);
    free(remainingValue);
    return bestValue;
}

/* ==================== 随机数生成 ==================== */

// 生成整数随机数组
void generateRandomInts(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100000;
    }
}

// 生成背包物品
void generateItems(Item items[], int n) {
    for (int i = 0; i < n; i++) {
        items[i].id = i + 1;
        items[i].weight = (rand() % 100) + 1; // 1~100
        items[i].value = ((rand() % 90100) + 10000) / 100.0; // 100.00~1000.00
    }
}

/* ==================== 排序实验 ==================== */

void runSortingExperiments() {
    FILE *fout = fopen("output/sorting_results.csv", "w");
    fprintf(fout, "Algorithm,Scale,Comparisons,Time_ms\n");
    
    int scales[] = {10, 100, 1000, 2000, 5000, 10000, 100000};
    int numScales = 7;
    
    printf("========================================\n");
    printf("        排序算法实验\n");
    printf("========================================\n\n");
    
    // 任务①：两次生成100个随机数，记录比较次数
    printf("【任务①】两次生成100个随机数，比较操作次数对比：\n");
    for (int run = 1; run <= 2; run++) {
        int n = 100;
        int *arr = (int*)malloc(n * sizeof(int));
        int *copyArr = (int*)malloc(n * sizeof(int));
        generateRandomInts(arr, n);
        
        printf("第%d次生成的数据：", run);
        for (int i = 0; i < n; i++) printf("%d ", arr[i]);
        printf("\n");
        
        copyArray(arr, copyArr, n);
        bubbleSort(copyArr, n);
        printf("  冒泡排序比较次数: %lld\n", compare_count);
        
        copyArray(arr, copyArr, n);
        mergeSort(copyArr, n);
        printf("  合并排序比较次数: %lld\n", compare_count);
        
        copyArray(arr, copyArr, n);
        quickSort(copyArr, n);
        printf("  快速排序比较次数: %lld\n\n", compare_count);
        
        free(arr);
        free(copyArr);
    }
    
    // 任务②：不同规模下的比较次数
    printf("\n【任务②&③】不同规模下各算法比较次数:\n");
    printf("%-8s %-16s %-16s %-16s\n", "规模", "冒泡排序", "合并排序", "快速排序");
    printf("--------------------------------------------------------------\n");
    
    FILE *fsub = fopen("output/subproblem_sizes.csv", "w");
    fprintf(fsub, "Algorithm,Scale,SubProblemSize\n");
    
    for (int s = 0; s < numScales; s++) {
        int n = scales[s];
        int *arr = (int*)malloc(n * sizeof(int));
        int *copyArr = (int*)malloc(n * sizeof(int));
        generateRandomInts(arr, n);
        
        clock_t start, end;
        double time_ms;
        
        // 冒泡排序
        copyArray(arr, copyArr, n);
        start = clock();
        bubbleSort(copyArr, n);
        end = clock();
        time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
        long long bubbleCmp = compare_count;
        fprintf(fout, "Bubble,%d,%lld,%.3f\n", n, compare_count, time_ms);
        
        // 合并排序
        copyArray(arr, copyArr, n);
        start = clock();
        mergeSort(copyArr, n);
        end = clock();
        time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
        long long mergeCmp = compare_count;
        int mergeSubCount = merge_sub_idx;
        fprintf(fout, "Merge,%d,%lld,%.3f\n", n, compare_count, time_ms);
        for (int i = 0; i < merge_sub_idx; i++) {
            fprintf(fsub, "Merge,%d,%d\n", n, merge_sub_size[i]);
        }
        
        // 快速排序
        copyArray(arr, copyArr, n);
        start = clock();
        quickSort(copyArr, n);
        end = clock();
        time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
        long long quickCmp = compare_count;
        int quickSubCount = quick_sub_idx;
        fprintf(fout, "Quick,%d,%lld,%.3f\n", n, compare_count, time_ms);
        for (int i = 0; i < quick_sub_idx; i++) {
            fprintf(fsub, "Quick,%d,%d\n", n, quick_sub_size[i]);
        }
        
        printf("%-8d %-16lld %-16lld %-16lld\n", n, bubbleCmp, mergeCmp, quickCmp);
        
        free(arr);
        free(copyArr);
    }
    
    fclose(fout);
    fclose(fsub);
    
    // 输出子问题规模统计
    printf("\n【任务③】合并排序和快速排序子问题规模已写入 output/subproblem_sizes.csv\n");
}

/* ==================== 0-1背包实验 ==================== */

void runKnapsackExperiments() {
    FILE *fout = fopen("output/knapsack_results.csv", "w");
    fprintf(fout, "Method,N,Capacity,BestValue,Time_ms,Space_bytes\n");
    
    // 物品数量
    int ns[] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
                20000, 40000, 80000, 160000, 320000};
    int numN = 15;
    // 背包容量
    int capacities[] = {10000, 100000, 1000000};
    int numCap = 3;
    
    printf("\n========================================\n");
    printf("        0-1背包问题实验\n");
    printf("========================================\n\n");
    
    for (int ci = 0; ci < numCap; ci++) {
        int C = capacities[ci];
        printf("\n========== 背包容量 C = %d ==========\n", C);
        printf("%-10s %-14s %-14s %-14s %-14s %-14s\n",
               "物品数N", "蛮力法(ms)", "动态规划(ms)", "贪心法(ms)", "回溯法(ms)", "最优值");
        printf("--------------------------------------------------------------------------------------------\n");
        
        for (int ni = 0; ni < numN; ni++) {
            int N = ns[ni];
            Item *items = (Item*)malloc(N * sizeof(Item));
            generateItems(items, N);
            
            // 如果N==1000，记录数据统计信息
            if (N == 1000) {
                FILE *fdata = fopen("output/items_1000.csv", "w");
                fprintf(fdata, "ItemID,Weight,Value\n");
                for (int k = 0; k < N; k++) {
                    fprintf(fdata, "%d,%d,%.2f\n", items[k].id, items[k].weight, items[k].value);
                }
                fclose(fdata);
            }
            
            int *selected = (int*)calloc(N, sizeof(int));
            double bestValue;
            clock_t start, end;
            double t_bf = -1, t_dp = -1, t_greedy = -1, t_bt = -1;
            double bestVal = -1;
            
            // 蛮力法 (仅小规模)
            if (N <= 25) {
                memset(selected, 0, N * sizeof(int));
                start = clock();
                bestValue = knapsackBruteForce(items, N, C, selected, &bestValue);
                end = clock();
                t_bf = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                if (bestValue > bestVal) bestVal = bestValue;
                fprintf(fout, "BruteForce,%d,%d,%.2f,%.3f,%lld\n", N, C, bestValue, t_bf, space_used);
            } else {
                fprintf(fout, "BruteForce,%d,%d,-1,-1,-1\n", N, C);
            }
            
            // 动态规划法 (限制规模 N*C <= 2亿)
            if ((long long)N * C <= 200000000LL) {
                memset(selected, 0, N * sizeof(int));
                start = clock();
                bestValue = knapsackDP(items, N, C, selected);
                end = clock();
                t_dp = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                if (bestValue > bestVal) bestVal = bestValue;
                fprintf(fout, "DP,%d,%d,%.2f,%.3f,%lld\n", N, C, bestValue, t_dp, space_used);
            } else {
                fprintf(fout, "DP,%d,%d,-1,-1,-1\n", N, C);
            }
            
            // 贪心法
            memset(selected, 0, N * sizeof(int));
            start = clock();
            bestValue = knapsackGreedy(items, N, C, selected);
            end = clock();
            t_greedy = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
            if (bestValue > bestVal) bestVal = bestValue;
            fprintf(fout, "Greedy,%d,%d,%.2f,%.3f,%lld\n", N, C, bestValue, t_greedy, space_used);
            
            // 回溯法 (限制规模)
            if (N <= 5000) {
                memset(selected, 0, N * sizeof(int));
                start = clock();
                bestValue = knapsackBacktrack(items, N, C, selected);
                end = clock();
                t_bt = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                if (bestValue > bestVal) bestVal = bestValue;
                fprintf(fout, "Backtrack,%d,%d,%.2f,%.3f,%lld\n", N, C, bestValue, t_bt, space_used);
            } else {
                fprintf(fout, "Backtrack,%d,%d,-1,-1,-1\n", N, C);
            }
            
            printf("%-10d %-14.1f %-14.1f %-14.1f %-14.1f %-14.2f\n",
                   N,
                   t_bf > 0 ? t_bf : -1,
                   t_dp > 0 ? t_dp : -1,
                   t_greedy,
                   t_bt > 0 ? t_bt : -1,
                   bestVal > 0 ? bestVal : bestValue);
            
            free(items);
            free(selected);
        }
    }
    
    fclose(fout);
    
    // 小规模示例：输出详细选择结果
    printf("\n\n【示例】背包容量10，5个物品的详细结果:\n");
    int small_n = 5;
    Item smallItems[5];
    smallItems[0].id = 1; smallItems[0].weight = 2; smallItems[0].value = 6;
    smallItems[1].id = 2; smallItems[1].weight = 2; smallItems[1].value = 3;
    smallItems[2].id = 3; smallItems[2].weight = 6; smallItems[2].value = 5;
    smallItems[3].id = 4; smallItems[3].weight = 5; smallItems[3].value = 4;
    smallItems[4].id = 5; smallItems[4].weight = 4; smallItems[4].value = 6;
    int smallCap = 10;
    int *sel = (int*)calloc(small_n, sizeof(int));
    double bf_val;
    
    printf("物品: 重量=[2,2,6,5,4], 价值=[6,3,5,4,6]\n\n");
    
    // 蛮力法
    knapsackBruteForce(smallItems, small_n, smallCap, sel, &bf_val);
    printf("蛮力法最优值: %.0f, 选择: ", bf_val);
    for (int i = 0; i < small_n; i++) if (sel[i]) printf("%d ", smallItems[i].id);
    printf("\n");
    
    // 动态规划
    memset(sel, 0, small_n * sizeof(int));
    double dp_val = knapsackDP(smallItems, small_n, smallCap, sel);
    printf("动态规划最优值: %.0f, 选择: ", dp_val);
    for (int i = 0; i < small_n; i++) if (sel[i]) printf("%d ", smallItems[i].id);
    printf("\n");
    
    // 贪心
    memset(sel, 0, small_n * sizeof(int));
    double greedy_val = knapsackGreedy(smallItems, small_n, smallCap, sel);
    printf("贪心法值: %.0f, 选择: ", greedy_val);
    for (int i = 0; i < small_n; i++) if (sel[i]) printf("%d ", smallItems[i].id);
    printf("\n");
    
    // 回溯法
    memset(sel, 0, small_n * sizeof(int));
    double bt_val = knapsackBacktrack(smallItems, small_n, smallCap, sel);
    printf("回溯法最优值: %.0f, 选择: ", bt_val);
    for (int i = 0; i < small_n; i++) if (sel[i]) printf("%d ", smallItems[i].id);
    printf("\n");
    
    free(sel);
}

/* ==================== 主函数 ==================== */

int main() {
    srand((unsigned)time(NULL));
    
    printf("╔══════════════════════════════════════════╗\n");
    printf("║   算法设计与分析实验 - 2026年春         ║\n");
    printf("║   排序问题 & 0-1背包问题                ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf("\n实验开始时间: %s\n\n", __TIMESTAMP__);
    
    // 创建输出目录
    system("mkdir output 2>nul");
    
    clock_t totalStart = clock();
    
    runSortingExperiments();
    runKnapsackExperiments();
    
    clock_t totalEnd = clock();
    double totalTime = ((double)(totalEnd - totalStart) * 1000) / CLOCKS_PER_SEC;
    
    printf("\n========================================\n");
    printf("  实验完成！总耗时: %.2f 秒\n", totalTime / 1000.0);
    printf("  结果文件已保存至 output/ 目录\n");
    printf("========================================\n");
    
    return 0;
}