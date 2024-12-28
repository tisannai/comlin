#ifndef COMLIN_H
#define COMLIN_H

/**
 * @file   comlin.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Mon Oct 29 23:51:57 2018
 *
 * @brief Comlin library provides low manifest command line parsing.
 *
 * @mainpage
 *
 * # Comlin
 *
 * ## Introduction
 *
 * Comlin provides low manifest command line option parsing and
 * deployment. The command line options are described in compact table
 * format and option values are stored to conveniently named
 * properties. Comlin builds command usage information based on the
 * option table (+ generic program info) and displays it automatically
 * if necessary. Comlin supports also subcommands.
 *
 *
 * ## Usage Examples
 *
 * Two simple examples are presented in this section. First one
 * includes a straight forward command definition and the second is a
 * bit more complicated with subcommand feature in use.
 *
 * ### Simple example
 *
 * Below is a small example program ("comlin_simple") that demonstrates
 * typical usage.
 *
 * #### Program listing
 *
 * @include comlin_simple.c
 *
 * First Comlin and other essential headers are included.
 *
 * "cl_command" method takes 4 arguments:
 * - progname: Name of the program (or command).
 * - author: Author of the program.
 * - year: Year (or any date) for the program.
 * - option table: Description of the command options.
 *
 * Each option table entry (row/sub-array) includes 4 fields and
 * specifies one option:
 *   @code
 *   [ type, name, mnemonic, doc ]
 *   @endcode
 *
 * Two different types are present in the example:
 * - single: Single means that the option requires one argument (and
 *           only one).
 * - switch: Switch is an optional flag (default value is false).
 *
 * Option name is used to reference the option value that user has
 * given.  The command line option values are stored
 * automatically. For example the file option value is returned by
 * (array with NULL termination):
 *
 * @code
 *   opt->value
 * @endcode
 *
 * or without the option reference directly by option name:
 * @code
 *   cl_value( "file" )
 * @endcode
 *
 * A single value can be directly accessed with:
 * @code
 *   opt->value[0]
 * @endcode
 *
 *
 * The option name also doubles as long option format, i.e. one could
 * use <pre>--file \<filename\></pre> on the command line.
 *
 * Existence of optional options can be tested using the "given"
 * method. For example:
 *
 * @code
 *   cl_given( "debug" )
 * @endcode
 *
 * would return "non-null" if "-d" was given on the command line.
 *
 * Mnemonic is the short form option specification e.g. "-f". If short
 * form is replaced with "NULL", the long option format is only
 * available.
 *
 * Doc includes documentation for the option. It is displayed when
 * "help" ("-h") option is given. Help option is added to the command
 * automatically as default behavior.
 *
 * #### Simple example executions
 *
 * Normal behavior would be achieved by executing:
 *
 * @code
 *   shell> comlin_simple -f example -d
 * @endcode
 *
 * The program would execute with the following output:
 *
 @verbatim
 Option "file" is given: 1
 Option "file" values:
 example
 Option "debug" is given: 1
 @endverbatim
 *
 * Same output would be achieved with:
 *
 * @code
 *   shell> comlin_simple --file example --debug
 * @endcode
 *
 * Since option name doubles as long option.
 *
 * Comlin includes certain "extra" behavior out-of-box. Required
 * arguments are checked for existence and error is displayed if
 * arguments are not given.
 *
 * For example given the command:
 *
 * @code
 *   shell> comlin_simple
 * @endcode
 *
 * The following is displayed on the screen:
 *
 @verbatim
 comlin_simple error: Option "-f" missing for "comlin_simple"...

 Usage:
 comlin_simple -f <file> [-d]

 -f          File argument.
 -d          Enable debugging.


 Copyright (c) 2018 by Programmer
 @endverbatim
 *
 * Missing option error is displayed since "file" is a mandatory
 * option. The error message is followed by "usage" display (Usage
 * Help). Documentation string is taken from the option specification to
 * "usage" display.
 *
 * Given the command:
 * @code
 *   shell> comlin_simple -h
 * @endcode
 *
 * would display the same "usage" screen except without the error
 * line.
 *
 *
 * ### Subcommand example
 *
 * Subcmd example includes a program which has subcommands. Subcommands
 * can have their own command line switches and options.
 *
 * #### Program listing
 *
 * @include comlin_subcmd.c
 *
 * "cl_maincmd" method defines a program (command) with possible
 * subcommands. Program name, author and date are provided as
 * parameters. The rest of the parameters defined the options and/or
 * subcmds.
 *
 * The "cl_subcmd" methods define subcommands for the parent
 * command. This example includes one subcommand level, but multiple
 * levels are allowed.
 *
 * "cl_finish" is marker for complete program options definion. It
 * will start parsing and checking for options. After "cl_finish"
 * the user can query the options.
 *
 * Main (root) commands can be referenced through variables:
 * <pre>
 *   cl_main or cl_cmd
 * </pre>
 *
 * The subcommands can be referenced through "cl_main" (etc.)
 * @code
 *   cl_given_subcmd
 * @endcode
 * or by name
 * @code
 *   cl_subcmd( "add" )
 * @endcode
 *
 * The queries have too versions: "cl_<query>" and
 * "cl_cmd_<query>". For "cl_<query>" it is assumed that the query
 * is targeted to cl_main. For "cl_cmd_<query>" the first argument
 * is always a "cl_cmd_t" which defines the scope of query.
 *
 *
 * #### Subcommand example executions
 *
 * Normal behavior would be achieved by executing:
 * @code
 *   shell> comlin_subcmd add -fo -f example
 * @endcode
 *
 * The program would execute with the following output:
 * @code
 Options for: comlin_subcmd
 Given "help": false
 Given "add": true
 Given "rm": false
 Options for: add
 Given "help": false
 Given "force": true
 Given "password": false
 Given "username": false
 Given "file": true
 Value "file": example
 * @endcode
 *
 *
 * Help is automatically provided on each command level, thus these are
 * both valid.
 *   shell> comlin_subcmd -h
 * and
 *   shell> comlin_subcmd rm -h
 *
 *
 *
 * ## Option specification
 *
 * ### Overview
 *
 * Option specification includes the minimum set of information
 * required for command line parsing. It is used to:
 * - Parse the command line.
 * - Check for wrong options and report.
 * - Check for mandatory arguments and report.
 * - Set the options given/non-given state.
 * - Set the options value. Array/String for all except no value for
 *   switches (check given property instead).
 * - Generate Usage Help printout.
 *
 *
 * ### Option types
 *
 * The following types can be defined for the command line options:
 * - CL_SUBCMD: Subcmd option. Subcmd specific options are provided
 *           separately.
 * - CL_SWITCH: Single switch option (no arguments).
 * - CL_SINGLE: Mandatory single argument option.
 * - CL_MULTI: Mandatory multiple argument option (one or many). Option
 *          values in array.
 * - CL_OPT_SINGLE: Optional single argument option.
 * - CL_OPT_MULTI: Optional multiple argument option (one or many). Option
 *              values in array.
 * - CL_OPT_ANY: Optional multiple argument option (also none
 *            accepted). Option values in array.
 * - CL_DEFAULT: Default option (no switch associated). Name and
 *            option String values can be left out, since only the
 *            document string is used. Default option is referred with
 *            NULL for name.
 * - CL_EXCLUSIVE: Option that does not coexist with other options.
 * - CL_SILENT: Option that does not coexist with other options and is not
 *           displayed as an option in Usage Help display. In effect a
 *           sub-option of :exclusive.
 *
 * Options use all the 4 option fields:
 * @code
 *   [ type, name, mnemonic, doc ]
 * @endcode
 *
 * "type" field is mandatory for all options.
 *
 * "name" field is also mandatory for all options. "mnemonic" can be
 * left out (set to NULL), but then option accepts only long option format.
 *
 * "CL_DEFAULT" uses only "doc" and "CL_SUBCMD" doesn't use the "mnemonic"
 * field. Those fields should be set to "NULL", however.
 *
 * "CL_MULTI", "CL_OPT_MULTI", and "CL_OPT_ANY" option arguments are
 * terminated only when an option specifier is found. This can be a
 * problem if "CL_DEFAULT" option follows. The recommended solution is to
 * use a "CL_SILENT" option that can be used to terminate the argument
 * list. For example:
 * @code
 *   { CL_SILENT, "terminator", "-", "The terminator." },
 * @endcode
 *
 *
 * ### Option type primitives
 *
 * Comlin converts option types into option type primitives. Option types
 * are not completely orthogonal, but primitives are.
 *
 * Primitives:
 *
 * - CL_P_NONE: No arguments (i.e. switch).
 * - CL_P_ONE: One argument.
 * - CL_P_MANY: More than one argument.
 * - CL_P_OPT: Optional argument(s).
 * - CL_P_DEFAULT: Default option.
 * - CL_P_MUTEX: Mutually exclusive option.
 * - CL_P_HIDDEN: Hidden option (no usage doc).
 *
 * Types to primitives mapping:
 *
 * - CL_SWITCH: CL_P_NONE, CL_P_OPT
 * - CL_SINGLE: CL_P_ONE
 * - CL_MULTI: CL_P_ONE, CL_P_MANY
 * - CL_OPT_SINGLE: CL_P_ONE, CL_P_OPT
 * - CL_OPT_MULTI: CL_P_ONE, CL_P_MANY, CL_P_OPT
 * - CL_OPT_ANY: CL_P_NONE, CL_P_ONE, CL_P_MANY, CL_P_OPT
 * - CL_DEFAULT: CL_P_NONE, CL_P_ONE, CL_P_MANY, CL_P_OPT, CL_P_DEFAULT
 * - CL_EXCLUSIVE: CL_P_NONE, CL_P_ONE, CL_P_MANY, CL_P_OPT, CL_P_MUTEX
 * - CL_SILENT: CL_P_NONE, CL_P_OPT, CL_P_HIDDEN
 *
 * Primitives can be used in place of types if exotic options are
 * needed. Instead of a single type, ored combination of primitives
 * are given for option type. Order of primitives is not significant.
 *
 * For example:
 * @code
 *   { CL_P_NONE | CL_P_HIDDEN | CL_P_OPT, "terminator", "-", "The terminator." },
 * @endcode
 *
 * Comlin does not check the primitive combinations, thus care and
 * consideration should be applied.
 *
 *
 * ### Option specification method configuration
 *
 * Option behavior can be controlled with several configuration options.
 *
 * The configuration options are set by execution configuration
 * function. These are the called after option has been specified and
 * before cl_finish. Setting the configuration at "cl_maincmd"
 * will propagate the config options to all the subcommands as
 * well. Configuration can be given to each subcommand separately to
 * override the inherited config values. Subcommand settings are not
 * inherited, but apply only in the subcommand.
 *
 * The usable configuration keys:
 * - autohelp: Add help option automatically (default: true). Custom
 *             help option can be provided and it can be made also
 *             visible to user.
 * - header: Header lines before standard usage printout.
 * - footer: Footer lines after standard usage printout.
 * - subcheck: Automatically check that a subcommand is provided
 *             (default: true).
 * - check_missing: Check for missing arguments (default: true).
 * - check_invalid: Error for unknown options (default: true).
 * - tab: Tab stop column for option documentation (default: 12).
 * - help_exit: Exit program if help displayed (default: true).
 *
 *
 *
 * ## Option referencing
 *
 * ### Existence and values
 *
 * cl_opt_t includes the parsed option values. All options can be
 * tested whether they are specified on the command line using:
 * @code
 *   cl_given( "name" )
 * @endcode
 * or
 * @code
 *   cl_cmd_given( cmd, "name" )
 * @endcode
 *
 * Provided value(s) is returned by:
 * @code
 *   cl_value( "name" )
 * @endcode
 * or
 * @code
 *   cl_cmd_value( cmd, "name" )
 * @endcode
 *
 * For "CL_SWITCH" there is no value and for the other types they are
 * string (array of one) or an array of multiple strings.
 *
 * With "CL_OPT_ANY" type, the user should first check if the option was given:
 *
 * @code
 *   cl_cmd_given( cmd, "many_files_or_none" )
 * @endcode
 *
 * Then check how many arguments where given, and finally decide what
 * to do. The value array is terminated with NULL. The number of
 * values are also stored to "valuecnt" field in cl_opt_t struct.
 *
 * Header file "comlin.h" includes user definitions and documentation
 * for user interface functions.
 *
 *
 * ### Subcommand options
 *
 * The given subcommand for the parent command is return by
 * "cl_given_subcmd" or "cl_cmd_given_subcmd". Commonly the
 * program creator should just check directly which subcommand has
 * been selected and check for any subcommand options set.
 *
 *
 * ### Program external options
 *
 * If the user gives the "\--" option (double-dash), the arguments after
 * that option is stored as an array to "cl_external".
 *
 *
 * ## Customization
 *
 * If the default behavior is not satisfactory, changes can be
 * implemented simply by complementing the existing functions. Some
 * knowledge of the internal workings of Comlin is required though.
 *
 *
 * ## Background
 *
 * Comlin (for C-lang) implements a subset of Comlin (for Ruby).
 *
 * C-lang version limitations:
 * - No rule checking support.
 *
 *
 * ## User API function index
 *
 * ### Option specification
 *
 * - #cl_command( prog,author,year,... )
 * - #cl_maincmd( prog,author,year,... )
 * - #cl_subcmd( name,parentname,... )
 * - void cl_finish( void );
 * - void cl_end( void );
 *
 *
 * ### Option queries
 *
 * - cl_opt_t cl_opt( char* name );
 * - char** cl_value( char* name );
 * - cl_opt_t cl_given( char* name );
 * - cl_opt_t cl_cmd_opt( cl_cmd_t cmd, char* name );
 * - char** cl_cmd_value( cl_cmd_t cmd, char* name );
 * - cl_opt_t cl_cmd_given( cl_cmd_t cmd, char* name );
 * - cl_cmd_t cl_cmd_subcmd( cl_cmd_t, char* name );
 * - cl_cmd_t cl_given_subcmd( void );
 * - cl_cmd_t cl_cmd_given_subcmd( cl_cmd_t parent );
 *
 *
 * ### Configuration option setting functions
 *
 * - void cl_conf_autohelp( mc_bool_t val );
 * - void cl_conf_header( char* val );
 * - void cl_conf_footer( char* val );
 * - void cl_conf_subcheck( mc_bool_t val );
 * - void cl_conf_check_missing( mc_bool_t val );
 * - void cl_conf_check_invalid( mc_bool_t val );
 * - void cl_conf_tab( int val );
 * - void cl_conf_help_exit( mc_bool_t val );
 *
 *
 * ### Generic functions
 *
 * - void cl_error( const char* format, ... );
 * - void cl_usage( void );
 * - void cl_cmd_usage( cl_cmd_t cmd );
 *
 */


