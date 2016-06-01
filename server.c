#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MAXCLIENTES 20
#define MAXNOMEHOST 30
#define CONCATENAR 1
#define NAO_CONCATENAR 0

struct cliente {
    long ip;
    int total_msg, msg_rec, msg_per, msg_err, msg_atual;
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
    c[index].msg_rec = 0;
    c[index].msg_per = 0;
    c[index].msg_err = 0; // Ordem errada
    c[index].total_msg = MaxMsg;
}

void imprime_resultados(cliente c[], int num_clientes) {
    int i;
    for(i=0; i<num_clientes; i++) {
        printf("Cliente %d: %ld, recebi %d e perdi %d  mensagens e recebi %d na ordem errada.\n", i, c[i].ip, c[i].msg_rec, c[i].total_msg - c[i].msg_rec, c[i].msg_err);
    }
    return ;
}

void log_simplificado(cliente c[], int num_clientes, int concatenar, const char* nome_arquivo) {
    int i;
    FILE *fp;
    char modo[2];
    int total_recebidas = 0, total_perdidas = 0, total_err = 0, total_msg = 0;

    modo[0] = concatenar ? 'a' : 'w';
    modo[1] = '\0';

    if(!(fp = fopen(nome_arquivo, modo))) {
        puts("Nao foi possivel abrir o arquivo de log.");
        exit(1);
    }

    if(num_clientes <= 0) {
        fprintf(fp, "Nao houve nenhum cliente.\n");
        fclose(fp);
        return ;
    }

    for(i=0; i<num_clientes; i++) {
        total_msg += c[i].total_msg;
        total_recebidas += c[i].msg_rec;
        total_perdidas += c[i].msg_per;
        total_err += c[i].msg_err;
    }

    fprintf(fp, "Total: De %d mensagens, %d (%f%%) foram recebidas, %d (%f%%) foram perdidas, %d (%f%%) estavam fora de sequencia.\n",
            total_msg, total_recebidas, (double) total_recebidas * 100 / total_msg,
            total_perdidas, (double) total_perdidas * 100 / total_msg,
            total_err, (double) total_err * 100 / total_msg);

    for(i=0; i<num_clientes; i++) {
        fprintf(fp, "Cliente %d: De %d mensagens, %d (%f%%) foram recebidas, %d (%f%%) foram perdidas, %d (%f%%) estavam fora de sequencia.\n", i+1,
                c[i].total_msg, c[i].msg_rec, (double) c[i].msg_rec * 100 / c[i].total_msg,
                c[i].msg_per, (double) c[i].msg_per * 100 / c[i].total_msg,
                c[i].msg_err, (double) c[i].msg_err * 100 / c[i].total_msg);
    }

    fclose(fp);
}

