#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#define TAMFILA 5
#define MAXNOMEHOST 30

void flushBuff(char* buffer, int size) {
    int i=0;
    for(; i<size; ++i) {
        buffer[i] = '\0';
    }
}

int main(int argc, char*argv[]) {
    int sock_escuta, sock_atende;
    unsigned int aux;
    char buffer[BUFSIZ+1];
    struct sockaddr_in enderecLocal, enderecCliente;
    struct hostent *registroDNS;
    char nomeHost[MAXNOMEHOST];

    signal(); // Evitar que o processo ocupe a porta como zumbi.
    if(argc != 2) {
        puts("Uso correto: servidor <porta>");
        exit(1);
    }

    gethostname(nomeHost, MAXNOMEHOST); // Syscall para descobrir o nome do host (local).

    if((registroDNS = gethostbyname(nomeHost)) == NULL) {
        puts("Não consegui meu próprio IP.");
        exit(1);
    }

    enderecLocal.sin_family = AF_INET;
    enderecLocal.sin_port = htons(atoi(argv[1]));

    bcopy((char*) registroDNS->h_addr, (char*)&enderecLocal.sin_addr, registroDNS->h_length);

    if((sock_escuta = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("Não consegui abrir o socket.");
        exit(1);
    }

    if(bind(sock_escuta, (struct sockaddr*)&enderecLocal, sizeof(enderecLocal)) < 0) {
        puts("Não consegui fazer o bind."); // Troque de porta!
        exit(1);
    }

    listen(sock_escuta, TAMFILA);

    while(1) {
        aux = sizeof(enderecLocal);
        if((sock_atende = accept(sock_escuta, (struct sockaddr*)&enderecCliente, &aux)) < 0) {
            puts("Não consegui completar a conexão.");
            exit(1);
        }

        read(sock_atende, buffer, BUFSIZ);
        printf("Sou o servidor, recebi %s\n", buffer);
        write(sock_atende, buffer, BUFSIZ);
        close(sock_atende);
        flushBuff(buffer, BUFSIZ);
    }
}
