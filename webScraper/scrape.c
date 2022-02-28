#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <errno.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

//check certs. not used, might be later
void showCerts(SSL *ssl) {
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    } else {
        printf("Info: No client certificates configured.\n");
    }
}

//open raw connection to web server
int openConnection(char *hostdomain, int port) {
    int sock;

    //get host domain name and convert to ipv4 (usually)
    struct hostent *host;
    struct sockaddr_in addr;

    if ((host = gethostbyname(hostdomain)) == NULL) {
        perror("gethostbyname");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    
    //socket struct
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);

    //make the connection
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    
    return sock;
}

int main() {
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char *serverResp = malloc(20000000 * sizeof(char)); //20mb
    char *serverRespPtr;
    char buf[1024];
    int bytes = 1;
    int totalBytes = 0;
    char acClientRequest[1024];
    char *portnum = "443";
    char *findArticle = malloc(2000000 * sizeof(char)); //2mb
    char *findArticle2 = malloc(2000000 * sizeof(char)); //2mb
    char *findArticlePtr;
    char *findArticle2Ptr;
    char *endArticleEnd;
    char *findBreaking;
    char *endTitleEnd;
    char *endParagraphEnd;
    char *title = malloc(2000 * sizeof(char)); //2kb
    char *paragraphs = malloc(20000 * sizeof(char)); //20kb

    char *fullArticle = malloc(2000000 * sizeof(char)); //2mb

    char site[256];
    char page[512];


    printf("enter the site to scrape from (eg: www.bbc.com): ");
    scanf("%s", site);
    printf("enter the page to scrape (eg: /news/live): ");
    scanf("%s", page);

    
    while (1) {
        //initialize ssl library
        SSL_library_init();

        //setup all ssl vars we need
        const SSL_METHOD *method;

        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        method = SSLv23_client_method();
        ctx = SSL_CTX_new(method);
        if (ctx == NULL) {
            ERR_print_errors_fp(stderr);
            abort();
        }

        //open the connection and switch from raw socket to ssl socket
        server = openConnection(site, atoi(portnum));
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, server);

        //confirm the connection
        if (SSL_connect(ssl) == -1) {
            ERR_print_errors_fp(stderr);
        } else {
            //ignore
            /*printf("\n\nConnected with %s encryption\n", SSL_get_cipher(ssl));
            showCerts(ssl);
            //SSL_write(ssl, acClientRequest, strlen(acClientRequest));
            bytes = SSL_read(ssl, buf, sizeof(buf));
            buf[bytes] = 0;
            printf("Received: \"%s\"\n", buf);*/


            //send get request to http server through SSL_write
            sprintf(acClientRequest, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", page, site);
            SSL_write(ssl, acClientRequest, strlen(acClientRequest));
            bytes = SSL_read(ssl, buf, sizeof(buf)-64);

            //get response back from server and put into serverResp buffer
            while (bytes > 0) {
                if (totalBytes > 19999999) continue;
                strcat(serverResp, buf);
                memset(buf, '\0', strlen(buf));
                bytes = SSL_read(ssl, buf, sizeof(buf)-64); //-64 so it doesnt send back my http request (which is 64 bytes)
                totalBytes += bytes;
            }

            //close connection
            SSL_free(ssl);
        }

        //find latest breaking news article then get title and paragraphs from it
        serverRespPtr = serverResp;
        while ((serverRespPtr = strstr(serverRespPtr, "<article")) != NULL) {
            //find end of article start tag
            endArticleEnd = strchr(serverRespPtr, '>');
            if (!endArticleEnd) {
		printf("endArticleEnd error\n");
		break;
	    }
            endArticleEnd++;

            //find end of article tag
            serverRespPtr = strstr(endArticleEnd, "</article>");
            if (!serverRespPtr) {
                //fwrite(endSpanEnd, 1, strlen(endSpanEnd), stdout);
            } else {
                //create copies of the article tag so the program can do different string operations on them
                strncpy(findArticle, endArticleEnd, serverRespPtr - endArticleEnd);
                strcpy(findArticle2, findArticle);
                findArticlePtr = findArticle;
                findArticle2Ptr = findArticle2;

                //check if the article is a breaking news article
                if ((findBreaking = strstr(findArticlePtr, "Breaking")) != NULL) {

                    //find h3 tag (which holds the title)
                    while ((findArticlePtr = strstr(findArticlePtr, "<h3")) != NULL) {
                        //find end of h3 start tag
                        endTitleEnd = strchr(findArticlePtr, '>');
                        if (!endTitleEnd) {
			    printf("endTitleEnd error\n");
			    break;
			}
                        endTitleEnd++;

                        //find h3 end tag
                        findArticlePtr = strstr(endTitleEnd, "</h3>");
                        if (findArticlePtr) {
                            //inside of h3 tag, get title and format the result
                            strcpy(title, "title: ");
                            strncat(title, endTitleEnd, findArticlePtr-endTitleEnd);
                            strcat(title, "\n\n");
                            strcpy(fullArticle, title);
			    memset(title, '\0', strlen(title));
                        }
			break;
                    }

                    //find paragraphs in article
                    while ((findArticle2Ptr = strstr(findArticle2Ptr, "<p "))) {
                        //find end of p start tag
                        endParagraphEnd = strchr(findArticle2Ptr, '>');
                        if (!endParagraphEnd) {
			    printf("endParagraphEnd error\n");
			    break;
			}
                        endParagraphEnd++;

                        //find end p tag
                        findArticle2Ptr = strstr(endParagraphEnd, "</p>");
                        if (findArticle2Ptr) {
                            //inside of paragraph tag, get paragraph and format the result
                            strcpy(paragraphs, "paragraph: ");
                            strncat(paragraphs, endParagraphEnd, findArticle2Ptr-endParagraphEnd);
                            strcat(paragraphs, "\n\n");
                            strcat(fullArticle, paragraphs);
                            memset(paragraphs, '\0', strlen(paragraphs));
                        }
                    }
		    //print article for logging purposes
                    printf("%s\n\n", fullArticle);

                    //put data into txt file
                    FILE *fptr = fopen("/home/pi/Desktop/webServer/websites/ytviewer.ga/article.txt", "w");
                    fprintf(fptr, "%s", fullArticle);
                    fclose(fptr);
                } else {
		    continue;
		}
            }
	    //only find the latest breaking news article
	    break;
        }


        //clear everything and prepare to restart proccess after 5 mins (300 seconds)
	memset(serverResp, '\0', strlen(serverResp));
        memset(buf, '\0', strlen(buf));
        totalBytes = 0;
	memset(findBreaking, '\0', strlen(findBreaking));
        memset(acClientRequest, '\0', strlen(acClientRequest));
        memset(findArticle, '\0', strlen(findArticle));
        memset(findArticle2, '\0', strlen(findArticle2));
        memset(endArticleEnd, '\0', strlen(endArticleEnd));
        memset(endTitleEnd, '\0', strlen(endTitleEnd));
        memset(endParagraphEnd, '\0', strlen(endParagraphEnd));
        memset(fullArticle, '\0', strlen(fullArticle));

        close(server);
        SSL_CTX_free(ctx);
        sleep(300); //5mins/300secs
    }

    free(serverResp);
    close(server);
    SSL_CTX_free(ctx);
    return 0;
}

//run this program on my computer
//cd /home/rowan/Desktop/webScraper/ && gcc scrape.c -o scrape -Wall -lcrypto -lssl && "/home/rowan/Desktop/webScraper/"scrape -Wall -lcrypto -lssl


