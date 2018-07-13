#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <ctype.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
using namespace std;

#define TAM 4096
#define N 100

/*  
    Nombre del archivo: servidor.cpp
    Trabajo Práctico 3 - Ejercicio 5
    Grupo: 12
    Gómez Markowicz, Federico - 38858109
    Kuczerawy, Damián - 37807869
    Mediotte, Facundo - 39436162
    Siculin, Luciano - 39213320
    Tamashiro, Santiago - 39749147
*/

char* encriptarFrase(char*);
char* desencriptarFrase(char*);
char* memoriaLeer();
void memoriaEscribir(char*);

int largo;
char* stringATratar;
char opcion;
mutex mtx;
//char frase[N];

void escuchar (int connfd) {
  int vivo = 1;
  while(vivo) {
    read(connfd, &opcion, sizeof(opcion));
    //mtx.lock(); // el mutex se bloquea hasta que el cliente reciba el mensaje
    if(opcion != 's') {
      stringATratar=memoriaLeer();
      largo = sizeof(stringATratar);
      if(opcion == 'e') {
        stringATratar=encriptarFrase(stringATratar);
      } else {
          stringATratar=desencriptarFrase(stringATratar);
      }
      memoriaEscribir(stringATratar);
      opcion='z';
      write(connfd,&opcion,sizeof(opcion));
      //mtx.unlock();
    } else vivo = 0;
  }
  cout << "Un cliente se desconectó." << endl;
  close(connfd);
}

int main(int argc, char *argv[])
{
    if (argc == 2) {
      string param(argv[1]);
      if (param == "-help" || param == "-h" || param == "-?") {
        cout << "./servidor" << endl;
        cout << "Este programa no lleva parámetros." << endl;
        cout << "El servidor permite encriptar/desencriptar cadenas enviadas por los clientes." << endl;
        return 0;
      } else {
        cerr << "Error en los parámetros enviados, utilice -help." << endl;
        return -1;
      }
    } else if (argc != 1) {
      cerr << "Error en los parámetros enviados, utilice -help." << endl;
      return -1;
    }

    //printf("Ingrese el puerto a utilizar: ");
    //scanf("%i",&puerto);
    struct sockaddr_in serv_addr;
    char sendBuff[1025];

    memset(sendBuff, '0', sizeof(sendBuff));
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);
    char mensaje[250];

    printf("Server iniciado.\n");
    while (1)
    {
        int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        puts("Cliente conectado.");
        thread hilo = thread(&escuchar, connfd);
        hilo.detach();
    }
}


void memoriaEscribir(char *mensaje){
    int memoriaID;
    char *punteroAMemoriaCompartida = NULL;
    if((memoriaID = shmget(1315511,TAM,0660|IPC_CREAT))==-1) { //El primer valor es un identificador unico, puede dar problemas
        //fprintf(stderr,"Error al reservar la memoria");
        perror("Error al reservar la memoria");
    } //Creo la memoria compartida
    punteroAMemoriaCompartida = (char*)shmat(memoriaID,(void *)0,0); //Asociacion
    strcpy(punteroAMemoriaCompartida,mensaje);
}

char* memoriaLeer(){
    int memoriaID;
    char *punteroAMemoriaCompartida = NULL;
    memoriaID = shmget(1315511,TAM,0660|IPC_CREAT);
    punteroAMemoriaCompartida = (char*)shmat(memoriaID,NULL,0); //Asociacion
    //shmdt(&punteroAMemoriaCompartida); //Desasociacion
    if(shmctl(memoriaID,IPC_RMID,NULL)==-1){
        fprintf(stderr,"Error al liberar la memoria");
    }
    return punteroAMemoriaCompartida;

}

char* encriptarFrase (char *frase){
    int i;
    for(i=0 ; i<largo ; ++i)
    {
        //convierte las minuscalas a mayuscula.
        frase[i] = toupper(frase[i]);


        //descarta los digitos
        if(isalpha(frase[i]))
        {
            if(frase[i] > 87)
            {
                frase[i]= frase[i]-23;
            }
            else
            {
                frase[i]= frase[i]+3;
            }
        }
    }
    return frase;
}

char* desencriptarFrase (char *frase)
{

    int i;

    for(i=0 ; i<largo ; ++i)
    {
        frase[i] = toupper(frase[i]);

        if(isalpha(frase[i]))
        {

            if(frase[i] < 68)
            {
                frase[i]= frase[i] + 23;
            }
            else
            {
                frase[i]= frase[i] - 3;
            }

        }
    }
    return frase;
}
