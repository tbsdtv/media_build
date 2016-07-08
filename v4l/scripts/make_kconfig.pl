#!/usr/bin/perl
use FileHandle;
use strict;

my %depend = ();
my %minver = ();
my %config = ();
my %stringopt = ();
my %intopt = ();
my %hexopt = ();
my %tristate = ();
my %kernopts = ();
my %depmods = ();
my ($version, $level, $sublevel, $kernver);

my $kernel = shift;
my $kernsrc = shift;
my $force_kconfig = shift;
my $enable_staging = shift;

my $debug=0;

###########################################################
# Read a .config file (first argument) and put the resulting options in a
# hash, which is returned.  This subroutine should be called in a hash
# context or it will not be very useful.
sub process_config($)
{
	my $filename = shift;
	my %conf;
	my $in = new FileHandle;

	open $in, $filename or die "File not found: $filename";
	while (<$in>) {
		if (/^CONFIG_(\w+)\s*=\s*(\S.*?)\s*$/) {
			my $c = \($conf{$1} = $2);
			$$c =~ y/nmy/012/ if $$c =~ /^.$/;
			next;
		}
		if (/^# CONFIG_(\w+) is not set$/) {
			$conf{$1} = 0;
			next;
		}
		unless (/^#|$/) {
			print "Confused by this line in $filename:\n$_";
		}
	}
	close $in;
	return %conf;
}

sub add_bool($)
{
	my $arg=shift;

	exists $config{$arg} or die "Adding unknown boolean '$arg'";
	print "Boolean: $arg\n" if $debug;
	$tristate{$arg} = "bool";
}

sub add_tristate($)
{
	my $arg=shift;

	exists $config{$arg} or die "Adding unknown tristate '$arg'";
	print "Tristate: $arg\n" if $debug;
	$tristate{$arg}="tristate";
	# tri default is 'm'
	$config{$arg} = 1 if($config{$arg});
}

sub add_int($)
{
	my $arg=shift;

	print "Int: $arg\n" if $debug;
	exists $config{$arg} or die "Adding unknown int '$arg'";
	$intopt{$arg} = 0;
}

sub add_hex($)
{
	my $arg=shift;

	print "Hex: $arg\n" if $debug;
	exists $config{$arg} or die "Adding unknown hex '$arg'";
	$hexopt{$arg} = 0;
}

sub add_string($)
{
	my $arg=shift;

	print "String: $arg\n" if $debug;
	exists $config{$arg} or die "Adding unknown string '$arg'";
	$stringopt{$arg} = "";
}

sub set_int_value($$)
{
	my $key = shift;
	my $val = shift;

	exists $intopt{$key} or die "Default for unknown int option '$key'";
	$intopt{$key} = $val;
}

sub set_hex_value($$)
{
	my $key = shift;
	my $val = shift;

	exists $hexopt{$key} or die "Default for unknown hex option '$key'";
	$hexopt{$key} = "0x$val";
}

sub set_string_value($$)
{
	my $key = shift;
	my $val = shift;

	exists $stringopt{$key} or die "Default for unknown string option '$key'";
	$stringopt{$key} = "\"$val\"";
}

sub add_config($)
{
	my $arg = shift;

	if ($arg =~ /^(\w+)\s*$/) {
		$config{$1} = 2;
	} else {
		die "Do not understand config variable '$arg'";
	}
}

########################################
# Turn option off, iff it already exists
sub disable_config($)
{
	my $key = shift;

	$config{$key} = 0 if (exists $config{$key});
}

# %{$depend{'FOO'}} lists all variables which depend on FOO.  This is the
# reverse of the variables that FOO depends on.
my %depend = ();
sub finddeps($$)
{
	my $key = shift;
	my $deps = shift;

	$deps =~ s/^\W+//;
	$depend{$_}{$key} = 1 foreach(split(/\W[\Wynm]*/, $deps));
}

# @{$depends{'FOO'}} is a list of dependency expressions that FOO depends
# on.  This is the reverse of the variables that depend on FOO.
my %depends = ();
sub depends($$)
{
	my $key = shift;
	my $deps = shift;

	(!defined $key || $key eq '') and
		die "Got bad key with $deps\n";
	finddeps($key, $deps);
	push @{$depends{$key}}, $deps;
}

sub selects($$$)
{
	my $key = shift;
	my $select = shift;
	my $if = shift;

	finddeps($key, $select);
	if(defined $if) {
		finddeps($key, $if);
		# Transform "select X if Y" into "depends on !Y || X"
		$select = "!($if) || ($select)";
	}
	push @{$depends{$key}}, $select;
}

