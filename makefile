all:server client f_client put multiconn
server:server.o
	g++ -g -o server server.o -pthread -Wl,--no-as-needed
client:client.o
	g++ -g -o client client.o
f_client:file_client.o
	g++ -g -o f_client file_client.o
put:put2map.o
	g++ -g -o put put2map.o
multiconn:multiconn.o
    g++ -g -o multiconn multiconn.o -pthread -Wl,--no-as-needed
server.o:server.cpp
	g++ -g -c server.cpp -std=c++11
client.o:client.cpp
	g++ -g -c client.cpp -std=c++11
file_client.o:file_client.cpp
	g++ -g -c file_client.cpp -std=c++11
put2map.o:put2map.cpp
	g++ -g -c put2map.cpp -std=c++11 
multiconn.o:multiconn.cpp
    g++ -g -c multiconn.cpp -std=c++11 
clean:all
	rm all