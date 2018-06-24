#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <string.h>
#include <signal.h>
#include <mutex>
#include <memory>
#include <chrono>
using namespace std;
using namespace chrono;
// Clases
class Pregunta {
  string texto;
  int r_correcta;
  vector <string> opciones;

  public:
    Pregunta();
    Pregunta(string texto, int p_correcta, vector <string> opciones);
    void mostrar();
    string mostrar_string();
    int get_respuesta_correcta();
};

class Cliente {
  mutex *mtx;
  thread hilo;
  string nombre;
  int con, id, vivo, respuesta, puntos;
  char buffer[8192] = {0};
  void escuchar();
  Pregunta pregunta;
  time_point<high_resolution_clock> tiempo;

  public:
    Cliente(int con, int id, mutex *mtx);
    void enviar(string comando, string param);
    void cerrar();
    void mostrar();
    string mostrar_string();
    string mostrar_puntaje();
    int esta_vivo();
    int get_puntos();
    void set_puntos(int puntos);
    int get_id();
    int get_respuesta();
    time_point<high_resolution_clock> get_tiempo();
    string get_nombre();
    void jugar_ronda(int num, Pregunta& pregunta);
};

// Funciones
vector<string> parse(string str, const string& delimitador);
string trim(const string& str, const string& whitespace);
map<string, string> leer_config(string archivo);
vector<Pregunta> leer_preguntas(string archivo);
void aceptar_conexiones(int puerto);
void terminar (int signum);