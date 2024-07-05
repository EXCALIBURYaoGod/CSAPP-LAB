#include "cachelab.h"
#include <getopt.h>

int T = 0;  //全局的一个时刻表，第一个访存指令的时刻为0，之后每次更新时都累加1，更新当前正在执行的缓存行的时间戳
int s;  //组索引位数 S = 2 ^ s
int E;  //每个组的高速缓存行数
int b;  //块偏移位数
int verbose = 0;    //标识是否需要详细输出内存执行情况

typedef struct
{
    bool valid; //有效位
    unsigned long tag;  //标志位
    int timestamp;  //时间戳
}cache_line;    //一个高速缓存行

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
    cache_line** cache = (cache_line)malloc(row * sizeof(cache_line*));  //动态内存分配
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
        for(j = 0; j < col; j ++){
            cache[i][j].valid = 0;
            cache[i][j].timestamp = 0;
        }
    }
    return cache;
}

void get_trace(cache_line** cache){
    FILE *fp = fopen(t, "r");
    if(fp == NULL){
        perror("Error opening file")
        exit(1);
    }
}

int main()
{ 
    printSummary(0, 0, 0);
    return 0;
}
                                                                                                                                                                                                                                                          