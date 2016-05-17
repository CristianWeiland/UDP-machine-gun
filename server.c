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
    int numMensagens, esperado;
    int recebidas = 0, ordemErrada = 0;
    unsigned int i;
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

    // Escuta a primeira mensagem, que contem o numero de mensagens que serao enviadas por um cliente.
    i = sizeof(enderecCliente);

    struct timeval timeout={2,0};
    if((setsockopt(sock_atende, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval)) == 0) {
        exit(0);
    }
    //read(sock_atende, buffer, BUFSIZ);
    recvfrom(sock_atende, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, &i);

    numMensagens = atoi(buffer);
    printf("Server: Esperando %d mensagens.\n", numMensagens);
    //write(sock_atende, buffer, BUFSIZ); // Responde qualquer coisa.
    sendto(sock_atende, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, i);
    flushBuff(buffer, BUFSIZ);

    int msgAtual = 0;

    while(msgAtual < numMensagens) {
        recvfrom(sock_atende, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, &i);
        //read(sock_atende, buffer, BUFSIZ);
        printf("Sou o servidor, recebi %s\n", buffer);
        if(msgAtual != atoi(buffer)) {
            ordemErrada++;
        }
        recebidas++;

        flushBuff(buffer, BUFSIZ);
    }

    /*
    Calcula media, desvio padrao.
    */

/*
    while(1) {
        i = sizeof(enderecLocal);
        if((sock_atende = accept(sock_escuta, (struct sockaddr*)&enderecCliente, &i)) < 0) {
            puts("Não consegui completar a conexão.");
            exit(1);
        }

        read(sock_atende, buffer, BUFSIZ);
        printf("Sou o servidor, recebi %s\n", buffer);
        write(sock_atende, buffer, BUFSIZ);
        close(sock_atende);
        flushBuff(buffer, BUFSIZ);
    }
*/
}
