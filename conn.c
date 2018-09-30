/*************************************************************************
>          
>          File Name: conn.c
>          
*************************************************************************/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stddef.h>
#include <syslog.h>

#include "libconn.h"

#define	 CLI_PATH	"/var/tmp/"
#define	 QLEN	        16

//needed if module is use into library
#define  USE_MUTEX  1

#if USE_MUTEX
  //use mutex to protect tempnam & file creation
  static pthread_mutex_t mutexSock = PTHREAD_MUTEX_INITIALIZER;
# define MUTEX_LOCK      pthread_mutex_lock(&mutexSock)
# define MUTEX_UNLOCK    pthread_mutex_unlock(&mutexSock)
#else
# define MUTEX_LOCK      
# define MUTEX_UNLOCK    
#endif

#define NO_DEBUG  1
//#define DEBUG_PRINTF

#if NO_DEBUG
#    define _TRACE_DBG(f, ...)     do {} while(0)
#    define _TRACE_ERR(f, ...)     syslog(LOG_LOCAL1 | LOG_ERR,   "%s: "f, __FUNCTION__, __VA_ARGS__)
#else
#  ifdef DEBUG_PRINTF 
#    define _TRACE_ERR(f, ...)     fprintf(stderr, "%s: "f"\n", __FUNCTION__, __VA_ARGS__)
#    define _TRACE_DBG(f, ...)     fprintf(stdout, "%s: "f"\n", __FUNCTION__, __VA_ARGS__)
#  else
#    define _TRACE_ERR(f, ...)     syslog(LOG_LOCAL1 | LOG_ERR,   "%s: "f, __FUNCTION__, __VA_ARGS__)
#    define _TRACE_DBG(f, ...)     syslog(LOG_LOCAL1 | LOG_DEBUG, "%s: "f, __FUNCTION__, __VA_ARGS__)
#  endif
#endif

/*
 * use instead of sleep 
 * to avoid SIG_ALARM
 */
static void wait_mdelay(int ms)
{
   struct timeval t;
   do
   {
      //small delays => reload in case of interrupt
      t.tv_sec = ms/1000;
      t.tv_usec = (ms%1000)*1000;
   }
   while ((select(0,NULL,NULL,NULL,&t) != 0) && (errno == EINTR)); 
}

/*
 * ************************************************************************
 * ************************************************************************
 * SERVER PART
 * ************************************************************************
 * ************************************************************************
 */
static int conn_server_listen(const char * sockName)
{
    int fd = -1;
    int len, err, ret;
    struct sockaddr_un local; //相当于注定TCP 中IP 端口，在这里指的是路径名

    if(strlen(sockName) >= sizeof(local.sun_path))
    {
        _TRACE_ERR("name %s too long!", sockName);
        errno = ENAMETOOLONG;
        return CONN_ERR;
    }

    //fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    /*
	AF_UNIX：AF_UNIX则只能用于本机内进程之间的通信
	依赖路径名标识发送方和接收方。即发送数据时，指定接收方绑定的路径名，操作系统根据该路径名可以直接找到对应的接收方，
	并将原始数据直接拷贝到接收方的内核缓冲区中，并上报给接收方进程进行处理。同样的接收方可以从收到的数据包中获取到发送方的路径名，
	并通过此路径名向其发送数据。
	SOCK_STREAM TCP

	*/
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        _TRACE_ERR("socket failed (%s, %d)!", sockName, errno);
        ret = CONN_ERR_SERVER_SOCKET;
        goto errout;
    }
	/* unlink：不能删除目录，剩下的和rm 一模一样*/
    unlink(sockName);    //In case the file exists //删除传入的文件？？   

    //Local addr init
    memset(&local, 0, sizeof(local)); 
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, sockName);
	/*求出这个MEMBER在该结构体的中的位置，size_t 就是int，取该成员的地址，再强制转化为int，会
的得到一个以字节为单位的值；
	typedef struct test
	{
	        int i;
	        int j;
	        char * a;
	}T;

	int ret = offsetof(T,a);//ret--->8
	sockaddr_un：
		sun_family，sun_path；（只有两个参数）
	从上面的例子可以看出来。再加上MEMBER的长度，就是整个结构体的长度了
	*/
    len = offsetof(struct sockaddr_un, sun_path) + strlen(sockName); 

    //Bind
    if(bind(fd, (struct sockaddr *)&local, len) < 0)
    {
        _TRACE_ERR("bind failed (%s, %d)!", sockName, errno);
        ret = CONN_ERR_SERVER_BIND;
        goto errout;
    }

    //Listen        
    if(listen(fd, QLEN) < 0)
    {
        _TRACE_ERR("listen failed (%s, %d)!", sockName, errno);
        ret = CONN_ERR_SERVER_LISTEN;
        goto errout;
    }

    return fd;