# Needs:
# %depend <- %depend{FOO} lists the variables that depend on FOO
# %depends <- %depends{FOO} lists the dependency expressions for FOO
# %config <- the value of all media variables
# %kernopts <- the value of all kernel variables (including media ones)
#
# Checks the dependencies of all media variables, recursively.  Returns
# a new hash that has the new values of all media variables.
sub checkdeps()
{
	my %allconfig;

	@allconfig{keys %depend} = @kernopts{keys %depend};
	@allconfig{keys %config} = values %config;
	# Set undef values to 0
	map { defined $_ or $_ = 0 } values %allconfig;

	# First run, check deps for all the v4l-dvb variables.  Following
	# runs, check all variables which depend on a variable that was
	# changed.  Stop when no more variables change.
	for(my %changed = (), my @tocheck = keys %config; @tocheck;
	    @tocheck = keys %changed, %changed = ()) {
		foreach (@tocheck) {
			next unless($allconfig{$_});	# skip disabled vars
			if (!checkvardeps($_)) {
				$changed{$_} = 1 foreach keys %{$depend{$_}};
			}
		}
	}
	return map { $_ => $allconfig{$_} } keys %config;

	# Check the dependencies of a variable, if one fails the variable
	# is set to 0.  Returns 0 if the variable failed, 1 if it passed.
	sub checkvardeps($) {
		my $key = shift;
		my $deps = $depends{$key};
		foreach (@$deps) {
			next if($_ eq '');
			if (!eval(toperl($_))) {
				print "Disabling $key, dependency '$_' not met\n" if $debug;
				$allconfig{$key} = 0;
				return 0;
			}
		}
		return 1;
	}

	# Convert a Kconfig expression to a Perl expression
	sub toperl($)
	{
		local $_ = shift;

		# Turn n,m,y into 0,1,2
		s/\bn\b/0/g; s/\bm\b/1/g; s/\by\b/2/g;

		# Turn = into ==, but keep != as != (ie. not !==)
		s/(?<!!)=/==/g;

		# Turn FOO into the value of $allconfig{FOO}
		s/\b([A-Za-z_]\w*)\b/$allconfig{$1}/g;

		return $_;
	}
}

# Text to be added to disabled options
my $disabled_msg = <<'EOF';
	---help---
	  WARNING! This driver needs at least kernel %s!  It may not
	  compile or work correctly on your kernel, which is too old.

EOF

# List of all kconfig files read in, used to make dependencies for the
# output combined Kconfig file.
my @kconfigfiles = ();

# "if" statements found in Kconfig files.
my @kifs = ();

