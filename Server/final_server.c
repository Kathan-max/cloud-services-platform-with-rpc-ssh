#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "youtube_api.h"
#include "distribution.h"



#define MAX_PASSWORD_LENGTH 100
#define MAX_API_KEYS 100
#define MAX_API_KEY_LENGTH 9

typedef struct {
    char password[MAX_PASSWORD_LENGTH];
    char api_keys[MAX_API_KEYS][MAX_API_KEY_LENGTH];
    int num_keys;
} UserData;




void add_api_key_to_user_file(const char *username, const char *new_api_key) {
    printf("Making Changes in this file: %s.xml\n", username);
    char filename[200];
    snprintf(filename, sizeof(filename), "User/%s.xml", username);

    // Load the XML document
    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        printf("Error: could not parse file %s\n", filename);
        return;
    }

    // Get the root element
    xmlNode *root_element = xmlDocGetRootElement(doc);
    xmlNode *apiKeysElement = NULL;

    // Find the APIKeys node
    for (xmlNode *current_element = root_element->children; current_element; current_element = current_element->next) {
        if (current_element->type == XML_ELEMENT_NODE && strcmp((char *)current_element->name, "APIKeys") == 0) {
            apiKeysElement = current_element;
            break;
        }
    }

    // Check if APIKeys node exists, if not create it
    if (apiKeysElement == NULL) {
        apiKeysElement = xmlNewChild(root_element, NULL, BAD_CAST "APIKeys", NULL);
        xmlNodeAddContent(apiKeysElement, BAD_CAST "    "); // Add newline and indentation for the first key
    }

    int num_keys = 0;
    // Count existing keys and check if already at MAX_API_KEYS
    for (xmlNode *key_element = apiKeysElement->children; key_element; key_element = key_element->next) {
        if (key_element->type == XML_ELEMENT_NODE && strcmp((char *)key_element->name, "Key") == 0) {
            num_keys++;
        }
    }

    if (num_keys >= MAX_API_KEYS) {
        printf("Error: Maximum number of API keys reached for user %s\n", username);
    } else {
        // Add new API key with proper indentation and line breaks
        xmlNode *newKeyNode = xmlNewTextChild(apiKeysElement, NULL, BAD_CAST "Key", BAD_CAST new_api_key);
        xmlNodeAddContent(newKeyNode, BAD_CAST ""); // Add newline and indentation after the key value

        // Reformat the closing tag of APIKeys
        xmlNodeAddContent(apiKeysElement, BAD_CAST "\n    "); // Indentation for closing tag
    }

    // Save the document
    int result = xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
    if (result == -1) {
        printf("Error: could not save updated XML file %s\n", filename);
    }
    printf("%d\n",result);

    // Free the document
    xmlFreeDoc(doc);
}





// Function to read the XML file and extract user data
static void read_user_data_from_xml(const char *filename, UserData *user_data) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *current_element = NULL;

    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        printf("in read Error: could not parse file %s\n", filename);
        return;
    }

    root_element = xmlDocGetRootElement(doc);

    for (current_element = root_element->children; current_element; current_element = current_element->next) {
        if (current_element->type == XML_ELEMENT_NODE) {
            if (strcmp((char *)current_element->name, "Password") == 0) {
                strncpy(user_data->password, (char *)xmlNodeGetContent(current_element), MAX_PASSWORD_LENGTH);
            } else if (strcmp((char *)current_element->name, "APIKeys") == 0) {
                xmlNode *key_element = current_element->children;
                int key_index = 0;
                while (key_element && key_index < MAX_API_KEYS) {
                    if (key_element->type == XML_ELEMENT_NODE && strcmp((char *)key_element->name, "Key") == 0) {
                    	 printf("Keycontent: %s\n",xmlNodeGetContent(key_element));
                        strncpy(user_data->api_keys[key_index], (char *)xmlNodeGetContent(key_element), MAX_API_KEY_LENGTH);
                        key_index++;
                    
                    }
                    key_element = key_element->next;
		//key_index++;
                    
                }
//                key_index++;
                user_data->num_keys = key_index;
            }
        }
    }

    xmlFreeDoc(doc);
}




static int check_credentials_and_get_api_keys(const char *username, const char *password, UserData *user_data) {
    char filename[200];
    snprintf(filename, sizeof(filename), "User/%s.xml", username);
    printf("File name %s\n",filename);
    read_user_data_from_xml(filename, user_data);

    if (strcmp(user_data->password, password) == 0) {
        return 1;  // Password matches
    }

    return 0;  // Password does not match or not found
}