errout:
    err = errno;
    if (fd >= 0) close(fd);
    errno = err;
    return ret;
}

static int conn_server_accept(int listenfd)
{
    int    clifd;
    socklen_t    len;
    struct sockaddr_un    un;
    char * name; 

    name = malloc(sizeof(un.sun_path) + 1);
    if(NULL == name)
    {
        return -1;
    }
    
    len = sizeof(un);
    if((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
    {
        _TRACE_ERR("accept failed (%d)!", errno);
        free(name);
        return CONN_ERR_SERVER_ACCEPT;
    }
    
    len -= offsetof(struct sockaddr_un, sun_path);
    memcpy(name, un.sun_path, len);
    name[len] = 0;

    unlink(name);    //Do not need it anymore
    free(name);

    return(clifd); 
}


void conn_server_loop_request(const char *sockName, connProcessFunc func, int maxCmdLen)
{
    int maxfd, listenfd;
    fd_set allset;

    //domain socked here
    /*FD_ZERO宏完成的工作就是一个初始化套接字集合（其实就是清空套接字集合），就你给出的程序而言，
    FD_ZERO在循化外循环内都是一样的。不过一般来讲，初始化服务端的所有套接字组成的集合就应该把FD_ZERO
    放在循环外，而初始化具有可读或者可写属性的套接字集合就应该把FD_ZERO放在循环内部。因为服务端一旦
    启动服务线程，就会一直处于循环状态，每一次循环完成都应该将具有可读可写属性的套接字集合清空。*/
    FD_ZERO(&allset);  //有初始化作用

    /* open server listener */
    while(1) 
    {
        listenfd = conn_server_listen(sockName); //创建一个socket，并绑定该文件路径，并监听它
        if(listenfd < 0)
        {
            static int failed = 0;
            if (0 == failed) {
               failed = 1;
               _TRACE_ERR("conn server listen failed (%s, %d)!", sockName, errno);
            }
            wait_mdelay(1000);
            continue;
       }
       else break;
    }
    _TRACE_DBG("server listen: %s fd=%d", sockName, listenfd);

    // init fd set with server fd
    FD_SET(listenfd, &allset);  //添加一个
    // init max fd num
    maxfd = listenfd;   //（int）

    for(;;)
    {
        int clifd, nread, n;
        fd_set rset = allset;

        while (((n=select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) && (errno == EINTR));
        if(n < 0)
        {
            static int failed = 0;
            if (0 == failed) {
               _TRACE_ERR("select failed (%s, %d)!", sockName, errno);
               failed = 1;
            }
            wait_mdelay(1000);
            continue;
        }

        // find fd client from fd=3 (skip 0:stdin, 1:stdout, 2:stderr)
        for (clifd=3; clifd<=maxfd; clifd++)
        {
            if(FD_ISSET(clifd, &rset))
            {
                if (clifd == listenfd)
                {
                    //Client connection
                    int fd;
                    if((fd = conn_server_accept(listenfd)) < 0)
                    {
                        _TRACE_ERR("conn server accept failed (%s, %d)!", sockName, errno);
                        wait_mdelay(1000);
                        continue;
                    }

                    // add fd to set
                    FD_SET(fd, &allset);
                    // new max ?
                    if(fd > maxfd)
                    {
                        maxfd = fd;
                    }
                    _TRACE_DBG("Client connected to server %s (%d): client %d (max=%d)\n", sockName, listenfd, fd, maxfd);
                }
                else
                {
                    char *cmd = malloc(maxCmdLen);
                    //Receive message from Client
                    _TRACE_DBG("Client read: %s fd=%d ....", sockName, clifd);
                    if((nread = recv(clifd, cmd, maxCmdLen, MSG_NOSIGNAL)) <= 0)
                    {
                        // remove fd from set
                        _TRACE_DBG("Client disconected: %s fd=%d ....", sockName, clifd);
                        FD_CLR(clifd, &allset);
                        close(clifd);
                    }
                    else
                    {
                        char *out = NULL;

                        // callback to process cmd
                        n = (*func)(cmd, nread, &out);
                        if ((n > 0) && (out != NULL))
                        {
                           _TRACE_DBG("writing response to client: %s fd=%d len=%d....", sockName, clifd, n);
                           n = send(clifd, out, n, MSG_NOSIGNAL);
                           _TRACE_DBG("write done to client: %s fd=%d (n=%d)", sockName, clifd, n);
                           free(out);
                        }
                    }
                    free(cmd);
                }
                break; //exit from secondary loop
            }
        }
    }
} 

/*
 * ************************************************************************
 * ************************************************************************
 * CLIENT PART
 * ************************************************************************
 * ************************************************************************
 */
static int conn_client_connect(const char *sockName, char *sockId)
{
    int fd, len, err, ret;
    struct sockaddr_un local, dest;
    int do_unlink = 0;
    char *tmp = NULL;

    if(strlen(sockName) > sizeof(dest.sun_path))
    {
        errno = ENAMETOOLONG;
        return CONN_ERR;
    }

    //fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (fd < 0)
    {
        _TRACE_ERR("socket failed (%s, %d)!", sockName, errno);
        return CONN_ERR;
    }

    //Local addr init
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_LOCAL;
    MUTEX_LOCK;
    if (tmp = tempnam(CLI_PATH, (sockId) ? sockId : "conn."))
    {
       sprintf(local.sun_path, "%s.%ld", tmp, (long)getpid());
       free(tmp); 
       tmp = NULL;
    }
    else
    {
       sprintf(local.sun_path, CLI_PATH"%sdummy.%ld", (sockId) ? sockId : "conn.", (long)getpid());
    }
    _TRACE_DBG("local.sun_path=%s\n", local.sun_path);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(local.sun_path);

    //Bind
    unlink(local.sun_path);        //In case the file exists
    ret = bind(fd, (struct sockaddr *) &local, len);
    MUTEX_UNLOCK;
    if (ret < 0)
    {
        _TRACE_ERR("bind failed (%s, %d)!", sockName, errno);
        ret = CONN_ERR_CLIENT_BIND;
        do_unlink = 1;
        goto errout;
    }

    //Fill and connect server
    memset(&dest, 0, sizeof(dest));
    dest.sun_family = AF_LOCAL;
    strcpy(dest.sun_path, sockName);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(sockName);
    ret = connect(fd, (struct sockaddr *)&dest, len);
    if(ret < 0)
    {
        _TRACE_ERR("connect failed (%s, %d)!", sockName, errno);
        ret = CONN_ERR_CLIENT_CONNECT;
        do_unlink = 1;
        goto errout;
    }

    return fd;

errout:
    err = errno;
    close(fd);
    if(do_unlink)
        unlink(local.sun_path);
    errno = err;
    return ret; 
}

int conn_client_request(const char *sockName, const char *sockId, const char *cmd, int lenCmd, char *resp, int maxResp, int timeout)
{
    int ret = CONN_OK;
    int csfd = -1;
    int nread, nwrite, n;
    fd_set rset;
    struct timeval to;

    //Connet to server
    csfd = conn_client_connect(sockName, sockId);
    if(csfd < 0)
    {
        _TRACE_ERR("connect failed (%s, %d)!", sockName, csfd);
        ret = CONN_ERR_CLIENT_CONNECT;
        goto send_out;
    }

    // send request
    n = lenCmd;
    if ((nwrite = send(csfd, cmd, n, MSG_NOSIGNAL)) != n)
    {
       _TRACE_ERR("send failed (%s, %d)", sockName, nwrite);
       ret = CONN_ERR_CLIENT_SEND;
       goto send_out;
    }

    // get response if requested
    ret = 0;
    if ((resp != NULL) && (maxResp > 0))
    {
       to.tv_sec = timeout;  
       to.tv_usec = 0;
       FD_ZERO(&rset);
       FD_SET(csfd, &rset);
       while (((n = select(csfd+1, &rset, NULL, NULL, &to)) < 0) && (errno == EINTR));
       if (n <= 0)
       {
           _TRACE_ERR("select failed (%s, %d)!\n", sockName, errno);
           ret = CONN_ERR_CLIENT_SELECT;
           goto send_out;
       }
       _TRACE_DBG("select done (%s, %d)!\n", sockName, csfd);

       if(FD_ISSET(csfd, &rset))
       {
           if((nread = recv(csfd, resp, maxResp, MSG_NOSIGNAL)) < 0)
           {
               _TRACE_ERR("recv failed (%s, %d)!\n", sockName, errno);
               ret = CONN_ERR_CLIENT_RECV;
               goto send_out;
           }
           _TRACE_DBG("recv done (%s, %d)!\n", sockName, nread);
           ret = nread;
       }
    }

send_out:
    if (csfd >= 0) close(csfd);
    _TRACE_DBG("returns %d\n", ret);
    return ret;
}

int conn_client_debug_request(const char *sockName, const char *sockId, const char *cmd, char *resp, int maxResp, int timeout)
{
    int ret;

    // add +1 to cmd  len to send ending null char
    // add -1 to resp len to add ending null char
    ret = conn_client_request(sockName, sockId, cmd, strlen(cmd)+1, resp, maxResp-1, timeout);
    if ((ret >= 0) && (resp != NULL) && (maxResp != 0))
    {
        // add ending null char
        resp[maxResp-1]=0;
    }
    return ret;
}
