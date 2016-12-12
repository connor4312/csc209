#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "friends.h"

#define INPUT_ARG_MAX_NUM 12
#define DELIM "\n"
#define GREETING "What is your user name?\n"
#define NOW_FRIENDS_PREFIX "You are now friends with "

/**
 * A linked list of connected clients to this server. From the example code.
 */
struct client {
    /* File descriptor of the connection */
    int fd;
    /* Buffer of client input; every time this is updated, we'll try to read
       off commands. Leftovers will remain in the buf for the next read. */
    struct string_buffer *buf;
    /* The user this client identified as. Note that this will initially be
       NULL until they tell us who they are. */
    User *user;
    /* Next client in the linked list. */
    struct client *next;
};

/**
 * Writes a string to the client connection, followed by a new line character
 * if there wasn't one in the string.
 */
void send_str(struct client *c, char *str)
{
    int len = strlen(str);
    write(c->fd, str, len * sizeof(char));

    if (str[len - 1] != '\n') {
        write(c->fd, "\n", 1);
    }
}

/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, " ");
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, " ");
    }

    return cmd_argc;
}

/*
 * Takes a line command from the client and runs it, writing back to the
 * client itself if necessary. This is mostly based on the starter code/A2.
 *
 * Return:  -1 for quit command
 *          0 otherwise
 */
int run_command(struct client *c, char *line, User *user_list) {
    char *cmd_argv[INPUT_ARG_MAX_NUM];
    int cmd_argc = tokenize(line, cmd_argv);
    char *text;

    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        list_users(user_list, &text);
        send_str(c, text);
        free(text);
    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {
        switch (make_friends(c->user->name, cmd_argv[1], user_list)) {
            case 0:
                write(c->fd, NOW_FRIENDS_PREFIX, strlen(NOW_FRIENDS_PREFIX) * sizeof(char));
                write(c->fd, cmd_argv[1], strlen(cmd_argv[1]));
                send_str(c, ".\n");
                break;
            case 1:
                send_str(c, "You are already friends.");
                break;
            case 2:
                send_str(c, "at least one user you entered has the max number of friends");
                break;
            case 3:
                send_str(c, "You can't friend yourself");
                break;
            case 4:
                send_str(c, "The user you entered does not exist");
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 2; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = malloc(space_needed);
        if (contents == NULL) {
            perror("malloc");
            exit(1);
        }

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[2]);
        for (int i = 4; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *target = find_user(cmd_argv[1], user_list);
        switch (make_post(c->user, target, contents)) {
            case 1:
                send_str(c, "You can only post to your friends");
                break;
            case 2:
                send_str(c, "The user you want to post to does not exist");
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        if (print_user(user, &text) == 1) {
            send_str(c, "User not found");
        } else {
            send_str(c, text);
            free(text);
        }
    } else {
        send_str(c, "Incorrect syntax");
    }
    return 0;
}



/**
 * Binds and listens on a port, returning the listener fd. Based on
 * the example code provided.
 */
int bind_and_listen()
{
    struct sockaddr_in r;
    int listen_fd;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&r, 0, sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }

    if (listen(listen_fd, 5)) {
        perror("listen");
        exit(1);
    }

    return listen_fd;
}

/**
 * Removes the client from the linked list headed by `top`. Returns the
 * new head.
 */
struct client *remove_client(struct client *top, struct client *toRemove)
{
    // If we aren't the ones being removed, recurse down to the next client.
    if (toRemove != top) {
        top->next = remove_client(top->next, toRemove);
        return top;
    }

    // Otherwise free all our resources.
    struct client *next = top->next;
    buffer_free(top->buf);
    close(top->fd);
    free(top);

    return next;
}

/**
 * Attempts to find a user by the given name in the list of users. If a user
 * is not found, it's created and appended.
 */
User *find_or_create_user(struct client *c, char *name, User **users)
{
    // To copy the behaviour of the example server:
    if (strlen(name) > MAX_NAME) {
        name[MAX_NAME] = '\0';
    }

    if (create_user(name, users) == 0) {
        send_str(c, "Welcome.");
    } else {
        send_str(c, "Welcome back.");
    }

    send_str(c, "Go ahead and enter user commands>");

    return find_user(name, *users);
}

/**
 * Reads and processes input from the client. Operates on the list of users
 * provided. Returns 1 if it was closed, 0 otherwise.
 */
int process_input(struct client *c, User **users)
{
    int len;
    char buf[128];
    while ((len = read(c->fd, buf, sizeof(buf) - 1)) != -1) {
        buf[len] = '\0';
        buffer_write(c->buf, buf);
    }

    char *line;
    int exits = 0;
    while (buffer_pull_before(c->buf, &line, DELIM) == 0) {
        // A wee bit of special casing: the first line they send will be
        // their name. Handle that separately.
        if (c->user == NULL) {
            c->user = find_or_create_user(c, line, users);
        } else if (run_command(c, line, *users) == -1) {
            exits = 1;
            break;
        }

        free(line);
    }

    // If we returned from read for a reason other than that we were out of
    // data, consider the socket to be closed.
    if (exits || errno != EAGAIN) {
        return 1;
    }

    return 0;
}

/**
 * Accepts a new connection from the listener FD and adds it to the clients
 * linked list.
 */
void add_client(int listen_fd, struct client **top)
{
    int fd;
    if ((fd = accept(listen_fd, NULL, 0)) < 0) {
        perror("accept");
        return;
    }

    struct client *cnx = malloc(sizeof(struct client));
    if (cnx == NULL) {
        perror("malloc");
        exit(1);
    }

    // Set the socket to non-blocking mode. When we process inputs, we will
    // greedily read from the socket until it gives us EAGAIN. This lets
    // us read everything we can in one go, without depending on the
    // buffer size. A bit more efficient.
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    write(fd, GREETING, strlen(GREETING) * sizeof(char));

    cnx->fd = fd;
    cnx->buf = create_buffer();
    cnx->user = NULL;
    cnx->next = *top;

    *top = cnx;
}

int main(int argc, char const *argv[])
{
    int listen_fd = bind_and_listen();
    User *users = NULL;
    struct client *top = NULL;
    struct client *p;

    /**
     * The following is partly based on the example code with annotations
     * and minor style tweaks.
     *
     * Listen for connections in an infinite loop. On each iteration, we
     * add all connections we know about to an FD set then select from
     * that set. Then, we add connections, do writing, and loop again.
     */
    while (1) {
        fd_set fdlist;
        int maxfd = listen_fd;
        FD_ZERO(&fdlist);

        // Add all fds we know about to the set:
        FD_SET(listen_fd, &fdlist);
        for (p = top; p; p = p->next) {
            FD_SET(p->fd, &fdlist);
            if (p->fd > maxfd) {
                maxfd = p->fd;
            }
        }

        // Wait for change on one of them:
        if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
            perror("select");
            continue;
        }

        // Look for the FD that changed, and process them.
        for (p = top; p; p = p->next) {
            if (!FD_ISSET(p->fd, &fdlist)) {
                continue;
            }

            if (process_input(p, &users)) {
                top = remove_client(top, p);
                break; // can't keep looping, we'll pick up any closes next time
            }
        }

        // New connection incoming!
        if (FD_ISSET(listen_fd, &fdlist)) {
            add_client(listen_fd, &top);
        }
    }
    return 0;
}
