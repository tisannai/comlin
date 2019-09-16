/**
 * @file comlin_options.c
 *
 * Test option types.
 */

//#include "../src/mc.h"
#include "../src/comlin.h"

int main( int argc, char** argv )
{
    cl_opt_p opts;
    cl_opt_t o;

    cl_command( "comlin_options",
                "Comlin Tester",
                "2018",
                { CL_EXCLUSIVE,
                  "doc",
                  NULL,
                  "Documentation for option\n\twith too much description\n\tfor one line." },
                { CL_SINGLE, "file", "-f", "File argument." },
                { CL_SWITCH, "debug", NULL, "Enable debugging." },
                { CL_OPT_SINGLE, "mode", "-m", "Mode." },
                { CL_OPT_MULTI, "params", NULL, "Parameters." },
                { CL_OPT_ANY, "types", "-t", "Types." },
                { CL_SILENT, "terminator", "-", "The terminator." },
                { CL_MULTI, "dir", "-d", "Directory argument(s)." },
                { CL_DEFAULT, NULL, NULL, "Leftovers." }, );

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
