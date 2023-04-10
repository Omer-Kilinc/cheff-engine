all:
	g++ -oFast cheff.cpp -o cheff 
	x86_64-w64-minigw32-gcc -oFast cheff.cpp -o cheff.exe
debug:
	g++ cheff.cpp -o cheff 
	x86_64-w64-minigw32-gcc cheff.cpp -o cheff.exe