/* Select Slinky or Slider as library. */
#ifdef COMLIN_USE_SLINKY
#define COMLIN_USE_SLIDER 0
#else
#define COMLIN_USE_SLIDER 1
#endif


#include <sixten.h>
#if COMLIN_USE_SLIDER == 1
#include <slider.h>
#else
#include <slinky.h>
#endif


/** Comlin-library version. */
extern const char* comlin_version;


/** Subcmd option. */
#define CL_SUBCMD ( 1 << 0 )
/** Switch option. */
#define CL_SWITCH ( 1 << 1 )
/** Single value option. */
#define CL_SINGLE ( 1 << 2 )
/** Multi value option. */
#define CL_MULTI ( 1 << 3 )
/** Optional single value option. */
#define CL_OPT_SINGLE ( 1 << 4 )
/** Optional multi value option. */
#define CL_OPT_MULTI ( 1 << 5 )
/** 0, 1, or more value option. */
#define CL_OPT_ANY ( 1 << 6 )
/** No id option. */
#define CL_DEFAULT ( 1 << 7 )
/** Disables the other options. */
#define CL_EXCLUSIVE ( 1 << 8 )
/** Non-documented option. */
#define CL_SILENT ( 1 << 9 )

/** No arguments (i.e. switch). */
#define CL_P_NONE ( 1 << 10 )
/** One argument. */
#define CL_P_ONE ( 1 << 11 )
/** More than one argument. */
#define CL_P_MANY ( 1 << 12 )
/** Optional argument(s). */
#define CL_P_OPT ( 1 << 13 )
/** Default option. */
#define CL_P_DEFAULT ( 1 << 14 )
/** Mutually exclusive option. */
#define CL_P_MUTEX ( 1 << 15 )
/** Hidden option (no usage doc). */
#define CL_P_HIDDEN ( 1 << 16 )


