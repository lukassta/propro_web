#include <sqlite3.h> 
#include <stdio.h>
#include <string.h>

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int insertData(sqlite3 *db, char *name) {
    char *errorMessage = 0;
    char sql[256];

    sprintf(sql, "INSERT INTO Friends(Name) VALUES('%s');", name);

    int resultCode = sqlite3_exec(db, sql, callback, 0, &errorMessage);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
        return 1;
    }

    return 0;
}

int selectTable(sqlite3 *db) {
    char *errorMessage = 0;
    char *sql = "SELECT * FROM Friends ORDER BY Id";
    int resultCode = sqlite3_exec(db, sql, callback, 0, &errorMessage);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    sqlite3 *db;
    char *errorMessage = 0;

    int resultCode = sqlite3_open("db.db", &db);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS Friends(Id INTEGER PRIMARY KEY, NAME TEXT);";
    resultCode = sqlite3_exec(db, sql, 0, 0, &errorMessage);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return 1;
    }

    if ((argc > 1) && (strcmp(argv[1], "-s") == 0)) {
        selectTable(db);
    }

    if ((argc > 1) && (strcmp(argv[1], "-i") == 0)) {
        if (argc == 2) {
            printf("usage: specific -i <name>\n");
            return 1;
        }
        printf("insert data\n");
        insertData(db, argv[2]);
    }

    sqlite3_close(db);

    return 0;
}
