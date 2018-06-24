#include "servidor.hpp"
using namespace std;

// Funciones
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
// Leer archivo de configuración
map<string, string> leer_config (string archivo) {
  stringstream ss;
  map<string, string> config;
  string linea;
  ifstream arch(archivo);

  if (arch.is_open()) {
    string p_texto;
    vector<string> p_respuestas;
    while (getline (arch, linea)) {
      // parsea la linea
      if(!linea.empty()) {
        int ind = linea.find_first_of(':', 0);
        string tipo = linea.substr(0, ind);

        // levanta los parámetros del archivo de configuración
        config[tipo] = trim(linea.substr(ind + 1, linea.length()));
      }
    }
    arch.close();
  } else {
    ss << "El archivo " << archivo << " no existe o no es accesible.";
    throw runtime_error(ss.str());
  }
  
  return config;
}

// Leer archivo de preguntas
vector<Pregunta> leer_preguntas (string archivo) {
  stringstream ss;
  vector<Pregunta> preguntas;
  string linea;
  ifstream arch(archivo);

  if (arch.is_open()) {
    int cont_resp = 0;
    int cont_pregs = 0;
    int p_rc = 0;
    string p_texto;
    vector<string> p_respuestas;
    while (getline (arch, linea)) {
      // parsea la linea
      if(!linea.empty()) {
        int ind = linea.find_first_of(':', 0);
        string tipo = linea.substr(0, ind);

        if (tipo == "R") {
          cont_resp++;
          p_respuestas.push_back(trim(linea.substr(ind + 1, linea.length())));
        } else if (tipo == "RC") {
          cont_resp++;
          p_respuestas.push_back(trim(linea.substr(ind + 1, linea.length())));
          p_rc = cont_resp;
        }

        if (tipo == "P" || arch.eof()) {
          if (cont_pregs != 0) {
            // valido cosas
            if (cont_resp == 0) {
              ss << "La pregunta " << cont_pregs << " no tiene respuesta correcta.";
              throw runtime_error(ss.str());
            } else if (p_rc == 0) {
              ss << "La pregunta " << cont_pregs << " no tiene respuesta ninguna respuesta.";
              throw runtime_error(ss.str());
            }
            // agrego la pregunta en el vector de preguntas
            preguntas.push_back(Pregunta(p_texto, p_rc, p_respuestas));
            // reseteo todas las variables
            p_rc = 0;
            cont_resp = 0;
            p_respuestas.clear();
          }

          cont_pregs++;
          p_texto = trim(linea.substr(ind + 1, linea.length()));
        }
      }
    }
    arch.close();
  } else {
    ss << "El archivo " << archivo << " no existe o no es accesible.";
    throw runtime_error(ss.str());
  }
  
  return preguntas;
}

// Clases
// Pregunta
Pregunta::Pregunta() {}

Pregunta::Pregunta(string texto, int r_correcta, vector <string> opciones) {
  Pregunta::texto = texto;
  Pregunta::r_correcta = r_correcta;
  Pregunta::opciones = opciones;
}

void Pregunta::mostrar() {
  cout << "Pregunta: " << texto << endl;
  int cont = 1;
  for (auto x : opciones) {
    cout << cont << ") " << x << endl;
    cont++;
  }
  cout << "La respuesta correcta es: " << r_correcta << endl;
}

string Pregunta::mostrar_string() {
  stringstream stream;
  stream << "Pregunta: " << texto << endl;
  int cont = 1;
  for (auto x : opciones) {
    stream << cont << ") " << x << endl;
    cont++;
  }
  return stream.str();
}

int Pregunta::get_respuesta_correcta() { return r_correcta; }

// Cliente
Cliente::Cliente(int con, int id, mutex *mtx) {
  memset(buffer, '\0', 8192);
  Cliente::id = id;
  Cliente::con = con;
  Cliente::mtx = mtx;
  puntos = respuesta = 0;
  hilo = thread(&Cliente::escuchar, this);
  hilo.detach();
  vivo = 1;
  mostrar();
}

void Cliente::enviar (string comando, string param) {
  // si el cliente murió no mando nada 
  if (!vivo) return;
  stringstream stream;
  stream << comando << ":" << param << "|";
  //cout << "Mensaje enviado a " << id << ": " << stream.str() << endl;
  send(con, stream.str().c_str(), stream.str().length(), 0);
}

void Cliente::escuchar() {
  while (vivo) {
    int n = read(con, buffer, 8192);
    mtx -> lock();
    //cout << "Mensaje recibido del jugador " << id << " (" << nombre << "): "<< buffer << endl;
    if (n < 0) {
      cerr << "Error al leer mensaje del cliente " << id << endl;
      return;
    }
    // parsear mensaje
    for (auto mensaje : parse(string(buffer), "|")) {
      int ind = mensaje.find_first_of(':', 0);
      string comando = mensaje.substr(0, ind);
      string param = trim(mensaje.substr(ind + 1, mensaje.length()));

      if (comando == "drop") {
        // el comando drop no hace nada en el cliente, es para que salga del bloqueo del read
        Cliente::enviar("drop", "1");
        // el cliente se desconecta
        Cliente::cerrar();
        cout << "El cliente " << id << " (" << nombre << ") se ha desconectado." << endl;
      } else if (comando == "nombre") {
        nombre = param;
        cout << "El jugador " << param << " se ha conectado al servidor." << endl;
        Cliente::enviar("hola", to_string(id));
      } else if (comando == "respuesta") {
        respuesta = stoi(param);
        tiempo = high_resolution_clock::now();
      }
    }
    mtx -> unlock();
    memset(buffer, '\0', 8192);
    sleep(1);
  }

  cout << "Hilo del cliente " << id << " (" << nombre << ") terminado." << endl;
}

void Cliente::cerrar() {
  close(con);
  // el thread debe morir al poner vivo en 0 porque sale del bucle while
  vivo = 0;
}

void Cliente::mostrar() {
  cout << "Jugador " << id << ", nombre: " << nombre << endl;
}

string Cliente::mostrar_string() {
  stringstream stream;
  stream << "Jugador " << id << ", nombre: " << nombre;
  return stream.str();
}

string Cliente::mostrar_puntaje() {
  stringstream stream;
  stream << "El jugador " << id << " (" << nombre << ") tiene " << puntos << " puntos.";
  return stream.str();
}
void Cliente::jugar_ronda(int num, Pregunta& pregunta) {
  Cliente::pregunta = pregunta;
  stringstream stream;
  enviar("ronda", to_string(num));
  enviar("nota", pregunta.mostrar_string());
}

int Cliente::esta_vivo() { return vivo; }

int Cliente::get_puntos() { return puntos; }

void Cliente::set_puntos(int puntos) { Cliente::puntos = puntos; }

int Cliente::get_id() { return id; }

string Cliente::get_nombre() { return nombre; }

int Cliente::get_respuesta() { return respuesta; }

time_point<high_resolution_clock> Cliente::get_tiempo() { return tiempo; }