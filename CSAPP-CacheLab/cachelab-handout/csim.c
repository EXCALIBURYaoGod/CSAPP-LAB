#include "cachelab.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef char filename;
int result[3] = {0};
enum Category {HIT, MISS, EVICTION};
int T = 0;  //全局的一个时刻表，第一个访存指令的时刻为0，之后每次更新时都累加1，更新当前正在执行的缓存行的时间戳
int s;  //组索引位数 S = 2 ^ s
int E;  //每个组的高速缓存行数
int b;  //块偏移位数
int verbose = 0;    //标识是否需要详细输出内存执行情况
filename t[20];
int occupancy[1024] = {0}; //1024个set组
char category_string[3][20] = {"hit", "miss", "eviction"};

typedef struct
{
    bool valid; //有效位
    unsigned long tag;  //标志位
    int timestamp;  //时间戳
}cache_line;    //一个高速缓存行

//读入命令行参数e、s、b、t，然后分配内存，创建高速缓存
cache_line** create_cache(int argc, char** argv);
//读取文件t中的内存操作，获得地址映射的组数和标志位，作为参数向cache_simulator中传递
void get_trace(cache_line** cache);
//根据地址映射的组数set、标志位tag，在缓存寻找这个地址的内存块
void cache_simulate(cache_line** cache, int set, unsigned long tag);
//在高速缓存模拟过程中，更新缓存行cache_set[pos]，并计数（命中、不命中、替换次数）
void update(cache_line* cache_set, enum Category category, int pos, int tag);
//LRU替换策略，选择最后被访问的时间距现在最远的块（CSAPP p423页）,col是二维结构体的列数，也是一个组中高速缓存行的数量
int LRU_replace(cache_line* cache_set);
//释放内存
void destory(cache_line** cache);

cache_line** create_cache(int argc, char** argv){
    int opt;
    while(-1 != (opt = getopt(argc, argv, "vs:E:b:t:"))){
        switch(opt){
            case 'v':
                verbose = 1;    //设置verbose为1，表示详细输出缓存过程
                break;
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
                strcpy(t, optarg);
                break;
            default:
                break;  //程序健壮性检验，如果不是一个合法的参数，就会退出switch,继续while读取
        }
    }

    int row = pow(2, s);
    int col = E;
    cache_line** cache = (cache_line**)malloc(row * sizeof(cache_line*));  //动态内存分配
    if(cache == NULL)
    {
        printf("Failed to allocate memory!\n");
        exit(1);
    }
    for(int i = 0; i < row; i ++){
        cache[i] = (cache_line*)malloc(col * sizeof(cache_line));
        if(cache[i] == NULL)
        {
            printf("Failed to allocate memory!\n");
            exit(1);
        }
    }
    //初始化，有效位为0，时间戳为0
    for(int i = 0; i < row; i ++){
        for(int j = 0; j < col; j ++){
            cache[i][j].valid = 0;
            cache[i][j].timestamp = 0;
        }
    }
    return cache;
}

void get_trace(cache_line** cache){
    FILE *fp = fopen(t, "r");
    if(fp == NULL){
        perror("Error opening file");
        exit(1);
    }

    char operation;
    unsigned long addr;
    int bytes; 
    int set;
    unsigned long tag;

    while(fscanf(fp, " %c %lx,%d", &operation, &addr, &bytes) == 3){
        set = (addr>>b) & (unsigned long)((1<<s)-1);
        tag = addr >> (b+s);
        switch(operation)
        {
            case 'L':
            case 'S':
                if(verbose) printf("%c %lx,%d ", operation, addr, bytes);
                cache_simulate(cache, set, tag);
                if(verbose) printf("\n");
                break;
            case 'M':
                if(verbose) printf("%c %lx,%d ", operation, addr, bytes);
                cache_simulate(cache, set, tag);
                cache_simulate(cache, set, tag);
                if(verbose) printf("\n");
                break;
            default:
                break;
        }
    }
}

void cache_simulate(cache_line** cache, int set, unsigned long tag){
    bool find = false;   //标识是否缓存命中
    int col = E;
    int pos = 0;
    //缓存命中
    for(int j = 0; j < col; j ++){
        if(cache[set][j].valid == 1 && cache[set][j].tag == tag){
            pos = j;
            update(cache[set], HIT, pos, tag);
            find = true;
            break;
        }
    }
    //缓存未命中，先用一个数组occupancy表示cache[set]中缓存行数目
    if(!find){
        if(occupancy[set] != E){
            occupancy[set] ++;
            for(int j = 0; j < col; j ++){
                if(cache[set][j].valid == 0){   
                    pos = j;
                    update(cache[set], MISS, pos, tag); //将空缓存行替换为目标数据块
                    break;
                }
            }
        }else{
            pos = LRU_replace(cache[set]);  //若都有效，则用LRU策略替换
            update(cache[set], MISS, pos, tag);
            update(cache[set], EVICTION, pos, tag);
        }
    }
}

void update(cache_line* cache_set, enum Category category, int pos, int tag){

    result[category] ++;
    printf("%s ", category_string[category]);
    cache_set[pos].tag = tag;
    cache_set[pos].valid = 1;
    cache_set[pos].timestamp = T;
    T ++;
}

//遍历得出时间戳最小的缓存行pos
int LRU_replace(cache_line* cache_set){
    int min = cache_set[0].timestamp;
    int pos = 0;
    for(int j = 1; j < E; j ++){
        if(cache_set[j].timestamp < min){
            pos = j;
            min = cache_set[j].timestamp;
        }
    }
    return pos;
}

//释放内存
void destory(cache_line** cache){
    int row = pow(2, s);
    for(int i = 0; i < row; i ++){
        free(cache[i]);
    }
    free(cache);
}

int main(int argc, char** argv)
{ 
    cache_line** cache = create_cache(argc, argv);
    get_trace(cache);
    destory(cache);
    printSummary(result[0], result[1], result[2]);
    return 0;
}                                                                                                                                                                                                    