# Read and parse a Kconfig file.  First argument is base of source
# directory tree, the second is the file to open (with path).  Recursivly
# parses Kconfig files from 'source' directives.
#
# Prints to OUT a combined version of all the Kconfig files.  This file
# is edited slightly to disable options that need a newer kernel.
sub open_kconfig($$) {
	my ($dir,$file)=@_;
	my $in = new FileHandle;
	my $key;
	my $disabled = 0;
	my $in_help = 0;
	my $default_seen = 0;
	my $if;
	my $line;

	print "Opening $file\n" if $debug;
	open $in, '<', $file or die "File not found: $file";
	push @kconfigfiles, $file;
	while (<$in>) {
		$line = $_;
		# In our Kconfig files, the first non-help line after the
		# help text always has no indention.  Technically, the
		# help text is ended by just by the indention decreasing,
		# but that is a pain to keep track of.
		if ($in_help && /^\S/) {
			$in_help = 0;
		} elsif ($in_help) {
			# Still inside help text
			next;
		}

		# Start of help text
		if (/^\s*(---)?help(---)?\s*$/) {
			$in_help = 1;
			# Insert VIDEO_KERNEL_VERSION dependency, default
			# n line, and help text note for disabled drivers.
			if ($disabled) {
				if(exists $tristate{$key} && !$default_seen) {
					print OUT "\tdefault n\n";
				}
				print OUT "\tdepends on VIDEO_KERNEL_VERSION\n";
				$line = sprintf($disabled_msg, $minver{$key});
			}
			next;
		}
		# No help text should get processed past this point
		$in_help and die "I'm very confused";

		# start of a new stanza, reset
		if (/^\w/) {
			$disabled = 0;
			$default_seen = 0;
			$key = undef;
			$if = "";
		}
		next if (/^\s*#/ || /^\s*$/); # skip comments and blank lines

		# Erase any comments on this line
		s/(?<!\\)#(.*)$//;

		if (m|^\s*source\s+"([^"]+)"\s*$| ||
		    m|^\s*source\s+(\S+)\s*$|) {
			open_kconfig($dir, "$dir/$1");
			$line = '';	# don't print the source line itself
			next;
		}

		my $nothandled = 0;
		if (m|^\s*(?:menu)?config (\w+)\s*$|) {
			$key = $1;
			$if = "";
			print "Found config '$key' at $file:$.\n" if $debug;
			add_config($key);

			if (exists $minver{$key} &&
			    cmp_ver($minver{$key}, $kernver) > 0) {
				$disabled = 1;
				disable_config($key);
				print "$key: Requires at least kernel $minver{$key}\n";
				print OUT "# $key disabled for insufficient kernel version\n";
			} else {
				$disabled=0;
			}
			# Add dependencies from enclosing if/endif blocks
			depends($key, $_) foreach (@kifs);
		} elsif (m|^\s*comment\s+"[^"]*"\s*$|) {
			$key = 'comment';
		} elsif (m|^\s*menu\s+"[^"]*"\s*$|) {
			$key = 'menu';
			push @kifs, '';  # placeholder for any depends on clauses
		} elsif (m|^\s*if\s+(.+?)\s*$|) {
			push @kifs, $1;
		} elsif (/^\s*end(if|menu)\s*(?:#.*)?$/) {
			# Won't handle menu/if blocks that aren't strictly
			# nested, but no one should do that!
			$#kifs >= 0 or die "Unmatched end$1 at $file:$.\n";
			pop @kifs;
		} else {
			$nothandled = 1;
		}
		next unless ($nothandled);
		# Remaining Kconfig directives only makse sense inside Kconfig blocks
		unless(defined $key) {
			print "Skipping $file:$. $_" if $debug;
			next;
		}

		# Don't process any directives in comment blocks
		next if ($key eq 'comment');

		# Only depends on lines are accepted in menus
		if ($key eq 'menu') {
			if (m|^\s*depends on\s+(.+?)\s*$|) {
			    my $x = pop @kifs;
			    $x .= ' && ' if($x ne '');
			    $x .= "($1)";
			    push @kifs, $x;
			} else {
			    print "Skipping unexpected line in menu stanza $file:$.$_" if $debug;
			}
			next;
		}

		# config type
		if(/^\s*bool(ean)?\s/) {
			add_bool($key);
			if (m|if (.*)\s*$|) {
				printf("Boolean $key with if '$1'\n") if $debug;
				if ($if eq "") {
					$if = "($1)";
				} else {
					$if .= " && ($1)";
				}
			}
		} elsif (/^\s*tristate\s/) {
			add_tristate($key);
			if (m|if (.*)\s*$|) {
				printf("Boolean $key with if '$1'\n") if $debug;
				if ($if eq "") {
					$if = "($1)";
				} else {
					$if .= " && ($1)";
				}
			}
		} elsif (/^\s*int\s/) {
			add_int($key);
		} elsif (/^\s*hex\s/) {
			add_hex($key);
		} elsif (/^\s*string\s/) {
			add_string($key);

		# select and depend lines
		} elsif (m|^\s*depends on\s+(.+?)\s*$|) {
			depends($key, $1);
		} elsif (m|^\s*select\s+(\w+)(\s+if\s+(.+?))?\s*$|) {
			selects($key, $1, $3);

		# default lines
		} elsif (m|^\s*default\s+(.+?)(?:\s+if .*)?\s*$|) {
			my $o = $1;
			if ($2 ne "") {
				if ($if eq "") {
					$if = "($2)";
				} else {
					$if .= " && ($2)";
				}
			}

			# Get default for int options
			if ($o =~ m|^"(\d+)"$| && exists $intopt{$key}) {
				set_int_value($key, $1);
			# Get default for hex options
			} elsif ($o =~ m|^"(0x)?([[:xdigit:]]+)"$| && exists $hexopt{$key}) {
				set_hex_value($key, $2);
			# Get default for string options
			} elsif ($o =~ m|^"(.*)"$| && exists $stringopt{$key}) {
				set_string_value($key, $1);

			# Override default for disabled tri/bool options
			# We don't care about the default for tri/bool options otherwise
			} elsif (!$o =~ /^(y|n|m|"yes"|"no")$/i && exists $tristate{$key}) {
				print "Default is an expression at $file:$. $_\n" if $debug;
				if ($if eq "") {
					depends($key, "$o");
				}
			}
			if ($if ne "") {
				# FIXME: What happens if no default clause exists?
				# the $if won't be handled
				depends($key, "$if || $o");
			}

			if ($disabled) {
				$default_seen = 1;
				$line = "\tdefault n\n";
			}
		} else {
			print "Skipping $file:$. $_" if $debug;
		}
	} continue {
		if (cmp_ver($kernver, '2.6.37') < 0 && $line =~ m/visible\sif\s.*/) {
			$line =~ s/visible\sif\s(.*)//;
			print OUT $line;
			print OUT "\tdepends on $1\n";
		} else {
			print OUT $line;
		}
	}
	close $in;
}

