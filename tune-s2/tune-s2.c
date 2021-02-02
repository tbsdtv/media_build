/* tune-s2 -- simple zapping tool for the Linux DVB S2 API
 *
 * UDL (updatelee@gmail.com)
 * Derived from work by:
 * 	Igor M. Liplianin (liplianin@me.by)
 * 	Alex Betis <alex.betis@gmail.com>
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

#include "tune-s2.h"

#if DVB_API_VERSION < 5 || DVB_API_VERSION_MINOR < 2
#error szap-s2 requires Linux DVB driver API version 5.2 and newer!
#endif

#ifndef DTV_STREAM_ID
	#define DTV_STREAM_ID DTV_ISDBS_TS_ID
#endif

#ifndef NO_STREAM_ID_FILTER
	#define NO_STREAM_ID_FILTER	(~0U)
#endif

int name2value(char *name, struct options *table)
{
	while (table->name) {
		if (!strcmp(table->name, name))
			return table->value;
		table++;
	}
}

char * value2name(int value, struct options *table)
{
	while (table->name) {
		if (table->value == value)
			return table->name;
		table++;
	}
}

int check_frontend (int frontend_fd)
{
	fe_status_t status;
	uint32_t ber;
	unsigned int ber_scale;
	float snr;
	unsigned int snr_scale;
	float lvl;
	unsigned int lvl_scale;

	if (ioctl(frontend_fd, FE_READ_STATUS, &status) == -1) {
		perror("FE_READ_STATUS failed"); 
	}

	struct dtv_property p[3];
	p[0].cmd = DTV_STAT_SIGNAL_STRENGTH;
	p[1].cmd = DTV_STAT_CNR;
	p[2].cmd = DTV_STAT_POST_ERROR_BIT_COUNT;

	struct dtv_properties p_status;
	p_status.num = 3;
	p_status.props = p;

	if (ioctl(frontend_fd, FE_GET_PROPERTY, &p_status) == -1) {
		perror("FE_GET_PROPERTY failed");
	}

	lvl_scale = p_status.props[0].u.st.stat[0].scale;
	if (lvl_scale == FE_SCALE_DECIBEL) {
		lvl = p_status.props[0].u.st.stat[0].svalue * 0.001;
	} else {
		int lvl2;
		if (ioctl(frontend_fd, FE_READ_SIGNAL_STRENGTH, &lvl2) == -1) {
			lvl = 0;
		} else {
			lvl = (float)lvl2 * 100 / 0xffff;
			if (lvl < 0) {
				lvl = 0;
			}
		}
	}
	snr_scale = p_status.props[1].u.st.stat[0].scale;
	if (snr_scale == FE_SCALE_DECIBEL) {
		snr = p_status.props[1].u.st.stat[0].svalue * .001;
	} else {
		unsigned int snr2 = 0;
		if (ioctl(frontend_fd, FE_READ_SNR, &snr2) == -1) {
			snr2 = 0;
		}
		snr = (float)snr2/10;
	}
	ber_scale = p_status.props[2].u.st.stat[0].scale;
	if (ber_scale == FE_SCALE_COUNTER) {
		ber = p_status.props[2].u.st.stat[0].uvalue;
	} else {
		ber = 0;
		if (ioctl(frontend_fd, FE_READ_BER, &ber) == -1) {
			ber = 0;
		}
	}
	printf ("status %s | signal %2.1f %s | snr %2.1f dB | ber %u | ",
		(status & FE_HAS_LOCK) ? "Locked" : "Unlocked", lvl, (lvl_scale == FE_SCALE_DECIBEL) ? "dBm" : "%", snr, ber);
	if (status & FE_HAS_LOCK) {
		printf("FE_HAS_LOCK \n");
 	} else printf("\n");

	return 0;
}

int tune(int frontend_fd, struct tune_p *t)
{
	struct dtv_property p_clear[] = { 
		{ .cmd = DTV_CLEAR }
	};

	struct dtv_properties cmdseq_clear = {
		.num = 1,
		.props = p_clear
	};

	if ((ioctl(frontend_fd, FE_SET_PROPERTY, &cmdseq_clear)) == -1) {
		perror("FE_SET_PROPERTY DTV_CLEAR failed");
		return -1;
	}
	usleep (20000);

	// discard stale QPSK events
	struct dvb_frontend_event ev;
	while (1) {
		if (ioctl(frontend_fd, FE_GET_EVENT, &ev) == -1)
			break;
	}

	struct dtv_property p_tune[] = {
		{ .cmd = DTV_DELIVERY_SYSTEM,	.u.data = t->system },
		{ .cmd = DTV_FREQUENCY,			.u.data = t->freq * 1000 },
		{ .cmd = DTV_VOLTAGE,			.u.data = t->voltage },
		{ .cmd = DTV_SYMBOL_RATE,		.u.data = t->sr * 1000},
		{ .cmd = DTV_TONE,				.u.data = t->tone },
		{ .cmd = DTV_MODULATION,		.u.data = t->modulation },
		{ .cmd = DTV_INNER_FEC,			.u.data = t->fec },
		{ .cmd = DTV_INVERSION,			.u.data = t->inversion },
		{ .cmd = DTV_ROLLOFF,			.u.data = t->rolloff },
		{ .cmd = DTV_BANDWIDTH_HZ,		.u.data = 0 },
		{ .cmd = DTV_PILOT,			.u.data = t->pilot },
		{ .cmd = DTV_STREAM_ID,			.u.data = t->mis },
		{ .cmd = DTV_TUNE },
	};
	struct dtv_properties cmdseq_tune = {
		.num = 13,
		.props = p_tune
	};

	printf("\nTuning specs: \n");
	printf("System:     %s \n", value2name(p_tune[0].u.data, dvb_system));
	printf("Frequency:  %d %s %d \n", abs(p_tune[1].u.data/1000 + t->LO), value2name(p_tune[2].u.data, dvb_voltage) , p_tune[3].u.data / 1000);
	printf("22khz:      %s \n", value2name(p_tune[4].u.data, dvb_tone));
	printf("Modulation: %s \n", value2name(p_tune[5].u.data, dvb_modulation));
	printf("FEC:        %s \n", value2name(p_tune[6].u.data, dvb_fec));
	printf("Inversion:  %s \n", value2name(p_tune[7].u.data, dvb_inversion));
	printf("Rolloff:    %s \n", value2name(p_tune[8].u.data, dvb_rolloff));
	printf("Pilot:      %s \n", value2name(p_tune[10].u.data, dvb_pilot));
	printf("MIS:        %d \n\n", p_tune[11].u.data);

	if (ioctl(frontend_fd, FE_SET_PROPERTY, &cmdseq_tune) == -1) {
		perror("FE_SET_PROPERTY TUNE failed");
	}
	usleep (200000);

	// wait for zero status indicating start of tunning
	do {
		ioctl(frontend_fd, FE_GET_EVENT, &ev);
	}
	while(ev.status != 0);

	if (ioctl(frontend_fd, FE_GET_EVENT, &ev) == -1) {
		ev.status = 0;
	}

	int i;
	fe_status_t status;
	for ( i = 0; i < 20; i++)
	{
		if (ioctl(frontend_fd, FE_READ_STATUS, &status) == -1) {
			perror("FE_READ_STATUS failed");
		}

		if (status & FE_HAS_LOCK || status & FE_TIMEDOUT)
			break;
		else
			sleep(1);
	}

	struct dtv_property p[] = {
		{ .cmd = DTV_DELIVERY_SYSTEM },
		{ .cmd = DTV_FREQUENCY },
		{ .cmd = DTV_VOLTAGE },
		{ .cmd = DTV_SYMBOL_RATE },
		{ .cmd = DTV_TONE },
		{ .cmd = DTV_MODULATION },
		{ .cmd = DTV_INNER_FEC },
		{ .cmd = DTV_INVERSION },
		{ .cmd = DTV_ROLLOFF },
		{ .cmd = DTV_BANDWIDTH_HZ },
		{ .cmd = DTV_PILOT },
		{ .cmd = DTV_STREAM_ID }
	};

	struct dtv_properties p_status = {
		.num = 12,
		.props = p
	};

	// get the actual parameters from the driver for that channel
	if ((ioctl(frontend_fd, FE_GET_PROPERTY, &p_status)) == -1) {
		perror("FE_GET_PROPERTY failed");
		return -1;
	}

		int m,num,den;
  		double fec_result;
		double bw;
		double dr;
		if (p_status.props[5].u.data < 1) 
			m = 2;
		else if (p_status.props[5].u.data < 10)
			m = 3;
		else if (p_status.props[5].u.data < 11)
			m = 4;
		else
			m = 5;
		bw = (p_status.props[3].u.data) / 1000 * (1 + atof(value2name(p_status.props[8].u.data, dvb_rolloff))/100)/1000;
		sscanf (value2name(p_status.props[6].u.data, dvb_fec), "%d/%d", &num, &den);
		fec_result = ((float)num/den);
		if (p_status.props[0].u.data < 6)
			dr = ((p_status.props[3].u.data / 1000) * fec_result * 188/204 * m)/1000;
		else
			dr = ((p_status.props[3].u.data / 1000) * fec_result * 0.98 * m)/1000;
		printf("Tuned specs: \n");
		printf("System:     %s %d \n", value2name(p_status.props[0].u.data, dvb_system), p_status.props[0].u.data);
		printf("Frequency:  %d %s %d \n", abs(p_status.props[1].u.data/1000 + t->LO), value2name(p_status.props[2].u.data, dvb_voltage) , p_status.props[3].u.data / 1000);
		printf("22khz:      %s \n", value2name(p_status.props[4].u.data, dvb_tone));
		printf("Modulation: %s %d \n", value2name(p_status.props[5].u.data, dvb_modulation), p_status.props[5].u.data);
		printf("FEC:        %s %d \n", value2name(p_status.props[6].u.data, dvb_fec), p_status.props[6].u.data);
		printf("Inversion:  %s %d \n", value2name(p_status.props[7].u.data, dvb_inversion), p_status.props[7].u.data);
		printf("Rolloff:    %s %d \n", value2name(p_status.props[8].u.data, dvb_rolloff), p_status.props[8].u.data);
		if(p_status.props[10].u.data)
			printf("Pilot:      ON %d \n",p_status.props[10].u.data);
		else	printf("Pilot:      OFF %d \n",p_status.props[10].u.data);
		printf("MIS:        %d \n", p_status.props[11].u.data);
  		printf("Bandwidth:  %3.4f MHz \n", bw);
  		printf("Data Rate:  %3.4f Mbps \n", dr);

// create a numerical representation of modcode 
		uint32_t current_modcode;
		current_modcode=(p_status.props[0].u.data << 8) + (p_status.props[5].u.data << 4)  + (p_status.props[6].u.data);
//		printf("Modcode: %03x \n", current_modcode);
// cn failure table, data from http://www.satbroadcasts.com/news,81,Minimum_carrier_to_noise_ratio_values_CNR,_CN_for_DVB_S2_system.html
// and http://www.satbroadcasts.com/news,79,Minimum_carrier_to_noise_ratio_values_CNR,_CN_for_DVB_S_system.html

		typedef struct { float cn; uint32_t modcode; } cn_failure_t;
		cn_failure_t cn[30] =
		{
			{ 2.7,	0x501 }, // S1 qpsk 1/2
			{ 4.4,	0x502 }, // S1 qpsk 2/3
			{ 5.5,	0x503 }, // S1 qpsk 3/4
			{ 6.5,	0x505 }, // S1 qpsk 5/6
			{ 7.2,	0x507 }, // S1 qpsk 7/8
			{ 1.0,	0x601 }, // S2 qpsk 1/2
			{ 2.2,	0x60A }, // S2 qpsk 3/5
			{ 3.1,	0x602 }, // S2 qpsk 2/3
			{ 4.0,	0x603 }, // S2 qpsk 3/4
			{ 4.6,	0x604 }, // S2 qpsk 4/5
			{ 5.2,	0x605 }, // S2 qpsk 5/6
			{ 6.2,	0x608 }, // S2 qpsk 8/9
			{ 6.5,	0x60B }, // S2 qpsk 9/10
			{ 5.5,	0x69A }, // S2 8psk 3/5
			{ 6.6,	0x692 }, // S2 8psk 2/3
			{ 7.9,	0x693 }, // S2 8psk 3/4
			{ 9.45,	0x695 }, // S2 8psk 5/6
			{ 10.7,	0x698 }, // S2 8psk 8/9
			{ 11.0,	0x69B }, // S2 8psk 9/10
			{ 9.0,	0x6A2 }, // S2 16APSK 2/3
			{ 10.2,	0x6A3 }, // S2 16APSK 3/4
			{ 11.0,	0x6A4 }, // S2 16APSK 4/5
			{ 11.6,	0x6A5 }, // S2 16APSK 5/6
			{ 12.9,	0x6A8 }, // S2 16APSK 8/9
			{ 13.1,	0x6AB }, // S2 16APSk 9/10
			{ 12.6,	0x6B3 }, // S2 32APSK 3/4
			{ 13.6,	0x6B4 }, // S2 32APSK 4/5
			{ 14.3,	0x6B5 }, // S2 32APSK 5/6
			{ 15.7,	0x6B8 }, // S2 32APSK 8/9
			{ 16.1,	0x6BB }  // S2 32APSK 9/10
		};
// based on the current modcode (system, modulation, fec) print the cn failure point
		int j;
		for (j = 0; j < 30; j++)
		{
			if(current_modcode == cn[j].modcode) {
				printf("CN Failure: %2.1f dB \n\n", cn[j].cn);
		}
			else ;
		}
	char c;
	do
	{
		check_frontend(frontend_fd);
		if (t->quit != 1)
			c = getch();
		switch ( c ) {
			case 'e':
				motor_dir(frontend_fd, 0);
				break;
			case 'w':
				motor_dir(frontend_fd, 1);
				break;
			case 's':{
				int pmem = 0;
				printf ("enter a position no. to save: ");
				scanf("%d", &pmem);
				motor_gotox_save(frontend_fd, pmem);
				break; }
		}
	} while(c != 'q' && t->quit != 1);
	return 0;
}

char *usage =
	"\nusage: tune-s2 12224 V 20000 [options]\n"
	"	-adapter N     : use given adapter (default 0)\n"
	"	-frontend N    : use given frontend (default 0)\n"
	"	-2             : use 22khz tone\n"
	"	-committed N   : use DiSEqC COMMITTED switch position N (1-4)\n"
	"	-uncommitted N : use DiSEqC uncommitted switch position N (1-4)\n"
	"	-servo N       : servo delay in milliseconds (20-1000, default 20)\n"
	"	-gotox NN      : Drive Motor to Satellite Position NN (0-99)\n"
	"	-usals N.N     : orbital position\n"
	"	-long N.N      : site long\n"
	"	-lat N.N       : site lat\n"
	"	-lnb lnb-type  : STANDARD, UNIVERSAL, DBS, CBAND or \n"
	"	-system        : System DVB-S or DVB-S2\n"
	"	-modulation    : modulation BPSK QPSK 8PSK\n"
	"	-fec           : fec 1/2, 2/3, 3/4, 3/5, 4/5, 5/6, 6/7, 8/9, 9/10, AUTO\n"
	"	-rolloff       : rolloff 35=0.35 25=0.25 20=0.20 0=UNKNOWN\n"
	"	-inversion N   : spectral inversion (OFF / ON / AUTO [default])\n"
	"	-pilot N       : pilot (OFF / ON / AUTO [default])\n"
	"	-mis N         : MIS #\n"
	"	-quit          : quit after tuning, used to time lock aquisition"
	"	-help          : help\n";

int main(int argc, char *argv[])
{
	if (!argv[1] || strcmp(argv[1], "-help") == 0)
	{
		printf("%s", usage);
		return -1;
	}

	struct lnb_p lnb_DBS = { 11250, 0, 0 };
	struct lnb_p lnb_STANDARD = { 10750, 0, 0 };
	struct lnb_p lnb_10600 = { 10600, 0, 0 };
	struct lnb_p lnb_10745 = { 10745, 0, 0 };
	struct lnb_p lnb_UNIVERSAL = { 9750, 10600, 11700 };
	struct lnb_p lnb_CBAND = { -5150, 0, 0 };
	struct lnb_p lnb_IF = { 0, 0, 0 };
	struct lnb_p lnb = lnb_STANDARD;

	char frontend_devname[80];
	int adapter = 0, frontend = 0;
	int committed	= 0;
	int uncommitted	= 0;
	int servo = 20;
	int pmem = 0;

	double site_lat		= 0;
	double site_long	= 0;
	double sat_long		= 0;

	struct tune_p t;
	t.fec		= FEC_AUTO;
	t.system	= SYS_DVBS;
	t.modulation	= QPSK;
	t.rolloff	= ROLLOFF_AUTO;
	t.inversion	= INVERSION_AUTO;
	t.pilot		= PILOT_AUTO;
	t.tone		= SEC_TONE_OFF;
	t.freq		= strtoul(argv[1], NULL, 0);
	t.voltage	= name2value(argv[2], dvb_voltage);
	t.sr		= strtoul(argv[3], NULL, 0);
	t.mis		= -1;

	int a;
	for( a = 4; a < argc; a++ )
	{
		if ( !strcmp(argv[a], "-quit") )
			t.quit = 1;
		else
			t.quit = 0;

		if ( !strcmp(argv[a], "-adapter") )
			adapter = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-frontend") )
			frontend = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-2") )
			t.tone = SEC_TONE_ON;
		if ( !strcmp(argv[a], "-committed") )
			committed = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-uncommitted") )
			uncommitted = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-servo") )
			servo = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-gotox") )
			pmem = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-usals") )
			sat_long = strtod(argv[a+1], NULL);
		if ( !strcmp(argv[a], "-long") )
			site_long = strtod(argv[a+1], NULL);
		if ( !strcmp(argv[a], "-lat") )
			site_lat = strtod(argv[a+1], NULL);
		if ( !strcmp(argv[a], "-lnb") )
		{
			if (!strcmp(argv[a+1], "DBS"))		lnb = lnb_DBS;
			if (!strcmp(argv[a+1], "10600"))	lnb = lnb_10600;
			if (!strcmp(argv[a+1], "10745"))	lnb = lnb_10745;
			if (!strcmp(argv[a+1], "STANDARD"))	lnb = lnb_STANDARD;
			if (!strcmp(argv[a+1], "UNIVERSAL"))	lnb = lnb_UNIVERSAL;
			if (!strcmp(argv[a+1], "CBAND"))	lnb = lnb_CBAND;
			if (!strcmp(argv[a+1], "IF"))		lnb = lnb_IF;
		}
		if ( !strcmp(argv[a], "-fec") )
			t.fec = name2value(argv[a+1], dvb_fec);
		if ( !strcmp(argv[a], "-system") )
			t.system = name2value(argv[a+1], dvb_system);
		if ( !strcmp(argv[a], "-modulation") )
			t.modulation = name2value(argv[a+1], dvb_modulation);
		if ( !strcmp(argv[a], "-rolloff") )
			t.rolloff = name2value(argv[a+1], dvb_rolloff);
		if ( !strcmp(argv[a], "-inversion") )
			t.inversion = name2value(argv[a+1], dvb_inversion);
		if ( !strcmp(argv[a], "-pilot") )
			t.pilot = name2value(argv[a+1], dvb_pilot);
		if ( !strcmp(argv[a], "-mis") )
			t.mis = strtoul(argv[a+1], NULL, 0);
		if ( !strcmp(argv[a], "-help") )
		{
			printf("%s", usage);
			return -1;
		}
	}

	printf("LNB: low: %d high: %d switch: %d \n", lnb.low, lnb.high, lnb.threshold);

	snprintf(frontend_devname, 80, "/dev/dvb/adapter%d/frontend%d", adapter, frontend);
	printf("opening: %s\n", frontend_devname);
	int frontend_fd;
	if ((frontend_fd = open(frontend_devname, O_RDWR | O_NONBLOCK)) < 0)
	{
		printf("failed to open '%s': %d %m\n", frontend_devname, errno);
		return -1;
	}

        struct dvb_frontend_info info;
        if ((ioctl(frontend_fd, FE_GET_INFO, &info)) == -1) {
                perror("FE_GET_INFO failed\n");
                return -1;
        }
        printf("frontend: (%s) \nfmin %d MHz \nfmax %d MHz \nmin_sr %d Ksps\nmax_sr %d Ksps\n", info.name,
        info.type == 0 ? info.frequency_min / 1000: info.frequency_min / 1000000,
        info.type == 0 ? info.frequency_max / 1000: info.frequency_max / 1000000,
        info.type == 0 ? info.symbol_rate_min / 1000: info.symbol_rate_min /1000000,
        info.type == 0 ? info.symbol_rate_max / 1000: info.symbol_rate_max /1000000);

	if (lnb.threshold && lnb.high && t.freq > lnb.threshold)
	{
		printf("HIGH band\n");
		t.tone = SEC_TONE_ON;
		t.LO = lnb.high;
		t.freq = abs(t.freq - abs(t.LO));
	} else {
		printf("LOW band\n");
		t.LO = lnb.low;
		t.freq = abs(t.freq - abs(t.LO));;
	}
	if(servo >= 1000) {
		servo = 1000;
	} else if(servo <= 20) {
		servo = 20;
	}
	if (sat_long)
		motor_usals(frontend_fd, site_lat, site_long, sat_long);
	if (!t.tone || committed || uncommitted)
		setup_switch (frontend_fd, t.voltage, t.tone, committed, uncommitted, servo);

	if(pmem)
		motor_gotox(frontend_fd, pmem);
	tune(frontend_fd, &t);

	printf("Closing frontend ... \n");
	close (frontend_fd);
	return 0;
}