char* generate_random_api_key() {
    
    char *str = (char *)malloc(MAX_API_KEY_LENGTH + 1); // +1 for the null terminator

    if (str == NULL) {

        return NULL;
    }

    // Seed the random number generator

    srand((unsigned int)time(NULL));

    for (int i = 0; i < MAX_API_KEY_LENGTH; ++i) {
        int randomCharType = rand() % 2;
        if (randomCharType == 0) {
            // Generate a lowercase letter (ASCII: 97 to 122)
            str[i] = (char)((rand() % (122 - 97 + 1)) + 97);
        } else {
            // Generate an uppercase letter (ASCII: 65 to 90)
            //str[i] = (char)((rand() % (57 - 48 + 1)) + 48);
            str[i] = (char)((rand() % (122 - 97 + 1)) + 97);
        }
    }
    str[MAX_API_KEY_LENGTH] = '\0'; // Null-terminate the string
    return str;
}


static xmlrpc_value *receive_credentials(xmlrpc_env * const envP,
                                         xmlrpc_value * const paramArrayP,
                                         void * const serverContextP) {
    char *received_username;
    char *received_password;

    xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &received_username, &received_password);
    if (envP->fault_occurred) {
        return NULL;
    }

    UserData user_data;
    int credentials_ok = check_credentials_and_get_api_keys(received_username, received_password, &user_data);


    if (credentials_ok) {
        xmlrpc_value *api_key_array = xmlrpc_array_new(envP);
        for (int i = 0; i < user_data.num_keys; ++i) {
            xmlrpc_array_append_item(envP, api_key_array, xmlrpc_build_value(envP, "s", user_data.api_keys[i]));
        }

        return api_key_array;  // Returning the array of API keys
    } else {
        return xmlrpc_build_value(envP, "s", "-1");
    }
}

static xmlrpc_value *receive_youtube_credentials(xmlrpc_env * const envP,
                                                 xmlrpc_value * const paramArrayP,
                                                 void * const serverContextP) {
    
    printf("Called the receive youtube method\n");
    char *t_api_key; // renamed from api_key
    char *api_key; // new variable for your_api_key
    char *username;
    xmlrpc_value *video_id_array;
    int num_videos;

    // Decompose the input parameters
    xmlrpc_decompose_value(envP, paramArrayP, "(sssA)", &t_api_key, &api_key, &username, &video_id_array);
    if (envP->fault_occurred)
        return NULL;

    // Read user data from XML file
    UserData user_data;
    char filename[200];
    snprintf(filename, sizeof(filename), "User/%s.xml", username);
    read_user_data_from_xml(filename, &user_data);

    // Check if provided API key matches any of the user's API keys
    int key_match = 0;
    //printf("\n--------------\n");
    for (int i = 0; i < user_data.num_keys; ++i) {
        //printf("%s---%s\n",user_data.api_keys[i],api_key);
        if (strncmp(user_data.api_keys[i], api_key,9) == 0) {
            key_match = 1;
            break;
        }
    }

    if (!key_match) {
        printf("Error: Wrong API key provided for user %s\n", username);
        return xmlrpc_build_value(envP, "s", "Error: Wrong API key provided");
    }

    num_videos = xmlrpc_array_size(envP, video_id_array);
    
    const char *video_ids[num_videos];
    for (int i = 0; i < num_videos; ++i) {
        xmlrpc_value *video_id_value;
        xmlrpc_array_read_item(envP, video_id_array, i, &video_id_value);
        xmlrpc_read_string(envP, video_id_value, &video_ids[i]);
        xmlrpc_DECREF(video_id_value);
    }

    char inputFiles[256];
    snprintf(inputFiles, sizeof(inputFiles), "data_files/%s_youtube_output.csv", username);

    fetchAndSaveYouTubeData(api_key, video_ids, num_videos, username);
    make_table(username);
    splitFile(inputFiles, username, 2);

    // Cleanup
    for (int i = 0; i < num_videos; ++i) {
        free((char *)video_ids[i]);
    }
    xmlrpc_DECREF(video_id_array);



    return xmlrpc_build_value(envP, "s", "Success");
}



static xmlrpc_value *generate_api_key(xmlrpc_env * const envP,
                                      xmlrpc_value * const paramArrayP,
                                      void * const serverContextP) {
    char *username;
    xmlrpc_decompose_value(envP, paramArrayP, "(s)", &username);
    if (envP->fault_occurred) {
        return NULL;
    }

    //char new_api_key[MAX_API_KEY_LENGTH];
    char *new_api_key = generate_random_api_key();
    
    printf("Generated new API_KEY: %s\n",new_api_key);

    add_api_key_to_user_file(username, new_api_key);
    
    UserData user_data;
    read_user_data_from_xml(username, &user_data);
    
    //strncpy(user_data.api_keys[user_data.num_keys], new_api_key, MAX_API_KEY_LENGTH);
    //user_data.num_keys++;

    return xmlrpc_build_value(envP, "s", new_api_key);
}


