// FIFO
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h> // Define constantes de modo
#include <fcntl.h> // Define constantes O_*
#include <semaphore.h>
#include <string.h>
#include <sys/wait.h>
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
using namespace std;

int main(int argc, char const *argv[])
{
  cout << "1" << endl;
  mkfifo("/tmp/fifo", 0600);
  cout << "2" << endl;
  char buffer[8192] = {0};
  memset(buffer, '\0', 8192);
  cout << "3" << endl;
  int fd = open("/tmp/fifo", O_WRONLY);
  cout << "4" << endl;
  string cadena;
  cin >> cadena;
  write(fd, cadena.c_str(), cadena.length() + 1);
  cout << "5" << endl;
  close(fd);
  /*cout << "6" << endl;
  int fd2 = open("/tmp/fifo", O_RDONLY);
  cout << "7" << endl;
  read(fd2, buffer, sizeof(buffer));
  cout << "8" << endl;
  string cont_buffer = string(buffer);
  cout << cont_buffer << endl;
  cout << "9" << endl;
  close(fd2);*/

  cout << "end" << endl;
  return 0;
}