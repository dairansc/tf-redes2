// Arquivo com biblioteca de funções utilizadas na aplicação Share Center


#include <iostream>
#include <stdlib.h>
#include <sstream> // necessária para conversão em string
#include <fstream> // manipulação de arquivos
#include <sys/stat.h> // Manipulação de diretórios (mkdir)
#include <dirent.h> // Manipulação de diretórios (DIR)
#include <cerrno> // Identificação de erros de execução strerror(errno) (mkdir)
#include <cstdio> // stdio.h do C
#include <rpc/rpc.h>
#include "shareit.h"

using namespace std;

string convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

string preencheComEspaco(string texto){
   int i, tam = texto.length();
   stringstream ss;

   ss << texto;
   
   for (i=tam;i<=30;i++) {
      ss << " ";
   }
   return ss.str();

}

// Os stubs utilizam string nas declarações, que na verdade são char** no C, está função converte string do C++ para char** utilizado nos stubs
char **toStringRPC(string texto){
    static char* r = new char(texto.length()+1);
     
    strcpy(r,texto.c_str());

    return ((char **) &r);
}

// Retorna o tamanho do arquivo, também utilizada para verificar se o arquivo existe
int tamanhoArquivo(string nome){
    ifstream file(nome.c_str());
    int tamanho;
    
    if(file.is_open() || !file.good()){ // verifica se o arquivo está aberto e pronto para utilização
        file.seekg(0, ios::end);
        tamanho = file.tellg();
        file.close();
        return tamanho;
    }
    return -1;
}

// Retorna o tamanho de um diretório
int tamanhoDiretorio(string nome){
    DIR *dirstream;
    struct dirent *direntry;
    int tamanho = 0;

    dirstream = opendir(nome.c_str());
    if ( ! dirstream ){
        return -1;
    }
    while (direntry = readdir (dirstream)){
        //cout << direntry->d_name << endl;
        if(direntry->d_name[0]!='.'){ // dentro de cada diretório existe '.' e '..', não incluir eles na contagem
            tamanho += tamanhoArquivo(nome + "/" + direntry->d_name);
        }
    }
    (void) closedir (dirstream);
    return tamanho;
}

// Exclui todos arquivos dentro de um diretório
int limpaDiretorio(string nome){
    DIR *dirstream;
    struct dirent *direntry;

    dirstream = opendir(nome.c_str());
    if ( ! dirstream ){
        return -1;
    }
    while (direntry = readdir (dirstream)){
        //cout << direntry->d_name << endl;
        if(direntry->d_name[0]!='.'){ // dentro de cada diretório existe '.' e '..', não incluir eles na contagem
            if( remove( (nome + "/" + direntry->d_name).c_str() ) < 0 ){ // se não foi possível excluir algum arquivo
                (void) closedir (dirstream);
                return -1;
            }
        }
    }
    (void) closedir (dirstream);
    return 1;
}
