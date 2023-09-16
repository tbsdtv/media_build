#!/usr/bin/perl
use strict;
use File::Find;
use Proc::ProcessTable;


my %depend = ();
my %depend2 = ();
my %rmlist = ();
my @nodep;
my @modlist;
my @allmodules;
my %reqmodules;
my %loaded = ();
my $i=0;

# Device debug parameters
#		Module name		   Debug option
my %debug = (	"tuner"			=> "tuner_debug=1",
		"dvb-core"		=> "cam_debug=1",
		"dvb-ttpci"		=> "debug=247",
		"b2c2-flexcop"		=> "debug=0x01",
		"b2c2-flexcop-usb"	=> "debug=0x01",
		"b2c2-flexcop-pci"	=> "debug=0x01",
		"dvb-usb"		=> "debug=0x33",
		"dvb-usb-gp8psk"	=> "debug=0x03",
		"dvb-usb-vp7045"	=> "debug=0x03",
		"dvb-usb-dtt200u"	=> "debug=0x03",
		"dvb-usb-dibusb-common"	=> "debug=0x03",
	    );

sub findprog($)
{
	foreach(split(/:/, $ENV{PATH}),qw(/sbin /usr/sbin /usr/local/sbin)) {
		return "$_/$_[0]" if(-x "$_/$_[0]");
	}
	die "Can't find needed utility '$_[0]'";
}

sub parse_dir {
	my $file = $File::Find::name;
	my $modinfo = findprog('modinfo');

	if (!($file =~ /[.]ko$/)) {
		return;
	}

	my $module = $file;
	$module =~ s|^[./]*(.*)[.]ko|\1|;

	open IN, "$modinfo $file|grep depends|cut -b 17-|";
	while (<IN>) {
		my $deps = $_;
		$deps =~ s/\n//;
		$deps =~ s/[,]/ /g;
		$deps = " $deps ";
		$depend{$module} = $deps;
		push @allmodules, $module;
		$i++;
	}
	close IN;
}

sub parse_loaded {
	open IN,  "/proc/modules";
	while (<IN>) {
		m/^([\w\d_-]+)/;
		$loaded{$1}=1;
	}
	close IN;
}

sub cleandep()
{
	my $dep;

	while ( my ($k, $v) = each(%depend) ) {
		my $arg=$v;
		my $arg2=" ";
		while (!($arg =~ m/^\s*$/)) {
			if ($arg =~ m/^ ([^ ]+) /) {
				my $val=$1;
				if (exists($depend{$val})) {
					$arg2="$arg2 $val ";
				} else {
					$reqmodules{$val}=1;
				}
			}
			$arg =~ s/^ [^ ]+//;
			$arg2 =~ s/\s\s+/ /;
		}
		$depend2 { $k } = $arg2;
	}

}

sub rmdep()
{
	my $dep;

	while ($dep=pop @nodep) {
		while ( my ($k, $v) = each(%depend2) ) {
			if ($v =~ m/\s($dep)\s/) {
				$v =~ s/\s${dep}\s/ /;
				$v =~ s/\s${dep}\s/ /;
				$v =~ s/\s${dep}\s/ /;
				$depend2 {$k} = $v;
			}
		}
	}
}

sub orderdep ()
{
	my $old;
	do {
		$old=$i;
		while ( my ($key, $value) = each(%depend2) ) {
			if ($value =~ m/^\s*$/) {
				push @nodep, $key;
				push @modlist, $key;
				$i=$i-1;
				delete $depend2 {$key};
			}
		}
		rmdep();
	} until ($old==$i);
	while ( my ($key, $value) = each(%depend2) ) {
		printf "ERROR: bad dependencies - $key ($value)\n";
	}
}

