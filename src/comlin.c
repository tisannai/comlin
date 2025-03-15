/**
 * @file   comlin.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Mon Oct 29 23:51:57 2018
 *
 * @brief  Comlin - Command line parsing library.
 *
 */

#include "comlin.h"



/*
 * ------------------------------------------------------------
 * Comlin external vars.
 */

const char* comlin_version = "0.1";
cl_cmd_t    cl_cmd = NULL;
cl_cmd_t    cl_main = NULL;
int         cl_argc = 0;
char**      cl_argv = NULL;



/*
 * ------------------------------------------------------------
 * Comlin internal vars.
 */

/** Command line argument index. */
static int arg_idx;

/** Global list of commands. */
static cl_cmd_t cmd_list[ 128 ];

/** Index for command list append. Index to list end. */
static int cmd_list_idx = 0;

/** Main command configuration. */
static cl_config_t cl_conf = NULL;




/*
 * ------------------------------------------------------------
 * Comlin internal functions.
 */


/**
 * Report fatal (internal) error.
 *
 */
// GCOV_EXCL_START
void cl_fatal( const char* format, ... )
{
    va_list ap;

    va_start( ap, format );
    fputs( "COMLIN FATAL: ", stderr );
    vfprintf( stderr, format, ap );
    va_end( ap );
}
// GCOV_EXCL_STOP


/**
 * Create cl_cmd_t data structure.
 *
 * @return Structure.
 */
static cl_cmd_t cmd_create( void )
{
    cl_cmd_t cmd;

    cmd = st_new( cl_cmd_s );
    cmd->name = NULL;
    cmd->longname = NULL;
    cmd->author = NULL;
    cmd->year = NULL;
    cmd->givencnt = 0;
    cmd->given = st_false;
    cmd->errors = 0;
    cmd->parent = NULL;
    cmd->subcmdcnt = 0;
    cmd->subcmds = NULL;

    cmd->opts = NULL;

    return cmd;
}


/**
 * Create and setup cl_opt_t data structure.
 *
 * @param type Option type.
 * @param name Option name.
 * @param opt Option mnemonic.
 * @param doc Option documentaion.
 *
 * @return Structure.
 */
static cl_opt_t opt_create( cl_opt_type_t type, const char* name, const char* opt, const char* doc )
{
    cl_opt_t co;

    co = st_new( cl_opt_s );

    if ( type == CL_DEFAULT ) {
        /* Force these for default type. */
        co->name = "<default>";
        co->shortopt = "<default>";
    } else {
        co->name = name;
        co->shortopt = opt;
    }

    /* Convert type definition to primitives. */
    switch ( type ) {
        case CL_SWITCH:
            type = CL_P_NONE | CL_P_OPT;
            break;
        case CL_SINGLE:
            type = CL_P_ONE;
            break;
        case CL_MULTI:
            type = CL_P_ONE | CL_P_MANY;
            break;
        case CL_OPT_SINGLE:
            type = CL_P_ONE | CL_P_OPT;
            break;
        case CL_OPT_MULTI:
            type = CL_P_ONE | CL_P_MANY | CL_P_OPT;
            break;
        case CL_OPT_ANY:
            type = CL_P_NONE | CL_P_ONE | CL_P_MANY | CL_P_OPT;
            break;
        case CL_DEFAULT:
            type = CL_P_NONE | CL_P_ONE | CL_P_MANY | CL_P_OPT | CL_P_DEFAULT;
            break;
        case CL_EXCLUSIVE:
            type = CL_P_NONE | CL_P_ONE | CL_P_MANY | CL_P_OPT | CL_P_MUTEX;
            break;
        case CL_SILENT:
            type = CL_P_NONE | CL_P_OPT | CL_P_HIDDEN;
            break;
        default:
            break;
    }

    co->type = type;
    co->doc = doc;

#if COMLIN_USE_SLIDER == 1
    co->longopt = sd_use( co->longopt_use, 128, sd_typ_st );
    sd_format_quick( &co->longopt, "--%s", co->name );
#else
    co->longopt = sl_from_str_with_size_c( "--", strlen( co->name ) + 3 );
    sl_concatenate_c( &co->longopt, co->name );
#endif

    co->valuecnt = 0;
    co->value = NULL;
    co->given = st_false;

    return co;
}


/**
 * Create cl_config_t data structure.
 *
 * @return Structure.
 */
static cl_config_t config_create( void )
{
    cl_config_t conf;

    conf = st_new( cl_config_s );

    /* Setup config defaults. */
    conf->autohelp = st_true;
    conf->header = NULL;
    conf->footer = NULL;
    conf->subcheck = st_true;
    conf->check_missing = st_true;
    conf->check_invalid = st_true;
    conf->tab = 12;
    conf->help_exit = st_true;

    return conf;
}


