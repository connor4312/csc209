#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "friends.h"
#include "CUnit/Basic.h"

void free_user_list(User *user_list)
{
    if (user_list == NULL) {
        return;
    }

    free_user_list(user_list->next);
    free(user_list);
}

void createUsernameTooLong(void)
{
    CU_ASSERT_EQUAL(create_user("thisstringisfartoolong!!!!!!!!!!!!!!!!!", NULL), 2);
}

void buildList(User **user_list)
{
    CU_ASSERT_EQUAL(create_user("joe", user_list), 0);
    CU_ASSERT_STRING_EQUAL((*user_list)->name, "joe");
    CU_ASSERT_EQUAL((*user_list)->next, NULL);

    CU_ASSERT_EQUAL(create_user("bob", user_list), 0);
    CU_ASSERT_STRING_EQUAL((*user_list)->name, "joe");
    CU_ASSERT_STRING_EQUAL((*user_list)->next->name, "bob");
    CU_ASSERT_EQUAL((*user_list)->next->next, NULL);

    CU_ASSERT_EQUAL(create_user("connor", user_list), 0);
    CU_ASSERT_STRING_EQUAL((*user_list)->name, "joe");
    CU_ASSERT_STRING_EQUAL((*user_list)->next->name, "bob");
    CU_ASSERT_STRING_EQUAL((*user_list)->next->next->name, "connor");
    CU_ASSERT_EQUAL((*user_list)->next->next->next, NULL);
}

void addUsersSuccessfully(void)
{
    User *user_list = NULL;
    buildList(&user_list);
}

void findsUser(void)
{
    User *user_list = NULL;
    buildList(&user_list);

    CU_ASSERT_STRING_EQUAL(find_user("joe", user_list)->name, "joe");
    CU_ASSERT_STRING_EQUAL(find_user("connor", user_list)->name, "connor");
    CU_ASSERT_STRING_EQUAL(find_user("bob", user_list)->name, "bob");
    CU_ASSERT_EQUAL(find_user("foo", user_list), NULL);
    CU_ASSERT_EQUAL(find_user("foo", NULL), NULL);
}

void listsUsers(void)
{
    User *user_list = NULL;
    buildList(&user_list);

    char *list;
    list_users(user_list, &list);
    CU_ASSERT_STRING_EQUAL(list, "joe\nbob\nconnor\n");
    free(list);

    list_users(NULL, &list);
    CU_ASSERT_STRING_EQUAL(list, "");
    free(list);
}

void makesFriends(void)
{
    User *user_list = NULL;
    buildList(&user_list);

    CU_ASSERT_EQUAL(make_friends("foo", "bar", user_list), 4);
    CU_ASSERT_EQUAL(make_friends("foo", "connor", user_list), 4);
    CU_ASSERT_EQUAL(make_friends("joe", "foo", user_list), 4);

    CU_ASSERT_EQUAL(make_friends("connor", "connor", user_list), 3);
    CU_ASSERT_EQUAL(make_friends("foo", "foo", user_list), 4); // return largest error

    CU_ASSERT_EQUAL(make_friends("connor", "joe", user_list), 0);
    CU_ASSERT_STRING_EQUAL(user_list->friends[0]->name, "connor");
    CU_ASSERT_STRING_EQUAL(user_list->next->next->friends[0]->name, "joe");

    CU_ASSERT_EQUAL(make_friends("connor", "joe", user_list), 1);
    CU_ASSERT_EQUAL(make_friends("joe", "connor", user_list), 1);
}