/** Option type. */
typedef uint64_t cl_opt_type_t;


/**
 * Option specification entry.
 */
st_struct( cl_opt_spec )
{
    cl_opt_type_t type; /**< Option type. */
    const char*   name; /**< Option name (for reference). */
    const char*   opt;  /**< Short switch ("-x" or NULL). Longopt is used if NULL. */
    const char*   doc;  /**< Option documentation. */
};


/**
 * Parsed option content. Includes option info for the user.
 */
st_struct( cl_opt )
{

    /** Option type. */
    cl_opt_type_t type;

    /** Option name (for reference). */
    const char* name;

    /** Short switch ("-x" or NULL). Longopt is used if NULL. */
    const char* shortopt;

    /** Option documentation. */
    const char* doc;

    /** Generated longopt name: "--#{name}". */
#if COMLIN_USE_SLIDER == 1
    char longopt_use[ 128 ];
    sd_s longopt;
#else
    sl_t longopt;
#endif

    /** Number of values. */
    int valuecnt;

    /** Array of given option values. */
    char** value;

    /** True if option was set on CLI. */
    st_bool_t given;
};


/**
 * Command configuration options. User can change the values with
 * "cl_conf_<option>" functions, e.g. cl_conf_header.
 */
st_struct( cl_config )
{

    /**
     * Generate help option automatically.
     *   default: true
     */
    st_bool_t autohelp;

    /**
     * Usage help header. User have to include all newlines.
     * default: NULL
     */
    char* header;

    /**
     * Usage help footer. User have to include all newlines.
     * default: NULL
     */
    char* footer;

    /**
     * Check for missing sub-commands.
     * default: true
     */
    st_bool_t subcheck;

    /**
     * Check for missing options.
     * default: true
     */
    st_bool_t check_missing;

    /**
     * Check for invalid options.
     * default: true
     */
    st_bool_t check_invalid;

    /**
     * Option mnemonic tab stop for option doc.
     * default: 12
     */
    int tab;

    /**
     * Exit after usage help display.
     * default: true
     */
    st_bool_t help_exit;
};


