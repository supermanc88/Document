# socket

Linux文件的一种类型：伪文件



socket一定是成对出现的

socket通信过程中一定要绑定IP和Port

Linux内核实现：一个文件描述符指向两个缓冲区，一个读，一个写



创建socket之后  



文件描述符 fd



有两个缓冲区：

1. 数据读入
2. 数据写出

![image-20200526152658929](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200526152658929.png)



## 网络字节序

网络数据流采用大端字节序



大端：低地址 -- 高位

小端：低地址 -- 低位



网络字节序和本地字节序的转换

- htonl

- htons

- ntohl

- ntohs



## IP地址转换函数

- inet_pton
- inet_ntop

![image-20200526133900745](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200526133900745.png)



## sockaddr 数据结构

sockaddr---->sockaddr_in

```c
           struct sockaddr_in {
               sa_family_t    sin_family; /* address family: AF_INET */
               in_port_t      sin_port;   /* port in network byte order */
               struct in_addr sin_addr;   /* internet address */
           };

           /* Internet address. */
           struct in_addr {
               uint32_t       s_addr;     /* address in network byte order */
           };
```



![image-20200526135120907](https://raw.githubusercontent.com/supermanc88/ImageSources/master/image-20200526135120907.png)



bind

accept

connect

涉及上面类型转换的问题





## 网络套接字函数

- socket

```c
       #include <sys/socket.h>

       sockfd = socket(int socket_family, int socket_type, int protocol);
```

成功，返回指向新创建的socket文件描述符，失败返回-1

- bind

```c
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);
```

- listen

**不是用来监听端口的，是用来限制同时有多少个客户端建立连接**

- accept

```c
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

成功返回一个**新的**socket文件描述符，用于和客户端通信，失败返回-1，设置errno

- connect

```c
       #include <sys/types.h>          /* See NOTES */
       #include <sys/socket.h>

       int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen);
```





服务器：

socket --> bind --> listen --> accept（阻塞直到有客户端连接）

socket 建立套接字

bind 绑定本地ip和端口号

listen 最大同时建立数

accept 等待客户端发起连接

read 

write给客户端

close

客户端：

socket --> connect

socket创建套接字

bind也可以调用，指定使用端口，不调用时，隐式绑定

connect连接服务器

write

read

close