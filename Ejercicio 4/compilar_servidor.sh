#!/bin/bash
g++ servidor_utils.cpp servidor.cpp -o servidor.exe -lpthread

if [ $? -eq 0 ]; then
    echo "servidor compilado, ejecutando..."
    ./servidor.exe
fi
