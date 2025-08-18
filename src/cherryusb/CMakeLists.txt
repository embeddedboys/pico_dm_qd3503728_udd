set(library_name cherryusb_device)

add_library(${library_name})

target_sources(${library_name} PRIVATE
	usb.c
	usbd_vendor.c
	${CHERRY_USB_PATH}/port/rp2040/usb_dc_rp2040.c
	${CHERRY_USB_PATH}/core/usbd_core.c

	# ${CHERRY_USB_PATH}/demo/cdc_acm_template.c
	# ${CHERRY_USB_PATH}/class/cdc/usbd_cdc_acm.c
)

target_include_directories(${library_name} PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
	${CHERRY_USB_PATH}/common
	${CHERRY_USB_PATH}/core
	${CHERRY_USB_PATH}/port/rp2040
	# ${CHERRY_USB_PATH}/class/cdc
)

target_link_libraries(${library_name} PRIVATE
	pico_stdlib
	hardware_irq
)
