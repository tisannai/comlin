#include "unity.h"
#include "comlin.h"


int run_test( char* testname )
{
    char buf[ 256 ];
    sprintf( buf, "test/run_test -f %s", testname );
    return system( buf );
}


void test_options( void )
{
    TEST_ASSERT_TRUE( run_test( "options" ) == 0 );
}


void test_config( void )
{
    TEST_ASSERT_TRUE( run_test( "config" ) == 0 );
}


void test_subcmd( void )
{
    TEST_ASSERT_TRUE( run_test( "subcmd" ) == 0 );
}


void test_type_prim( void )
{
    TEST_ASSERT_TRUE( run_test( "type_prim" ) == 0 );
}
