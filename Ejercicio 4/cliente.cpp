#include "cliente.hpp"
using namespace std;

unique_ptr<Listener> listener;
// función para cerrar la aplicación
void terminar (int signum) {
  cout << endl << "cerrando..." << endl;
  listener -> enviar("drop", to_string(listener -> getId()));
  listener -> cerrar();
  exit(0);
}

int main(int argc, char const *argv[]) {
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  string nombre, ip, puerto;
  // atrapar señales en caso de cierre
  signal(SIGINT, terminar);
  signal(SIGTERM, terminar);

  if (argc == 2) {
    // si hay 2 parámetros, el segundo puede ser -help o archivo de config
    string param(argv[1]);
    if (param == "-help") {
      cout << "./cliente.exe ip puerto nombre" << endl;
      cout << "Donde:" << endl;
      cout << "ip: Número de dirección IP a donde se conectará el cliente." << endl;
      cout << "puerto: Número de puerto a donde se conectará el cliente." << endl;
      cout << "nombre: Nombre de jugador con el que se identificará." << endl;
      return 0;
    }
  } else if (argc == 4) {
    ip = string(argv[1]);
    puerto = string(argv[2]);
    nombre = string(argv[3]);
  } else {
    cerr << "Error en la cantidad de parámetros enviados, utilice -help.";
    return -1;
  }
  
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    cerr << "Error al crear socket." << endl;
    return -1;
  }
  // configurar serv_addr para conectarse al server
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(puerto.c_str()));
  if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
    // si esto falla puede ser porque el flaco pasó el nombre de maquina por parámetro

    cerr << "Error al intentar asignar la dirección IP." << endl;
    return -1;
  }
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    cerr << "Error al conectar con el servidor." << endl;
    return -1;
  }
  // creo el listener para comunicarse con el server
  listener = unique_ptr<Listener>(new Listener(sock));

  listener -> enviar("nombre", nombre);
  cout << "Conectado al servidor." << endl;

  while(1) {
    int input;
    if (cin >> input) {
      if (input > 0)
        listener -> elegir_respuesta(input);
      else cout << "El número debe ser entero mayor a 0!" << endl;
    } else {
      cout << "Solo puede ingresar valores enteros!" << endl;
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
  }
  
  return 0;
}
