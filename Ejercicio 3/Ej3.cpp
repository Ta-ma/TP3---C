#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h> // Define constantes de modo
#include <fcntl.h> // Define constantes O_*
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

/* Aca habia usado primitivas de cola, un desastre. */

int main(int argc, char const *argv[]) {
  stringstream stream;
  string nombre_fifo, path_destino;
  int tam_etiqueta;

  pid_t pid, sid;
  char filename[255]; 
  if (argc == 4) {
    nombre_fifo = string(argv[1]);
    tam_etiqueta = stoi(string(argv[2]));
    path_destino = string(argv[3]);
  } else if (argc == 2) {
    string param(argv[1]);
    if (param == "-help" || param == "-h" || param == "-?") {
      cout << "./Ej3.exe nombre_fifo tam_etiqueta path_destino" << endl;
      cout << "Donde:" << endl;
      cout << "nombre_fifo: path del archivo fifo." << endl;
      cout << "tam_etiqueta: tamaño de la etiqueta de los registros." << endl;
      cout << "path_destino: directorio donde se guardarán los archivos." << endl;
      cout << "Ejemplo: ./Ej3.exe /tmp/fifo 8 out" << endl;
      cout << "El programa genera un log.txt para debugging." << endl;
      return 0;
    } else {
      cerr << "Error en los parámetros enviados, utilice -help." << endl;
      return -1;
    }
  } else {
    cerr << "Error en los parámetros enviados, utilice -help." << endl;
    return -1;
  }
  mkfifo(nombre_fifo.c_str(), 0600);
  /* Forkeo el proceso padre */
  pid = fork();
  if (pid < 0) {
      printf("Hubo un error. Saliendo...\n");
      return -1;
  }
  /* Mato al padre  */
  if (pid > 0) {
      sleep(1);
      printf("Fin del proceso padre.\n");
  } else {
    /* Hago que el demonio sea visible para todo el mundo */
    umask(0);
    /* Creo un SID porque al haber matado al padre el hijo puede quedarse 
    zombie entonces con esto el sistema se hace cargo del proceso huérfano
    otorgándole un nuevo SID */
    sid = setsid();
    if (sid < 0) {
        printf("Fallo en la creación del nuevo SID.");
        return -1;
    }
        
    /* Por seguridad, cambio el directorio de trabajo */
    /*if ((chdir("/")) < 0) {
        printf("Fallo en  el cambio de directorio.");
        return -1;
    }   */   

    char buffer[8192] = {0};
    memset(buffer, '\0', 8192);
    // hago un archivo de log para debug
    ofstream log;
    log.open("log.txt");
    while (1) {
      // grabo la hora para el log
      system_clock::time_point p = system_clock::now();
      time_t t = system_clock::to_time_t(p);
      log << ctime(&t) << endl;

      // abro el fifo
      log << "Revisando fifo: " << nombre_fifo.c_str() << endl;
      int fd = open(nombre_fifo.c_str(), O_RDONLY);
      read(fd, buffer, sizeof(buffer));
      string cont_buffer = string(buffer);
      if (!cont_buffer.empty()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        string etiqueta = cont_buffer.substr(0, tam_etiqueta);
        log << "El fifo contiene lo siguiente: " << endl;
        log << "Buffer: " << cont_buffer << endl;
        log << "Etiqueta: " << etiqueta << endl;
        cont_buffer.erase(0, tam_etiqueta);
        log << "Cadena: " << cont_buffer << endl;
        ofstream arch;

        stream << path_destino << "/" << etiqueta << "_" << put_time(&tm, "%Y%m%d") << ".txt";
        arch.open (stream.str().c_str());
        stream.str("");
        arch << cont_buffer;
        arch.close();
      } else {
        log << "No se pudo abrir el fifo." << endl;
      }
      close(fd);
      sleep(2);
    }
  }
  return 0;
}