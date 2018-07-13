all: cliente servidor

cliente: cliente.cpp
	g++ -o cliente cliente.cpp -lpthread --std=c++11

servidor: servidor.cpp
	g++ -o servidor servidor.cpp -lpthread --std=c++11

clean:
	rm cliente servidor 2> /dev/null
	
