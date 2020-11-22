Bot:
	gcc -o Bot cosmo-engine.c `curl-config --cflags --libs`
Web:
	gcc -o Web/server Web/server.c

