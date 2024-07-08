

# CacheLab

实验目的：

1、理解高速缓存cache，掌握内存操作指令，理解LRU策略。

2、掌握最小化高速缓存的不命中的次数。

文件说明：

![image-20240705145311562](../imgs/image-20240705145311562.png) 

实验分为两部分：

Part A要求根据traces中的内存操作文件，编写一个高速缓存模拟器（csim.c）

Part B要求优化矩阵转置函数，最小化高速缓存的不命中的次数（trans.c）

缓存存入内存地址的原理：

![image-20240705151750305](../imgs/image-20240705151750305.png) 



## Part A

### 预备理论

![image-20240705150127579](../imgs/image-20240705150127579.png) 

本实验中采用的是Set associative形式(set为1时就是Direct mapped)存储数据，其存储结构如下：

![image-20240705150605685](../imgs/image-20240705150605685.png) 

在本实验中，内存操作格式为：`[space]operation address, size`

operation 有 4 种：

- `I` 表示加载指令，表示从内存中读取指令内容
- `L`加载数据，表示程序从某个内存地址加载了数据。（缓存命中、不命中、执行替换策略）
- `S` 存储数据，表示程序向某个内存地址存储了数据。（写回、写分配）
- `M` 修改数据，表示程序对某个内存地址的数据进行了修改。

不需要考虑加载指令`I`,`M`指令相当于先进行`L`再进行`S`，模拟器要做出的反馈有 3 种：

- `hit`：命中，表示要操作的数据在对应组的其中一行
- `miss`：不命中，表示要操作的数据不在对应组的任何一行
- `eviction`：驱逐，表示要操作的数据的对应组已满，进行了替换操作

### PartA的主要操作

编写csim.c，这个程序的执行效果要与csim-ref相同，能够模拟一个高速缓存器（参数自定义），执行traces/xx.trace中的内存操作过程。这个模拟器不需要真的存储数据，只是计算traces/xx.trace的内存操作过程中，缓存的命中、不命中、LRU替换的数量，然后将这些参数作为答案，传给printSummary函数。

### 思路

1、创建高速缓存：这包括从命令行中读取参数信息，初始化缓存
2、读入内存操作：就是读取xx.trace文件中每行的内容
3、根据读入的内存操作，模拟高速缓存的行为：核心代码，这包括在高速缓存中查找地址所指示的字，对不命中的处理（是加载到一个空的缓存行还是需要执行LRU替换策略）。每次执行一次缓存(caching)，就更新缓存(cache)信息（有效位、标志位、时间戳），同时统计hit、miss、eviction

#### 创建高速缓存  

利用二维结构体数组实现cache line定义：

```
typedef struct
{
    bool valid; //有效位
    unsigned long tag;  //标志位
    int timestamp;  //时间戳
}cache_line;    //一个高速缓存行
```

从Linux命令行中读取参数信息，并初始化cache：

```
cache_line** create_cache(int argc, char** argv){
    int opt;
    while(-1 != (opt = getopt(argc, argv, "vs:E:b:t:"))){
        switch(opt){
            case 'v':
                verbose = 1;    //设置verbose为1，表示详细输出缓存过程
                break;
            case 's':
                s = atoi(optarg); //组数 2 ^ s
                break;
            case 'E':
                E = atoi(optarg);	//缓存行数
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
```

#### 读入.trace文件中的内存操作

**读取文件`t`的每一行**，对指令进行解析，然后获取地址映射的组索引、标记，用于后续在模拟高速缓存时，在高速缓存中查找这个地址的内容

使用课件中建议的`fscanf`函数，读取文件`t`的每一行指令。

```
void get_trace(cache_line** cache){
    FILE *fp = fopen(t, "r");
    if(fp == NULL){
        perror("Error opening file")
        exit(1);
    }

    char operation;
    unsigned long addr;
    int bytes; n 
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
```

#### 模拟高速缓存

根据从内存操作中由地址映射的组索引 `set`、标记`tag`，模拟缓存过程

为了简化参数，我使用一个数组`result[3]`来存放`hit`、`miss`、`eviction`的次数。

为了记录缓存行`cache[i][j]`最后被执行的时间，即**设置时间戳**`cache[i][j].timestamp`，我使用一个全局变量`T`作为整体的时间。初始`T`设置为`0`，每当进行一次缓存(caching)，就要对`T`加`1`，这样当需要进行LRU替换时，我们**遍历查找这个组，驱逐时间戳最小的缓存行**。恰好我们在每一次缓存(caching)后，使用`update`函数更新缓存(cache)的信息，所以当调用`update`函数时，就意味着进行了一次缓存(caching)，因此可以在`update`函数中对`T`加`1`，更新整体的时间。

##### 行匹配

遍历检查缓存行`cache[set][j]`的有效位和标记位，以确定地址中的字是否在缓存中。如果找到了一个有效的行`cache[set][pos]`，它的标记与地址中的标记`tag`相匹配，则**缓存命中**；若遍历了所有的行都不匹配，则为**缓存不命中**

  ```
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
                  }else{
                      pos = LRU_replace(cache[set]);  //若都有效，则用LRU策略替换
                      update(cache[set], MISS, pos, tag);
                      update(cache[set], EVICTION, pos, tag); //缓存EVICTION
                  }
              }
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
  ```

### 测试

![image-20240708164915018](../imgs/image-20240708164915018.png) 

## Part B

### 预备理论

- 矩阵分块转置技术：将一个矩阵分为不同大小的块再进行转置。
- C语言中二维数组在内存中是逐行连续存储的，读取`A[0][0]`时，`A[0][1]`,`A[0][2]`等也会按行被加载进缓存，但是写入`B[0][0]`时，`B[1][0]`，`B[2][0]`并不在相邻的内存地址块中，因此可能造成缓存不命中。

### Part B的主要操作

在`trans.c`中，使用分块技术优化矩阵转置函数，处理三个不同的输入`32 * 32`,`64 * 64`,`61 * 67`，你要将优化后的函数写到`transpose_submit`函数里。

### 实验说明

1、只运行使用最多12个int局部变量

2、不能使用递归函数

3、不能对原始的矩阵A进行修改

4、不能通过malloc申请空间

测试的高速缓存架构为`s = 5, E = 1, b = 5`

`b = 5`表示每个缓冲行能存32个字节，即8个int数据（c语言）

