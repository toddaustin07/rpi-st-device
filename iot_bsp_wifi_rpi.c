/*******************************************************************************************************************************************
Enabling Raspberry Pi to run SmartThings direct-connected device applications
    This file replaces iot_bsp_wifi_posix.c in the SmartThings Core SDK
                           Version 0.20201217

 Copyright 2020 Todd A. Austin

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.


Description:
    This module enables Raspberry Pi-based IOT devices to connect with Samsung SmartThings direct connected device architecture.
    This file must be compiled as part of the SmartThings SDK for Direct Connect Devices.
    This module replaces the posix wifi module in the BSP porting directory of the SDK:
            ~/st-device-sdk/src/port/bsp/posic/iot_bsp_wifi_posix.c
    All remaining posix modules in the SDK BSP port files are used for Raspberry Pi builds, namely:
        iot_os_util_posix.c, iot_bsp_debug_posix.c, iot_bsp_nv_data_posix.c, iot_bsp_random_posix.c, iot_bsp_system_posix.c

With thanks to Kwang-Hui of Samsung who patiently answered my many questions during development.

********************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include </home/pi/st-device-sdk-c/src/include/bsp/iot_bsp_wifi.h>
#include </home/pi/st-device-sdk-c/src/include/iot_error.h>
#include </home/pi/st-device-sdk-c/src/include/iot_debug.h>
#include "/home/pi/st-device-sdk-c/src/include/os/iot_os_util.h"
#include "/home/pi/st-device-sdk-c/src/include/iot_util.h"

#define RPICONFFILE "RPISetup.conf"
#define DEFAULTDIR "./"
#define SOFTAPCONFFILE "/etc/hostapd/hostapd.conf"
#define SOFTAPCONTROLDIR "~/rpi-st-device/"
#define SOFTAPSTARTFILE "softapstart"
#define SOFTAPSTOPFILE  "softapstop"
#define PRIORCONF "./__prior_hostapd.conf"

#define configtag_ETH "USE_ETHERNET"
#define configtag_AP "AP_SHUTDOWN"
#define configtag_devSTA "STATION_DEV"
#define configtag_devAP "AP_DEV"
#define configtag_devETH "ETH_DEV"

#define MAXDEVNAMESIZE 10

#define SSIDWAITRETRIES 7
#define SSIDWAITTIME 250000
#define SCANMODEWAITTIME 800000
#define SOFTAPWAITTIME 999999
#define SEQSYSCMDWAIT 500000

extern int errno;

/** DECLARE FUNCTIONS CONTAINED IN THIS FILE **/

int _perform_scan();
void _parsemac(char *textptr, uint8_t *hexbuf);
unsigned int _htoi (const char *ptr);
int _getnumeric(int maxdigits, char *text);
bool _getrpiconf(char *currdir);
int _isupIface(char *devname);
int _isconfWifi(char *devname, char *ssid);
int _parseconfparm(char *parmstr, char *text);
int _enableWifi(char *dev);
int _softblock(char *dev, char set);
int _waitWifiConn(char *dev, char *ssid);
int _SoftAPControl(char *cmd);
int _initDevNames();
int _setupHostapd(char*ssid, char *password, char *iface);
int _updateHfile(char *fname, char *ssid,char *password, char*iface);
int _switchSSID(char *dev, char *ssid);
bool _checkfortestdevfile();
bool _checksoftapcontrol(char *dir);
bool _switchmode(char *mode);
bool _checkstartSoftAP(char *service);
bool _restorehfile(char *backupfile);
bool _restoreAP();

/** DEFINE GLOBAL STATIC VARIABLES **/

struct scandata {
    int apcount;
    iot_wifi_scan_result_t apdata[IOT_WIFI_MAX_SCAN_RESULT];
};

static struct scandata scanstore;

static bool Ethernet = true;
static bool ManageAP = true;
static bool ConcurrentWifi = false;
static bool DualWifidev = false;
static bool APWifionly = false;
static bool APWifionlyRestore = false;
static bool STWifionly = true;
static bool AP_ON = false;
static char PHYSWIFIDEV[5] = "phy0";
static char wifi_sta_dev[MAXDEVNAMESIZE+1] = "";
static char wifi_ap_dev[MAXDEVNAMESIZE+1] = "";
static char eth_dev[MAXDEVNAMESIZE+1] = "";
static char SOFTAPSTART[60] = "";
static char SOFTAPSTOP[60] = "";
static uint8_t wifimacaddr[IOT_WIFI_MAX_BSSID_LEN];
static uint8_t ethmacaddr[IOT_WIFI_MAX_BSSID_LEN];

static int WIFI_INITIALIZED = false;

/**********************************************************************************************************************
    Required BSP fuction: iot_bsp_wifi_init()

    Purpose:    Validate & Initialize RPI wireless devices that will be needed; includes retrieving wifi devices,
                reading optional RPI configuration file, and initializing global static flags

    Input:      none

    Output:     return IOT_ERROR_ value (IOT_ERROR_NONE if no errors)

***********************************************************************************************************************/
iot_error_t iot_bsp_wifi_init()
{

    IOT_INFO("[rpi] iot_bsp_wifi_init");

    if (!WIFI_INITIALIZED)  {

        if (!_getrpiconf(DEFAULTDIR)) {                           // read config file

            if (!_initDevNames()) {                     // initialize device names & info
                IOT_ERROR("[rpi] Failure initializing interface device names");
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }

            //Station device only
            if ((strcmp(wifi_sta_dev,"") != 0) && (strcmp(wifi_ap_dev,"") == 0)) {

                STWifionly=true;
                ManageAP=true;
                ConcurrentWifi=false;
                DualWifidev=false;
                APWifionly=false;
                IOT_INFO("[rpi] Wifi station device %s found",wifi_sta_dev);
            }

            // AP device only
            if ((strcmp(wifi_sta_dev,"") == 0) && (strcmp(wifi_ap_dev,"") != 0)) {

                if (Ethernet) {
                    APWifionly=true;
                    ManageAP=false;                             // leave AP always on
                    ConcurrentWifi=false;
                    DualWifidev=false;
                    STWifionly=false;
                    IOT_INFO("[rpi] Wifi AP device %s found",wifi_ap_dev);
                } else {
                    IOT_ERROR("[rpi] No station device available");
                    return IOT_ERROR_CONN_OPERATE_FAIL;
                }
            }

            // Found both Station and AP devices
            if ((strcmp(wifi_sta_dev,"") != 0) && (strcmp(wifi_ap_dev,"") != 0)) {

                DualWifidev=true;
                ManageAP=true;                          // default is to manage them up and down
                ConcurrentWifi=false;                   // no Concurrent support for now
                APWifionly=false;
                STWifionly=false;
                IOT_INFO("[rpi] %s station and %s AP devices found",wifi_sta_dev,wifi_ap_dev);
            }

        }

        // We have wireless devices, now check Ethernet

        if (Ethernet) {

            if (_isupIface(eth_dev))
                IOT_INFO("[rpi] Ethernet connection available: %s",eth_dev);

            else {

                IOT_ERROR("[rpi] Ethernet not connected");
                return IOT_ERROR_NET_INVALID_INTERFACE;
            }

        }
        else
            IOT_INFO("[rpi] No Ethernet");

		if (!_checksoftapcontrol(SOFTAPCONTROLDIR)) {		// Make sure SoftAP control scripts are present
			IOT_ERROR("Missing SoftAP control scripts");
			return IOT_ERROR_CONN_OPERATE_FAIL;
        }
    }

	WIFI_INITIALIZED = true;
	IOT_INFO("[rpi] Wifi Initialization Done");
	IOT_DUMP(IOT_DEBUG_LEVEL_DEBUG, IOT_DUMP_BSP_WIFI_INIT_SUCCESS, 0, 0);

	return IOT_ERROR_NONE;
}