sub insmod ($)
{
	my $debug=shift;
	my $modprobe = findprog('modprobe');
	my $insmod = findprog('insmod');

	while ( my ($key, $value) = each(%reqmodules) ) {
		print ("$modprobe $key\n");
		system ("$modprobe $key");
	}

	foreach my $key (@modlist) {
		if ($debug) {
			my $dbg=$debug{$key};

			print "$insmod ./$key.ko $dbg\n";
			system "$insmod ./$key.ko $dbg\n";
		} else {
			print "$insmod ./$key.ko\n";
			system "$insmod ./$key.ko\n";
		}
	}
}

my @pulse;
my $try_pulseaudio = 1;

sub check_pulseaudio()
{
	my $t = new Proc::ProcessTable;
	foreach my $p ( @{$t->table} ) {
		push @pulse, $p->uid if ($p->cmndline =~m,/pulseaudio ,);
	}
	$try_pulseaudio = 0 if (!@pulse);

	print "Pulseaudio is running with UUID(s): @pulse\n";
}

sub unload_pulseaudio($)
{
	my $driver_name = shift;
	my $cur_module;

	return if (!$try_pulseaudio);

	check_pulseaudio() if (!@pulse);
	return if (!$try_pulseaudio);

	for my $pid (@pulse) {
		my $user = getpwuid($pid);

#		printf "LANG=C su $user -c \"pacmd list-sources\" |\n";
		open IN, "LANG=C su $user -c \"pacmd list-sources\" |";
		while (<IN>) {
			$cur_module = $1 if (/^\s*module:\s*(\d+)/);

			if (/^\s*alsa.driver_name\s*=\s*"(.*)"/) {
				if ($1 eq $driver_name) {
#					print "LANG=C su $user -c \"pactl unload-module $cur_module\"\n";
					system ("LANG=C su $user -c \"pactl unload-module $cur_module\"");
				}
				next;
			}

			# Special case: em28xx sometimes use a Vendor Class at
			# the same interface as the video node. Pulseaudio can't
			# get the driver name in this case
			if (/^\s*alsa.card_name\s*=\s*"Em28xx/) {
#				print "LANG=C su $user -c \"pactl unload-module $cur_module\"\n";
				system ("LANG=C su $user -c \"pactl unload-module $cur_module\"");
			}
		}
		close IN;

#		printf "LANG=C su $user -c \"pacmd list-sinks\" |\n";
		open IN, "LANG=C su $user -c \"pacmd list-sinks\" |" or return;
		while (<IN>) {
			$cur_module = $1 if (/^\s*module:\s*(\d+)/);
			if (/^\s*alsa.driver_name\s*=\s*"(.*)"/) {
				if ($1 eq $driver_name) {
#					print "LANG=C su $user -c \"pactl unload-module $1\"\n";
					system ("LANG=C su $user -c \"pactl unload-module $1\"");
				}
			}
		}
	}
	close IN;
}

sub rmmod(@)
{
	my $rmmod = findprog('rmmod');
	my @not;
	foreach (reverse @_) {
		s/-/_/g;
		if (exists ($loaded{$_})) {
			my $module = $_;
			print "$rmmod $module\n";
			unload_pulseaudio($module);
			unshift @not, $module if (system "$rmmod $module");
		}
	}
	return @not;
}

sub prepare_cmd()
{
	find(\&parse_dir, ".");
	printf "found $i modules\n";

	cleandep();
	orderdep();
}

# main
my $mode=shift;
if ($mode eq "load") {
		prepare_cmd;
		insmod(0);
} else {
	if ($mode eq "unload") {
		prepare_cmd;
		parse_loaded;
		my @notunloaded = rmmod(@modlist);
		@notunloaded = rmmod(@notunloaded) if (@notunloaded);
		if (@notunloaded) {
			print "Couldn't unload: ", join(' ', @notunloaded), "\n";
		}
	} elsif ($mode eq "reload") {
		prepare_cmd;
		parse_loaded;
		rmmod(@modlist);
		insmod(0);
	} elsif ($mode eq "debug") {
		prepare_cmd;
		parse_loaded;
		insmod(1);
	} elsif ($mode eq "check") {
		prepare_cmd;
		parse_loaded;
	} else {
		printf "Usage: $0 [load|unload|reload|debug|check]\n";
	}
}

