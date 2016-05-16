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

    enderecRemoto.sin_family = AF_INET;
    enderecRemoto.sin_port = htons(atoi(argv[2]));
    sock_descr = socket(AF_INET, SOCK_STREAM, 0);
   
    if(connect(sock_descr, (struct sockaddr*)&enderecRemoto, sizeof(enderecRemoto)) < 0) {
        puts("Não consegui conectar com o servidor.");
        exit(1);
    }

    while(msgAtual < numMensagens) {
        cria_msg(dados, msgAtual);
        tamanhoDados = strlen(dados);
        if(write(sock_descr, dados, tamanhoDados) != tamanhoDados) {
            puts("Não consegui transmitir os dados.");
            exit(1);
        }

        read(sock_descr, buffer, BUFSIZ);
        if(atoi(dados) == msgAtual) {
            printf("Sou o cliente, recebi corretamente %s\n", buffer);
            recebidas++;
        } else {
            printf("Sou o cliente, esperava mensagem %d mas recebi %s\n", msgAtual, buffer);
            ordemErrada++;
            recebidas++;
        }
    }

    /* Contabilização do resultado:
     * Manda mais uma mensagem dizendo quantas falhas? Se o servidor for contar sozinho, pode dar treta.
     * Tipo, se o servidor receber a ultima mensagem com numero 500 e supor que foram 500 mensagens, mas foram 510,
     * eu vou ignorar parte do resultado, o que é realmente bad. Então a gente faz o cliente no final mandar uma
     * mensagem que indique quantas mensagens foram enviadas / quantas mensagens falharam / quantas chegaram fora de ordem.
     * Ou uma mensagem só que tenha esses 3 valores, pode ser válido.
     */

    /* Fim da transmissão:
     * Vejo 3 formas:
     * 1- A primeira mensagem é uma mensagem que diz quantas mensagens vou mandar (daí tem que definir se inclui a msg de contabilização ou n)
     * 2- A última mensagem tem dados == 0, e depois dela vem uma última que é a de contabilização.
     * 3- Vai recebendo mensagens até achar um formato diferente - que representa a última mensagem, de contabilização.
          Possivelmente fazer tipo, a mensagem de contabilização assim: "Resultado: %d - %d - %d" e no servidor while(dados[0] != 'R') {}
     */

/*
    if(write(sock_descr, dados, strlen(dados)) != strlen(dados)) {
        puts("Não consegui transmitir os dados.");
        exit(1);
    }

    read(sock_descr, buffer, BUFSIZ);
    printf("Sou o cliente, recebi %s\n", buffer);
    close(sock_descr);
    exit(0);
*/
}

char* itobase10(char *buf, size_t sz, int value) {
    snprintf(buf, sz, "%d", value);
    return buf;
}

void cria_msg(char *dados, int msgAtual) {
    flushBuf(dados, DATA_SIZE);
    itobase10(dados, DATA_SIZE, msgAtual); // Usa 10 porque é base decimal.
}

void flushBuf(char *buf, int size) {
    int i=0;
    for(; i<size; ++i) {
        buf[i] = '\0';
    }
}


