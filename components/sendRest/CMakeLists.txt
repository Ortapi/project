idf_component_register(SRCS 
                        "send_rest.c"
                        
                        INCLUDE_DIRS "."

                        REQUIRES myServer
                        REQUIRES esp_http_client
                        REQUIRES json
                        REQUIRES nvs_flash
)