sub parse_versions()
{
	my $in = new FileHandle;
	my $ver;

	open $in, '<versions.txt' or die "File not found: versions.txt";
	while (<$in>) {
		if (/\[(\d+\.\d+\.\d+)\]/) {
			$ver = $1;
		} elsif (/^\s*(\w+)/) {
			$minver{$1} = $ver;
			print "minimal version for $1 is $ver\n" if $debug;
		}
	}
	close $in;
}

# Read in the .version file to get the kernel version
sub get_version()
{
	open IN, '<.version' or die 'File not found: .version';
	while (<IN>) {
		if (/KERNELRELEASE\s*[:]*[=]+\s*(\d+)\.(\d+)[\.-](\d+)/) {
			$version=$1;
			$level=$2;
			$sublevel=$3;
			return $kernver="$version.$level.$sublevel";
		}
	}
	close IN;
}

# Like ver1 <=> ver2, but understands X.Y.Z version strings
sub cmp_ver($$)
{
	shift =~ /(\d+)\.(\d+)\.(\d+)/;
	my ($v1_ver,$v1_level,$v1_sublevel) = ($1,$2,$3);
	shift =~ /(\d+)\.(\d+)\.(\d+)/;
	my ($v2_ver,$v2_level,$v2_sublevel) = ($1,$2,$3);

	my $cmp = $v1_ver <=> $v2_ver;
	return $cmp unless($cmp == 0);
	$cmp = $v1_level <=> $v2_level;
	return $cmp unless($cmp == 0);
	return $v1_sublevel <=> $v2_sublevel;
}

# Get kernel version
get_version();
print "Preparing to compile for kernel version $kernver\n";

# Get Kernel's config settings
%kernopts = process_config("$kernel/.config");

# Modules must be on, or building out of tree drivers makes no sense
if(!$kernopts{MODULES}) {
	print <<"EOF";
You appear to have loadable modules turned off in your kernel.  You can
not compile the v4l-dvb drivers, as modules, and use them with a kernel
that has modules disabled.

If you want to compile these drivers into your kernel, you should
use 'make kernel-links' to link the source for these drivers into
your kernel tree.  Then configure and compile the kernel.
EOF
	exit -1;
}

# Kernel < 2.6.22 is missing the HAS_IOMEM option
if (!defined $kernopts{HAS_IOMEM} && cmp_ver($kernver, '2.6.22') < 0) {
    $kernopts{HAS_IOMEM} = 2;
}

# Kernel < 2.6.22 is missing the HAS_DMA option
if (!defined $kernopts{HAS_DMA} && cmp_ver($kernver, '2.6.22') < 0) {
    $kernopts{HAS_DMA} = 2;
}

# Kernel < 2.6.23 is missing the VIRT_TO_BUS option
if (!defined $kernopts{VIRT_TO_BUS} && cmp_ver($kernver, '2.6.23') < 0) {
	# VIRT_TO_BUS -> !PPC64
	$kernopts{VIRT_TO_BUS} = 2 - $kernopts{PPC64};
}

# Kernel < 2.6.37 is missing the BKL option
if (!defined $kernopts{BKL} && cmp_ver($kernver, '2.6.37') < 0) {
    $kernopts{BKL} = 2;
}

# Kernel < 3.2 is missing the DMA_SHARED_BUFFER option
if (!defined $kernopts{DMA_SHARED_BUFFER} && cmp_ver($kernver, '3.2.0') < 0) {
    $kernopts{DMA_SHARED_BUFFER} = 2;
}

# Get minimum kernel version for our variables
parse_versions();

kernelcheck();

