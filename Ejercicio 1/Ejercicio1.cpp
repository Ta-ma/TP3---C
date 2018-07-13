#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>

void hijo();
void nieto();
void nietoZ();
void bisnietoD();
void bisnietoZ();

using namespace std;

int pidPadre;
int i=2;
int bis=3;
int hijos=4;

int main(int argc, char const *argv[])
{
    if (argc == 2) {
        string param(argv[1]);
        if (param == "-help") {
          cout << "./Ej1.exe" << endl;
          cout << "Este ejecutable no lleva parámetros." << endl;
          cout << "Genera una serie de procesos de distinatas características mostradas por pantalla" << endl;
          return 0;
        } else {
          cerr << "Error en los parámetros enviados, utilice -help.";
          return -1;
        }
    } else if (argc != 1) {
        cerr << "Error en la cantidad de parámetros enviados, utilice -help.";
        return -1;
    }

    printf("soy el padre con pid %d\n", getpid());
    int stado;
    while(hijos--){
        bis=hijos-1;
        pidPadre=getpid();
        pid_t pidHijo=fork();
        if(pidHijo != 0){
            hijo();
            exit(1);
        }
        else 
            waitpid(pidHijo,&stado,0);
    }
    return 0;
}

void printear(int generacion, char* parentesco){
    printf("Soy el proceso con PID: %d y pertenezco a la generación N: %d Pid padre: %d Parentesco/Tipo: %s\n", getpid(), generacion, pidPadre, parentesco);
}

void hijo(){
    printear(1,(char*)"hijo");
    int stado;
    if(hijos>0){
        pidPadre=getpid();
        pid_t pidHijo=fork();
        if(!pidHijo){
            nieto();
        }
        else {
            waitpid(pidHijo,&stado,0);
            pidPadre=getpid();
            pid_t pidHijo=fork();
            if(!pidHijo)
                nietoZ();
        }
    }
    if(!hijos)
        nietoZ();
    exit(1);
}

void nieto(){
    printear(2,(char*)"nieto");
    int stado;
    pidPadre=getpid();
    pid_t pidHijo=fork();
    if(bis>0 && !pidHijo){
        bisnietoD();
    }
    else{
        pidPadre=getpid();
        pid_t pidHijo=fork();
        if(bis==1&&!pidHijo)
            bisnietoZ();
    }
    exit(1);
}

void nietoZ(){
    printear(2,(char*)"nieto/zombie");
    sleep(100);
    exit(1);
}

void bisnietoD(){
    printear(3,(char*)"bisnieto/demonio");
    sleep(100);
    exit(1);
}

void bisnietoZ(){
    printear(3,(char*)"bisnieto/zombie");
    sleep(100);
    exit(1);
}