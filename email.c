#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>

// This code will not work out of the box! It depends on a body file, you can add the file in the same folder as the compiled program.
// The file requires no special format, you can simple make a body.txt and write normally inside of it.
// remember to compile using the curl tag.
// Edit the fieds (FILE path and the msg_info struct) to send proper emails.

// I'm using the date calculation as a name. You can hardcode it somewhere in build_message() or add a filed to msg_info if desired

struct upload_status {
  size_t bytes_read;
  size_t payload_size;
  char* payload;
};

typedef struct {
    char* app_pwd;
    char* from_email;
    char* to_email;
    char* smtp;
} msg_info;


struct upload_status create_payload(size_t buffer_size);
void make_date(char *buffer, size_t size);
char* build_message(msg_info *msg, size_t buffer_size);
struct upload_status create_payload(size_t buffer_size);
static size_t read_cb(char *ptr, size_t size, size_t nmemb, void *userp);
int send_email(msg_info *msg, CURL *curl, CURLcode *result, char* message_body);

void make_date(char *buffer, size_t size)
{
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);

  strftime(buffer, size, "%a, %d %b %Y %H:%M:%S %z", tm_info);
}

char* build_message(msg_info *msg, size_t buffer_size)
{
    struct upload_status lost_media = create_payload(buffer_size);
    char date[128];
    make_date(date, sizeof(date));
    size_t full_size = 1024 + lost_media.payload_size;

    char *buffer = malloc(full_size);

    snprintf(buffer, full_size,
        "Date: %s\r\n"
        "To: %s\r\n"
        "From: %s\r\n"
        "Subject: %s\r\n"
        "\r\n"
        "%s\r\n",
        date,
        msg->to_email,
        msg->from_email,
        date,
        lost_media.payload
    );
    free(lost_media.payload);
    return buffer;
}

struct upload_status create_payload(size_t buffer_size){
  FILE *file = NULL;
  char *buffer = (char *)malloc(buffer_size * sizeof(char));
  if (!buffer) {
      perror("malloc");
      exit(EXIT_FAILURE);
  }
  file = fopen("PATH TO YOUR FILE/FILE.TYPE", "r");
  if(!file) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }
  struct upload_status payload;
  payload.bytes_read = 0;  

  size_t i = 0;
  int c = 0;
  while((c = fgetc(file)) != EOF) {
    if (i >= buffer_size)
    {
      buffer_size += 1000;
      buffer = realloc(buffer, buffer_size);
      if(!buffer) {
        perror("Realloc");
        exit(EXIT_FAILURE);
      }
    }
    buffer[i] = c;
    i++;
  }
  if (i < buffer_size)
  {
    buffer[i] = '\0';
  } else {
    buffer_size++;
    buffer = realloc(buffer, buffer_size);
    buffer[i] = '\0';
  }
  fclose(file);

  payload.payload = buffer;
  payload.payload_size = i;

  return payload;
}

static size_t read_cb(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  size_t max_buffer = size * nmemb;
 
  if(upload_ctx->bytes_read >= upload_ctx->payload_size)
    return 0; // Out of data
 
  size_t to_copy = upload_ctx->payload_size - upload_ctx->bytes_read;
  
  if(to_copy > max_buffer)
    to_copy = max_buffer;
    
  memcpy(ptr, upload_ctx->payload + upload_ctx->bytes_read, to_copy);
  upload_ctx->bytes_read += to_copy;
 
  return to_copy;
}

int send_email(msg_info *msg, CURL *curl, CURLcode *result, char* message_body) {
  if(curl) {
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx = { 0 };
    upload_ctx.payload = message_body;
    upload_ctx.payload_size = strlen(message_body);
    upload_ctx.bytes_read = 0;
  curl_easy_setopt(curl, CURLOPT_URL,msg->smtp);
  curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  curl_easy_setopt(curl, CURLOPT_USERNAME,msg->from_email);
  curl_easy_setopt(curl, CURLOPT_PASSWORD,msg->app_pwd);

  curl_easy_setopt(curl, CURLOPT_MAIL_FROM, msg->from_email);
  recipients = curl_slist_append(recipients, msg->to_email);
  recipients = curl_slist_append(recipients, msg->from_email);

  curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb);
  curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
  *result = curl_easy_perform(curl);
  curl_slist_free_all(recipients);

  }
  return (int)(*result);
}

int main(int argc, char *argv[])
{
  CURL *curl;
  CURLcode result;
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  msg_info msg = {
      .app_pwd = "Generate an app password using gmail or another service if necessary",
      .from_email = "Your email here",
      .to_email = "Target email here",
      .smtp = "smtp://smtp.gmail.com:587"
  };

  char* message_body = build_message(&msg, 1000);
  send_email(&msg, curl, &result, message_body);
  free(message_body);
  curl_easy_cleanup(curl);
  curl_global_cleanup();

return EXIT_SUCCESS;
}
