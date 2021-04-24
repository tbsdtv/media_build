#!/usr/bin/perl

# This script allows checking what patches are needed for a given
# kernel version.
# Use it as ./patches_for_kernel.pl <version>
# The backport files are specified at: ../backports/backports.txt

use strict;

my $version = shift or die "Need a version to apply patches";

my $file = "../backports/backports.txt";
open IN, $file or die "can't find $file\n";

sub kernel_version($) {
	my ($version, $patchlevel, $sublevel) = $_[0] =~ m/^(\d+)\.(\d+)\.?(\d*)/;

	# fix kernel version for distros that 'translated' 3.0 to 2.6.40
	if ($version == 2 && $patchlevel == 6 && $sublevel >= 40) {
		$version = 3;
		$patchlevel = $sublevel - 40;
		$sublevel = 0;
	}
	$sublevel = 0 if ($sublevel == "");
	$sublevel = 255 if ($sublevel > 255);
	return ($version * 65536 + $patchlevel * 256 + $sublevel);
}

my $kernel = kernel_version($version);

die "Invalid kernel version" if (!$kernel);

my $cur;
my @patches;
my $ln;
while (<IN>) {
	$ln++;
	s/\#.*//;
	next if (m/^\n*$/);
	if (m/\[(.*)\]/) {
		$cur = kernel_version($1);
		next;
	}
	if (m/add\s+(.*)/) {
		next if ($cur < $kernel);
		push @patches, $1;
		next;
	}
	if (m/delete\s+(.*)/) {
		next if ($cur < $kernel);
		for (my $i = 0; $i < @patches; $i++) {
			delete $patches[$i] if ($patches[$i] eq $1);
		}
		next;
	}
	die "Can't process line $ln\n";
}
close IN;
die ("Unsupported Kernel version $version") if ($cur > $kernel);

foreach my $patch (@patches) {
	printf "%s ", $patch;
}
print "\n";
