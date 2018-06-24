#include "cliente.hpp"
using namespace std;

// implementación de la función trim para c++
string trim(const string& str, const string& whitespace = " \t")
{
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return "";

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

vector<string> parse(string str, const string& delimitador) {
  vector<string> muestras;
  size_t pos = 0;
  string muestra;
  while ((pos = str.find(delimitador)) != string::npos) {
      muestra = str.substr(0, pos);
      muestras.push_back(muestra);
      str.erase(0, pos + delimitador.length());
  }
  if(!str.empty())
    muestras.push_back(str);
  
  return muestras;
}

// Listener
Listener::Listener() {}

Listener::Listener(int con) {
  memset(buffer, '\0', 8192);
  Listener::con = con;
  en_ronda = respuesta = 0;
  hilo = thread(&Listener::escuchar, this);
  hilo.detach();
  vivo = 1;
}

void Listener::enviar (string comando, string param) {
  stringstream stream;
  stream << comando << ":" << param << "|";
  //cout << "Mensaje enviado: " << stream.str() << endl;
  send(con, stream.str().c_str(), stream.str().length(), 0);
}

void Listener::escuchar() {
  while (vivo) {
    int n = read(con, buffer, 8192);
    //cout << "Mensaje recibido: " << buffer << endl;
    if (n < 0) {
      vivo = 0;
      cerr << "Error al leer mensaje del servidor." << endl;
      cerr << strerror(errno) << endl;
    }
    //string sbuffer = string(buffer)
    // parsear mensaje
    for (auto mensaje : parse(string(buffer), "|")) {
      int ind = mensaje.find_first_of(':', 0);
      string comando = mensaje.substr(0, ind);
      string param = trim(mensaje.substr(ind + 1, mensaje.length()));

      if (comando == "salir") {
        // el server se desconecta
        vector<string> params = parse(param, " ");
        cout << "El servidor se ha cerrado." << endl;
        if (params[0] == "0")
          cout << "No hay ganador debido a que el servidor se cerró antes de que comenzara el juego." << endl;
        else
          cout << "El ganador es el jugador N°" << params[0] << " (" << params[1] <<") " << endl;
        Listener::cerrar();
      } else if (comando == "nota") {
        // las notas son usadas por el server para simplemente mostrar algo por pantalla
        // en el cliente
        cout << param << endl;
      } else if (comando == "hola") {
        // el servidor le da la bienvenida al cliente
        id = stoi(param);
        cout << "Usted es el jugador N°:" << id << endl;
        cout << "Esperando a que el servidor inicie el juego..." << endl;
      } else if (comando == "rechazo") {
        cout << "El servidor rechazó la conexión debido a que se acabó el tiempo de espera para conectarse." << id << endl;
        Listener::cerrar();
      } else if (comando == "ronda") {
        system("clear");
        cout << "Ronda " << param[0] << endl;
        en_ronda = 1;
      } else if (comando == "fin_ronda") {
        cout << "Se acabó el tiempo para responder." << endl;
        cout << "La respuesta correcta era " << param << endl;
        en_ronda = 0;
      }
    }
    memset(buffer, '\0', 8192);
    sleep(1);
  }
  cout << "Hilo de escucha del servidor terminado." << endl;
}

void Listener::cerrar() {
  close(con);
  vivo = 0;
  // cuando se cierra la conexión se mata al programa porque es al pedo seguir
  exit(0);
}

void Listener::elegir_respuesta (int respuesta) {
  if (en_ronda) {
    enviar("respuesta", to_string(respuesta));
    en_ronda = 0;
    cout << "Respuesta enviada." << endl;
  } else cout << "No debe ingresar nada mientras no haya una ronda en juego!" << endl;
}

int Listener::getId() { return Listener::id; }