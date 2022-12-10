package joy_parser;

use v5.24.1;
use lib ".";
use Marpa::R2;

use joy_types;

my $dsl = <<'END_OF_DSL';
:default ::= action => [name,values]
:discard ~ whitespace

lexeme default = latm => 1

Script ::= Statement *
Statement ::= 
  Definition
  | Function action => ::do_function
Definition ::= Symbol '=' Function action => ::do_definition
Function ::= Atom | Sequence
Sequence ::= 
  '[' Atoms ']'
  | '[]' action => ::do_sequence
Atoms ::= Atom*
Atom ::= Number | Symbol | String | Sequence
String ::= '"' NotDoubleQuotes '"'
  | singleQuote NotSingleQuotes singleQuote
NotDoubleQuotes ::= notDoubleQuote*
NotSingleQuotes ::= notSingleQuote*

whitespace ~ [\s]+
notDoubleQuote ~ [^"]
notSingleQuote ~ [^']
singleQuote ~ [`]
END_OF_DSL

sub new {
  my $class = shift;

  my $self = {};
  bless $self, $class;

  $self->_init(@_);

  return $self;
}

sub _init {
  my $self = shift;
  my $options = shift;

  $self->{options} = $options;
  $self->{input} = '';
  $self->{used} = 0;
  $self->{sequences} = [[]];
  $self->{symbols};
  $self->{result} = undef;
  $self->{error} = undef;
  $self->{grammar} = Marpa::R2::Scanless::G->new( { source => \$dsl } );
}


sub parse {
  my $self = shift;
  my $text = shift;
  my $symbols = shift;

  return not $self->{error};
}

sub prog {
  my $self = shift;

  return $self->{result};
}

sub error {
  my $self = shift;

  return $self->{error};
}


1;
