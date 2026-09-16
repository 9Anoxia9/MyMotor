add_subdirectory(src)

add_library(tinyusb)
tinyusb_target_add(tinyusb)

target_sources(tinyusb PUBLIC
        src/portable/synopsys/dwc2/dcd_dwc2.c
        src/portable/synopsys/dwc2/hcd_dwc2.c
        src/portable/synopsys/dwc2/dwc2_common.c
)

target_include_directories(tinyusb PUBLIC
        src/portable/synopsys
)


