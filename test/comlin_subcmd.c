/**
 * @file comlin_subcmd.c
 *
 * Test option types.
 */


#include "../src/comlin.h"


/**
 * Hierarchically show results for options.
 */
void display_options( cl_cmd_t cmd )
{
    cl_opt_p opts;
    cl_opt_t o;
    cl_cmd_t subcmd;

    printf( "Options for: %s\n", cmd->name );

    opts = cmd->opts;
    while ( *opts ) {
        o = *opts;

        printf( "  Given \"%s\": %s\n", o->name, o->given ? "true" : "false" );

        if ( o->given && o->value ) {
            printf( "  Value \"%s\": ", o->name );
            cl_display_values( stdout, o );
            printf( "\n" );
        }

        opts++;
    }

    subcmd = cl_cmd_given_subcmd( cmd );
    if ( subcmd )
        display_options( subcmd );
}


int main( int argc, char** argv )
{
    cl_maincmd( "comlin_subcmd",
                "Comlin Tester",
                "2018",
                { CL_OPT_SINGLE, "password", "-p", "User password." },
                { CL_OPT_MULTI, "username", "-u", "Username(s)." },
                { CL_SILENT, "terminator", "-", "Terminator." },
                { CL_SUBCMD, "add", NULL, "Add file to repo." },
                { CL_SUBCMD, "rm", NULL, "Remove file from repo." },
                { CL_SUBCMD, "commit", NULL, "Commit (pending) changes to repo." } );

    cl_subcmd( "add",
               "comlin_subcmd",
               { CL_SWITCH, "force", "-fo", "Force operation." },
               { CL_OPT_SINGLE, "username", "-u", "Username." },
               { CL_SINGLE, "file", "-f", "File." } );

    cl_subcmd( "rm",
               "comlin_subcmd",
               { CL_SWITCH, "force", "-fo", "Force operation." },
               { CL_OPT_SINGLE, "file", "-f", "File." } );

    cl_subcmd( "commit",
               "comlin_subcmd",
               { CL_SWITCH, "quiet", "-q", "Quiet operation." },
               { CL_OPT_SINGLE, "username", "-u", "Username." },
               { CL_DEFAULT, NULL, NULL, "File(s) to commit." } );

    cl_finish();


    display_options( cl_cmd );


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
