#include <stdio.h>
#include <comlin.h>


/**
 * Hierarchically show results for options.
 */
void display_options( cl_cmd_t cmd )
{
    cl_cmd_t subcmd;
    cl_opt_p opts;
    cl_opt_t o;

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
                { CL_SUBCMD, "add", NULL, "Add file to repo." },
                { CL_SUBCMD, "rm", NULL, "Remove file from repo." }, );

    cl_subcmd( "add",
               "comlin_subcmd",
               { CL_SWITCH, "force", "-fo", "Force operation." },
               { CL_OPT_SINGLE, "password", "-p", "User password." },
               { CL_OPT_SINGLE, "username", "-u", "Username." },
               { CL_SINGLE, "file", "-f", "File." } );

    cl_subcmd( "rm",
               "comlin_subcmd",
               { CL_SWITCH, "force", "-fo", "Force operation." },
               { CL_OPT_SINGLE, "file", "-f", "File." } );

    cl_finish();

    display_options( cl_cmd );

    if ( cl_cmd->external ) {
        char** value;
        bool_t first = mc_true;

        printf( "External: [" );

        value = cl_cmd->external;

        while ( *value ) {
            if ( !first )
                printf( ", " );
            printf( "\"%s\"", *value );
            first = mc_false;
            value++;
        }

        printf( "]\n" );
    }

    return 0;
}
