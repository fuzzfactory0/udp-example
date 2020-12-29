SRV=server
CLI=client

main: udpClient.o udpServer.o
	g++ udpClient.o -o $(CLI)
	g++ udpServer.o -o $(SRV)

udpClient.o: udpClient.cpp

udpServer.o: udpServer.cpp

clean:
	find . -name '*.o' -delete
	rm -rf $(SRV)
	rm -rf $(CLI)
