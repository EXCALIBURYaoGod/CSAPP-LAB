# ProxyLab

## 预备知识

CSAPP Unix IO：

RIO接口

![image-20240730155324767](..\imgs\image-20240730155324767.png) 

```
#include "csapp.h"

ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd); 
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

//unbuffered
ssize_t rio_readn(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n; //剩余要读的字节数
    ssize_t nread; //每次要读的字节数
    char *bufp = usrbuf; //读入字节要存入的位置指针

    while (nleft > 0) {
	if ((nread = read(fd, bufp, nleft)) < 0) { //调用read读入最大nleft个字节，返回实际读取的字节数
	    if (errno == EINTR) /* 判断是否被信号中断 */
		nread = 0;      /* 若中断则重新读 */
	    else
		return -1;      /* 读取错误 */ 
	} 
	else if (nread == 0)
	    break;              /* 文件结束EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* 返回与要求读的字节数 - 剩余要读的字节数 = 实际已经读的字节数*/
}
//unbuffered 与read类似
ssize_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* Interrupted by sig handler return */
		nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}
//buffered read
/*
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;            内部缓冲区的文件描述符
    int rio_cnt;           缓冲区中的字节数
    char *rio_bufptr;      缓冲区中指向下一个要读的字节
    char rio_buf[RIO_BUFSIZE];  缓冲区字符串数组
}rio_t; 
*/
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* 检查缓冲区是否为空，若为空则重新填充 */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) { //robust机制，若被中断信号终止则继续read
	    if (errno != EINTR) /* 若出错的不是EINTR则返回-1 */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* 指向读入字符串的起始处 */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt; //若缓冲区的字节数 < 要读的字节数 则把缓冲区中的字节全部读入 
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt; //返回读取的字节数
}
void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
	if ((nread = rio_read(rp, bufp, nleft)) < 0) 
        if (errno != EINTR) /* Interrupted by sig handler return */
            nread = 0;
		else return -1;         /* errno set by read() */ 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break;    /* EOF, some data was read */
        } else
            return -1;    /* Error */
    }
    *bufp = 0; //此时bufp指向该行字符串结尾，因此要添加\0表示结束
    return n - 1;
}

```

 

客户端-服务器模型：

![img](../imgs/image.png)  

应用程序网络模型：

![image-20240726135830304](..\imgs\image-20240726135830304.png) 

- ip地址是大端字节序，主机地址不一定
- 一个域名可以对应多个ip地址

套接字接口网络应用模型：

![image-20240726140207655](..\imgs\image-20240726140207655.png) 

### TinyWebsever 解析

将一个实际的浏览器指向我们自己的服务器，看着它显示一个复杂的带有文本和图片的Web页面，真是颇具成就感~

下面就来看看它的实现全过程：

### **1. Tiny-main程序**

```
int main(int argc, char **argv)
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
   
    /* Check command line args */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);
   
    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        doit(connfd);                                             //line:netp:tiny:doit
        Close(connfd);                                            //line:netp:tiny:close
    }
}
```

Tiny是一个迭代服务器，监听在命令行中确定的端口上的连接请求。在通过open_listenedfd函数打开一个监听套接字以后，Tiny执行典型的无限服务循环，反复地接受一个连接(accept)请求，执行事务(doit)，最后关闭连接描述符(close)。

### **2. Tiny-doit函数**

```
/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
   
    /* Read request line and headers */
    Rio_readinitb(&rio, fd);    //Associate a descriptor with a read buffer and reset buffer
    Rio_readlineb(&rio, buf, MAXLINE);                   //line:netp:doit:readrequest
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (strcasecmp(method, "GET"))  //ignore case        //line:netp:doit:beginrequesterr
    {
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr
    read_requesthdrs(&rio);                              //line:netp:doit:readrequesthdrs
   
    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);       //line:netp:doit:staticcheck
    if (stat(filename, &sbuf) < 0)                       //line:netp:doit:beginnotfound
    {
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }                                                    //line:netp:doit:endnotfound
   
    if (is_static)   /* Serve static content */
    {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))   //line:netp:doit:readable
        {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);        //line:netp:doit:servestatic
    }
    else   /* Serve dynamic content */
    {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))   //line:netp:doit:executable
        {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);            //line:netp:doit:servedynamic
    }
}
/* $end doit */
```

