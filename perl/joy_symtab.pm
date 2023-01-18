package joy_symtab;

use v5.24.1;
use lib ".";

sub new {
  my $class = shift;

  my $self = {};
  bless $self, $class;

  $self->_init(@_);

  return $self;
}

sub _init {
  my $self = shift;

  $self->{namespaces} = {};
  $self->{current} = $self->get('');
}

sub get_namespace {
  my $self = shift;
  my $name = shift;

  unless (exists $self->{namespaces}) {
    my $new = joy_namespace->new();
    $self->{namespaces}->{$name} = $new;
    return $new;
  }

  return $self->{namespaces}->{$name};
}

sub get_symbol {
  my $self = shift;
  my $name = shift;

  my ($namespace, $symbol) = $self->_split($name);
  
}

sub _split {
  my $self = shift;
  my $name = shift;

  my @res = $name =~ /(.+):(.+)/;
  return @res if @res;
  return (undef, $name);
}

package joy_namespace;

sub new {
  my $class = shift;

  my $self = {};
  bless $self, $class;

  $self->_init(@_);

  return $self;
}

sub _init {
  my $self = shift;

  $self->{symbols} = {};
}

sub get {
  my $self = shift;
  my $name = shift;

  unless (exists $self->{symbols}->{$name}) {
    my $new = joy_symbol->new();
    $self->{symbols}->{$name} = $new;
    return $new;
  }

  return $self->{symbols}->{$name};
}

package joy_symbol;

sub new {
  my $class = shift;
  my $name = shift;

  my $self = [$name, undef];
  bless $self, $class;

  return $self;
}

sub name {
  return $_[0]->[0];
}

sub val {
  return $_[0]->[1];
}

sub set {
  $_[0]->[1] = $_[1];
}

1;
