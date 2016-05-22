#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MAXCLIENTES 20
#define TAMFILA 5
#define MAXNOMEHOST 30

void flushBuff(char* buffer, int size) {
    int i=0;
    for(; i<size; ++i) {
        buffer[i] = '\0';
    }
}

struct cliente {
    long ip;
    int total_msg, msg_rec, msg_perd, msg_err;
};

typedef struct cliente cliente;

/* Sobre essa estrutura:
O ideal eh implementar uma arvore (pode ser binaria, que eh easy) pra fazer insercao. Acho que nao precisamos nos preocupar
muito com remocao, pq nao devem ficar aparecendo muitos clientes aleatorios, entao se soh fizer insercao jah dah boa.
Mesmo assim, se quiser implementar remocao, ficaria ainda melhor. A vantagem de arvore eh que dai a gente faz a busca binaria
pelo IP, e como eh um long, eh tranquilo fazer a funcao de comparacao.
Se realmente implementar isso, tem que mudar a funcao ja_comuniquei
*/

void cria_cliente(cliente c[], int index, long ip) {
    c[index].ip = ip; // isa.sin_addr.s_addr;
    c[index].total_msg = 0;
    c[index].msg_rec = 0;
    c[index].msg_perd = 0;
    c[index].msg_err = 0; // Ordem errada
}

int ja_comunicou(cliente *c, int num_clientes, long ip) {
    // Retorna 1 se o cliente já comunicou, 0 caso contrário.
    int i;
    for(i=0; i<num_clientes; ++i) {
        if(ip == c[i].ip) {
            return ip;
        }
    }
    return -1;
}
/*
if(jah_comuniquei(c, num_clientes, isa.sin_addr.s_addr) == 1) {
    // Comuniquei
} else {
    // Nao comuniquei, cria um cliente e faz num_clientes++.
    num_clientes++;
}
*/
int main(int argc, char*argv[]) {
    int sock_escuta, sock_atende;
    int numMensagens, esperado;
    //int recebidas = 0, ordemErrada = 0;
    int num_clientes = 0;
    unsigned int i;
    char buffer[BUFSIZ+1];
    struct sockaddr_in enderecLocal, enderecCliente;
    struct hostent *registroDNS;
    char nomeHost[MAXNOMEHOST];
    cliente c[MAXCLIENTES];

    signal(); // Evitar que o processo ocupe a porta como zumbi.
    if(argc != 3) {
        puts("Uso correto: servidor <porta> <num_mensagens_por_cliente>");
        exit(1);
    }

    const int MaxMsg = atoi(argv[2]);

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
    if((setsockopt(sock_atende, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval))) == 0) {
        exit(0);
    }
    //read(sock_atende, buffer, BUFSIZ);
    recvfrom(sock_atende, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, &i);

    numMensagens = atoi(buffer);
    //printf("Server: Esperando %d mensagens.\n", MaxMsg);
    //write(sock_atende, buffer, BUFSIZ); // Responde qualquer coisa.
    sendto(sock_atende, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, i);
    flushBuff(buffer, BUFSIZ);

    int msgAtual = 0, index;

    while(1) {
        recvfrom(sock_atende, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, &i);
        //read(sock_atende, buffer, BUFSIZ);
        //if(enderecCliente.sin_addr.s_addr != 0) { // Se for == 0, ach oque nao recebi nenhuma mensagem
            printf("Sou o servidor, recebi %s do ip %ld\n", buffer, enderecCliente.sin_addr.s_addr);
        //}

        if((index = ja_comunicou(c, num_clientes, enderecCliente.sin_addr.s_addr)) == -1) {
            cria_cliente(c, num_clientes, enderecCliente.sin_addr.s_addr);
            index = num_clientes;
            num_clientes++;
        }

        //if(msgAtual != atoi(buffer)) {
        if(c[index].total_msg != atoi(buffer)) {
            //ordemErrada++;
            c[index].msg_err++;
        }
        c[index].msg_rec++;
        c[index].total_msg++;
        //recebidas++;

        if(strcmp(buffer,"End") == 0) {
            break;
        }

        flushBuff(buffer, BUFSIZ);
    }

    for(i=0; i<num_clientes; ++i) {
        c[i].msg_perd = c[i].total_msg - c[i].msg_rec;
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
