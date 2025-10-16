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

    // Enviar rol y mensaje en 1 o 2 sends, funciona igual
    const char *hdr="PUB\n";
    const char *msg="Gol de Equipo A al minuto 32\n";
    send(s, hdr, strlen(hdr), 0);
    usleep(10*1000); // pequeña pausa para ver ambos caminos
    send(s, msg, strlen(msg), 0);

    char buf[BUF]; int n=recv(s,buf,BUF-1,0);
    if(n>0){ buf[n]='\0'; printf("[PUB] ACK: %s", buf); }
    close(s);
    return 0;
}