**解析：**

rio_readinitb(&rio,fd) ：将程序的内部缓存区与描述符相关联。

rio_readlineb(&rio,buf,MAXLINE) :从内部缓存区读出一个文本行至buf中，以null字符来结束这个文本行。当然，每行最大的字符数量不能超过MAXLINE。

sscanf(buf,"%s %s %s",method,uri,version) :作为例子，一般此时buf中存放的是“GET / HTTP/1.1”,所以可知method为“GET”，uri为“/”，version为“HTTP/1.1”。其中sscanf的功能：把buf中的字符串以空格为分隔符分别传送到method、uri及version中。

strcasecmp(method,"GET") :忽略大小写比较method与“GET”的大小，相等的话返回0。

read_requesthdrs(&rio) :读并忽略请求报头。

parse_uri(uri,filename,cgiargs) :解析uri，得文件名存入filename中，参数存入cgiargs中。

stat(filename,&sbuf) :将文件filename中的各个元数据填写进sbuf中，如果找不到文件返回0。

S_ISREG(sbuf,st_mode) :此文件为普通文件。

S_IRUSR & sbuf.st_mode :有读取权限。

serve_static(fd,filename,sbuf.st_size) :提供静态服务。

serve_dynamic(fd,filename,cgiargs) :提供动态服务。

（1）从doit函数中可知，我们的Tiny Web服务器只支持“GET”方法，其他方法请求的话则会发送一条错误消息，主程序返回，并等待下一个请求。否则，我们读并忽略请求报头。（其实，我们在请求服务时，直接不用写请求报头即可，写上只是为了符合HTTP协议标准）。

（2）然后，我们将uri解析为一个文件名和一个可能为空的CGI参数，并且设置一个标志位，表明请求的是静态内容还是动态内容。通过stat函数判断文件是否存在。

（3）最后，如果请求的是静态内容，我们需要检验它是否是一个普通文件，并且可读。条件通过，则我们服务器向客服端发送静态内容；相似的，如果请求的是动态内容，我就核实该文件是否是可执行文件，如果是则执行该文件，并提供动态功能。

### **3. Tiny的clienterror函数**

```
/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];
   
    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
   
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */
```

向客户端返回错误信息。

sprintf(buf,"------------"):将字符串“------------”输送到buf中。

rio_writen(fd,buf,strlen(buf)):将buf中的字符串写入fd描述符中。

### **4. Tiny的read_requesthdrs函数**

```
/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
   
    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n"))            //line:netp:readhdrs:checkterm
    {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */
```

Tiny不需要请求报头中的任何信息，所以我们这个函数就是来跳过这些请求报头的。具体做法就是读这些请求报头，直到空行，然后返回。OK！

### **5. Tiny的parse_uri函数**

```
/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
   
    if (!strstr(uri, "cgi-bin")) {  /* Static content */ //line:netp:parseuri:isstatic
        strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
        strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1
        strcat(filename, uri);                           //line:netp:parseuri:endconvert1
        if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
            strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
        return 1;
    }
    else {  /* Dynamic content */                        //line:netp:parseuri:isdynamic
        ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
        if (ptr)
        {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
        strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
        strcat(filename, uri);                           //line:netp:parseuri:endconvert2
        return 0;
    }
}
/* $end parse_uri */
```

根据uri中是否含有cgi-bin来判断请求的是静态内容还是动态内容。如果没有cgi-bin，则说明请求的是静态内容。那么，我们需把cgiargs置NULL，然后获得文件名，如果我们请求的uri最后为 “/”，则自动添加上home.html。比如说，我们请求的是“/”,则返回的文件名为“./home.html”,而我们请求“/logo.gif”,则返回的文件名为“./logo.gif”。如果uri中含有cgi-bin，则说明请求的是动态内容。那么，我们需要把参数拷贝到cgiargs中，把要执行的文件路径写入filename。举例来说，uri为/cgi-bin/adder?3&5,则cigargs中存放的是3&5，filename中存放的是“./cgi-bin/adder”,OK!

