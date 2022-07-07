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
        g_error("Require: %s-%s failed with: %s\n", ns, ver, error->message);
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

    // Don't use GValue directly.
    // At the same time is possible to use
    // GValue val = G_VALUE_INIT;
    // if (g_registered_type_info_get_g_type(biValue) == G_TYPE_VALUE) {
    //    size = sizeof(GValue);
    //}
    gsize size = g_struct_info_get_size(biValue);
    gpointer val = g_malloc0(size);

    GIArgument argsInit[] = {
        { .v_pointer = val            },
        { .v_size    = G_TYPE_STRING  },
    };
    if (!g_function_info_invoke(
            fiValueInit,
            argsInit, LENGTH(argsInit),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.init: %ld\n", G_TYPE_STRING);

    const gchar *hello = "Hello";

    GIArgument argsSetString[] = {
        { .v_pointer = val             },
        { .v_pointer = (gpointer)hello },
    };
    if (!g_function_info_invoke(
            fiValueSetString,
            argsSetString, LENGTH(argsSetString),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.setString: %s\n", hello);

    GIArgument argsGetString[] = {
        { .v_pointer = val },
    };
    if (!g_function_info_invoke(
            fiValueGetString,
            argsGetString, LENGTH(argsGetString),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.getString: %s\n", retval.v_string);

    GIArgument argsUnset[] = {
        { .v_pointer = val },
    };
    if (!g_function_info_invoke(
            fiValueUnset,
            argsUnset, LENGTH(argsUnset),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_free(val);
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

    GIFieldInfo *fieldRed = g_struct_info_find_field(biRgba, "red");
    if (!fieldRed) {
        g_error("ERROR: %s\n", "Could not find Gdk.RGBA.read");
        return 1;
    }

    GIFieldInfo *fieldGreen = g_struct_info_find_field(biRgba, "green");
    if (!fieldGreen) {
        g_error("ERROR: %s\n", "Could not find Gdk.RGBA.green");
        return 1;
    }

    GIFieldInfo *fieldBlue = g_struct_info_find_field(biRgba, "blue");
    if (!fieldBlue) {
        g_error("ERROR: %s\n", "Could not find Gdk.RGBA.blue");
        return 1;
    }

    GIFieldInfo *fieldAlpha = g_struct_info_find_field(biRgba, "alpha");
    if (!fieldAlpha) {
        g_error("ERROR: %s\n", "Could not find Gdk.RGBA.alpha");
        return 1;
    }

    GIFunctionInfo *fiRgbaToString = g_struct_info_find_method(biRgba, "to_string");
    if (!fiRgbaToString) {
        g_error("ERROR: %s\n", "Could not find Gdk.RGBA.to_string");
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

    GIBaseInfo *biValue = g_irepository_find_by_name(repo, "GObject", "Value");
    if (!biValue) {
        g_error("ERROR: %s\n", "Could not find GObject.Value");
        return 1;
    }

    GIFunctionInfo *fiValueInit = g_struct_info_find_method(biValue, "init");
    if (!fiValueInit) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.init");
        return 1;
    }

    GIFunctionInfo *fiValueSetBoxed = g_struct_info_find_method(biValue, "set_boxed");
    if (!fiValueSetBoxed) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.set_boxed");
        return 1;
    }

    GIFunctionInfo *fiValueGetBoxed = g_struct_info_find_method(biValue, "get_boxed");
    if (!fiValueGetBoxed) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.get_boxed");
        return 1;
    }

    GIFunctionInfo *fiValueUnset = g_struct_info_find_method(biValue, "unset");
    if (!fiValueUnset) {
        g_error("ERROR: %s\n", "Could not find GObject.Value.unset");
        return 1;
    }

    /*
      https://github.com/ten0s/node-gtk

      const color0 = new Gdk.RGBA({ red: 0.5, blue: 0.5, green: 0.5, alpha: 0.5 })
      const type = GObject.typeFromName('GdkRGBA')

      const val = new GObject.Value()
      val.init(type)
      val.setBoxed(color0)
      const color1 = val.getBoxed()

      color0.toString() => 'rgba(128,128,128,0.5)'
      color1.toString() => 'rgba(128,128,128,0.5)'

      val.unset()
   */

    //
    // const color0 = new Gdk.RGBA({ red: 0.5, blue: 0.5, green: 0.5, alpha: 0.5 })
    //

    gsize szRgba = g_struct_info_get_size(biRgba);
    gpointer color0 = g_malloc0(szRgba);

    g_printf("RGBA color0: %p\n", color0);

    GIArgument arg = {0};
    arg.v_double = 0.5;
    if (!g_field_info_set_field(fieldRed, color0, &arg)) {
        g_error("ERROR: set red field\n");
        return 1;
    }

    if (!g_field_info_set_field(fieldGreen, color0, &arg)) {
        g_error("ERROR: set green field\n");
        return 1;
    }

    if (!g_field_info_set_field(fieldBlue, color0, &arg)) {
        g_error("ERROR: set blue field\n");
        return 1;
    }

    if (!g_field_info_set_field(fieldAlpha, color0, &arg)) {
        g_error("ERROR: set alpha field\n");
        return 1;
    }

    //
    // const type = GObject.typeFromName('GdkRGBA')
    //

    const char *name = "GdkRGBA";

    GError *error = NULL;
    GIArgument retval = {0};

    GIArgument argsTypeFromName[] = {
        { .v_pointer = (gpointer)name },
    };
    if (!g_function_info_invoke(
            fiTypeFromName,
            argsTypeFromName, LENGTH(argsTypeFromName),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    GType type = retval.v_size;
    g_printf("GObject.type_from_name(\"%s\") -> %ld\n", name, type);

    //
    // const val = new GObject.Value()
    //

    gsize szVal = g_struct_info_get_size(biValue);
    gpointer val = g_malloc0(szVal);
    g_printf("new GObject.Value()-> %p\n", val);

    //
    // val.init(type)
    //

    GIArgument argsInit[] = {
        { .v_pointer = val },
        { .v_size    = type },
    };
    if (!g_function_info_invoke(
            fiValueInit,
            argsInit, LENGTH(argsInit),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.init(%p, %ld)\n", val, type);

    //
    // val.setBoxed(color0)
    //

    GIArgument argsSetBoxed[] = {
        { .v_pointer = val    },
        { .v_pointer = color0 },
    };
    if (!g_function_info_invoke(
            fiValueSetBoxed,
            argsSetBoxed, LENGTH(argsSetBoxed),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_printf("GObject.Value.setBoxed(%p, %p)\n", val, color0);

    //
    // const color1 = val.getBoxed()
    //

    GIArgument argsGetBoxed[] = {
        { .v_pointer = val },
    };
    if (!g_function_info_invoke(
            fiValueGetBoxed,
            argsGetBoxed, LENGTH(argsGetBoxed),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    gconstpointer color1 = retval.v_pointer;
    g_printf("GObject.Value.getBoxed(%p) -> %p\n", val, color1);
    g_printf("RGBA color1: %p\n", color1);

    //
    // color0.toString() => 'rgba(128,128,128,0.5)'
    //

    GIArgument argsToString0[] = {
        { .v_pointer = color0 },
    };
    if (!g_function_info_invoke(
            fiRgbaToString,
            argsToString0, LENGTH(argsToString0),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    gchar *str0 = retval.v_pointer;
    g_printf("RGBA.to_string(%p) -> %s\n", color0, str0);
    //g_printf("RGBA str0: %s\n", str0);
    g_free(str0);

    //
    // color1.toString() => 'rgba(128,128,128,0.5)'
    //

    GIArgument argsToString1[] = {
        { .v_pointer = (gpointer)color1 },
    };
    if (!g_function_info_invoke(
            fiRgbaToString,
            argsToString1, LENGTH(argsToString1),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    gchar *str1 = retval.v_pointer;
    g_printf("RGBA.to_string(%p) -> %s\n", color1, str1);
    //g_printf("RGBA str1: %s\n", str1);
    g_free(str1);

    //
    // val.unset()
    //

    GIArgument argsUnset[] = {
        { .v_pointer = val },
    };
    if (!g_function_info_invoke(
            fiValueUnset,
            argsUnset, LENGTH(argsUnset),
            NULL, 0,
            &retval,
            &error)) {
        g_error("ERROR: %s\n", error->message);
        return 1;
    }

    g_print("GObject.Value.unset(%p)\n", val);

    g_free(val);
    g_free(color0);

    g_base_info_unref(fiValueUnset);
    g_base_info_unref(fiValueGetBoxed);
    g_base_info_unref(fiValueSetBoxed);
    g_base_info_unref(fiValueInit);
    g_base_info_unref(biValue);

    g_base_info_unref(biTypeFromName);

    g_base_info_unref(fiRgbaToString);
    g_base_info_unref(fieldAlpha);
    g_base_info_unref(fieldBlue);
    g_base_info_unref(fieldGreen);
    g_base_info_unref(fieldRed);
    g_base_info_unref(biRgba);

    return 0;
}

int main(void)
{
    GIRepository *repo = g_irepository_get_default();

    gobject_value(repo);
    gdk_rgba(repo);

    return 0;
}
