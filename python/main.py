import signal
import sys
from time import sleep
from STDevice import *


###################################################################################
#               Define global values
###################################################################################

status_map = {
    IOT_STATUS_IDLE: "Idle, not connected",
    IOT_STATUS_PROVISIONING: "Onboarding",
    IOT_STATUS_NEED_INTERACT: "User interaction required",
    IOT_STATUS_CONNECTING: "Connecting to server"
}

level_map = {
    IOT_STAT_LV_STAY: "...staying",
    IOT_STAT_LV_START: "...starting",
    IOT_STAT_LV_DONE: "...DONE",
    IOT_STAT_LV_FAIL: "...FAILED",
    IOT_STAT_LV_CONN: "...connected to mobile",
    IOT_STAT_LV_SIGN_IN: "...signing in"
}

exit_now = False

def signal_handler(sig, frame):

    global exit_now

    print("\n\033[97mKeyboard interrupt detected")
    exit_now = True

def event_loop():

    global exit_now

    while not exit_now:
        sleep(1)
    print("EXITING\033[0m\n")

###################################################################################################################
#                           DEFINE SMARTTHINGS CALLBACK FUNCTIONS
#                 * Additional callbacks must also be declared in iotcorebuild.py *
###################################################################################################################

@ffi.def_extern()
def handleNotifications(noti_data, user_data):

    print("Notification message received")

    if noti_data.type == IOT_NOTI_TYPE_DEV_DELETED:
        print("\n\033[97mDEVICE DELETED\033[0m\n")

    else:
        if noti_data.type == IOT_NOTI_TYPE_RATE_LIMIT:
            print("\033[93m")
            print("RATE LIMIT; remaining time: %d, sequence number: %d" % (noti_data.raw.rate_limit.remainingTime,
                                                                           noti_data.raw.rate_limit.sequenceNumber))
            print("\033[0m")


@ffi.def_extern()
def handleStatus(status, level, user_data):

    message = status_map.get(status, "Unknown IOT status") + level_map.get(level, "Unknown IOT level")

    if status == IOT_STATUS_CONNECTING and level == IOT_STAT_LV_DONE:
        print("\033[97m** CONNECTED TO MQTT SERVER **\033[0m")
    else:
        if level == IOT_STAT_LV_FAIL:
            print("\033[91m"+message+"\033[0m")
        else:
            print("\033[96m"+message+"\033[0m")


@ffi.def_extern()
def handleSwitchInit(handle, userdata):

    print ("\n\033[96mSwitch initialization invoked")

    seqnum = STDevice.setstrattr(handle, "switch", "off")

    if seqnum > 0:
        print("\033[96mSwitch attribute initialized to OFF; sequence number =", seqnum, "\033[0m\n")
    else:
        print("\033[91mError updating switch attribute\033[0m\n")


@ffi.def_extern()
def handleSwitchOn(handle, cmddata, userdata):

    print("\n\033[96mReceived Switch ON command")

    seqnum = STDevice.setstrattr(handle, "switch", "on")

    if seqnum > 0:
        print("\033[96mSwitch attribute updated to ON; sequence number =", seqnum, "\033[0m\n")
    else:
        print("\033[91mError updating switch attribute\033[0m\n")


@ffi.def_extern()
def handleSwitchOff(handle, cmddata, userdata):

    print("\n\033[96mReceived Switch OFF command")

    seqnum = STDevice.setstrattr(handle, "switch", "off")

    if seqnum > 0:
        print("\033[96mSwitch attribute updated to OFF; sequence number =", seqnum, "\033[0m\n")
    else:
        print("\033[91mError updating switch attribute\033[0m\n")


###########################################################################################################
#                                               MAIN
###########################################################################################################

if __name__ == '__main__':

    signal.signal(signal.SIGINT, signal_handler)

    DEVICEINFO_PATH = './device_info.json'
    ONBCONFIG_PATH = './onboarding_config.json'

    mydevice = STDevice()

    if mydevice.init_device(DEVICEINFO_PATH, ONBCONFIG_PATH):

        if mydevice.set_notification_cb(lib.handleNotifications):

            switchhandle = mydevice.init_capability("switch", lib.handleSwitchInit)

            if switchhandle != 0:

                if mydevice.register_cmd_callback(switchhandle, "on", lib.handleSwitchOn):

                    if mydevice.register_cmd_callback(switchhandle, "off", lib.handleSwitchOff):

                        print("\033[97mSTARTING DEVICE\033[0m")
                        mydevice.start(lib.handleStatus)        # ignore errors, retries will be handled by Core SDK

                        event_loop()

                    else:
                        print("\033[91mFailed to set OFF command callback\033[0m")
                else:
                    print("\033[91mFailed to set ON command callback\033[0m")
            else:
                print("\033[91mFailed to initialize capability\033[0m")
        else:
            print("\033[91mFailed to set notification callback\033[0m")
    else:
        print("\033[91mFailed to initialize device\033[0m")

    exit()