/*************************************************************************************
Subroutine: _getrpiconf (usage TBD)

Purpose:    Read optional RPI config file and use parameters to initialize global values

Input:      String pointer to configuration file name

Ouput:      Global values:

**************************************************************************************/

bool _getrpiconf(char *currdir) {

    FILE *pf;
    char pathname[100];
    char *readline = NULL;
    size_t len = 0;
    int errnum;
    char *textptr;
    char parmstr[50];

    // Initialize global static defaults

    strcpy(pathname,currdir);
    strcat(pathname,RPICONFFILE);

    if ((pf = fopen(pathname,"r")) != NULL) {

        while (getline(&readline,&len,pf)!= EOF) {

            if ((readline[0] != '#') && (readline[0] != '\n'))  {       // skip comments and blank lines

                if((textptr = strstr(readline,configtag_ETH))) {

                   if(_parseconfparm(parmstr, textptr)) {

                        if ((parmstr[0] == 'Y') || (parmstr[0] == 'y'))
                            Ethernet = true;

                        else if ((parmstr[0] == 'N') || (parmstr[0] == 'n'))
                            Ethernet = false;

                    }
                } else {
                    if ((textptr = strstr(readline,configtag_AP))) {


                        if (_parseconfparm(parmstr, textptr)) {

                            if ((*parmstr == 'Y') || (*parmstr == 'y'))
                                ManageAP = true;

                            else if ((*parmstr == 'N') || (*parmstr == 'n'))
                                ManageAP = false;
                        }

                    } else {

                        if ((textptr = strstr(readline,configtag_devSTA))) {

                            if(_parseconfparm(parmstr,textptr))

                                strcpy(wifi_sta_dev,parmstr);

                        }
                        else {

                            if ((textptr = strstr(readline,configtag_devAP))) {

                                if(_parseconfparm(parmstr,textptr))

                                    strcpy(wifi_ap_dev,parmstr);
                            }
                            else {
                                if ((textptr = strstr(readline,configtag_devETH))) {

                                    if(_parseconfparm(parmstr,textptr))

                                        strcpy(eth_dev,parmstr);

                                }

                            }
                        }
                    }
                }
            }
        }
        fclose(pf);

        if (readline)
            free(readline);

    }  else {
        errnum=errno;
        if (errnum != ENOENT)
            IOT_INFO("[rpi] Could read RPI configuration file");

        return false;
    }

    return true;
}

// Secret test file for forcing wifi device definitions (not used in production)
bool _checkfortestdevfile() {

    #define TESTDEVFILE "./.dev"

    FILE *pf;
    char pathname[100];
    char *readline = NULL;
    char *textptr;
    char parmstr[50];
    size_t len;
    int errnum;

    // Initialize global static defaults

    strcpy(pathname,TESTDEVFILE);

    if ((pf = fopen(pathname,"r")) != NULL) {

        while (getline(&readline,&len,pf)!= EOF) {

            if ((readline[0] != '#') && (readline[0] != '\n'))  {       // skip comments and blank lines


                if((textptr = strstr(readline,configtag_devSTA))) {

                    if(_parseconfparm(parmstr, textptr))

                        strcpy(wifi_sta_dev,parmstr);
                }
                else {

                    if((textptr = strstr(readline,configtag_devAP))) {

                        if(_parseconfparm(parmstr, textptr))

                            strcpy(wifi_ap_dev,parmstr);

                    }
                    else {
                        if((textptr = strstr(readline,configtag_devETH))) {

                            if(_parseconfparm(parmstr, textptr))

                                strcpy(eth_dev,parmstr);


                        }
                    }
                }
            } //end if blank line
        }


        fclose(pf);
        if (strcmp(eth_dev, "") == 0)
            Ethernet = false;
        else
            Ethernet = true;
        return true;

    } else {
        errnum = errno;
        if (errnum != ENOENT)       // anything but file not found is unexpected
            IOT_INFO("[rpi] Warning: file error %d on dev test file open",errnum);
        return false;
    }

}
/****************************************************************

Initialize global variables for station, AP, and Ethernet devices

****************************************************************/

