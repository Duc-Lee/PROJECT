#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_VERTICES 100
#define MAX_NAME_LENGTH 100
#define RESULT_FILE "path_result.txt"

// Node for adjacency list
typedef struct node {
    int dinh;
    int trong_so;
    struct node *next;
} node;

// Graph structure
typedef struct {
    int so_dinh;
    node **danh_sach_ke;
} Dothi;

// Min heap node
typedef struct {
    int dinh;
    int khoang_cach;
} node_min_heap;

// Min heap structure
typedef struct {
    int kich_thuoc;
    int suc_chua;
    int *vitri;
    node_min_heap **arr;
} min_heap;

// Create a new adjacency list node
node *createnode(int dinh, int trong_so) {
    node *newnode = (node*)malloc(sizeof(node));
    newnode->dinh = dinh;
    newnode->trong_so = trong_so;
    newnode->next = NULL;
    return newnode;
}

// Create a new graph
Dothi *createDothi(int so_dinh) {
    Dothi *newgraph = (Dothi*)malloc(sizeof(Dothi));
    newgraph->so_dinh = so_dinh;
    newgraph->danh_sach_ke = (node**)malloc(so_dinh * sizeof(node*));
    for (int i = 0; i < so_dinh; i++) newgraph->danh_sach_ke[i] = NULL;
    return newgraph;
}

// Add undirected edge
void them_canh(Dothi *dothi, int dinh_nguon, int dinh_dich, int trong_so) {
    node *newnode = createnode(dinh_dich, trong_so);
    newnode->next = dothi->danh_sach_ke[dinh_nguon];
    dothi->danh_sach_ke[dinh_nguon] = newnode;
    newnode = createnode(dinh_nguon, trong_so);
    newnode->next = dothi->danh_sach_ke[dinh_dich];
    dothi->danh_sach_ke[dinh_dich] = newnode;
}

// Min heap functions
node_min_heap *taoNodeMinHeap(int dinh, int khoang_cach) {
    node_min_heap *node = (node_min_heap*)malloc(sizeof(node_min_heap));
    node->dinh = dinh;
    node->khoang_cach = khoang_cach;
    return node;
}

min_heap *taoMinHeap(int suc_chua) {
    min_heap *minheap = (min_heap*)malloc(sizeof(min_heap));
    minheap->kich_thuoc = 0;
    minheap->suc_chua = suc_chua;
    minheap->vitri = (int*)malloc(suc_chua * sizeof(int));
    minheap->arr = (node_min_heap**)malloc(suc_chua * sizeof(node_min_heap*));
    return minheap;
}

void swapNode(node_min_heap **a, node_min_heap **b) {
    node_min_heap *temp = *a;
    *a = *b;
    *b = temp;
}

void shiftDown(min_heap *minheap, int idx) {
    int nho_nhat = idx;
    int trai = 2 * idx + 1;
    int phai = 2 * idx + 2;
    if (trai < minheap->kich_thuoc && minheap->arr[trai]->khoang_cach < minheap->arr[nho_nhat]->khoang_cach)
        nho_nhat = trai;
    if (phai < minheap->kich_thuoc && minheap->arr[phai]->khoang_cach < minheap->arr[nho_nhat]->khoang_cach)
        nho_nhat = phai;
    if (nho_nhat != idx) {
        minheap->vitri[minheap->arr[nho_nhat]->dinh] = idx;
        minheap->vitri[minheap->arr[idx]->dinh] = nho_nhat;
        swapNode(&minheap->arr[nho_nhat], &minheap->arr[idx]);
        shiftDown(minheap, nho_nhat);
    }
}

int isEmpty(min_heap *minheap) {
    return minheap->kich_thuoc == 0;
}

node_min_heap *extractMin(min_heap *minheap) {
    if (isEmpty(minheap)) return NULL;
    node_min_heap *root = minheap->arr[0];
    node_min_heap *cuoi = minheap->arr[minheap->kich_thuoc - 1];
    minheap->arr[0] = cuoi;
    minheap->vitri[root->dinh] = minheap->kich_thuoc - 1;
    minheap->vitri[cuoi->dinh] = 0;
    minheap->kich_thuoc--;
    shiftDown(minheap, 0);
    return root;
}

