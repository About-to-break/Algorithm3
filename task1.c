//Check comments in solution1 then delete them.
//Solution is a basic function. U can delete it too.


#include <emacs-module.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

const volatile bool plugin_is_GPL_compatible; // necessary for plugin to get loaded

static int register_function(emacs_env *env, const emacs_value func, const char *function_name) {
    const emacs_value name_as_symbol = env->intern (env, function_name);
    if (!func || !name_as_symbol)
        return 1;
    emacs_value args[] = {name_as_symbol, func};
    env->funcall(env, env->intern(env, "defalias"), sizeof(args) / sizeof(emacs_value), args);
    return 0;
}

static bool has_exception_happened(emacs_env *env) {
 return emacs_funcall_exit_return != env->non_local_exit_check(env);
}

static unsigned get_last_line_number(emacs_env *env) {
    emacs_value old_caret_location = env->funcall(env, env->intern(env, "point"), 0, NULL);
    env->funcall(env, env->intern(env, "end-of-buffer"), 0, NULL);
    emacs_value last_line_num = env->funcall(env, env->intern(env, "line-number-at-pos"), 0, NULL);
    env->funcall(env, env->intern(env, "goto-char"), 1, &old_caret_location);
    return (unsigned) env->extract_integer(env, last_line_num);
}

        static emacs_value solution(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data) {
            (void)data; // silence "unused argument" warning
            // we throw exception if something went wrong, the return value doesn't matter
            emacs_value ret = env->intern(env, "nil");

            assert(nargs == 1); // nargs check is done by Emacs so this shouldn't fail
            
            unsigned last_line = get_last_line_number(env);
            if (has_exception_happened(env))
                return ret;

            printf("last line number is %u\n", last_line);

            const char hello_world[] = "Hello world from plugin!\n\n";
            emacs_value hello_world_lisp_string = env->make_string(env, hello_world, sizeof(hello_world)-1);
            env->funcall(env, env->intern(env, "insert"), 1, &hello_world_lisp_string);

            if (has_exception_happened(env))
                return ret;

            env->funcall(env, env->intern(env, "write-file"), 1, args);

            return ret;
        }

static emacs_value solution1(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data) {
    (void)data; // Silence "unused argument" warning
    emacs_value ret = env->intern(env, "nil");

    assert(nargs == 1); // Argument count check
    
    //Get the number of lines
    unsigned last_line = get_last_line_number(env);
    if (has_exception_happened(env)) {
        return ret;
    }
    //Go to the start of the buffer
    env->funcall(env, env->intern(env, "beginning-of-buffer"), 0, NULL);
    //Print 1. here
    const char first_line_num_prefix[] = "1. ";
    //Create lisp emacs string
    emacs_value first_line_prefix_lisp_string = env->make_string(env, first_line_num_prefix, sizeof(first_line_num_prefix)-1);
    //Insert modified string to buffer
    env->funcall(env, env->intern(env, "insert"), 1, &first_line_prefix_lisp_string);
    //Create lisp emacs integer 1 to move one line down all the time
    emacs_value one = env->make_integer(env, 1);

    for (unsigned i = 2; i < last_line; i++) {
        // Move to the start of the next line and do extra check
        env->funcall(env, env->intern(env, "forward-line"), 1, &one);
        env->funcall(env, env->intern(env, "beginning-of-line"), 0, NULL);
        //Creating char buffer to store prefix
        char buffer[32];
        //Add int to C string
        snprintf(buffer, sizeof(buffer), "%u. ", i);
        //Create lisp emacs string
        emacs_value current_line_prefix_lisp_string = env->make_string(env, buffer, strlen(buffer));
        //Insert modified string to buffer
        env->funcall(env, env->intern(env, "insert"), 1, &current_line_prefix_lisp_string);
    }
    if (has_exception_happened(env))
        return ret;
        
    //Write changes to file
    env->funcall(env, env->intern(env, "write-file"), 1, args);

    return ret;
}


int emacs_module_init(struct emacs_runtime *ert) {
    emacs_env *env = ert->get_environment(ert);
    //Only solution1 is registered, so do not be distracted with compilation warning
    emacs_value func = env->make_function(env, 1, 1, solution1,
                                          "A solution to lab assignment!", NULL);
    return register_function(env, func, "solution1");
}
