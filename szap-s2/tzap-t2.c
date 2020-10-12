/* tzap-t2 -- simple zapping tool for the Linux DVB S2 API
 *
 * Copyright (C) 2008 Igor M. Liplianin (liplianin@me.by)
 * Copyright (C) 2013 CrazyCat (crazycat69@narod.ru)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>
#include <sys/time.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/version.h>
#include "lnb.h"

#if DVB_API_VERSION < 5 || DVB_API_VERSION_MINOR < 2
#error szap-s2 requires Linux DVB driver API version 5.2 and newer!
#endif

#ifndef DTV_STREAM_ID
	#define DTV_STREAM_ID 42
#endif

#ifndef NO_STREAM_ID_FILTER
	#define NO_STREAM_ID_FILTER	(~0U)
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif

/* location of channel list file */
#define CHANNEL_FILE "channels.conf"

/* one line of the szap channel file has the following format:
 * ^name:frequency_KHz:inversion:bandwidth:coderate_hp:coderate_lp:modulation:transmission_mode:guard_interval:hierarchy:vpid:apid:service_id$
 * one line of the VDR channel file has the following format:
 * ^name:frequency_KHz:[bandwidth][delivery][modulation][coderate_hp][coderate_lp][transmission_mode][guard_interval][hierarchy]]plp_id]:vpid:apid:?:?:service_id:?:?:?$
 */


#define FRONTENDDEVICE "/dev/dvb/adapter%d/frontend%d"
#define DEMUXDEVICE "/dev/dvb/adapter%d/demux%d"
#define AUDIODEVICE "/dev/dvb/adapter%d/audio%d"

struct t_channel_parameter_map {
  int user_value;
  int driver_value;
  const char *user_string;
  };
/* --- Channel Parameter Maps From VDR---*/

static struct t_channel_parameter_map inversion_values[] = {
  {   0, INVERSION_OFF, "off" },
  {   1, INVERSION_ON,  "on" },
  { 999, INVERSION_AUTO },
  { -1 }
  };

static struct t_channel_parameter_map bw_values [] = {
	{ 6, BANDWIDTH_6_MHZ, "6MHz" },
	{ 7, BANDWIDTH_7_MHZ, "7MHz" },
	{ 8, BANDWIDTH_8_MHZ, "8MHz" },
	{ 5, BANDWIDTH_6_MHZ, "5MHz" },
	{ 10, BANDWIDTH_10_MHZ, "10MHz" },
	{ 999, BANDWIDTH_AUTO, "auto" },
	{ -1 }
  };

static struct t_channel_parameter_map coderate_values[] = {
  {   0, FEC_NONE, "none" },
  {  12, FEC_1_2,  "1/2" },
  {  23, FEC_2_3,  "2/3" },
  {  34, FEC_3_4,  "3/4" },
  {  35, FEC_3_5,  "3/5" },
  {  45, FEC_4_5,  "4/5" },
  {  56, FEC_5_6,  "5/6" },
  {  67, FEC_6_7,  "6/7" },
  {  78, FEC_7_8,  "7/8" },
  {  89, FEC_8_9,  "8/9" },
  { 910, FEC_9_10, "9/10" },
  { 999, FEC_AUTO, "auto" },
  { -1 }
  };

static struct t_channel_parameter_map modulation_values[] = {
 // {   0, NONE,    "none" },
  {   2, QPSK,    "QPSK" },
  {  10, VSB_8,    "VSB8" },
  {  11, VSB_16,   "VSB16" },
  {  16, QAM_16,   "QAM16" },
  {  64, QAM_64,   "QAM64" },
  { 128, QAM_128,  "QAM128" },
  { 256, QAM_256,  "QAM256" },
  { 999, QAM_16 },
  { -1 }
  };

static struct t_channel_parameter_map mode_values [] = {
	{ 2, TRANSMISSION_MODE_2K, "2k" },
	{ 8, TRANSMISSION_MODE_2K, "8k" },
	{ 999, TRANSMISSION_MODE_AUTO, "auto" },
	{ 4, TRANSMISSION_MODE_4K, "4k" },
	{ 1, TRANSMISSION_MODE_1K, "1k" },
	{ 16, TRANSMISSION_MODE_16K, "16k" },
	{ 32, TRANSMISSION_MODE_32K, "32k" },
	{ -1 }
  };

