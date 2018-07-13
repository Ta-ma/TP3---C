#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <signal.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

#define TAM 4096

/*  
    Nombre del archivo: cliente.cpp
    Trabajo Práctico 3 - Ejercicio 5
    Grupo: 12
    Gómez Markowicz, Federico - 38858109
    Kuczerawy, Damián - 37807869
    Mediotte, Facundo - 39436162
    Siculin, Luciano - 39213320
    Tamashiro, Santiago - 39749147
*/

char* memoriaLeer();
void memoriaEscribir(char* mensaje);
char opcion;
int sockfd;

void terminar (int signum) {
  cout << endl << "cerrando..." << endl;
  // envio s al servidor para que se de cuenta que se desconectó el cliente
  opcion = 's';
  write(sockfd,&opcion,sizeof(opcion));
  exit(0);
}

void limpiar_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

int main(int argc, char *argv[])
{
    if (argc == 2) {
      string param(argv[1]);
      if (param == "-help" || param == "-h" || param == "-?") {
        cout << "./cliente ip" << endl;
        cout << "Donde:" << endl;
        cout << "ip: Dirección de IP a donde se conectará el cliente." << endl;
        cout << "El cliente puede enviar una frase para encriptar/desencriptar."
        << " y el servidor realizará la acción deseada." << endl;
        return 0;
      }
    } else {
      cerr << "Error en los parámetros enviados, utilice -help." << endl;
      return -1;
    }
    signal(SIGINT, terminar);
    signal(SIGTERM, terminar);
    char* respuesta;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("Error: No se pudo crear el socket.");
        return 1;
    }

    char recvBuff[1024];
    memset(recvBuff, '0',sizeof(recvBuff));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr); // Error si IP mal escrita

    if ( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("Error: No se pudo conectar.\n");
       return 1;
    }
    char mensaje[250];
    int bytesRecibidos = 0;
    // creo el semáforo y lo inicializo en 1
    // si ya fue creado (por otro cliente) entonces referencia al mismo semáforo y no lo pisa.
    sem_t *semaforo = sem_open("semaforo", O_CREAT, S_IRUSR | S_IWUSR, 1);
    while(1) {
      do {
          puts("Desea encriptar, desencriptar o salir? (e/d/s)");
          scanf("%c",&opcion);
          limpiar_stdin();
      } while(opcion != 'e' && opcion != 'd' && opcion != 's');
      if (opcion == 'e') printf("Escriba mensaje a encriptar: \n");
      else if (opcion == 'd') printf("Escriba mensaje a desencriptar: \n");
      else {
          write(sockfd,&opcion,sizeof(opcion));
          return 0;
      }
      scanf("%s",mensaje);
      sem_wait(semaforo);
      limpiar_stdin();
      write(sockfd,&opcion,sizeof(opcion));
      memoriaEscribir(mensaje);
      read(sockfd,&opcion,sizeof(opcion));
      respuesta=memoriaLeer();
      sem_post(semaforo);
      if(opcion == 'e') printf("El mensaje encriptado es: %s\n",respuesta);
      else printf("El mensaje desencriptado es: %s\n",respuesta);
    }
    return 0;
}

char* memoriaLeer(){
    int pid,memoriaID;
    char *punteroAMemoriaCompartida = NULL;
    memoriaID = shmget(1315511,TAM,0660|IPC_CREAT);
    punteroAMemoriaCompartida = (char*)shmat(memoriaID,NULL,0); //Asociacion
    shmdt(&punteroAMemoriaCompartida); //Desasociacion
    if(shmctl(memoriaID,IPC_RMID,NULL)==-1){
        fprintf(stderr,"Error al liberar la memoria");
    }
    return punteroAMemoriaCompartida;
}

void memoriaEscribir(char *mensaje){
    int pid,memoriaID;
    char *punteroAMemoriaCompartida = NULL;
    if((memoriaID = shmget(1315511,TAM,0660|IPC_CREAT))==-1) { //El primer valor es un identificador unico, puede dar problemas
        fprintf(stderr,"Error al reservar la memoria");
    } //Creo la memoria compartida
    punteroAMemoriaCompartida = (char*)shmat(memoriaID,(void *)0,0); //Asociacion
    strcpy(punteroAMemoriaCompartida,mensaje);
}
