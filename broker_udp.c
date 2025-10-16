#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF 2048
#define MAX_SUBS 256

typedef struct { struct sockaddr_in addr; int active; } sub_t;

int main(void){
    int fd = socket(AF_INET, SOCK_DGRAM, 0); if(fd<0){perror("socket");return 1;}
    struct sockaddr_in srv; memset(&srv,0,sizeof(srv));
    srv.sin_family=AF_INET; srv.sin_addr.s_addr=INADDR_ANY; srv.sin_port=htons(PORT);
    if (bind(fd,(struct sockaddr*)&srv,sizeof(srv))<0){ perror("bind"); return 1; }

    sub_t subs[MAX_SUBS]; for(int i=0;i<MAX_SUBS;i++) subs[i].active=0;

    printf("Broker UDP escuchando en %d...\n", PORT);
    char buf[BUF];
    while(1){
        struct sockaddr_in cli; socklen_t clen=sizeof(cli);
        int n = recvfrom(fd, buf, BUF-1, 0, (struct sockaddr*)&cli, &clen);
        if (n<=0) continue;
        buf[n]='\0';

        if (!strncmp(buf,"SUB",3)){
            int placed=0;
            for (int i=0;i<MAX_SUBS;i++){
                if (!subs[i].active){ subs[i].addr=cli; subs[i].active=1; placed=1; break; }
            }
            printf("[UDP] SUB de %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
            const char* ok="OK SUB\n"; sendto(fd, ok, strlen(ok), 0, (struct sockaddr*)&cli, clen);
        } else if (!strncmp(buf,"PUB:",4)){
            const char* msg = buf+4;
            printf("[UDP] PUB difundir: %s", msg);
            for (int i=0;i<MAX_SUBS;i++){
                if (subs[i].active){
                    sendto(fd, msg, strlen(msg), 0, (struct sockaddr*)&subs[i].addr, sizeof(subs[i].addr));
                }
            }
            const char* ack="OK PUB\n"; sendto(fd, ack, strlen(ack), 0, (struct sockaddr*)&cli, clen);
        }
    }
    close(fd);
    return 0;
}

