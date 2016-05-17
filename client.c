#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 128

char* itobase10(char *buf, size_t sz, int value);
void cria_msg(char *dados, int msgAtual);
void flushBuf(char *buf, int size);

int main(int argc, char *argv[]) {
    int sock_descr;
    int numBytesRecebidos;
    int numMensagens = -1, msgAtual = 0;
    int tamanhoDados = 0;
    int ordemErrada = 0, recebidas = 0;
    struct sockaddr_in enderecRemoto;
    struct hostent *registroDNS;
    char buffer[BUFSIZ+1];
    char *nomeHost;
    char *dados;

    if(argc != 4) {
        puts("Uso correto: client <nome-servidor> <porta> <numero-mensagens>");
        exit(1);
    }

    numMensagens = atoi(argv[3]);
    if(numMensagens <= 0) {
        puts("Não consigo enviar menos de 1 mensagem.");
        exit(1);
    }

    nomeHost = argv[1];
    if((registroDNS = gethostbyname(nomeHost)) == NULL) {
        puts("Não consegui obter o endereço IP do servidor.");
        exit(1);
    }

    if((dados = malloc(sizeof(char) * DATA_SIZE)) == NULL) {
        puts("Não consegui allocar memória para os dados.");
        exit(1);
    }

    bcopy((char*)registroDNS->h_addr, (char*)&enderecRemoto.sin_addr, registroDNS->h_length);

    enderecRemoto.sin_port = htons(atoi(argv[2]));
    if((sock_descr = socket(registroDNS->h_addrtype, SOCK_DGRAM, 0)) < 0) {
        puts("Nao consegui abrir o socket.");
        exit(1);
    }

    strcpy(dados, "Tamanho:");
    tamanhoDados = strlen(dados);
    itobase10(dados, DATA_SIZE - tamanhoDados, numMensagens);

    if(sendto(sock_descr, dados, strlen(dados)+1, 0, (struct sockaddr *) &enderecRemoto, sizeof(enderecRemoto)) == 0) {
        puts("Nao consegui transmitir a mensagem inicial.");
        exit(1);
    }

    int i = sizeof(enderecRemoto);

    recvfrom(sock_descr, dados, DATA_SIZE, 0, (struct sockaddr *) &enderecRemoto, &i);

    //read(sock_descr, buffer, BUFSIZ); // Se o servidor respondeu, significa que ele sabe quantas mensagens vou mandar. Hora de enviar.
//    printf("Sou o cliente, recebi %s\n", buffer);

    flushBuf(dados, DATA_SIZE);

    while(msgAtual < numMensagens) {
        cria_msg(dados, msgAtual);
        tamanhoDados = strlen(dados);
        if(sendto(sock_descr, dados, strlen(dados)+1, 0, (struct sockaddr *) &enderecRemoto, sizeof(enderecRemoto)) == 0) {
            puts("Nao consegui transmitir a mensagem inicial.");
            exit(1);
        }
        msgAtual++;
    }
}

char* itobase10(char *buf, size_t sz, int value) {
    snprintf(buf, sz, "%d", value);
    return buf;
}

void cria_msg(char *dados, int msgAtual) {
    flushBuf(dados, DATA_SIZE);
    itobase10(dados, DATA_SIZE, msgAtual);
}

void flushBuf(char *buf, int size) {
    int i=0;
    for(; i<size; ++i) {
        buf[i] = '\0';
    }
}
