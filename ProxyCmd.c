
#include "libssh2_config.h"
#include <libssh2.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

//following is required for loging into text file.
#include <stdarg.h>
#include <time.h>

#include <stdio.h>

//for thread
#include <pthread.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif


#ifndef INADDR_NONE
#define INADDR_NONE (in_addr_t)-1
#endif

#define LOG_PRINT(...) log_print(__FILE__, __LINE__, __VA_ARGS__ )



//Local Forward function variables

char *localForward_keyfile1;
char *localForward_keyfile2;
char *localForward_username;                         /* HA username */
char *localForward_password;                         /* HA password */
char *localForward_server_ip;                        /* HA ip       */
char *localForward_local_listenip;                   /* HA ip       */
unsigned int localForward_local_listenport;          /* HA port     */
char *localForward_remote_desthost;                  /* switch ip   */ 
unsigned int localForward_remote_destport;           /* switch port */


//Remote Forward function variables

char *remoteForward_keyfile1;
char *remoteForward_keyfile2;
char *remoteForward_username;				         /* HM username */
char *remoteForward_password;					     /* HM password */
char *remoteForward_server_ip;				         /* HM ip       */
char *remoteForward_remote_listenhost;               /* HM ip       */
unsigned int remoteForward_remote_wantport;			 /* HM port     */
char *remoteForward_local_destip;				     /* HA ip       */
unsigned int remoteForward_local_destport;			 /* HA port     */

int remote_listenport;


/* pre function declaration */
void *LocalPortForwarding();
void *RemotePortForwarding();

enum {
    AUTH_NONE = 0,
    AUTH_PASSWORD,
    AUTH_PUBLICKEY
};

/* Following is for logging into file */
FILE *fp ;
static int SESSION_TRACKER; //Keeps track of session
 
char* print_time()
{
    time_t t;
    char *buf;
     
    time(&t);
    buf = (char*)malloc(strlen(ctime(&t))+ 1);
     
    snprintf(buf,strlen(ctime(&t)),"%s ", ctime(&t));
    
    return buf;
}
void log_print(char* filename, int line, char *fmt,...)
{
    va_list         list;
    char            *p, *r;
    int             e;
 
    if(SESSION_TRACKER > 0)
      fp = fopen ("ProxyCmdLog.txt","a+");
    else
      fp = fopen ("ProxyCmdLog.txt","w");
     
    fprintf(fp,"%s ",print_time());
    va_start( list, fmt );
 
    for ( p = fmt ; *p ; ++p )
    {
        if ( *p != '%' )//If simple string
        {
            fputc( *p,fp );
        }
        else
        {
            switch ( *++p )
            {
                /* string */
            case 's':
            {
                r = va_arg( list, char * );
 
                fprintf(fp,"%s", r);
                continue;
            }
 
            /* integer */
            case 'd':
            {
                e = va_arg( list, int );
 
                fprintf(fp,"%d", e);
                continue;
            }
 
            default:
                fputc( *p, fp );
            }
        }
    }
    va_end( list );
    fprintf(fp," [%s][line: %d] ",filename,line);
    fputc( '\n', fp );
    SESSION_TRACKER++;
    fclose(fp);
}