static struct t_channel_parameter_map guard_values [] = {
	{ 32, GUARD_INTERVAL_1_32, "1/32" },
	{ 16, GUARD_INTERVAL_1_16, "1/16" },
	{ 8, GUARD_INTERVAL_1_8, "1/8" },
	{ 4, GUARD_INTERVAL_1_4, "1/4" },
	{ 999, GUARD_INTERVAL_AUTO, "auto" },
	{ 1128, GUARD_INTERVAL_1_128, "1/128" },
	{ 19128, GUARD_INTERVAL_19_128, "19/128" },
	{ 19256, GUARD_INTERVAL_19_256, "19/256" },
	{ -1 }
  };

static struct t_channel_parameter_map hierarchy_values [] = {
	{ 0, HIERARCHY_NONE, "none" },
	{ 1, HIERARCHY_NONE, "1" },
	{ 2, HIERARCHY_NONE, "2" },
	{ 4, HIERARCHY_NONE, "4" },
	{ 999, HIERARCHY_NONE, "auto" },
	{ -1 }
  };

static struct t_channel_parameter_map system_values[] = {
  {   0, SYS_DVBT,  "DVB-T" },
  {   1, SYS_DVBT2, "DVB-T2" },
  { -1 }
  };

static int user_index(int value, const struct t_channel_parameter_map * map)
{
  const struct t_channel_parameter_map *umap = map;
  while (umap && umap->user_value != -1) {
        if (umap->user_value == value)
           return umap - map;
        umap++;
        }
  return -1;
};

static int driver_index(int value, const struct t_channel_parameter_map *map)
{
  const struct t_channel_parameter_map *umap = map;
  while (umap && umap->user_value != -1) {
        if (umap->driver_value == value)
           return umap - map;
        umap++;
        }
  return -1;
};

static int map_to_user(int value, const struct t_channel_parameter_map *map, char **string)
{
  int n = driver_index(value, map);
  if (n >= 0) {
     if (string)
        *string = (char *)map[n].user_string;
     return map[n].user_value;
     }
  return -1;
}

static int map_to_driver(int value, const struct t_channel_parameter_map *map)
{
  int n = user_index(value, map);
  if (n >= 0)
     return map[n].driver_value;
  return -1;
}

static struct lnb_types_st lnb_type;

static int exit_after_tuning;
static int interactive;

static char *usage_str =
    "\nusage: tzap-t2 -q\n"
    "         list known channels\n"
    "       tzap-t2 [options] {-n channel-number|channel_name}\n"
    "         zap to channel via number or full name (case insensitive)\n"
    "     -a number : use given adapter (default 0)\n"
    "     -f number : use given frontend (default 0)\n"
    "     -d number : use given demux (default 0)\n"
    "     -c file   : read channels list from 'file'\n"
    "     -V        : use VDR channels list file format (default zap)\n"
    "     -b        : enable Audio Bypass (default no)\n"
    "     -x        : exit after tuning\n"
    "     -H        : human readable output\n"
    "     -D        : params debug\n"
    "     -r        : set up /dev/dvb/adapterX/dvr0 for TS recording\n"
    "     -i        : run interactively, allowing you to type in channel names\n"
    "     -p        : add pat and pmt to TS recording (implies -r)\n"
    "                 or -n numbers for zapping\n"
    "     -t        : add teletext to TS recording (needs -V)\n"
    "     -S        : delivery system type DVB-T=0, DVB-T2=1\n"
    "     -M        : modulation 2=QPSK 16=16QAM 64=64QAM 256=256QAM\n"
    "     -C        : fec 0=NONE 12=1/2 23=2/3 34=3/4 35=3/5 45=4/5 56=5/6 67=6/7 89=8/9 910=9/10 999=AUTO\n"
    "     -m        : physical layer pipe 0-255\n";

