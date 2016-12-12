#include <stdio.h>
#include <string.h>
#include "mapreduce.h"
#include "CUnit/Basic.h"

void test_hello_world(void)
{
    CU_ASSERT_TRUE(1);
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

    CU_add_test(pSuite, "test says hello", test_hello_world);
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