index(uri,'?') : 找出uri字符串中第一个出现参数‘？’的地址，并将此地址返回。

### **6. Tiny的serve_static函数**

```
/*
 * serve_static - copy a file back to the client
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
   
    /* Send response headers to client */
    get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));       //line:netp:servestatic:endserve
   
    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//line:netp:servestatic:mmap
    Close(srcfd);                           //line:netp:servestatic:close
    Rio_writen(fd, srcp, filesize);         //line:netp:servestatic:write
    Munmap(srcp, filesize);                 //line:netp:servestatic:munmap
}
   
/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}
/* $end serve_static */
```

打开文件名为filename的文件，把它映射到一个虚拟存储器空间，将文件的前filesize字节映射到从地址srcp开始的虚拟存储区域。关闭文件描述符srcfd，把虚拟存储区的数据写入fd描述符，最后释放虚拟存储器区域。

### **7. Tiny的serve_dynamic函数**

```
/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };
   
    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
   
    if (Fork() == 0) { /* child */ //line:netp:servedynamic:fork
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2
        Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
    }
    Wait(NULL); /* Parent waits for and reaps child */ //line:netp:servedynamic:wait
}
/* $end serve_dynamic */
```

Tiny在发送了响应的第一部分后，通过派生一个子进程并在子进程的上下文中运行一个cgi程序（可执行文件），来提供各种类型的动态内容。

setenv("QUERY_STRING",cgiargs,1) :设置QUERY_STRING环境变量。

dup2 （fd，STDOUT_FILENO) ：重定向它的标准输出到已连接描述符。此时，任何写到标准输出的东西都直接写到客户端。

execve(filename,emptylist,environ) :加载运行cgi程序。

## 多线程、带缓存的Web代理服务器

#### Proxy.c