int _initDevNames() {

    FILE *pf;
    const int maxdatasize = 100;
    char data[maxdatasize];
    char devname[MAXDEVNAMESIZE+1];
    char devtype[10];
    char command[40];
    uint8_t tmpmac[IOT_WIFI_MAX_BSSID_LEN];
    bool found = false;
    int i;
    char *textptr;


    if(_checkfortestdevfile())      // If test device file exists, we'll pick up device names from there
        return true;

    // FIRST GET ETHERNET NAME AND CONFIRM CONNECTED

    strcpy(eth_dev,"");

    pf = popen("ls /sys/class/net","r");

    if (pf)  {

         if(fgets(data,maxdatasize,pf)) {

            pclose(pf);

            i = 0;

            while ((*(data+i) != ' ') && (*(data+i) != '\n'))  {        // Ethernet device will be first text in ls command output
                devname[i] = *(data+i);
                ++i;
            }

            devname[i] = '\0';

            if (strcmp(devname,"eth0") != 0) {
                IOT_ERROR("[rpi] Unexpected Ethernet device name",devname);
                Ethernet = false;
            } else {

                strcpy(eth_dev,devname);            // Found device name; save it in global static variable

                sprintf(command,"cat /sys/class/net/%s/carrier",devname);
                pf = popen(command,"r");
                if (pf)  {

                    if(fgets(data,maxdatasize,pf)) {

                        pclose(pf);
                        if (*data == '1') {

                            Ethernet = true;

                        } else {

                            IOT_INFO("Ethernet device %s not connected",devname);
                            Ethernet = false;

                        }
                    } else {
                        IOT_ERROR("[rpi] Could not read Ethernet status %s",devname);
                        pclose(pf);
                        Ethernet = false;
                    }
                } else {
                    IOT_ERROR("[rpi] Could not check Ethernet status %s",devname);
                    Ethernet = false;
                }

                // Now get the Ethernet mac address

                if (Ethernet) {
                    sprintf(command,"cat /sys/class/net/%s/address",devname);
                    pf = popen(command,"r");
                    if (pf)  {

                        if(fgets(data,maxdatasize,pf)) {
                            pclose(pf);
                            _parsemac(data,ethmacaddr);

                        } else {
                            IOT_ERROR("[rpi] Could not read Ethernet mac address %s",devname);
                            pclose(pf);
                        }
                    } else {
                        IOT_ERROR("[rpi] Could not check Ethernet mac address %s",devname);
                    }
                }
            }

        } else {

            IOT_ERROR("[rpi] Could not check Ethernet status %s",devname);
            pclose(pf);
            Ethernet = false;
        }



    } else {

        IOT_ERROR("[rpi] Could not read device directory");
        Ethernet = false;

    }

    // NOW GET WIRELESS DEVICE NAMES

    strcpy(wifi_ap_dev,"");
    strcpy(wifi_sta_dev, "");

    pf = popen("iw dev","r");

    if (pf) {
        while(fgets(data,maxdatasize,pf)) {             // search for Interface line

            if ((textptr = strstr(data,"Interface"))) {

                i = 0;
                textptr += 10;

                while ((*(textptr+i) != ' ') && (*(textptr+i) != '\n'))  {             // get the device name

                    *(devname+i) = *(textptr+i);
                    ++i;
                }
                devname[i] ='\0';
                found = false;

                while(fgets(data,maxdatasize,pf) && !found) {     // search for type line
                    if ((textptr = strstr(data,"type"))) {

                        i = 0;
                        textptr += 5;

                        while ((*(textptr+i) != ' ') && (*(textptr+i) != '\n')) {       // get the type descriptor

                            *(devtype+i) = *(textptr+i);
                            ++i;
                        }
                        devtype[i] = '\0';

                        if (strcmp(devtype,"managed") == 0) {
                            strcpy(wifi_sta_dev, devname);
                            memcpy(wifimacaddr,tmpmac,7);
                        }
                        else {
                            if (strcmp(devtype,"AP") == 0) {
                                strcpy(wifi_ap_dev,devname);
                                if (wifimacaddr[0] == 0)
                                    memcpy(wifimacaddr,tmpmac,7);

                            }
                        }
                        found = true;

                    } else {

                        if ((textptr = strstr(data,"addr")))
                            _parsemac(textptr+5,tmpmac);        // grab the mac address

                    }

                }   // end search-for-type loop
            }

        }    // end Interface search loop

        pclose(pf);


    } else {

        IOT_ERROR("[rpi] Could not perform iw dev command");
        return 0;
    }

    return 1;

}
// Make sure start and stop script files are found
bool _checksoftapcontrol(char *dir) {

	FILE *pf;
	char filename[60];
	int okflag=0;
	int errnum;

	// Check for Start script file in both given dir and current dir

	strcpy(filename,dir);
	strcat(filename,SOFTAPSTARTFILE);
	pf = fopen(filename,"r");
    if (pf) {
		fclose(pf);
		strcpy(SOFTAPSTART,filename);
        okflag++;
    } else {

        errnum=errno;
		if (errnum == ENOENT) {
	 		strcpy(filename,"./");
			strcat(filename,SOFTAPSTARTFILE);
			pf = fopen(filename,"r");
			if (pf) {
				fclose(pf);
				strcpy(SOFTAPSTART,filename);
				okflag++;
			} else {

				errnum=errno;
				if (errnum == ENOENT)
					IOT_ERROR("[rpi] SoftAP start script not found");
				else
					IOT_ERROR("[rpi] Can't validate SoftAP start script");
			}
		} else
			IOT_ERROR("[rpi] Can't validate SoftAP start script");
	}

    // Check for Stop script file in both given dir and current dir

	strcpy(filename,dir);
	strcat(filename,SOFTAPSTOPFILE);
	pf = fopen(filename,"r");
	if (pf) {
		fclose(pf);
		strcpy(SOFTAPSTOP,filename);
		okflag++;
	} else {

		errnum=errno;
		if (errnum == ENOENT) {
			strcpy(filename,"./");
			strcat(filename,SOFTAPSTOPFILE);
			pf = fopen(filename,"r");
			if (pf) {
				fclose(pf);
				strcpy(SOFTAPSTOP,filename);
				okflag++;
			} else {
				errnum=errno;
				if (errnum == ENOENT)
					IOT_ERROR("[rpi] SoftAP stop script not found");
				else
					IOT_ERROR("[rpi] Can't validate SoftAP stop script");
			}
		} else
			IOT_ERROR("[rpi] Can't validate SoftAP stop script");
	}

    if (okflag == 2)
        return true;
    else
        return false;

}

