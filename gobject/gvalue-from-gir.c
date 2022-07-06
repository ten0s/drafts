#include <girepository.h>
#include <glib/gprintf.h>
#include "debug.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

// parse_ns_ver("GLib-2.0", &ns, &ver)
void parse_ns_ver(const char *name, char **ns, char **ver) {
    size_t nsLen = strcspn(name, "-");
    *ns = malloc(nsLen + 1);
    memset(*ns, 0, nsLen + 1);
    strncpy(*ns, name, nsLen);

    size_t verLen = strlen(name) - nsLen - 1/*-*/;
    *ver = malloc(verLen + 1);
    memset(*ver, 0, verLen + 1);
    strncpy(*ver, name + nsLen + 1/*-*/, verLen);
}

void print_indent(int level) {
   g_printf("%*s", 4*level, "");
}

gboolean require(GIRepository *repo, const char *ns, const char *ver, int level) {
    print_indent(level);
    g_printf("%s-%s Loading\n", ns, ver);

    GError *error = NULL;
    g_irepository_require(repo, ns, ver, 0, &error);
    if (error) {
        g_error ("Require: %s-%s failed with: %s\n", ns, ver, error->message);
        return FALSE;
    }

    gchar** deps = g_irepository_get_dependencies(repo, ns);
    gchar** iter = deps;
    gboolean failed = FALSE;
    while (*iter && !failed) {
        const char *dep = *iter;

        char *depNs = NULL;
        char *depVer = NULL;
        parse_ns_ver(dep, &depNs, &depVer);
        //g_printf("%s-%s\n", depNs, depVer);

        failed = !require(repo, depNs, depVer, level+1);

        free(depNs);
        free(depVer);

        iter++;
    }

    print_indent(level);
    g_printf("%s-%s %s\n", ns, ver, failed ? "Failed" : "Done");

free:
    iter = deps;
    while (*iter) {
        g_free(*iter);
        iter++;
    }
    g_free(deps);

    return !failed;
}

int main(void)
{
    GIRepository *repo = g_irepository_get_default();

    if (!require(repo, "GObject", "2.0", 0)) {
        return 1;
    }

    if (!require(repo, "Gdk", "3.0", 0)) {
        return 1;
    }

    // GObject Value
    // https://docs.gtk.org/gobject/struct.Value.html
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

    // GObject g_type_from_name
    // https://docs.gtk.org/gobject/func.type_from_name.html
    GIBaseInfo *biTypeFromName = g_irepository_find_by_name(repo, "GObject", "type_from_name");
    if (!biTypeFromName) {
        g_error("ERROR: %s\n", "Could not find GObject.type_from_name");
        return 1;
    }
    GIFunctionInfo *fiTypeFromName = (GIFunctionInfo *)biTypeFromName;


    GError *error = NULL;

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

    const gchar *hello = "Hello";

    GIArgument argsSetString[2] = {
        { .v_pointer = (gpointer)&val  },
        { .v_pointer = (gpointer)hello },
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

    g_printf("GObject.Value.setString: %s\n", hello);

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


    const char *name = "GdkRGBA";

    GIArgument argsTypeFromName[] = {
        { .v_pointer = (gpointer)name },
    };
    if (!g_function_info_invoke(
            fiTypeFromName,
            (const GIArgument *)argsTypeFromName, LENGTH(argsTypeFromName),
            NULL, 0,
            &retval,
            &error)) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_base_info_unref(biTypeFromName);

    g_printf("GObject.type_from_name(\"%s\"): %ld\n", name, retval.v_ulong);

    return 0;
}