```
#include "csapp.h"
#include "sbuf.h"
#include "cache.h"

/* Recommended max cache and object sizes */
#define WEB_PREFIX "http://"
#define NTHREADS 4
#define SBUFSIZE 16
#define NO_EXIST_IN_CACHE 0
#define EXIST_IN_CACHE 1
#define CR "\r\n"
#define READER_SAME_TIME 3

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

static sbuf_t sbuf; /* Shared buffer of connected descriptors */
static cache_t cache;
static int cacheSize; /* gloabl variable to check the cache size */
static int readcnt;
static sem_t mutex, W;

void* thread(void* vargp);
void handleRequest(int);
void clientError(int , char* , char* , char* , char* );
int readAndFormatRequestHeader(rio_t* , char* , char*, char* , char* , char* , char*, char*);
int checkGetMethod(char* , char* , char* );
void replaceHTTPVersion(char* );
void parseLine(char* , char*, char* , char* , char* , char*, char*);
void readAndWriteResponse(int , rio_t*, char* uri);
void writeToCache(obj_t* );
obj_t* readItem(char* , int );

void handleRequest(int fd){
    /* the argument used to parse the first line of http request */
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], fileName[MAXLINE];

    /* client request header and body */
    char clientRequest[MAXLINE];

    /* request header */
    char host[MAXLINE], port[MAXLINE];

    /* IO for proxy--client,  proxy-server */
    rio_t rio, rioTiny;

    /** step1: read request from client */
    Rio_readinitb(&rio, fd);
    if(Rio_readlineb(&rio, buf, MAXLINE) == 0){
        //the request has the empty space
        printf("empty request\n");
        return;
    }
  
    /* if start with 1.1, we change it to 1.0 */
    replaceHTTPVersion(buf);

    /* parse one line request line into several parts */
    parseLine(buf, host, port, method, uri, version, fileName);
    
    /** step2: determine if it is a valid request */
    if(strcasecmp(method, "GET") != 0){
        clientError(fd, method, "501", "Not Implemented", "Tiny Does not implement this method");
        return;
    }

    int rvv = readAndFormatRequestHeader(&rio, clientRequest, host, port, method, uri, version, fileName);
    if(rvv == 0){ // bad request, ignore it
        return;
    }
    
    printf("========= $$BEGIN we have formatted the reqeust into $$=========\n");
    printf("%s", clientRequest);
    printf("========= $$END we have formatted the reqeust into $$=========\n");

    obj_t* rv = readItem(uri, fd);
    if(rv != NULL){ // we send it using cache
        printf("======we are using cache to send the response back ====\n");

        Rio_writen(fd, rv->respHeader, rv->respHeaderLen);
        Rio_writen(fd, CR, strlen(CR));
        Rio_writen(fd, rv->respBody  , rv->respBodyLen);
        return;
    }
    /** step3: establish own connection with tiny server
     *         and forward the request to the client
     * localhost:1025
     * */
    char hostName[100];
    char* colon = strstr(host, ":");
    strncpy(hostName, host, colon - host);
    int clientfd = Open_clientfd(hostName, port);

    Rio_readinitb(&rioTiny, clientfd);
    Rio_writen(rioTiny.rio_fd, clientRequest, strlen(clientRequest));

    /** step4: read the response from tiny and send it to the client */
    printf("---prepare to get the response---- \n");
    // int n; char tinyResponse[MAXLINE];
    // while( (n = Rio_readlineb(&rioTiny, tinyResponse, MAXLINE)) != 0){
    //     Rio_writen(fd, tinyResponse, n);
    // }
    readAndWriteResponse(fd, &rioTiny, uri);
}

void readAndWriteResponse(int fd, rio_t* rioTiny, char* uri){
    char tinyResponse[MAXLINE];
    int n, totalBytes = 0;
    //new response
    obj_t* obj = Malloc(sizeof(*obj));
    obj->flag = '0';
    strcpy(obj->uri, uri);
    *obj->respHeader = 0;
    *obj->respBody   = 0;

    printf("obj->url == %s\n", obj->uri);

    while( (n = rio_readlineb(rioTiny, tinyResponse, MAXLINE)) != 0){
        Rio_writen(fd, tinyResponse, n);

        if(strcmp(tinyResponse, "\r\n") == 0) // prepare for body part
            break;

        strcat(obj->respHeader, tinyResponse);
        totalBytes += n;
    }

    obj->respHeaderLen = totalBytes;
    totalBytes = 0;

    while( (n = rio_readlineb(rioTiny, tinyResponse, MAXLINE)) != 0){
        Rio_writen(fd, tinyResponse, n);
        totalBytes += n;
        strcat(obj->respBody, tinyResponse);
    }

    obj->respBodyLen = totalBytes;
    //check if the cache is too large
    if(totalBytes >= MAX_OBJECT_SIZE){
        Free(obj);
        return;
    }

    printf("In reading Responsse, we have cache the follwing item\n");
    printf("======= response header ========\n");
    printf("%s", obj->respHeader);
    printf("======= response body ==========\n");
    printf("%s", obj->respBody);

    /* try to read current capacity and write into it */
    P(&W);
    writeToCache(obj);
    printf("=======writng this object in to cache=========\n");
    V(&W);
}


int readAndFormatRequestHeader(rio_t* rio, char* clientRequest, char* Host, char* port,
                        char* method, char* uri, char* version, char* fileName){
    int UserAgent = 0, Connection = 0, ProxyConnection = 0, HostInfo = 0;
    char buf[MAXLINE / 2];
    int n;

    /* 1. add GET HOSTNAME HTTP/1.0 to header && Host Info */
    sprintf(clientRequest, "GET %s HTTP/1.0\r\n", fileName);

    n = Rio_readlineb(rio, buf, MAXLINE);
    char* findp;
    while(strcmp("\r\n", buf) != 0 && n != 0){
        strcat(clientRequest, buf);

        if( (findp = strstr(buf, "User-Agent:")) != NULL){
            UserAgent = 1;
        }else if( (findp = strstr(buf, "Proxy-Connection:")) != NULL){
            ProxyConnection = 1;
        }else if( (findp = strstr(buf, "Connection:")) != NULL){
            Connection = 1;
        }else if( (findp = strstr(buf, "Host:")) != NULL){
            HostInfo = 1;
        }

        n = Rio_readlineb(rio, buf, MAXLINE);
    }

    if(n == 0){
        return 0;
    }

    if(HostInfo == 0){
        sprintf(buf, "Host: %s\r\n", Host);
        strcat(clientRequest, buf);
    }

    /** append User-Agent */
    if(UserAgent == 0){
        strcat(clientRequest, user_agent_hdr);
    }
    
    /** append Connection */
    if(Connection == 0){
        sprintf(buf, "Connection: close\r\n");
        strcat(clientRequest, buf);
    }
    
    /** append Proxy-Connection */
    if(ProxyConnection == 0){
        sprintf(buf, "Proxy-Connection: close\r\n");
        strcat(clientRequest, buf);
    }

    /* add terminator for request */
    strcat(clientRequest, "\r\n");
    return 1;
}


void replaceHTTPVersion(char* buf){
    char* pos = NULL;
    if( (pos = strstr(buf, "HTTP/1.1")) != NULL){
        buf[pos - buf + strlen("HTTP/1.1") - 1] = '0';
    }
}

/**
 * @param: client request is like this
 *          GET http://www.cmu.edu/hub/index.html HTTP/1.1
 * @function: we will parse that into differnt parameters
 */
void parseLine(char* buf, char* host, char* port, char* method, char* uri, char* version, char* fileName){
    sscanf(buf, "%s %s %s", method, uri, version);
    //method = "GET", uri = "http://localhost:15213/home.html", version = "HTTP1.0"
    
    char* hostp = strstr(uri, WEB_PREFIX) + strlen(WEB_PREFIX);
    char* slash = strstr(hostp, "/");
    char* colon = strstr(hostp, ":");
    //get host name
    strncpy(host, hostp, slash - hostp);
    //get port number
    strncpy(port, colon + 1, slash - colon - 1);
    //get file name
    strcpy(fileName, slash);
}   

void clientError(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg){
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

void* thread(void* vargp){
    Pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        handleRequest(connfd);
        Close(connfd);
    }
}

obj_t* readItem(char* targetURI, int clientfd){
    P(&mutex);
    readcnt++;
    if(readcnt == 1){
        P(&W);
    }
    V(&mutex);

    /***** reading section starts *****/
    obj_t* cur = cache.head->next;
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    while(cur->flag != '@'){
        if(strcmp(targetURI, cur->uri) == 0){
            return cur;
        }

        cur = cur->next;
    }


    /***** reading section ends *****/
    P(&mutex);
    readcnt--;
    if(readcnt == 0){
        V(&W);
    }
    V(&mutex);

    return NULL;
}

/*
 * This function is guarded by Write Lock, thus is thread safe
 * assume head is the newest part, we evict the last part
 * if possible
 */
void writeToCache(obj_t* obj){
    /* step1: check current capacity, if full ,delete one */
    while(obj->respBodyLen + cacheSize > MAX_CACHE_SIZE && cache.head->next != cache.tail){
        obj_t* last = cache.tail->prev;
        last->next->prev = last->prev;
        last->prev->next = last->next;

        last->next = NULL;
        last->prev = NULL;
        Free(last);
    }

    /* step2: add into the cache */
    //mount the current obj into cache
    obj->next = cache.head->next;
    obj->prev = cache.head;
    cache.head->next->prev = obj;
    cache.head->next       = obj;
    cacheSize += obj->respBodyLen;
}



int main(int argc, char** argv){
    if(argc != 2){
        unix_error("proxy usage: ./proxy <port number>");
    }

    int listenfd = Open_listenfd(argv[1]), i;
    pthread_t tid;
    struct sockaddr_storage clientAddr;
    char hostName[MAXLINE], port[MAXLINE];
    sbuf_init(&sbuf, SBUFSIZE);

    for(i = 0; i < NTHREADS; i++){	//创建线程池
        Pthread_create(&tid, NULL, thread, NULL);
    }
    /* initialize the cache */ //创建缓存池
    cacheSize = 0;
    initializeCache(&cache);

    /* initialize the read / write lock*/
    readcnt = 0;
    Sem_init(&mutex, 0, READER_SAME_TIME);
    Sem_init(&W    , 0, 1);

    while(1){
        socklen_t addrLen = sizeof(struct sockaddr_storage);
        int connfd = Accept(listenfd, (SA*)&clientAddr, &addrLen);
        Getnameinfo((SA*)&clientAddr, addrLen, hostName, MAXLINE, port, MAXLINE, 0);
        printf("Accepting Connection from (%s, %s) \n", hostName, port);
        sbuf_insert(&sbuf, connfd);
    }
}
```

