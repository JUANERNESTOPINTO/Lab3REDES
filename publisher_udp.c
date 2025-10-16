#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF 2048

int main(void){
    int fd = socket(AF_INET, SOCK_DGRAM, 0); if(fd<0){perror("socket");return 1;}
    struct sockaddr_in srv; memset(&srv,0,sizeof(srv));
    srv.sin_family=AF_INET; srv.sin_port=htons(PORT); inet_pton(AF_INET,"127.0.0.1",&srv.sin_addr);

    const char* payload="PUB:Gol de Equipo A al minuto 32\n";
    sendto(fd, payload, strlen(payload), 0, (struct sockaddr*)&srv, sizeof(srv));

    char buf[BUF]; int n = recvfrom(fd, buf, BUF-1, 0, NULL, NULL);
    if (n>0){ buf[n]='\0'; printf("%s", buf); }
    close(fd); return 0;
}

