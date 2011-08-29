#!/usr/bin/perl

use strict;
use warnings;

$/ = undef;

if(!open(GFMOD_KO, "../gfmod/gfmod.ko"))
{
    print STDERR "Failed to open gfmod.ko\n";
    exit 1;
}

if(!open(GFMOD_HEADER, ">", "gfmod.h"))
{
    print STDERR "Failed to open gfmod.h\n";
    exit 1;
}

my $gfmod_ko = <GFMOD_KO>;
my @gfmod_koArray = unpack('C*', $gfmod_ko );

print GFMOD_HEADER "#include <stdint.h>\n";
print GFMOD_HEADER "static uint8_t gfmod_ko[] = {" . join(', ', @gfmod_koArray), "};\n";

if(!open(GKMEM_KO, "../gkmem/gkmem.ko"))
{
    print STDERR "Failed to open gkmem.ko\n";
    exit 1;
}

if(!open(GKMEM_HEADER, ">", "gkmem.h"))
{
    print STDERR "Failed to open gkmem.h\n";
    exit 1;
}

my $gkmem_ko = <GKMEM_KO>;
my @gkmem_koArray = unpack('C*', $gkmem_ko );

print GKMEM_HEADER "#include <stdint.h>\n";
print GKMEM_HEADER "static uint8_t gkmem_ko[] = {" . join(', ', @gkmem_koArray), "};\n";
