
#define USBPRINT_IOCTL_INDEX  0x0000
#define IOCTL_USBPRINT_GET_LPT_STATUS \
                     CTL_CODE(FILE_DEVICE_UNKNOWN, USBPRINT_IOCTL_INDEX+12, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                           

#define IOCTL_USBPRINT_GET_1284_ID \
                     CTL_CODE(FILE_DEVICE_UNKNOWN, USBPRINT_IOCTL_INDEX+13, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                           

#define IOCTL_USBPRINT_VENDOR_SET_COMMAND \ 
                     CTL_CODE(FILE_DEVICE_UNKNOWN, USBPRINT_IOCTL_INDEX+14, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                           

#define IOCTL_USBPRINT_VENDOR_GET_COMMAND \
                      CTL_CODE(FILE_DEVICE_UNKNOWN, USBPRINT_IOCTL_INDEX+15, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                           

#define IOCTL_USBPRINT_SOFT_RESET \
                      CTL_CODE(FILE_DEVICE_UNKNOWN, USBPRINT_IOCTL_INDEX+16, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                           