static int set_demux(int dmxfd, int pid, int pes_type, int dvr)
{
	struct dmx_pes_filter_params pesfilter;

	if (pid < 0 || pid >= 0x1fff) /* ignore this pid to allow radio services */
		return TRUE;

	if (dvr) {
		int buffersize = 64 * 1024;
		if (ioctl(dmxfd, DMX_SET_BUFFER_SIZE, buffersize) == -1)
			perror("DMX_SET_BUFFER_SIZE failed");
	}

	pesfilter.pid = pid;
	pesfilter.input = DMX_IN_FRONTEND;
	pesfilter.output = dvr ? DMX_OUT_TS_TAP : DMX_OUT_DECODER;
	pesfilter.pes_type = pes_type;
	pesfilter.flags = DMX_IMMEDIATE_START;

	if (ioctl(dmxfd, DMX_SET_PES_FILTER, &pesfilter) == -1) {
		fprintf(stderr, "DMX_SET_PES_FILTER failed "
			"(PID = 0x%04x): %d %m\n", pid, errno);

		return FALSE;
	}

	return TRUE;
}

int get_pmt_pid(char *dmxdev, int sid)
{
	int patfd, count;
	int pmt_pid = 0;
	int patread = 0;
	int section_length;
	unsigned char buft[4096];
	unsigned char *buf = buft;
	struct dmx_sct_filter_params f;

	memset(&f, 0, sizeof(f));
	f.pid = 0;
	f.filter.filter[0] = 0x00;
	f.filter.mask[0] = 0xff;
	f.timeout = 0;
	f.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	if ((patfd = open(dmxdev, O_RDWR)) < 0) {
		perror("openening pat demux failed");
		return -1;
	}

	if (ioctl(patfd, DMX_SET_FILTER, &f) == -1) {
		perror("ioctl DMX_SET_FILTER failed");
		close(patfd);
		return -1;
	}

	while (!patread) {
		if (((count = read(patfd, buf, sizeof(buft))) < 0) && errno == EOVERFLOW)
			count = read(patfd, buf, sizeof(buft));
		if (count < 0) {
			perror("read_sections: read error");
			close(patfd);
			return -1;
		}

		section_length = ((buf[1] & 0x0f) << 8) | buf[2];
		if (count != section_length + 3)
			continue;

		buf += 8;
		section_length -= 8;

		patread = 1; /* assumes one section contains the whole pat */
		while (section_length > 0) {
			int service_id = (buf[0] << 8) | buf[1];
			if (service_id == sid) {
				pmt_pid = ((buf[2] & 0x1f) << 8) | buf[3];
				section_length = 0;
			}
			buf += 4;
			section_length -= 4;
		}
	}
	close(patfd);
	return pmt_pid;
}

static int do_tune(int fefd, unsigned int freq, unsigned int bw, enum fe_delivery_system delsys,
		   int modulation, int fec_hp, int fec_lp, int mode, int guard, int hierarchy, int stream_id)
{
	struct dvb_frontend_event ev;
	struct dtv_property p[] = {
		{ .cmd = DTV_DELIVERY_SYSTEM,	.u.data = delsys },
		{ .cmd = DTV_FREQUENCY,		.u.data = freq },
		{ .cmd = DTV_BANDWIDTH_HZ,	.u.data = bw },
		{ .cmd = DTV_MODULATION,	.u.data = modulation },
		{ .cmd = DTV_CODE_RATE_HP,	.u.data = fec_hp },
		{ .cmd = DTV_CODE_RATE_LP,	.u.data = fec_lp },
		{ .cmd = DTV_INVERSION,		.u.data = INVERSION_AUTO },
		{ .cmd = DTV_TRANSMISSION_MODE,	.u.data = mode },
		{ .cmd = DTV_GUARD_INTERVAL,	.u.data = guard },
		{ .cmd = DTV_HIERARCHY,		.u.data = hierarchy },
		{ .cmd = DTV_STREAM_ID,		.u.data = stream_id },
		{ .cmd = DTV_TUNE },
	};
	struct dtv_properties cmdseq = {
		.num = sizeof(p)/sizeof(p[0]),
		.props = p
	};

	/* discard stale QPSK events */
	while (1) {
		if (ioctl(fefd, FE_GET_EVENT, &ev) == -1)
		break;
	}

	if ((delsys != SYS_DVBT) && (delsys != SYS_DVBT2))
		return -EINVAL;

	if ((ioctl(fefd, FE_SET_PROPERTY, &cmdseq)) == -1) {
		perror("FE_SET_PROPERTY failed");
		return FALSE;
	}

	return TRUE;
}


