#!/usr/bin/perl

use strict;
use warnings;

$/ = undef;

if(!open(KO, "wpthis.ko"))
{
    print STDERR "Failed to open wpthis.ko\n";
    exit 1;
}

if(!open(HEADER, ">", "wpthis.h"))
{
    print STDERR "Failed to open wpthis.h\n";
    exit 1;
}

my $ko = <KO>;
my @koArray = unpack('C*', $ko);

print HEADER "#include <stdint.h>\n";
print HEADER "static uint8_t wpthis_ko[] = {" . join(', ', @koArray), "};\n";
