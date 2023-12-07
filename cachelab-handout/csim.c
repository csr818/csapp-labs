#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define addrlen 32
static int s; // 2^s个组
static int E; // 组里有E个行
static int b; // block位数为b，不用真的在行里包含数据内容
char filename[1000];

// 不能简单的在行中数据处存一int，如果使用o(n)的方式实现lru
static int hit_times, miss_times, eviction_times;


typedef struct {
    LRU_node* pre;
    LRU_node* nxt;
    int line_idx;
}LRU_node;


void move(int i) {

}


int** create_cache() {
    int taglen = addrlen - s - b;
    // 每行用一个数字表示
    int S = 1 << s;
    int** cache = (int**) malloc(sizeof(int*) * S);
    for (int i = 0; i < S; i++) {
        cache[i] = (int*) malloc(sizeof(int) * E);
    }
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j] = 0;
        }
    }
    return cache;
}

void test_print_cache(int** cache) {
    for (int i = 0; i < (1 << s); i++) {
        for (int j = 0; j < E; j++) {
            printf("%d ", cache[i][j]);
        }
        puts("");
    }
}

void do_command(int** cache,char type, unsigned int address, int size) {
    unsigned int tag = address >> (b + s); // >> 优先级高
    int S_idx = (address >> b) & ((-1U) >> (addrlen - s));
    // 当前组内所有的行都不包含这个值而且行满了

    int cache_date;
    // 找是否存在cache中
    for (int i = 0; i < E; i++) {
        cache_date = cache[S_idx][i];
        if (cache_date & (1 << b - 1) == tag && cache_date >> (b + 1) & 1) {
            ++hit_times;
            return ;
        }
    } 

    // 不在cache中，检查cache中是否存在空行
    for (int i = 0; i < E; i++) {
        cache_date = cache[S_idx][i];
        if (cache_date >> (b + 1) & 1 == 0) {
            switch (type) {
                case 'L':
                    ++miss_times;
                    break;
                default:
                    ++miss_times, ++hit_times;
                    break;
            }
            cache[S_idx][i] = tag | 1 << (addrlen - b - s); // 设置cache中的值为有效位为1，tag是当前地址的tag
            return ;
        }
    }

    // 不在cache中也不存在空行，那么需要对这个组里进行行的替换

}

void start_simulate(int** cache, FILE* f) {
    char type;
    unsigned int address; // 地址应该是无符号数
    int size;
    while (3 == fscanf(f, " %c %x %d", &type, &address, &size)) {
        do_command(cache, type, address, size);
    }
}

int main(int argc, char* argv[])
{
    int optchr;
    while (-1 != (optchr = getopt(argc, argv, "s:E:b:t:"))) {
        
        switch (optchr)
        {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
            default:
                perror("error happens");
        }
    }
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "the file is wrong!");
        exit(0);
    }
    int** cache_pointer = create_cache();
    start_simulate(cache_pointer, f);
    free_cache();
    printSummary(hit_times, miss_times, eviction_times);
    return 0;
}
