#define _GNU_SOURCE
#define DSQLITE_HAS_CODEC 1
#include "gtk/gtk.h"
#include <sqlcipher/sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define UI_FILE "index.glade"
#define WIDGET_WINDOW "main_w"
GtkWidget *window;
GtkBuilder *builder;
int windows;
typedef struct {
  char *arr;
  size_t size;
} String;

size_t len_str(String *str) { return strlen(str->arr); }
String *stringFrom(char *str) {
  String *s = (String *)calloc(1, sizeof(String));
  s->size = sizeof(char);
  while (s->size < strlen(str)) {
    s->size *= 2;
  }
  s->arr = calloc(s->size, sizeof(char));
  strcpy(s->arr, str);
  return s;
}
// void str_push(String* str, char element){
//     if (str->size==len_str(str)){
//         str->arr=realloc(str->arr, 2*str->size);
//         str->size*=2;
//     }
//     *(str->arr+len_str(str))=element;
// }
void str_append(String *str, char *s) {
  size_t len = strlen(s);
  size_t endlen = len + len_str(str);
  while (str->size <= endlen || len > str->size) {
    str->arr = realloc(str->arr, 2 * str->size);
    str->size *= 2;
  }
  strcat(str->arr, s);
}
typedef int (*sqlite3_callback)(
    void *,  /* Data provided in the 4th argument of sqlite3_exec() */
    int,     /* The number of columns in row */
    char **, /* An array of strings representing fields in the row */
    char **  /* An array of strings representing column names */
);
static int callback(void *data, int argc, char **argv, char **azColName) {
  GtkWidget *loc = GTK_WIDGET(gtk_builder_get_object(builder, "loc"));
  GtkWidget *mail = GTK_WIDGET(gtk_builder_get_object(builder, "mail"));
  GtkWidget *pass = GTK_WIDGET(gtk_builder_get_object(builder, "pass"));
  gtk_entry_set_text(GTK_ENTRY(loc), argv[0] ? argv[0] : "NULL");
  gtk_entry_set_text(GTK_ENTRY(mail), argv[1] ? argv[1] : "NULL");
  gtk_entry_set_text(GTK_ENTRY(pass), argv[2] ? argv[2] : "NULL");
  return 0;
}
static int callback2(void *data, int argc, char **argv, char **azColName) {
  GtkWidget *loc = GTK_WIDGET(gtk_builder_get_object(builder, "loc"));
  GtkWidget *mail = GTK_WIDGET(gtk_builder_get_object(builder, "mail"));
  GtkWidget *pass = GTK_WIDGET(gtk_builder_get_object(builder, "pass"));
  gtk_entry_set_text(GTK_ENTRY(loc), argv[0] ? argv[0] : "NULL");
  gtk_entry_set_text(GTK_ENTRY(mail), argv[1] ? argv[1] : "NULL");
  gtk_entry_set_text(GTK_ENTRY(pass), argv[2] ? argv[2] : "NULL");
  return 0;
}
void getPermission(GtkWidget *widget, gpointer data) {
  char *cmd;
  const char *password = gtk_entry_get_text(
      GTK_ENTRY(gtk_builder_get_object(builder, "password")));
  asprintf(&cmd, "echo %s | sudo -S echo", password);
  int status = system(cmd);
  if (status != 0) {
    GtkWidget *passfail = GTK_WIDGET(gtk_builder_get_object(builder, "wrongw"));
    gtk_widget_show(passfail);
    return;
  }
  GtkWidget *passwin =
      GTK_WIDGET(gtk_builder_get_object(builder, "passwindow"));
  gtk_widget_show(passwin);
  windows += 1;
  gtk_widget_destroy(window);
}
void getstf(GtkWidget *widget, gpointer data) {
  sqlite3 *db;
  int rc;
  char *zErrMsg = 0;
  String *sql = stringFrom("SELECT * FROM Passwords WHERE location='");
  str_append(sql, gtk_entry_get_text(
                      GTK_ENTRY(gtk_builder_get_object(builder, "loc"))));
  str_append(sql, "'");
  // printf("%s\n", sql->arr);
  rc = sqlite3_open("encrypted.db", &db);
  // LIFE'S TOO SHORT TO COMPILE SQLCIPHER MYSELF. JUST USE system("sqlite3 -cmd
  // '<command>'")
  rc = sqlite3_key(db, "testkey", 7);
  if (rc) {
    // fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return;
  } else {
    // fprintf(stdout, "Opened database successfully\n");
  }
  sqlite3_exec(db, sql->arr, callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    // fprintf(stdout, "Operation done successfully\n");
  }
  sqlite3_close(db);
  free(sql->arr);
  free(sql);
}
void addstf(GtkWidget *widget, gpointer data) {
  sqlite3 *db;
  int rc;
  char *zErrMsg = 0;
  GtkWidget *loc = GTK_WIDGET(gtk_builder_get_object(builder, "loc"));
  GtkWidget *mail = GTK_WIDGET(gtk_builder_get_object(builder, "mail"));
  GtkWidget *pass = GTK_WIDGET(gtk_builder_get_object(builder, "pass"));
  char* locstr = gtk_entry_get_text(GTK_ENTRY(loc));
  char* mailstr = gtk_entry_get_text(GTK_ENTRY(mail));
  char* passstr = gtk_entry_get_text(GTK_ENTRY(pass));
  char *sql;
  asprintf(&sql,
           "INSERT INTO Passwords (location, email, password) VALUES ('%s', "
           "'%s', '%s')",
           locstr, mailstr, passstr);
  rc = sqlite3_open("encrypted.db", &db);
  rc = sqlite3_key(db, "testkey", 7);
  if (rc) {
    // fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return;
  } else {
    // fprintf(stdout, "Opened database successfully\n");
  }
  sqlite3_exec(db, sql, callback2, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    // fprintf(stdout, "Operation done successfully\n");
  }
  sqlite3_close(db);
  free(sql);
}
void quitButtonClicked(GtkWidget *widget, gpointer data) {
  windows -= 1;
  if (windows == 0) {
    gtk_main_quit();
    return;
  }
}
int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, UI_FILE, NULL);

  window = GTK_WIDGET(gtk_builder_get_object(builder, WIDGET_WINDOW));
  gtk_builder_connect_signals(builder, NULL);

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(cssProvider, "style.css", NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);
  windows = 1;
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
