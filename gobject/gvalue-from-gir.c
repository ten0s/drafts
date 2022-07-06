#include <girepository.h>
#include <glib/gprintf.h>
#include "debug.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

void print_deps(GIRepository *repo, const char *name) {
    g_printf("%s deps:\n", name);
    gchar** deps = g_irepository_get_dependencies(repo, name);
    gchar** iter = deps;
    int i = 1;
    while (*iter) {
        g_printf("%d - %s\n", i, *iter);
        g_free(*iter);
        iter++;
        i++;
    }
    g_free(deps);
}

int main(void)
{
    GError *error = NULL;

    GIRepository *repo = g_irepository_get_default();
    g_irepository_require(repo, "GLib", "2.0", 0, &error);
    if (error) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_irepository_require(repo, "GObject", "2.0", 0, &error);
    if (error) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_irepository_require(repo, "Gdk", "3.0", 0, &error);
    if (error) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    print_deps(repo, "GLib");
    print_deps(repo, "GObject");
    print_deps(repo, "Gdk");

    GIBaseInfo *biValue = g_irepository_find_by_name(repo, "GObject", "Value");
    if (!biValue) {
        g_error("ERROR: %s\n", "Could not find GObject.Value");
        return 1;
    }

    //print_info(biValue);

    GIFunctionInfo *fiInit = g_struct_info_find_method(biValue, "init");
    if (!fiInit) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.init");
        return 1;
    }

    GIFunctionInfo *fiSetString = g_struct_info_find_method(biValue, "set_string");
    if (!fiSetString) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.set_string");
        return 1;
    }

    GIFunctionInfo *fiGetString = g_struct_info_find_method(biValue, "get_string");
    if (!fiGetString) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.get_string");
        return 1;
    }

    GIFunctionInfo *fiUnset = g_struct_info_find_method(biValue, "unset");
    if (!fiUnset) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.unset");
        return 1;
    }

    GIArgument retval;
    GValue val = G_VALUE_INIT;

    GIArgument argsInit[2] = {
        { .v_pointer = (gpointer)&val },
        { .v_ulong   = G_TYPE_STRING  },
    };
    if (!g_function_info_invoke(
            fiInit,
            (const GIArgument *)argsInit, LENGTH(argsInit),
            NULL, 0,
            &retval,
            &error)) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.init: %ld\n", G_TYPE_STRING);

    const gchar *str = "Hello";

    GIArgument argsSetString[2] = {
        { .v_pointer = (gpointer)&val },
        { .v_pointer = (gpointer)str  },
    };
    if (!g_function_info_invoke(
            fiSetString,
            (const GIArgument *)argsSetString, LENGTH(argsSetString),
            NULL, 0,
            &retval,
            &error)) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.setString: %s\n", str);

    GIArgument argsGetString[1] = {
        { .v_pointer = (gpointer)&val },
    };
    if (!g_function_info_invoke(
            fiGetString,
            (const GIArgument *)argsGetString, LENGTH(argsGetString),
            NULL, 0,
            &retval,
            &error)) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.getString: %s\n", retval.v_string);

    GIArgument argsUnset[1] = {
        { .v_pointer = (gpointer)&val },
    };
    if (!g_function_info_invoke(
            fiUnset,
            (const GIArgument *)argsUnset, LENGTH(argsUnset),
            NULL, 0,
            &retval,
            &error)) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_base_info_unref(fiUnset);
    g_base_info_unref(fiGetString);
    g_base_info_unref(fiSetString);
    g_base_info_unref(fiInit);
    g_base_info_unref(biValue);

    g_printf("GObject.Value unset\n");

    return 0;
}