void printsProfile(void)
{
    User *user = NULL;
    buildList(&user);

    char *text;
    CU_ASSERT_EQUAL(print_user(NULL, &text), 1);
    CU_ASSERT_EQUAL(print_user(user, &text), 0);
    CU_ASSERT_STRING_EQUAL(text,
        "Name: joe\n"
        "\n"
        "------------------------------------------\n"
        "Friends:\n"
        "------------------------------------------\n"
        "Posts:\n"
        "------------------------------------------\n"
    );
    free(text);

    make_friends("joe", "connor", user);

    CU_ASSERT_EQUAL(print_user(user, &text), 0);
    CU_ASSERT_STRING_EQUAL(text,
        "Name: joe\n"
        "\n"
        "------------------------------------------\n"
        "Friends:\n"
        "connor\n"
        "------------------------------------------\n"
        "Posts:\n"
        "------------------------------------------\n"
    );
    free(text);
}

void createsPost(void)
{
    User *joe = NULL;
    buildList(&joe);

    User *bob = joe->next;

    CU_ASSERT_EQUAL(make_post(NULL, NULL, "hello world!"), 2);
    CU_ASSERT_EQUAL(make_post(bob, NULL, "hello world!"), 2);
    CU_ASSERT_EQUAL(make_post(NULL, bob, "hello world!"), 2);
    CU_ASSERT_EQUAL(make_post(joe, bob, "hello world!"), 1);

    CU_ASSERT_EQUAL(make_friends("bob", "joe", joe), 0);

    CU_ASSERT_EQUAL(make_post(joe, bob, "hello world!"), 0);
    CU_ASSERT_STRING_EQUAL(bob->first_post->contents, "hello world!");
    CU_ASSERT_STRING_EQUAL(bob->first_post->author, "joe");
    CU_ASSERT_EQUAL(bob->first_post->next, NULL);

    CU_ASSERT_EQUAL(make_post(joe, bob, "yay!"), 0);
    CU_ASSERT_STRING_EQUAL(bob->first_post->next->contents, "hello world!");
    CU_ASSERT_STRING_EQUAL(bob->first_post->contents, "yay!");
}

void string_buffer_writes(void)
{
    struct string_buffer *sb = create_buffer();

    char expected[4096] = { 'a', '\0' };
    char buf[4096];
    buffer_write(sb, "a");

    for (int i = 0; i < 11; i++) {
        buffer_write(sb, expected);

        memcpy(buf, expected, sizeof(expected));
        strcat(expected, buf);
        CU_ASSERT_STRING_EQUAL(sb->str, expected);
    }

    buffer_free(sb);
}

void string_buffer_pull(void)
{
    struct string_buffer *sb = create_buffer();

    buffer_write(sb, "hello world, it's a sunny day");

    char *str;

    CU_ASSERT_EQUAL(buffer_pull_before(sb, &str, " "), 0);
    CU_ASSERT_STRING_EQUAL(str, "hello");
    CU_ASSERT_STRING_EQUAL(sb->str, "world, it's a sunny day");
    free(str);

    CU_ASSERT_EQUAL(buffer_pull_before(sb, &str, "it's"), 0);
    CU_ASSERT_STRING_EQUAL(str, "world, ");
    CU_ASSERT_STRING_EQUAL(sb->str, " a sunny day");
    free(str);

    CU_ASSERT_EQUAL(buffer_pull_before(sb, &str, "it's"), 1);

    buffer_free(sb);
}

int main()
{
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    pSuite = CU_add_suite("Suite_1", NULL, NULL);
    if (NULL == pSuite) {
        return CU_get_error();
    }

    CU_add_test(pSuite, "friendme: test create too long username", createUsernameTooLong);
    CU_add_test(pSuite, "friendme: test adds users successfully", addUsersSuccessfully);
    CU_add_test(pSuite, "friendme: test finds users", findsUser);
    CU_add_test(pSuite, "friendme: test lists users", listsUsers);
    CU_add_test(pSuite, "friendme: test makes friends", makesFriends);
    CU_add_test(pSuite, "friendme: test prints profile", printsProfile);
    CU_add_test(pSuite, "friendme: test creates post", createsPost);

    CU_add_test(pSuite, "buffer: test string writes", string_buffer_writes);
    CU_add_test(pSuite, "buffer: test string pulls", string_buffer_pull);


    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