/**
 * Program level option information including program information and
 * parsing results.
 */
st_struct( cl_cmd )
{

    /** Command name. */
    char* name;

    /** Name with hierachy. */
    char* longname;

    /** Program author. */
    char* author;

    /** Year (or data for program). */
    char* year;

    /** Number of options. */
    int optcnt;

    /** Array of options (objects). */
    cl_opt_p opts;

    /** Parent (host) for this subcmd. */
    cl_cmd_t parent;

    /** Number of subcmds. */
    int subcmdcnt;

    /** Array of subcmds. */
    cl_cmd_p subcmds;

    /** Array of program external options. */
    char** external;

    /** Is this subcmd given?. */
    int given;

    /** Number of given arguments. */
    int givencnt;

    /** Number of option errors. */
    int errors;

    /** Command configuration. */
    cl_config_t conf;
};


/**
 * Active comlin command (under processing). Same as main after option
 * parsing.
 */
extern cl_cmd_t cl_cmd;

/** Main command, i.e. root of command hierarchy. */
extern cl_cmd_t cl_main;


/** Number of arguments for comlin. */
extern int cl_argc;

/** Array of arguments for comlin. */
extern char** cl_argv;



/* User interface macros: */

/**
 * User interface (macro) for command and option specification
 * (including parsing).
 */
