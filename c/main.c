//////////////////////////////////////////////
// Simple TCP Socket Client
// Code based on example from: http://cs.baylor.edu/~donahoo/practical/CSockets/code/TCPEchoClient.c
/////////////////////////////////////////////

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "sensocol.h"
#include "sensocol_dump.h"
#include "sensocol_schema.h"

#include "md5.h"

// The following example is written WITHOUT dynamic memory allocation.
// The buffer sizes need to be set appropriately based on the data being sent.

#define RCVBUFSIZE 5   /* Size of receive buffer */
#define SNDBUFSIZE 512  /* Size of send buffer */
#define SCHBUFSIZE 300

// Error messaging
void DieWithError(char *errorMessage)  /* Error handling function */
{
  fprintf(stderr, "Error: %s\n", errorMessage);
  exit(1);
}

// Read byts from socket into buffer
void RecvBytes(int sock, char *recvBuffer, int expectedLength)
{
  int bytesRcvd, totalBytesRcvd = 0;   /* Bytes read in single recv() */

   while ( totalBytesRcvd < expectedLength)
    {
        /* Receive up to the buffer size (minus 1 to leave space for
           a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, recvBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        printf("Total bytes recieved: %i\n", totalBytesRcvd);
    }
}

// The code below illustrates how to use sensocol, but is device/platform dependent.
// Please see the inline comments for details
int main(int argc, char *argv[])
{
  
    int sock;                        /* Socket descriptor */
    struct sockaddr_in servAddr; /* Server address */
    unsigned short servPort;     /* Server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char sendBuffer[SNDBUFSIZE];                /* Buffer to hold data sent across TCP */
    unsigned char recvBuffer[RCVBUFSIZE];     /* Buffer to hold data received */
    char schemaBuffer[SCHBUFSIZE];          /* Buffer to hold schema */
    unsigned int sendBufferLength, schemaLength;      /* Length of string to echo */
    int i;
    sensocol_packet_t packet;       /* Sensocol packet */
    md5_hash_t schema_md5;
    unsigned char access_token[16] = {0xbd, 0x3b, 0xf0, 0x4f, 0xe3, 0x99, 0xfc, 0x90, 0x14, 0xfa, 0x0d, 0xdb, 0x03, 0xf2, 0x75, 0x9b}; /* Used to authenticate to server */
    unsigned long access_key = 2134507778; /* Used to authenticate with server */
    ///////////////////////
    //  The AES 128 encryption key for this device
    unsigned char key[16]; //= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    for (i = 0; i < sizeof(key);i++) key[i] = i;
    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port> \n",
               argv[0]);
       exit(1);
    }

    servIP = argv[1];             /* First arg: server IP address (dotted quad) */

    servPort = atoi(argv[2]); /* Use given port, if any */

    ///////////////////////////////
    // Create the sensor packet
    // /////////////////////
    sensocol_packet_init(&packet, "test_sensor", strlen("test_sensor"));

    sensocol_packet_add_long(&packet, strlen("testLong"), "testLong", 1);
    sensocol_packet_add_long_long(&packet, strlen("testLong64"), "testLong64", 123456789000);
    sensocol_packet_add_string(&packet,strlen("testString"), "testString", "testing", strlen("testing"));
    sensocol_packet_add_bytes(&packet, strlen("testBytes"), "testBytes", "foo foo", strlen("foo foo"));
    sensocol_packet_add_float(&packet, strlen("afloat"), "afloat", 123.34);
    sensocol_packet_add_double(&packet, strlen("testDouble"), "testDouble", 123.321);
    // Create the schema
    // NOTE: The schema in an embedded system can be HARD CODED as a constant.  It need not be generated dynamically.
    schemaLength = sensocol_schema_from_packet(&packet, schemaBuffer);

    // Get schema MD5.  This too can/should be hard-coded
    md5(schema_md5, schemaBuffer, schemaLength * 8);

    // For test/debugging purposes dump the MD5 & Schema
    dump(stdout, schema_md5, 16);
    printf("Schema is: %s\n", schemaBuffer);

    // For test/debugging purposes, dump the encoded data
    sensocol_print(&packet);

    // Generate the actual packet to send across the wire
    //sendBufferLength = sensocol_generate_aes128_post_packet(&packet, access_key, access_token, schema_md5, sendBuffer, key);

    sendBufferLength = sensocol_generate_post_packet(&packet, access_key, access_token, schema_md5, sendBuffer);
    printf("Sending Over Wire:\n");
    dump(stdout, sendBuffer, sendBufferLength);

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));     /* Zero out structure */
    servAddr.sin_family      = AF_INET;             /* Internet address family */
    servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    servAddr.sin_port        = htons(servPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("connect() failed");


    /* Send the string to the server */
    if (send(sock, sendBuffer, sendBufferLength, 0) != sendBufferLength)
        DieWithError("send() sent a different number of bytes than expected");

    RecvBytes(sock, recvBuffer, 1);
    /* Receive the same string back from the server */

    switch(recvBuffer[0]){
      case SENSOCOL_RESPONSE_OK:
        printf("Data Sent Successfully\n");
        break;
      case SENSOCOL_REQUEST_SCHEMA:
        printf("Schema Request\n");
        sendBufferLength = sensocol_schema_generate_response(&packet, schemaBuffer, schemaLength, sendBuffer);
        if (send(sock, sendBuffer, sendBufferLength, 0) != sendBufferLength)
          DieWithError("send() sent a different number of bytes than expected");
        RecvBytes(sock, recvBuffer, 1);

        switch(recvBuffer[0]){
          case SENSOCOL_RESPONSE_OK:
            printf("Data Sent Successfully\n");
            break;
          case SENSOCOL_RESPONSE_SCHEMA_ERROR:
            DieWithError("Error with Schema\n");
            break;
          case SENSOCOL_RESPONSE_PARSE_ERROR:
            DieWithError("Error parsing packet\n");
            break;
        }
        break;
      case SENSOCOL_RESPONSE_AUTH_ERROR:
        printf("Invalid User Key or Token\n");
        break;
      default:
        printf("Unknown Response");
        break;
    }
    
    close(sock);
    exit(0);
}

