#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <pthread.h>


void * udpthread(void *) {
    int sfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buf[BUFSIZ];

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    bind(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    client_len = sizeof(client_addr);

    while (1) {
        bzero(buf, BUFSIZ);
        int n = recvfrom(sfd, buf, BUFSIZ, 0, (struct sockaddr *) &client_addr, &client_len);

        for (int i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }

        sendto(sfd, buf, n, 0, (struct sockaddr *) &client_addr, client_len);
    }

    return NULL;
}

int main()
{
    int lfd, cfd;

    pthread_t manual_switch_tid, get_local_status_tid;
    int ret = pthread_create(&manual_switch_tid, NULL, udpthread, NULL);


    lfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    bzero(&client_addr, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(lfd, 128);

    socklen_t socklen = 0;
    cfd = accept(lfd, (struct sockaddr *)&client_addr, &socklen);

    while(true) {
        char buf[BUFSIZ] = {0};
        int n = read(cfd, buf, BUFSIZ);

        for (int i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }

        n = write(cfd, buf, n);
    }


    std::cout << "Hello, World!" << std::endl;
    return 0;
}


// nc 127.0.0.1 8888
// nc -u 127.0.0.1 8888