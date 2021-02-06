from cffi import FFI
ffibuilder = FFI()

SMARTTHINGSHEADERFILE='./py_st_dev.h'

with open(SMARTTHINGSHEADERFILE) as f:

	ffibuilder.cdef(f.read() + '''
		/* ADD PYTHON CALLBACK FUNCTION DECLARATIONS HERE */
		extern "Python" void handleNotifications(iot_noti_data_t *noti_data, void *noti_usr_data);
		extern "Python" void handleStatus(iot_status_t status, iot_stat_lv_t stat_lv, void *usr_data);
		extern "Python" void handleSwitchInit(IOT_CAP_HANDLE *handle, void *usr_data);
		extern "Python" void handleSwitchOn(IOT_CAP_HANDLE *handle, iot_cap_cmd_data_t *cmd_data, void *usr_data);
		extern "Python" void handleSwitchOff(IOT_CAP_HANDLE *handle, iot_cap_cmd_data_t *cmd_data, void *usr_data);
	''')

ffibuilder.set_source("STDK_API",
'''
	#include <stdbool.h>
	#include "'''+SMARTTHINGSHEADERFILE+'"',
	libraries=['./iotcore','ssl','pthread','rt','crypto'])

ffibuilder.compile()
	
