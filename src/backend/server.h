#include <ctype.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

typedef struct
{
    char *key;
    char *value;
}
dict;

void accept_request(int client_sock_fd, SSL_CTX *sslctx);
void parse_request(SSL *client_ssl);
void route_get(SSL *client_ssl, char *uri);
void route_post(SSL *client_ssl, char *uri, dict *body);
char *get_value_by_key(dict *dictionary, const char* name);
void url_decode(char *src);

void initialize_ssl();
void destroy_ssl();
void shutdown_ssl(SSL *client_ssl);

void start_server(int *server_sock_fd, struct sockaddr_in *address, socklen_t *addrlen)
{
    int opt = 1;

    if((*server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Server socket failure");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(*server_sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failure");
        exit(EXIT_FAILURE);
    }

    address -> sin_family = AF_INET;
    address -> sin_addr.s_addr = INADDR_ANY;
    address -> sin_port = htons(PORT);

    if(bind(*server_sock_fd, (struct sockaddr*)address, sizeof(*address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(*server_sock_fd, 100) < 0) {
        perror("Listen failure");
        exit(EXIT_FAILURE);
    }

    initialize_ssl();
}

void constantly_accept_requests(int server_sock_fd, struct sockaddr_in address, socklen_t addrlen)
{
    SSL_CTX *sslctx;
    int client_sock_fd, use_cert, use_prv;

    sslctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);

    use_cert = SSL_CTX_use_certificate_file(sslctx, "cert.pem" , SSL_FILETYPE_PEM);
    use_prv = SSL_CTX_use_PrivateKey_file(sslctx, "key.pem", SSL_FILETYPE_PEM);

    if(use_cert < 1 || use_prv < 1)
    {
        perror("Certificate error");
        exit(EXIT_FAILURE);
    }

    printf("Server started %shttps://127.0.0.1:%d%s\n\n", "\033[92m", PORT, "\033[0m");

    while(1)
    {
        if((client_sock_fd = accept(server_sock_fd, (struct sockaddr*)&address, &addrlen)) < 0)
        {
            perror("Accept failure");
            exit(EXIT_FAILURE);
        }

        if(fork() == 0)
        {
            accept_request(client_sock_fd, sslctx);
            exit(0);
        }

    }
}

void accept_request(int client_sock_fd, SSL_CTX *sslctx)
{
    SSL *client_ssl;
    int ssl_err;

    client_ssl = SSL_new(sslctx);
    SSL_set_fd(client_ssl, client_sock_fd);
    ssl_err = SSL_accept(client_ssl);

    if(ssl_err <= 0)
    {
        printf("SSL failed %d\n", SSL_get_error(client_ssl, ssl_err));
    }
    else
    {
        parse_request(client_ssl);
    }

    shutdown_ssl(client_ssl);
    close(client_sock_fd);
}

void parse_request(SSL *client_ssl)
{
    int result, ssl_err, b, payload_size;
    char buffer[2048], *method, *uri, *prot, *temp_ptr, *key_ptr, *value_ptr, *payload_ptr;
    static dict req_headers[30] = {};
    static dict body[20] = {};

    for(int i = 0; i < 30; ++i)
    {
        req_headers[i].key = 0;
        req_headers[i].value = 0;
    }

    for(int i = 0; i < 20; ++i)
    {
        body[i].key = 0;
        body[i].value = 0;
    }

    result = SSL_read(client_ssl, buffer, sizeof(buffer) - 1);

    if(result < 0)
    {
        ssl_err = SSL_get_error(client_ssl, result);
        printf("SSL failed %d\n", ssl_err);
        return;
    }
    else if(result == 0)
    {
        printf("Connection closed\n");
        return;
    }

    method = strtok(buffer, " \t\r\n");
    uri = strtok(NULL, " \t");
    prot = strtok(NULL, " \t\r\n");

    if(!method || !uri || !prot)
    {
        printf("Invalid request\n");
        return;
    }

    printf("%s%s %s%s\n", "\033[92m", method, "\033[0m", uri);

    for(int i = 0; i < 30; ++i)
    {
        key_ptr = strtok(NULL, "\r\n: \t");
        if(!key_ptr)
        {
             break;
        }

        value_ptr = strtok(NULL, "\r\n");
        while(*value_ptr && *value_ptr == ' ')
        {
            value_ptr++;
        }

        req_headers[i].key  = key_ptr;
        req_headers[i].value = value_ptr;
        printf("[H] %s: %s\n", key_ptr, value_ptr);

        temp_ptr = value_ptr + strlen(value_ptr) + 1;

        if(temp_ptr[0] == '\n' && temp_ptr[1] == '\r' && temp_ptr[2] == '\n')
        {
            break;
        }
    }

    if(!strcmp(method, "GET"))
    {
        route_get(client_ssl, uri);
    }
    else if(!strcmp(method, "POST"))
    {
        payload_ptr = strtok(NULL, "\n") + 2;

        payload_size = atoi(get_value_by_key(req_headers, "Content-Length"));
        payload_ptr[payload_size] = 0;

        for(int i = 0; i < 20; ++i)
        {
            key_ptr = strtok(NULL, "=");
            if(!key_ptr || payload_ptr + payload_size <= key_ptr)
            {
                break;
            }
            body[i].key = key_ptr;

            value_ptr = strtok(NULL, "&\n\0");
            if(!value_ptr)
            {
                break;
            }
            url_decode(value_ptr);
            body[i].value = value_ptr;

            printf("[B] %s: %s\n", key_ptr, get_value_by_key(body, key_ptr));
        }

        route_post(client_ssl, uri, body);
    }
    printf("\n");
}

char *get_value_by_key(dict *dictionary, const char* target_key)
{
    dict *pair = dictionary;

    while(pair->key)
    {
        if(strcmp(pair->key, target_key) == 0)
        {
            return pair->value;
        }
        pair++;
    }

    return NULL;
}

void send_file(SSL *client_ssl, char *file_name)
{
    int file_buff_size = 16384;
    char file_buff[file_buff_size];
    FILE *file_ptr;
    size_t bytes_read;

    if((file_ptr = fopen(file_name, "rb")) == NULL)
    {
        perror("Cant open file");
        exit(EXIT_FAILURE);
    }

    while((bytes_read = fread(file_buff, 1, file_buff_size, file_ptr )) > 0)
    {
        SSL_write(client_ssl, file_buff, bytes_read);
    }

    fclose(file_ptr);
}

void initialize_ssl()
{
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void destroy_ssl()
{
    ERR_free_strings();
    EVP_cleanup();
}

void shutdown_ssl(SSL *client_ssl)
{
    SSL_shutdown(client_ssl);
    SSL_free(client_ssl);
}

void url_decode(char *src)
{
    char a, b, *dst = src;

    while(*src)
    {
        if((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b)))
        {
            if('a' <= a)
            {
                a -= 'a' - 'A';
            }
            if('A' <= a)
            {
                a -= 'A' - 10;
            }
            else
            {
                a -= '0';
            }

            if('a' <= b)
            {
                b -= 'a'-'A';
            }
            if('A' <= b)
            {
                b -= ('A' - 10);
            }
            else
            {
                b -= '0';
            }
            // Writing a hex value of char to destination
            *dst++ = 16*a + b;
            src += 3;
        }
        else if(*src == '+')
        {
            *dst++ = ' ';
            ++src;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}
