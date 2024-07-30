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