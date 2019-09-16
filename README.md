# Comlin

Comlin library provides low manifest command line option parsing and
deployment. The command line options are described in compact table
format and option values are stored to conveniently named
properties. Comlin builds command usage information based on the option
table (+ generic program info) and displays it automatically if
necessary. Comlin supports also subcommands.


A simple example program using Comlin:

```
    #include <stdio.h>
    #include <comlin.h>

    int main( int argc, char** argv )
    {
      cl_opt_t* opt;

      /* Specify program and all the options. Parsing and help is managed
         automatically. */
      cl_command( "comlin_simple", "Programmer", "2018",
                    { CL_SINGLE, "file",  "-f", "File argument." },
                    { CL_SWITCH, "debug", "-d", "Enable debugging." }
                    );

      /* Get handle of option "file". */
      opt = cl_opt( "file" );

      /* Print "file" option's properties. */
      printf( "Option \"%s\" is given: %d\n", opt->name, opt->given );

      /* Print "file" option's value if option was given. */
      if ( opt->given )
        {
          char** value;

          printf( "Option \"%s\" values:\n", opt->name );

          value = opt->value;
          while ( *value )
            {
              printf( "  %s\n", *value );
              value++;
            }
        }

      /* Print "debug" option's "given" property. */
      printf( "Option \"%s\" is given: %d\n", "debug", cl_given( "debug" ) );

      return 0;
    }

```

You have automatically:
  - `-h` option defined with usage printout.
  - Check that `-f` has one and only one argument.
  - Tests whether a particular option was given or not.
  - Long options based on option name.
  - Option values collected to an array.



# Documentation

Manual page for Comlin is included in the installation (see:
`man/`). It can be generated with:

    shell> rake

The source code is documented in Doxygen format. Documentation can be
generated with:

    shell> doxygen .doxygen

`comlin.h` related file will include the usage information (equal to man
page).


# Examples

There are two simple examples in the `examples/` directory:
`comlin_simple.c` and `comlin_subcmd.c`.

For a complete set of features, see the test programs in `test/`
directory (`*.c`).


## Building

Ceedling based flow is in use:

    shell> ceedling

Testing:

    shell> ceedling test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Segman uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new segman

in the directory above Segman. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.


## License

See: COPYING



Comlin library by Tero Isannainen, (c) Copyright 2018.
