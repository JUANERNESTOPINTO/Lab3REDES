#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF 2048

static void must(int ok, const char* msg){ if(!ok){ perror(msg); exit(1);} }

int main(void){
    int s = socket(AF_INET, SOCK_STREAM, 0); must(s>=0,"socket");
    struct sockaddr_in srv; memset(&srv,0,sizeof(srv));
    srv.sin_family=AF_INET; srv.sin_port=htons(PORT); inet_pton(AF_INET,"127.0.0.1",&srv.sin_addr);
    must(connect(s,(struct sockaddr*)&srv,sizeof(srv))==0,"connect");

    send(s, "SUB\n", 4, 0);
    printf("Subscriber conectado. Esperando...\n");
    char buf[BUF];
    while (1){
        int n = recv(s, buf, BUF-1, 0);
        if (n<=0) break;
        buf[n]='\0';
        printf("[SUB] %s", buf);
        fflush(stdout);
    }
    close(s);
    return 0;
}

