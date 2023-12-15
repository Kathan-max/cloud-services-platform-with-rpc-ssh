#include "youtube_api.h"
#include <jansson.h>
#include <curl/curl.h>
#include <string.h>

// Define a buffer to accumulate the response
static char response_buffer[8192];  // Adjust the size based on your needs

// Define the size of the accumulated response
static size_t response_size = 0;

// Function definitions
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;

    // Check if the accumulated buffer size is sufficient
    if (response_size + realsize >= sizeof(response_buffer)) {
        fprintf(stderr, "Response buffer overflow.\n");
        return 0;  // Stop the transfer
    }

    // Copy the received data to the buffer
    memcpy(response_buffer + response_size, contents, realsize);
    response_size += realsize;

    return realsize;
}

void parseYouTubeJson(const char *json_response, FILE *output_file) {
    // Parse the JSON string
    json_t *root;
    json_error_t error;

    root = json_loads(json_response, 0, &error);

    // Check for parse errors
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return;
    }

    // Extract information from the parsed JSON
    const char *kind = json_string_value(json_object_get(root, "kind"));
    const char *etag = json_string_value(json_object_get(root, "etag"));

    // Access the "items" array
    json_t *items = json_object_get(root, "items");
    if (json_is_array(items)) {
        size_t index;
        json_t *value;

        // Iterate over each item in the array
        json_array_foreach(items, index, value) {
            // Extract information from each item
            const char *videoId = json_string_value(json_object_get(value, "id"));

            // Access the "snippet" object
            json_t *snippet = json_object_get(value, "snippet");
            if (json_is_object(snippet)) {
                const char *publishedAt = json_string_value(json_object_get(snippet, "publishedAt"));
                const char *title = json_string_value(json_object_get(snippet, "title"));
                const char *description = json_string_value(json_object_get(snippet, "description"));

                // Print or use the extracted information
                fprintf(output_file, "Video ID: %s\n", videoId);
                fprintf(output_file, "Published At: %s\n", publishedAt);
                fprintf(output_file, "Title: %s\n", title);
                fprintf(output_file, "Description: %s\n", description);
                fprintf(output_file, "\n");
            }
        }
    }

    // Release the parsed JSON
    json_decref(root);
}

void fetchAndSaveYouTubeData(const char *api_key, const char **video_ids, size_t num_videos, const char *username) {
    CURL *curl;
    CURLcode res;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Set the API endpoint URL (replace with your YouTube API URL)
        const char *base_url = "https://www.googleapis.com/youtube/v3/videos";
        const char *part = "snippet";
        const char *url_template = "%s?id=%s&key=%s&part=%s&maxResults=%d";
        
        char output_file_name[256];
        snprintf(output_file_name, sizeof(output_file_name), "data_files/%s_youtube_output.txt", username);

        // Open the file for writing (or create if it doesn't exist)
        FILE *output_file = fopen(output_file_name, "a");
        if (!output_file) {
            fprintf(stderr, "Error opening the file for writing.\n");
            return;
        }

        // Iterate over each video ID
        for (size_t i = 0; i < num_videos; ++i) {
            // Set up the URL without pagination parameters
            char url[256];
            snprintf(url, sizeof(url), url_template, base_url, video_ids[i], api_key, part, 1);  // Max results set to 1

            // Print the generated URL for debugging
            printf("Generated URL: %s\n", url);

            // Set up libcurl options
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);  // Pass NULL instead of &json_response

            // Perform the request
            res = curl_easy_perform(curl);

            // Null-terminate the accumulated response
            response_buffer[response_size] = '\0';

            // Check for errors
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {
                // Call the function to parse and extract information
                parseYouTubeJson(response_buffer, output_file);
            }

            // Reset the response buffer size for the next request
            response_size = 0;
        }

        // Close the file
        fclose(output_file);

        // Clean up
        curl_easy_cleanup(curl);
    }

    // Clean up libcurl
    curl_global_cleanup();
}


void make_table(const char *username){
    char output_file_name[256];
    snprintf(output_file_name, sizeof(output_file_name), "data_files/%s_youtube_output.txt", username);
    
    char output_file_name_csv[256];
    snprintf(output_file_name_csv, sizeof(output_file_name_csv), "data_files/%s_youtube_output.csv", username);
        
    
    FILE *inputFile = fopen(output_file_name, "r"); // Open the file containing the data
    FILE *outputFile = fopen(output_file_name_csv, "w"); // Open or create the CSV file for output

    if (inputFile == NULL || outputFile == NULL) {
        printf("Error opening file.\n");
        //return 1;
    }

    fprintf(outputFile, "Video_ID,Published_At,Title,Description\n"); // Write the CSV headers

    char line[1024];
    char videoID[20], publishedAt[30], title[200], description[1024];
    int isVideoID = 0, isPublishedAt = 0, isTitle = 0, isDescription = 0;

    while (fgets(line, sizeof(line), inputFile)) {
        if (strncmp(line, "Video ID:", 9) == 0) {
            strcpy(videoID, line + 10); // Extract Video ID
            videoID[strcspn(videoID, "\n")] = 0; // Remove newline character
            isVideoID = 1;
        } else if (strncmp(line, "Published At:", 13) == 0) {
            strcpy(publishedAt, line + 14); // Extract Published At
            publishedAt[strcspn(publishedAt, "\n")] = 0;
            isPublishedAt = 1;
        } else if (strncmp(line, "Title:", 6) == 0) {
            strcpy(title, line + 7); // Extract Title
            title[strcspn(title, "\n")] = 0;
            isTitle = 1;
        } else if (strncmp(line, "Description:", 12) == 0) {
            strcpy(description, line + 13); // Extract Description
            description[strcspn(description, "\n")] = 0;
            isDescription = 1;
        }

        // Check if all fields are read
        if (isVideoID && isPublishedAt && isTitle && isDescription) {
            // Write to CSV
            fprintf(outputFile, "\"%s\",\"%s\",\"%s\",\"%s\"\n", videoID, publishedAt, title, description);
            // Reset flags
            isVideoID = isPublishedAt = isTitle = isDescription = 0;
        }
    }
    
    fclose(inputFile);
    fclose(outputFile);

}
