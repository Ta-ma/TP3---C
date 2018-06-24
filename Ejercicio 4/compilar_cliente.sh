#!/bin/bash
g++ cliente_utils.cpp cliente.cpp -o cliente.exe -lpthread

if [ $? -eq 0 ]; then
    echo "cliente compilado, ejecutando..."
    ./cliente.exe $1 $2 $3
fi

