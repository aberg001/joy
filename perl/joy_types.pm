package joy_types;

use v5.24.1;
use lib ".";
use joy_types;

package joy_types::number;

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub name {
  return 'number';
}

package joy_types::symbol;

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub name {
  return 'symbol';
}

package joy_types::string;

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub name {
  return 'string';
}

package joy_types::sequence;

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub name {
  return 'sequence';
}

1;