//This is entry point of the funtion i.e main.
int main(int argc, char *argv[])
{

 		pthread_t local_thread, remote_thread;
		int  local_thread_ret, remote_thread_ret;
        char *HA_ip,*HA_username,*HA_password,*HM_ip,*HM_username,*HM_password,*Switch_ip;
		unsigned int HA_port,HM_port,Switch_port;


		 if (argc > 1)
      	 	HA_ip = argv[1];
   		 if (argc > 2)
        	HA_port = atoi(argv[2]);
   		 if (argc > 3)
        	HA_username = argv[3];
    	 if (argc > 4)
        	HA_password = argv[4];
    	 if (argc > 5)
        	HM_ip = argv[5];
    	 if (argc > 6)
            HM_port = atoi(argv[6]);
         if (argc > 7)
            HM_username = argv[7];
 		 if (argc > 8)
            HM_password = argv[8];
		 if (argc > 9)
            Switch_ip = argv[9];
         if (argc > 10)
            Switch_port = atoi(argv[10]);



		localForward_keyfile1 = "/home/localForward_username/.ssh/id_rsa.pub";
		localForward_keyfile2 = "/home/localForward_username/.ssh/id_rsa";
		localForward_username = HA_username; 				  	  /* HA username */
	    localForward_password = HA_password;                      /* HA password */
		localForward_server_ip = HA_ip;                           /* HA ip       */
		localForward_local_listenip =HA_ip;          	          /* HA ip       */
		localForward_local_listenport = HA_port;        		  /* HA port     */
		localForward_remote_desthost = Switch_ip;                 /* switch ip   */ 
		localForward_remote_destport = Switch_port;               /* switch port */


		//Remote Forward function variables

		remoteForward_keyfile1 = "/home/remoteForward_username/.ssh/id_rsa.pub";
		remoteForward_keyfile2 = "/home/remoteForward_username/.ssh/id_rsa";
		remoteForward_username = HM_username;					    /* HM username */
		remoteForward_password = HM_password;					    /* HM password */
		remoteForward_server_ip = HM_ip;				            /* HM ip       */
		remoteForward_remote_listenhost =HM_ip;	                    /* HM ip       */
		remoteForward_remote_wantport = HM_port;			        /* HM port     */
		remoteForward_local_destip = HA_ip;				            /* HA ip       */
		remoteForward_local_destport = HA_port;			            /* HA port     */

		LOG_PRINT("In Main Function");

        remote_thread_ret = pthread_create( &remote_thread, NULL, RemotePortForwarding,NULL); 
 		local_thread_ret= pthread_create( &local_thread, NULL, LocalPortForwarding,NULL);
     		
        pthread_join(remote_thread , NULL); 
        pthread_join(local_thread, NULL);

 		printf("Remote port forwarding thread returns: %d\n",remote_thread_ret);
		LOG_PRINT("Remote port forwarding thread returns: %d",remote_thread_ret);	
	 	printf("Local port forwarding thread  returns: %d\n",local_thread_ret);
        LOG_PRINT("Local port forwarding thread  returns: %d",local_thread_ret);	
        exit(0);

        LOG_PRINT("Exiting the main function");	
}

void *LocalPortForwarding()
{
    LOG_PRINT("In LocalPortForwading function");
    int rc, i, auth = AUTH_NONE;
    struct sockaddr_in sin;
    socklen_t sinlen;
    const char *fingerprint;
    char *userauthlist;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel = NULL;
    const char *shost;
    unsigned int sport;
    fd_set fds;
    struct timeval tv;
    ssize_t len, wr;
    char buf[16384];

#ifdef WIN32l
    char sockopt;
    SOCKET sock = INVALID_SOCKET;
    SOCKET listensock = INVALID_SOCKET, forwardsock = INVALID_SOCKET;
    WSADATA wsadata;
    int err;

    err = WSAStartup(MAKEWORD(2,0), &wsadata);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", err);
	    LOG_PRINT("WSAStartup failed with error: %d", err);
        //return 1;
    }
#else
    int sockopt, sock = -1;
    int listensock = -1, forwardsock = -1;
#endif

/*
    if (argc > 1)
        localForward_server_ip = argv[1];
    if (argc > 2)
        localForward_username = argv[2];
    if (argc > 3)
        localForward_password = argv[3];
    if (argc > 4)
        localForward_local_listenip = argv[4];
    if (argc > 5)
        localForward_local_listenport = atoi(argv[5]);
    if (argc > 6)
        localForward_remote_desthost = argv[6];
    if (argc > 7)
        localForward_remote_destport = atoi(argv[7]);
*/
    rc = libssh2_init (0);
    if (rc != 0) {
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        LOG_PRINT("libssh2 initialization failed (%d)", rc);
        //return 1;
    }

//while loop for contineously creating the new socs and listen it.

    /* Connect to SSH server */
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef WIN32
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "failed to open socket!\n");
        LOG_PRINT("failed to open socket!");
        //return -1;
    }
#else
    if (sock == -1) {
        perror("socket");
        //return -1;
    }
