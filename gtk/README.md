$ apt-file search 'gtk/gtk.h'
libgtk-3-dev: /usr/include/gtk-3.0/gtk/gtk.h
libcairo2-dev: /usr/include/cairo/cairo.h

$ sudo apt install libgtk-3-dev libcairo2-dev

$ gcc -o hello-gtk hello-gtk.c `pkg-config --libs --cflags gtk+-3.0`
$ ./hello-gtk

$ gcc -o hello-gtk-cairo hello-gtk-cairo.c `pkg-config --libs --cflags gtk+-3.0`
$ ./hello-gtk-cairo

