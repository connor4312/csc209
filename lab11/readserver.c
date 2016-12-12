#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#ifndef PORT
  #define PORT 30000
#endif

int setup (void) {
  int on = 1;
  struct sockaddr_in self;
  int listenfd;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // Make sure we can reuse the port immediately after the server terminates.
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                 &on, sizeof(on)) == -1) {
    perror("setsockopt -- REUSEADDR");
  }

  memset(&self, '\0', sizeof(self));
  self.sin_family = AF_INET;
  self.sin_addr.s_addr = INADDR_ANY;
  self.sin_port = htons(PORT);
  printf("Listening on %d\n", PORT);

  if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
    perror("bind"); // probably means port is in use
    exit(1);
  }

  if (listen(listenfd, 5) == -1) {
    perror("listen");
    exit(1);
  }
  return listenfd;
}

void run_read(int fd)
{
  char buf[128];
  char msg[128];
  char *eol;

  memset(buf, 0, sizeof(buf));

  while (read(fd, buf + strlen(buf), sizeof(buf) - 1 - strlen(buf)) > 0) {
    while ((eol = strstr(buf, "\r\n")) != NULL) {
      int index = eol - buf;
      memcpy(msg, buf, index);
      msg[index] = '\0';

      printf("Next message: %s\n", msg);

      int rest = sizeof(buf) - index + 2; // +2 for the size of "\r\n"
      memcpy(buf, eol + 2, rest);
      memset(buf + rest, 0, sizeof(buf) - rest);
    }
  }
}

int main(void) {
  int listenfd;
  int fd;
  struct sockaddr_in peer;
  socklen_t socklen;

  listenfd = setup();
  while (1) {
    socklen = sizeof(peer);
    // Note that we're passing in valid pointers for the second and third
    // arguments to accept here, so we can actually store and use client
    // information.
    if ((fd = accept(listenfd, (struct sockaddr *)&peer, &socklen)) < 0) {
      perror("accept");
    } else {
      printf("New connection on port %d\n", ntohs(peer.sin_port));
      run_read(fd);
      // Receive messages
      close(fd);
    }
  }
}