open OUT, ">Kconfig" or die "Cannot write Kconfig file";
print OUT <<"EOF";
mainmenu "V4L/DVB menu"
source Kconfig.kern
config VIDEO_KERNEL_VERSION
	bool "Enable drivers not supported by this kernel"
	default n
	---help---
	  Normally drivers that require a kernel newer $version.$level.$sublevel,
	  the kernel you are compiling for now, will be disabled.

	  Turning this switch on will let you enabled them, but be warned
	  they may not work properly or even compile.

	  They may also work fine, and the only reason they are listed as
	  requiring a newer kernel is that no one has tested them with an
	  older one yet.

	  If the driver works, please post a report to the V4L mailing list:
			 linux-media\@vger.kernel.org.

	  Unless you know what you are doing, you should answer N.

EOF

open_kconfig('../linux', '../linux/drivers/media/Kconfig');
open_kconfig('.', './Kconfig.sound');
open_kconfig('.', './Kconfig.mm');
open_kconfig('.', './Kconfig.misc');
open_kconfig('../linux', './Kconfig.staging');
close OUT;

# These options should default to off
disable_config('DVB_AV7110_FIRMWARE');
disable_config('DVB_CINERGYT2_TUNING');
disable_config('VIDEO_HELPER_CHIPS_AUTO');
disable_config('VIDEO_FIXED_MINOR_RANGES');
disable_config('STAGING') if (!$enable_staging);
disable_config('STAGING_BROKEN');
$config{'COMPILE_TEST'} = 0;
$intopt { "DVB_MAX_ADAPTERS" } = 48;

# Check dependencies
my %newconfig = checkdeps();

# TODO: tell the user which options were disabled at this point
%config = %newconfig;

# Create Kconfig.kern, listing all non-media (i.e. kernel) variables
# that something depended on.
$depend{MODULES}{always} = 1;	# Make sure MODULES will appear
open OUT, '>Kconfig.kern' or die "Cannot write Kconfig.kern file: $!";
while (my ($key, $deps) = each %depend) {
	next if exists $config{$key};	# Skip media variables

	print OUT "# Needed by ", join(', ', keys %$deps), "\n";
	print OUT "config $key\n\ttristate\n";
	print OUT "\tdefault ", qw(n m y)[$kernopts{$key}], "\n\n";
	print OUT "\toption modules\n" if ($key eq "MODULES");
}
close OUT;

# Create make dependency rules for the Kconfig file
open OUT, '>.kconfig.dep' or die "Cannot write .kconfig.dep file: $!";
print OUT 'Kconfig: ';
print OUT join(" \\\n\t", @kconfigfiles), "\n";
close OUT;

# Produce a .config file if forced or one doesn't already exist
if ($force_kconfig==1 || !-e '.config') {
	open OUT, '>.config' or die "Cannot write .config file: $!";
	foreach (keys %tristate) {
		if ($config{$_}) {
			print OUT "CONFIG_$_=", qw(n m y)[$config{$_}], "\n";
		} else {
			print OUT "# CONFIG_$_ is not set\n";
		}
	}
	while (my ($key,$value) = each %intopt) {
		print OUT "CONFIG_$key=$value\n" if($config{$key});
	}
	while (my ($key,$value) = each %hexopt) {
		print OUT "CONFIG_$key=$value\n" if($config{$key});
	}
	while (my ($key,$value) = each %stringopt) {
		print OUT "CONFIG_$key=$value\n" if($config{$key});
	}
	close OUT;
	print "Created default (all yes) .config file\n";
}

# Check for full kernel sources and print a warning
sub kernelcheck()
{
	my $fullkernel="$kernsrc/fs/fcntl.c";
	if (! -e $fullkernel) {
		print <<"EOF2";

***WARNING:*** You do not have the full kernel sources installed.
This does not prevent you from building the v4l-dvb tree if you have the
kernel headers, but the full kernel source may be required in order to use
make menuconfig / xconfig / qconfig.

If you are experiencing problems building the v4l-dvb tree, please try
building against a vanilla kernel before reporting a bug.

Vanilla kernels are available at http://kernel.org.
On most distros, this will compile a newly downloaded kernel:

cp /boot/config-`uname -r` <your kernel dir>/.config
cd <your kernel dir>
make all modules_install install

Please see your distro's web site for instructions to build a new kernel.

EOF2
	}
print << "EOF3";
WARNING: This is the V4L/DVB backport tree, with experimental drivers
	 backported to run on legacy kernels from the development tree at:
		http://git.linuxtv.org/media-tree.git.
	 It is generally safe to use it for testing a new driver or
	 feature, but its usage on production environments is risky.
	 Don't use it in production. You've been warned.
EOF3
	sleep 5;
}
