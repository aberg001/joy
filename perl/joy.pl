#!/usr/bin/env perl

use warnings;
use strict;
use v5.24.1;

use FindBin;
use lib "$FindBin::Bin/.";
use joy;

my $j = joy->new();
# $j->parse(q(1 2 3 + +));
# my $r = $j->parse(q(1 [2 3 +] +));
my $r = $j->parse(q(foo = 1 [2 3 +] + .));
say 'result: ', $r;
$j->run();
$j->print();

exit 1;
