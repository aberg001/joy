package joy_types;

use v5.24.1;
use lib ".";
use joy_types;

package joy_types::number;

sub new {
  my $class = shift;
  my $val = shift;
  my $self = \$val;
  bless $self, $class;
  return $self;
}

sub desc {
  my $self = shift;
  return "number<$$self>";
}

sub print {
  my $self = shift;
  my $val = $$self;
  print $val;
}

sub name {
  return 'number';
}

sub key {
  return undef;
}

sub val {
  my $self = shift;
  return $$self;
}

package joy_types::symbol;

sub new {
  my $class = shift;
  my $key = shift;
  my $val = shift;
  my $self = [$key, $val];
  bless $self, $class;
  return $self;
}

sub desc {
  my $self = shift;
  my ($key, $val) = @$self;
  return "symbol<$key,$val>";
}

sub print {
  my $self = shift;
  my ($key, $val) = @$self;
  print "$key";
}

sub name {
  return 'symbol';
}

sub key {
  my $self = shift;
  return $self->[0];
}

sub val {
  my $self = shift;
  return $self->[1];
}

package joy_types::string;

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub desc {
  my $self = shift;
  return "string<'$self'>";
}

sub print {
  my $self = shift;
  my $str = $self;

  $str =~ s/"/""/g;
  print qq("$str");
}

sub name {
  return 'string';
}

sub key {
  return undef;
}

sub val {
  my $self = shift;
  return $$self;
}

package joy_types::sequence;

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub desc {
  my $self = shift;
  return "sequence<...>";
}

sub print {
  my $self = shift;

  print '[ ';
  for my $e (@$self) {
    $e->print();
    print ' ';
  }
  print '] ';
}

sub name {
  return 'sequence';
}

sub key {
  return undef;
}

sub val {
  my $self = shift;
  return $self;
}

1;
