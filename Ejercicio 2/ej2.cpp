#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;
using namespace std::chrono;

/*  
    Nombre del archivo: ej2.cpp
    Trabajo Práctico 3 - Ejercicio 2
    Grupo: 12
    Gómez Markowicz, Federico - 38858109
    Kuczerawy, Damián - 37807869
    Mediotte, Facundo - 39436162
    Siculin, Luciano - 39213320
    Tamashiro, Santiago - 39749147
*/

int vocales[] = {65, 97, 69, 101, 73, 105, 79, 111, 85, 117};
string p_in, p_out, primer_archivo, ult_archivo;
int n = 0, archivos_analizados = 0;
vector<string> archivos;
vector<int> tam_archivos;
mutex mtx_out, mtx_cont;

// devuelve 1 si es vocal, 2 si es consonante o 3 si es otro
int determinar_caracter (char c) {
  for (int i = 0; i < 10; i++) {
    if (c == vocales[i]) {
      //cout << c << " es voc." << endl;
      return 1;
    }
  }
  if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 164 && c <= 165)) {
    //cout << c << " es con." << endl;
    return 2;
  } else {
    //cout << c << " es otr." << endl;
    return 3;
  }
}
// devuelve la cantidad de caracteres analizados
int buscar_caracteres (string &linea, int* c_voc, int* c_con, int* c_otr) {
  int total = 0;
  for(int i = 0; i < linea.size(); i++) {
    total++;
    // evaluo caracter
    switch (determinar_caracter(linea[i])) {
      case 1:
        (*c_voc)++;
        break;
      case 2:
        (*c_con)++;
        break;
      case 3:
        (*c_otr)++;
        break;
    }
  }
  return total;
}

void analizar (int n_thread) {
  int offset = 0;
  int ind = (n_thread + offset) - 1;
  int c_voc = 0, c_con = 0, c_otr = 0;
  stringstream stream;
  vector<string> arch_analizados;
  // hora actual
  system_clock::time_point p = system_clock::now();
  time_t t = system_clock::to_time_t(p);
  string hora_inicio = string(ctime(&t));

  while (ind < archivos.size()) {
    // nombre del archivo a analizar
    string nombre_arch = archivos[ind];
    // agrego el archivo a la lista de analizados
    arch_analizados.push_back(nombre_arch);
    // compongo el path del archivo
    if(p_in[p_in.size() - 1] == '/')
      stream << p_in << nombre_arch;
    else stream << p_in << "/" << nombre_arch;
    // abro archivo
    ifstream arch(stream.str());
    if (arch.is_open()) {
      string linea;
      while (getline(arch, linea)) {
        //cout << linea << endl;
        // busco caracteres
        tam_archivos[ind] += buscar_caracteres(linea, &c_voc, &c_con, &c_otr);
      }
    }
    arch.close();
    stream.str("");
    offset += n;
    ind = (n_thread + offset) - 1;
    // escribo archivo de salida
    // hora fin
    p = system_clock::now();
    t = system_clock::to_time_t(p);
    string hora_fin = string(ctime(&t));
    // compongo el path del archivo
    if(p_out[p_out.size() - 1] == '/')
      stream << p_out << nombre_arch;
    else stream << p_out << "/" << nombre_arch;
    ofstream arch_out;
    arch_out.open(stream.str());
    stream.str("");
    arch_out << "Hora inicio: " << hora_inicio;
    arch_out << "Thread N°" << n_thread << endl;
    arch_out << "Vocales: " << c_voc << endl;
    arch_out << "Consonantes: " << c_con << endl;
    arch_out << "Otros: " << c_otr << endl;
    arch_out << "Hora fin: " << hora_fin;
    // reseteo variables
    c_voc = 0, c_con = 0, c_otr = 0;
    arch_out.close();
    // me fijo el tema de primer o último archivo
    mtx_cont.lock();
    archivos_analizados++;
    if(archivos_analizados == 1) 
      primer_archivo = nombre_arch;
    if (archivos_analizados == archivos.size())
      ult_archivo = nombre_arch;
    mtx_cont.unlock();
  }
  // final del procesamiento de archivos
  // muestro info por pantalla
  mtx_out.lock();
  if (arch_analizados.size() == 0) {
    cout << "El thread " << n_thread << " no hizo nada." << endl;
    mtx_out.unlock();
    return;
  }
  cout << "Archivos analizados por el thread " << n_thread << ": ";
  for(int i = 0; i < arch_analizados.size(); i++)
    cout << arch_analizados[i] << " ";
  cout << endl;
  mtx_out.unlock();
}

int main(int argc, char const *argv[])
{
  if (argc == 2) {
    string param(argv[1]);
    if (param == "-help" || param == "-h" || param == "-?") {
      cout << "./ej2.exe p_in p_out n" << endl;
      cout << "Donde: " << endl;
      cout << "p_in: Directorio de entrada donde estarán los archivos de texto." << endl;
      cout << "p_out: Directorio de salida donde se generarán los resultados." << endl;
      cout << "n: Cantidad de threads en los que se repartirán los archivos." << endl;
      cout << "En la carpeta de salida, se generarán archivos de texto conteniendo" << endl;
      cout << "datos variados sobre los archivos de entrada." << endl;
      return 0;
    } else {
      cerr << "Error en los parámetros enviados, utilice -help.";
      return -1;
    }
  } else if (argc == 4) {
    p_in = string(argv[1]);
    p_out = string(argv[2]);
    n = stoi(string(argv[3]));
  } else {
    cerr << "Error en la cantidad de parámetros enviados, utilice -help.";
    return -1;
  }
  // valido datos
  if (n < 1) {
    cerr << "La cantidad de threads debe ser 1 o mayor!" << endl;
    return -1;
  }
  if (p_in == p_out) {
    cerr << "El directorio de entrada no puede ser el mismo que el de salida!" << endl;
    return -1;
  }
  // busco archivos en p_in
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(p_in.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      string nombre_arch = string(ent -> d_name);
      if (nombre_arch != "." && nombre_arch != "..")
        archivos.push_back(string(ent -> d_name));
    }
    closedir(dir);
  } else {
    perror("No se pudo arbrir el directorio de entrada");
    return -1;
  }
  // valido dir de entrada
  if (archivos.size() == 0) {
    cerr << "No se realizó ninguna operación debido a que no hay ningún archivo en el directorio de entrada." << endl;
    return -1;
  }
  // limpio vector de ints
  for (int i = 0; i < archivos.size(); i++)
    tam_archivos.push_back(0);
  // creo n threads
  thread hilo[n];
  for (int i = 0; i < n; i++) {
    hilo[i] = thread(&analizar, i + 1);
    hilo[i].join();
  }
  // busco menor y mayor
  int menor = 0, mayor = 0;
  string arch_menor, arch_mayor;
  for(int i = 0; i < tam_archivos.size(); i++) {
    int val = tam_archivos[i];
    if (menor > val || menor == 0) {
      menor = val;
      arch_menor = archivos[i];
    }
    if (mayor < val || mayor == 0) {
      mayor = val;
      arch_mayor = archivos[i];
    }
  }
  cout << "--------------------------------------------------------" << endl;
  cout << "El primer archivo analizado fue " << primer_archivo << endl;
  cout << "El último archivo analizado fue " << ult_archivo << endl;
  cout << "El archivo con mayor cantidad de caracteres totales es " << arch_mayor
  << " con un total de " << mayor << " caracteres." << endl;
  cout << "El archivo con menor cantidad de caracteres totales es " << arch_menor
  << " con un total de " << menor << " caracteres." << endl;
  return 0;
}