void log_detalhado(cliente c[], int num_clientes, int concatenar, const char* nome_arquivo) {
    int i;
    FILE *fp;
    char modo[2];
    int total_recebidas = 0, total_perdidas = 0, total_err = 0;

    modo[0] = concatenar ? 'a' : 'w';
    modo[1] = '\0';

    if(!(fp = fopen(nome_arquivo, modo))) {
        puts("Nao foi possivel abrir o arquivo de log.");
        exit(1);
    }
/*
    fprintf(fp, "===============================================================\n");
    fprintf(fp, "Inicio da execucao: programa que implementa uma artilharia UDP.\n");
    fprintf(fp, "Prof. Elias P. Duarte Jr. - Disciplina Redes de Computadores II\n");
    fprintf(fp, "===============================================================\n\n");
*/
    if(num_clientes <= 0) {
        fprintf(fp, "Nao houve nenhum cliente.\n");
        fclose(fp);
        return ;
    }

    fprintf(fp, "O servidor recebeu um total de %d mensagens de %d clientes.\n\n", c[0].total_msg, num_clientes);
    for(i=0; i<num_clientes; i++) {
        total_recebidas += c[i].msg_rec;
        total_perdidas += c[i].msg_per;
        total_err += c[i].msg_err;
    }

    fprintf(fp, "Eram esperadas %d mensagens, sobre as quais:\n",c[0].total_msg * num_clientes);
    fprintf(fp, " - %d foram recebidas (%f%%);\n",total_recebidas, (double) total_recebidas * 100 / c[0].total_msg * num_clientes);
    fprintf(fp, " - %d foram perdidas (%f%%);\n",total_perdidas, (double) total_perdidas * 100 / c[0].total_msg * num_clientes);
    fprintf(fp, " - %d estavam fora de sequencia (%f%%);\n\n",total_err, (double) total_err * 100 / c[0].total_msg * num_clientes);

    for(i=0; i<num_clientes; i++) {
        fprintf(fp, "O cliente %d, que enviou %d mensagens, teve como resultados:\n", i+1, c[i].total_msg);
        fprintf(fp, " - %d foram recebidas (%f%%).\n", c[i].msg_rec, (double) c[i].msg_rec * 100 / c[i].total_msg);
        fprintf(fp, " - %d foram perdidas (%f%%).\n", c[i].msg_per, (double) c[i].msg_per * 100 / c[i].total_msg);
        fprintf(fp, " - %d estavam fora de sequencia (%f%%).\n\n", c[i].msg_err, (double) c[i].msg_err * 100 / c[i].total_msg);
    }

    fprintf(fp, "Fim do log.\n\n");

    fclose(fp);
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

int main(int argc, char*argv[]) {
    int sock;
    int msgAtual = 0, num_clientes = 0, index;
    unsigned int i;
    char buffer[BUFSIZ+1];
    struct sockaddr_in enderecLocal, enderecCliente;
    struct hostent *registroDNS;
    char nomeHost[MAXNOMEHOST];
    FILE *fp;
    cliente c[MAXCLIENTES];

    signal(); // Evitar que o processo ocupe a porta como zumbi.
    if(argc != 4) {
        puts("Uso correto: servidor <porta> <num_mensagens_por_cliente> <log_detalhado>");
        exit(1);
    }

    const int MaxMsg = atoi(argv[2]);
    const int logDetalhado = atoi(argv[3]);

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

    if(!(fp = fopen("log_detalhado.txt","w"))) {
        puts("Nao foi possivel abrir o arquivo log_detalhado.txt");
        exit(1);
    }

    if(logDetalhado) {
        fprintf(fp, "Servidor iniciado, esperando %d mensagens de cada cliente.\n", MaxMsg);
    }

    while(1) {
        recvfrom(sock, buffer, BUFSIZ, 0, (struct sockaddr *) &enderecCliente, &i);

        if((index = ja_comunicou(c, num_clientes, enderecCliente.sin_addr.s_addr)) == -1) {
            cria_cliente(c, num_clientes, enderecCliente.sin_addr.s_addr, MaxMsg);
            index = num_clientes;
            num_clientes++;
        }

        if(logDetalhado) {
            fprintf(fp, "Recebi a mensagem '%s' do cliente numero %d.\n", buffer, index+1);
        }

        if(strcmp(buffer, "End") == 0)
            break;

        if(strcmp(buffer,"Resultado") == 0) {
            imprime_resultados(c, num_clientes);
               // Msg recebida - ultima msg recebida
        } else if(atoi(buffer) < c[index].msg_atual) { // Ve se a mensagem ta na ordem errada pra esse cliente
            c[index].msg_err++;
            //printf("Ordem errada: %s\n", buffer);
            if(logDetalhado) {
                fprintf(fp, "Mensagem recebida, mas em ordem errada.\n");
            }
        } else { // Tava na ordem certa, atualiza msgAtual
            c[index].msg_atual = atoi(buffer);
        }

        //puts(buffer);

        //msgAtual = atoi(buffer);
        c[index].msg_rec++;

        flushBuff(buffer, BUFSIZ);
    }

    for(i=0; i<num_clientes; ++i) {
        c[i].msg_per = c[i].total_msg - c[i].msg_rec;
    }

    if(logDetalhado)
        log_detalhado(c, num_clientes, CONCATENAR, "log_detalhado.txt");
    //imprime_log(c, num_clientes, CONCATENAR, "log.txt");

    log_simplificado(c, num_clientes, CONCATENAR, "log.txt");

    //pega_dados("log.txt")

    /*
    Calcula media, desvio padrao.
    */
}
