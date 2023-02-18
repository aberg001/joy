package joy_function;

use v5.24.1;
use lib ".";
use joy_types;

# ========================================================================
# We're making $STACK a package variable which we will use 'local' to
# make dynamic. This way we can access it from any functions in this
# package without having to pass it as an argument everywhere.
# ========================================================================

our $STACK;

sub init_fns {
  my $namespace = shift;

  init_math($namespace);
}

sub init_math {
  my $namespace = shift;

  init_table($namespace, [
      _BINOP('+', sub { return $_[0] + $_[1]; }),
      _BINOP('-', sub { return $_[0] - $_[1]; }),
      _BINOP('*', sub { return $_[0] * $_[1]; }),
      _BINOP('/', sub { return $_[0] / $_[1]; }),
    ]);
}

sub init_table {
  my $namespace = shift;
  my $tab = shift;

  foreach my $f (@$tab) {
    my ($name, $fn) = @$f;
    $namespace->set($name, joy_types::primitive->new($name, $fn));
  }
}

sub _BINOP {
  my $name = shift;
  my $fn = shift;

  return [$name, sub {
      local $STACK = shift;
      _PUSH($fn->(_POP(), _POP()));
    }];
}

sub _POP {
  return pop @$STACK;
}

sub _PUSH {
  push @$STACK, @_;
}

1;
