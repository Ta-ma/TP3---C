#include "servidor.hpp"
#include <thread>
using namespace std;
using namespace chrono;

vector<unique_ptr<Cliente>> clientes;
string nombre_ganador = "yo";
int id_ganador = 0;
int tiempo_inicio = 0;
mutex mtx;
// función para manejar las conexiones con los clientes
void aceptar_conexiones (int puerto) {
  int fd, con, addrlen, opt = 1;
  struct sockaddr_in addr;
  // creación del socket del servidor
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    cerr << "Error al crear socket." << endl;
    exit(-1);
  }
  // configuración del socket
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    cerr << "Error al intentar configurar el socket." << endl;
    exit(-1);
  }
  memset(&addr, '0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(puerto);
  // Vinculación del socket
  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    cerr << "Error al intentar vincular el socket." << endl;
    exit(-1);
  }
  // listening
  listen(fd, 999);

  cout << "Esperando conexiones..." << endl;
  int cont = 0;
  for(;;) {
    cont++;
    addrlen = sizeof(addr);
    // espera a que un cliente se conecte
    con = accept(fd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
    if (con < 0) {
      cerr << "Error al aceptar una conexión." << endl;
    } else if (tiempo_inicio) {
      cout << "Conexión aceptada con éxito." << endl;
      unique_ptr<Cliente> ptrCliente(new Cliente(con, cont, &mtx));
      clientes.push_back(move(ptrCliente));
    } else {
      // se agotó el tiempo de inicio por lo que se rechazarán las conexiones
      send(con, "rechazo:1|", 11, 0);
    }
    sleep(1);
  }
}

