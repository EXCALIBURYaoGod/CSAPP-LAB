# ShellLab

Shell Lab 要求实现一个带有作业控制的 Unix Shell 程序，需要考虑基础的并发，进程控制以及信号和信号处理。做这个实验之前一定要将 CSAPP 的第八章（异常控制流）仔细研读。

需要我们实现的函数：

- `eval`：解析命令行 **[约 70 行]**
- `builtin_cmd`：检测是否为内置命令`quit`、`fg`、`bg`、`jobs`**[约 25 行]**
- `do_bgfg`：实现内置命令`bg`和`fg`**[约 50 行]**
- `waitfg`：等待前台作业执行完成 **[约 20 行]**
- `sigchld_handler`：处理`SIGCHLD`信号，即子进程停止或者终止 **[约 80 行]**
- `sigint_handler`：处理`SIGINT`信号，即来自键盘的中断`ctrl-c`**[约 15 行]**
- `sigtstp_handler`：处理`SIGTSTP`信号，即来自终端的停止信号 **[约 15 行]**

作者提供的帮助函数及功能：

```c
/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);  //解析命令行参数，如果后台运行则返回 1
void sigquit_handler(int sig);

void clearjob(struct job_t *job);               //清除job结构体
void initjobs(struct job_t *jobs);              //初始化jobs列表
int maxjid(struct job_t *jobs);                 //返回jobs列表中jid最大值
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);    //在jobs列表中添加job
int deletejob(struct job_t *jobs, pid_t pid);   //在jobs列表中删除pid对应的job
pid_t fgpid(struct job_t *jobs);                //返回前台运行的job的pid
struct job_t *getjobpid(struct job_t *jobs, pid_t pid); //返回对应pid的job
struct job_t *getjobjid(struct job_t *jobs, int jid);   //返回对应jid的job
int pid2jid(pid_t pid);     //返回对应pid的job的jid
void listjobs(struct job_t *jobs);  //打印jobs列表

void usage(void);   //帮助信息
void unix_error(char *msg); //错误信息
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
```

CMU 文档的一些说明与要求：

- Shell 的提示标记符为 "tsh> "

- 用户键入的命令行应包括命令名称和 0 个或多个参数，所有参数以一个或多个空格分隔。如果 name 是一个内置命令，则 tsh 应该立即处理它并等待下一个命令行。否则，tsh 假定该名称是一个可执行程序的路径，并在在一个初始子进程的上下文中加载并运行

- tsh 不需要支持管道（|）或 I/O 重定向（< 和 >）

- 键入 ctrl-c（ctrl-z）应该会发送一个 SIGINT（SIGTSTP）信号给当前的前台作业以及它的子进程。如果没有前台作业，则该信号应该没有任何作用

- 如果命令行以 ＆ 结束，则 tsh 应该在后台运行该作业。否则，它应该在前台运行

- 每个作业都可以由一个进程 ID（PID）或一个作业 ID（JID）标识，该 ID 是一个由 tsh 分配的正整数。 JID应该在命令行上以前缀 “％” 表示。例如，“％5” 表示 JID 5，“ 5” 表示 PID 5

- tsh 应该支持以下内置命令：

- - quit：终止 tsh 程序。
  - jobs：列出所有后台作业
  - bg ：向作业发送 SIGCONT 信号来重新启动 ，然后在后台运行
  - fg ：向作业发送 SIGCONT 信号来重新启动 ，然后在前台运行

- tsh 必须回收所有的僵死进程

## 预备理论

### 回收子进程

一个终止了但是还未被回收的进程称为僵死进程。对于一个长时间运行的程序（比如 Shell）来说，内核不会安排`init`进程去回收僵死进程，而它虽不运行却仍然消耗系统资源，因此实验要求我们回收所有的僵死进程。

`waitpid`是一个非常重要的函数，一个进程可以调用`waitpid`函数来等待它的子进程终止或停止，从而回收子进程，在本实验大量用到，我们必须学习它的用法：

这个函数用来挂起调用进程的执行，直到`pid`对应的等待集合的一个子进程的改变才返回，包括三种状态的改变：

- 子进程终止
- 子进程收到信号停止
- 子进程收到信号重新执行

如果一个子进程在调用之前就已经终止了，那么函数就会立即返回，否则，就会阻塞，直到一个子进程改变状态。

等待集合以及监测那些状态都是用函数的参数确定的，函数定义如下：

```c
pid_t waitpid(pid_t pid, int *wstatus, int options);
```

**各参数含义及使用:**

- **pid：判定等待集合成员**

- - pid > 0 : 等待集合为 pid 对应的单独子进程
  - pid = -1: 等待集合为所有的子进程
  - pid < -1: 等待集合为一个进程组，ID 为 pid 的绝对值
  - pid = 0 : 等待集合为一个进程组，ID 为调用进程的 pid

