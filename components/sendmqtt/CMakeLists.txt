idf_component_register(SRCS 
                        "sendmqtt.c"
                        INCLUDE_DIRS "."
                        EMBED_TXTFILES "../../cert/isrgrootx1.pem"
                        REQUIRES myServer
                        REQUIRES mqtt

                        
)