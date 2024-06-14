#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <iwlib.h>

#include "tooltip.h"
#include "volume.h"

struct pw_core *core = NULL;
struct pw_context *context = NULL;
struct pw_node *node = NULL;
struct pw_device *device = NULL;


char* displaytime(void);
char* display_battery_status(int,int*);
int   display_battery_int(void);
char* get_battery_info(int);

char* display_ipv4(void);
char* display_SSID(void);
int display_net_speed(void);
char* get_speed_icon(int);

char* get_volume(float);

char*
displaytime(void)
{
	char* t = malloc(sizeof(char) * 25);
	time_t current_time;
	struct tm* time_info;
	char time_string[25] = {0};

	time(&current_time);

	time_info = localtime(&current_time);
	strftime(time_string,sizeof(char) * 25,"%d/%m/%Y %T", time_info);
	strcpy(t,time_string);


	return t;
}

int 
display_battery_int(void)
{
	char info[5];
	FILE* fp = fopen(
			"/sys/class/power_supply/BAT1/capacity","r");

	if (fgets(info, 4, fp) == NULL) {
			perror("Failed to read battery info");
			fclose(fp);
			return -1;
	}
	fclose(fp);

	return atoi(info);
}


char*
display_battery_status(int charge,int *mode)
{

	char status[12] = {0};
	char* info = malloc(sizeof(char) * 5);

	FILE* fp = fopen(
			"/sys/class/power_supply/BAT1/status","r");
	
	if (fgets(status,12,fp) == NULL) {
		perror("Failed to read battery status");
		fclose(fp);
		return NULL;
	}
//	strncpy(info,status,11);
//	return info;
	char symbol = 0;
	if(!strncmp(status,"Charging",8)) {
		symbol = '+';
		*mode = 1;
	}
	else if (!strncmp(status,"Discharging",11)) {
		symbol= '-';
		*mode = 0;
	}
	else {
		symbol = 'o';
		*mode = 0;
	}

	char* icon = NULL;
	switch(symbol) {
		case('-'):
        icon = 
            (charge >= 95)? "󰁹":
            (charge >= 85)? "󰂂":
            (charge >= 75)? "󰂁":
            (charge >= 65)? "󰂀":
            (charge >= 55)? "󰁿":
            (charge >= 45)? "󰁾":
            (charge >= 35)? "󰁽":
            (charge >= 25)? "󰁼":
            (charge >= 15)? "󰁻":
            (charge >= 10)? "󰁺":
            (charge < 10)?  "󱃍":
            "󰂑";
				break;
		case('+'):
        icon = 
            (charge >= 95)? "󰂅":
            (charge >= 85)? "󰂋":
            (charge >= 75)? "󰂊":
            (charge >= 65)? "󰢞":
            (charge >= 55)? "󰂉":
            (charge >= 45)? "󰢝":
            (charge >= 35)? "󰂈":
            (charge >= 25)? "󰂇":
            (charge >= 15)? "󰂆":
            (charge >= 10)? "󰢜":
            (charge < 10)?  "󰢟":
            "󰂑";
				break;
		case('o'):
        icon = 
            (charge >= 95)? "󰁹":
            (charge >= 85)? "󰂂":
            (charge >= 75)? "󰂁":
            (charge >= 65)? "󰂀":
            (charge >= 55)? "󰁿":
            (charge >= 45)? "󰁾":
            (charge >= 35)? "󰁽":
            (charge >= 25)? "󰁼":
            (charge >= 15)? "󰁻":
            (charge >= 10)? "󰁺":
            (charge < 10)?  "󱃍":
            "󰂑";
				break;
		default:
				icon = "?";
				break;
	}


	strncpy(info,icon,sizeof(char)*5);
	fclose(fp);
	

	return info;
}