static xmlrpc_value *call_method(xmlrpc_env * const envP,
                                      xmlrpc_value * const paramArrayP,
                                      void * const serverContextP) {
    char *username;
    char *api_key;
    char *method_name;
    char *combined_buffer;
    xmlrpc_decompose_value(envP, paramArrayP, "(sss)", &username,&api_key,&method_name);
    if (envP->fault_occurred) {
        return NULL;
    }

// Read user data from XML file
    UserData user_data;
    char filename[200];
    snprintf(filename, sizeof(filename), "User/%s.xml", username);
    read_user_data_from_xml(filename, &user_data);

    // Check if provided API key matches any of the user's API keys
    int key_match = 0;
    //printf("\n--------------\n");
    for (int i = 0; i < user_data.num_keys; ++i) {
        //printf("%s---%s\n",user_data.api_keys[i],api_key);
        if (strncmp(user_data.api_keys[i], api_key,9) == 0) {
            key_match = 1;
            break;
        }
    }

    if (!key_match) {
        printf("Error: Wrong API key provided for user %s\n", username);
        return xmlrpc_build_value(envP, "s", "Error: Wrong API key provided");
    }
    
    if (strcmp("generate_summ",method_name) == 0){
       char distfilename[200];
       snprintf(distfilename, sizeof(distfilename), "%s_distribution_log.txt", username);
       char command1[256];
       snprintf(command1, sizeof(command1), "python3 data_ana2.py %s",distfilename); // Use snprintf safely
       system(command1);
       
       
       char pat1name[200];
       snprintf(pat1name, sizeof(pat1name), "%s_data_an1.txt", username);
       char pat2name[200];
       snprintf(pat2name, sizeof(pat2name), "%s_data_an2.txt", username);
       
       FILE *file1, *file2;
       char *buffer1, *buffer2;
       long file_size1, file_size2;
       file1 = fopen(pat1name, "rb");
    
       if (file1 == NULL) {
         perror("Error opening file");
       }

      fseek(file1, 0, SEEK_END);
      file_size1 = ftell(file1);
      fseek(file1, 0, SEEK_SET);

      buffer1 = (char *)malloc(file_size1 + 1);

      if (buffer1 == NULL) {
         perror("Memory allocation error");
         fclose(file1);
      }

      fread(buffer1, 1, file_size1, file1);
      buffer1[file_size1] = '\0';
      fclose(file1);
      
      char command2[256];
      sprintf(command2, "echo \"%s\" | ssh cloud-lab@10.20.24.81 \"cat > /home/cloud-lab/Desktop/Cloud_C_proj/msg.txt\"", distfilename);
      system(command2);
      
      sleep(200);
      file2 = fopen(pat2name, "rb");
    
       if (file2 == NULL) {
         perror("Error opening file");
       }

      fseek(file2, 0, SEEK_END);
      file_size2 = ftell(file2);
      fseek(file2, 0, SEEK_SET);

      buffer2 = (char *)malloc(file_size2 + 1);

      if (buffer2 == NULL) {
         perror("Memory allocation error");
         fclose(file2);
      }

      fread(buffer2, 1, file_size2, file2);
      buffer2[file_size2] = '\0';
      fclose(file2);

      combined_buffer = (char *)malloc(file_size1 + file_size2 + 2); // 2 for the newline and null-terminator

    // Concatenate buffer1, newline, buffer2, and null-terminate
      sprintf(combined_buffer, "%s\n%s", buffer1, buffer2);
    }
    


    return xmlrpc_build_value(envP, "s", combined_buffer);
}



int main(void) {
    xmlrpc_env env;
    xmlrpc_server_abyss_parms serverParams;
    xmlrpc_registry *registryP;

    LIBXML_TEST_VERSION

    xmlrpc_env_init(&env);
    registryP = xmlrpc_registry_new(&env);

    xmlrpc_registry_add_method(&env, registryP, NULL, "receive_credentials", &receive_credentials, NULL);
    xmlrpc_registry_add_method(&env, registryP, NULL, "generate_api_key", &generate_api_key, NULL);
    xmlrpc_registry_add_method(&env, registryP, NULL, "receive_youtube_credentials", &receive_youtube_credentials, NULL);
    xmlrpc_registry_add_method(&env, registryP, NULL, "call_method", &call_method, NULL);

    serverParams.config_file_name = NULL;
    serverParams.registryP = registryP;
    serverParams.port_number = 8000;

    printf("Running XML-RPC server on port %d...\n", serverParams.port_number);
    xmlrpc_server_abyss(&env, &serverParams, XMLRPC_APSIZE(port_number));

    xmlrpc_env_clean(&env);
    xmlrpc_registry_free(registryP);
    xmlCleanupParser();

    return 0;
}
