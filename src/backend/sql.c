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

int getLatestEntryId(sqlite3 *db){
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
    }
    return latestId;
}

int insertData(sqlite3 *db, char *teamName, char *title, char *code, char *description) {
    char *errorMessage = 0;
    char sql[1024];

    sprintf(sql, "INSERT INTO Entries(TeamName, Title, Code, Description) VALUES('%s', '%s', '%s', '%s');", teamName, title, code, description);
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
    char *sql = "SELECT * FROM Entries ORDER BY Id";
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

    char *sql = "CREATE TABLE IF NOT EXISTS Entries(Id INTEGER PRIMARY KEY, TeamName TEXT, Title TEXT, Code TEXT, Description TEXT, Likes INTEGER DEFAULT 0, Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
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
        if (argc != 6) {
            printf("usage: specific -i <TeamName> <Title> <Code> <Description>\n");
            return 1;
        }
        printf("insert data\n");
        insertData(db, argv[2], argv[3], argv[4], argv[5]);
    }

    if ((argc > 1) && (strcmp(argv[1], "-l") == 0)) {
        printf("Latest id: %d\n", getLatestEntryId(db));
    }

    sqlite3_close(db);

    return 0;
}
