#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int sock_descr;
    int numBytesRecebidos;
    struct sockaddr_in enderecRemoto;
    struct hostent *registroDNS;
    char buffer[BUFSIZ+1];
    char *nomeHost;
    char *dados;

    if(argc != 4) {
        puts("Uso correto: client <nome-servidor> <porta> <dados>");
        exit(1);
    }

    nomeHost = argv[1];
    dados = argv[3];
    if((registroDNS = gethostbyname(nomeHost)) == NULL) {
        puts("Não consegui obter o endereço IP do servidor.");
        exit(1);
    }

    bcopy((char*)registroDNS->h_addr, (char*)&enderecRemoto.sin_addr, registroDNS->h_length);

    enderecRemoto.sin_family = AF_INET;
    enderecRemoto.sin_port = htons(atoi(argv[2]));
    sock_descr = socket(AF_INET, SOCK_STREAM, 0);
   
    if(connect(sock_descr, (struct sockaddr*)&enderecRemoto, sizeof(enderecRemoto)) < 0) {
        puts("Não consegui conectar com o servidor.");
        exit(1);
    }

    if(write(sock_descr, dados, strlen(dados)) != strlen(dados)) {
        puts("Não consegui transmitir os dados.");
        exit(1);
    }

    read(sock_descr, buffer, BUFSIZ);
    printf("Sou o cliente, recebi %s\n", buffer);
    close(sock_descr);
    exit(0);
}
