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
  $self->{root} = $self->named_namespace('JOY');
  $self->{current} = $self->named_namespace('USER');
  $self->{contexts} = [ $self->{root} ];
}

sub enter_namespace {
  my $self = shift;
  my $name = shift;

  push @{$self->{contexts}}, $self->{current};
  $self->change_namespace($name);
}

sub exit_namespace {
  my $self = shift;

  $self->{current} = pop @{$self->{contexts}};
}

sub change_namespace {
  my $self = shift;
  my $name = shift;

  $self->{current} = $self->named_namespace($name);
}

sub named_namespace {
  my $self = shift;
  my $name = shift;

  return $self->{namespaces}->{$name} if exists $self->{namespaces}->{$name};

  my $new = joy_namespace->new();
  $self->{namespaces}->{$name} = $new;
  return $new;

}

sub current_namespace {
  my $self = shift;

  return $self->{current};
}

sub get_symbol {
  my $self = shift;
  my $name = shift;

  my $namespace;
  my ($ns, $sym) = $self->_split($name);
  if (defined $ns) {
    $namespace = $self->named_namespace($ns);
  } else {
    $namespace = $self->current_namespace();
  }
  return $namespace->get($sym);
}

sub set_symbol {
  my $self = shift;
  my $name = shift;
  my $val = shift;

  my $sym = $self->get_symbol($name);
  return 0 if $sym->val();
  $sym->set($val)
}

sub _split {
  my $self = shift;
  my $name = shift;

  my @res = $name =~ /^(.+):(.+)$/;
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

sub set {
  my $self = shift;
  my $name = shift;
  my $val = shift;

  return undef if exists $self->{symbols}->{$name};
  my $new = joy_symbol->new($name);
  $new->set($val);
  return $new;
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