static
int check_frontend (int fe_fd, int dvr, int human_readable, int params_debug)
{
	(void)dvr;
	fe_status_t status;
	uint16_t snr, signal;
	uint32_t ber, uncorrected_blocks;
	int timeout = 0;
	char *field;
	struct dtv_property p[] = {
		{ .cmd = DTV_FREQUENCY },
		{ .cmd = DTV_BANDWIDTH_HZ },
		{ .cmd = DTV_DELIVERY_SYSTEM },
		{ .cmd = DTV_MODULATION },
		{ .cmd = DTV_CODE_RATE_HP },
		{ .cmd = DTV_CODE_RATE_LP },
		{ .cmd = DTV_TRANSMISSION_MODE },
		{ .cmd = DTV_GUARD_INTERVAL },
		{ .cmd = DTV_HIERARCHY },
	};
	struct dtv_properties cmdseq = {
		.num = sizeof(p)/sizeof(p[0]),
		.props = p
	};

	do {
		if (ioctl(fe_fd, FE_READ_STATUS, &status) == -1)
			perror("FE_READ_STATUS failed");
		/* some frontends might not support all these ioctls, thus we
		 * avoid printing errors
		 */
		if (ioctl(fe_fd, FE_READ_SIGNAL_STRENGTH, &signal) == -1)
			signal = -2;
		if (ioctl(fe_fd, FE_READ_SNR, &snr) == -1)
			snr = -2;
		if (ioctl(fe_fd, FE_READ_BER, &ber) == -1)
			ber = -2;
		if (ioctl(fe_fd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks) == -1)
			uncorrected_blocks = -2;

		if (human_readable) {
			printf ("status %02x | signal %3u%% | snr %3u%% | ber %d | unc %d | ",
				status, (signal * 100) / 0xffff, (snr * 100) / 0xffff, ber, uncorrected_blocks);
		} else {
			printf ("status %02x | signal %04x | snr %04x | ber %08x | unc %08x | ",
				status, signal, snr, ber, uncorrected_blocks);
		}
		if (status & FE_HAS_LOCK)
			printf("FE_HAS_LOCK");
		printf("\n");

		if (exit_after_tuning && ((status & FE_HAS_LOCK) || (++timeout >= 10)))
			break;

		usleep(1000000);
	} while (1);

	if ((status & FE_HAS_LOCK) == 0)
		return 0;

	if ((ioctl(fe_fd, FE_GET_PROPERTY, &cmdseq)) == -1) {
		perror("FE_GET_PROPERTY failed");
		return 0;
	}
	/* printout found parameters here */
	if (params_debug){
		printf("frequency %d, ", p[0].u.data);
		printf("bandwidth %d, ", p[1].u.data);
		printf("delivery %d, ", p[2].u.data);
		printf("modulation %d, ", p[3].u.data);
		printf("coderate hp %d, ", p[4].u.data);
		printf("coderate lp %d, ", p[5].u.data);
		printf("mode %d, ", p[6].u.data);
		printf("guard %d, ", p[7].u.data);
		printf("hierarchy %d", p[8].u.data);
	} else {
		printf("frequency %d, ",p[0].u.data);
		field = NULL;
		map_to_user(p[1].u.data, bw_values, &field);
		printf("bandwidth %s, ", field);
		field = NULL;
		map_to_user(p[2].u.data, system_values, &field);
		printf("delivery %s, ", field);
		field = NULL;
		map_to_user(p[3].u.data, modulation_values, &field);
		printf("modulation %s, ", field);
		field = NULL;
		map_to_user(p[4].u.data, coderate_values, &field);
		printf("coderate-hp %s, ", field);
		field = NULL;
		map_to_user(p[5].u.data, coderate_values, &field);
		printf("coderate-lp %s, ", field);
		field = NULL;
		map_to_user(p[6].u.data, mode_values, &field);
		printf("mode %s, ", field);
		field = NULL;
		map_to_user(p[7].u.data, guard_values, &field);
		printf("guard %s, ", field);
		field = NULL;
		map_to_user(p[8].u.data, hierarchy_values, &field);
		printf("hierarchy %s", field);
	}

	printf("\n");

	return 0;
}

