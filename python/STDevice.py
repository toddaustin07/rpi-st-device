#################################################################################
# Python API Wrapper Class for SmartThings Direct-connected Device Applications
#          
#                           Version 0.202103
#
# Copyright 2021 Todd A. Austin
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.                        
#
# Constants are replicated from Samsung SmartThings Direct-connect core SDK
# application header definitions; source: /st-device-sdk-c/src/include/st_dev.h
#################################################################################

from STDK_API import ffi, lib

###################################################################################
#                       SmartThings API Constants
###################################################################################
# IOT Status
IOT_STATUS_IDLE = 0b0001
IOT_STATUS_PROVISIONING = 0b0010
IOT_STATUS_NEED_INTERACT = 0b0100
IOT_STATUS_CONNECTING = 0b1000
IOT_STATUS_ALL = IOT_STATUS_IDLE & IOT_STATUS_PROVISIONING & IOT_STATUS_NEED_INTERACT & IOT_STATUS_CONNECTING

# IOT Level
IOT_STAT_LV_STAY = 0
IOT_STAT_LV_START = 1
IOT_STAT_LV_DONE = 2
IOT_STAT_LV_FAIL = 3
IOT_STAT_LV_CONN = 4
IOT_STAT_LV_SIGN_UP = IOT_STAT_LV_START
IOT_STAT_LV_SIGN_IN = 6

# Capability Attribute Types
IOT_CAP_VAL_TYPE_UNKNOWN = -1
IOT_CAP_VAL_TYPE_INTEGER = 0
IOT_CAP_VAL_TYPE_NUMBER = 1
IOT_CAP_VAL_TYPE_INT_OR_NUM = 2
IOT_CAP_VAL_TYPE_STRING = 3
IOT_CAP_VAL_TYPE_STR_ARRAY = 4
IOT_CAP_VAL_TYPE_JSON_OBJECT = 5
IOT_CAP_VAL_TYPE_BOOLEAN = 6

MAX_CAP_ARG = 5

# IOT Notification Types
IOT_NOTI_TYPE_UNKNOWN = -1
IOT_NOTI_TYPE_DEV_DELETED = 0
IOT_NOTI_TYPE_RATE_LIMIT = 1
IOT_NOTI_TYPE_QUOTA_REACHED = 2
IOT_NOTI_TYPE_SEND_FAILED = 3

IOT_INFO_TYPE_IOT_STATUS_AND_STAT = 0
IOT_INFO_TYPE_IOT_PROVISIONED = 1

# Dump modes
IOT_DUMP_MODE_NEED_BASE64 = 1
IOT_DUMP_MODE_NEED_DUMP_STATE = 2

#############################################################################################################
#         Define SmartThings Direct-connected Device Class, which will wrapper the C library APIs
#############################################################################################################

class STDevice(object):

    def __init__(self):

        pass

    def init_device(self, deviceinfo, onboardingconfig):

        try:
            with open(deviceinfo, 'r') as f1:
                device_info = f1.read()

            with open(onboardingconfig, 'r') as f2:
                onboarding_config = f2.read()

            self.ctx = lib.st_conn_init(onboarding_config.encode('utf-8'), len(onboarding_config),
                                        device_info.encode('utf-8'), len(device_info))

            if self.ctx == ffi.NULL:
                return False
            else:
                return True

        except Exception as ex:
            print("\033[91mSTDevice init: Failed to load json files: ", ex, "\033[0m")
            return False

    def set_notification_cb(self, notify_cb):

        iot_err = lib.st_conn_set_noti_cb(self.ctx, notify_cb, ffi.NULL)
        if iot_err != 0:
            return False
        else:
            return True

    def init_capability(self, capname, initcallback, compname="main"):

        handle = lib.st_cap_handle_init(self.ctx, compname.encode('utf-8'), capname.encode('utf-8'), initcallback, ffi.NULL)

        return(handle)

    def register_cmd_callback(self, handle, cmd, callback):

        iot_err = lib.st_cap_cmd_set_cb(handle, cmd.encode('utf-8'), callback, ffi.NULL)

        if iot_err > 0:
            return False
        else:
            return True

    def start(self, status_callback):

        iot_err = lib.st_conn_start(self.ctx, status_callback, 15, ffi.NULL, ffi.NULL)
        if iot_err == 0:
            return True
        else:
            return False

    @classmethod
    def setstrattr(cls, handle, attrname, attrvalue):

        value = ffi.new("iot_cap_val_t *")
        value.type = cls.get_val_type(attrvalue)

        if value.type == IOT_CAP_VAL_TYPE_INTEGER:
            value.integer = attrvalue
        if value.type == IOT_CAP_VAL_TYPE_NUMBER:
            value.number = attrvalue
        elif value.type == IOT_CAP_VAL_TYPE_STR_ARRAY:            
            strings = ffi.new(f"char *[{len(attrvalue)}]", [ffi.new(f"char [{len(v) + 8}]", v.encode('utf-8')) for v in attrvalue])
            value.str_num = len(attrvalue)
            value.strings = strings

        elif value.type == IOT_CAP_VAL_TYPE_STRING:
            leng = len(attrvalue) + 8
            fstring = ffi.new(f"char [{leng}]", attrvalue.encode('utf-8'))
            value.string = fstring

        attr = ffi.new("IOT_EVENT *")
        attr = lib.st_cap_create_attr(handle, attrname.encode('utf-8'), value, ffi.NULL, ffi.NULL)
        if attr != ffi.NULL:
            attrlist = ffi.new("IOT_EVENT **")
            attrlist = [attr]
            seq = lib.st_cap_send_attr(attrlist, 1)
            lib.st_cap_free_attr(attr)
            return(seq)
        else:
            return(-1)

    def get_val_type(iotvalue):
        switcher = {
            int: IOT_CAP_VAL_TYPE_INTEGER,
            float: IOT_CAP_VAL_TYPE_NUMBER,
            str: IOT_CAP_VAL_TYPE_STRING,
            bool: IOT_CAP_VAL_TYPE_BOOLEAN,
            list: IOT_CAP_VAL_TYPE_STR_ARRAY
        }

        return switcher.get(type(iotvalue), IOT_CAP_VAL_TYPE_UNKNOWN)

    def convert_to_python(s):
        type=ffi.typeof(s)
        if type.kind == 'struct':
            return dict(__convert_struct_field( s, type.fields ) )
        elif type.kind == 'array':
            if type.item.kind == 'primitive':
                if type.item.cname == 'char':
                    return ffi.string(s)
                else:
                    return [ s[i] for i in range(type.length) ]
            else:
                return [ convert_to_python(s[i]) for i in range(type.length) ]
        elif type.kind == 'primitive':
            return int(s)
