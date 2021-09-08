#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <sqlite3.h>
#include <json-c/json.h>
#include <uci.h>
#include <curl/curl.h>

#include "database.h"
#include "getvaluesconfig.h"

#define CONFIGFILE "/etc/config/mqttconfig"

#define FROM    "dovydas0000@gmail.com"
#define TO		"dovydas0000@gmail.com"
#define CC		"dovydas0000@gmail.com"
 
static const char *payload_text[] = {
  "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
  "To: " TO "\r\n",
  "From: " FROM "(Example User)\r\n",
  "Cc: " CC "(Another example User)\r\n",
  "Subject: SMTP example message\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */ 
  "The body of the message starts here.\r\n",
  "\r\n",
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n",
  "Check RFC5322.\r\n",
  NULL
};
 
struct upload_status {
  int lines_read;
};
 
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = payload_text[upload_ctx->lines_read];
 
  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;
 
    return len;
  }
 
  return 0;
}

void mail_sending(char *payload){
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;
 
  upload_ctx.lines_read = 0;
 
  curl = curl_easy_init();
  if(curl) {
    /* This is the URL for your mailserver */ 
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com");

    /* Note that this option isn't strictly required, omitting it will result
     * in libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise,
     * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
     * details.
     */ 
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
 
    /* Add two recipients, in this particular case they correspond to the
     * To: and Cc: addressees in the header, but they could be any kind of
     * recipient. */ 
    recipients = curl_slist_append(recipients, TO);
    recipients = curl_slist_append(recipients, CC);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
    /* We're using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
	curl_easy_setopt(curl, CURLOPT_USERNAME, FROM);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "Writeyoupassword");

	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
 
    /* Send the message */ 
    res = curl_easy_perform(curl);
 
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
 
    /* Free the list of recipients */ 
    curl_slist_free_all(recipients);
 
    /* curl won't send the QUIT command until you call cleanup, so you should
     * be able to re-use this connection for additional messages (setting
     * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
     * curl_easy_perform() again. It may not be a good idea to keep the
     * connection open for a very long time though (more than a few minutes
     * may result in the server timing out the connection), and you do want to
     * clean up in the end.
     */ 
    curl_easy_cleanup(curl);
  }

}
 


void check_comparison_type(char *comparisontype, char *key, char *casetype, char *casevalue, char *payload){
	if(atoi(casetype) == 0){
		if(atoi(comparisontype) == 0){
			printf("!=\n");
			if(strcmp(key, casevalue) != 0){
				printf("%s\n", payload);
				mail_sending(payload);
				
			}
		}
		else{
			if(atoi(comparisontype) == 1){
				printf("==\n");
				if(strcmp(key, casevalue) == 0){
				printf("%s\n", payload);
				mail_sending(payload);
				}
			}
		}
	}
	if(atoi(casetype) == 1){
		if(atoi(comparisontype) == 0){
				printf("!=\n");
				if(atoi(key) != casevalue){
					printf("%s\n", payload);
					mail_sending(payload);
				}
			}
			else{
				if(atoi(comparisontype) == 1){
					printf("==\n");
					if(atoi(key) == casevalue){
						printf("%s\n", payload);
						mail_sending(payload);
				}
				}
				else{
					if(atoi(comparisontype) == 2){
						printf("<=\n");
						if(atoi(key) <= casevalue){
							printf("%s\n", payload);
							mail_sending(payload);
						}
					}
					else{
						if(atoi(comparisontype) == 3){
							printf(">=\n");
							if(atoi(key) >= casevalue){
									printf("%s\n", payload);
									mail_sending(payload);
								}
						}
						else{
							if(atoi(comparisontype) == 4){
								printf("<\n");
								if(atoi(key) < casevalue){
									printf("%s\n", payload);
									mail_sending(payload);
								}
							}
							else{
								if(atoi(comparisontype) == 5){
									printf(">\n");
									if(atoi(key) > casevalue){
										printf("%s\n", payload);
										mail_sending(payload);
									}
								}
							}
						}
					}
				}
			}
		} 	
}


void curl_send_message(struct Node *head, char *topic, char *payload){
	
	int rc = 0;
	struct Node *ptr = head;
	struct json_object *parsed_json;
	struct json_object *string;

    while(ptr != NULL) //all topics
        {
			if(strcmp(ptr->datatopics, topic) == 0){
				struct Case *ptr2 = ptr->chead;
				while(ptr2 != NULL){
					parsed_json = json_tokener_parse(payload);

					json_object_object_get_ex(parsed_json, ptr2->casekey, &string);
					printf("Key: %s\n", json_object_get_string(string));
					check_comparison_type(ptr2->casecomparisontype, json_object_get_string(string), ptr2->casetype, ptr2->casevalue, payload);

					ptr2 = ptr2->next;
				}
			}
			
            ptr = ptr->next;
        }
    
}
