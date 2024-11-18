#include <sqlite3.h>
#include "server.h"

int main()
{
    serve_forever("5000");
    return 0;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

void route()
{
    if(0 == strcmp(uri,(char*)"/") && 0 == strcmp(method,(char*)"GET"))
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        /*printf("<!DOCTYPE html><html><head><title>Page Title</title><style>body {");*/
        /*printf("background-color: powderblue;}h1 {color: red;}p {color: blue;}</style></head>  <body><h1>This is a Heading</h1><p>This is a paragraph.</p><p>The content of the body element is displayed in the browser window.</p><p>The content of the title element is displayed in the browser tab, in favorites and in search-engine results.</p></body></html>");*/
    }
    else if(0 == strcmp(uri,(char*)"/create-post") && 0 == strcmp(method,(char*)"POST"))
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        sqlite3 *db;

        sqlite3_open("db.db", &db);

        char *errorMessage = 0;
        char sql[256];
        char name[] = "Lukas";

        sprintf(sql, "INSERT INTO Friends(Name) VALUES('%s');", name);

        sqlite3_exec(db, sql, callback, 0, &errorMessage);

        sqlite3_close(db);
    }
    else // Not found
    {
        printf("HTTP/1.1 404 NOT_FOUND\r\n\r\n");
        printf("<!DOCTYPE html><html><body><h1>404</h1><p>Not found.</p></body></html> ");
    }
}

FILE *fptr;
int file_write(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        fprintf(fptr, "<div>%s = %s</div>", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    return 0;
}

void generate_main_html()
{
    fptr = fopen("./src/frontend/index.html", "w");
    sqlite3 *db;

    sqlite3_open("db.db", &db);

    char *errorMessage = 0;
    char *sql = "SELECT * FROM Friends ORDER BY Id";
    sqlite3_exec(db, sql, file_write, 0, &errorMessage);

    fclose(fptr); 
}
