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
#include <arpa/inet.h>
#include <thread>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <memory>
#include <netdb.h>
using namespace std;

class Listener {
  char buffer[8192];
  thread hilo;
  int en_ronda, id, vivo, con, respuesta;
  void escuchar();

  public:
    Listener();
    Listener(int con);
    void enviar(string comando, string param);
    void cerrar();
    int getId();
    void elegir_respuesta(int respuesta);
};

// Funciones
vector<string> parse(string str, const string& delimitador);
string trim(const string& str, const string& whitespace);
int host_a_ip(string host, string* ip);