#### sbuf.c

```
/* $begin sbufc */
#include "csapp.h"
#include "sbuf.h"

/* Create an empty, bounded, shared FIFO buffer with n slots */
/* $begin sbuf_init */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int)); 
    sp->n = n;                       /* Buffer holds max of n items */
    sp->front = sp->rear = 0;        /* Empty buffer iff front == rear */
    Sem_init(&sp->mutex, 0, 1);      /* Binary semaphore for locking */
    Sem_init(&sp->slots, 0, n);      /* Initially, buf has n empty slots */
    Sem_init(&sp->items, 0, 0);      /* Initially, buf has zero data items */
}
/* $end sbuf_init */

/* Clean up buffer sp */
/* $begin sbuf_deinit */
void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}
/* $end sbuf_deinit */

/* Insert item onto the rear of shared buffer sp */
/* $begin sbuf_insert */
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->items);                          /* Announce available item */
}
/* $end sbuf_insert */

/* Remove and return the first item from buffer sp */
/* $begin sbuf_remove */
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);                          /* Wait for available item */
    P(&sp->mutex);                          /* Lock the buffer */
    item = sp->buf[(++sp->front)%(sp->n)];  /* Remove the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->slots);                          /* Announce available slot */
    return item;
}
/* $end sbuf_remove */
/* $end sbufc */
```

