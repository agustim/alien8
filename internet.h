//---------------------------//
//         Constants         //
//---------------------------//
#define INET_SUCCESS 0
#define INET_INTERNET_FAILED 1
#define INET_CONNECT_FAILED 2
#define INET_OPENREQUEST_FAILED 3
#define INET_SENDREQUEST_FAILED 4

//---------------------------//
//         Type defs         //
//---------------------------// 



//---------------------------//
//         Variables         //
//---------------------------// 


//---------------------------//
//    Function prototypes    //
//---------------------------// 
extern int inet_connect(char *);
extern void inet_close(void);
extern int inet_sendscore(char *,char *);