#endif

    sin.sin_family = AF_INET;
    if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(localForward_server_ip))) {
        perror("inet_addr");
        //return -1;
    }
    sin.sin_port = htons(22);
    if (connect(sock, (struct sockaddr*)(&sin),sizeof(struct sockaddr_in)) != 0) {
        fprintf(stderr, "failed to connect!\n");
        LOG_PRINT("failed to connect!");
        //return -1;
    }

    /* Create a session instance */
    session = libssh2_session_init();
    if(!session) {
        fprintf(stderr, "Could not initialize SSH session!\n");
        LOG_PRINT("Could not initialize SSH session!");
     //   return -1;
    }

    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    rc = libssh2_session_handshake(session, sock);
    if(rc) {
        fprintf(stderr, "Error when starting up SSH session: %d\n", rc);
        LOG_PRINT("Error when starting up SSH session: %d", rc);
       // return -1;
    }

    /* At this point we havn't yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     */
    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    fprintf(stderr, "Fingerprint: ");
    for(i = 0; i < 20; i++)
	{
        fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(stderr, "\n");

    /* check what authentication methods are available */
    userauthlist = libssh2_userauth_list(session, localForward_username, strlen(localForward_username));
    fprintf(stderr, "Authentication methods: %s\n", userauthlist);
    LOG_PRINT("Authentication methods: %s", userauthlist);
    if (strstr(userauthlist, "password"))
        auth |= AUTH_PASSWORD;
    if (strstr(userauthlist, "publickey"))
        auth |= AUTH_PUBLICKEY;

    /* check for options */
	/*
    if(argc > 8) {
        if ((auth & AUTH_PASSWORD) && !strcasecmp(argv[8], "-p"))
            auth = AUTH_PASSWORD;
        if ((auth & AUTH_PUBLICKEY) && !strcasecmp(argv[8], "-k"))
            auth = AUTH_PUBLICKEY;
    }
	*/
    if (auth & AUTH_PASSWORD) {
        if (libssh2_userauth_password(session, localForward_username, localForward_password)) {
            fprintf(stderr, "Authentication by localForward_password failed.\n");
			LOG_PRINT("Authentication by localForward_password failed.");
            goto shutdown;
        }
    } else if (auth & AUTH_PUBLICKEY) {
        if (libssh2_userauth_publickey_fromfile(session, localForward_username, localForward_keyfile1,
                                                localForward_keyfile2, localForward_password)) {
            fprintf(stderr, "\tAuthentication by public key failed!\n");
			LOG_PRINT("\tAuthentication by public key failed!");
            goto shutdown;
        }
        fprintf(stderr, "\tAuthentication by public key succeeded.\n");
		LOG_PRINT("\tAuthentication by public key succeeded.");
    } else {
        fprintf(stderr, "No supported authentication methods found!\n");
		LOG_PRINT("No supported authentication methods found!");
        goto shutdown;
    }

    listensock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef WIN32
    if (listensock == INVALID_SOCKET) {
        fprintf(stderr, "failed to open listen socket!\n");
		LOG_PRINT("failed to open listen socket!");
       // return -1;
    }
#else
    if (listensock == -1) {
        perror("socket");
        //return -1;
    }
#endif

    sin.sin_family = AF_INET;
    sin.sin_port = htons(localForward_local_listenport);
    if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(localForward_local_listenip))) {
        perror("inet_addr");
        goto shutdown;
    }
    sockopt = 1;
    setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
    sinlen=sizeof(sin);
    if (-1 == bind(listensock, (struct sockaddr *)&sin, sinlen)) {
        perror("bind");
        goto shutdown;
    }
    if (-1 == listen(listensock, 2)) {
        perror("listen");
        goto shutdown;
    }

    fprintf(stderr, "Waiting for TCP connection on %s:%d...\n",inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    LOG_PRINT("Waiting for TCP connection on %s:%d...",inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    forwardsock = accept(listensock, (struct sockaddr *)&sin, &sinlen);
#ifdef WIN32
    if (forwardsock == INVALID_SOCKET) {
        fprintf(stderr, "failed to accept forward socket!\n");
 		LOG_PRINT("failed to accept forward socket!");
        goto shutdown;
    }
#else
    if (forwardsock == -1) {
        perror("accept");
        goto shutdown;
    }
#endif

    shost = inet_ntoa(sin.sin_addr);
    sport = ntohs(sin.sin_port);

    fprintf(stderr, "Forwarding connection from %s:%d here to remote %s:%d\n",shost, sport, localForward_remote_desthost, localForward_remote_destport);
	LOG_PRINT("Forwarding connection from %s:%d here to remote %s:%d",shost, sport, localForward_remote_desthost, localForward_remote_destport);

    channel = libssh2_channel_direct_tcpip_ex(session, localForward_remote_desthost,
        localForward_remote_destport, shost, sport);
    if (!channel) {
        fprintf(stderr, "Could not open the direct-tcpip channel!\n" "(Note that this can be a problem at the server!" " Please review the server logs.)\n");
		LOG_PRINT("Could not open the direct-tcpip channel!" "(Note that this can be a problem at the server!" " Please review the server logs.)");
        goto shutdown;
    }

    /* Must use non-blocking IO hereafter due to the current libssh2 API */
    libssh2_session_set_blocking(session, 0);

    while (1) {
        FD_ZERO(&fds);
        FD_SET(forwardsock, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        rc = select(forwardsock + 1, &fds, NULL, NULL, &tv);
        if (-1 == rc) {
            perror("select");
            goto shutdown;
        }
        if (rc && FD_ISSET(forwardsock, &fds)) {
            len = recv(forwardsock, buf, sizeof(buf), 0);
            if (len < 0) {
                perror("read");
                goto shutdown;
            } else if (0 == len) {
                fprintf(stderr, "The client at %s:%d disconnected!\n", shost,sport);
				LOG_PRINT("The client at %s:%d disconnected!", shost,sport);
                goto shutdown;
            }
            wr = 0;
            while(wr < len) {
                i = libssh2_channel_write(channel, buf + wr, len - wr);
                if (LIBSSH2_ERROR_EAGAIN == i) {
                    continue;
                }
                if (i < 0) {
                    fprintf(stderr, "libssh2_channel_write: %d\n", i);
 					LOG_PRINT("libssh2_channel_write: %d", i);
                    goto shutdown;
                }
                wr += i;
            }
        }
        while (1) {
            len = libssh2_channel_read(channel, buf, sizeof(buf));
            if (LIBSSH2_ERROR_EAGAIN == len)
                break;
            else if (len < 0) {
                fprintf(stderr, "libssh2_channel_read: %d", (int)len);
				LOG_PRINT("libssh2_channel_read: %d", (int)len);
                goto shutdown;
            }
            wr = 0;
            while (wr < len) {
                i = send(forwardsock, buf + wr, len - wr, 0);
                if (i <= 0) {
                    perror("write");
                    goto shutdown;
                }
                wr += i;
            }
            if (libssh2_channel_eof(channel)) {
                fprintf(stderr, "The server at %s:%d disconnected!\n",localForward_remote_desthost, localForward_remote_destport);
				LOG_PRINT("The server at %s:%d disconnected!",localForward_remote_desthost, localForward_remote_destport);
                goto shutdown;
            }
        }
    }

shutdown:
#ifdef WIN32
    closesocket(forwardsock);
    closesocket(listensock);
#else
    close(forwardsock);
    close(listensock);
#endif
    if (channel)
        libssh2_channel_free(channel);
    libssh2_session_disconnect(session, "Client disconnecting normally");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    libssh2_exit();

    LOG_PRINT("Exiting LocalPortForwadring function");	
}

void *RemotePortForwarding()
{
    LOG_PRINT("In RemotePortForwading function");
    int rc, i, auth = AUTH_NONE;
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    const char *fingerprint;
    char *userauthlist;
    LIBSSH2_SESSION *session;
    LIBSSH2_LISTENER *listener = NULL;
    LIBSSH2_CHANNEL *channel = NULL;
    fd_set fds;
    struct timeval tv;
    ssize_t len, wr;
    char buf[16384];

#ifdef WIN32
    SOCKET sock = INVALID_SOCKET, forwardsock = INVALID_SOCKET;
    WSADATA wsadata;
    int err;

    err = WSAStartup(MAKEWORD(2,0), &wsadata);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", err);
		LOG_PRINT("WSAStartup failed with error: %d", err);
        //return 1;
    }
#else
    int sock = -1, forwardsock = -1;
#endif
/*
    if (argc > 1)
        remoteForward_server_ip = argv[1];
    if (argc > 2)
        remoteForward_username = argv[2];
    if (argc > 3)
        password = argv[3];
    if (argc > 4)
        remoteForward_remote_listenhost = argv[4];
    if (argc > 5)
        remoteForward_remote_wantport = atoi(argv[5]);
    if (argc > 6)
        remoteForward_local_destip = argv[6];l
    if (argc > 7)
        remoteForward_local_destport = atoi(argv[7]);

    rc = libssh2_init (0);
    if (rc != 0) {
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
        return 1;
    }
*/

    /* Connect to SSH server */
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef WIN32
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "failed to open socket!\n");
		LOG_PRINT("failed to open socket!");
       // return -1;
    }
