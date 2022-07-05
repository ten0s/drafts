#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

int main(int argc, char *argv[])
{
    char data[] = "hello";

    // $ echo -en 'hello\0' | base64
    // aGVsbG8A
    gchar* encoded = g_base64_encode(data, LENGTH(data));
    g_printf("data    : %s\n"
             "base64  : %s\n"
             "expected: %s\n", data, encoded, "aGVsbG8A");
    g_free(encoded);

    return 0;
}
