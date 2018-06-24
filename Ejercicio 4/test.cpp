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
#include <mutex>
using namespace std;
int fuck = 0;

class Listener {
  mutex *mtx;
  thread hilo;
  int id, privint;
  void escuchar();

  public:
    Listener();
    Listener(int id, mutex *mtx);
    void enviar(string comando, string param);
    void cerrar();
    int getId();
};

Listener::Listener(int id, mutex *mtx) {
  Listener::id = id;
  privint = 0;
  Listener::mtx = mtx;
  hilo = thread(&Listener::escuchar, this);
  hilo.detach();
}

void Listener::escuchar() {
  for(;;) {
    mtx -> lock();
    privint += id;
    fuck++;
    cout << "Thread: " << id << " privint: " << privint << " fuck: " << fuck << endl;
    mtx -> unlock();
    sleep(1);
  }
}

void bucle() {
  while (1) {
    fuck++;
    cout << "Thread: " << &fuck << endl;
    sleep(1);
  }
}

int main(int argc, char const *argv[]) {
  mutex mtx;
  /*thread hilo;
  hilo = thread(bucle);
  hilo.detach();
  
  while (1) {
    cout << "Main: " << &fuck << endl;
    sleep(1);
  }*/
  vector<Listener*> ls;
  Listener *l = new Listener(1, &mtx);
  ls.push_back(l);
  l = new Listener(2, &mtx);
  ls.push_back(l);
  /*Listener l1(1);
  Listener l2(2);*/

  for(;;) {
    sleep(1);
  }
}