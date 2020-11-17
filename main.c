#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    const int maxdatasize = 100;
    //#define DEFAULTDIR "/home/pi/st-device-sdk-c/example/"
    #define DEFAULTDIR "./"
    #define HILITE_TXT "\033[0;36m"
    #define ERR_TXT "\033[0;31m"
    #define INFO_TXT "\033[0;33m"
    #define BOLDINFO_txt "\033[01;33m"
    #define RESET_TXT "\033[0m"

    FILE *pf;

    extern int errno;

    char data[maxdatasize];
    char macaddr[8];
    char dir[100];
    char filedir[150];

    enum {
        IOT_WIFI_AUTH_OPEN = 0,
        IOT_WIFI_AUTH_WEP,
        IOT_WIFI_AUTH_WPA_PSK,
        IOT_WIFI_AUTH_WPA2_PSK,
        IOT_WIFI_AUTH_WPA_WPA2_PSK,
        IOT_WIFI_AUTH_WPA2_ENTERPRISE,
        IOT_WIFI_AUTH_MAX
    };

    if (argc == 2)  {
        if ((argv[1][0] == '-')) {

            printf("\n%sSTProv <directory>%s\n",BOLDINFO_txt,INFO_TXT);
            printf("\n\t <directory>    location of SmartThings provisioning data files");
            printf("\n\t\t\t(where the device app was run)");
            printf("\n\t\t\tIf no directory given, curent directory assumed.");
            printf("\n\nFor debugging and information use only, this utility displays the contents of the \
SmartThings Direct Connect provisioning data files which are created during initial device provisioning.");

            printf("%s\n\n",RESET_TXT);

            exit(0);
        }

        strcpy(dir,argv[1]);
        if ((dir[strlen(dir)-1]) != '/')
            strcat(dir,"/");
    }
    else {
        strcpy(dir, DEFAULTDIR);

    }
    printf("\n(Using directory: %s)\n\n",dir);

    strcpy(filedir,dir);
    strcat(filedir,"WifiProvStatus");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: WifiProvStatus does not exist in this directory%s\n\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\tWifi Provisioning Status...%s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);

    if (!strcmp(data,"NONE")) {
        printf("\n\n");
        exit(0);
    }

    strcpy(filedir,dir);
    strcat(filedir,"IotAPSSID");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: IotAPSSID does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\n\t\tSSID = %s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);


    strcpy(filedir,dir);
    strcat(filedir,"IotAPAuthType");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: IotAPAuthType does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    switch (atoi(data)) {

        case IOT_WIFI_AUTH_OPEN:
            printf("\n\t\tAuthorization type: %sOPEN%s",HILITE_TXT,RESET_TXT);
            break;

        case IOT_WIFI_AUTH_WEP:
            printf("\n\t\tAuthorization type: %sWEP%s",HILITE_TXT,RESET_TXT);
            break;

        case IOT_WIFI_AUTH_WPA_PSK:
            printf("\n\t\tAuthorization type: %sWPA%s",HILITE_TXT,RESET_TXT);
            break;

        case IOT_WIFI_AUTH_WPA2_PSK:
            printf("\n\t\tAuthorization type: %sWPA2%s",HILITE_TXT,RESET_TXT);
            break;

        case IOT_WIFI_AUTH_WPA_WPA2_PSK:
            printf("\n\t\tAuthorization type: %sWPA/WPA2%s",HILITE_TXT,RESET_TXT);
            break;

        case IOT_WIFI_AUTH_WPA2_ENTERPRISE:
            printf("\n\t\tAuthorization type: %sWPA2 Enterprise%s",HILITE_TXT,RESET_TXT);
            break;

        case IOT_WIFI_AUTH_MAX:
            printf("\n\t\tAuthorization type: %sMAX%s",HILITE_TXT,RESET_TXT);
            break;
    }

    fclose(pf);

    strcpy(filedir,dir);
    strcat(filedir,"IotAPPASS");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: IotAPPASS does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\n\t\tPassword = %s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);

    strcpy(filedir,dir);
    strcat(filedir,"IotAPBSSID");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: IotAPBSSID does not exist; ERR #%d %s\n",ERR_TXT,errno,RESET_TXT);
        exit(1);
    }

    fread(&data, 6, 1,pf);

    printf("\n\t\tMac Address:  %s",HILITE_TXT);

    sprintf(macaddr,"%x",data[0]);
    if (strlen(macaddr) == 1)
        printf("0");
    printf("%s:",macaddr);
    sprintf(macaddr,"%x",data[1]);
    if (strlen(macaddr) == 1)
        printf("0");
    printf("%s:",macaddr);
    sprintf(macaddr,"%x",data[2]);
    if (strlen(macaddr) == 1)
        printf("0");
    printf("%s:",macaddr);
    sprintf(macaddr,"%x",data[3]);
    if (strlen(macaddr) == 1)
        printf("0");
    printf("%s:",macaddr);
    sprintf(macaddr,"%x",data[4]);
    if (strlen(macaddr) == 1)
        printf("0");
    printf("%s:",macaddr);
    sprintf(macaddr,"%x",data[5]);
    if (strlen(macaddr) == 1)
        printf("0");
    printf("%s",macaddr);

    fclose(pf);

    printf("%s\n",RESET_TXT);

    // Cloud Provisioning Files

    strcpy(filedir,dir);
    strcat(filedir,"CloudProvStatus");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: CloudProvStat does not exist%s\n\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\n\tCloud Provisioning Status...%s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);

    if (!strcmp(data,"NONE"))
        exit(0);

    strcpy(filedir,dir);
    strcat(filedir,"ServerURL");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: ServerURL does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\n\t\tST Server URL = %s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);

    strcpy(filedir,dir);
    strcat(filedir,"ServerPort");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: ServerPort does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\n\t\tST Server Port = %s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);

    strcpy(filedir,dir);
    strcat(filedir,"DeviceID");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: DeviceID does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    fscanf(pf,"%s",data);

    printf("\n\n\tDevice ID = %s%s%s",HILITE_TXT,data,RESET_TXT);
    fclose(pf);


    strcpy(filedir,dir);
    strcat(filedir,"Label");

    if ((pf = fopen(filedir,"r")) == NULL) {
        printf("\n%sERROR: Label does not exist%s\n",ERR_TXT,RESET_TXT);
        exit(1);
    }

    printf("\n\tLabel = %s",HILITE_TXT);
    while (fscanf(pf,"%s",data) != EOF)

        printf("%s ",data);


    fclose(pf);

    printf("%s\n\n",RESET_TXT);

    exit(0);
}
