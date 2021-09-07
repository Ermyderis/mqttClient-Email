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

#include "database.h"
#include "getvaluesconfig.h"

#define CONFIGFILE "/etc/config/mqttconfig"


void check_comparison_type(char *comparisontype, char *key, char *casetype, char *casevalue, char *payload){
	if(atoi(casetype) == 0){
		if(atoi(comparisontype) == 0){
			printf("!=\n");
			if(strcmp(key, casevalue) != 0){
				printf("%s\n", payload);
			}
		}
		else{
			if(atoi(comparisontype) == 1){
				printf("==\n");
				if(strcmp(key, casevalue) == 0){
				printf("%s\n", payload);
				}
			}
		}
	}
	if(atoi(casetype) == 1){
		if(atoi(comparisontype) == 0){
				printf("!=\n");
				if(atoi(key) != casevalue){
					printf("%s\n", payload);
				}
			}
			else{
				if(atoi(comparisontype) == 1){
					printf("==\n");
					if(atoi(key) == casevalue){
						printf("%s\n", payload);
				}
				}
				else{
					if(atoi(comparisontype) == 2){
						printf("<=\n");
						if(atoi(key) <= casevalue){
							printf("%s\n", payload);
						}
					}
					else{
						if(atoi(comparisontype) == 3){
							printf(">=\n");
							if(atoi(key) >= casevalue){
									printf("%s\n", payload);
								}
						}
						else{
							if(atoi(comparisontype) == 4){
								printf("<\n");
								if(atoi(key) < casevalue){
									printf("%s\n", payload);
								}
							}
							else{
								if(atoi(comparisontype) == 5){
									printf(">\n");
									if(atoi(key) > casevalue){
										printf("%s\n", payload);
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