#else
    if (sock == -1) {
        perror("socket");
        //return -1;
    }
#endif

    sin.sin_family = AF_INET;
    if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(remoteForward_server_ip))) {
        perror("inet_addr");
        //return -1;l
    }
    sin.sin_port = htons(22);
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) {
        fprintf(stderr, "failed to connect!\n");
		LOG_PRINT("failed to connect!");
     //   return -1;
    }

    /* Create a session instance */
    session = libssh2_session_init();
    if(!session) {
        fprintf(stderr, "Could not initialize SSH session!\n");
		LOG_PRINT("Could not initialize SSH session!");
       // return -1;
    }

    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    rc = libssh2_session_handshake(session, sock);
    if(rc) {
        fprintf(stderr, "Error when starting up SSH session: %d\n", rc);
  		LOG_PRINT("Error when starting up SSH session: %d", rc);
        //return -1;
    }

    /* At this point we havn't yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     */
    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    fprintf(stderr, "Fingerprint: ");
    for(i = 0; i < 20; i++)
	{
        fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
    }
    fprintf(stderr, "\n");
	
    /* check what authentication methods are available */
    userauthlist = libssh2_userauth_list(session, remoteForward_username, strlen(remoteForward_username));
    fprintf(stderr, "Authentication methods: %s\n", userauthlist);
    LOG_PRINT("Authentication methods: %s", userauthlist);
    if (strstr(userauthlist, "password"))
        auth |= AUTH_PASSWORD;
    if (strstr(userauthlist, "publickey"))
        auth |= AUTH_PUBLICKEY;


    /* 
	//check for options 
    if(argc > 8) {
        if ((auth & AUTH_PASSWORD) && !strcasecmp(argv[8], "-p"))
            auth = AUTH_PASSWORD;
        if ((auth & AUTH_PUBLICKEY) && !strcasecmp(argv[8], "-k"))
            auth = AUTH_PUBLICKEY;
    }
*/
    if (auth & AUTH_PASSWORD) {
        if (libssh2_userauth_password(session, remoteForward_username, remoteForward_password)) {
            fprintf(stderr, "Authentication by password failed.\n");
 			LOG_PRINT("Authentication by password failed.");
            goto shutdown;
        }
    } else if (auth & AUTH_PUBLICKEY) {
        if (libssh2_userauth_publickey_fromfile(session, remoteForward_username,remoteForward_keyfile1,
                                                remoteForward_keyfile2, remoteForward_password)) {
            fprintf(stderr, "\tAuthentication by public key failed!\n");
 			LOG_PRINT("\tAuthentication by public key failed!");
            goto shutdown;
        }
        fprintf(stderr, "\tAuthentication by public key succeeded.\n");
		LOG_PRINT("\tAuthentication by public key succeeded.");
    } else {
        fprintf(stderr, "No supported authentication methods found!\n");
		LOG_PRINT("No supported authentication methods found!");
        goto shutdown;
    }

    fprintf(stderr, "Asking server to listen on remote %s:%d\n",remoteForward_remote_listenhost, remoteForward_remote_wantport);
 	LOG_PRINT("Asking server to listen on remote %s:%d",remoteForward_remote_listenhost, remoteForward_remote_wantport);
    listener = libssh2_channel_forward_listen_ex(session, remoteForward_remote_listenhost,
        remoteForward_remote_wantport, &remote_listenport, 1);
    if (!listener) {
        fprintf(stderr, "Could not start the tcpip-forward listener!\n" "(Note that this can be a problem at the server!"" Please review the server logs.)\n");
		LOG_PRINT("Could not start the tcpip-forward listener!" "(Note that this can be a problem at the server!"" Please review the server logs.)");
        goto shutdown;
    }

    fprintf(stderr, "Server is listening on %s:%d\n", remoteForward_remote_listenhost,remote_listenport);
	LOG_PRINT("Server is listening on %s:%d", remoteForward_remote_listenhost,remote_listenport);

    fprintf(stderr, "Waiting for remote connection\n");
 	LOG_PRINT("Waiting for remote connection");
    channel = libssh2_channel_forward_accept(listener);
    if (!channel) {
        fprintf(stderr, "Could not accept connection!\n""(Note that this can be a problem at the server!"" Please review the server logs.)\n");
        LOG_PRINT("Could not accept connection!""(Note that this can be a problem at the server!"" Please review the server logs.)");
        goto shutdown;
    }

    fprintf(stderr,"Accepted remote connection. Connecting to local server %s:%d\n",remoteForward_local_destip, remoteForward_local_destport);
	LOG_PRINT("Accepted remote connection. Connecting to local server %s:%d",remoteForward_local_destip, remoteForward_local_destport);
    forwardsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef WIN32
    if (forwardsock == INVALID_SOCKET) {
        fprintf(stderr, "failed to open forward socket!\n");
		LOG_PRINT("failed to open forward socket!");
        goto shutdown;
    }