// función para limpiar del vector los clientes que se desconectaron
void limpiar_clientes () {
  vector<int> clientes_a_eliminar;
  for (int i = 0; i < clientes.size(); i++) {
    // si el cliente no está vivo, lo marco para eliminar
    if(!(clientes[i] -> esta_vivo()))
      clientes_a_eliminar.push_back(i);
  }
  // elimino clientes
  for (int i = 0; i < clientes_a_eliminar.size(); i++) {
    clientes[clientes_a_eliminar[i]].reset();
    clientes.erase(clientes.begin() + clientes_a_eliminar[i]);
  }
}
// envia un mensaje a todos los clientes
void clientes_broadcast(string comando, string param) {
  for (int j = 0; j < clientes.size(); j++)
      clientes[j] -> enviar(comando, param);
}
// función para decidir el ganador
void decidir_ganador () {
  int ganador = 0, puntos_ganador = 0;
  // si hay tiempo de inicio quiere decir que se cerró antes de comenzar el juego
  // por ende no habrá ganador
  if (tiempo_inicio) return;
  string nombre = "";
  // si todavía hay tiempo de inicio entonces el server no comenzó el juego
  if (tiempo_inicio) return;
  for (int i = 0; i < clientes.size(); i++) {
    int id = clientes[i] -> get_id();
    int puntos = clientes[i] -> get_puntos();
    // en caso de empate, gana el que se conectó primero
    if (puntos > puntos_ganador || (puntos == puntos_ganador && (ganador > id || ganador == 0))) {
      nombre = clientes[i] -> get_nombre();
      puntos_ganador = puntos;
      ganador = id;
    }
  }
  id_ganador = ganador;
  nombre_ganador = nombre;
}
// función para cerrar el server
void terminar (int signum) {
  stringstream stream;
  decidir_ganador();
  stream << id_ganador << " " << nombre_ganador;
  cout << endl;
  cout << "cerrando..." << endl;
  for (int i = 0; i < clientes.size(); i++) {
    clientes[i] -> enviar("salir", stream.str());
    clientes[i] -> cerrar();
  }
  exit(0);
}
// main
int main (int argc, char **argv) {
  map<string, string> config;
  string arch_config = "config.txt";
  vector<Pregunta> preguntas;
  int cant_preguntas, tiempo_respuesta, puerto, fin = 0;
  string arch_preguntas;
  thread net_manager;
  stringstream stream;
  // atrapar señales en caso de cierre
  signal(SIGINT, terminar);
  signal(SIGTERM, terminar);

  if (argc == 2) {
    // si hay 2 parámetros, el segundo puede ser -help o archivo de config
    string param(argv[1]);
    if (param == "-help" || param == "-h" || param == "-?") {
      cout << "Hay 3 formas de llamar a este programa:" << endl;
      cout << "./servidor.exe" << endl;
      cout << "El programa buscará el archivo de configuración por defecto llamado config.txt y tomará de allí los parámetros." << endl;
      cout << "---------------------------------------" << endl;
      cout << "./servidor.exe arch_config" << endl;
      cout << "El programa buscará el archivo de configuración en el path arch_config." << endl;
      cout << "---------------------------------------" << endl;
      cout << "El archivo de configuración debe tener el siguiente formato:" << endl;
      cout << "puerto: <Número de puerto a utilizar por el servidor.>" << endl;
      cout << "archivo_preguntas: <Archivo que contiene las preguntas.>" << endl;
      cout << "cant_preguntas: <Cantidad de preguntas que se les preguntaran a los jugadores en la partida.>" << endl;
      cout << "tiempo_inicio: <Tiempo en segundos que esperará el servidor para aceptar conexiones.>" << endl;
      cout << "tiempo_respuesta: <Tiempo en segundos que tienen los jugadores para responder.>" << endl;
      cout << "./servidor.exe puerto archivo_preguntas cant_preguntas tiempo_inicio tiempo_respuesta" << endl;
      cout << "Los parámetros a enviar son los mismos que los mencionados previamente." << endl;
      cout << "---------------------------------------" << endl;
      return 0;
    } else arch_config = param;
  } else if (argc == 6) {
    config["puerto"] = string(argv[1]);
    config["archivo_preguntas"] = string(argv[2]);
    config["cant_preguntas"] = string(argv[3]);
    config["tiempo_inicio"] = string(argv[4]);
    config["tiempo_respuesta"] = string(argv[5]);
  } else if (argc != 1) {
    cerr << "Error en la cantidad de parámetros enviados, utilice -help." << endl;
    return -1;
  }

  // leer archivo de configuración
  // si se pasaron los parámetros por línea entonces no es necesario leerlo
  if (argc != 5) {
    cout << "Leyendo archivo de configuración..." << endl;
    config = leer_config(arch_config);
  }

  // validación de parámetros
  arch_preguntas = config["archivo_preguntas"];
  if (arch_preguntas.empty()) {
    cerr << "El archivo de preguntas no ha sido especificado por parámetro "
    << "ni por archivo de configuraciones." << endl;
    return -1;
  }
  try {
    puerto = stoi(config["puerto"]);
  } catch (invalid_argument e) {
    cerr << "El parámetro puerto no ha sido especificado " <<
    "o no es un número entero válido." << endl;
    return -1;
  }
  try {
    cant_preguntas = stoi(config["cant_preguntas"]);
  } catch (invalid_argument e) {
    cerr << "El parámetro cantidad de preguntas no ha sido especificado " <<
    "o no es un número entero válido." << endl;
    return -1;
  }
  try {
    tiempo_inicio = stoi(config["tiempo_inicio"]);
  } catch (invalid_argument e) {
    cerr << "El parámetro tiempo de inicio no ha sido especificado " <<
    "o no es un número entero válido." << endl;
    return -1;
  }
  try {
    tiempo_respuesta = stoi(config["tiempo_respuesta"]);
  } catch (invalid_argument e) {
    cerr << "El parámetro tiempo de respuesta no ha sido especificado " <<
    "o no es un número entero válido." << endl;
    return -1;
  }
  cout << "Inicializando servidor..." << endl;
  cout << "--Parámetros cargados con éxito--" << endl;
  cout << "Puerto: " << puerto << endl;
  cout << "Archivo de preguntas: " << arch_preguntas << endl;
  cout << "Cantidad de preguntas: " << cant_preguntas << endl;
  cout << "Tiempo de inicio: " << tiempo_inicio << endl;
  cout << "Tiempo de respuesta: " << tiempo_respuesta << endl;
  cout << "---------------------------------" << endl;

  // leer archivo de preguntas
  try {
    preguntas = leer_preguntas(arch_preguntas);
  } catch (runtime_error e) {
    cerr << "Error al leer archivo de preguntas:" << endl;
    cerr << e.what() << endl;
    return -1;
  }
  if (preguntas.size() < cant_preguntas) {
    cerr << "La cantidad de preguntas del juego es mayor que la cantidad de preguntas " <<
    "cargadas en el archivo de preguntas!" << endl;
    return -1;
  }
  // thread para aceptar conexiones
  net_manager = thread(&aceptar_conexiones, puerto);
  net_manager.detach();
  // contador del tiempo de inicio, cuando se acaba se rechazarán las conexiones
  while (tiempo_inicio) {
    sleep(1);
    tiempo_inicio--;
  }
  cout << "Tiempo de espera de conexiones agotado." << endl;
  limpiar_clientes();
  if(!clientes. size()) {
    cout << "Cerrando servidor debido a que no hay jugadores." << endl;
    exit(0);
  }
  cout << "Los jugadores conectados son:" << endl;
  clientes_broadcast("nota", "Los jugadores conectados son:");
  for (int i = 0; i < clientes.size(); i++) {
    clientes[i] -> mostrar();
    for (int j = 0; j < clientes.size(); j++)
      clientes[i] -> enviar("nota", clientes[j] -> mostrar_string());
  }
  cout << "El juego comenzará en 5 segundos." << endl;
  clientes_broadcast("nota", "El juego comenzará en 5 segundos.");
  stream << "Tiene " << tiempo_respuesta << " segundos para responder cada pregunta." << endl;
  clientes_broadcast("nota", stream.str());
  stream.str("");
  sleep(5);

  int cont = 0, tiempo = 0;
  while(cant_preguntas) {
    if(!clientes. size()) {
      cout << "Cerrando servidor debido a que se han desconectado todos los jugadores." << endl;
      exit(0);
    }
    cont++;
    tiempo = tiempo_respuesta;
    cout << "Ronda: " << cont << endl;
    preguntas[cont - 1].mostrar();
    for (int i = 0; i < clientes.size(); i++) {
      clientes[i] -> jugar_ronda(cont, preguntas[cont - 1]);
    }
    // inicio de timer para responder
    auto start = high_resolution_clock::now();
    int jugador_mas_rapido = 0, ind_mas_rapido = 0, respuesta_mas_rapido = 0;
    double tiempo_mas_rapido = 0;
    while(tiempo) {
      sleep(1);
      tiempo--;
    }
    // fin de ronda
    cout << "Ronda finalizada." << endl;
    if(!clientes. size()) {
      cout << "Cerrando servidor debido a que se han desconectado todos los jugadores." << endl;
      exit(0);
    }
    clientes_broadcast("nota", "-----------------------------------------------");
    clientes_broadcast("fin_ronda", to_string(preguntas[cont - 1].get_respuesta_correcta()));
    // calcular quien ganó
    for (int i = 0; i < clientes.size(); i++) {
      time_point<high_resolution_clock> end = clientes[i] -> get_tiempo();
      duration<double, std::milli> tmp = end - start;
      if((tmp.count() < tiempo_mas_rapido || tiempo_mas_rapido == 0) &&
      clientes[i] -> get_respuesta() != 0) {
        respuesta_mas_rapido = clientes[i] -> get_respuesta();
        tiempo_mas_rapido = tmp.count();
        ind_mas_rapido = i;
      }
    }
    // me fijo si su respuesta era correcta
    if (respuesta_mas_rapido != 0) {
      if(respuesta_mas_rapido == preguntas[cont - 1].get_respuesta_correcta()) {
        stream << "El que más rápido respondió fue el " << clientes[ind_mas_rapido] -> mostrar_string()
        << " y su respuesta fue correcta por lo que suma 1 punto." << endl;
        clientes_broadcast("nota", stream.str());
        stream.str("");
        clientes[ind_mas_rapido] -> set_puntos(clientes[ind_mas_rapido] -> get_puntos() + 1);
      } else {
        stream << "El que más rápido respondió fue el " << clientes[ind_mas_rapido] -> mostrar_string()
        << " pero su respuesta fue incorrecta por lo que resta 1 punto." << endl;
        clientes_broadcast("nota", stream.str());
        stream.str("");
        clientes[ind_mas_rapido] -> set_puntos(clientes[ind_mas_rapido] -> get_puntos() - 1);
      }
    } else {
      clientes_broadcast("nota", "Nadie respondió la pregunta.");
    }
    // mostrar puntajes
    for (int i = 0; i < clientes.size(); i++)
      clientes_broadcast("nota", clientes[i] -> mostrar_puntaje());
    cant_preguntas--;
    // antes de comenzar la siguiente ronda, limpiar jugadores que se hayan desconectado
    limpiar_clientes();
    if (cant_preguntas)
      clientes_broadcast("nota", "La siguiente ronda comenzará en 5 segundos...");
    else
      clientes_broadcast("nota", "No hay más preguntas, el juego finalizará en 5 segundos...");
    sleep(5);
  }
  // cuando sale del bucle, se acabaron las preguntas por ende hay que decidir ganador
  clientes_broadcast("nota", "-----------------------------------------------");
  clientes_broadcast("nota", "El juego ha finalizado");
  terminar(0);
}
