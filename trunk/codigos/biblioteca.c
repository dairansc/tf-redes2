#define BUFFERMAX 1500
#define PORTAPADRAO 10000
#define NOMEARQUIVOMAX BUFFERMAX-sizeof(long int)-sizeof(int)

struct fileHeader
{
    long int tamanho;
    int tamanhoNome;
    char nome[NOMEARQUIVOMAX];
};

char *nomeBaseArquivo(char *caminho)
{
    char *base = strrchr(caminho, '/');
    return base ? base+1 : caminho;
}