// Parse parameter / value pairs from config file
int _parseconfparm(char *parmstr, char *text) {

    char *subtxtptr;
    int i;

    subtxtptr = strstr(text,"=");

    if (subtxtptr) {
        subtxtptr++;
        while(*subtxtptr == ' ')
            subtxtptr++;

        i=0;
        while((*(subtxtptr+i) != '\0') && (*(subtxtptr+i) != '\n')) {
            *(parmstr+i) = *(subtxtptr+i);
            i++;
        }
        *(parmstr+i) = 0;

        if (i>0)
            return(1);
        else
            return(0);
    }
    return(0);

}

// Validate network interface device is up
int _isupIface(char *devname) {

    FILE *pf;
    const int maxdatasize = 1200;
    char data[maxdatasize];
    char command[300];
    char devsearch[10];
    char *lineptr;
    char *delim1;
    char *delim2;
    char *sublineptr;

    sprintf(command,"ip link show ");
    strcat(command,devname);

    pf = popen(command,"r");

    if (pf)  {

        if(fgets(data,maxdatasize,pf)) {

            pclose(pf);
            strcpy(devsearch,devname);
            strcat(devsearch,":");
            lineptr = strstr(data,devsearch);

            if (lineptr) {

                delim1 = strstr(lineptr,"<");
                if (delim1) {

                    delim2 = strstr(lineptr,">");
                    if (delim2) {

                        sublineptr = strstr(lineptr,",UP");
                        if ((sublineptr) && ((sublineptr > delim1) && (sublineptr < delim2)))
                            return(1);

                    }
                }
            }
        } else
            pclose(pf);

    }

    IOT_ERROR("[rpi] Can't execute ip link show command");
    return(0);

}

// check if wifi device is configured
int _isconfWifi(char *devname, char *ssid) {

    FILE *pf;

    const int maxdatasize = 1200;
    char data[maxdatasize];
    char command[300];
    char searchstr[30];
    char *lineptr;
    int i;

    strcpy(command,"iw ");
    strcat(command,devname);
    strcat(command," info");

    strcpy(searchstr,"Interface ");
    strcat(searchstr,devname);

    pf = popen(command,"r");

    if(fgets(data,maxdatasize,pf)) {

        lineptr = strstr(data,searchstr);

        if (lineptr)  {

            while(fgets(data,maxdatasize,pf)) {
                lineptr = strstr(data,"ssid");
                if (lineptr) {
                    lineptr = lineptr + 5;
                    i=0;
                    while ((*(lineptr+i) != ' ') && (*(lineptr+i) != '\n')) {
                        *(ssid+i) = *(lineptr+i);
                        i++;
                    }
                    *(ssid+i) = '\0';
                    break;
                }

            }

        } else {
            pclose(pf);
            return(0);
        }

        pclose(pf);
        return(1);

    }

    return(0);

}

/*******************************************************************************************
    Required BSP fuction: iot_bsp_wifi_set_mode()

    Purpose:    Switch wireless operation to request modes (off/scan/station/AP)

    Input:      iot_wifi_conf

    Output:     return IOT_ERROR_ value (IOT_ERROR_NONE if no errors)

*******************************************************************************************/

iot_error_t iot_bsp_wifi_set_mode(iot_wifi_conf *conf)
{
	//iot_wifi_scan_result_t scanresult[IOT_WIFI_MAX_SCAN_RESULT];

	char connected_ssid[IOT_WIFI_MAX_SSID_LEN+1];
	int scancount;
	char SoftAPdev[16];


//	IOT_INFO("[rpi] iot_bsp_wifi_set_mode = %d", conf->mode);
	IOT_DUMP(IOT_DEBUG_LEVEL_DEBUG, IOT_DUMP_BSP_WIFI_SETMODE, conf->mode, 0);

	switch(conf->mode) {
	case IOT_WIFI_MODE_OFF:

        IOT_INFO("[rpi] Requested mode OFF");

        if (AP_ON && ManageAP) {

            _SoftAPControl("stop");                     // make sure hostapd/dnsmasq are stopped

            if (STWifionly) {                           // if wlan0 only then switch mode back to station
                if (! _switchmode("STA")) {
                    IOT_ERROR("[rpi] Failed to switch wireless modes");
                    return IOT_ERROR_CONN_OPERATE_FAIL;
                }
            }
        }

        _restoreAP();                                    // restore prior AP config if AP-only wifi

		break;

	case IOT_WIFI_MODE_SCAN:

        IOT_INFO("[rpi] Requested mode SCAN");

        if (!AP_ON || DualWifidev || APWifionly) {

            scancount = _perform_scan();                        // do scan and check resulting AP count

            if (scancount == 0) {                               // if no results...

                usleep(SCANMODEWAITTIME);                       //     pause and
                scancount = _perform_scan();                    //     try one more time
            }

            IOT_INFO("[rpi] WiFi scan completed. %d APs found",scancount);
        } else
            IOT_INFO("[rpi] Scan not performed while in AP mode");

		break;

	case IOT_WIFI_MODE_STATION:     // For PI this could be either Wifi client or use ETH0


        IOT_INFO("[rpi] Requested mode STATION");

        if (AP_ON && ManageAP) {                                          // Turn off SoftAP

            if (!_SoftAPControl("stop"))
                IOT_INFO("[rpi] Problem stopping SoftAP");

            if (STWifionly) {
                if (! _switchmode("STA")) {
                    IOT_ERROR("[rpi] Failed to switch wifi mode");
                    return IOT_ERROR_CONN_OPERATE_FAIL;
                }
            }

        }

        _restoreAP();                                    // restore prior AP config if AP only wifi

        if (STWifionly) {
            usleep(SOFTAPWAITTIME);                         // Switching wlan0; give it some time to transition
            usleep(SOFTAPWAITTIME);
        }

        if(DualWifidev || STWifionly)  {

            //NOW CHANGE CONNECTION PER conf->ssid

            if (!_switchSSID(wifi_sta_dev,conf->ssid)) {    // Switch to ssid; if failed...

                if (Ethernet)
                    IOT_INFO("[rpi] Could not connect to ssid %s. Will use Ethernet",conf->ssid);
                else {
                    IOT_ERROR("[rpi] Failed to connect to ssid %s",conf->ssid);
                    return IOT_ERROR_CONNECT_FAIL;
                }

            } else {                                       // Switch SSID went OK

                usleep(SSIDWAITTIME);

                if (!_waitWifiConn(wifi_sta_dev,connected_ssid))  {   // confirm connected SSID

                    if (Ethernet)
                        IOT_INFO("[rpi] Didn't connect to ssid %s.  Will use Ethernet",conf->ssid);
                    else {
                        IOT_ERROR("[rpi] Failed to connect to ssid %s",conf->ssid);
                        return IOT_ERROR_NET_CONNECT;
                    }
                } else
                    IOT_INFO("[rpi] Connected to AP SSID: %s", conf->ssid);

            }

        }

		break;

	case IOT_WIFI_MODE_SOFTAP:

        IOT_INFO("[rpi] Requested mode SoftAP");

        if (STWifionly)
            strcpy(SoftAPdev,wifi_sta_dev);
        else
            strcpy(SoftAPdev,wifi_ap_dev);

        if (!_setupHostapd(conf->ssid,conf->pass,SoftAPdev)) {               // Setup hostapd.conf file with ssid & password
            IOT_ERROR("[rpi] Couldn't update hostapd.conf file");
            return IOT_ERROR_CONN_OPERATE_FAIL;
        }


        if (!DualWifidev)
            usleep(SOFTAPWAITTIME);                                 // pause to let wireless come up

        if (STWifionly) {
            if (!_switchmode("AP")) {
                IOT_ERROR("[rpi] Failed to switch to AP wifi mode on device %s",SoftAPdev);
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }
        }

        usleep(SOFTAPWAITTIME);
        usleep(SOFTAPWAITTIME);

        if (APWifionly) {
            if (!_SoftAPControl("stop")) {                             // start hostapd & dnsmasq services
                IOT_ERROR("[rpi] Problem stopping SoftAP");
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }
            usleep(SOFTAPWAITTIME);
        }

        if (!_SoftAPControl("start")) {                             // start hostapd & dnsmasq services
            IOT_ERROR("[rpi] Problem starting SoftAP");
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }

        usleep(SOFTAPWAITTIME);                                 //pause to let hostapd & dnsmasq to come up

        // Confirm hostapd has started
        if (!_checkstartSoftAP("hostapd")) {
            usleep(SOFTAPWAITTIME);
            if (!_checkstartSoftAP("hostapd")) {
                IOT_ERROR("[rpi] SoftAP service failed to start");
                    return IOT_ERROR_CONN_OPERATE_FAIL;
            }
        }

        if (APWifionly)
            APWifionlyRestore=true;

		IOT_DEBUG("wifi_init_softap finished.SSID:%s password:%s",
				wifi_config.ap.ssid, wifi_config.ap.password);


        IOT_INFO("[rpi] AP Mode Started on device %s",SoftAPdev);

		break;

	default:
		IOT_ERROR("RPI cannot support this mode = %d", conf->mode);
		IOT_DUMP(IOT_DEBUG_LEVEL_ERROR, IOT_DUMP_BSP_WIFI_ERROR, conf->mode, __LINE__);
		return IOT_ERROR_CONN_OPERATE_FAIL;
	}

	return IOT_ERROR_NONE;
}

