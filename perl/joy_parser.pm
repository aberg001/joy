package joy_parser;

use v5.24.1;
use lib ".";
use joy_types;
use joy_symtab;

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
}

sub parse {
  my $self = shift;
  my $text = shift;
  my $symbols = shift;

  say("parse()");
  $self->{input} = $text;
  $self->{used} = 0;
  $self->{symbols} = $symbols;
  $self->{sequences} = [[]];
  $self->_script();
  $self->{result} = joy_types::sequence->new($self->{sequences}[0]);
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

=pod
EBNF-like Description

script ::= { discard | statement } *
statement ::= definition | funcall
funcall ::= sequence | atom 
definition ::= symbol discard '=' { discard | statement } + '.'
sequence ::= '[' { discard | funcall } * ']'
atom ::= symbol | number | string
discard ::= whitespace | comment

symbol ::= /[^\[\]"=.\d][^\[\]"=]*/
number ::= /\d+(\.\d+)?/
string ::= /"([^"]|"")*"/
comment ::= /--.*?\n/
whitespace ::= /[\s]*/

=cut

## #################################
## Parse Functions
## #################################

sub _script {
  my $self = shift;

  say "_script.begin";
  while ($self->_discard() or $self->_statement()) { }
  say "_script.end";
  return 1;
}

sub _statement {
  my $self = shift;

  return ($self->_definition() or $self->_funcall());
}

sub _funcall {
  my $self = shift;

  return ($self->_sequence() or $self->_atom());
}

sub _definition {
  my $self = shift;

  my $used = $self->{used};
  my $success = 0;
  eval {
    my @seq = ();
    unshift @{$self->{sequences}}, \@seq;
    my $symbol = $self->_symbol() or die;
    $self->_discard();
    $self->_consume('=') or die;
    $self->_discard() or $self->_statement() or die;
    while ($self->_discard() or $self->_statement()) { }
    $self->_consume('.') or die;
    shift @{$self->{sequences}};
    my $name = shift @seq;
    $self->{namespace}->set_symbol($name, \@seq) or die;
    $success = 1;
  };
  $self->{used} = $used unless $success;
  shift @{$self->{sequences}} unless $success;
  return $success;
}

sub _sequence {
  my $self = shift;

  my $used = $self->{used};
  my $success = 0;
  eval {
    $self->_consume('[') or die;
    my $val = [];
    unshift @{$self->{sequences}}, $val;
    while ($self->_discard() or $self->_funcall()) { }
    $self->_consume(']') or die;
    shift @{$self->{sequences}};
    $self->_append(joy_types::sequence->new($val));
    $success = 1;
  };
  $self->{used} = $used unless $success;
  shift @{$self->{sequence}} unless $success;
  return $success;
}

sub _atom {
  # takes an atom from the input, and adds it to the current sequence.
  my $self = shift;

  return ($self->_symbol() or $self->_number() or $self->_string());
}

sub _discard {
  my $self = shift;

  return ($self->_whitespace() or $self->_comment());
}

sub _number {
  # looks for a number and adds it to the current sequence.
  my $self = shift;

  if (my $val = $self->_match(qr(^\d+(\.\d+)?))) {
    $self->_append(joy_types::number->new($val));
    return 1;
  }
  return 0;
}

sub _symbol {
  # looks for a symbol and adds it to the current sequence.
  my $self = shift;

  say '_symbol?';
  if (my $val = $self->_match(qr(^[^\d\s\[\]"=.][^\s\[\]=]*))) {
    $self->_append(joy_types::symbol->new($val, $self->{symtab}->get_symbol($val)));
    return 1;
  }
  return 0;
}

sub _string {
  # looks for a string and adds it to the current sequence.
  my $self = shift;

  if (my $val = $self->_match(qr(^"([^"]|"")*"))) {
    $self->_append(joy_types::string->new($val));
    return 1;
  }
  return 0;
}

sub _whitespace {
  # looks for one or more whitespace characters and discards them.
  my $self = shift;

  return $self->_match(qr(^\s+));
}

sub _comment {
  # looks for a comment and discards it.
  my $self = shift;

  return $self->_match(qr(^--.*?\n));
}

## #################################
## Utility Functions
## #################################

sub _append {
  # takes a value of some sort and adds it to the current constructing
  # seuqence.
  my $self = shift;
  my $val = shift;

  say "_append('".$val->desc()."')";
  push @{$self->{sequences}->[0]}, $val;
}

sub _matchahead {
  # takes a regular expression, and if it matches against the current
  # point of the input returns true.
  my $self = shift;
  my $re = shift;

  my $rest = substr($self->{input}, $self->{used});
  return scalar($rest =~ $re);
}

sub _match {
  # takes a regular expression, and if it matches against the current
  # point of the input it removes the match from the input and returns
  # it.  Returns false (empty string: '') otherwise.
  my $self = shift;
  my $re = shift;

  my $rest = substr($self->{input}, $self->{used});
  return '' unless $rest =~ $re;
  my $result = substr($rest, $-[0], $+[0] - $-[0]);
  # my $old_used = $self->{used};
  $self->{used} += $+[0] - $-[0];
  # say sprintf "match success, '%s' ('%s'+%d->%d) +[%d] -[%d]", $result, $rest, $old_used, $self->{used}, $+[0], $-[0];
  return $result;
}

sub _lookahead {
  # takes a string, returns true if it matches the current point.
  my $self = shift;
  my $str = shift;
  my $len = length($str);
  return $str eq substr($self->{input}, $self->{used}, $len);
}

sub _consume { 
  #  takes a string, and if it matches the current point of the input
  #  it removes it from the input.  Returns true if matched, false
  #  otherwise.
  my $self = shift;
  my $str = shift;
  my $len = length($str);
  
  my $init = substr($self->{input}, $self->{used}, $len);
  return 0 if $init ne $str;
  $self->{used} += $len;
  return 1;
}

sub __shallowCopy {
  my $h = shift;
  my $result = {};
  for (keys %$h) { $result->{$_} = $h->{$_} };
  return $result;
}

1;
