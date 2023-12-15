#ifndef YOUTUBE_API_H
#define YOUTUBE_API_H

#include <stdio.h>

// Function declarations
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
void parseYouTubeJson(const char *json_response, FILE *output_file);
void fetchAndSaveYouTubeData(const char *api_key, const char **video_ids, size_t num_videos, const char *username);
void make_table(const char *username);

#endif  // YOUTUBE_API_H