static
int zap_to(unsigned int adapter, unsigned int frontend, unsigned int demux,
	   unsigned int freq, unsigned int bw,
	   unsigned int vpid, unsigned int apid,
	   unsigned int tpid, int sid,
	   int dvr, int rec_psi, int bypass, unsigned int delivery,
	   int modulation, int fec_hp, int fec_lp, int mode, int guard, int hierarchy, int stream_id, int human_readable,
	   int params_debug)
{
	struct dtv_property p[] = {
		{ .cmd = DTV_CLEAR },
	};

	struct dtv_properties cmdseq = {
		.num = sizeof(p)/sizeof(p[0]),
		.props = p
	};

	char fedev[128], dmxdev[128], auddev[128];
	static int fefd, dmxfda, dmxfdv, dmxfdt = -1, audiofd = -1, patfd, pmtfd;
	int pmtpid;
	int result;

	if (!fefd) {
		snprintf(fedev, sizeof(fedev), FRONTENDDEVICE, adapter, frontend);
		snprintf(dmxdev, sizeof(dmxdev), DEMUXDEVICE, adapter, demux);
		snprintf(auddev, sizeof(auddev), AUDIODEVICE, adapter, demux);
		printf("using '%s' and '%s'\n", fedev, dmxdev);

		if ((fefd = open(fedev, O_RDWR | O_NONBLOCK)) < 0) {
			perror("opening frontend failed");
			return FALSE;
		}
		
		if ((dmxfdv = open(dmxdev, O_RDWR)) < 0) {
			perror("opening video demux failed");
			close(fefd);
			return FALSE;
		}

		if ((dmxfda = open(dmxdev, O_RDWR)) < 0) {
			perror("opening audio demux failed");
			close(fefd);
			return FALSE;
		}

		if ((dmxfdt = open(dmxdev, O_RDWR)) < 0) {
			perror("opening teletext demux failed");
			close(fefd);
			return FALSE;
		}

		if (dvr == 0)	/* DMX_OUT_DECODER */
			audiofd = open(auddev, O_RDWR);

		if (rec_psi){
			if ((patfd = open(dmxdev, O_RDWR)) < 0) {
				perror("opening pat demux failed");
				close(audiofd);
				close(dmxfda);
				close(dmxfdv);
				close(dmxfdt);
				close(fefd);
				return FALSE;
			}

			if ((pmtfd = open(dmxdev, O_RDWR)) < 0) {
				perror("opening pmt demux failed");
				close(patfd);
				close(audiofd);
				close(dmxfda);
				close(dmxfdv);
				close(dmxfdt);
				close(fefd);
				return FALSE;
			}
		}
	}


	result = FALSE;

	if ((ioctl(fefd, FE_SET_PROPERTY, &cmdseq)) == -1) {
		perror("FE_SET_PROPERTY DTV_CLEAR failed");
		return FALSE;
	}

	switch(bw) 
	{
		case BANDWIDTH_5_MHZ:	bw = 5000000; break;
		case BANDWIDTH_6_MHZ:	bw = 6000000; break;
		case BANDWIDTH_7_MHZ:	bw = 7000000; break;
		case BANDWIDTH_8_MHZ:	bw = 8000000; break;
		case BANDWIDTH_10_MHZ:	bw = 10000000; break;
		case BANDWIDTH_AUTO:
		default:		bw = 0;
	}

	if (do_tune(fefd, freq, bw, delivery, modulation, fec_hp, fec_lp, mode, guard, hierarchy, stream_id))
			if (set_demux(dmxfdv, vpid, DMX_PES_VIDEO, dvr))
				if (audiofd >= 0)
					(void)ioctl(audiofd, AUDIO_SET_BYPASS_MODE, bypass);
	if (set_demux(dmxfda, apid, DMX_PES_AUDIO, dvr)) {
		if (rec_psi) {
			pmtpid = get_pmt_pid(dmxdev, sid);
			if (pmtpid < 0) {
				result = FALSE;
			}
			if (pmtpid == 0) {
				fprintf(stderr,"couldn't find pmt-pid for sid %04x\n",sid);
				result = FALSE;
			}
			if (set_demux(patfd, 0, DMX_PES_OTHER, dvr))
				if (set_demux(pmtfd, pmtpid, DMX_PES_OTHER, dvr))
					result = TRUE;
		} else {
			result = TRUE;
		}
	}

	if (tpid != -1 && !set_demux(dmxfdt, tpid, DMX_PES_TELETEXT, dvr)) {
		fprintf(stderr, "set_demux DMX_PES_TELETEXT failed\n");
	}

	check_frontend (fefd, dvr, human_readable, params_debug);

	if (!interactive) {
		close(patfd);
		close(pmtfd);
		if (audiofd >= 0)
			close(audiofd);
		close(dmxfda);
		close(dmxfdv);
		close(dmxfdt);
		close(fefd);
	}

	return result;
}
static char *parse_parameter(const char *s, int *value, const struct t_channel_parameter_map *map)
{
	if (*++s) {
		char *p = NULL;
		errno = 0;
		int n = strtol(s, &p, 10);
		if (!errno && p != s) {
			value[0] = map_to_driver(n, map);
			if (value[0] >= 0)
				return p;
                }
        }
        fprintf(stderr, "ERROR: invalid value for parameter '%C'\n", *(s - 1));
        return NULL;
}

