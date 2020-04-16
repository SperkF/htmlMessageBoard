/*
** server.c -- a stream socket server demo
** in part taken from Beej's network programming book
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //close()
#include <errno.h> //perror()
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> //htons()
#include <sys/wait.h>
#include <signal.h>
#include "err_macro.h"

#define PORT "8000"  // the port we will bind our server to, and the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold ->used for bind() call

#define FS_DEBUG 1  //1.. debug comments acitve, 0..debug comments inactive

#define HTML_FILE "my_webpage.html" //name of html file (where our website code is saved)

#include <fcntl.h> // setsockopt()

//signal handler for SIGCHLD
void sigchld_handler (int s)
{
	// waitpid() might overwrite errno, so we save and restore it
	int saved_errno = errno;

//reap dead child, dont block
	while (waitpid (-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


char *get_html_file_content (char *html_file_name)
{
	/***********************__http specific part______________****************/
	int html_file_chr_cnt = 0;
	FILE *fp_html = NULL;
	char *content_html_file = NULL;
	if ( (fp_html = fopen (html_file_name, "r")) == NULL)
		ERR_EXIT();

//count characters inside html file and allocate mem-space for conent accordingly
	while (fgetc (fp_html) != EOF)
		html_file_chr_cnt++;

	if ( (content_html_file = calloc (html_file_chr_cnt, sizeof (char))) == NULL)
		ERR_EXIT();


//reopen html file to get pointer back to first pos of file and read content into array
	if ( (freopen (html_file_name, "r", fp_html)) == NULL)
		ERR_EXIT();

	for (int i = 0; i < html_file_chr_cnt; i++)
		* (content_html_file + i) = fgetc (fp_html);

	return content_html_file;
}


int main (void)
{
	char *content_html_file = NULL;
//inside function memory is allocated dymically dont forget to free()
	content_html_file = get_html_file_content (HTML_FILE);

	/*setup strings for communication*/
	//is returned to client upon connect ->Protocol used is 1.1, and request OK(data will be sent)
	char http_header[] = "HTTP/1.0 200 OK\r\n\r\n";
	int http_header_len = 0;
	http_header_len = strlen (http_header);
	//put Header on top of our html data string(webpage code writen in html)
	strcat (http_header, content_html_file);
	printf ("\n\t__%s__\n", http_header);

	/*********************__setup standard TCP stream server*************************/
	int server_fd, client_fd;  // listen on server_fd, new connection on client_fd
	struct addrinfo hints, *serverinfo, *hp_serverinfo;
	struct sockaddr_storage client_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

//setup hints for getaddrinfo()
	memset (&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ( (rv = getaddrinfo (NULL, PORT, &hints, &serverinfo)) != 0) {
		//in theory gai_strerro() should return some meaningfull string informing us about the error
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
		exit (EXIT_FAILURE);
	}

/* loop through all the results till we get succesfull bind or list ends*/
/*
->hp_serverinfo = serverinfo ... means that hp_serverinfo points to first element of the linked
	list (Hilfspointer um die Adresse des ersten Listenelements zu "speicher" da wir dieses später
	freeaddrinfo() als argument übergeben müssen; serverinfo wird ja dann "durchgeparst")
->p_serverinfo->ai_next ..is pointer to next struct of linked-list
	*/

	for (hp_serverinfo = serverinfo ; serverinfo != NULL; serverinfo = serverinfo->ai_next) {
		//try if we can open a socket with the given values in the current list
		if ( (server_fd = socket (serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol)) == -1) {
			perror ("server: socket");
			continue; //skip rest of for()-loop and start at top of loop again
		}
		//set options for created socket ->see OneNote
		//this is just a meassurement to clear out the PORT
		if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
			perror ("setsockopt");
			exit (EXIT_FAILURE);
		}
		//try to bind socket to address retireved from the current list
		if (bind (server_fd, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1) {
			close (server_fd); //if bind() fails close socket
			perror ("server: bind");
			continue; //if the bind() fails ->try with next list entry
		}

		break; //in case bind() did not fail we came to this point an jump out of the for loop
	}

	if (serverinfo == NULL) {
		fprintf (stderr, "server: failed to bind (hint: lookup runing servers with _ps_)\n");
		exit (1);
	}

	//print adress of server
	//inet_Networ-to-printable
	char server_IP_dotted_rep[INET6_ADDRSTRLEN];
	inet_ntop (serverinfo->ai_family, (struct sockaddr *) serverinfo->ai_addr, \
		   server_IP_dotted_rep, sizeof (server_IP_dotted_rep));
	printf ("server: my IP is: %s\n", server_IP_dotted_rep);

// all infos taken from linked-list ->free full list
// p_serverinfo point to first list-ellement
	freeaddrinfo (hp_serverinfo);


//set server fd to listen, so we can than call accept() in order to accept connections from clients
	if (listen (server_fd, BACKLOG) == -1) {
		perror ("listen");
		exit (1);
	}

//setup signalhandler with sigaction()
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction (SIGCHLD, &sa, NULL) == -1) {
		perror ("sigaction");
		exit (1);
	}


	/*to test server with inet-exporer type:
	  http://localhost:<PORT-NUMBER>   or,
	  http://127.0.0.1:<PORT-NUMBER>   or,
	  http://<SERVER-IP>:<PORT-NUMBER>
	    PORT-NUMBER -> as defined by programming
	    SERVER-IP ->gets writen to cmd-prompt upon server startup
	*/
	while (1) { // main accept() loop
    printf ("server: waiting for connections...\n");
		sin_size = sizeof client_addr;
  //accept blocks till we get connection-request from a client
		client_fd = accept (server_fd, (struct sockaddr *) &client_addr, &sin_size);
		if (client_fd == -1) {
			perror ("accept");
			continue;
		} else {
			inet_ntop (client_addr.ss_family, (struct sockaddr *) &client_addr, s, sizeof s);
			printf ("server: got connection from %s\n", s);
		}



		if (!fork()) { // this is the child process
			close (server_fd); // child doesn't need the listener
			if (send (client_fd, http_header, sizeof (http_header), 0) == -1)
				perror ("send");
			close (client_fd);
			exit (0);
		}
		close (client_fd); // parent doesn't need this
	}

	return 0;
}
