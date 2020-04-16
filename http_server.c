/*
  THis example code is based on a youtube video
      -Socket Programming Tutorials in C For Beginners | Part 2 | Eduonix
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> //close()
#include <arpa/inet.h> //htons()

#define MAX_HTML_FILE_CHAR_CNT 1024  //use dynamic memory instead of those defines later on
#define HTTP_HEADER_SIZE 4000



int main (void){

/*get data from an html file ->where our "website" is represented*/
  //open htmo-file that lies in same directory as program in readmode
  FILE *fp_html_file = NULL;
  if( (fp_html_file = (FILE*)fopen("my_webpage.html", "r")) == NULL)
  {
    fprintf(stderr, "fopen() for html-file failed\n");
    exit(EXIT_SUCCESS);
  }
  //read data from html-file into char array
/*  char html_data[MAX_HTML_FILE_CHAR_CNT];
  fgets(html_data, MAX_HTML_FILE_CHAR_CNT, fp_html_file); //do errorchecking here
*/
  char *html_data = NULL;
  int html_file_char_cnt = 0;
  while( fgetc(fp_html_file) != EOF)
  {
    html_file_char_cnt++;
  }
  html_data = (char*)calloc(html_file_char_cnt, sizeof(char));


/*setup strings for communication*/
  //is returned to client upon connect ->Protocol used is 1.1, and request OK(data will be sent)
  char http_header[] = "HTTP/1.1 200 OK\r\n\r\n";
  int http_header_len = 0;
  http_header_len = strlen(http_header);
  //put Header on top of our html data string(webpage code writen in html)
  strncat(http_header, html_data, http_header_len + html_file_char_cnt);

/*setup a TCP stream server socket*/
  //create socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  //define IP and Port of socket
  struct sockaddr_in server_adress;
  server_adress.sin_family = AF_INET;
  server_adress.sin_port = htons(8001); //Host-To-Network-Order; to make sure 8001 is represented correct (big endian)
  server_adress.sin_addr.s_addr = INADDR_ANY;
  //bind server to Port and IP
  bind(server_socket, (struct sockaddr*)&server_adress, sizeof(server_adress));
  //set server to listen + allow a max. of 5 pending server anfragen
  listen(server_socket, 5);
  //pack accept() in while loop
  int client_socket;
  while(1)
  {
    //nimm connect entgegen, sende Daten, schließe verbindung
    client_socket = accept(server_socket, NULL, NULL);
    send(client_socket, http_header, sizeof(http_header), 0);
  //  close(client_socket);
  }

  return 0;
}
