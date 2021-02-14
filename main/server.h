#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(SERVER_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)
#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)



typedef enum {
  Alarm_Off, 		//alarm is not enabled
  Alarm_Standby,	//alarm enabled, waiting for trigger time
  Alarm_Trig		//alarm enabled, time match (wake up time)
} alarm_state_t;

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

esp_err_t start_rest_server(const char *base_path);
alarm_state_t compare_time(int hour, int minute);
esp_err_t disable_alarm();