/**
 * Duplicate configuration.
 *
 * @param src Source data.
 *
 * @return New config.
 */
static cl_config_t config_dup( cl_config_t src )
{
    cl_config_t conf;

    conf = config_create();

    /* Setup config defaults. */
    conf->autohelp = src->autohelp;
    if ( src->header ) {
        conf->header = st_strdup( src->header );
    }
    if ( src->footer ) {
        conf->footer = st_strdup( src->footer );
    }
    conf->subcheck = src->subcheck;
    conf->check_missing = src->check_missing;
    conf->check_invalid = src->check_invalid;
    conf->tab = src->tab;
    conf->help_exit = src->help_exit;

    return conf;
}


/**
 * Register command to global lookup list.
 *
 * @param cmd Command to add.
 */
static void register_cmd( cl_cmd_t cmd )
{
    cmd_list[ cmd_list_idx++ ] = cmd;
    cmd_list[ cmd_list_idx ] = NULL;
}


/**
 * Find command by name.
 *
 * @param name Name.
 *
 * @return Command.
 */
static cl_cmd_t find_cmd_by_name( char* name )
{
    for ( int i = 0; i < cmd_list_idx; i++ ) {
        if ( strcmp( cmd_list[ i ]->name, name ) == 0 ) {
            return cmd_list[ i ];
        }
    }
    return NULL;
}


/**
 * Add subcmd command to parents list.
 *
 * @param parent Host for subcmd.
 * @param subcmd Subcmd to add.
 */
static void add_subcmd( cl_cmd_t parent, cl_cmd_t subcmd )
{
    if ( !parent->subcmds ) {
        parent->subcmds = st_new_n( cl_cmd_s, 128 );
    }

    if ( parent->subcmdcnt >= 128 ) {
        cl_fatal( "Too many sub-commands for \"%s\"!", parent->longname );
    }

    parent->subcmds[ parent->subcmdcnt++ ] = subcmd;
}


/**
 * Find option by type.
 *
 * @param cmd Command including option.
 * @param type Option type.
 *
 * @return Option (or NULL).
 */
static cl_opt_t find_opt_by_type( cl_cmd_t cmd, cl_opt_type_t type )
{
    int i = 0;

    while ( cmd->opts[ i ] ) {
        if ( cmd->opts[ i ]->type & type ) {
            return cmd->opts[ i ];
        }
        i++;
    }

    return NULL;
}


/**
 * Find option by name.
 *
 * @param cmd Command including option.
 * @param name Option name (or NULL for CL_DEFAULT arg).
 *
 * @return Option (or NULL).
 */
static cl_opt_t find_opt_by_name( cl_cmd_t cmd, char* name )
{
    int i = 0;

    if ( name == NULL ) {
        /* Find default arg. */
        return find_opt_by_type( cmd, CL_P_DEFAULT );
    } else {
        while ( cmd->opts[ i ] ) {
            if ( strcmp( cmd->opts[ i ]->name, name ) == 0 ) {
                return cmd->opts[ i ];
            }
            i++;
        }
    }

    return NULL;
}


/**
 * Find option by characterics:
 *  - Default type is searched if str is NULL.
 *  - Long option is searched if str starts with "--".
 *  - Short option is searched if str starts with "-".
 *  - Named option is searched otherwise.
 *
 * @param cmd Command including option.
 * @param str Option tag to search for.
 *
 * @return Option (or NULL).
 */