#### sbuf.h

```
#include "csapp.h"

/* $begin sbuft */
typedef struct {
    int *buf;          /* Buffer array */         
    int n;             /* Maximum number of slots */
    int front;         /* buf[(front+1)%n] is first item */
    int rear;          /* buf[rear%n] is last item */
    sem_t mutex;       /* Protects accesses to buf */
    sem_t slots;       /* Counts available slots */
    sem_t items;       /* Counts available items */
} sbuf_t;
/* $end sbuft */

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);
```

#### cache.c

```
#include "csapp.h"
#include "cache.h"

void initializeCache(cache_t* cache){
    cache->head = Malloc(sizeof(*(cache->head)));
    cache->head->flag = '@';
    cache->head->prev = NULL;
    cache->head->next = NULL;

    cache->tail = Malloc(sizeof(*(cache->tail)));
    cache->tail->flag = '@';
    cache->tail->prev = NULL;
    cache->tail->next = NULL;

    /* construct the doubly linked list */
    cache->head->next = cache->tail;
    cache->tail->prev = cache->head;

    cache->nitems = 0;
}
```

#### cache.h

```
#include "csapp.h"
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct _obj_t{
    char flag;
    char uri[100];
    char respHeader[1024];
    char respBody[MAX_OBJECT_SIZE];
    int respHeaderLen;
    int respBodyLen;
    struct _obj_t* prev;
    struct _obj_t* next;
}obj_t;

typedef struct _cache_t{
    obj_t* head;
    obj_t* tail;    
    int nitems;
}cache_t;

//write to cache
//read cache
//search cache

void initializeCache(cache_t* );
```

