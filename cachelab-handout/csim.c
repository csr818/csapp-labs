#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#define addrlen 32
static int s; // 2^s个组
static int E; // 组里有E个行
static int b; // block位数为b，不用真的在行里包含数据内容
char filename[1000];

// 不能简单的在行中数据处存一int，如果使用o(n)的方式实现lru
static int hit_times, miss_times, eviction_times;

typedef struct {
    int timestamp; // 多久没被访问
    unsigned int line_info; // 低addrlen - s - b位记录的是tag 更高一位是valid位
} cache_line;


cache_line** create_cache() {
    // 每行用一个数字表示
    int S = 1 << s;
    cache_line** cache = (cache_line**) malloc(sizeof(cache_line*) * S);
    for (int i = 0; i < S; i++) {
        cache[i] = (cache_line*) malloc(sizeof(cache_line) * E);
    }
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j].timestamp = 0;
            cache[i][j].line_info = 0;
        }
    }
    return cache;
}


int valid(unsigned int info) {
    return (info >> (addrlen-s-b)) & 1;
}

void do_command(cache_line** cache,char type, unsigned int address, int size) {
    unsigned int tag = address >> (b + s); // >> 优先级高
    int S_idx = (address >> b) & ((1 << s) - 1); // 也可以写成这样(address >> b) & ((-1U) >> (addrlen - s));
    int taglen = addrlen - s - b;
    // 当前组内所有的行都不包含这个值而且行满了

    unsigned int cache_date;
    // 找是否存在cache中
    for (int i = 0; i < E; i++) {
        cache_date = cache[S_idx][i].line_info;
        if ((cache_date & ((1 << taglen) - 1)) == tag && valid(cache_date)!=0) {
            ++hit_times;
            cache[S_idx][i].timestamp = 0;
            return ;
        } 
    } 

    // 不在cache中，检查cache中是否存在空行
    for (int i = 0; i < E; i++) {
        cache_date = cache[S_idx][i].line_info;
        if (valid(cache_date) == 0) {
            ++miss_times;
            cache[S_idx][i].line_info = tag | (1 << taglen); // 设置cache中的值为有效位为1，tag是当前地址的tag
            cache[S_idx][i].timestamp = 0;
            return ;
        }
    }

    // 不在cache中也不存在空行，那么需要对这个组里进行行的替换
    int max_stamp = -1;
    int max_idx = -1;
    for (int i = 0; i < E; i++) {
        if (cache[S_idx][i].timestamp > max_stamp) 
            max_idx = i, max_stamp = cache[S_idx][i].timestamp;
    }
    ++miss_times, ++eviction_times;
    cache[S_idx][max_idx].timestamp = 0;
    cache[S_idx][max_idx].line_info = tag | (1 << taglen);
}

void update_timestamp(cache_line** cache) {
    int S = 1 << s;
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            if (valid(cache[i][j].line_info)) cache[i][j].timestamp += 1;
        }
    }
}

void start_simulate(cache_line** cache, FILE* f) {
    char type;
    unsigned int address; // 地址应该是无符号数
    int size;
    while (3 == fscanf(f, " %c %x,%d", &type, &address, &size)) {
        switch (type) {
            case 'L':
                do_command(cache, type, address, size);
                break;
            case 'M':
                do_command(cache, type, address, size);
            case 'S':
                do_command(cache, type, address, size);
                break;
        }
        update_timestamp(cache);
    }
}

void free_cache(cache_line** cache) {
    int S = 1 << s;
    for (int i = 0; i < S; i++) {
        free(cache[i]);
    }
    free(cache);
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
    cache_line** cache_pointer = create_cache();
    start_simulate(cache_pointer, f);
    free_cache(cache_pointer);
    fclose(f);
    printSummary(hit_times, miss_times, eviction_times);
    return 0;
}