- **options：修改默认行为**

- - WNOHANG：集合中任何子进程都未终止，立即返回 0
  - WUNTRACED：阻塞，直到一个进程终止或停止，返回 PID
  - WCONTINUED：阻塞，直到一个停止的进程收到 SIGCONT 信号重新开始执行
  - 也可以用或运算把 options 的选项组合起来。例如 WNOHANG | WUNTRACED 表示：立即返回，如果等待集合中的子进程都没有被停职或终止，则返回值为 0；如果有一个停止或终止，则返回值为该子进程的 PID

- **statusp：检查已回收子进程的退出状态**

- - waitpid 会在 status 中放上关于导致返回的子进程的状态信息。

### 并发编程原则

这里仅列出在本实验中用到的原则，后面的解析也会大量提及

1. 注意保存和恢复 errno。很多函数会在出错返回时设置 errno，在处理程序中调用这样的函数可能会干扰主程序中其他依赖于 errno 的部分，解决办法是在进入处理函数时用局部变量保存它，运行完成后再将其恢复
2. 访问全局数据时，阻塞所有信号。这里很容易理解，不再解释了
3. 不可以用信号来对其它进程中发生的事情计数。未处理的信号是不排队的，即每种类型的信号最多只能有一个待处理信号。**举例**：如果父进程将要接受三个相同的信号，当处理程序还在处理一个信号时，第二个信号就会加入待处理信号集合，如果此时第三个信号到达，那么它就会被简单地丢弃，从而出现问题
4. 注意考虑同步错误：竞争

## job

```
struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
```

本实验中一个job对应一个线程，即一个jid只有一个pid，在linux或unix系统中，一个jid通常对应于多个pid

## builtin_cmd

功能：判断是否为内置命令，是的话立即执行并返回1，不是则返回0

思路：判断argv[0]

```
int builtin_cmd(char **argv) 
{
    if (!strcmp(argv[0], "quit"))
    {
        exit(0);
    }
    else if (!strcmp(argv[0], "jobs"))
    {
        listjobs(jobs);
        return 1;
    }
    else if (!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg"))
    {
        do_bgfg(argv);
        return 1;
    }
    else if (!strcmp(argv[0], "&"))
    {
        return 1;
    }
    return 0;     /* not a builtin command */
}
```

## eval

功能：解析命令是内置命令还是其他命令，是前台作业还是后台作业

思路：

1. 首先通过parseline函数将命令拆分到argv中，并获取是前台命令还是后台命令

2. 然后通过后面实现的buildin_cmd函数判断是内置命令还是其他命令
3. 如果是内置，则buildin_cmd可以直接实现
4. 如果不是，则首先要屏蔽SIGCHLD信号：因为如果在 execve执行前，fork的子进程已经结束，但父进程还没有来得及处理 SIGCHLD 信号，新程序就可能在一个 “已经结束的” 子进程中开始运行，这会导致状态混乱
5. 屏蔽后就fork一个子进程用execve执行该命令
6. 然后父进程还要判断是否是前台命令，如果是的话需要用waitpid等待，否则输出后台信息
7. 最后注意在调用addjob函数时需要阻塞所有信号，防止在运行addjob时子进程运行完成了

```
void eval(char *cmdline) 
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    int state;
    pid_t pid;
    sigset_t mask_all, mask_one, prev_one;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    state = bg? BG : FG;

    if (argv[0] == NULL)
    {
        return;
    }

    if (!builtin_cmd(argv))
    {
        sigfillset(&mask_all);
        sigemptyset(&mask_one);
        sigaddset(&mask_one, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
        
        if ((pid = fork()) == 0) //若为子进程则执行
        {
            sigprocmask(SIG_SETMASK, &prev_one, NULL); //解除SIGCHLD阻塞
            setpgid(0, 0);
            execve(argv[0], argv, environ);
            exit(0);
        }
        if (state == FG)
        {
            sigprocmask(SIG_BLOCK, &mask_all, NULL);  //阻塞所有信号
            addjob(jobs, pid, state, cmdline);  
            sigprocmask(SIG_SETMASK, &mask_one, NULL); // 恢复至阻塞SIGCHLD
            waitfg(pid); //等待前台进程完成
        }
        else //后台
        {
            sigprocmask(SIG_BLOCK, &mask_all, NULL);
            addjob(jobs, pid, state, cmdline);
            sigprocmask(SIG_SETMASK, &mask_one, NULL);
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline); //输出后台job的jid、pid
        }
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
    }

    return;
}
```

## do_bgfg

功能：实现fg和bg内置命令

思路：

1. 区分是fg函数还是bg函数

