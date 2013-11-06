/* header file for communications between server and client */

/**** DEFINES ****/
/* handshakes */
#define FRIENDLY 3.14159
#define DEADLY   2.71828
#define GOODBYE  2.414
/* server socket name */
#define ADDRESS "shik?i"

/**** TYPEDEFS ****/
typedef struct HANDSHAKE {
    double message;
} HANDSHAKE;