int _enableWifi(char *dev) {


    if(_softblock(PHYSWIFIDEV,'N')) {                       // ensure softblock is off for wifi

        usleep(SEQSYSCMDWAIT);

        if (strcmp(dev,wifi_ap_dev) != 0) {                 // If not AP device check (won't show 'UP')

            if (_isupIface(dev))                            // verify interface is up
                return(1);

            else
                IOT_ERROR("[rpi] Wifi interface is down");
        } else
            return(1);
    }
    else
        IOT_ERROR("[rpi] Wifi couldn't be enabled");


    return(0);

}

int _softblock(char *phys, char setter) {                                       // input parm = 'Y' or 'N' to turn off or on wifi

    const int maxdatasize = 100;
    char data[maxdatasize];
    char unblockcommand[25] = "sudo rfkill unblock ";
    char blockcommand[25] = "sudo rfkill block ";
    char getblockstat[] ="sudo rfkill list all";
    char *lineptr;
    char devid[2];
    FILE *pf, *pf2;;

    pf = popen(getblockstat,"r");
    if (pf) {

        while (fgets(data,maxdatasize,pf)) {

            lineptr = strstr(data,phys);
            if (lineptr) {

                devid[0] = data[0];
                devid[1] = 0;
                strcat(unblockcommand,devid);
                strcat(blockcommand,devid);

                while(fgets(data,maxdatasize,pf)) {

                    lineptr = strstr(data,"Soft blocked");

                    if (lineptr) {

                        if ((strstr(data,"yes") && (setter == 'N'))) {
                            pf2 = popen(unblockcommand,"r");
                            pclose(pf2);
                        }
                        else if ((strstr(data,"no") && (setter == 'Y'))) {
                            pf2 = popen(blockcommand,"r");
                            pclose(pf2);
                        }
                    }
                }
            }
        }
    } else {
        IOT_ERROR("[rpi] Failed rfkill command");
        return (0);
    }

    pclose(pf);

    return (1);
}

int _waitWifiConn(char *dev, char *ssid) {

    int retries;

    retries = SSIDWAITRETRIES;

    strcpy(ssid, "");

    while (retries > 0) {

        _isconfWifi(wifi_sta_dev,ssid);
        if (strlen(ssid) > 0)
            retries = 0;
        else {
            usleep(SSIDWAITTIME);    // give some time for wlan connection
            retries--;
        }
    }

    if (strlen(ssid)>0)
        return(1);
    else
        return(0);

}

// This function checks status of hostapd service to be sure it started
bool _checkstartSoftAP(char *service) {

    #define SERVSTATCMD "systemctl status "
    const int maxdatasize = 200;
    FILE *pf;
    char *lineptr;
    int errnum;
    char command[20];
    char data[maxdatasize];
    bool fflag = false;
    int linecounter = 0;
    int foundstartedline = 0;

    strcpy(command,SERVSTATCMD);
    strcat(command,service);
    pf = popen(command,"r");

    if (pf) {

        while (fgets(data,maxdatasize,pf)) {
            linecounter++;
            lineptr = strstr(data,"AP-ENABLED");

            if (lineptr) {

                while (fgets(data,maxdatasize,pf)) {
                    linecounter++;
                    lineptr = strstr(data,"Started Advanced IEEE 802.11 AP");

                    if (lineptr) {
                        fflag=true;
                        foundstartedline=linecounter;
                    }
                }
            }
        }
        pclose(pf);
        if (fflag && (foundstartedline == linecounter))
            return true;

    } else {
        errnum=errno;
        IOT_INFO("[rpi] WARNING: Failed to execute service status command; error #%d",errnum);
    }

    return false;
}

