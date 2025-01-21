#include "headers.h"

const char DATABASE[] = "src/backend/snipper.db";

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

    sqlite3_exec(db, sql, render_snippet_element, &pointers, &errorMessage);

    send_file(client_ssl, "./src/frontend/snippets_end.html");


    sqlite3_close(db);

    fclose(snippet_template_ptr);
}


int render_snippet_element(void *pointers_ptr, int argc, char **argv, char **azColName)
{
    char element[2048], buffer[2048];
    struct TwoPointers *pointers;
    SSL *client_ssl;
    FILE *snippet_template_ptr;

    pointers = pointers_ptr;
    client_ssl = pointers->pointer_1;
    snippet_template_ptr = pointers->pointer_2;

    fread(buffer, 1, 2048, snippet_template_ptr);

    sprintf(element, buffer, argv[1], argv[2], argv[4], argv[3], argv[5]);

    SSL_write(client_ssl, element, strlen(element));

    return 0;
}


void render_admin_snippets_page(SSL *client_ssl)
{
    char *sql = "SELECT * FROM Entries ORDER BY Id", *errorMessage = 0;
    sqlite3 *db;
    FILE *snippet_template_ptr;
    struct TwoPointers pointers;

    pointers.pointer_1 = client_ssl;
    snippet_template_ptr = fopen("./src/frontend/admin_snippet_template.html", "rb");
    pointers.pointer_2 = snippet_template_ptr;

    sqlite3_open(DATABASE, &db);


    send_file(client_ssl, "./src/frontend/snippets_beg.html");

    sqlite3_exec(db, sql, render_admin_snippet_element, &pointers, &errorMessage);

    send_file(client_ssl, "./src/frontend/snippets_end.html");


    sqlite3_close(db);

    fclose(snippet_template_ptr);
}


int render_admin_snippet_element(void *pointers_ptr, int argc, char **argv, char **azColName)
{
    char element[2048], buffer[2048];
    struct TwoPointers *pointers;
    SSL *client_ssl;
    FILE *snippet_template_ptr;

    pointers = pointers_ptr;
    client_ssl = pointers->pointer_1;
    snippet_template_ptr = pointers->pointer_2;

    fread(buffer, 1, 2048, snippet_template_ptr);

    sprintf(element, buffer, argv[1], argv[2], argv[4], argv[3], argv[5], argv[0]);

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


int create_snippet(char team_name[], char title[], char language[], char code[], char description[])
{
    char *errorMessage = 0;
    char sql[2048];
    sqlite3 *db;

    sqlite3_open(DATABASE, &db);

    sprintf(sql, "INSERT INTO Entries(TeamName, Title, Language, Code, Description) VALUES('%s', '%s', '%s', '%s', '%s');", team_name, title, language, code, description);

    int resultCode = sqlite3_exec(db, sql, do_nothing, 0, &errorMessage);

    if(resultCode != SQLITE_OK)
    {
        return 1;
    }

    sqlite3_close(db);

    return 0;
}


int delete_snippet(int snippet_id)
{
    int result_code;
    char sql[256], *error_message;
    sqlite3 *db;

    sqlite3_open(DATABASE, &db);

    sprintf(sql, "DELETE FROM Entries WHERE Id = '%d';", snippet_id);

    result_code = sqlite3_exec(db, sql, do_nothing, 0, &error_message);

    if(result_code != SQLITE_OK)
    {
        fprintf(stderr, "Failed to delete entry: %s\n", sqlite3_errmsg(db));

        sqlite3_free(error_message);
        sqlite3_close(db);

        return 1;
    }

    sqlite3_close(db);

    return 0;
}


int do_nothing(void *NotUsed, int argc, char **argv, char **azColName)
{
    return 0;
}
