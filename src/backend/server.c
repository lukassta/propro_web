#include "server.h"

int main()
{
    serve_forever("5000");
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
    }
    else // Not found
    {
        printf("HTTP/1.1 404 NOT_FOUND\r\n\r\n");
        printf("<!DOCTYPE html><html><body><h1>404</h1><p>Not found.</p></body></html> ");
    }
}

void generate_main_html()
{
    FILE *fptr;

    fptr = fopen("test.txt", "w");

    fprintf(fptr, "QS: %s", qs);

    fclose(fptr); 
}
