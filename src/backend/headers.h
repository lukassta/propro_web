#ifndef HEADERS_H
#define HEADERS_H

#include <ctype.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct
{
    char *key;
    char *value;
}
dict;

struct TwoPointers
{
    void *pointer_1;
    void *pointer_2;
};

void route_get(SSL *client_ssl, char *uri);
void route_post(SSL *client_ssl, char *uri, dict *body);
void bad_request_body(SSL *client_ssl);

void start_server(int *server_sock_fd, struct sockaddr_in *address, socklen_t *addrlen);
void constantly_accept_requests(int server_sock_fd, struct sockaddr_in address, socklen_t addrlen);
void accept_request(int client_sock_fd, SSL_CTX *sslctx);
void parse_request(SSL *client_ssl);
char *get_value_by_key(dict *dictionary, const char* name);
void send_file(SSL *client_ssl, char *file_name);
void initialize_ssl();
void shutdown_ssl(SSL *client_ssl);
void destroy_ssl();
void url_decode(char *src);

void initiate_db();
int getLatestId(sqlite3 *db);
int create_snippet(char team_name[], char title[], char language[], char code[], char description[]);
int delete_snippet(int snippet_id);
int do_nothing(void *NotUsed, int argc, char **argv, char **azColName);
void render_snippets_page(SSL *client_ssl);
void render_admin_snippets_page(SSL *client_ssl);
int render_snippet_element(void *client_ssl_p, int argc, char **argv, char **azColName);
int render_admin_snippet_element(void *client_ssl_p, int argc, char **argv, char **azColName);

#endif