#define cl_command( prog, author, year, ... ) \
    do {                                      \
        cl_init( argc, argv, author, year );  \
        cl_subcmd( prog, NULL, __VA_ARGS__ ); \
        cl_finish();                          \
    } while ( 0 )

/**
 * User interface (macro) for command and option specification
 * (including parsing).
 */
#define cl_complete( prog, author, year, ... ) \
    do {                                       \
        cl_init( argc, argv, author, year );   \
        cl_subcmd( prog, NULL, __VA_ARGS__ );  \
        cl_finish();                           \
    } while ( 0 )


/**
 * User interface (macro) for main cmd (program) specification.
 */
#define cl_maincmd( prog, author, year, ... ) \
    do {                                      \
        cl_init( argc, argv, author, year );  \
        cl_subcmd( prog, NULL, __VA_ARGS__ ); \
    } while ( 0 )


/**
 * User interface (macro) for sub-command specification.
 */
#define cl_subcmd( name, parentname, ... ) \
    cl_spec_subcmd(                        \
        name, parentname, (cl_opt_spec_s[]){ __VA_ARGS__ }, cl_spec_size( __VA_ARGS__ ) )

/**
 * Option specification list (array) size.
 */
#define cl_spec_size( ... ) ( sizeof( (cl_opt_spec_s[]){ __VA_ARGS__ } ) / sizeof( cl_opt_spec_s ) )



