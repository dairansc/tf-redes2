/* Código que será traduzido para gerar os stubs e headers necessários */
/* use: rpcgen shareit.x; */

/* Não é possível comentar o código com "//" */
/* No lugar de #define, utilizar const */


/* Constantes da aplicação Share Center */
/* Tamanho em bytes do repositório */
const TAM_MAX_REPOSITORIO = 1024;
/* Tamanho máximo do nome do usuário */
const TAM_MAX_NOME_USER = 20;
/* Tamanho máximo do nome de arquivos */
const TAM_MAX_NOME_ARQ = 50;
/* Quantidade de arquivos que cada usuário pode compartilhar */
const QNT_ARQS_USER = 50;
/* Número máximo de conexões com o servidor */
const NUM_MAX_CON = 4;
/* Nome do repositório do servidor */
const DIR_REP = "RepServ";

/* IDENTIFICAÇÕES DE ERROS
/* indice da lista de arquivos vazio*/
const LISTA_ARQ_VAZIA = -2;
/* erro ao abrir arquivo */
const ERRO_ABRIR_ARQ =  -1;
/* conexão com servidor ok */
const CON_OK =           0;
/* número máximo de conexões atingidas no servidor */
const ERRO_MAX_CON =     1;
/* usuário não conectado */
const ERRO_NAO_CON =     2;
/* usuário já conectado ao servidor */
const ERRO_JA_CON =      3;
/* sem espaço no repositório */
const ERRO_SEM_ESPACO =  4;
/* arquivo já existente */
const ERRO_ARQ_JA_EXISTE=5;
/* arquivo aguarda em lista para fazer upload */
const AGUARDANDO_ENVIO = 6;
/* arquivo enviado da lista para servidor */
const ENVIO_SUCESSO = 	 7;
/* Erro desconhecido do servidor */
const ERRO_SERVIDOR = 	 8;


/* estrutura que encapsulará o transporte de um arquivo entre servidor e cliente */
struct transporteArquivo{  
    char usuario[TAM_MAX_NOME_USER];
    char nome[TAM_MAX_NOME_ARQ];
    char conteudo[TAM_MAX_REPOSITORIO];
    int tamanho;
};

program SHARECENTERPROG {                /* valor para resgitrar o programa */
    version SHAREVERSION {            /* versão precisa ter uma identificação */
        int CONECTUSER_ARGS(string) = 1;  /* conecta o usuário com o servidor */
        int DESCONECTUSER_ARGS(string) = 2;  /* desconecta usuário com o servidor */
        int SHARE_ARGS(transporteArquivo) = 3;
        transporteArquivo DOWNLOAD_ARGS(transporteArquivo) = 4;
        string LIST() = 5;
    } =1;                          /* valor da versão */
} = 0x20000F05;                    /* valor do programa */ 
