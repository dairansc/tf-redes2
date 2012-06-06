#define BUFFERMAX 1500
#define PORTAPADRAO 10000


char *nomeBaseArquivo(char *caminho)
{
    char *base = strrchr(caminho, '/');
    return base ? base+1 : caminho;
}