bool _switchmode(char *mode) {

    #define DHCPCDCONF "/etc/dhcpcd.conf"

    FILE *pf;
    int errnum=0;
    char command[100];

    if (strcmp(mode,"AP") == 0)
        strcpy(command,"sudo sed -i \"/^#\tnohook wpa_supplicant/c\\\tnohook wpa_supplicant\" ");
    else
        strcpy(command,"sudo sed -i \"/^\tnohook wpa_supplicant/c\\#\tnohook wpa_supplicant\" ");


    strcat(command,DHCPCDCONF);

    pf = popen(command, "r");
    if (pf) {
        pclose(pf);

        strcpy(command,"sudo systemctl restart dhcpcd");
        pf = popen(command, "r");
        if (pf) {
            pclose(pf);
            return true;
        } else {
            errnum=errno;
            IOT_ERROR("[rpi] Failed to perform dhcpcd restart; error #%d",errnum);
            return false;
        }
    } else {
        errnum=errno;
        IOT_ERROR("[rpi] Failed to update dhcpcd.conf; error #%d",errnum);
        return false;
    }

}

int _SoftAPControl(char *cmd) {

    FILE *pf;
    char command[30];
    int errnum;


	strcpy(command,"bash ");
    if (strcmp(cmd,"start") == 0)
        strcat(command,SOFTAPSTART);

    else {
        if (strcmp(cmd,"stop") == 0)
			strcat(command, SOFTAPSTOP);

        else
            return 0;
	}

    pf = popen(command,"r");

    if (!pf) {

        errnum = errno;
        if (errnum == ENOENT)
            IOT_ERROR("[rpi] Missing Service %s service control script",cmd);
        else
            IOT_ERROR("[rpi] File open error %d on %s service control script",errnum,cmd);

        return (0);
    }

    pclose(pf);

    if (strcmp(cmd,"start") == 0)
        AP_ON = true;
    else
        AP_ON = false;

    IOT_INFO("[rpi] SoftAP service %s issued",cmd);
    return(1);
}

int _setupHostapd(char*ssid, char *password, char *iface) {

    FILE *pf;
    char *readline;
    unsigned int len = sizeof(readline);
    char *textptr;
    char readSSID[IOT_WIFI_MAX_SSID_LEN+1];
    char readPW[35];
    char readInterface[MAXDEVNAMESIZE+1];

    int rc;
    int errnum;
    int progcount = 0;
    int updateflag = 0;

    if((pf=fopen(SOFTAPCONFFILE, "r"))) {

        while (getline(&readline,&len,pf)!= EOF) {

            if ((textptr = strstr(readline,"ssid="))) {
                if (textptr == readline) {                  // make sure SSID= is at beginning of line
                    progcount++;
                    _parseconfparm(readSSID,textptr);

                    if (strcmp(readSSID,ssid) != 0)
                        updateflag++;
                }
            } else  {

                if ((textptr = strstr(readline,"wpa_passphrase"))) {
                    progcount++;
                    _parseconfparm(readPW,textptr);

                    if (strcmp(readPW,password) != 0)
                        updateflag++;
                } else {

                    if ((textptr = strstr(readline,"interface"))) {

                        progcount++;
                        _parseconfparm(readInterface,textptr);

                        if (strcmp(readInterface,iface) != 0)
                            updateflag++;

                    }

                }

            }
        }

        fclose(pf);
        if (readline)
            free(readline);

        if ((progcount == 3) && (updateflag > 0)) {         // if found ssid, password, and interface ok, AND any needs updating

            if(_updateHfile(SOFTAPCONFFILE,ssid,password,iface)) {       // update hostapd.conf file with ssid & password

                IOT_INFO("[rpi] hostapd config file updated with ssid=%s, pw=%s, device=%s",ssid,password,iface);
                rc = 1;
            }
            else {

                IOT_ERROR("[rpi] Could not update hostapd config file: %s",SOFTAPCONFFILE);
                rc = 0;
            }

        }
        else {
            if (progcount < 3) {
                IOT_ERROR("[rpi] Missing interface, ssid, or passphrase in %s",SOFTAPCONFFILE);
                rc = 0;
            }
        }
        rc = 1;

    }
    else {
        errnum = errno;
        IOT_ERROR("[rpi] Could not open config file: %s; errno=%d",SOFTAPCONFFILE,errnum);
        rc = 0;
    }

    return(rc);

}

int _updateHfile(char *fname, char *ssid,char *password, char *iface) {

    #define TMPFILENAME "./__tempconf"

    FILE *fp1, *fp2, *fp3;

    #define MAX 100
    char *readline;
    char *textptr;
    char command[300];
    unsigned int len = 0;

    fp1 = fopen(fname,"r");
    if (!fp1) {
        IOT_ERROR("[rpi] Cannot open %s",fname);
        return(1);
    }

    fp2 = fopen(TMPFILENAME,"w");
    if (!fp2) {
        IOT_ERROR("[rpi] Cannot open temp file %s",TMPFILENAME);
        fclose(fp1);
        return(1);
    }

    while (getline(&readline,&len,fp1)!= EOF) {


        if ((textptr = strstr(readline,"ssid="))) {
            if (textptr == readline)

                fprintf(fp2,"ssid=%s\n",ssid);

        }
        else {

            if ((textptr = strstr(readline,"wpa_passphrase=")))

                fprintf(fp2,"wpa_passphrase=%s\n",password);

            else {

                if ((textptr = strstr(readline,"interface=")))

                    fprintf(fp2,"interface=%s\n",iface);

                else

                    fprintf(fp2,"%s",readline);
            }
        }

    }

    if (readline)
        free(readline);
    fclose(fp1);
    fclose(fp2);

    sprintf(command,"sudo rm %s",PRIORCONF);
    fp3 = popen(command,"r");                               // delete prior saved config file (ok if doesn't exist)
    fclose(fp3);

    sprintf(command,"sudo cp %s %s",fname,PRIORCONF);
    fp3 = popen(command, "r");                              // save current hostapd.conf to another file
    if (fp3) {
        fclose(fp3);
        sprintf(command,"sudo rm %s",fname);
        fp3 = popen(command, "r");                       // delete current hostapd.conf
        if (fp3) {
            fclose(fp3);
            sprintf(command,"sudo cp %s %s",TMPFILENAME,fname);
            fp3 = popen(command, "r");                   // copy new file to hostapd.conf
            if (fp3) {
                fclose(fp3);
                sprintf(command,"sudo rm %s",TMPFILENAME);
                fp3 = popen(command, "r");              // delete temporary file
                fclose(fp3);
            }
            else {
                IOT_ERROR("[rpi] Cannot copy new %s",fname);
                return(0);
            }
        }
        else {
            IOT_ERROR("[rpi] Cannot remove current %s",fname);
            return(0);
        }
    }
    else {
        IOT_ERROR("[rpi] Cannot copy current %s to %s",fname,PRIORCONF);
        return(0);
    }


    return(1);
}

