#!/usr/bin/perl

# dump the PBL, and the PBL Downloader from oemsbl. (probably requires radio s-off)

use strict;
use warnings;

use Fcntl;

$| = 1;

{
    my ($fd, $tty) = setupTTY();
    
    if(!defined $fd)
    {
	print "Failed to find/open TTY.\n";
	exit 1;
    }
    
    print "Using TTY: $tty\n";

    print "Dumping PBL... (0xffff0000 - 0xffffffff)\n";
    dumpMemory($fd, 0xffff0000, 0x10000, "pbl.bin");
    dumpMemory($fd, 0xfffe6000, 0xa000, "pbl_download.bin");

}

sub dumpMemory
{
    my $fd = shift;
    my $address = shift;
    my $length = shift;
    my $outfile = shift;

    print $fd "mb " . sprintf('%x', $address) . " " . sprintf('%x', $length) . "\r\n";

    my @binary;
    while(<$fd>)
    {
	my ($line) = /^(.*)\r$/;

	last if(!length($line));

        my @bytes = split(/\s/, join('', $line =~ /^[^\:]+\: ((?:[0-9A-F]{2}\s){8})-\s((?:[0-9A-F]{2}\s){8})/));
        push @binary, @bytes;
        print "Received: ", scalar @binary, "\r";
    }
    print "\n";
    
    my $output = pack('C*', map { hex } @binary);

    open OUTPUT, ">", $outfile or die;
    print OUTPUT $output;
    close OUTPUT;
}

sub setupTTY
{
    my $tty;
    my $fd;
    my @usbDevices;

    my @devices = grep { !/\:/ && !/usb/ } map { (split(/\//, $_))[-1] } glob '/sys/bus/usb/devices/*';

    foreach my $device (@devices)
    {
	my $product = getFile("/sys/bus/usb/devices/$device/idProduct");
	my $vendor = getFile("/sys/bus/usb/devices/$device/idVendor");
	
	next if(!defined $product || !defined $vendor);
	
	if($vendor eq '05c6' && $product eq '900e')
	{
	    # it's us.
	    my @drivers = grep /^$device:\d+\.\d+$/, map { (split(/\//, $_))[-1] } glob "/sys/bus/usb/devices/$device/$device:*";
	    
	    foreach my $driver (@drivers)
	    {
		
		my @ttys = map { (split(/\//, $_))[-1] } glob "/sys/bus/usb/devices/$device/$driver/tty*";
		
		if(scalar @ttys == 1)
		{
		    $tty = "/dev/$ttys[0]";
		}
	    }
	}
    }

    if(defined $tty)
    {
	system "/bin/stty raw -iexten -echo < $tty";
	return undef if(!open($fd, '+<', $tty));
	return ($fd, $tty);
    }

    return undef;
}

sub getFile
{
    my $file = shift;

    local $/ = undef;

    return undef if(!open(FILE, $file));

    my $content = <FILE>;
    $/ = "\n";
    close FILE;
    chomp $content;
    return $content;
}
