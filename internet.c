#include "internet.h"
#include "allegro.h"
#include "stdio.h"

// USE HERE THE PROPER DEFINITION FOR THE OS YOU ARE COMPILING DEFLEKTOR ON
//#define INTT_WIN // Windows
//#define INTT_MAC // Macintosh
#define INTT_LNX // Linux
//#define INTT_OTR // Other

#ifdef INTT_WIN
// Windows internet functions **************************************************
#include "winalleg.h"
#include "wininet.h"
HINTERNET internet;
HINTERNET connection;

int inet_connect(char *server)
{
	int result=INET_SUCCESS;

	// Attempt to connect to internet
	internet = InternetOpen("InetURL/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if(internet!=NULL)
	{
		// Attempt to open connection
		connection = InternetConnect(internet, server, 80, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
		if(connection==NULL)
		{
			result = INET_CONNECT_FAILED;
		}
	}
	else
	{
		result = INET_INTERNET_FAILED;
	}

	return result;
}

void inet_close(void)
{
	// Close connections
	if(connection!=NULL) InternetCloseHandle(connection);
	if(internet!=NULL) InternetCloseHandle(internet);
}

int inet_sendscore(char *name, char *scorecode)
{
	char url[1000];
	int result=INET_SUCCESS;
	HINTERNET data;

	// Create request URL
	sprintf(url,"/highscore.php?yourname=%s&code=%s",name,scorecode);

	// Open request
	data = HttpOpenRequest(connection, "GET", url, "HTTP/1.0", NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION, 0 );
	if(data!=NULL)
	{
		// Send data
		if(HttpSendRequest(data, NULL, 0, NULL, 0)) InternetCloseHandle(data);
		else result = INET_SENDREQUEST_FAILED;
	}
	else
	{
		result = INET_OPENREQUEST_FAILED;
	}

	return(result);

}
#endif

#ifdef INTT_MAC
// Macintosh internet functions ************************************************
int inet_connect(char *server)
{
 return INET_INTERNET_FAILED;
}

void inet_close(void)
{
}

int inet_sendscore(char *name, char *scorecode)
{
 return INET_INTERNET_FAILED;
}

#endif


#ifdef INTT_LNX
// Linux internet functions ****************************************************
// The linux version uses libcurl to work,
// Code by Christian Morales Vega
#include <curl/curl.h>
static CURL *internet;
static char *serv;
size_t static dummy(void *ptr, size_t size, size_t nmemb, void *stream){return size*nmemb;}

int inet_connect(char *server)
{
 int result=INET_SUCCESS;

 if(curl_global_init(CURL_GLOBAL_NOTHING))
 {
  return INET_INTERNET_FAILED;
 }

 internet = curl_easy_init();
 if(internet!=NULL)
 {
  // Attempt to open connection
  curl_easy_setopt(internet, CURLOPT_WRITEFUNCTION, &dummy);
  curl_easy_setopt(internet, CURLOPT_URL, server);
  curl_easy_setopt(internet, CURLOPT_CONNECT_ONLY, 1);
  CURLcode connection = curl_easy_perform(internet);
  serv = server;
  curl_easy_setopt(internet, CURLOPT_CONNECT_ONLY, 0);
  if(connection)
  {
   inet_close();
   result = INET_CONNECT_FAILED;
  }
 }
 else
 {
  inet_close();
  result = INET_INTERNET_FAILED;
 }

 return result;
}

void inet_close(void)
{
 if(internet!=NULL) curl_easy_cleanup(internet);
 curl_global_cleanup();
}

int inet_sendscore(char *name, char *scorecode)
{
 char url[1000];
 int result=INET_SUCCESS;

 // Create request URL
 sprintf(url,"%s/highscore.php?yourname=%s&code=%s",serv,name,scorecode);
 printf("%s\n",url);
 curl_easy_setopt(internet, CURLOPT_URL, url);

 // Send data
 CURLcode connection = curl_easy_perform(internet);
 if(connection)
 {
  result = INET_SENDREQUEST_FAILED;
 }

 return(result);
}
#endif


#ifdef INTT_OTR
// Other OS internet functions *************************************************
int inet_connect(char *server)
{
 return INET_INTERNET_FAILED;
}

void inet_close(void)
{
}

int inet_sendscore(char *name, char *scorecode)
{
 return INET_INTERNET_FAILED;
}
#endif