/*
 * User interface functions:
 */

/**
 * Finalize setup and parse all options.
 */
void cl_finish( void );



/* ------------------------------------------------------------
 * Option query functions:
 */


/**
 * Get main command option (by name).
 *
 * @param name Option name (NULL for default arg).
 *
 * @return Option.
 */
cl_opt_t cl_opt( char* name );


/**
 * Get value of main command option.
 *
 * @param name Option name.
 *
 * @return Option value.
 */
char** cl_value( char* name );


/**
 * Get given status of main command option.
 *
 * @param name Option name.
 *
 * @return Option if given.
 */
cl_opt_t cl_given( char* name );


/**
 * Get command option (by name).
 *
 * @param cmd Command containing option.
 * @param name Option name (NULL for default arg).
 *
 * @return Option.
 */
cl_opt_t cl_cmd_opt( cl_cmd_t cmd, char* name );


/**
 * Get value of command option.
 *
 * @param cmd Command containing option.
 * @param name Option name.
 *
 * @return Option value.
 */
char** cl_cmd_value( cl_cmd_t cmd, char* name );


/**
 * Get given status of command option.
 *
 * @param cmd Command containing option.
 * @param name Option name.
 *
 * @return Option's given status (true if given).
 */
cl_opt_t cl_cmd_given( cl_cmd_t cmd, char* name );


/**
 * Get cmd's sub-command (by name).
 *
 * @param cmd Parent.
 * @param name Subcmd name.
 *
 * @return Subcmd.
 */
