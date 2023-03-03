#include "../include/utils/utils.h"
#include "../include/utils/log.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

char *str_replace(char *str, const char *old, const char *neww) {
  char *result;
  int i, count = 0;
  size_t newlen = strlen(neww);
  size_t oldlen = strlen(old);

  for (i = 0; str[i] != '\0';) {
    if (strstr(&str[i], old) == &str[i]) {
      count++;
      i += oldlen;
    } else {
      i++;
    }
  }

  result = (char *)malloc(i + count * (newlen - oldlen) + 1);

  i = 0;
  while (*str) {
    if (strstr(str, old) == str) {
      strcpy(&result[i], neww);
      i += newlen;
      str += oldlen;
    } else {
      result[i++] = *str++;
    }
  }

  result[i] = '\0';
  return result;
}

char *remove_extension(char *filename) {
  char *new_filename = malloc(strlen(filename) + 1);
  strcpy(new_filename, filename);
  char *dot = strrchr(new_filename, '.');
  if (dot && dot != new_filename) {
    *dot = '\0';
  }
  return new_filename;
}

char *replace_newlines_with_escapes(char *str) {
  size_t len = strlen(str);
  char *escaped_str = (char *)malloc(
      len * 2 + 1); // allocate enough memory for worst-case scenario
  size_t j = 0;

  for (size_t i = 0; i < len; i++) {
    if (str[i] == '\n') {
      escaped_str[j++] = '\\';
      escaped_str[j++] = 'n';
    } else {
      escaped_str[j++] = str[i];
    }
  }

  escaped_str[j] = '\0'; // null-terminate the string
  return escaped_str;
}

char *extract_filename(const char *path) {
  char *filename = NULL;
  const char *last_slash = strrchr(path, '/');
  if (last_slash) {
    const char *dot = strrchr(last_slash, '.');
    if (dot && dot != last_slash) {
      filename = malloc(dot - last_slash);
      strncpy(filename, last_slash + 1, dot - last_slash - 1);
      filename[dot - last_slash - 1] = '\0';
    } else {
      filename = strdup(last_slash + 1);
    }
  } else {
    filename = strdup(path);
  }
  return remove_extension(filename);
}

char *http_get(char *url) {
  CURL *curl_handle;
  CURLcode res;
  struct MemoryStruct chunk;

  chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if (res != CURLE_OK) {
    log_error(NULL, "remote execution failed: %s. %s\n",
              curl_easy_strerror(res), url);
    return NULL;
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();

  return chunk.memory;
}

int file_exists(char *filename) { return !access(filename, F_OK); }

int is_url(char *path) {
  if (strstr(path, "http") != 0) {
    return 1;
  }

  return 0;
}

char *download_file_to_string(char *url) {
  char *source = http_get(url);

  return source;
}

char *run_shell_command(char *command) {
  log_print(command);

  // system(command);

  return "hiii";
}

void change_working_dir(char *dirname) {
  int result = chdir(dirname);
  if (result != 0) {
    log_error(NULL, "could not change working directory to: %s", dirname);
  }
}

char *get_working_dir() {
  char dir[1024];

  if (getcwd(dir, sizeof(dir)) == NULL) {
    log_error(NULL, "cannot get current working directory");
  }

  char *working_dir = (char *)malloc(strlen(dir) * sizeof(char));
  strcpy(working_dir, dir);

  return working_dir;
}

char *extract_working_dir(char *file_path) {
  int i;
  int last_slash_index = -1;
  int path_len = strlen(file_path);

  for (i = 0; i < path_len; i++) {
    if (file_path[i] == '/') {
      last_slash_index = i;
    }
  }

  char dir[300];

  strncpy(dir, file_path, last_slash_index + 1);
  dir[last_slash_index + 1] = '\0';

  char *working_dir = (char *)malloc(strlen(dir) * sizeof(char));
  strcpy(working_dir, dir);

  return working_dir;
}

char *read_file_to_string(char *filename) {

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    log_error(NULL, "Could not open file %s", filename);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char *file_contents = (char *)malloc(sizeof(char) * (file_size + 1));
  if (file_contents == NULL) {
    fclose(file);
    return NULL;
  }

  fread(file_contents, sizeof(char), file_size, file);
  file_contents[file_size] = '\0';

  fclose(file);

  return file_contents;
}

int file_has_extension(const char *file_name, const char *extension) {
  char *dot = strrchr(file_name, '.');
  if (dot && !strcmp(dot, extension)) {
    return 1;
  }
  return 0;
}

NumberFormat check_number(const char *str) {
  NumberFormat format = DECIMAL; // default to decimal

  if (strlen(str) < 2) {
    format = DECIMAL;
  } else if (strchr(str, '.') != 0) {
    format = FLOAT;
  } else if (str[0] == '0') {
    if (str[1] == 'x' || str[1] == 'X') {
      format = HEXADECIMAL;
      str += 2;
    } else if (str[1] == 'b' || str[1] == 'B') {
      format = BINARY;
      str += 2;
    } else {
      format = OCTAL;
      str += 1;
    }
  }

  // check if the rest of the string is valid for the detected format
  for (int i = 0; str[i] != '\0'; i++) {
    switch (format) {
    case DECIMAL:
      if (!isdigit(str[i])) {
        return INVALID;
      }
      break;
    case OCTAL:
      if (!isdigit(str[i]) || str[i] > '7') {
        return INVALID;
      }
      break;
    case HEXADECIMAL:
      if (!isxdigit(str[i])) {
        return INVALID;
      }
      break;
    case BINARY:
      if (str[i] != '0' && str[i] != '1') {
        return INVALID;
      }
      break;
    case FLOAT:

      if (!isdigit(str[i]) && str[i] != '.' && str[i] != '+' && str[i] != '-' &&
          str[i] != 'e' && str[i] != 'E') {
        return INVALID;
      }
      if (str[i] == '.') {
        int j = i + 1;
        if (!isdigit(str[j])) {
          return INVALID;
        }
        for (; isdigit(str[j]); ++j)
          ;
        if (str[j] == 'e' || str[j] == 'E') {
          j++;
          if (str[j] == '+' || str[j] == '-') {
            j++;
          }
          if (!isdigit(str[j])) {
            return INVALID;
          }
        }
      }
      break;

    default:
      return INVALID;
    }
  }
  return format;
}