bool _restorehfile(char *backupfile) {

    FILE *pf;
    char command[60];
    int errnum;

    sprintf(command,"sudo cp %s %s",backupfile,SOFTAPCONFFILE);

    pf = popen(command, "r");
    errnum = errno;

    if (!pf) {
        IOT_ERROR("[rpi] Failed to restore hostapd config file to prior state; error #%d",errnum);
        return false;
    } else {
        pclose(pf);
        return true;
    }

}

// if full-time AP wifi, then restore prior config if necessary
bool _restoreAP() {

    if (AP_ON && APWifionly && APWifionlyRestore) {

        if (!_SoftAPControl("stop")) {
            IOT_ERROR("[rpi] Problem stopping SoftAP");
            return false;
        }

        if (!_restorehfile(PRIORCONF)) {
            IOT_ERROR("[rpi] Couldn't restore hostapd.conf");
            return false;
        }

        if (!_SoftAPControl("start")) {
            IOT_ERROR("[rpi] Problem re-starting SoftAP");
            return false;
        }

        APWifionlyRestore=false;
    }
    return true;
}

int _switchSSID(char *dev, char *ssid) {

    FILE *pf;
    char command[100];
    const int maxdatasize = 50;
    char data[maxdatasize];
    char *lineptr;
    int foundid = 0;
    char netid;

    sprintf(command,"wpa_cli -i %s list_networks",dev);

    pf = popen(command, "r");
    if (pf) {
        while ((fgets(data,maxdatasize,pf)) && (foundid == 0)) {

            lineptr = strstr(data,ssid);
            if (lineptr) {
                netid = *data;
                foundid = 1;
                pclose(pf);
                sprintf(command,"wpa_cli -i %s select_network %c",dev,netid);
                pf = popen(command,"r");
                if (!pf) {
                    IOT_ERROR("[rpi] Cannot connect to %s",ssid);
                    return(0);
                }
            }
        }

        if (!foundid) {
            pclose(pf);
            IOT_ERROR("[rpi] %s not currently available to connect",ssid);
            return(0);
        }


    } else {
        IOT_ERROR("[rpi] Failed to issue wpa_cli command");
        return(0);
    }

    return(1);
}

uint16_t iot_bsp_wifi_get_scan_result(iot_wifi_scan_result_t * scan_result)
{
    int index;

    IOT_INFO("[rpi] Get scan result requested");

    for(index = 0;index < scanstore.apcount;index++) {

        scan_result[index].bssid[0] = scanstore.apdata[index].bssid[0];
        scan_result[index].bssid[1] = scanstore.apdata[index].bssid[1];
        scan_result[index].bssid[2] = scanstore.apdata[index].bssid[2];
        scan_result[index].bssid[3] = scanstore.apdata[index].bssid[3];
        scan_result[index].bssid[4] = scanstore.apdata[index].bssid[4];
        scan_result[index].bssid[5] = scanstore.apdata[index].bssid[5];

        strcpy((char *) scan_result[index].ssid, (char *) scanstore.apdata[index].ssid);

        scan_result[index].rssi = scanstore.apdata[index].rssi;
        scan_result[index].freq = scanstore.apdata[index].freq;

        scan_result[index].authmode = scanstore.apdata[index].authmode;

    }

    return(scanstore.apcount);
}

