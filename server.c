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

void flushBuff(char* buffer, int size) {
    int i=0;
    for(; i<size; ++i) {
        buffer[i] = '\0';
    }
}

void cria_cliente(cliente c[], int index, long ip, const int MaxMsg) {
    c[index].ip = ip; // isa.sin_addr.s_addr;
    c[index].total_msg = MaxMsg;
    c[index].msg_rec = 0;
    c[index].msg_perd = 0;
    c[index].msg_err = 0; // Ordem errada
}

void imprime_resultados(cliente c[], int num_clientes) {
    int i;
    for(i=0; i<num_clientes; i++) {
        printf("Cliente %d: %ld, recebi %d e perdi %d  mensagens e recebi %d na ordem errada.\n", i, c[i].ip, c[i].msg_rec, c[i].total_msg - c[i].msg_rec, c[i].msg_err);
    }
    return ;
}

int ja_comunicou(cliente *c, int num_clientes, long ip) {
    // Retorna o indice i de c[i] se o cliente já comunicou com o cliente i, -1 caso contrário.
    int i;
    for(i=0; i<num_clientes; ++i) {
        if(ip == c[i].ip) {
            return i;
        }
    }
    return -1;
}
/* Fiz soh de memoria, n sei se isso funciona, eh soh uma ideia
cliente media(cliente c[], int tam) {
    int i;
    cliente total;
    for(i=0; i<tam; i++) {
        total.total_msg += c[i].total_msg;
        total.msg_rec += c[i].msg_rec;
        total.msg_per += c[i].msg_per;
        total.msg_err += c[i].msg_err;
    }

    total.total_msg = total.total_msg / tam;
    total.msg_rec = total.msg_rec / tam;
    total.msg_per = total.msg_per / tam;
    total.msg_err = total.msg_err / tam;

    return total;
}

cliente desvio_padrao(cliente media, cliente c[], int tam) {
    int i;
    cliente desvio;
    for(i=0; i<tam; i++) {
        desvio.total_msg += ((c[i].total_msg - media.total_msg) * (c[i].total_msg - media.total_msg));
        desvio.msg_rec += ((c[i].msg_rec - media.msg_rec) * (c[i].msg_rec - media.msg_rec));
        desvio.msg_per += ((c[i].msg_per - media.msg_per) * (c[i].msg_per - media.msg_per));
        desvio.msg_err += ((c[i].msg_err - media.msg_err) * (c[i].msg_err - media.msg_err));
    }

    desvio.total_msg = sqrt(desvio.total_msg);
    desvio.msg_rec = sqrt(desvio.msg_rec);
    desvio.msg_per = sqrt(desvio.msg_per);
    desvio.msg_err = sqrt(desvio.msg_err);

    return desvio;
}
*/
int main(int argc, char*argv[]) {
    int sock;
    int numMensagens, esperado;
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

    enderecLocal.sin_port = htons(atoi(argv[1]));

    bcopy((char*) registroDNS->h_addr, (char*)&enderecLocal.sin_addr, registroDNS->h_length);

    enderecLocal.sin_family = registroDNS->h_addrtype;

    if((sock = socket(registroDNS->h_addrtype, SOCK_DGRAM, 0)) < 0) {
        puts("Nao consegui abrir o socket.");
        exit(1);
    }

    if(bind(sock, (struct sockaddr *) &enderecLocal, sizeof(enderecLocal)) < 0) {
        puts("Nao consegui fazer o bind");
        exit(1);
    }

    // Escuta a primeira mensagem, que contem o numero de mensagens que serao enviadas por um cliente.
    i = sizeof(enderecCliente);

    int msgAtual = 0, index;

    while(1) {
        recvfrom(sock, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, &i);
        //if(enderecCliente.sin_addr.s_addr != 0) { // Se for == 0, ach oque nao recebi nenhuma mensagem
            //printf("Sou o servidor, recebi %s do ip %ld\n", buffer, enderecCliente.sin_addr.s_addr);
        //}

        if((index = ja_comunicou(c, num_clientes, enderecCliente.sin_addr.s_addr)) == -1) {
            cria_cliente(c, num_clientes, enderecCliente.sin_addr.s_addr, MaxMsg);
            index = num_clientes;
            num_clientes++;
        }

        if(c[index].msg_rec != atoi(buffer)) {
            c[index].msg_err++;
        }
        c[index].msg_rec++;
        //c[index].total_msg++;

        if(strcmp(buffer,"Resultado") == 0) {
            imprime_resultados(c, num_clientes);
        }

        flushBuff(buffer, BUFSIZ);
    }

    for(i=0; i<num_clientes; ++i) {
        c[i].msg_perd = c[i].total_msg - c[i].msg_rec;
    }

    /*
    Calcula media, desvio padrao.
    */

}
