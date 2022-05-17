## Ubuntu

```
$ make -f Makefile.ubuntu init
$ make -f Makefile.ubuntu compile
$ make -f Makefile.ubuntu run
$ make -f Makefile.ubuntu clean
```

```
$ apt-file search 'gtk/gtk.h'
libgtk-3-dev: /usr/include/gtk-3.0/gtk/gtk.h
libcairo2-dev: /usr/include/cairo/cairo.h
```

## MinGW64

```
$ pacman -S make
$ make -f Makefile.mingw64 init
$ make -f Makefile.mingw64 compile
$ make -f Makefile.mingw64 run
$ make -f Makefile.mingw64 clean
```

* https://www.gtk.org/docs/installations/windows/#using-gtk-from-msys2-packages
* https://blogs.gnome.org/nacho/2014/08/01/how-to-build-your-gtk-application-on-windows
* https://blog.latepaul.com/tag/gtk3/
