#include "server.h"
#include <sqlite3.h>
#include <stdio.h>

void initiate_db();

void initiate_server(int *server_sock_fd, struct sockaddr_in *address, socklen_t *addrlen);

void send_file(int client_sock_fd, char *file_name);

void render_index_html();

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    return 0;
}

int main()
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int server_sock_fd;

    initiate_db();
    start_server(&server_sock_fd, &address, &addrlen);

    accept_requests_server(server_sock_fd, address, addrlen);
    return 0;
}

void route_get(int client_sock_fd, char *uri)
{
    if(!strcmp(uri, "/favicon.png"))
    {
        send(client_sock_fd, "HTTP/1.1 200 OK\nContent-Type:image/png\n\n", 40, 0);
        char file_name[] = "./public/favicon.png";
        send_file(client_sock_fd, file_name);
    }
    else
    {
        send(client_sock_fd, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n", 42, 0);

        send(client_sock_fd, "<h1>BANANA</h1>\n", 14, 0);
        send_file(client_sock_fd, "./src/frontend/test.html");
    }
}

void route_post(int client_sock_fd, char *uri)
{
    if(!strcmp(uri, "/create-post"))
    {
        sqlite3 *db;

        sqlite3_open("snipper.db", &db);

        char *errorMessage = 0;
        char sql[256];
        char teamName[] = "Snipper";
        char title[] = "Test code";
        char code[] = "<div>a</div>";
        char description[] = "Sitas kodas...";

        sprintf(sql, "INSERT INTO Entries(TeamName, Title, Code, Description) VALUES('%s', '%s', '%s', '%s');", teamName, title, code, description);

        int resultCode = sqlite3_exec(db, sql, callback, 0, &errorMessage);

        if(resultCode != SQLITE_OK)
        {
            sqlite3_close(db);
            perror("Cant open database");
            exit(EXIT_FAILURE);
        }

        sqlite3_close(db);

        send(client_sock_fd, "HTTP/1.1 204 No content\n\n", 25, 0);

        render_index_html();
    }
    else
    {
        send(client_sock_fd, "HTTP/1.1 404 Not Found\n\n", 24, 0);
    }
}

void initiate_db()
{
    sqlite3 *db;
    char *errorMessage = 0;

    int resultCode = sqlite3_open("snipper.db", &db);

    if(resultCode != SQLITE_OK)
    {
        sqlite3_close(db);
        perror("Cant open database");
        exit(EXIT_FAILURE);
    }

    char *sql = "CREATE TABLE IF NOT EXISTS Entries(Id INTEGER PRIMARY KEY, TeamName TEXT, Title TEXT, Code TEXT, Description TEXT, Likes INTEGER DEFAULT 0, Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    resultCode = sqlite3_exec(db, sql, 0, 0, &errorMessage);

    if (resultCode != SQLITE_OK) {
        perror("SQL error");
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    sqlite3_close(db);
}

FILE *fptr;
int file_write(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        fprintf(fptr, "<div>%s</div>\n", argv[i]);
    }

    return 0;
}

void render_index_html()
{
    fptr = fopen("./src/frontend/index.html", "w");
    sqlite3 *db;

    sqlite3_open("snipper.db", &db);

    char *errorMessage = 0;
    char *sql = "SELECT * FROM Entries ORDER BY Id";
    sqlite3_exec(db, sql, file_write, 0, &errorMessage);

    fclose(fptr); 

    sqlite3_close(db);
}
