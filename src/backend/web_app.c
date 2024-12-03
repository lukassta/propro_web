#include "server.h"
#include <openssl/ssl.h>
#include <sqlite3.h>
#include <stdio.h>

void initiate_db();

void initiate_server(int *server_sock_fd, struct sockaddr_in *address, socklen_t *addrlen);

int callback(void *NotUsed, int argc, char **argv, char **azColName);

int getLatestId(sqlite3 *db);

void deleteEntryById(sqlite3 *db, int idToDelete);

void render_snippets_page(SSL *client_ssl);

int write_snippet_element(void *client_ssl_p, int argc, char **argv, char **azColName);

const char DATABASE[] = "src/backend/snipper.db";

struct TwoPointers
{
    void *pointer_1;
    void *pointer_2;
};

int main()
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int server_sock_fd;

    initiate_db();
    start_server(&server_sock_fd, &address, &addrlen);

    constantly_accept_requests(server_sock_fd, address, addrlen);

    return 0;
}

void route_get(SSL *client_ssl, char *uri)
{
    if(!strcmp(uri, "/favicon.png"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:image/png\r\n\r\n", 42);

        send_file(client_ssl, "./public/favicon.png");
    }
    else if(!strcmp(uri, "/snipper_logo.png"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./public/snipper_logo.png");
    }
    else if(!strcmp(uri, "/styles.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/styles.css");
    }
    else if(!strcmp(uri, "/index.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/index.css");
    }
    else if(!strcmp(uri, "/upload.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/upload.css");
    }
    else if(!strcmp(uri, "/snippets.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/snippets.css");
    }
    else if(!strcmp(uri, "/"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/index.html");
    }
    else if(!strcmp(uri, "/upload"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/upload.html");
    }
    else if(!strcmp(uri, "/snippets"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        render_snippets_page(client_ssl);
    }
    else if(!strcmp(uri, "/about"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/about.html");
    }
    else // Not found
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        SSL_write(client_ssl, "<h1>404</h1>\n", 14);
    }
}

void bad_request_body(SSL *client_ssl)
{
    SSL_write(client_ssl, "HTTP/1.1 400 Bad request\r\n\r\n", 28);
}

void route_post(SSL *client_ssl, char *uri, dict *body)
{
    if(!strcmp(uri, "/submit-snippet"))
    {
        sqlite3 *db;

        sqlite3_open(DATABASE, &db);

        char *errorMessage = 0;
        char sql[2048];

        char *team_name = get_value_by_key(body, "team_name");
        char *title = get_value_by_key(body, "title");
        char *language = get_value_by_key(body, "language");
        char *code = get_value_by_key(body, "code");
        char *description = get_value_by_key(body, "description");

        if(!team_name || !title || !language || !code || !description)
        {
            bad_request_body(client_ssl);
            return;
        }

        sprintf(sql, "INSERT INTO Entries(TeamName, Title, Language, Code, Description) VALUES('%s', '%s', '%s', '%s', '%s');", team_name, title, language, code, description);

        int resultCode = sqlite3_exec(db, sql, callback, 0, &errorMessage);

        if(resultCode != SQLITE_OK)
        {
            sqlite3_close(db);
            perror("Cant open database");
            exit(EXIT_FAILURE);
        }

        sqlite3_close(db);

        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/index.html");
    }
    else
    {
        SSL_write(client_ssl, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
    }
}

void initiate_db()
{
    sqlite3 *db;
    char *errorMessage = 0;

    int resultCode = sqlite3_open(DATABASE, &db);

    if(resultCode != SQLITE_OK)
    {
        sqlite3_close(db);
        perror("Cant open database");
        exit(EXIT_FAILURE);
    }

    char *sql = "CREATE TABLE IF NOT EXISTS Entries(Id INTEGER PRIMARY KEY, TeamName TEXT, Title TEXT, Language TEXT, Code TEXT, Description TEXT, Likes INTEGER DEFAULT 0, Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    resultCode = sqlite3_exec(db, sql, 0, 0, &errorMessage);

    if (resultCode != SQLITE_OK) {
        perror("SQL error");
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    sqlite3_close(db);
}

void render_snippets_page(SSL *client_ssl)
{
    char *sql = "SELECT * FROM Entries ORDER BY Id", *errorMessage = 0;
    sqlite3 *db;
    FILE *snippet_template_ptr;
    struct TwoPointers pointers;

    pointers.pointer_1 = client_ssl;
    snippet_template_ptr = fopen("./src/frontend/snippet_template.html", "rb");
    pointers.pointer_2 = snippet_template_ptr;

    sqlite3_open(DATABASE, &db);

    send_file(client_ssl, "./src/frontend/snippets_beg.html");

    sqlite3_exec(db, sql, write_snippet_element, &pointers, &errorMessage);

    send_file(client_ssl, "./src/frontend/snippets_end.html");

    sqlite3_close(db);

    fclose(snippet_template_ptr);
}

int write_snippet_element(void *pointers_ptr, int argc, char **argv, char **azColName)
{
    char element[2048], buffer[2048];
    struct TwoPointers *pointers;
    SSL *client_ssl;
    FILE *snippet_template_ptr;

    pointers = pointers_ptr;
    client_ssl = pointers->pointer_1;
    snippet_template_ptr = pointers->pointer_2;

    fread(buffer, 1, 2048, snippet_template_ptr);

    sprintf(element, buffer, argv[2], argv[5], argv[4], argv[3], argv[1]);

    SSL_write(client_ssl, element, strlen(element));

    return 0;
}

int getLatestEntryId(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Id FROM Entries ORDER BY Id DESC LIMIT 1;";
    int latestId = 0;

    int resultCode = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    resultCode = sqlite3_step(stmt);
    if (resultCode == SQLITE_ROW) {
        latestId = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "No entries found in the table.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_close(db);

    return latestId;
}

void deleteEntryById(sqlite3 *db, int idToDelete)
{
    char *error_message = 0;
    char sql[256];

    sprintf(sql, "DELETE FROM Entries WHERE Id = '%d';", idToDelete);

    int resultCode = sqlite3_exec(db, sql, callback, 0, &error_message);

    if (resultCode != SQLITE_OK){
        fprintf(stderr, "Failed to delete entry: %s\n", sqlite3_errmsg(db));
        sqlite3_free(error_message);
        sqlite3_close(db);
    }
}

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    return 0;
}