static int read_channels(const char *filename, int list_channels,
			uint32_t chan_no, const char *chan_name,
			unsigned int adapter, unsigned int frontend,
			unsigned int demux, int dvr, int rec_psi,
			int bypass, unsigned int delsys,
			int modulation, int fec_hp, int fec_lp, int mode, int guard, int hierarchy, int stream_id, int human_readable,
			int params_debug, int use_vdr_format, int use_tpid)
{
	FILE *cfp;
	char buf[4096];
	char inp[256];
	char *field, *tmp, *p;
	unsigned int line;
	unsigned int freq = 0, bw = 0, vpid, apid, tpid, sid;
	int ret;
	int trash;
again:
	line = 0;
	if (!(cfp = fopen(filename, "r"))) {
		fprintf(stderr, "error opening channel list '%s': %d %m\n",
			filename, errno);
		return FALSE;
	}

	if (interactive) {
		fprintf(stderr, "\n>>> ");
		if (!fgets(inp, sizeof(inp), stdin)) {
			printf("\n");
			return -1;
		}
		if (inp[0] == '-' && inp[1] == 'n') {
			chan_no = strtoul(inp+2, NULL, 0);
			chan_name = NULL;
			if (!chan_no) {
				fprintf(stderr, "bad channel number\n");
				goto again;
			}
		} else {
			p = strchr(inp, '\n');
			if (p)
			*p = '\0';
			chan_name = inp;
			chan_no = 0;
		}
	}

	while (!feof(cfp)) {
		if (fgets(buf, sizeof(buf), cfp)) {
			line++;

		if (chan_no && chan_no != line)
			continue;

		tmp = buf;
		field = strsep(&tmp, ":");

		if (!field)
			goto syntax_err;

		if (list_channels) {
			printf("%03u %s\n", line, field);
			continue;
		}

		if (chan_name && strcasecmp(chan_name, field) != 0)
			continue;

		printf("zapping to %d '%s':\n", line, field);

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		freq = strtoul(field, NULL, 0);

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		while (field && *field) {
			switch (toupper(*field)) {
			case 'B':
				field = parse_parameter(field, &bw, bw_values);
				break;
			case 'M':
				if (modulation == -1)
					field = parse_parameter(field, &modulation, modulation_values);
				else
					field = parse_parameter(field, &trash, modulation_values);
				break;
			case 'I':/* ignore */
				field = parse_parameter(field, &ret, inversion_values);
				break;
			case 'C':
				if (fec_hp == -1)
					field = parse_parameter(field, &fec_hp, coderate_values);
				else
					field = parse_parameter(field, &trash, coderate_values);
				break;
			case 'D':
				if (fec_lp == -1)
					field = parse_parameter(field, &fec_lp, coderate_values);
				else
					field = parse_parameter(field, &trash, coderate_values);
				break;
			case 'T':
				if (mode == -1)
					field = parse_parameter(field, &mode, mode_values);
				else
					field = parse_parameter(field, &trash, mode_values);
				break;
			case 'G':
				if (guard == -1)
					field = parse_parameter(field, &guard, guard_values);
				else
					field = parse_parameter(field, &trash, guard_values);
				break;
			case 'Y':
				if (hierarchy == -1)
					field = parse_parameter(field, &hierarchy, hierarchy_values);
				else
					field = parse_parameter(field, &trash, hierarchy_values);
				break;
			case 'P':
				stream_id = strtol(++field, &field, 10);
				break;
			case 'S':
				if (delsys == -1)
					field = parse_parameter(field, &delsys, system_values);
				else
					field = parse_parameter(field, &trash, system_values);
				break;
			default:
				goto syntax_err;
			}
		}
		/* default values for empty parameters */
		if (modulation == -1)
			modulation = QAM_16;

		if (delsys == -1)
			delsys = SYS_DVBT;

		if (fec_hp == -1)
			fec_hp = FEC_AUTO;

		if (fec_lp == -1)
			fec_lp = FEC_AUTO;

		if (mode == -1)
			mode = TRANSMISSION_MODE_AUTO;

		if (guard == -1)
			guard = GUARD_INTERVAL_AUTO;

		if (hierarchy == -1)
			hierarchy = HIERARCHY_NONE;

		if (stream_id<0 || stream_id>255)
			stream_id = NO_STREAM_ID_FILTER;

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		vpid = strtoul(field, NULL, 0);
		if (!vpid)
			vpid = 0x1fff;

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		p = strchr(field, ';');

		if (p) {
			*p = '\0';
			p++;
			if (bypass) {
				if (!p || !*p)
					goto syntax_err;
				field = p;
			}
		}

		apid = strtoul(field, NULL, 0);
		if (!apid)
			apid = 0x1fff;

		tpid = -1;
		if (use_vdr_format) {
			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;

			if (use_tpid)
				tpid = strtoul(field, NULL, 0);

			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;

			strtoul(field, NULL, 0);
		}

		if (!(field = strsep(&tmp, ":")))
			goto syntax_err;

		sid = strtoul(field, NULL, 0);

		fclose(cfp);

		field = NULL;
		map_to_user(bw, bw_values, &field);
		printf("frequency %u KHz, bandwidth %s, ",
			freq, field);

		if (params_debug){
			printf("delivery %d, ", delsys);
		} else {
			field = NULL;
			map_to_user(delsys, system_values, &field);
			printf("delivery %s, ", field);
		}

		if (params_debug){
			printf("modulation %d\n", modulation);	
		} else {
			field = NULL;
			map_to_user(modulation, modulation_values, &field);
			printf("modulation %s\n", field);
		}

		if (params_debug){
			printf("coderate-hp %d, ", fec_hp);
		} else {
			field = NULL;
			map_to_user(fec_hp, coderate_values, &field);
			printf("coderate-hp %s, ", field);
		}

		if (params_debug){
			printf("coderate-lp %d, ", fec_hp);
		} else {
			field = NULL;
			map_to_user(fec_lp, coderate_values, &field);
			printf("coderate-lp %s, ", field);
		}

		if (params_debug){
			printf("mode %d, ", mode);
		} else {
			field = NULL;
			map_to_user(mode, mode_values, &field);
			printf("mode %s, ", field);
		}

		if (params_debug){
			printf("guard %d, ", mode);
		} else {
			field = NULL;
			map_to_user(guard, guard_values, &field);
			printf("guard %s, ", field);
		}

		if (params_debug){
			printf("hierarchy %d, ", mode);
		} else {
			field = NULL;
			map_to_user(hierarchy, hierarchy_values, &field);
			printf("hierarchy %s, ", field);
		}

		printf("plp_id %d\n", stream_id);

		printf("vpid 0x%04x, apid 0x%04x, sid 0x%04x\n", vpid, apid, sid);

		ret = zap_to(adapter, frontend, demux, freq * 1000, bw,
				vpid, apid, tpid, sid, dvr, rec_psi, bypass,
				delsys, modulation, fec_hp, fec_lp, mode, guard, hierarchy, stream_id, human_readable,
				params_debug);

		if (interactive)
			goto again;

		if (ret)
			return TRUE;

		return FALSE;

syntax_err:
		fprintf(stderr, "syntax error in line %u: '%s'\n", line, buf);
	} else if (ferror(cfp)) {
		fprintf(stderr, "error reading channel list '%s': %d %m\n",
		filename, errno);
		fclose(cfp);
		return FALSE;
	} else
		break;
	}

	fclose(cfp);

	if (!list_channels) {
		fprintf(stderr, "channel not found\n");

	if (!interactive)
		return FALSE;
	}
	if (interactive)
		goto again;

	return TRUE;
}

