#include <stdio.h>
#include <string.h>
#include "friends.h"
#include "CUnit/Basic.h"

extern char *print_list;

void free_print_list()
{
    free(print_list);
    print_list = NULL;
}

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

void disallowsDuplicateUsernames(void)
{
    User *user_list = NULL;
    buildList(&user_list);

    CU_ASSERT_EQUAL(create_user("joe", &user_list), 1);
    CU_ASSERT_EQUAL(create_user("bob", &user_list), 1);
    CU_ASSERT_EQUAL(create_user("connor", &user_list), 1);
    CU_ASSERT_EQUAL(create_user("asdfds", &user_list), 0);
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

    list_users(user_list);
    CU_ASSERT_STRING_EQUAL(print_list, "User List\n\tjoe\n\tbob\n\tconnor\n");
    free_print_list();

    list_users(NULL);
    CU_ASSERT_STRING_EQUAL(print_list, "User List\n");
    free_print_list();
}

void updatesPic(void)
{
    User *user_list = NULL;
    buildList(&user_list);

    CU_ASSERT_EQUAL(update_pic(user_list, "thisstringisfartoolong!!!!!!!!!!!!!!!!!"), 2);
    CU_ASSERT_EQUAL(update_pic(user_list, "notAFile"), 1);
    CU_ASSERT_EQUAL(update_pic(user_list, "alien.ascii"), 0);
    CU_ASSERT_STRING_EQUAL(user_list->profile_pic, "alien.ascii");
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

    CU_ASSERT_EQUAL(print_user(NULL), 1);
    CU_ASSERT_EQUAL(print_user(user), 0);
    CU_ASSERT_STRING_EQUAL(print_list,
        "Name: joe\n"
        "\n"
        "------------------------------------------\n"
        "Friends:\n"
        "------------------------------------------\n"
        "Posts:\n"
        "------------------------------------------\n"
    );
    free_print_list();

    make_friends("joe", "connor", user);

    CU_ASSERT_EQUAL(print_user(user), 0);
    CU_ASSERT_STRING_EQUAL(print_list,
        "Name: joe\n"
        "\n"
        "------------------------------------------\n"
        "Friends:\n"
        "connor\n"
        "------------------------------------------\n"
        "Posts:\n"
        "------------------------------------------\n"
    );
    free_print_list();

    CU_ASSERT_EQUAL(update_pic(user, "alien.ascii"), 0);

    CU_ASSERT_EQUAL(print_user(user), 0);
    CU_ASSERT_STRING_EQUAL(print_list,
        "    \n"
        "      .--.   |V|\n"
        "     /    \\ _| /\n"
        "     q .. p \\ /\n"
        "      \\--/  //\n"
        "jgs  __||__//\n"
        "    /.    _/\n"
        "   // \\  /\n"
        "  //   ||\n"
        "  \\\\  /  \\\n"
        "   )\\|    |\n"
        "  / || || |\n"
        "  |/\\| || |\n"
        "     | || |\n"
        "     \\ || /\n"
        "   __/ || \\__\n"
        "  \\____/\\____/\n"
        "\n"
        "Name: joe\n"
        "\n"
        "------------------------------------------\n"
        "Friends:\n"
        "connor\n"
        "------------------------------------------\n"
        "Posts:\n"
        "------------------------------------------\n"
    );
    free_print_list();
}

void deletesUser(void)
{
    User *user_list = NULL;

    // From head
    buildList(&user_list);
    CU_ASSERT_EQUAL(delete_user("joe", &user_list), 0);
    CU_ASSERT_EQUAL(find_user("joe", user_list), NULL);
    CU_ASSERT_NOT_EQUAL(find_user("bob", user_list), NULL);
    CU_ASSERT_NOT_EQUAL(find_user("connor", user_list), NULL);
    free_user_list(user_list);
    user_list = NULL;

    // From middle
    buildList(&user_list);
    CU_ASSERT_EQUAL(delete_user("bob", &user_list), 0);
    CU_ASSERT_NOT_EQUAL(find_user("joe", user_list), NULL);
    CU_ASSERT_EQUAL(find_user("bob", user_list), NULL);
    CU_ASSERT_NOT_EQUAL(find_user("connor", user_list), NULL);
    free_user_list(user_list);
    user_list = NULL;

    // From end
    buildList(&user_list);
    CU_ASSERT_EQUAL(delete_user("connor", &user_list), 0);
    CU_ASSERT_NOT_EQUAL(find_user("joe", user_list), NULL);
    CU_ASSERT_NOT_EQUAL(find_user("bob", user_list), NULL);
    CU_ASSERT_EQUAL(find_user("connor", user_list), NULL);
    free_user_list(user_list);
    user_list = NULL;
}

void deleteRemovesFriends(void)
{
    User *user_list = NULL;

    // Single friend
    buildList(&user_list);
    CU_ASSERT_EQUAL(make_friends("connor", "joe", user_list), 0);
    CU_ASSERT_STRING_EQUAL(user_list->friends[0]->name, "connor");
    CU_ASSERT_EQUAL(delete_user("connor", &user_list), 0);
    CU_ASSERT_EQUAL(user_list->friends[0], NULL);
    free_user_list(user_list);
    user_list = NULL;

    // Multiple friends
    buildList(&user_list);
    CU_ASSERT_EQUAL(make_friends("connor", "joe", user_list), 0);
    CU_ASSERT_EQUAL(make_friends("bob", "joe", user_list), 0);
    CU_ASSERT_STRING_EQUAL(user_list->friends[0]->name, "connor");
    CU_ASSERT_EQUAL(delete_user("connor", &user_list), 0);
    CU_ASSERT_STRING_EQUAL(user_list->friends[0]->name, "bob");
    free_user_list(user_list);
    user_list = NULL;
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
    CU_ASSERT_STRING_EQUAL(bob->first_post->contents, "hello world!");
    CU_ASSERT_STRING_EQUAL(bob->first_post->next->contents, "yay!");
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

    CU_add_test(pSuite, "test create too long username", createUsernameTooLong);
    CU_add_test(pSuite, "test adds users successfully", addUsersSuccessfully);
    CU_add_test(pSuite, "test disallows duplicate names", disallowsDuplicateUsernames);
    CU_add_test(pSuite, "test finds users", findsUser);
    CU_add_test(pSuite, "test lists users", listsUsers);
    CU_add_test(pSuite, "test updates profile picture", updatesPic);
    CU_add_test(pSuite, "test makes friends", makesFriends);
    CU_add_test(pSuite, "test prints profile", printsProfile);
    CU_add_test(pSuite, "test deletes user", deletesUser);
    CU_add_test(pSuite, "test deletes removes friends", deleteRemovesFriends);
    CU_add_test(pSuite, "test creates post", createsPost);
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
