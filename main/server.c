#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "server.h"



static int alarm_is_set; 	//global var that reports if alarm is enabled
static int alarm_hour; 		//global var reports alarm hour value
static int alarm_minute;	//global var reports alarm minute value


static const char *SERVER_TAG = "alarm server";

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(SERVER_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(SERVER_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(SERVER_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(SERVER_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}



esp_err_t URI_get_handler(httpd_req_t *req){

	return ESP_OK;
}

esp_err_t URI_put_handler(httpd_req_t *req){

	return ESP_OK;
}

esp_err_t URI_post_handler(httpd_req_t *req){
    char content[100];//create buffer for received data
    httpd_req_recv(req, content, req->content_len);//send received data to buffer
    cJSON *root = cJSON_Parse(content);//parse received data

    alarm_is_set = cJSON_GetObjectItem(root, "is_set")->valueint;//check if alarm is enabled
    if(alarm_is_set){
    	alarm_hour  = cJSON_GetObjectItem(root, "hour")->valueint;//read hour value
    	alarm_minute = cJSON_GetObjectItem(root, "minute")->valueint;//read minute value

        printf("Alarm set to %i:%02i\r\n", alarm_hour, alarm_minute);

    }
    else{
    	printf("Alarm Disabled\r\n");
    }


	//char *rendered=cJSON_Print(root);

	cJSON_Delete(root);

	//printf("Full JSON: %s", *rendered);
	//printf("Full JSON: %s\r\n", content);
    //printf("Alarm set to %i:%i", hour, minute);
    //printf("Alarm Set To %s", meridian);


	return ESP_OK;
}


esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(SERVER_TAG, "Creating URI handlers");

    httpd_uri_t URI_get = {
        .uri       = "/get",
        .method    = HTTP_GET,
        .handler   = URI_get_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t URI_put = {
        .uri       = "/put",
        .method    = HTTP_PUT,
        .handler   = URI_put_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t URI_post = {
        .uri       = "/post",
        .method    = HTTP_POST,
        .handler   = URI_post_handler,
        .user_ctx  = NULL
    };

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };

    ESP_LOGI(SERVER_TAG, "Starting HTTP Server");
    //REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(SERVER_TAG, "Registering URI handlers to Server");
        httpd_register_uri_handler(server, &URI_get);
        httpd_register_uri_handler(server, &URI_put);
        httpd_register_uri_handler(server, &URI_post);
        httpd_register_uri_handler(server, &common_get_uri);
    }
    else {
        free(rest_context);
        goto err;
    }

    return ESP_OK;
err:
    return ESP_FAIL;
}



//check alarm status (is it time to wake up?)
alarm_state_t compare_time(int hour, int minute){
	alarm_state_t alarm_state;
	if (alarm_is_set == 0){
        alarm_state = Alarm_Off;
        ESP_LOGI(SERVER_TAG, "Alarm Disabled");
	}
	else if ((hour == alarm_hour) && (minute == alarm_minute)){
        alarm_state = Alarm_Trig;
        ESP_LOGI(SERVER_TAG, "Alarm Triggered! Wake Up!!!");
	}
	else{
        alarm_state = Alarm_Standby;
        ESP_LOGI(SERVER_TAG, "Alarm standby");
	}

	return alarm_state;
}

esp_err_t disable_alarm(){
	alarm_is_set = 0;
	return ESP_OK;
}

