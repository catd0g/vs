#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

int main(int argc, char **argv)
{
        int ret;                // return value from functions

        // Check command line arguments
        if (argc != 3) {
                fprintf(stderr,
                        "Missing parameters. Usage: %s <server-name-or-ip> <server-port>\n",
                        argv[0]);
                return 1;
        }
        // Address information structure
        struct addrinfo aii;
        // Set whole structure to 0s
        memset(&aii, 0, sizeof(aii));
        // A stream (TCP) connection
        aii.ai_socktype = SOCK_STREAM;
        // We do not care whether it is IPv4 or IPv6
        aii.ai_family = PF_UNSPEC;

        struct addrinfo *aio;
        // Get address information.
        // First parameter is host string, either hostname or numerical IPv4/IPv6 address
        // Second parameter is port/service string, either as port number
        // or well-known identifier, e.g. http
        // So, e.g. getaddrinfo( "www.compeng.uni-frankfurt.de", "http", ... getaddrinfo( "141.2.248.1", "80", ...
        // Third parameter is input address info structure (cf. above)
        // Fourth parameter is output address info structure, a linked list of potential addresses
        ret = getaddrinfo(argv[1], argv[2], &aii, &aio);
        if (ret) {
                fprintf(stderr, "Error getting address for %s:%s: %s\n",
                        argv[1], argv[2], gai_strerror(ret));
                return 1;
        }

        // File descriptor for the socket
        int sock = -1;
        struct addrinfo *iter;
        // Iterate over linked list of specified output addresses,
        // use first address to which a connection can be established
        for (iter = aio; iter != NULL && sock == -1; iter = iter->ai_next) {
                // Create socket given the parameters from the found address info.
                sock =
                    socket(iter->ai_family, iter->ai_socktype,
                           iter->ai_protocol);
                if (sock < 0)
                        continue;       // Appropriate socket could not be created, try next address

                // Socket created successfully, now try to connect to remote target address
                // taken from address info
                ret = connect(sock, iter->ai_addr, iter->ai_addrlen);
                if (ret) {
                        // Socket could not be connected to remote target
                        close(sock);    // Close socket
                        sock = -1;
                        continue;       // try next address
                }
        }
        freeaddrinfo(aio);      // Release address information allocated in getaddrinfo

        if (sock == -1) {
                // No connection at all could be established to remote target
                fprintf(stderr, "Unable to establish any connection to %s:%s\n",
                        argv[1], argv[2]);
                return 1;
        }


        //WRITE MESSAGE TO SERVER
        int max = 1042;
        char line[max];
        char message[] = "GET";
        fgets(line, max, stdin);
        int msglen = strlen(line) + 1;
        int recieved;
        // Write the whole message in one go, fail if this does not work
        recieved = write(sock, line, msglen);
        printf("write message %s\n", line);
        //printf(recieved);
        if (recieved != msglen) {
                perror("Error during write");
                return 1;
        }

        // Get message from server
        // Maximum size of incoming message
        int re_msglen = 100;

        // Buffer for message
        char re_buf[re_msglen + 1];   // One more to ensure that there is a trailing NULL char.
        memset(re_buf, 0, re_msglen + 1);
        ret = read(sock, re_buf, re_msglen);     // Return value is amount of bytes read, -1 in case of error
        printf("Data read: '%s'\n", re_buf);
        // Clean up after us and close the socket.

        close(sock);
        return 0;
}