char*
get_battery_info(int mode)
{
	char* info = malloc(sizeof(char) * 64);
	char buffer[256];
	long charge_now = 0, charge_full = 0, current_now = 0;

	   FILE* fp = fopen("/sys/class/power_supply/BAT1/charge_now", "r");
    if (fp == NULL) {
        perror("Error opening charge_now fp");
        return NULL;
    }
    fgets(buffer, sizeof(buffer), fp);
    charge_now = atoi(buffer);
    fclose(fp);

    // Read charge_full
    fp = fopen("/sys/class/power_supply/BAT1/charge_full", "r");
    if (fp == NULL) {
        perror("Error opening charge_full fp");
        return NULL;
    }
    fgets(buffer, sizeof(buffer), fp);
    charge_full = atoi(buffer);
    fclose(fp);

    // Read current_now
    fp = fopen("/sys/class/power_supply/BAT1/current_now", "r");
    if (fp == NULL) {
        perror("Error opening current_now fp");
        return NULL;
    }
    fgets(buffer, sizeof(buffer), fp);
    current_now = atoi(buffer);
    fclose(fp);

    if (mode == 0) {
        // Calculate estimated remaining battery life
        if (current_now != 0) {
            float seconds= (float)(charge_now * 3600) / current_now;
						int hours = floor(seconds/3600);
						int minutes = floor((float)((int)seconds % 3600)/60);
						if(hours >= 1)
							snprintf(info,64,"Remaining Battery Life: %d hours, %d minutes",
									hours,minutes);
						else
							snprintf(info,64,"Remaining Battery Life: %d minutes",
									(int)minutes);
        } else {
            snprintf(info,64,"Battery is not discharging\n");
        }
    } else if (mode == 1) {
        // Calculate charge time to full
        float remaining_charge = charge_full - charge_now;
        if (current_now != 0 && remaining_charge > 0) {
            float seconds = (remaining_charge * 3600) / current_now;
						int hours = floor(seconds/3600);
						int minutes = floor((float)((int)seconds % 3600)/60);
						if(hours >= 1)
							snprintf(info,64,"Time until Full Charge: %d hours, %d minutes",
									hours,minutes);
						else
							snprintf(info,64,"Time until Full Charge: %d minutes",minutes);
        } else {
            snprintf(info,64,"Battery is already fully charged\n");
        }
    } else {
        snprintf(info,64,"Invalid mode\n");
    }

	return info;
}

char*
display_ipv4(void)
{
	int sockfd;
	struct ifreq ifr;
	char* ipv4_address = NULL;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("Error opening socket");
		return NULL;
	}

	strncpy(ifr.ifr_name,"wlan0",IFNAMSIZ);
	if (ioctl(sockfd,SIOCGIFADDR, &ifr) < 0) {
		perror("Error getting interface address");
		close(sockfd);
		return NULL;
	}

	struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
	ipv4_address = inet_ntoa(addr->sin_addr);
	close(sockfd);

	return strdup(ipv4_address);
}

char*
display_SSID(void)
{
	char* SSID = NULL;
	char essid[IW_ESSID_MAX_SIZE+1];
	int sockfd = iw_sockets_open();
	if(sockfd < 0){
		perror("Error opening socket");
		return NULL;
	}

	char interface[IFNAMSIZ];
	if(iw_get_kernel_we_version()>=21) {
		struct ifreq ifr;
		strncpy(ifr.ifr_name,"wlan0",IFNAMSIZ);
		if(ioctl(sockfd,SIOCGIWNAME,&ifr) < 0) {
			perror("Error getting interface name");
			iw_sockets_close(sockfd);
			return NULL;
		}
		strncpy(interface,ifr.ifr_name,IFNAMSIZ);
	} else {
		strcpy(interface,"wlan0");
	}
	struct iwreq req;
	strncpy(req.ifr_name,interface,IFNAMSIZ);
	req.u.essid.pointer = essid;
	req.u.essid.length = IW_ESSID_MAX_SIZE+1;

	if (ioctl(sockfd,SIOCGIWESSID,&req) < 0)
	{
		perror("Error getting ESSID");
		iw_sockets_close(sockfd);
		return NULL;
	}
	iw_sockets_close(sockfd);

	SSID = strdup(essid);

	return SSID;
}

