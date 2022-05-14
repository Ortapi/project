set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

# spiffs_create_partition_image(storage ../font FLASH_IN_PROJECT)
spiffs_create_partition_image(web   ../site/build FLASH_IN_PROJECT)
spiffs_create_partition_image(fonts ../font FLASH_IN_PROJECT)