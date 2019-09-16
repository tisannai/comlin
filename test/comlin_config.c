/**
 * @file comlin_config.c
 *
 * Test option types.
 */

//#include "../src/mc.h"
#include "../src/comlin.h"

int main( int argc, char** argv )
{
    cl_opt_p opts;
    cl_opt_t o;

    cl_maincmd( "comlin_config",
                  "Comlin Tester",
                  "2018",
                  { CL_EXCLUSIVE, "doc", NULL, "Documentation." },
                  { CL_SINGLE, "file", "-f", "File argument." },
                  { CL_SWITCH, "debug", NULL, "Enable debugging." },
                  { CL_OPT_SINGLE, "mode", "-m", "Mode." },
                  { CL_OPT_MULTI, "params", NULL, "Parameters." },
                  { CL_OPT_ANY, "types", "-t", "Types." },
                  { CL_SILENT, "terminator", "-", "The terminator." },
                  { CL_MULTI, "dir", "-d", "Directory argument(s)." },
                  { CL_DEFAULT, NULL, NULL, "Leftovers." }, );

    cl_conf_header( "\nAdditional heading info.\n\n" );
    cl_conf_footer( "\nAdditional footer info.\n\n" );
    cl_conf_check_missing( st_false );
    cl_conf_check_invalid( st_false );
    cl_conf_tab( 10 );
    cl_conf_help_exit( st_false );

    cl_finish();


    opts = cl_cmd->opts;
    while ( *opts ) {
        o = *opts;

        printf( "Given \"%s\": %s\n", o->name, o->given ? "true" : "false" );

        if ( o->given && o->value ) {
            printf( "Value \"%s\": ", o->name );
            cl_display_values( stdout, o );
            printf( "\n" );
        }

        opts++;
    }

    if ( cl_cmd->external ) {
        char**    value;
        st_bool_t first = st_true;

        printf( "External: [" );

        value = cl_cmd->external;

        while ( *value ) {
            if ( !first )
                printf( ", " );
            printf( "\"%s\"", *value );
            first = st_false;
            value++;
        }

        printf( "]\n" );
    }


    cl_end();

    return 0;
}
