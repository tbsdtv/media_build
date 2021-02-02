# README #

 tune-s2

tune-s2 is a small linux app I've wrote to be able to tune my dvb devices, all I wanted it todo is control my motor, 
diseqc and 22khz switch's and tune a signal. From there it waits for the user to press the 'q' button to quit the app. 
This allows me to use other apps like my demux app or other apps like dvbsnoop and dvbtraffic.

clone repo: 

git clone https://bitbucket.org/updatelee/tune-s2.git

compile:

make

install it:

sudo make install

./tune-s2 -help

usage: tune-s2 12224 V 20000 [options]

        -adapter N     : use given adapter (default 0)

        -frontend N    : use given frontend (default 0)

        -2             : use 22khz tone

        -committed N   : use DiSEqC COMMITTED switch position N (1-4)

        -uncommitted N : use DiSEqC uncommitted switch position N (1-4)

        -servo N       : servo delay in milliseconds (20-1000, default 20)

        -gotox NN      : Drive Motor to Satellite Position NN (0-99)

        -usals N.N     : orbital position

        -long N.N      : site long

        -lat N.N       : site lat

        -lnb lnb-type  : STANDARD, UNIVERSAL, DBS, CBAND or 

        -system        : System DVB-S or DVB-S2

        -modulation    : modulation BPSK QPSK 8PSK

        -fec           : fec 1/2, 2/3, 3/4, 3/5, 4/5, 5/6, 6/7, 8/9, 9/10, AUTO

        -rolloff       : rolloff 35=0.35 25=0.25 20=0.20 0=UNKNOWN

        -inversion N   : spectral inversion (OFF / ON / AUTO [default])

        -pilot N           : pilot (OFF / ON / AUTO [default])

        -mis N             : MIS #

        -help          : help