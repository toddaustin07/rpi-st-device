/*******************************************************************************************************************************************
Description:
    This module enables Raspberry Pi-based IOT devices to connect with Samsung SmartThings direct connected device architecture.
    This file must be compiled as part of the SmartThings SDK for Direct Connect Devices.
    This module replaces the equivalent posix module in the BSP porting directory of the SDK: iot_bsp_wifi_posix.c
    All remaining posix modules in the SDK BSP port files are used for Raspberry Pi builds, namely:
        iot_os_util_posix.c, iot_bsp_debug_posix.c, iot_bsp_nv_data_posix.c, iot_bsp_random_posix.c, iot_bsp_system_posix.c


Author:     Todd Austin toddaustin07@yahoo.com
Date:       November 2020

With thanks to Kwang-Hui of Samsung who patiently answered my many questions during development.

********************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include </home/pi/st-device-sdk-c/src/include/bsp/iot_bsp_wifi.h>
#include </home/pi/st-device-sdk-c/src/include/iot_error.h>
#include </home/pi/st-device-sdk-c/src/include/iot_debug.h>
#include "/home/pi/st-device-sdk-c/src/include/os/iot_os_util.h"
#include "/home/pi/st-device-sdk-c/src/include/iot_util.h"

#define RPICONFFILE "RPIConfig.conf"
#define DEFAULTDIR "./"

#define configtag_ETH "ETHERNET"
#define configtag_MNG "MANAGEWIFI"
#define configtag_devSTA "STATIONDEV"
#define configtag_devAP "APDEV"
#define configtag_devETH "ETHDEV"
#define configtag_dual "DUALWIFIMODE"
#define configtag_hconf "HOSTAPDCONF"

#define SSIDWAITRETRIES 5
#define SSIDWAITTIME 250000
#define SCANMODEWAITTIME 800000
#define SOFTAPWAITTIME 999999


void _parsemac(char *textptr, uint8_t *hexbuf);
unsigned int _htoi (const char *ptr);
int _getnumeric(int maxdigits, char *text);
iot_error_t _getrpiconf(char *currdir);
int _isupIface(char *devname);
int _isconfWifi(char *devname, char *ssid);
int _parseconfparm(char *parmstr, char *text);
int _enableWifi(char *dev);
int _softblock(char *dev, char set);
int _waitWifiConn(char *dev, char *ssid);
int _SoftAPControl(char *cmd);
int _setupHostapd(char*ssid, char *password);
int _updateHfile(char *fname, char *ssid,char *password);
int _switchSSID(char *dev, char *ssid);

static char WifiMng_flag;
static char Ethernet_flag;
static char DualMode_flag;
static char PHYSWIFIDEV[5] = "phy0";
static char wifi_sta_dev[12] = "wlan0";
static char wifi_ap_dev[12] = "wlan0";
static char eth_dev[12] = "eth0";
static char hconfig[100];

static int WIFI_INITIALIZED = false;

iot_error_t iot_bsp_wifi_init()
{

    char ssid[IOT_WIFI_MAX_SSID_LEN+1];

    IOT_INFO("[rpi] iot_bsp_wifi_init");

    if (!WIFI_INITIALIZED)  {

        _getrpiconf(DEFAULTDIR);                                        // read config file

        if (Ethernet_flag == 'Y') {                                     // If ethernet, then check it's connected

            if (_isupIface(eth_dev))
                IOT_INFO("[rpi] Ethernet connection confirmed: %s",eth_dev);

            else {

                IOT_ERROR("Ethernet not connected");
                return IOT_ERROR_NET_INVALID_INTERFACE;
            }

        }

        if(_isconfWifi(wifi_sta_dev,ssid)) {                             // wifi doesn't have to be on or connected, just devices defined

            IOT_INFO("[rpi] Wifi station device confirmed: %s",wifi_sta_dev);

            if(_isconfWifi(wifi_ap_dev,ssid))
                IOT_INFO("[rpi] Wifi AP device confirmed: %s",wifi_ap_dev);

            else {
                IOT_ERROR("Wifi AP device not configured: %s",wifi_ap_dev);
                return IOT_ERROR_NET_INVALID_INTERFACE;
            }
        }
        else {
            IOT_ERROR("Wifi station device not configured: %s",wifi_sta_dev);
            return IOT_ERROR_NET_INVALID_INTERFACE;
        }

    }

	WIFI_INITIALIZED = true;
	IOT_INFO("[rpi] Wifi Initialization Done");
	IOT_DUMP(IOT_DEBUG_LEVEL_DEBUG, IOT_DUMP_BSP_WIFI_INIT_SUCCESS, 0, 0);

	return IOT_ERROR_NONE;
}

iot_error_t _getrpiconf(char *currdir) {

    FILE *pf;
    char pathname[100];
    char *readline = NULL;
    size_t len = 0;
    char *textptr;
    char parmstr[50];

    // Initialize global static defaults

    WifiMng_flag = 'Y';
    Ethernet_flag = 'Y';
    DualMode_flag = 'N';

    strcpy(pathname,currdir);
    strcat(pathname,RPICONFFILE);

    if ((pf = fopen(pathname,"r")) != NULL) {

        while (getline(&readline,&len,pf)!= EOF) {

            if ((readline[0] != '#') && (readline[0] != '\n'))  {       // skip comments and blank lines

                if((textptr = strstr(readline,configtag_ETH))) {

                   if(_parseconfparm(parmstr, textptr)) {

                        if ((parmstr[0] == 'Y') || (parmstr[0] == 'y'))
                            WifiMng_flag = 'Y';

                        else if ((parmstr[0] == 'N') || (parmstr[0] == 'n'))
                            WifiMng_flag = 'N';

                    }
                } else
                    if ((textptr = strstr(readline,configtag_MNG))) {


                        if (_parseconfparm(parmstr, textptr)) {

                            if ((*parmstr == 'Y') || (*parmstr == 'y'))
                                WifiMng_flag = 'Y';

                            else if ((*parmstr == 'N') || (*parmstr == 'n'))
                                WifiMng_flag = 'N';
                        }

                    } else

                        if ((textptr = strstr(readline,configtag_devSTA))) {

                            if(_parseconfparm(parmstr,textptr))

                                strcpy(wifi_sta_dev,parmstr);

                        }
                        else

                            if ((textptr = strstr(readline,configtag_devAP))) {

                                if(_parseconfparm(parmstr,textptr))

                                    strcpy(wifi_ap_dev,parmstr);
                            }
                            else
                                if ((textptr = strstr(readline,configtag_devETH))) {

                                    if(_parseconfparm(parmstr,textptr))

                                        strcpy(eth_dev,parmstr);

                                }
                                else
                                    if ((textptr = strstr(readline,configtag_dual))) {

                                        if (_parseconfparm(parmstr, textptr)) {

                                            if ((*parmstr == 'Y') || (*parmstr == 'y'))
                                                DualMode_flag = 'Y';

                                            else if ((*parmstr == 'N') || (*parmstr == 'n'))
                                                DualMode_flag = 'N';
                                        }


                                    }
                                    else
                                        if ((textptr = strstr(readline,configtag_hconf)))

                                            _parseconfparm(hconfig, textptr);


            }
        }
        fclose(pf);

        if (readline)
            free(readline);

    }  else

        IOT_INFO("[rpi] RPI configuration file not found; defaults assumed");

    return IOT_ERROR_NONE;
}

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

iot_error_t iot_bsp_wifi_set_mode(iot_wifi_conf *conf)
{
	//iot_wifi_scan_result_t scanresult[IOT_WIFI_MAX_SCAN_RESULT];

	char connected_ssid[IOT_WIFI_MAX_SSID_LEN+1];


//	IOT_INFO("[rpi] iot_bsp_wifi_set_mode = %d", conf->mode);
	IOT_DUMP(IOT_DEBUG_LEVEL_DEBUG, IOT_DUMP_BSP_WIFI_SETMODE, conf->mode, 0);

	switch(conf->mode) {
	case IOT_WIFI_MODE_OFF:

        IOT_INFO("[rpi] Requested mode OFF");

        if (WifiMng_flag == 'Y') {

            _softblock(PHYSWIFIDEV,'Y');                // issue a soft block to wifi device

            _SoftAPControl("stop");                     // make sure hostapd/dnsmasq are stopped

        }

		break;

	case IOT_WIFI_MODE_SCAN:

        IOT_INFO("[rpi] Requested mode SCAN");

        if (WifiMng_flag == 'Y')  {

            if(!_enableWifi(wifi_sta_dev))  {                       // wifi device needs to be up; but doesn't have to be connected

                IOT_ERROR("[rpi] Cannot enable Wifi");
                return IOT_ERROR_NET_CONNECT;
            }


            usleep(SCANMODEWAITTIME);                                             //give the wifi station some time to come up

            IOT_INFO("[rpi] WiFi operating & ready for scan");

        }

		break;

	case IOT_WIFI_MODE_STATION:     // For PI this could be either Wifi client or use ETH0


        IOT_INFO("[rpi] Requested mode STATION");

        if (WifiMng_flag == 'Y')  {

            if (DualMode_flag == 'N')
                if(!_SoftAPControl("stop"))                         // if not dual mode, AP must be stopped
                    IOT_INFO("[rpi] Problem stopping SoftAP");

            if(!_enableWifi(wifi_sta_dev))  {                       // Ensure wifi is operational

                if (Ethernet_flag == 'N') {

                    IOT_ERROR("[rpi] Cannot enable Wifi station mode");
                    return IOT_ERROR_NET_CONNECT;
                }
                else
                    IOT_INFO("[rpi] Wifi not operating; Defaulting to Ethernet for station mode");

            } else
                IOT_INFO("[rpi] WiFi is operational");

            //NOW CHANGE CONNECTION PER conf->ssid
            usleep(SSIDWAITTIME);
            if (!_switchSSID(wifi_sta_dev,conf->ssid)) {
                IOT_ERROR("[rpi] Failed to connect to ssid %s",conf->ssid);
                return IOT_ERROR_CONNECT_FAIL;
            }

            usleep(SSIDWAITTIME);
        } //end if manage flag on

        // Get SSID acquired (whether managed wifi or not)

        if (_waitWifiConn(wifi_sta_dev,connected_ssid))     // Regardless of managed or not, confirm connected SSID

            strcpy(conf->ssid,connected_ssid);              // If self-managed, we need to update the connected ssid

        else {
            IOT_ERROR("[rpi] Failed to connect to ssid %s",conf->ssid);
            return IOT_ERROR_NET_CONNECT;
        }

		IOT_INFO("[rpi] Connected to AP SSID: %s", conf->ssid);

        return IOT_ERROR_NONE;

		break;

	case IOT_WIFI_MODE_SOFTAP:


        IOT_INFO("[rpi] Requested mode SoftAP");

        if (WifiMng_flag == 'Y') {

            if (!_setupHostapd(conf->ssid,conf->pass)) {               // Setup hostapd.conf file with ssid & password

                IOT_ERROR("[rpi] Couldn't update hostapd.conf file");
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }


            if(!_enableWifi(wifi_sta_dev))  {                       // wifi device needs to be up; but doesn't have to be connected

                IOT_ERROR("[rpi] Cannot enable Wifi");
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }


            usleep(SOFTAPWAITTIME);                                  // pause to let wireless come up


            if(!_SoftAPControl("start")) {                            // start hostapd & dnsmasq services
                IOT_ERROR("[rpi] Problem starting SoftAP");
                return IOT_ERROR_CONN_OPERATE_FAIL;
            }

            usleep(SOFTAPWAITTIME);                                 //pause to let hostapd & dnsmasq to come up

        }


		IOT_DEBUG("wifi_init_softap finished.SSID:%s password:%s",
				wifi_config.ap.ssid, wifi_config.ap.password);

		IOT_INFO("[rpi] AP Mode Started");

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

        if (_isupIface(dev))                                // verify interface is up

            return(1);

        else
            IOT_ERROR("[rpi] Wifi interface is down");

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

int _SoftAPControl(char *cmd) {

    FILE *pf;
    char command[40];

    strcpy(command,"sudo service hostapd ");
    strcat(command,cmd);

    pf = popen(command,"r");

    if (!pf) {

        IOT_ERROR("[rpi] Could not %s hostapd service",cmd);
        return(0);
    }
    pclose(pf);

    strcpy(command,"sudo service dnsmasq ");
    strcat(command,cmd);

    pf = popen(command,"r");

    if (!pf) {

        IOT_ERROR("[rpi] Could not %s dnsmasq service",cmd);
        return(0);
    }
    pclose(pf);

    IOT_INFO("[rpi] SoftAP service %s issued",cmd);
    return(1);
}

int _setupHostapd(char*ssid, char *password) {

    FILE *pf;
    char *readline;
    unsigned int len = sizeof(readline);
    char *textptr;
    char readSSID[IOT_WIFI_MAX_SSID_LEN+1];
    char readPW[35];

    int rc;
    int progcount = 0;
    int updateflag = 0;

    if((pf=fopen(hconfig, "r"))) {

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
                }
            }
        }

        fclose(pf);
        if (readline)
            free(readline);

        if ((progcount == 2) && (updateflag > 0)) {         // if found both ssid & password ok, AND either needs updating

            if(_updateHfile(hconfig,ssid,password)) {       // update hostapd.conf file with ssid & password

                IOT_INFO("[rpi] hostapd config file updated with ssid=%s, pw=%s",ssid,password);
                rc = 1;
            }
            else {

                IOT_ERROR("[rpi] Could not update hostapd config file: %s",hconfig);
                rc = 0;
            }

        }
        else {
            if (progcount < 2) {
                IOT_ERROR("[rpi] Missing ssid or passphrase in %s",hconfig);
                rc = 0;
            }
        }
        rc = 1;

    }
    else {
        IOT_ERROR("[rpi] Could not open hostapd config file: %s",hconfig);
        rc = 0;
    }

    return(rc);

}

int _updateHfile(char *fname, char *ssid,char *password) {

    #define TMPFILENAME "./__tempconf"
    #define PRIORCONF "./__prior_hostapd.conf"
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
            else

                fprintf(fp2,"%s",readline);
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
        IOT_ERROR("[rpi] Failed issue wpa_cli command");
        return(0);
    }

    return(1);
}

uint16_t iot_bsp_wifi_get_scan_result(iot_wifi_scan_result_t * scan_result)
{
    const int maxdatasize = 1200;
    #define LINUXWIFISCAN_p1 "sudo iw "
    #define LINUXWIFISCAN_p2 " scan"

    FILE *pf;
    char command[30];
    char data[maxdatasize];
    int ap_num;
    char *lineptr;
    int i;
    char tmpbuf[IOT_WIFI_MAX_SSID_LEN+1];

    IOT_INFO("[rpi] Wifi AP scan results requested");

    // Execute system command to get wifi scan list
    strcpy(command,LINUXWIFISCAN_p1);
    strcat(command, wifi_sta_dev);
    strcat(command, LINUXWIFISCAN_p2);

    pf = popen(command,"r");
    ap_num = -1;

    if (pf) {

        // Read each line output from system command
        while (fgets(data,maxdatasize,pf)) {

            lineptr = strstr(data,"BSS ");                          // Start of new Mac Address? (BSS)
            if (lineptr) {


                if (lineptr == data) {                              // Make sure it is really a BSS record; should be no leading chars
                    ap_num = ap_num + 1;                                // Increment AP index

                    scan_result[ap_num].authmode = IOT_WIFI_AUTH_OPEN;  // Deafult to Open auth mode in case not specified

                    _parsemac(lineptr+4, scan_result[ap_num].bssid);    // Get Mac Addr and convert ASCII to 6-byte format
                }
            }

            else {
                lineptr = strstr(data,"\t\t * primary channel:");
                if (lineptr)
                                                                    // Found Wifi Channel
                    scan_result[ap_num].freq = _getnumeric(2,lineptr+22);

                else {
                    lineptr = strstr(data,"\tsignal:");
                    if (lineptr)
                                                                    // Found Wifi Signal Level (RSSI)
                        scan_result[ap_num].rssi = _getnumeric(6,lineptr+9);

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

                            strcpy((char*)scan_result[ap_num].ssid, tmpbuf);

                        }
                        else {
                            lineptr = strstr(data,"\t\t * Pairwise ciphers:");          // This is the Auth mode line
                            if (lineptr) {

                                if (strstr(lineptr,"CCMP TKIP"))
                                    scan_result[ap_num].authmode = IOT_WIFI_AUTH_WPA_WPA2_PSK;

                                else

                                    if (strstr(lineptr,"TKIP CCMP"))
                                        scan_result[ap_num].authmode = IOT_WIFI_AUTH_WPA_WPA2_PSK;

                                    else

                                        if (strstr(lineptr,"CCMP"))
                                            scan_result[ap_num].authmode = IOT_WIFI_AUTH_WPA2_PSK;

                                        else

                                            if (strstr(lineptr,"TKIP"))
                                                scan_result[ap_num].authmode = IOT_WIFI_AUTH_WPA_PSK;

                                            else

                                                if (strstr(lineptr,"WEP"))
                                                    scan_result[ap_num].authmode = IOT_WIFI_AUTH_WEP;   //Need to test this

                            }
                        }
                    }
                }
            }
        }
        pclose(pf);

    } else
        IOT_ERROR("[rpi] Failed to issue iw scan command");


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


iot_error_t iot_bsp_wifi_get_mac(struct iot_mac *wifi_mac)  {

    const int maxdatasize = 1200;
    #define LINUXIFCONFIG "ifconfig"
    FILE *pf;
    char command[sizeof(LINUXIFCONFIG)+1];
    char data[maxdatasize];
    char *lineptr, *sublineptr;

    IOT_INFO("[rpi] Mac address requested");
    // Execute system command to get wifi scan list
    sprintf(command,LINUXIFCONFIG);
    pf = popen(command,"r");

    if (pf) {
    // Read each line output from system command
        while (fgets(data,maxdatasize,pf)) {
            //printf("\n%s",data);
            lineptr = strstr(data,wifi_sta_dev);

            if (lineptr) {

                sublineptr = strstr(lineptr, "HWaddr");

                if (sublineptr) {
                    _parsemac(sublineptr+7, wifi_mac->addr);
                    pclose(pf);
                    return IOT_ERROR_NONE;
                }
            }
        }
        pclose(pf);
    } else
        IOT_ERROR("[rpi] Failed to execute ifconfig command");

    return IOT_ERROR_CONN_OPERATE_FAIL;

}


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
    *(hexbuf+6) = 0;

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