static cl_opt_t find_opt( cl_cmd_t cmd, char* str )
{
    int i = 0;

    if ( str == NULL ) {
        /* Default option. */
        while ( cmd->opts[ i ] ) {
            if ( cmd->opts[ i ]->type & CL_P_DEFAULT ) {
                return cmd->opts[ i ];
            }
            i++;
        }
    } else if ( strncmp( str, "--", 2 ) == 0 ) {
        /* Long option. */
        while ( cmd->opts[ i ] ) {
#if COMLIN_USE_SLIDER == 1
            if ( sd_compare_cs( &cmd->opts[ i ]->longopt, str ) == 0 ) {
#else
            if ( strcmp( cmd->opts[ i ]->longopt, str ) == 0 ) {
#endif
                return cmd->opts[ i ];
            }
            i++;
        }
    } else if ( str[ 0 ] == '-' ) {
        /* Short option. */
        while ( cmd->opts[ i ] ) {
            if ( cmd->opts[ i ]->shortopt && ( strcmp( cmd->opts[ i ]->shortopt, str ) == 0 ) ) {
                return cmd->opts[ i ];
            }
            i++;
        }
    } else {
        /* By name. */
        return find_opt_by_name( cmd, str );
    }

    return NULL;
}


/**
 * Get current argument from command line.
 *
 * @return Argument.
 */
static char* get_arg( void )
{
    return cl_argv[ arg_idx ];
}


/**
 * Advance command line argument index.
 *
 */
static void next_arg( void )
{
    arg_idx++;
}


/**
 * Is current command line argument an option?
 *
 *
 * @return True if is.
 */
static st_bool_t is_opt( void )
{
    char* s;

    s = get_arg();

    if ( s[ 0 ] == '-' ) {
        return st_true;
    } else {
        return st_false;
    }
}


/**
 * Is option of switch type? Note that default type is also consided a
 * switch.
 *
 * @param opt Option to check.
 *
 * @return True if is.
 */
static st_bool_t has_switch_style_doc( cl_opt_t opt )
{
    if ( ( ( opt->type & CL_P_NONE ) && !( opt->type & CL_P_MANY ) ) ||
         ( opt->type & CL_P_DEFAULT ) ) {
        return st_true;
    } else {
        return st_false;
    }
}


/**
 * Get multiple command line arguments upto to next option or end.
 *
 *
 * @return Array of arguments.
 */
static char** get_option_values( int* valuecnt )
{
    int    cnt = 0;
    int    org_idx = arg_idx;
    char** value;
    int    i;

    /* Count the number of args provided. */
    while ( get_arg() && !is_opt() ) {
        next_arg();
        cnt++;
    }

    arg_idx = org_idx;

    /* Allocate space for value array. */
    value = st_new_n( char*, cnt + 1 );

    for ( i = 0; i < cnt; i++ ) {
        value[ i ] = get_arg();
        next_arg();
    }

    value[ i ] = NULL;
    *valuecnt = cnt;

    return value;
}


/**
 * Add item to char pointer array pointer. Create new array is it
 * doesn't exist.
 *
 * @param [out] storage Pointer where allocation is stored.
 * @param [in] item Item to store.
 */
static void add_value( char*** storage, char* item )
{
    char** items;

    if ( *storage == NULL ) {
        /* First. */
        items = st_new_n( char*, 2 );
        items[ 0 ] = item;
        items[ 1 ] = NULL;
    } else {
        /* Non-first .*/
        int size = 0;

        items = *storage;
        while ( ( *items ) ) {
            items++;
            size++;
        }

        items = *storage;
        items = st_realloc( items, ( size + 2 ) * sizeof( char* ) );
        items[ size ] = item;
        items[ size + 1 ] = NULL;
    }

    *storage = items;
}


/**
 * Check for missing required arguments. Checking ends if exclusive
 * argument is given. Checking continues with subcmd if encountered.
 *
 * @param cmd Command to check.
 * @param errcmd Command that had missing options.
 *
 * @return True if no missing.
 */
static st_bool_t check_missing( cl_cmd_t cmd, cl_cmd_p errcmd )
{
    cl_opt_p  opts;
    cl_opt_t  o;
    st_bool_t ret = st_true;
    st_bool_t subcheck;
    cl_cmd_t  subcmd;

    if ( !cmd->conf->check_missing ) {
        return st_true;
    }

    /* Check for any exclusive args first. Missing are not checked if has exclusives. */
    opts = cmd->opts;
    while ( *opts ) {
        o = *opts;
        if ( ( o->type & CL_P_MUTEX ) && o->given ) {
            return ret;
        }
        opts++;
    }

    /* Check for missing options. */
    opts = cmd->opts;
    while ( *opts ) {
        o = *opts;
        if ( ( o->type != CL_SUBCMD ) && !( o->type & CL_P_OPT ) && !o->given ) {
            cl_error( "Option \"%s\" missing for \"%s\"...", cl_opt_id( o ), cmd->longname );
            *errcmd = cmd;
            return st_false;
        }
        opts++;
    }

    /* Check for missing subcmds. */
    if ( cmd->subcmds ) {
        /* Has subcmds. */
        if ( cmd->conf->subcheck ) {
            subcheck = st_true;
        } else {
            subcheck = st_false;
        }
    } else {
        subcheck = st_false;
    }

    subcmd = cl_cmd_given_subcmd( cmd );

    if ( subcmd ) {
        /* Go to level subcmd level. */
        return check_missing( subcmd, errcmd );
    } else if ( subcheck ) {
        cl_error( "Subcommand required for \"%s\"...", cmd->name );
        *errcmd = cmd;
        return st_false;
    }

    return ret;
}


/**
 * Parse command line and store given option values to options objects
 * until subcmd is encountered or end.
 *
 * @param cmd Command to update.
 * @param subcmd Command to update next.
 *
 * @retval 0 if all arguments have been parsed.
 * @retval 1 if should continue with subcmd.
 * @retval 2 if errors in parsing.
 */
static int parse_opts( cl_cmd_t cmd, cl_cmd_p subcmd )
{
    cl_opt_t o;
    cl_cmd_t c;

    while ( get_arg() ) {

        /* Option terminator?. */
        if ( strcmp( "--", get_arg() ) == 0 ) {
            /*  Rest of the args do not belong to this program. */
            next_arg();
            cl_cmd->external = &( cl_argv[ arg_idx ] );
            break;
        }

        else if ( is_opt() ) {

            /* Normal option. */

            o = find_opt( cmd, get_arg() );

            if ( !o ) {

                /* Not found, might be default. */

                if ( cmd->conf->check_invalid ) {
                    /* Report missing. */
                    cl_error( "Unknown option \"%s\"...", get_arg() );
                    break;
                } else {
                    /* Default option. */
                    o = find_opt_by_type( cmd, CL_P_DEFAULT );
                    if ( !o ) {
                        cl_error( "No default option specified to allow \"%s\"...", get_arg() );
                        break;
                    } else {
                        if ( !o->value ) {
                            cmd->givencnt++;
                        }
                        add_value( &( o->value ), get_arg() );
                        o->valuecnt++;
                    }
                }
            } else if ( o && ( ( o->type & CL_P_ONE ) || ( o->type & CL_P_MANY ) ) ) {

                /* Option with arguments. */

                next_arg();

                if ( ( !get_arg() || is_opt() ) && !( o->type & CL_P_NONE ) ) {
                    cl_error( "No argument given for \"%s\"...", cl_opt_id( o ) );
                    break;
                } else {
                    if ( o->type & CL_P_MANY ) {
                        /* Get all argument for multi-option. */
                        o->value = get_option_values( &( o->valuecnt ) );
                    } else {
                        if ( o->given ) {
                            cl_error( "Too many arguments for option (\"%s\")...", cl_opt_id( o ) );
                            break;
                        }

                        o->value = st_new_n( char*, 2 );
                        o->value[ 0 ] = get_arg();
                        o->value[ 1 ] = NULL;
                        o->valuecnt++;
                        next_arg();
                    }

                    o->given = st_true;
                    cmd->givencnt++;
                }
            } else {

                /* Switch option. */
                o->given = st_true;
                cmd->givencnt++;
                next_arg();
            }
        } else {

            /* Subcmd or default. Check for Subcmd first. */
            o = find_opt( cmd, get_arg() );

            if ( !o || o->type != CL_SUBCMD ) {

                /* Default argument. */

                o = find_opt_by_type( cmd, CL_P_DEFAULT );

                if ( !o ) {
                    if ( cmd->subcmds ) {
                        cl_error( "Unknown subcmd: \"%s\"...", get_arg() );
                    } else {
                        cl_error( "No default option specified to allow \"%s\"...", get_arg() );
                    }
                    next_arg();
                } else {
                    if ( !o->value ) {
                        cmd->givencnt++;
                    }
                    add_value( &( o->value ), get_arg() );
                    o->valuecnt++;
                    o->given = st_true;
                    next_arg();
                }
            } else {

                /* Subcmd. */

                /* Search for Subcmd. */
                c = find_cmd_by_name( get_arg() );
                o->given = st_true;
                c->given = st_true;
                next_arg();
                *subcmd = c;
                return 1;
            }
        }
    }

    if ( cl_cmd->errors > 0 ) {
        cmd->errors = cl_cmd->errors;
        *subcmd = cmd;
        return 2;
    } else {
        return 0;
    }
}


/**
 * Proxy for parse_opts. Checks for status after each subcmd and
 * recurses further if no errors.
 *
 * @param cmd Command to parse.
 * @param errcmd Command having errors.
 *
 * @return True if no errors.
 */
static st_bool_t setup_and_parse( cl_cmd_t cmd, cl_cmd_p errcmd )
{
    int      ret;
    cl_cmd_t subcmd;

    ret = parse_opts( cmd, &subcmd );

    if ( ret == 0 ) {
        /* done.*/
        return st_true;
    } else if ( ret == 1 ) {
        /* continue. */
        return setup_and_parse( subcmd, errcmd );
    } else if ( ret == 2 ) {
        /* error. */
        *errcmd = subcmd;
        return st_false;
    } else {
        return st_true;
    }
}


/**
 * Add option's command line formatting (usage) to str.
 *
 * @param [out] str String where command line is stored.
 * @param [in] o Option to add.
 */
#if COMLIN_USE_SLIDER == 1
static void opt_cmdline( sd_t str, cl_opt_t o )
{
    if ( o->type & CL_P_HIDDEN ) {
        return;
    }

    if ( o->type & CL_P_OPT ) {
        sd_format_quick( str, "[" );
    }

    sd_format_quick( str, "%s", cl_opt_id( o ) );

    if ( !has_switch_style_doc( o ) ) {
        sd_format_quick( str, " <%s>", o->name );

        if ( ( o->type & CL_P_NONE ) && ( o->type & CL_P_MANY ) ) {
            sd_format_quick( str, "*" );
        } else if ( o->type & CL_P_MANY ) {
            sd_format_quick( str, "+" );
        }
    }

    if ( o->type & CL_P_OPT ) {
        sd_format_quick( str, "]" );
    }
}
#else
static void opt_cmdline( sl_p str, cl_opt_t o )
{
    if ( o->type & CL_P_HIDDEN ) {
        return;
    }

    if ( o->type & CL_P_OPT ) {
        sl_format_quick( str, "[" );
    }

    sl_format_quick( str, "%s", cl_opt_id( o ) );

    if ( !has_switch_style_doc( o ) ) {
        sl_format_quick( str, " <%s>", o->name );

        if ( ( o->type & CL_P_NONE ) && ( o->type & CL_P_MANY ) ) {
            sl_format_quick( str, "*" );
        } else if ( o->type & CL_P_MANY ) {
            sl_format_quick( str, "+" );
        }
    }

    if ( o->type & CL_P_OPT ) {
        sl_format_quick( str, "]" );
    }
}
#endif


/**
 * Add option documentation line.
 *
 * @param str String where document is stored.
 * @param o Option to document.
 * @param cmd Containing command (for configuration lookup).
 */
#if COMLIN_USE_SLIDER == 1
static void opt_doc( sd_t str, cl_opt_t o, cl_cmd_t cmd )
{
    int       s, e;
    st_bool_t first;

    if ( o->type & CL_P_HIDDEN ) {
        return;
    }

    /*
     * Reformat doc str, so that newlines start a new line and tab
     * chars align the start to previous line.
     */

    first = st_true;
    s = 0;
    e = 0;
    for ( ;; ) {
        if ( o->doc[ e ] == '\n' || o->doc[ e ] == 0 ) {
            if ( first ) {
                sd_format_quick( str, "  %s%p ", cl_opt_id( o ), cmd->conf->tab + 2 );
                sd_append( str, sc_from_cl( (const char*)&( o->doc[ s ] ), ( e - s ) ) );
                sd_append_ch( str, '\n' );

                if ( o->doc[ e ] == 0 ) {
                    return;
                } else {
                    e++;
                }

                first = st_false;
            } else {
                if ( o->doc[ s ] == '\t' ) {
                    s++;
                    sd_format_quick( str, "  %s%p ", "", cmd->conf->tab + 2 );
                }
                sd_append( str, sc_from_cl( (const char*)&( o->doc[ s ] ), ( e - s ) ) );
                sd_append_ch( str, '\n' );

                if ( o->doc[ e ] == 0 ) {
                    return;
                } else {
                    e++;
                }
            }
            s = e;

        } else {
            e++;
        }
    }
}
#else
static void opt_doc( sl_p str, cl_opt_t o, cl_cmd_t cmd )
{
    int       s, e;
    st_bool_t first;

    if ( o->type & CL_P_HIDDEN ) {
        return;
    }

    /*
     * Reformat doc str, so that newlines start a new line and tab
     * chars align the start to previous line.
     */

    first = st_true;
    s = 0;
    e = 0;
    for ( ;; ) {
        if ( o->doc[ e ] == '\n' || o->doc[ e ] == 0 ) {
            if ( first ) {
                sl_format_quick( str, "  %s%p", cl_opt_id( o ), cmd->conf->tab + 2 );
                sl_append_substr( str, (const char*)&( o->doc[ s ] ), ( e - s ) );
                sl_append_char( str, '\n' );

                if ( o->doc[ e ] == 0 ) {
                    return;
                } else {
                    e++;
                }

                first = st_false;
            } else {
                if ( o->doc[ s ] == '\t' ) {
                    s++;
                    sl_format_quick( str, "  %s%p", "", cmd->conf->tab + 2 );
                }
                sl_append_substr( str, (char*)&( o->doc[ s ] ), ( e - s ) );
                sl_append_char( str, '\n' );

                if ( o->doc[ e ] == 0 ) {
                    return;
                } else {
                    e++;
                }
            }
            s = e;

        } else {
            e++;
        }
    }
}
#endif


/**
 * Display help if help option is given for any of the commands in the
 * hierarchy (recursion).
 *
 * @param cmd Command to search given help.
 */
static void usage_if_help( cl_cmd_t cmd )
{
    cl_cmd_t subcmd;

    if ( cl_cmd_given( cmd, "help" ) ) {
        cl_cmd_usage( cmd );
    } else if ( cmd->subcmds && ( subcmd = cl_cmd_given_subcmd( cmd ) ) ) {
        usage_if_help( subcmd );
    }
}


static void quit( int status )
{
    cl_end();
    exit( status );
}



/*
 * ------------------------------------------------------------
 * Comlin public functions.
 */


void cl_finish( void )
{
    cl_cmd_t  cmd;
    st_bool_t success;
    cl_cmd_t  errcmd;

    /* Parse all arguments and fill information to options. */

    cl_cmd = cl_main;
    cmd = cl_main;

    success = setup_and_parse( cmd, &errcmd );

    if ( !success ) {
        cl_cmd_usage( errcmd );
        quit( EXIT_FAILURE );
    } else if ( !check_missing( cmd, &errcmd ) ) {
        cl_cmd_usage( errcmd );
        quit( EXIT_FAILURE );
    } else {
        usage_if_help( cl_cmd );
    }
}


cl_opt_t cl_opt( char* name )
{
    return find_opt_by_name( cl_cmd, name );
}


char** cl_value( char* name )
{
    cl_opt_t co;
    co = find_opt_by_name( cl_cmd, name );
    return co->value;
}


cl_opt_t cl_given( char* name )
{
    cl_opt_t co;
    co = find_opt_by_name( cl_cmd, name );
    if ( co->given ) {
        return co;
    } else {
        return NULL;
    }
}


cl_opt_t cl_cmd_opt( cl_cmd_t cmd, char* name )
{
    return find_opt_by_name( cmd, name );
}


char** cl_cmd_value( cl_cmd_t cmd, char* name )
{
    cl_opt_t co;
    co = find_opt_by_name( cmd, name );
    return co->value;
}


cl_opt_t cl_cmd_given( cl_cmd_t cmd, char* name )
{
    cl_opt_t co;
    co = find_opt_by_name( cmd, name );
    if ( co->given ) {
        return co;
    } else {
        return NULL;
    }
}


cl_cmd_t cl_cmd_subcmd( cl_cmd_t cmd, char* name )
{
    for ( int i = 0; i < cmd_list_idx; i++ ) {
        if ( strcmp( cmd->subcmds[ i ]->name, name ) == 0 ) {
            return cmd->subcmds[ i ];
        }
    }
    return NULL;
}


cl_cmd_t cl_given_subcmd( void )
{
    return cl_cmd_given_subcmd( cl_cmd );
}


cl_cmd_t cl_cmd_given_subcmd( cl_cmd_t parent )
{
    for ( int i = 0; i < parent->subcmdcnt; i++ ) {
        if ( parent->subcmds[ i ]->given ) {
            return parent->subcmds[ i ];
        }
    }
    return NULL;
}


char** cl_external( void )
{
    return cl_main->external;
}


const char* cl_opt_id( cl_opt_t opt )
{
    if ( opt->type != CL_SUBCMD )
#if COMLIN_USE_SLIDER == 1
        return opt->shortopt ? opt->shortopt : sd_string( &opt->longopt );
#else
        return opt->shortopt ? opt->shortopt : opt->longopt;
#endif
    else {
        return opt->name;
    }
}


/*
 * Functions to set configuration items.
 */

void cl_conf_autohelp( st_bool_t val )
{
    cl_cmd->conf->autohelp = val;
}

void cl_conf_header( char* val )
{
    cl_cmd->conf->header = st_strdup( val );
}

void cl_conf_footer( char* val )
{
    cl_cmd->conf->footer = st_strdup( val );
}

void cl_conf_subcheck( st_bool_t val )
{
    cl_cmd->conf->subcheck = val;
}

void cl_conf_check_missing( st_bool_t val )
{
    cl_cmd->conf->check_missing = val;
}

void cl_conf_check_invalid( st_bool_t val )
{
    cl_cmd->conf->check_invalid = val;
}

void cl_conf_tab( int val )
{
    cl_cmd->conf->tab = val;
}

void cl_conf_help_exit( st_bool_t val )
{
    cl_cmd->conf->help_exit = val;
}


void cl_error( const char* format, ... )
{
    cl_cmd->errors++;

    va_list ap;
    fputc( '\n', stderr );
    fputs( cl_cmd->name, stderr );
    fputs( " error: ", stderr );
    va_start( ap, format );
    vfprintf( stderr, format, ap );
    va_end( ap );
    fputc( '\n', stderr );
}


// GCOV_EXCL_START
void cl_usage( void )
{
    cl_cmd_usage( cl_cmd );
}
// GCOV_EXCL_STOP


#if COMLIN_USE_SLIDER == 1
void cl_cmd_usage( cl_cmd_t cmd )
{
    char      buf[ 1024 ];
    sd_s      str;
    cl_opt_p  co;
    st_bool_t main_cmd, has_visible;

    str = sd_use( buf, 1024, sd_typ_st );

    if ( cmd->conf->header ) {
        sd_format_quick( &str, "%s", cmd->conf->header );
    } else {
        sd_format_quick( &str, "\n" );
    }

    if ( !cmd->parent ) {
        /* Main command. */
        main_cmd = st_true;
    } else {
        /* Subcmd. */
        main_cmd = st_false;
    }


    if ( main_cmd ) {
        sd_format_quick( &str, "  %s", cmd->name );
    } else {
        sd_format_quick( &str, "  Subcommand \"%s\" usage:\n    ", cmd->name );
        sd_format_quick( &str, "%s", cmd->longname );
    }

    /* Command line. */
    has_visible = st_false;
    co = cmd->opts;
    while ( *co ) {
        if ( !( ( *co )->type & CL_P_HIDDEN ) ) {

            has_visible = st_true;

            sd_format_quick( &str, " " );
            if ( ( *co )->type != CL_SUBCMD ) {
                opt_cmdline( &str, *co );
            } else {
                sd_format_quick( &str, "<<subcommand>>" );
                break;
            }
        }
        co++;
    }

    sd_format_quick( &str, "\n\n" );

    /* If cmd has subcmds, use categories: Options, Subcommands. */

    if ( cmd->subcmds && has_visible ) {
        sd_format_quick( &str, "  Options:\n" );
    }

    /* Option documents. */
    co = cmd->opts;
    while ( *co ) {
        if ( ( *co )->type != CL_SUBCMD ) {
            opt_doc( &str, *co, cmd );
        }
        co++;
    }

    if ( cmd->subcmds ) {
        sd_format_quick( &str, "\n  Subcommands:\n" );
    }

    /* Subcmd documents. */
    co = cmd->opts;
    while ( *co ) {
        if ( ( *co )->type == CL_SUBCMD ) {
            opt_doc( &str, *co, cmd );
        }
        co++;
    }

    if ( main_cmd ) {
        sd_format_quick( &str, "\n\n  Copyright (c) %s by %s\n", cmd->year, cmd->author );
    } else {
        sd_format_quick( &str, "\n" );
    }

    if ( cmd->conf->footer ) {
        sd_format_quick( &str, "%s", cmd->conf->footer );
    } else {
        sd_format_quick( &str, "\n" );
    }

    fprintf( stdout, sd_string( &str ) );

    sd_del( &str );

    if ( cmd->conf->help_exit ) {
        quit( EXIT_FAILURE );
    }
}
#else
void cl_cmd_usage( cl_cmd_t cmd )
{
    char      buf[ 1024 ];
    sl_t      str;
    cl_opt_p  co;
    st_bool_t main_cmd, has_visible;

    str = sl_use( buf, 1024 );

    if ( cmd->conf->header ) {
        sl_format_quick( &str, "%s", cmd->conf->header );
    } else {
        sl_format_quick( &str, "\n" );
    }

    if ( !cmd->parent ) {
        /* Main command. */
        main_cmd = st_true;
    } else {
        /* Subcmd. */
        main_cmd = st_false;
    }


    if ( main_cmd ) {
        sl_format_quick( &str, "  %s", cmd->name );
    } else {
        sl_format_quick( &str, "  Subcommand \"%s\" usage:\n    ", cmd->name );
        sl_format_quick( &str, "%s", cmd->longname );
    }

    /* Command line. */
    has_visible = st_false;
    co = cmd->opts;
    while ( *co ) {
        if ( !( ( *co )->type & CL_P_HIDDEN ) ) {

            has_visible = st_true;

            sl_format_quick( &str, " " );
            if ( ( *co )->type != CL_SUBCMD ) {
                opt_cmdline( &str, *co );
            } else {
                sl_format_quick( &str, "<<subcommand>>" );
                break;
            }
        }
        co++;
    }

    sl_format_quick( &str, "\n\n" );

    /* If cmd has subcmds, use categories: Options, Subcommands. */

    if ( cmd->subcmds && has_visible ) {
        sl_format_quick( &str, "  Options:\n" );
    }

    /* Option documents. */
    co = cmd->opts;
    while ( *co ) {
        if ( ( *co )->type != CL_SUBCMD ) {
            opt_doc( &str, *co, cmd );
        }
        co++;
    }

    if ( cmd->subcmds ) {
        sl_format_quick( &str, "\n  Subcommands:\n" );
    }

    /* Subcmd documents. */
    co = cmd->opts;
    while ( *co ) {
        if ( ( *co )->type == CL_SUBCMD ) {
            opt_doc( &str, *co, cmd );
        }
        co++;
    }

    if ( main_cmd ) {
        sl_format_quick( &str, "\n\n  Copyright (c) %s by %s\n", cmd->year, cmd->author );
    } else {
        sl_format_quick( &str, "\n" );
    }

    if ( cmd->conf->footer ) {
        sl_format_quick( &str, "%s", cmd->conf->footer );
    } else {
        sl_format_quick( &str, "\n" );
    }

    fprintf( stdout, str );

    sl_del( &str );

    if ( cmd->conf->help_exit ) {
        quit( EXIT_FAILURE );
    }
}
#endif


void cl_display_values( FILE* fh, cl_opt_t o )
{
    char**    value;
    st_bool_t first = st_true;

    if ( ( o->type & CL_P_MANY ) || ( o->type & CL_P_DEFAULT ) ) {
        fprintf( fh, "[" );
        value = o->value;
        while ( *value ) {
            if ( !first ) {
                fprintf( fh, ", " );
            }
            fprintf( fh, "\"%s\"", *value );
            first = st_false;
            value++;
        }
        fprintf( fh, "]" );
    } else {
        fprintf( fh, "%s", o->value[ 0 ] );
    }
}


void cl_init( int argc, char** argv, char* author, char* year )
{
    int i;

    arg_idx = 0;
    cl_argc = argc - 1;

    /* Null-terminate cl_argv. */
    cl_argv = st_new_n( char*, cl_argc + 1 );
    for ( i = 0; i < cl_argc; i++ ) {
        cl_argv[ i ] = argv[ i + 1 ];
    }

    cl_argv[ i ] = NULL;

    cl_cmd = cmd_create();
    register_cmd( cl_cmd );

    cl_cmd->author = st_strdup( author );
    cl_cmd->year = st_strdup( year );

    cl_cmd->conf = config_create();
    cl_conf = cl_cmd->conf;
}


void cl_spec_subcmd( char* name, char* parentname, cl_opt_spec_t spec, int size )
{
    cl_opt_spec_t ts;
    cl_opt_p      opts;
    cl_cmd_t      cmd;
    int           i, i2;
    cl_cmd_t      parent;

    if ( !parentname ) {
        /* Main cmd, i.e. cl_cmd_t is already initially setup. */
        cmd = cl_cmd;
        cl_main = cl_cmd;
        cmd->conf = cl_conf;

        /* For main both names are the same. */
        cmd->name = st_strdup( name );
        cmd->longname = st_strdup( name );
    } else {
        cmd = cmd_create();
        parent = find_cmd_by_name( parentname );
        if ( !parent ) {
            cl_fatal( "Parent \"%s\" does not exist!", parentname );
        }
        cmd->parent = parent;
        register_cmd( cmd );
        add_subcmd( parent, cmd );
        cmd->conf = config_dup( parent->conf );

        /*
         * For subcmd both name and longname are based on its
         * ancestors.
         */
        cmd->name = st_strdup( name );
#if COMLIN_USE_SLIDER == 1
        {
            char*     strmem;
            st_size_t size;
            sd_s      sd;

            size = sd_legalize_reservation( strlen( parent->longname ) + strlen( name ) + 2 );
            strmem = st_alloc( size );
            sd_use_mem( &sd, strmem, size, sd_typ_hp );
            sd_format_quick( &sd, "%s %s", parent->longname, name );
            cmd->longname = strmem;
        }
#else
        cmd->longname = sl_drop( sl_from_va_str_c( parent->longname, " ", name, NULL ) );
#endif
    }

    cmd->optcnt = size;

    if ( cmd->conf->autohelp ) {
        /* Add space for help. */
        cmd->optcnt++;
    }

    /* optcnt + NULL. */
    opts = st_new_n( cl_opt_s, cmd->optcnt + 1 );

    /* Insert help. */
    i = 0;
    if ( cmd->conf->autohelp ) {
        opts[ i ] = opt_create( CL_SILENT, "help", "-h", "Display usage info." );
        opts[ i ]->type |= CL_P_MUTEX;
        i++;
    }

    /* Create options (after help). */
    i2 = 0;
    while ( i < cmd->optcnt ) {
        ts = &spec[ i2 ];
        opts[ i ] = opt_create( ts->type, ts->name, ts->opt, ts->doc );
        i++;
        i2++;
    }
    opts[ i ] = NULL;

    cmd->opts = opts;
}


void cl_cmd_end( cl_cmd_t cmd )
{
    st_free( cmd->name );
    st_free( cmd->longname );

    /* Author and year are missing from subcmds. */
    if ( cmd->author ) {
        st_free( cmd->author );
    }

    if ( cmd->year ) {
        st_free( cmd->year );
    }

    if ( cmd->conf ) {
        if ( cmd->conf->header ) {
            st_free( cmd->conf->header );
        }
        if ( cmd->conf->footer ) {
            st_free( cmd->conf->footer );
        }
        st_free( cmd->conf );
    }

    st_for_n ( cmd->optcnt ) {
#if COMLIN_USE_SLIDER == 1
        sd_del( &( cmd->opts[ i ]->longopt ) );
#else
        sl_del( &( cmd->opts[ i ]->longopt ) );
#endif
        st_free( cmd->opts[ i ]->value );
        st_free( cmd->opts[ i ] );
    }

    if ( cmd->subcmds ) {
        st_free( cmd->subcmds );
    }

    st_free( cmd->opts );
    st_free( cmd );
}


void cl_end( void )
{
    cl_cmd_p cp = cmd_list;

    st_free( cl_argv );

    while ( *cp ) {
        cl_cmd_end( *cp );
        cp++;
    }
}
