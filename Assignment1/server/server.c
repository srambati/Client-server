/*Program Written by A.SRINIVAS REDDY      ROLL NO:201006001     MTECH-CSIS
Program Secure file transfer*/
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include <netdb.h>
#include<fcntl.h>
#include <sys/types.h>
#include<sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include "sha1.h"

/* Global constants */
#define Q_SIZE 5
#define MAX_LEN 1024
#define MAX_SIZE 80
#define STACK_SIZE 10000
#define NOT_EXIST 0xFFFF
#define LARGE 99
#define MAX_ITERATION 10  // Max tests in Miller-Robin Primality Test.
#define div /
#define mod %
#define and &&
#define true 1
#define false 0
#define DEFAULT_SERVER "192.168.1.241"

#define PUBKEY 10  /* Public key */
#define SIGNEDMSG 20     /* Message along with signature sent to server */
#define VERSTATUS 30  /* Signature verification status by server */

/* Define the header of a message structure */
typedef struct {
 int opcode;  
 int src_addr;
 int dest_addr;
 } Hdr;

/* Define the header of signature of the message */
typedef struct {
 int r;
 int s;
}Signature;
 
/* Define the body of a message */
typedef struct {
 Hdr hdr;                    /* Header for a message */
 char ID[MAX_SIZE];          /* The identifier of a user */
 long int p;                      /* A large prime */
 long int q;                      /* A prime factor of (p-1)*/
 long int g;                      /* g = h(p−1)/q mod p, with 1 < h < (p − 1) and g > 1 */
 long int y;                      /* A public key generated by user */
 char hash_algo[MAX_SIZE];   /*Type of hash algorithm is used */
 char plaintext[MAX_LEN];    /* Contains a plaintext message*/
 Signature sign;             /* Contains the signature on a plaintext message*/
 int ver_status;             /* Successful or unsuccessful result in signature verification */
 int dummy;                  /*dummy variable is used when necessary */
}Msg;

typedef unsigned long int UINT4;

/* Data structure for MD5 (Message Digest) computation */
typedef struct {
  UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
  UINT4 buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;




typedef struct{
	int top;
	char c[STACK_SIZE];
}stack;

typedef short boolean;
long int mul_inverse=0;
long int gcd_value;
char hash[90];
stack s;

int SERVICE_PORT;
int print_flag=0,shasum=0;


/* Function prototypes */
int startServer ( );
void serverLoop ( int );
void Talk_to_client ( int );
boolean MillerRobinTest(long int n, int iteration);
long int ModPower(long int x, long int e, long int n);
void extended_euclid(long int A1, long int A2, long int A3, long int B1, long int B2,long int B3);
void calculating_w(long int *cw,long int *cs,long int *cq);
void calculating_u1(long int *u1,char *plaintext,long int *cw,long int *cq,int opt);
void calculating_u2(long int *u2,long int *cr,long int *cw,long int *cq);
void calculting_v(long int *v,long int *cg,long int *u1,long int *publickey,long int *u2,long int *cp,long int *cq);
void decimal_to_binary(long int n, char str[]);
void reverse_string(char x[]);
long int modulo ( long int x, long int n );
long int gcd(long int a, long int b);


static void Transform ();

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z))) 

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

void MD5Init (mdContext)
MD5_CTX *mdContext;
{
  mdContext->i[0] = mdContext->i[1] = (UINT4)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT4)0x67452301;
  mdContext->buf[1] = (UINT4)0xefcdab89;
  mdContext->buf[2] = (UINT4)0x98badcfe;
  mdContext->buf[3] = (UINT4)0x10325476;
}

