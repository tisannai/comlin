#include <stdio.h>
#include <comlin.h>

int main( int argc, char** argv )
{
    cl_opt_t opt;

    /* Specify program and all the options. Parsing and help is managed
       automatically. */
    cl_command( "comlin_simple",
                "Programmer",
                "2018",
                { CL_SINGLE, "file", "-f", "File argument." },
                { CL_SWITCH, "debug", "-d", "Enable debugging." } );

    /* Get handle of option "file". */
    opt = cl_opt( "file" );

    /* Print "file" option's properties. */
    printf( "Option \"%s\" is given: %d\n", opt->name, opt->given );

    /* Print "file" option's value if option was given. */
    if ( opt->given ) {
        char** value;

        printf( "Option \"%s\" values:\n", opt->name );

        value = opt->value;
        while ( *value ) {
            printf( "  %s\n", *value );
            value++;
        }
    }

    /* Print "debug" option's "given" property. */
    printf( "Option \"%s\" is given: %d\n", "debug", cl_given( "debug" ) );

    return 0;
}
