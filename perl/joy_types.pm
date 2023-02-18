package joy_types;

use v5.24.1;
use lib ".";

sub desc {
  my $self = shift;

  my $key = $self->key();
  if ($key) {
    return $self->name().'<'.$key.'|'.$self->shortStr().'>';
  } else {
    return $self->name().'<'.$self->shortStr().'>';
  }
}

sub print {
  print $_[0]->toStr();
}

sub shortStr {
  return $_[0]->toStr();
}

sub eval {
  push @{$_[1]}, $_[0]->val();
}

sub key {
  return undef;
}

# ========================================================================
package joy_types::number;
use parent 'joy_types';

sub new {
  my $class = shift;
  my $val = shift;
  my $self = \$val;
  bless $self, $class;
  return $self;
}

sub name {
  return 'number';
}

sub toStr {
  my $val = ${$_[0]};
  return "$val";
}

sub val {
  return ${$_[0]};
}

# ========================================================================
package joy_types::symbol;
use parent 'joy_types';

sub new {
  my $class = shift;
  my $key = shift;
  my $val = shift;
  my $self = [$key, $val];
  bless $self, $class;
  return $self;
}

sub name {
  return 'symbol';
}

sub toStr {
  return $_[0]->key();
}

sub key {
  return $_[0]->[0];
}

sub val {
  return $_[0]->[1];
}

# ========================================================================
package joy_types::string;
use parent 'joy_types';

sub new {
  my $class = shift;
  my $self = shift;
  bless $self, $class;
  return $self;
}

sub name {
  return 'string';
}

sub toString {
  my $str = $_[0];

  $str =~ s/(^|"|$)/"$1/g;
  return $str;
}

sub val {
  return $_[0];
}

# ========================================================================
package joy_types::sequence;
use parent 'joy_types';

# A joy_types::sequence is just an arrayref, so it is fair to use
# any of the normal perlish array operations on it.

sub new {
  my $class = shift;
  my $self = shift // [];
  bless $self, $class;
  return $self;
}

sub name {
  return 'sequence';
}

sub desc {
  return "sequence<...>";
}

sub toStr {
  my $result = '[ ';
  for my $e (@{$_[0]}) {
    $result .= $e->toStr() . ' ';
  }
  $result .= ']';
}

sub key {
  return undef;
}

sub val {
  my $self = shift;
  return $self;
}

# ========================================================================
package joy_types::primitive;
use parent 'joy_types';

sub new {
  my $class = shift;
  my $name = shift;
  my $fn = shift;

  my $self = [$name, $fn];
  bless $self, $class;
  return $self;
}

sub name {
  return 'primitive';
}

sub toStr {
  return $_[0]->[0];
}

sub val {
  return '<fn>';
}

sub eval {
  my ($self, $stack) = @_;
  my ($name, $fn) = @$self;

  $fn->($stack);
}

sub key {
  return $_[0]->[0];
}

1;
