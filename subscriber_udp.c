#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF 2048

int main(void){
    int fd = socket(AF_INET, SOCK_DGRAM, 0); if(fd<0){perror("socket");return 1;}

    // Bind primero (puerto efímero local)
    struct sockaddr_in local; memset(&local,0,sizeof(local));
    local.sin_family=AF_INET; local.sin_addr.s_addr=INADDR_ANY; local.sin_port=0;
    if (bind(fd,(struct sockaddr*)&local,sizeof(local))<0){ perror("bind"); return 1; }

    struct sockaddr_in srv; memset(&srv,0,sizeof(srv));
    srv.sin_family=AF_INET; srv.sin_port=htons(PORT); inet_pton(AF_INET,"127.0.0.1",&srv.sin_addr);

    // Registrarse
    const char* sub="SUB";
    sendto(fd, sub, strlen(sub), 0, (struct sockaddr*)&srv, sizeof(srv));

    char buf[BUF];
    int n = recvfrom(fd, buf, BUF-1, 0, NULL, NULL); // OK SUB
    if (n>0){ buf[n]='\0'; printf("%s", buf); }

    printf("Subscriber UDP esperando...\n");
    while(1){
        n = recvfrom(fd, buf, BUF-1, 0, NULL, NULL);
        if (n<=0) continue;
        buf[n]='\0'; printf("[SUB-UDP] %s", buf); fflush(stdout);
    }
    close(fd);
    return 0;
}