int
display_net_speed(void)
{
	int speed = 0;
	int sockfd;

	sockfd = iw_sockets_open();
	if (sockfd < 0) {
		perror("Error opening socket");
		return -1;
	}
	
	char interface[IFNAMSIZ];
	if (iw_get_kernel_we_version() >= 21) {
		struct ifreq ifr;
		strncpy(ifr.ifr_name,"wlan0", IFNAMSIZ);
		if (ioctl(sockfd,SIOCGIWNAME, &ifr) < 0) {
			perror("Error getting interface name");
			return -1;
		}
		strncpy(interface,ifr.ifr_name,IFNAMSIZ);
	} else {
		strcpy(interface,"wlan0");
	}

	struct iwreq req;
	strncpy(req.ifr_name,interface,IFNAMSIZ);

	if (ioctl(sockfd,SIOCGIWRATE,&req)<0) {
		perror("Error getting internet speed");
		iw_sockets_close(sockfd);
		return -1;
	}
	speed = req.u.bitrate.value;
	iw_sockets_close(sockfd);
	return speed;
}

char*
get_speed_icon(int speed)
{
	float speed_mbs = speed/1000000.0;
	char* icon = NULL;

	icon =
		(speed_mbs >= 150)? "󰤨":
		(speed_mbs >= 100)? "󰤥":
		(speed_mbs >= 50)? "󰤢":
		(speed_mbs >= 25)? "󰤟":
		"󰤫";

	return strdup(icon);
}

char*
get_volume(float volume)
{
	char* info = malloc(sizeof(char)*8);
	int volume_int = (int)(volume*100);
	char* icon =
		(volume_int >= 70)? "󰕾":
		(volume_int >= 50)? "󰖀":
		(volume_int > 0)? "":
		(volume_int <= 0)? "󰖁":
		"";
	snprintf(info,64,"%d %s",volume_int,icon);
	return info;
}

int
main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stdout,"N/A");
		return 1;
	}
	char* button = getenv("BLOCK_BUTTON");

	if (!strcmp(argv[1],"--time")) {
		if(button)
		{
			int button_val = atoi(button);
			if(button_val) {
				FILE *pipe = popen("st -e sh -c 'khal interactive'","r");
				pclose(pipe);
			}
		}
		char *info =  displaytime();
		fprintf(stdout,"%s",info);
		free(info);
	}

	else if(!strcmp(argv[1],"--battery")) {
		int charge = display_battery_int();
		int mode = 0;
		char* info = display_battery_status(charge,&mode);
		if(button)
		{
			int button_val = atoi(button);
			if(button_val) {
				char *battery_info = get_battery_info(mode);
				create_tooltip(battery_info);
				free(battery_info);
			}
		}
		fprintf(stdout,"%d%% %s",charge,info);
		free(info);
	}

	else if(!strcmp(argv[1],"--network")) {
		char *info = display_ipv4();
		char* info2 = display_SSID();
		int info3_d = display_net_speed();
		char* info3 = get_speed_icon(info3_d);
		if(button) {
			int button_val = atoi(button);
			if(button_val) {
				FILE *pipe = popen("st -e sh -c 'nmtui'","r");
				pclose(pipe);
				return 0;
			}
		}

		if ((info==NULL)||(info2==NULL)) {
			fprintf(stdout,"N/A N/A %s",info3);
			return 0;
		}

		fprintf(stdout,"%s %s %s",info,info2,info3);
		
		free(info);
		free(info2);
		free(info3);
	}
	else if (!strcmp(argv[1], "--volume")) {

		if(button) {
			int button_val = atoi(button);
			if(button_val) {
				FILE* pipe = popen("pavucontrol-qt","r");
				pclose(pipe);
				return 0;
			}
		}
		initialize_pulseaudio();
		float info = get_current_volume();
		char* info2 = get_volume(info);
		fprintf(stdout,"%s\n",info2);
		cleanup_pulseaudio();
		free(info2);
	}
	else {
		fprintf(stdout,"N/A");
		return 1;
	}

	return 0;
}
