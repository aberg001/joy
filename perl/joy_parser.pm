package joy_parser;

use v5.24.1;
use lib ".";
use Marpa::R2;
use JSON;

use joy_types;

my $json = JSON->new->allow_nonref->pretty;

my $dsl = <<'END_OF_DSL';
:default ::= action => [name,values]
:discard ~ whitespace

lexeme default = latm => 1

Script ::= Statement *
Statement ::= 
  Definition
  | Function action => do_funcall
Definition ::= Symbol '=' Function action => do_definition
Function ::= Atom | Sequence
Sequence ::= 
  '[' Atoms ']' action => do_sequence
  | '[]' action => do_empty
Atoms ::= Atom*
Atom ::= Number | Symbol | String | Sequence
Number ::= IntNumber | FloatNumber
IntNumber ::= digits action => do_int
FloatNumber ::=  digits '.' digits action => do_float
Symbol ::= SingleSymbol action => do_symbol
SingleSymbol ::= symbolHead symbolBody
String ::= '"' NotDoubleQuotes '"'
  | singleQuote NotSingleQuotes singleQuote
NotDoubleQuotes ::= notDoubleQuote*
NotSingleQuotes ::= notSingleQuote*

whitespace ~ [\s]+
notDoubleQuote ~ [^"]
notSingleQuote ~ [^']
singleQuote ~ [']
digits ~ [\d]+
symbolHead ~ [^\x{5b}\x{5d}\s"'\d]
symbolBody ~ [^\x{5b}\x{5d}\s"']*
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
  $self->{grammar} = Marpa::R2::Scanless::G->new( { source => \$dsl, } );
  $self->{recce} = Marpa::R2::Scanless::R->new( {
      grammar => $self->{grammar},
      semantics_package => 'My_Actions',
    } );
}


sub parse {
  my $self = shift;
  my $input = shift;
  $self->{symbols} = shift;
  
  $self->{input} = $input;
  $self->{used} = $self->{recce}->read(\$input);
  $self->{result} = $self->{recce}->value();

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

sub My_Actions::do_funcall {
  say('funcall', $json->encode(\@_));
  my $h = shift;
  my $a = shift;
  return ' ';
}

sub My_Actions::do_symbol {
  say('symbol', $json->encode(\@_));
  my $h = shift;
  my $a = shift;
  return ' ';
}

sub My_Actions::do_int {
  say('number', $json->encode(\@_));
  return 'a number';
}

sub My_Actions::do_float {
  say('number', $json->encode(\@_));
  return 'a number';
}

sub My_Actions::do_definition {
  say('definition', $json->encode(\@_));
  my (undef, $t1, undef, $t2 ) = @_;
  return $t1 + $t2;
}

sub My_Actions::do_sequence {
  say('sequence', $json->encode(\@_));
  my (undef, $t1, undef, $t2 ) = @_;
  return $t1 + $t2;
}

sub My_Actions::do_empty {
  say('empty', $json->encode(\@_));
  return [];
}

1;
