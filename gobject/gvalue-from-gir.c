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

int gobject_value(GIRepository *repo) {

    if (!require(repo, "GObject", "2.0", 0)) {
        return 1;
    }

    //
    // GObject Value
    // https://docs.gtk.org/gobject/struct.Value.html
    //
    // /usr/share/gir-1.0/GObject-2.0.gir
    // Search for  <record name="Value"
    //

    GIBaseInfo *biValue = g_irepository_find_by_name(repo, "GObject", "Value");
    if (!biValue) {
        g_error("ERROR: %s\n", "Could not find GObject.Value");
        return 1;
    }

    //print_info(biValue);

    GIFunctionInfo *fiValueInit = g_struct_info_find_method(biValue, "init");
    if (!fiValueInit) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.init");
        return 1;
    }

    GIFunctionInfo *fiValueSetString = g_struct_info_find_method(biValue, "set_string");
    if (!fiValueSetString) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.set_string");
        return 1;
    }

    GIFunctionInfo *fiValueGetString = g_struct_info_find_method(biValue, "get_string");
    if (!fiValueGetString) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.get_string");
        return 1;
    }

    GIFunctionInfo *fiValueUnset = g_struct_info_find_method(biValue, "unset");
    if (!fiValueUnset) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.unset");
        return 1;
    }

    GError *error = NULL;
    GIArgument retval;

    GValue val = G_VALUE_INIT;

    GIArgument argsInit[2] = {
        { .v_pointer = (gpointer)&val },
        { .v_ulong   = G_TYPE_STRING  },
    };
    if (!g_function_info_invoke(
            fiValueInit,
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
            fiValueSetString,
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
            fiValueGetString,
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
            fiValueUnset,
            (const GIArgument *)argsUnset, LENGTH(argsUnset),
            NULL, 0,
            &retval,
            &error)) {
        g_error ("ERROR: %s\n", error->message);
        return 1;
    }

    g_base_info_unref(fiValueUnset);
    g_base_info_unref(fiValueGetString);
    g_base_info_unref(fiValueSetString);
    g_base_info_unref(fiValueInit);
    g_base_info_unref(biValue);

    g_printf("GObject.Value unset\n");

    return 0;
}

int gdk_rgba(GIRepository *repo) {

    if (!require(repo, "Gdk", "3.0", 0)) {
        return 1;
    }

    //
    // GDK RGBA
    // https://docs.gtk.org/gdk3/struct.RGBA.html
    //
    // /usr/share/gir-1.0/Gdk-3.0.gir
    // Search for <record name="RGBA"
    //

    GIBaseInfo *biRgba = g_irepository_find_by_name(repo, "Gdk", "RGBA");
    if (!biRgba) {
        g_error("ERROR: %s\n", "Could not find Gdk.Rgba");
        return 1;
    }

    print_info(biRgba);

    // GObject g_type_from_name
    // https://docs.gtk.org/gobject/func.type_from_name.html
    GIBaseInfo *biTypeFromName = g_irepository_find_by_name(repo, "GObject", "type_from_name");
    if (!biTypeFromName) {
        g_error("ERROR: %s\n", "Could not find GObject.type_from_name");
        return 1;
    }
    GIFunctionInfo *fiTypeFromName = (GIFunctionInfo *)biTypeFromName;


    const char *name = "GdkRGBA";

    GError *error = NULL;
    GIArgument retval;

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

    g_base_info_unref(biRgba);
    g_base_info_unref(biTypeFromName);

    g_printf("GObject.type_from_name(\"%s\"): %ld\n", name, retval.v_ulong);


    /*
    const color0 = new Gdk.RGBA({ red: 0.5, blue: 0.5, green: 0.5, alpha: 0.5 })
    console.log(color0)
    const val = new GObject.Value()
    console.log(val)
    const type = GObject.typeFromName('GdkRGBA')
    console.log(type)
    val.init(GObject.TYPE_STRING)
    val.setBoxed(color0)
    expect(color0.toString(), 'rgba(128,128,128,0.5)')
    const color1 = val.getBoxed()
    expect(color1.toString(), 'rgba(128,128,128,0.5)')
 */
}

int main(void)
{
    GIRepository *repo = g_irepository_get_default();

    gobject_value(repo);
    gdk_rgba(repo);

    return 0;
}
