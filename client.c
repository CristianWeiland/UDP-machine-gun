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
    int ordemErrada = 0, recebidas = 0;
    int logDetalhado;
    struct sockaddr_in enderecRemoto;
    struct hostent *registroDNS;
    char buffer[BUFSIZ+1];
    char *nomeHost;
    char *dados;

    if(argc != 5) {
        puts("Uso correto: client <nome-servidor> <porta> <numero-mensagens> <log-detalhado (1 = sim, 0 = nao)>");
        exit(1);
    }

    logDetalhado = atoi(argv[4]);
    numMensagens = atoi(argv[3]);
    if(numMensagens <= 0 && strcmp("result",argv[3]) != 0 && strcmp("end",argv[3]) != 0) {
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
    enderecRemoto.sin_family = registroDNS->h_addrtype;

    enderecRemoto.sin_port = htons(atoi(argv[2]));
    if((sock_descr = socket(registroDNS->h_addrtype, SOCK_DGRAM, 0)) < 0) {
        puts("Nao consegui abrir o socket.");
        exit(1);
    }

    if(strcmp(argv[3], "result") == 0) {
        puts("Enviando mensagem fim.");
        strcpy(dados, "Resultado");
        if(sendto(sock_descr, dados, strlen(dados)+1, 0, (struct sockaddr *) &enderecRemoto, sizeof(enderecRemoto)) == 0) {
            puts("Nao consegui transmitir a mensagem.");
            exit(1);
        } else {
            puts("Acho que enviei.");
        }
        close(sock_descr);
        return 0;
    }
    if(strcmp(argv[3], "end") == 0) {
        puts("Enviando mensagem fim.");
        strcpy(dados, "End");
        if(sendto(sock_descr, dados, strlen(dados)+1, 0, (struct sockaddr *) &enderecRemoto, sizeof(enderecRemoto)) == 0) {
            puts("Nao consegui transmitir a mensagem.");
            exit(1);
        } else {
            puts("Acho que enviei.");
        }
        close(sock_descr);
        return 0;
    }

    flushBuf(dados, DATA_SIZE);

    while(msgAtual < numMensagens) {
        cria_msg(dados, msgAtual);
        if(sendto(sock_descr, dados, strlen(dados)+1, 0, (struct sockaddr *) &enderecRemoto, sizeof(enderecRemoto)) == 0) {
            puts("Nao consegui transmitir a mensagem.");
            exit(1);
        } else if(logDetalhado) {
            printf("Enviada a mensagem número %d\n",msgAtual);
        }
        msgAtual++;
    }

    close(sock_descr);
    return 0;
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