static void handle_sigint(int sig)
{
	fprintf(stderr, "Interrupted by SIGINT!\n");
	exit(2);
}

void
bad_usage(char *pname)
{
	fprintf (stderr, usage_str, pname);
}

int main(int argc, char *argv[])
{
	const char *home;
	char chanfile[2 * PATH_MAX];
	int list_channels = 0;
	unsigned int chan_no = 0;
	const char *chan_name = NULL;
	unsigned int adapter = 0, frontend = 0, demux = 0, dvr = 0, rec_psi = 0;
	int bypass = 0;
	int opt, copt = 0;
	int human_readable = 0;
	int params_debug = 0;
	int use_vdr_format = 0;
	int use_tpid = 0;


	int delsys	= -1;
	int modulation	= -1;
	int fec		= -1;
	int stream_id	= NO_STREAM_ID_FILTER;
	
	while ((opt = getopt(argc, argv, "M:m:C:O:HDVhqrpn:a:f:d:S:c:l:xib")) != -1) {
		switch (opt) {
		case '?':
		case 'h':
		default:
			bad_usage(argv[0]);
			break;
		case 'C':
			parse_parameter(--optarg, &fec, coderate_values);
			break;
		case 'M':
			parse_parameter(--optarg, &modulation, modulation_values);
			break;
		case 'm':
			stream_id = strtol(optarg, NULL, 0);
			break;
		case 'S':
			parse_parameter(--optarg, &delsys, system_values);
			break;
		case 'b':
			bypass = 1;
			break;
		case 'q':
			list_channels = 1;
			break;
		case 'r':
			dvr = 1;
			break;
		case 'n':
			chan_no = strtoul(optarg, NULL, 0);
			break;
		case 'a':
			adapter = strtoul(optarg, NULL, 0);
			break;
		case 'f':
			frontend = strtoul(optarg, NULL, 0);
			break;
		case 'p':
			rec_psi = 1;
			break;
		case 'd':
			demux = strtoul(optarg, NULL, 0);
			break;
		case 'c':
			copt = 1;
			strncpy(chanfile, optarg, sizeof(chanfile));
			break;
		case 'x':
			exit_after_tuning = 1;
			break;
		case 'H':
			human_readable = 1;
			break;
		case 'D':
			params_debug = 1;
			break;
		case 'V':
			use_vdr_format = 1;
			break;
		case 't':
			use_tpid = 1;
			break;
		case 'i':
			interactive = 1;
			exit_after_tuning = 1;
		}
	}
	if (optind < argc)
		chan_name = argv[optind];
	if (chan_name && chan_no) {
		bad_usage(argv[0]);
		return -1;
	}
	if (list_channels && (chan_name || chan_no)) {
		bad_usage(argv[0]);
		return -1;
	}
	if (!list_channels && !chan_name && !chan_no && !interactive) {
		bad_usage(argv[0]);
		return -1;
	}

	if (!copt) {
		if (!(home = getenv("HOME"))) {
			fprintf(stderr, "error: $HOME not set\n");
		return TRUE;
	}
	snprintf(chanfile, sizeof(chanfile),
		"%s/.tzap/%i/%s", home, adapter, CHANNEL_FILE);
	if (access(chanfile, R_OK))
		snprintf(chanfile, sizeof(chanfile),
			 "%s/.tzap/%s", home, CHANNEL_FILE);
	}

	printf("reading channels from file '%s'\n", chanfile);

	if (rec_psi)
		dvr=1;

	signal(SIGINT, handle_sigint);

	if (!read_channels(chanfile, list_channels, chan_no, chan_name,
	    adapter, frontend, demux, dvr, rec_psi, bypass, delsys,
	    modulation, fec, -1, -1, -1, -1, stream_id, human_readable, params_debug,
	    use_vdr_format, use_tpid))

		return TRUE;

	return FALSE;
}
