/*
 * dvbls
 * Copyright (C) 2012, Andrey Dyldin <and@cesbo.com>
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/dvb/version.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/net.h>
#include <errno.h>

static int adapter = 0;
static int device = 0;
static char dev_name[512];

static void iterate_dir(const char *dir, const char *filter, void (*callback)(const char *))
{
    DIR *dirp = opendir(dir);
    if(!dirp)
    {
        printf("ERROR: opendir() failed %s [%s]\n", dir, strerror(errno));
        return;
    }

    char item[64];
    const int item_len = sprintf(item, "%s/", dir);
    const int filter_len = strlen(filter);
    do
    {
        struct dirent *entry = readdir(dirp);
        if(!entry)
            break;
        if(strncmp(entry->d_name, filter, filter_len))
            continue;
        sprintf(&item[item_len], "%s", entry->d_name);
        callback(item);
    } while(1);

    closedir(dirp);
}

static int get_last_int(const char *str)
{
    int i = 0;
    int i_pos = -1;
    for(; str[i]; ++i)
    {
        const char c = str[i];
        if(c >= '0' && c <= '9')
        {
            if(i_pos == -1)
                i_pos = i;
        }
        else if(i_pos >= 0)
            i_pos = -1;
    }

    if(i_pos == -1)
        return 0;

    return atoi(&str[i_pos]);
}

static void check_device_net(void)
{
    sprintf(dev_name, "/dev/dvb/adapter%d/net%d", adapter, device);

    int fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if(!fd)
    {
        printf("ERROR: open() failed %s [%s]\n", dev_name, strerror(errno));
        return;
    }

    struct dvb_net_if net = { 0, 0 ,0 };
    if(ioctl(fd, NET_ADD_IF, &net) < 0)
    {
        printf("ERROR: NET_ADD_IF failed %s [%s]\n", dev_name, strerror(errno));
        close(fd);
        return;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.ifr_name, "dvb%d_%d", adapter, device);

    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
        printf("ERROR: SIOCGIFHWADDR failed %s [%s]\n", dev_name, strerror(errno));
    else
    {
        const unsigned char *mac = ifr.ifr_hwaddr.sa_data;
        printf("    mac:%02X:%02X:%02X:%02X:%02X:%02X\n"
               , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    close(sock);

    if(ioctl(fd, NET_REMOVE_IF, net.if_num) < 0)
        printf("ERROR: NET_REMOVE_IF failed %s [%s]\n", dev_name, strerror(errno));

    close(fd);
}

static void check_device_frontend(void)
{
    sprintf(dev_name, "/dev/dvb/adapter%d/frontend%d", adapter, device);

    int fd = open(dev_name, O_RDONLY | O_NONBLOCK);
    if(!fd)
    {
        printf("ERROR: open() failed %s [%s]\n", dev_name, strerror(errno));
        return;
    }

    struct dvb_frontend_info feinfo;
    if(ioctl(fd, FE_GET_INFO, &feinfo) < 0)
    {
        printf("ERROR: FE_GET_INFO failed %s [%s]\n", dev_name, strerror(errno));
        close(fd);
        return;
    }

    printf("    frontend:%s\n", feinfo.name);

    close(fd);
}

static void check_device(const char *item)
{
    device = get_last_int(&item[(sizeof("/dev/dvb/adapter") - 1) + (sizeof("/net") - 1)]);

    printf("adapter:%d device:%d\n", adapter, device);
    check_device_frontend();
    check_device_net();
    printf("\n");
}

static void check_adapter(const char *item)
{
    adapter = get_last_int(&item[sizeof("/dev/dvb/adapter") - 1]);
    iterate_dir(item, "net", check_device);
}

int main(int argc, char const *argv[])
{
    printf("dvbls v.1\n\n");
    iterate_dir("/dev/dvb", "adapter", check_adapter);
    return 0;
}