void MD5Update (mdContext, inBuf, inLen)
MD5_CTX *mdContext;
unsigned char *inBuf;
unsigned int inLen;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT4)inLen << 3);
  mdContext->i[1] += ((UINT4)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
                (((UINT4)mdContext->in[ii+2]) << 16) |
                (((UINT4)mdContext->in[ii+1]) << 8) |
                ((UINT4)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

void MD5Final (mdContext)
MD5_CTX *mdContext;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
            (((UINT4)mdContext->in[ii+2]) << 16) |
            (((UINT4)mdContext->in[ii+1]) << 8) |
            ((UINT4)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}

/* Basic MD5 step. Transform buf based on in.
 */
static void Transform (buf, in)
UINT4 *buf;
UINT4 *in;
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

/* -- include the following file if the file md5.h is separate -- */
/* #include "md5.h" */

/* Prints message digest buffer in mdContext as 32 hexadecimal digits.
   Order is from low-order byte to high-order byte of digest.
   Each byte is printed with high-order hexadecimal digit first.
 */
static void MDPrint (MD5_CTX *mdContext)
{
  int i;
  char str[20][4],h[50];
  for (i = 0; i < 16; i++)
  {
    printf ("%02x", mdContext->digest[i]);
    sprintf (str[i],"%02x", mdContext->digest[i]);
  }
  printf("\n");
  for(i=0;i<16;i++)
    strcat(h,str[i]);
  strcpy(hash,h);
}
static void MDString (char *inString)
{
  MD5_CTX mdContext;
  unsigned int len = strlen (inString);

  MD5Init (&mdContext);
  MD5Update (&mdContext, inString, len);
  MD5Final (&mdContext);
  MDPrint (&mdContext);
  //printf (" \"%s\"\n\n", inString);
}














/* Start the server: socket(), bind() and listen() */
int startServer ()
{
   int sfd;                    /* for listening to port PORT_NUMBER */
   struct sockaddr_in saddr;   /* address of server */
   int status;

   /* Request for a socket descriptor */
   sfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sfd == -1) {
      fprintf(stderr, "*** Server error: unable to get socket descriptor\n");
      exit(1);
   }
   printf("Enter the Port Number:");
   scanf("%d",&SERVICE_PORT);
   /* Set the fields of server's internet address structure */
   saddr.sin_family = AF_INET;            /* Default value for most applications */
   saddr.sin_port = htons(SERVICE_PORT);  /* Service port in network byte order */
   saddr.sin_addr.s_addr = INADDR_ANY;    /* Server's local address: 0.0.0.0 (htons not necessary) */
   bzero(&(saddr.sin_zero),8);            /* zero the rest of the structure */

   /* Bind the socket to SERVICE_PORT for listening */
   status = bind(sfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
   if (status == -1) {
      fprintf(stderr, "*** Server error: unable to bind to port %d\n", SERVICE_PORT);
      exit(2);
   }

   /* Now listen to the service port */
   status = listen(sfd,Q_SIZE);
   if (status == -1) {
      fprintf(stderr, "*** Server error: unable to listen\n");
      exit(3);
   }

   fprintf(stderr, "+++ Server successfully started, listening to port %d\n", SERVICE_PORT);
   return sfd;
}
/* Accept connections from clients, spawn a child process for each request */
void serverLoop ( int sfd )
{
    int cfd,size;                    /* for communication with clients */
    struct sockaddr_in caddr;        /* address of client */
    size=sizeof(struct sockaddr_in);
    while (1) 
    {
              /* accept connection from clients */
        cfd = accept(sfd, (struct sockaddr *)&caddr, &size);
        if (cfd == -1) 
        {
           fprintf(stderr, "*** Server error: unable to accept request ***\n");
           continue;
        }
        fprintf(stderr, "**** Connected with %hd ***\n", inet_ntoa(caddr.sin_addr));
        /* fork a child to process request from client */
        if (!fork()) 
        {
           Talk_to_client (cfd);
           fprintf(stderr, "**** Closed connection with %d ***\n", inet_ntoa(caddr.sin_addr));
           close(cfd);
           exit(0);
        }
        /* parent (server) does not talk with clients */
        close(cfd);
        /* parent waits for termination of child processes */
        while (waitpid(-1,NULL,WNOHANG) > 0);     
    }
}
/* Interaction of the child process with the client */
void Talk_to_client ( int cfd )
{
   long int cp,cq,cg,publickey,cr,cs,cw,u1,u2,v,result,shasum;
   int src_addr, dest_addr,nbytes,status;
   char hashal[20],id[20],plaintext[MAX_LEN];
   Msg send_msg;
   Msg recv_msg;
   int flag;
   dest_addr = inet_addr("DEFAULT_SERVER");
   src_addr = inet_addr("192.168.1.245");
   
   //printf("server side");  
   /* Wait for responses from the clent (Alice, User A) */
   while ( 1 ) {

   /* receive messages from server */
   nbytes = recv(cfd, &recv_msg, sizeof(Msg), 0);
   //printf("opcode=%d",recv_msg.hdr.opcode);
   if (nbytes == -1) {
      fprintf(stderr, "*** Server error: unable to receive ***\n");
   }
   //printf("SErver received");
   flag = 1;   
   switch(recv_msg.hdr.opcode) 
   {
       case PUBKEY:
                     
                      send_msg.hdr.opcode=SIGNEDMSG;
                      cp=recv_msg.p;
                      cq=recv_msg.q;
                      cg=recv_msg.g;
                      strcpy(id,recv_msg.ID);
                      publickey=recv_msg.y;
                      strcpy(hashal,recv_msg.hash_algo); 
                      printf("Received Public Global Elements\n");
                      printf("p=%ld q=%ld g=%ld pub(y)=%ld\n",cp,cq,cg,publickey);
                    break;

       case VERSTATUS: 
                         printf("\n-------------------------------------------\n");
                         send_msg.hdr.opcode=SIGNEDMSG;  
                         cr=recv_msg.sign.r;
                         cs=recv_msg.sign.s;
                         cg=recv_msg.g;
                         printf("Received Signature\n");
                         printf("r=%ld s=%ld\n\n",cr,cs);
                         strcpy(plaintext,recv_msg.plaintext);
                         calculating_w(&cw,&cs,&cq);
                         if(mul_inverse!=NOT_EXIST)
                         {
                           if(recv_msg.dummy==1)
                           {    
                              calculating_u1(&u1,plaintext,&cw,&cq,1);
                              printf("MD5 is used\n");
                           } 
                           else
                           {  
                             calculating_u1(&u1,plaintext,&cw,&cq,2);
                             printf("SHA-1 is used\n");
                           }
                           calculating_u2(&u2,&cr,&cw,&cq);
                           calculting_v(&v,&cg,&u1,&publickey,&u2,&cp,&cq);
                           printf("\nw=%ld u1=%ld u2=%ld v=%ld\n",cw,u1,u2,v);
                           strcpy(send_msg.ID,"BOB");
                           if(cr==v)
                           {
                              send_msg.ver_status=1;
                              printf("\n****Signature Verified****\n");
                              printf("v=%ld r=%ld\n",v,cr);
                           }
                           else
                           {
                              send_msg.ver_status=0;
                              printf("\n****Signature not Verified****\n");
                              printf("\nv=%ld r=%ld\n",v,cr);
                           }
                      }
   }                         
   if(flag!=0) 
   {
   /* send the user's response to the server */
          status = send(cfd, &send_msg, sizeof(Msg), 0);
          if(status==-1) 
          {
                fprintf(stderr, "*** Client error: unable to send ***\n");
                return;
          }
   }
  }
}

void calculating_w(long int *cw,long int *cs,long int *cq)
{
     extended_euclid(1,0,*cq,0,1,*cs);
     if(mul_inverse==NOT_EXIST)
     {
          printf("\nInverse does not exist\n\n");
     }
     else if(mul_inverse<0)
        *cw=-*cq-mul_inverse;
     else
        *cw=mul_inverse;
}

void calculating_u1(long int *u1,char *plaintext,long int *cw,long int *cq,int opt)
{
 
    long int mdsum=0,result;
    int i=0,len;      
    if(opt==1) 
    {
        MDString(plaintext);
        len=strlen(hash);
        while(i<len)
        {
           mdsum=mdsum+hash[i];
           i++;
        }
    }
    else 
    {
          SHA1Context sha;
	  unsigned temp[5];
    	  int j;
	  char hex_output[5*8+1];
	  SHA1Reset(&sha);
    	  SHA1Input(&sha, (const unsigned char *)plaintext, strlen(plaintext));
	  for(i = 0; i < 5 ; i++)
	  {
			sprintf(hex_output+ i * 8,"%X", sha.Message_Digest[i]);
	  }
	  printf("Hash = %s\n",hex_output);
          for(i=0;i<strlen(hex_output);i++)
              mdsum=mdsum+hex_output[i];
     }
    result=mdsum%(*cq);
    result=result*((*cw)%(*cq));
    *u1=result%(*cq);
}
void calculating_u2(long int *u2,long int *cr,long int *cw,long int *cq)
{
     long int result;
     result=(*cr)%(*cq);
     result=result*((*cw)%(*cq));
     *u2=result%(*cq);
} 

void calculting_v(long int *v,long int *cg,long int *u1,long int *publickey,long int *u2,long int *cp,long int *cq)
{
      long int k1,k2;
      k1=ModPower(*cg,*u1,*cp);
      k2=ModPower(*publickey,*u2,*cp);
      k1=(k1*k2)%(*cp);
      *v=k1%(*cq);   
}


/*Convert decimal to binary format */
void decimal_to_binary(long int n, char str[])
{
	// n is the given decimal integer.
	// Purpose is to find the binary conversion
	// of n.
        // Initialise the stack.
	
	 int r;
	 s.top=0;
	
	while(n != 0)

	{
		r= n mod 2;
		s.top++;
		if(s.top >= STACK_SIZE)
		{
			printf("\nstack overflown!\n");
			return;
		}
		s.c[s.top]= r + 48;
		if(print_flag)
		 printf("\n s.c[%d]= %c\n",s.top,s.c[s.top]);
		n=n div 2;

	}

	while(s.top)
	{
          *str++=s.c[s.top--];
	}
	*str='\0';
return;
}


// Algorithm: reverse a string.
void reverse_string(char x[])
{
 int n=strlen(x)-1;
 int i=0;
 char temp[STACK_SIZE];

 for(i=0;i<=n;i++)
	 temp[i]= x[n-i];


 for(i=0;i<=n;i++)
	 x[i]=temp[i];
}

/* modulo operation */
long int modulo ( long int x, long int n )
{
if ( x >= 0 ) 
  x = x % n;
 
 while ( x < 0 ) {
   x = - x;
   x = (( n - 1) * ( x % n )) % n;
 }
 return x;
}

/* Algorithm: Modular Power: x^e(mod n) using 
   the repeated square-and-multiply algorithm */
long int ModPower(long int x, long int e, long int n)
{
	// To calculate y:=x^e(mod n).
        //long y;
        long int y;
	long int t;
        int i;
	int BitLength_e;
	char b[STACK_SIZE];
      
        //printf("e(decimal) = %ld\n",e);
	decimal_to_binary(e,b);
	if(print_flag)
	 printf("b = %s\n", b);
	BitLength_e = strlen(b);
        
	y = x;

	reverse_string(b);

	for(i = BitLength_e - 2; i >= 0 ; i--)
	{
		if(print_flag)
		 printf("\nb[%d]=%c", i, b[i]);
		if(b[i] == '0')
			t = 1;
		else t = x;
                y = y * y;
                y = modulo (y, n);

		y = y*t;
                y = modulo (y, n);
	}

	return y;
        
} 



/* Euclid's Extended GCD algorithm to compute the modular inverse of an element */
void extended_euclid(long int A1, long int A2, long int A3, long int B1, long int B2,long int B3)
{
	long int Q;
	long int T1,T2,T3;

	if(B3==0){
		  gcd_value= A3;
		  mul_inverse= NOT_EXIST;
		  return;
	}

	if(B3==1){
		  gcd_value= B3;
		  mul_inverse= B2;
		  return;
	}

	Q=(int)(A3/B3);

	T1=A1-Q*B1;
	T2=A2-Q*B2;
	T3=A3-Q*B3;

	A1=B1;
	A2=B2;
	A3=B3;

	B1=T1;
	B2=T2;
	B3=T3;

	extended_euclid(A1,A2,A3,B1,B2,B3);

}


int main ()
{
   int sfd;
   printf("******* This is the server side *****\n\n");
   sfd = startServer();
   serverLoop(sfd);
   return 0;
}
/*** End of server.c ***/

