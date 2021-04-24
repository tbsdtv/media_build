#!/usr/bin/perl
use strict;
use File::Find;
use File::Path;
use File::Copy;
use Fcntl ':mode';
use Getopt::Long;
use Digest::SHA;

my $silent = 0;
my $debug = 0;
my $recheck = 0;
my $get_patched = 0;
GetOptions( "--debug" => \$debug,
	    "--silent" => \$silent,
	    "--recheck" => \$recheck,
	    "--get_patched" => \$get_patched,
	  );

my $dir = shift;

my $ctlfile = ".linked_dir";
my $patchfile = ".patches_applied";

my $sync_patched = 0;
my %dirs;
my %files;
my $patches_applied;

#########################################
# Control info stored at the control file
my $path;
my %fhash;
my %fhash_patched;
#########################################

sub read_ctlfile()
{
	my $line;

	open IN, $ctlfile or return;
	while (<IN>) {
		next if (m/^\s*\#/);
		next if (m/^\n$/);
		if (m/^path:\s*([^\s]+)/) {
			$path = $1;
		} elsif (m/^hash\:\s*([^\s]+)\s*=\s*([^\s]+)/) {
			$fhash{$1} = $2;
		} elsif (m/^hash_patched\:\s*([^\s]+)\s*=\s*([^\s]+)/) {
			$fhash_patched{$1} = $2;
		} else {
			printf("Parse error on this line of $ctlfile:\n\t$_");
			die;
		}
	}
	close IN;
}

sub write_ctlfile()
{
	open OUT, ">$ctlfile" or print "Error: Can't write to $ctlfile\n";
	print OUT "path: $path\n";
	foreach my $file (keys %fhash) {
		printf OUT "hash: %s=%s\n", $file, $fhash{$file};
	}
	foreach my $file (keys %fhash_patched) {
		printf OUT "hash_patched: %s=%s\n", $file, $fhash_patched{$file};
	}
	close OUT;
}

sub add_dirs($)
{
	my $data = shift;
	my @dirs = split(' ', $data);

	foreach my $val (@dirs) {
		$dirs{$val} = 1;
	}
}

sub add_files($)
{
	my $data = shift;
	my @dirs = split(' ', $data);

	foreach my $val (@dirs) {
		$files{$val} = 1;
	}
}

sub get_file_dir_names()
{
	open IN, "Makefile" or die "Couldn't open Makefile";
	while (<IN>) {
		if (m/^\s*TARDIR\s*[\+\:]*=\s*([A-Za-z_].*)/) {
			add_dirs($1);
		} elsif (m/^\s*TARFILES\s*[\+\:]*=\s*([A-Za-z_].*)/) {
			add_files($1);
		}
	}
	close IN;
}


sub hash_calc($)
{
	my $file = shift;

	my $ctx = Digest::SHA->new;

	my $rc = open INHASH, $file;
	if (!$rc) {
		print "Couldn't open file $file\n" if ($debug);
		return 0;
	}
	$ctx->addfile(*INHASH);
	my $digest = $ctx->hexdigest;
	close INHASH;

	return $digest;
}

sub sync_files($)
{
	my $file = shift;
	my $path = $file;
	my $check_hash;
	my $need_sync;
	my $filehash;
	my $cpfilehash;
	my $patched_file;

	$path =~ s,/[^/]+$,,;

	$filehash = hash_calc("$dir/$file");
	$need_sync = 1 if ($filehash ne $fhash{$file});

	if (!$need_sync && $recheck) {
		$cpfilehash = hash_calc("$file");
		if ($patches_applied && exists($fhash_patched{$file})) {
			$patched_file = 1;
			$need_sync = 1 if ($cpfilehash ne $fhash_patched{$file});
		} else {
			$need_sync = 1 if ($cpfilehash ne $fhash{$file});
		}
	}

	if ($need_sync) {
		printf "Sync'ing file $file (orig = %s, copy = %s, patched = %s)\n",
			$filehash, $cpfilehash, $fhash_patched{$file} if ($debug || $recheck);

		if (exists($fhash_patched{$file})) {
			$sync_patched = 1;
		} else {
			$fhash{$file} = $filehash;
			mkpath($path);
			copy("$dir/$file", $file);
		}
	} else {
		print "Skipping file $file, as is already synchronized\n" if ($debug);
	}
}

sub get_patched_files()
{
	my %files;

	open IN, $patchfile or return %files;

	# Those files are always patched to add warnings about the usage of experimental version
	$files{"drivers/media/dvb-core/dvbdev.c"} = 1;
	$files{"drivers/media/v4l2-core/v4l2-dev.c"} = 1;
	$files{"drivers/media/rc/rc-main.c"} = 1;

	while (<IN>) {
		next if (/^\s*#/);

		if (m/(.*)\n/) {
			print ("Backport ../backports/$1 touch(es) file(s):\n") if ($debug);
			open IN2, "lsdiff -h --strip 1 ../backports/$1 |";
			while (<IN2>) {
				my $f = $_;
				$f =~ s/\n//;
				$files{$f} = 1;
				print ("\t$f\n") if ($debug);
			}
			close IN2;
		}
	}
	close IN;

	return %files;
}

sub sync_patched_files()
{
	my %patches = get_patched_files();
	return if (!%patches);

	foreach my $file (keys %patches) {
printf "sync patched file $file\n";
		$fhash{$file} = hash_calc("$dir/$file");
		mkpath($path);
		copy("$dir/$file", $file);
	}
	close IN;
}

sub get_patched_hashes()
{
	my %patches = get_patched_files();
	return if (!%patches);

	foreach my $file (keys %patches) {
		$fhash_patched{$file} = hash_calc("$file");
		printf "Hash for patched file $file = %s\n", $fhash_patched{$file} if ($debug);
	}
	close IN;
}

sub remove_deleted()
{
	my $file = $File::Find::name;
	my $mode = (stat($file))[2];

	return if ($mode & S_IFDIR);

	return if ($file =~ /^\./);
	return if ($file =~ /\.mod\.c/);

	if ($file =~ /Makefile$/ || $file =~ /Kconfig$/ || $file =~ /\.[ch]$/ ) {
		if (! -e "$dir/$file") {
			printf "Removing file $file\n" if (!$silent);
			delete $fhash{$file} if exists($fhash{$file});
			delete $fhash_patched{$file} if exists($fhash_patched{$file});
			unlink $file;
			return;
		}
	}
}

sub parse_dir()
{
	my $file = $File::Find::name;
	my $mode = (stat($file))[2];

	return if ($mode & S_IFDIR);

	$file =~ s,^($dir/),,;

	return if ($file =~ /^\./);
	return if ($file =~ /\.mod\.c/);

	if ($file =~ /Makefile$/ || $file =~ /Kconfig$/ || $file =~ /\.[ch]$/ ) {
		sync_files $file;
		return;
	}

	printf "Skipping bogus file $file\n" if ($debug);
}

sub sync_dirs($)
{
	my $subdir = shift;

	print "sync dir: $subdir\n" if (!$silent);
	find({wanted => \&parse_dir, no_chdir => 1}, "$dir/$subdir");
	find({wanted => \&remove_deleted, no_chdir => 1}, "$subdir");
}

sub sync_all()
{
	foreach my $val (keys %files) {
		print "sync file: $val\n" if (!$silent);
		sync_files($val);
	}
	foreach my $val (keys %dirs) {
		sync_dirs($val);
	}
}

sub sync_kernel_version()
{
	my ($source_v4l_version, $a, $b, $c, $ver);

	open IN, "$dir/Makefile" or die "Can't find $dir/Makefile";
	while (<IN>) {
		$a=$1 if (m/^\s*VERSION\s*=\s*(\d+)/);
		$b=$1 if (m/^\s*PATCHLEVEL\s*=\s*(\d+)/);
		$c=$1 if (m/^\s*SUBLEVEL\s*=\s*(\d+)/);
	}
	close IN;
	$source_v4l_version = ((($a) << 16) + (($b) << 8) + ($c > 255 ? 255 : $c));

	if (open IN, "kernel_version.h") {
		while (<IN>) {
			$ver=$1 if (m/^#define\s*V4L2_VERSION* \s*(\d+)/);
		}
		close IN;
	}

	if ($ver ne $source_v4l_version) {
		open OUT,">kernel_version.h";
		printf OUT "#define V4L2_VERSION %d\n", $source_v4l_version;
		close OUT;
	}
}

# Main

if (!$dir) {
	read_ctlfile();
	die "Please provide a directory to use" if !($path);
	$dir = $path;

	printf "Syncing with dir $dir\n";
} else {
	read_ctlfile();
}
sync_kernel_version();

if ($path ne $dir) {
	$path = $dir;
	%fhash = ();
}

$patches_applied = 1 if (-e $patchfile);

if ($get_patched && $patches_applied) {
	get_patched_hashes();
} else {
	get_file_dir_names();
	sync_all();

	if ($sync_patched) {
		sync_patched_files();
		unlink $patchfile;
	}
}

write_ctlfile();
system "git --git-dir $dir/.git log --pretty=oneline -n3 |sed -r 's,([\x22]),,g; s,([\x25\x5c]),\\1\\1,g' >git_log"