#else
    if (forwardsock == -1) {
        perror("socket");
        goto shutdown;
    }
#endif

    sin.sin_family = AF_INET;
    sin.sin_port = htons(remoteForward_local_destport);
    if (INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(remoteForward_local_destip))) {
        perror("inet_addr");
        goto shutdown;
    }
    if (-1 == connect(forwardsock, (struct sockaddr *)&sin, sinlen)) {
        perror("connect");
        goto shutdown;
    }

    fprintf(stderr, "Forwarding connection from remote %s:%d to local %s:%d\n",remoteForward_remote_listenhost, remote_listenport, 
			remoteForward_local_destip, remoteForward_local_destport);
    LOG_PRINT("Forwarding connection from remote %s:%d to local %s:%d",remoteForward_remote_listenhost, remote_listenport, 
			remoteForward_local_destip, remoteForward_local_destport);

    /* Must use non-blocking IO hereafter due to the current libssh2 API */
    libssh2_session_set_blocking(session, 0);

    while (1) {
        FD_ZERO(&fds);
        FD_SET(forwardsock, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        rc = select(forwardsock + 1, &fds, NULL, NULL, &tv);
        if (-1 == rc) {
            perror("select");
            goto shutdown;
        }
        if (rc && FD_ISSET(forwardsock, &fds)) {
            len = recv(forwardsock, buf, sizeof(buf), 0);
            if (len < 0) {
                perror("read");
                goto shutdown;
            } else if (0 == len) {
                fprintf(stderr, "The local server at %s:%d disconnected!\n",remoteForward_local_destip, remoteForward_local_destport);
				LOG_PRINT("The local server at %s:%d disconnected!",remoteForward_local_destip, remoteForward_local_destport);
                goto shutdown;
            }
            wr = 0;
            do {
                i = libssh2_channel_write(channel, buf, len);
                if (i < 0) {
                    fprintf(stderr, "libssh2_channel_write: %d\n", i);
					LOG_PRINT("libssh2_channel_write: %d", i);
                    goto shutdown;
                }
                wr += i;
            } while(i > 0 && wr < len);
        }
        while (1) {
            len = libssh2_channel_read(channel, buf, sizeof(buf));
            if (LIBSSH2_ERROR_EAGAIN == len)
                break;
            else if (len < 0) {
                fprintf(stderr, "libssh2_channel_read: %d", (int)len);
				LOG_PRINT("libssh2_channel_read: %d", (int)len);
                goto shutdown;
            }
            wr = 0;
            while (wr < len) {
                i = send(forwardsock, buf + wr, len - wr, 0);
                if (i <= 0) {
                    perror("write");
                    goto shutdown;
                }
                wr += i;
            }
            if (libssh2_channel_eof(channel)) {
                fprintf(stderr, "The remote client at %s:%d disconnected!\n",remoteForward_remote_listenhost, remoteForward_local_destport);
				LOG_PRINT("The remote client at %s:%d disconnected!",remoteForward_remote_listenhost, remoteForward_local_destport);
                goto shutdown;
            }
        }
    }

shutdown:
#ifdef WIN32
    closesocket(forwardsock);
#else
    close(forwardsock);
#endif
    if (channel)
        libssh2_channel_free(channel);
    if (listener)
        libssh2_channel_forward_cancel(listener);
    libssh2_session_disconnect(session, "Client disconnecting normally");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    libssh2_exit();
	LOG_PRINT("Exiting RemotePortForwadring function");	
}
