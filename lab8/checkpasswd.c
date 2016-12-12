#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Read a user id and password from standard input,
   - create a new process to run the validate program
   - send 'validate' the user id and password on a pipe,
   - print a message
        "Password verified" if the user id and password matched,
        "Invalid password", or
        "No such user"
     depending on the return value of 'validate'.
*/


int main(void) {
    char userid[10];
    char password[10];

    /* Read a user id and password from stdin */
    printf("User id:\n");
    scanf("%s", userid);
    printf("Password:\n");
    scanf("%s", password);

    int fd[2];
    pipe(fd);

    char *args[] = {NULL};
    if (fork() == 0) {
        dup2(fd[0], 0);
        execv("./validate", args);
        perror("execv");
        return 255;
    }

    write(fd[1], userid, 10);
    write(fd[1], password, 10);

    int status;
    while (wait(&status) > -1);

    if (!WIFEXITED(status)) {
        printf("Some unknown error occurred\n");
        exit(1);
    }

    switch (WEXITSTATUS(status)) {
    case 0: printf("Password verified\n"); break;
    case 1: printf("Could not open passwords file\n"); break;
    case 2: printf("Invalid password\n"); break;
    case 3: printf("No such user\n"); break;
    }

    exit(status);
}
