Bot main.c:
  gcc -o Bot main.c `curl-config --cflags --libs` && ./Bot