int _perform_scan()  {

    const int maxdatasize = 1200;
    #define LINUXWIFISCAN_p1 "sudo iw "
    #define LINUXWIFISCAN_p2 " scan"

    FILE *pf;
    char command[30];
    char data[maxdatasize];
    char scandev[16];
    int ap_num;
    char *lineptr;
    int errnum, ferr;
    int i;
    char tmpbuf[IOT_WIFI_MAX_SSID_LEN+1];

    if (strcmp(wifi_sta_dev,"") != 0)
        strcpy(scandev,wifi_sta_dev);
    else
        strcpy(scandev,wifi_ap_dev);

    IOT_INFO("[rpi] Running Wifi AP scan using %s",scandev);

    // Execute system command to get wifi scan list
    strcpy(command,LINUXWIFISCAN_p1);
    strcat(command, scandev);
    strcat(command, LINUXWIFISCAN_p2);

    scanstore.apcount = 0;
    ap_num = -1;

    pf = popen(command,"r");

    if (pf) {

        // Read each line output from system command
        while (fgets(data,maxdatasize,pf)) {

            lineptr = strstr(data,"BSS ");                          // Start of new Mac Address? (BSS)
            if (lineptr) {


                if (lineptr == data) {                              // Make sure it is really a BSS record; should be no leading chars
                    ap_num = ap_num + 1;                                // Increment AP index

                    scanstore.apdata[ap_num].authmode = IOT_WIFI_AUTH_OPEN;  // Deafult to Open auth mode in case not specified

                    _parsemac(lineptr+4, scanstore.apdata[ap_num].bssid);    // Get Mac Addr and convert ASCII to 6-byte format
                }
            }

            else {
                lineptr = strstr(data,"\t\t * primary channel:");
                if (lineptr)
                                                                    // Found Wifi Channel
                    scanstore.apdata[ap_num].freq = _getnumeric(2,lineptr+22);

                else {
                    lineptr = strstr(data,"\tsignal:");
                    if (lineptr)
                                                                    // Found Wifi Signal Level (RSSI)
                        scanstore.apdata[ap_num].rssi = _getnumeric(6,lineptr+9);

                    else {
                        lineptr = strstr(data,"\tSSID:");
                        if (lineptr) {
                                                                                    // Found SSID
                            i=0;
                            while ((i < IOT_WIFI_MAX_SSID_LEN) && (*(lineptr+i+7) != ' ') && (*(lineptr+i+7) != '\n')) {
                                tmpbuf[i] = *(lineptr + i + 7);
                                i++;
                            }
                            tmpbuf[i] = 0;

                            strcpy((char*)scanstore.apdata[ap_num].ssid, tmpbuf);

                        }
                        else {
                            lineptr = strstr(data,"\t\t * Pairwise ciphers:");          // This is the Auth mode line
                            if (lineptr) {

                                if (strstr(lineptr,"CCMP TKIP"))
                                    scanstore.apdata[ap_num].authmode = IOT_WIFI_AUTH_WPA_WPA2_PSK;

                                else

                                    if (strstr(lineptr,"TKIP CCMP"))
                                        scanstore.apdata[ap_num].authmode = IOT_WIFI_AUTH_WPA_WPA2_PSK;

                                    else

                                        if (strstr(lineptr,"CCMP"))
                                            scanstore.apdata[ap_num].authmode = IOT_WIFI_AUTH_WPA2_PSK;

                                        else

                                            if (strstr(lineptr,"TKIP"))
                                                scanstore.apdata[ap_num].authmode = IOT_WIFI_AUTH_WPA_PSK;

                                            else

                                                if (strstr(lineptr,"WEP"))
                                                    scanstore.apdata[ap_num].authmode = IOT_WIFI_AUTH_WEP;   //Need to test this

                            }
                        }
                    }
                }
            }
        }
        ferr = ferror(pf);
        if (ferr) {
            errnum = errno;
            IOT_ERROR("[rpi] Error reading scan results; ferror=%d, errno=%d",ferr,errnum);
        }
        pclose(pf);

    } else
        IOT_ERROR("[rpi] Failed to issue iw scan command");

    scanstore.apcount = ap_num+1;

    return (ap_num+1);
}

// This function parses presumed ASCII numeric digits; stops when max digits parsed or space or carriage return found
int _getnumeric(int maxdigits, char *text) {

    int index = 0;
    char tmpbuf[10];

    while ((index < maxdigits) && (*(text+index) != ' ') && (*(text+index) != '\n')) {
        tmpbuf[index] = *(text+index);
        index++;
    }
    tmpbuf[index] = 0;
    return(atoi(tmpbuf));

}

/*******************************************************************************************
    Required BSP fuction: iot_bsp_wifi_get_mac()

    Purpose:    Discover and return this device's mac address

    Input:      mac address structure (iot_mac)

    Output:     return IOT_ERROR_ value (IOT_ERROR_NONE if no errors) + filled-in wifi_mac

*******************************************************************************************/
iot_error_t iot_bsp_wifi_get_mac(struct iot_mac *wifi_mac)  {

    char txt[18];

    IOT_INFO("[rpi] Mac address requested");

    if (Ethernet)
        memcpy(wifi_mac->addr,ethmacaddr,IOT_WIFI_MAX_BSSID_LEN);

    else
        memcpy(wifi_mac->addr,wifimacaddr,IOT_WIFI_MAX_BSSID_LEN);


    sprintf(txt,"%x:%x:%x:%x:%x:%x", wifi_mac->addr[0], \
                                            wifi_mac->addr[1], \
                                            wifi_mac->addr[2], \
                                            wifi_mac->addr[3], \
                                            wifi_mac->addr[4], \
                                            wifi_mac->addr[5]);
    IOT_INFO("[rpi] Mac=%s",txt);

    return IOT_ERROR_NONE;
}

/*******************************************************************************************
    Required BSP fuction: iot_bsp_wifi_get_freq()

    Purpose:    Determine wireless frequency in use by device

    Input:      none

    Output:     IOT_WIFI_FREQ_2_4G_ONLY (only valid value at this time; 5 GHz not supported)

*******************************************************************************************/
iot_wifi_freq_t iot_bsp_wifi_get_freq(void)  {

    return IOT_WIFI_FREQ_2_4G_ONLY;
}

void _parsemac(char *textptr, uint8_t *hexbuf) {

    *(hexbuf+0) = _htoi(textptr + 0);
    *(hexbuf+1) = _htoi(textptr + 3);
    *(hexbuf+2) = _htoi(textptr + 6);
    *(hexbuf+3) = _htoi(textptr + 9);
    *(hexbuf+4) = _htoi(textptr + 12);
    *(hexbuf+5) = _htoi(textptr + 15);

    return;
}

unsigned int _htoi (const char *ptr) {

    unsigned int value = 0;
    char ch = *ptr;

    while (ch == ' ' || ch == '\t')
        ch = *(++ptr);

    for (;;) {

        if (ch >= '0' && ch <= '9')
            value = (value << 4) + (ch - '0');
        else if (ch >= 'A' && ch <= 'F')
            value = (value << 4) + (ch - 'A' + 10);
        else if (ch >= 'a' && ch <= 'f')
            value = (value << 4) + (ch - 'a' + 10);
        else
            return value;

        ch = *(++ptr);
    }

}



/*
iot_error_t iot_bsp_wifi_get_mac(struct iot_mac *wifi_mac)
{
	struct ifreq ifr;
	int sockfd = 0;
	iot_error_t err = IOT_ERROR_NONE;

	sockfd = _create_socket();
	if (sockfd < 0)
		return IOT_ERROR_READ_FAIL;

	strncpy(ifr.ifr_name, IFACE_NAME, IF_NAMESIZE);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		IOT_ERROR("ioctl(%d, %s): 0x%x", errno, strerror(errno), SIOCGIFHWADDR);
		err = IOT_ERROR_READ_FAIL;
		goto mac_out;
	}
	memcpy(wifi_mac->addr, ifr.ifr_hwaddr.sa_data, sizeof(wifi_mac->addr));

mac_out:
	close(sockfd);
	return err;
}

*/

/*
static int _create_socket()
{
    int sockfd = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        IOT_ERROR("Can't get socket (%d, %s)", errno, strerror(errno));
        return -errno;
    }
    return sockfd;
}
*/
