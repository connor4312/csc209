#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "friends.h"

extern char *print_list;

/**
 * Somewhat hacky function to redirect the prints to a buffer during
 * testing so that we can assert against it easily.
 */
void output_line(const char *str) {
    #ifdef TESTING
        if (!print_list) {
            print_list = "";
        }

        char *result = malloc(strlen(str) + strlen(print_list) + 2);
        strcpy(result, print_list);
        strcat(result, str);
        strcat(result, "\n");
        print_list = result;
    #else
        printf("%s\n", str);
    #endif
}

/*
 * Create a new user with the given name.  Insert it at the tail of the list
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 if successful
 *   - 1 if a user by this name already exists in this list
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator)
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) + 1 > MAX_NAME) {
        return 2;
    }
    if (find_user(name, *user_ptr_add) != NULL) {
        return 1;
    }

    User *parent = *user_ptr_add;
    User *user = malloc(sizeof (User));
    if (user == NULL) {
        exit(1);
    }

    strcpy(user->name, name);
    user->next = NULL;
    user->first_post = NULL;
    user->profile_pic[0] =  '\0';
    memset(user->friends, 0, MAX_FRIENDS * sizeof(User*));

    if (parent == NULL) {
        *user_ptr_add = user;
        return 0;
    }

    while (parent->next != NULL) {
        parent = parent->next;
    }

    parent->next = user;
    return 0;
}


/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head) {
    for (User *ptr = (User *)head; ptr != NULL; ptr = ptr->next) {
        if (strcmp(ptr->name, name) == 0) {
            return ptr;
        }
    }

    return NULL;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
void list_users(const User *curr) {
    output_line("User List");
    char buff[MAX_NAME + 2];
    for (User *ptr = (User *)curr; ptr != NULL; ptr = ptr->next) {
        sprintf(buff, "\t%s", ptr->name);
        output_line(buff);
    }
}


/*
 * Change the filename for the profile pic of the given user.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the file does not exist.
 *   - 2 if the filename is too long.
 */
int update_pic(User *user, const char *filename) {
    if (strlen(filename) + 1 > MAX_NAME) {
        return 2;
    }

    struct stat foo;
    if (stat(filename, &foo) != 0) {
        return 1;
    }

    strcpy(user->profile_pic, filename);
    return 0;
}

/**
 * Adds the friend to the user's friends list.
 */
void add_friend(User *user, User *friend) {
    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (user->friends[i] == NULL) {
            user->friends[i] = friend;
            return;
        }
    }
}

/**
 * Returns 1 if the provided user's friends list is full.
 */
int maxed_friends(const User *user) {
    return user->friends[MAX_FRIENDS - 1] != NULL;
}

/**
 * Returns 1 if the provided users are friends, 0 otherwise
 */
int are_friends(const User *a, const User *b) {
    for (int i = 0; i < MAX_FRIENDS && a->friends[i] && b->friends[i]; i++) {
        if (a->friends[i] == b || b->friends[i] == a) {
            return 1;
        }
    }

    return 0;
}

/**
 * Purges the user from the friends list of all users in the targets list.
 */
void remove_friend(User *target, const User *user) {
    if (!target) {
        return;
    }

    int offset = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i]; i++) {
        if (target->friends[i] == user) {
            offset++;
        }

        if (i + offset < MAX_FRIENDS) {
            target->friends[i] = target->friends[i + offset];
        } else {
            target->friends[i] = NULL;
        }
    }

    remove_friend(target->next, user);
}

/*
 * Make two users friends with each other.  This is symmetric - a pointer to
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * Do not modify either user if the result is a failure.
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *a = find_user(name1, head);
    User *b = find_user(name2, head);
    if (!a || !b) { // user not found
        return 4;
    }

    if (a == b) { // same user passed twice
        return 3;
    }

    if (are_friends(a, b)) {
        return 1;
    }

    if (maxed_friends(a) || maxed_friends(b)) {
        return 2;
    }

    add_friend(a, b);
    add_friend(b, a);
    return 0;
}

/**
 * Prints the profile picture for the provided user.
 */
void print_profile(const User *user) {
    if (strlen(user->profile_pic) == 0) {
        return;
    }

    FILE *fd = fopen(user->profile_pic, "r");

    char buff[1023];
    int end;

    while (fgets(buff, 1023, fd) != NULL) {
        end = strlen(buff) - 1;
        if (buff[end] == '\n') {
            buff[end] = '\0';
        }

        output_line(buff);
    }
    output_line("");

    fclose(fd);
}

/**
 * Prints a dividing line for print_user output.
 */
void print_divider() {
    output_line("------------------------------------------");
}

/**
 * Prints the list of user posts, separated by dividers.
 */
void print_posts(Post *p, int depth) {
    if (!p) {
        return;
    }
    print_posts(p->next, depth + 1);

    char buff[255];
    sprintf(buff, "From: %s", p->author);
    output_line(buff);
    sprintf(buff, "Date: %s", ctime(p->date));
    output_line(buff);
    sprintf(buff, "\n%s", p->contents);
    output_line(buff);

    if (depth > 0) {
        output_line("\n===\n");
    }
}

/*
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
int print_user(const User *user) {
    if (user == NULL) {
        return 1;
    }

    char buff[1025];

    print_profile(user);
    sprintf(buff, "Name: %s\n", user->name);
    output_line(buff);
    print_divider();
    output_line("Friends:");
    for (int i = 0; i < MAX_FRIENDS && user->friends[i]; i++) {
        output_line(user->friends[i]->name);
    }
    print_divider();
    output_line("Posts:");
    print_posts(user->first_post, 0);
    print_divider();

    return 0;
}

/**
 * Creates and allocates a Post written by the user with the provided contents.
 */
Post* build_post(const User *author, char *contents)
{
    Post *post = malloc(sizeof (Post));
    if (post == NULL) {
        exit(1);
    }

    post->contents = contents;
    post->next = NULL;
    strcpy(post->author, author->name);

    post->date = malloc(sizeof (time_t));
    if (post->date == NULL) {
        exit(1);
    }

    time(post->date);

    return post;
}

/**
 * Appends the post to the user's list of posts.
 */
void append_post(User *target, Post *post)
{
    Post *parent = target->first_post;
    if (!parent) {
        target->first_post = post;
        return;
    }

    while (parent->next) {
        parent = parent->next;
    }
    parent->next = post;
}

/**
 * Deletes and frees all of the posts in the chain.
 */
void delete_posts(Post *post)
{
    if (!post) {
        return;
    }

    delete_posts(post->next);
    free(post->contents);
    free(post);
}

/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (!author || !target) {
        return 2;
    }

    if (!are_friends(author, target)) {
        return 1;
    }

    append_post(target, build_post(author, contents));
    return 0;
}

/**
 * Recursive implementation of delete_user. Much much nicer than the
 * iterative one >.<
 */
int run_delete_user(const char *name, User **head, User **user_ptr_del) {
    User *user = *user_ptr_del;
    if (!user) {
        return 1;
    }

    if (strcmp(user->name, name) == 0) {
        *user_ptr_del = user->next;
        remove_friend(*head, user);
        delete_posts(user->first_post);
        free(user);
        return 0;
    }

    return run_delete_user(name, head, &(user->next));
}

/*
 * From the list pointed to by *user_ptr_del, delete the user
 * with the given name.
 * Remove the deleted user from any lists of friends.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user with this name does not exist.
 */
int delete_user(const char *name, User **user_ptr_del) {
    return run_delete_user(name, user_ptr_del, user_ptr_del);
}