cl_cmd_t cl_cmd_subcmd( cl_cmd_t cmd, char* name );


/**
 * Return given subcmd for cl_main.
 *
 * @return Subcmd (or NULL).
 */
cl_cmd_t cl_given_subcmd( void );


/**
 * Return given subcmd for parent.
 *
 * @param parent Parent for subcmd.
 *
 * @return Subcmd (or NULL).
 */
cl_cmd_t cl_cmd_given_subcmd( cl_cmd_t parent );


/**
 * Return program external argument list.
 *
 * @return External args.
 */
char** cl_external( void );


/**
 * Generate id for option. Use short option if defined, otherwise
 * longopt. For sub-commands the name is returned.
 *
 * @param opt Option.
 *
 * @return Id.
 */
const char* cl_opt_id( cl_opt_t opt );



/* ------------------------------------------------------------
 * Configuration option setting functions:
 */


/** Set autohelp configuration value. */
void cl_conf_autohelp( st_bool_t val );

/** Set header configuration value. */
void cl_conf_header( char* val );

/** Set footer configuration value. */
void cl_conf_footer( char* val );

/** Set subcheck configuration value. */
void cl_conf_subcheck( st_bool_t val );

/** Set check_missing configuration value. */
void cl_conf_check_missing( st_bool_t val );

/** Set check_invalid configuration value. */
void cl_conf_check_invalid( st_bool_t val );

/** Set tab configuration value. */
void cl_conf_tab( int val );

/** Set help_exit configuration value. */
void cl_conf_help_exit( st_bool_t val );



/* ------------------------------------------------------------
 * Generic functions
 */

/**
 * Report comlin error with command prefix. Increment error counter.
 *
 * @param format String formatter.
 * @param ... Args for formatter.
 */
void cl_error( const char* format, ... );


/**
 * Display main command usage.
 *
 */
void cl_usage( void );


/**
 * Display command usage.
 *
 * @param cmd Command to display.
 */
void cl_cmd_usage( cl_cmd_t cmd );


/**
 * Display options's value(s). Used for testing/debug.
 *
 * @param fh File stream to use.
 * @param o Option to display.
 */
void cl_display_values( FILE* fh, cl_opt_t o );


/* ------------------------------------------------------------
 * Functions called by macros.
 */

/**
 * Initialize global comlin state.
 *
 * @param argc C-main argument count.
 * @param argv C-main argument array.
 * @param author Program author.
 * @param year Program creation date (year).
 */
void cl_init( int argc, char** argv, char* author, char* year );


/**
 * Option specification for subcmd. If parentname is NULL, then this
 * command (subcmd) is made the main command. Subcmd is added to the
 * global command list.
 *
 * Subcmd can be configured after this function. If subcmd has a
 * parent, the configuration is inherited from the parent.
 *
 * Example:
 *
 * @code
 *  cl_spec_subcmd( "comlin_simple", "Programmer", "2018",
 *                     (cl_opt_spec_s []) {
 *                       { CL_OPT_SINGLE, "file",  "-f", "File argument." },
 *                       { CL_OPT_SWITCH, "debug", "-d", "Enable debugging." }
 *                     }, 2 );
 * @endcode
 *
 * @param name Name.
 * @param parentname Name of subcmd parent.
 * @param spec Array of option specifications.
 * @param size Size of the specification array.
 */
void cl_spec_subcmd( char* name, char* parentname, cl_opt_spec_t spec, int size );


/**
 * Cleanup for all allocations made by Comlin. User does not normally
 * need to do this, since command line options are parsed only once,
 * and the user might want to refer to the options upto the very end.
 *
 * This function is provided for the sake of completion and to ensure
 * that Comlin does not leak memory.
 *
 * @param cmd Command to clean.
 */
void cl_cmd_end( cl_cmd_t cmd );


/**
 * Same as @see cl_cmd_end(), but for default Comlin.
 */
void cl_end( void );


#endif
