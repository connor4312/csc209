#include <time.h>

#ifndef PORT
#define PORT 1337
#endif

#define MAX_NAME 32     // Max username and profile_pic filename lengths
#define MAX_FRIENDS 10  // Max number of friends a user can have

typedef struct user {
    char name[MAX_NAME];
    char profile_pic[MAX_NAME];  // This is a *filename*, not the file contents.
    struct post *first_post;
    struct user *friends[MAX_FRIENDS];
    struct user *next;
} User;

typedef struct post {
    char author[MAX_NAME];
    char *contents;
    time_t *date;
    struct post *next;
} Post;


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
int create_user(const char *name, User **user_ptr_add);

/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head);


/*
 * Creates a list of all users in the list starting at curr. Names
 * will be separated by "\n" characters. The caller is responsible
 * for freeing `str` after this function returns.
 */
void list_users(const User *curr, char **str);

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
int make_friends(const char *name1, const char *name2, User *head);

/*
 * Creates a user profile and stores it to str. The caller is responsible
 * for freeing the string.
 *
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
int print_user(const User *user, char **str);

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
int make_post(const User *author, User *target, char *contents);

/**
 * string_buffer is a data structure that you can write to with buffer_write
 * and have the allocation of the string automatically expand.
 */
struct string_buffer {
    char *str;
    int len;   // current contained string length
    int size;  // number of allocated bytes in str
};

/**
 * Creates and returns a new string buffer.
 */
struct string_buffer *create_buffer();

/**
 * Frees resources associated with a string buffer.
 */
void buffer_free(struct string_buffer *sb);

/**
 * Appends the string str to the buffer, dynamically allocating space as
 * needed.
 */
void buffer_write(struct string_buffer *sb, const char *str);

/**
 * Allocates and copies the contents of the buffer to the target string.
 */
void buffer_copy_to(struct string_buffer *sb, char **str);

/**
 * Attempts to read a string up to the "needle" substring within the buffer.
 * If found, content before the substring will be stored in the target and
 * removed from the buffer.
 *
 * Returns 0 if the substring was found, or 1 otherwise. If 0 is returned,
 * the caller is responsible for freeing the target after they're done with it.
 */
int buffer_pull_before(struct string_buffer *sb, char **target, const char *needle);

