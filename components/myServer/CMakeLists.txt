idf_component_register(SRCS 
                        "myServer.c"
                        "connect.c"
                        "ntp_time.c"
                        "toggleLed.c"
                       
                        
                        INCLUDE_DIRS "."

                        REQUIRES nvs_flash
                        REQUIRES esp_http_server
                        REQUIRES mdns
                        REQUIRES json
                        REQUIRES spiffs
)