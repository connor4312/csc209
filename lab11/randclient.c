#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef PORT
  #define PORT 30000
#endif

#define TIMES 5 // number of times to send the message
#define MINCHARS 3
#define MAXCHARS 7

int main(int argc, char** argv) {
  int soc;
  char message[18] = "A stitch in time\r\n";
  struct sockaddr_in peer;

  int current_byte, bytes_left, total_bytes, howmany;
  char piece[MAXCHARS];

  if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("randclient: socket");
    exit(1);
  }

  peer.sin_family = AF_INET;
  peer.sin_port = htons(PORT);
  if (inet_pton(AF_INET, argv[1], &peer.sin_addr) < 1) {
    perror("randclient: inet_pton");
    close(soc);
    exit(1);
  }

  if (connect(soc, (struct sockaddr *)&peer, sizeof(peer)) == -1) {
    perror("randclient: connect");
    exit(1);
  }

  total_bytes = TIMES * sizeof(message);
  current_byte = 0;
  while (current_byte < total_bytes) {
    howmany = rand() % (MAXCHARS - MINCHARS + 1) + MINCHARS;
    bytes_left = total_bytes - current_byte;
    if (howmany > bytes_left) {
      howmany = bytes_left;
    }
    for (int i = 0; i < howmany; i++) {
      piece[i] = message[(current_byte + i) % sizeof(message)];
    }
    write(soc, piece, howmany);
    current_byte += howmany;
  }
  close(soc);
  return 0;
}