void decreaseKey(min_heap *minheap, int dinh, int khoang_cach) {
    int i = minheap->vitri[dinh];
    minheap->arr[i]->khoang_cach = khoang_cach;
    while (i && minheap->arr[i]->khoang_cach < minheap->arr[(i - 1) / 2]->khoang_cach) {
        minheap->vitri[minheap->arr[i]->dinh] = (i - 1) / 2;
        minheap->vitri[minheap->arr[(i - 1) / 2]->dinh] = i;
        swapNode(&minheap->arr[i], &minheap->arr[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

int isInMinHeap(min_heap *minheap, int dinh) {
    return minheap->vitri[dinh] < minheap->kich_thuoc;
}

void giaiPhongMinHeap(min_heap* minHeap) {
    if (minHeap == NULL) return;
    for (int i = 0; i < minHeap->kich_thuoc; i++) {
        if (minHeap->arr[i] != NULL) free(minHeap->arr[i]);
    }
    free(minHeap->arr);
    free(minHeap->vitri);
    free(minHeap);
}

// Dijkstra
void dijkstra(Dothi* doThi, int dinhNguon, int khoangCach[], int parent[]) {
    int soDinh = doThi->so_dinh;
    min_heap* minHeap = taoMinHeap(soDinh);
    for (int v = 0; v < soDinh; v++) {
        khoangCach[v] = INT_MAX;
        parent[v] = -1;
        minHeap->arr[v] = taoNodeMinHeap(v, khoangCach[v]);
        minHeap->vitri[v] = v;
    }
    khoangCach[dinhNguon] = 0;
    decreaseKey(minHeap, dinhNguon, khoangCach[dinhNguon]);
    minHeap->kich_thuoc = soDinh;
    while (!isEmpty(minHeap)) {
        node_min_heap* nodeMin = extractMin(minHeap);
        int u = nodeMin->dinh;
        free(nodeMin);
        node* ke = doThi->danh_sach_ke[u];
        while (ke != NULL) {
            int v = ke->dinh;
            if (isInMinHeap(minHeap, v) && khoangCach[u] != INT_MAX &&
                ke->trong_so + khoangCach[u] < khoangCach[v]) {
                khoangCach[v] = khoangCach[u] + ke->trong_so;
                parent[v] = u;
                decreaseKey(minHeap, v, khoangCach[v]);
            }
            ke = ke->next;
        }
    }
    giaiPhongMinHeap(minHeap);
}

// Free graph
void giaiPhongDoThi(Dothi* doThi) {
    if (doThi == NULL) return;
    for (int i = 0; i < doThi->so_dinh; i++) {
        node* hienTai = doThi->danh_sach_ke[i];
        while (hienTai != NULL) {
            node* temp = hienTai;
            hienTai = hienTai->next;
            free(temp);
        }
    }
    free(doThi->danh_sach_ke);
    free(doThi);
}

// Find index of vertex name
int timIndex(char* tenToaNha[], int soDinh, const char* toa) {
    for (int i = 0; i < soDinh; i++) {
        if (strcmp(tenToaNha[i], toa) == 0) return i;
    }
    return -1;
}

// Print path recursively and save to file
void printPath(int parent[], int j, char* tenToaNha[], FILE* result_file) {
    if (parent[j] == -1) {
        printf("%s", tenToaNha[j]);
        fprintf(result_file, "%s", tenToaNha[j]);
        return;
    }
    printPath(parent, parent[j], tenToaNha, result_file);
    printf(" -> %s", tenToaNha[j]);
    fprintf(result_file, " -> %s", tenToaNha[j]);
}

int main() {
    int soDinh = 19;  // Số lượng tòa nhà cố định
    char *tenToaNha[] = {
        "Toa C1", "Toa C2", "Toa C3", "Toa C4", "Toa C5", "Toa C6", "Toa C7", "Toa C8", "Toa C9", "Toa C10",
        "Toa D3", "Toa D4", "Toa D5", "Toa D6", "Toa D7", "Toa D8", "Toa D9",
        "Thu Vien", "Nha TC"
    };

    Dothi *doThi = createDothi(soDinh);
    if (!doThi) {
        fprintf(stderr, "Error: Failed to create graph\n");
        return 1;
    }

    // Thêm các cạnh cố định
    them_canh(doThi, 0, 1, 50);   // C1 - C2
    them_canh(doThi, 1, 2, 50);   // C2 - C3
    them_canh(doThi, 2, 3, 50);   // C3 - C4
    them_canh(doThi, 3, 4, 50);   // C4 - C5
    them_canh(doThi, 4, 5, 50);   // C5 - C6
    them_canh(doThi, 5, 6, 50);   // C6 - C7
    them_canh(doThi, 6, 7, 50);   // C7 - C8
    them_canh(doThi, 7, 8, 50);   // C8 - C9
    them_canh(doThi, 8, 9, 50);   // C9 - C10
    them_canh(doThi, 10, 11, 50); // D3 - D4
    them_canh(doThi, 11, 12, 50); // D4 - D5
    them_canh(doThi, 12, 13, 50); // D5 - D6
    them_canh(doThi, 13, 14, 50); // D6 - D7
    them_canh(doThi, 14, 15, 50); // D7 - D8
    them_canh(doThi, 15, 16, 50); // D8 - D9
    them_canh(doThi, 0, 10, 100); // C1 - D3
    them_canh(doThi, 1, 11, 100); // C2 - D4
    them_canh(doThi, 2, 12, 100); // C3 - D5
    them_canh(doThi, 3, 13, 100); // C4 - D6
    them_canh(doThi, 4, 14, 100); // C5 - D7
    them_canh(doThi, 5, 15, 100); // C6 - D8
    them_canh(doThi, 6, 16, 100); // C7 - D9
    them_canh(doThi, 7, 17, 150); // C8 - Thu Vien
    them_canh(doThi, 8, 17, 150); // C9 - Thu Vien
    them_canh(doThi, 9, 17, 150); // C10 - Thu Vien
    them_canh(doThi, 10, 18, 150); // D3 - Nha TC
    them_canh(doThi, 11, 18, 150); // D4 - Nha TC
    them_canh(doThi, 12, 18, 150); // D5 - Nha TC

    char source[MAX_NAME_LENGTH], dest[MAX_NAME_LENGTH];
    if (!fgets(source, sizeof(source), stdin) || !fgets(dest, sizeof(dest), stdin)) {
        fprintf(stderr, "Error: Failed to read source or destination\n");
        giaiPhongDoThi(doThi);
        return 1;
    }

    source[strcspn(source, "\n")] = 0;
    dest[strcspn(dest, "\n")] = 0;

    int dinhNguon = timIndex(tenToaNha, soDinh, source);
    int dinhDich = timIndex(tenToaNha, soDinh, dest);

    if (dinhNguon == -1 || dinhDich == -1) {
        fprintf(stderr, "Error: Invalid source or destination\n");
        giaiPhongDoThi(doThi);
        return 1;
    }

    int *khoangCach = (int*)malloc(soDinh * sizeof(int));
    int *parent = (int*)malloc(soDinh * sizeof(int));
    if (!khoangCach || !parent) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        giaiPhongDoThi(doThi);
        free(khoangCach);
        free(parent);
        return 1;
    }

    dijkstra(doThi, dinhNguon, khoangCach, parent);

    // Mở file để ghi kết quả
    FILE* result_file = fopen(RESULT_FILE, "w");
    if (!result_file) {
        fprintf(stderr, "Error: Cannot open result file\n");
        giaiPhongDoThi(doThi);
        free(khoangCach);
        free(parent);
        return 1;
    }

    if (khoangCach[dinhDich] == INT_MAX) {
        printf("Không tìm thấy đường đi từ %s đến %s\n", source, dest);
        fprintf(result_file, "Không tìm thấy đường đi\n-1\n");
    } else {
        printPath(parent, dinhDich, tenToaNha, result_file);
        printf("\n%d\n", khoangCach[dinhDich]);
        fprintf(result_file, "\n%d\n", khoangCach[dinhDich]);
    }

    fclose(result_file);

    // Giải phóng bộ nhớ
    giaiPhongDoThi(doThi);
    free(khoangCach);
    free(parent);

    return 0;
}
       

   