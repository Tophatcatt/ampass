Password manager made with C, Gtk3 and sqlite3.

before compiling be sure to use sqlcipher to create a new encrypted database and change the key in the file.

here is the compilation script

gcc -rdynamic `pkg-config --cflags gtk+-3.0` file -o fileBasenameNoExtension.out `pkg-config --libs gtk+-3.0` -lsqlcipher && ./fileBasenameNoExtension.out


Just a tiny project
