
## Ubuntu

```
$ make -f Makefile.linux init
$ make -f Makefile.linux compile
$ make -f Makefile.linux run
$ make -f Makefile.linux clean
```

```
$ apt-file search 'gtk/gtk.h'
libgtk-3-dev: /usr/include/gtk-3.0/gtk/gtk.h
libcairo2-dev: /usr/include/cairo/cairo.h
```

## MinGW64

```
$ make -f Makefile.mingw64 init
$ make -f Makefile.mingw64 compile
$ make -f Makefile.mingw64 run
$ make -f Makefile.mingw64 clean
```