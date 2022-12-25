package joy;

use v5.24.1;
use lib ".";
use joy_parser;

sub new {
  my $class = shift;

  my $self = {};
  bless $self, $class;

  $self->_init(@_);

  return $self;
}

sub repl {
  my $self = shift;
  use Term::ReadLine;

  my $term = Term::ReadLine->new('Joy REPL');
  while ( defined ($_ = term->readline('j>')) ) {
    # Parse and eval $_, print the stack top.
    print "got: $_";
  }
}

sub eval {
  my $self = shift;
  my $text= shift;

  $self->parse($text);
  $self->run();
}

sub parse {
  my $self = shift;
  my $text = shift;

  print("parse… [$text]\n");
  my $parser = joy_parser->new();
  if ($parser->parse($text, $self->{symbols})) {
    $self->{prog} = $parser->prog();
    return 1;
  } else {
    return 0;
  }
}

sub print {
  my $self = shift;

  $self->{prog}->print();
}

sub run {
  my $self = shift;
  my $prog = shift;

  print("run…\n");
  $self->{prog} = $prog;
  $self->_eval();
  print("…run\n");
  1;
}

sub _init {
  my $self = shift;

  $self->{symbols} = {};
  $self->{prog} = undef;
  $self->{stack} = [];
  $self->{step} = undef;

  $self->_init_fns()
}

sub _init_fns {
  ## The symbols is a map from 'symbol' => [\'symbol', «joy_types::sequence | perl fn ref»].
  my $self = shift;
  my $fns = $self->{symbols};
}

sub _eval {
  my $self = shift;
  print("eval…\n");
  print("…eval\n");
}

1;
