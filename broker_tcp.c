#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS FD_SETSIZE
#define BUF 4096

typedef enum { UNKNOWN=0, SUBSCRIBER=1, PUBLISHER=2 } role_t;

int main(void){
    int listen_fd, yes=1;
    struct sockaddr_in addr;
    int clients[MAX_CLIENTS]; role_t roles[MAX_CLIENTS];
    for (int i=0;i<MAX_CLIENTS;i++){ clients[i]=-1; roles[i]=UNKNOWN; }

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ perror("socket"); exit(1); }
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr))<0){ perror("bind"); exit(1); }
    if (listen(listen_fd, 32)<0){ perror("listen"); exit(1); }

    printf("Broker TCP escuchando en puerto %d...\n", PORT);

    while (1){
        fd_set rfds; FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        int maxfd = listen_fd;

        for (int i=0;i<MAX_CLIENTS;i++){
            if (clients[i]!=-1){ FD_SET(clients[i], &rfds); if (clients[i]>maxfd) maxfd=clients[i]; }
        }

        int ready = select(maxfd+1, &rfds, NULL, NULL, NULL);
        if (ready < 0){ perror("select"); break; }

        if (FD_ISSET(listen_fd, &rfds)){
            int cfd = accept(listen_fd, NULL, NULL);
            if (cfd>=0){
                int placed=0;
                for(int i=0;i<MAX_CLIENTS;i++){
                    if (clients[i]==-1){ clients[i]=cfd; roles[i]=UNKNOWN; placed=1; break; }
                }
                if(!placed){ close(cfd); }
                else { printf("[BROKER] Nueva conexión fd=%d\n", cfd); }
            }
            if (--ready<=0) continue;
        }

        for (int i=0;i<MAX_CLIENTS;i++){
            int fd = clients[i]; if (fd==-1) continue;
            if (!FD_ISSET(fd, &rfds)) continue;

            char buf[BUF];
            int n = recv(fd, buf, BUF-1, 0);
            if (n<=0){ printf("[BROKER] Cierre fd=%d\n", fd); close(fd); clients[i]=-1; roles[i]=UNKNOWN; continue; }
            buf[n]='\0';

            if (roles[i]==UNKNOWN){
                // Esperamos "SUB\n" o "PUB\n" y quizás MÁS datos en el mismo recv
                char *nl = strchr(buf, '\n');
                if (!nl){
                    // Asegura que al menos role header llegue con '\n'
                    printf("[BROKER] Header sin newline; cerrando fd=%d\n", fd);
                    close(fd); clients[i]=-1; roles[i]=UNKNOWN; continue;
                }
                size_t header_len = (size_t)(nl - buf + 1);
                if (!strncmp(buf, "SUB\n", 4)){
                    roles[i]=SUBSCRIBER;
                    printf("[BROKER] fd=%d registrado como SUB\n", fd);
                } else if (!strncmp(buf, "PUB\n", 4)){
                    roles[i]=PUBLISHER;
                    printf("[BROKER] fd=%d registrado como PUB\n", fd);
                } else {
                    printf("[BROKER] Header inválido en fd=%d\n", fd);
                    close(fd); clients[i]=-1; roles[i]=UNKNOWN; continue;
                }

                // ¿Quedó "resto" después del header? (ej: "PUB\nGol...\n")
                if ((int)header_len < n && roles[i]==PUBLISHER){
                    char *rest = buf + header_len;
                    // difundir resto completo
                    printf("[BROKER] (resto tras header) difundir: %s", rest);
                    for (int j=0;j<MAX_CLIENTS;j++){
                        if (clients[j]!=-1 && roles[j]==SUBSCRIBER){
                            send(clients[j], rest, strlen(rest), 0);
                        }
                    }
                    const char *ack="Broker: publicado.\n";
                    send(fd, ack, strlen(ack), 0);
                }
                continue;
            }

            // Mensajes de publishers: difundir
            if (roles[i]==PUBLISHER){
                printf("[BROKER] PUB fd=%d -> difundir: %s", fd, buf);
                for (int j=0;j<MAX_CLIENTS;j++){
                    if (clients[j]!=-1 && roles[j]==SUBSCRIBER){
                        send(clients[j], buf, strlen(buf), 0);
                    }
                }
                const char *ack="Broker: publicado.\n";
                send(fd, ack, strlen(ack), 0);
            } else {
                // SUB: ignora cualquier cosa que envíe
            }
        }
    }
    close(listen_fd);
    return 0;
}

