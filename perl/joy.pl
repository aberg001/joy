#!/usr/bin/env perl

use warnings;
use strict;

use FindBin;
use lib "$FindBin::Bin/.";
use joy;

my $j = joy->new();
$j->parse(q(1 2.5 9. 3 - ++));
print "1\n";

exit 1;