2. 判断是使用的jid还是pid，并解析出id
3. 使用getjob函数找到该作业
4. 向该作业发送一个SIGCONT信号，用于让被停止（例如，因为接收到了 SIGSTOP、SIGTSTP、SIGTTIN或SIGTTOU 信号）的进程或进程组继续运行
5. 然后判断是fg命令否，是的话就用将要实现的waitfg等待该进程执行完，是bg的话输出信息

```
void do_bgfg(char **argv) 
{
    struct job_t *job = NULL;
    int state;
    int id;

    if (!strcmp(argv[0], "bg"))
    {
        state = BG;
    }
    else
    {
        state = FG;
    }

    if (argv[1] == NULL)
    {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }

    if (argv[1][0] == '%')
    {
        if (sscanf(&argv[1][1], "%d", &id) > 0)
        {
            job = getjobjid(jobs, id);
            if (job == NULL)
            {
                printf("%%%d: No such job\n", id);
                return;
            }
        }
    }
    else if (!isdigit(argv[1][0]))
    {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }
    else
    {
        id = atoi(argv[1]);
        job = getjobpid(jobs, id);
        if (job == NULL)
        {
            printf("(%d): No such process\n", id);
            return;
        }
    }
    kill(-(job->pid), SIGCONT);
    job->state = state;
    if (state == BG)
    {
        printf("[%d] (%d) %s",job->jid, job->pid, job->cmdline);
    }
    else
    {
        waitfg(job->pid);
    }

    return;
}

```

## waitfg

功能：等待前台作业完成

思路：一直循环等待子进程结束，这时候fgpid将会返回0

```
void waitfg(pid_t pid)
{
    sigset_t mask;
    sigemptyset(&mask);
    while (fgpid(jobs) != 0)
    {
        sigsuspend(&mask);
    }
    return;
}
```

## sigchld_handler

功能：处理sigchld信号。这个信号是一个由操作系统向父进程发送的信号，用于通知父进程其子进程的状态已经改变。状态改变可能包括子进程终止、停止或者继续运行，这里我们只需要处理暂停和终止

思路：

1. 使用waitpid等待所有子进程，如果有任意的子进程状态改变了，那就处理
2. 判断是正常退出，还是因为信号而终止了，还是因为信号而暂停了
3. WNOHANG 选项使得 waitpid() 在没有子进程结束时不会阻塞，而是立即返回 0。WUNTRACED 选项使得 waitpid() 在子进程被停止时也会返回
4. WIFEXITED：一个宏定义，检查子进程是否正常结束
5. WIFSIGNALED：一个宏定义，用于检查子进程是否因为接收到一个未被捕获的信号而终止
6. WIFSTOPPED：一个宏定义，用于检查子进程是否被停止

```
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    int status;
    pid_t pid;
    struct job_t *job;
    sigset_t mask_all, prev;
    sigfillset(&mask_all);

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) 
    {
        sigprocmask(SIG_BLOCK, &mask_all, &prev);
        if (WIFEXITED(status))
        {
            deletejob(jobs, pid);   //只要进程退出或终止都删除这个job
        }
        else if (WIFSIGNALED(status))
        {
            printf ("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));
            deletejob(jobs, pid);
        }
        else if (WIFSTOPPED(status))
        {
            printf ("Job [%d] (%d) stoped by signal %d\n", pid2jid(pid), pid, WSTOPSIG(status));
            job = getjobpid(jobs, pid);
            job->state = ST;
        }
        sigprocmask(SIG_SETMASK, &prev, NULL);          
    }
    errno = olderrno;
    return;
}
```

## sigint_handler

功能：处理ctrl-c，即SIGINT信号

思路：就是判断如果还有前台任务，则用kill向其发送一个SIGINT信号终止该进程即可

```
void sigint_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    sigset_t mask_all, prev;
    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, &prev);
    
    if ((pid = fgpid(jobs)) != 0)
    {
        sigprocmask(SIG_SETMASK, &prev, NULL);
        kill(-pid, SIGINT);
    }
    errno = olderrno;
    return;
}
```

## sigtstp_handler

功能：处理ctrl-z，即SIGTSTP信号

思路：与sigint_handler一样

```
void sigtstp_handler(int sig) 
{
    int olderrno = errno;
    pid_t pid;
    sigset_t mask_all, prev;
    sigfillset(&mask_all);
    sigprocvmask(SIG_BLOCK, &mask_all, &prev);
    
    if ((pid = fgpid(jobs)) != 0)
    {
        sigprocmask(SIG_SETMASK, &prev, NULL);
        kill(-pid, SIGSTOP);
    }
    errno = olderrno;
    return;
}
```

## 测试

![image-20240715171339989](..\imgs\image-20240715171339989.png) 

![](..\imgs\image-20240715171419974.png) 