package joy_parser;

use v5.24.1;
use lib ".";
use joy_types;

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

  print("parse()\n");
  $self->{input} = $text;
  $self->{used} = 0;
  $self->{symbols} = $symbols;
  $self->_script();
  $self->{result} = $self->{sequences}[0];
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

sub _script {
  my $self = shift;

  while ($self->_sequence() or $self->_atom() or $self->_whitespace() or $self->_comment()) {
    1;
  }
}

sub _sequence {
  my $self = shift;

  my $used = $self->{used};
  return 0 unless $self->_consume('[');
  unshift @{$self->{sequences}}, [];
  $self->_script();
  my $val = shift @{$self->{sequences}};
  unless ($self->_consume(']')) {
    $self->{used} = $used;
    return 0;
  }
  $self->_append(joy_types::sequence->new($val));
  return 1;
}

sub _atom {
  # takes an atom from the input, and adds it to the current sequence.
  my $self = shift;

  return $self->_symbol() or $self->_number() or $self->_string();
}

sub _number {
  # looks for a number and adds it to the current sequence.
  my $self = shift;

  if (my $val = $self->_match(qr(\d+(\.\d+)?))) {
    $self->_append(joy_types::number->new($val));
    return 1;
  }
  return 0;
}

sub _symbol {
  # looks for a symbol and adds it to the current sequence.
  my $self = shift;

  if (my $val = $self->_match(qr([\D\S]+))) {
    $self->_append(joy_types::symbol->new([$val, $self->{symtab}{$val}]));
    return 1;
  }
  return 0;
}

sub _string {
  # looks for a string and adds it to the current sequence.
  my $self = shift;

  if (my $val = $self->_match(qr("[^"]*"))) {
    $self->_append(joy_types::string->new($val));
    return 1;
  }
  return 0;
}

sub _whitespace {
  # looks for one or more whitespace characters and discards them.
  my $self = shift;

  return $self->_match(qr(\s+));
}

sub _comment {
  # looks for a comment and discards it.
  my $self = shift;

  return $self->_match(qr(--.*\n));
}

sub _append {
  # takes a value of some sort and adds it to the current constructing
  # seuqence.
  my $self = shift;
  my $val = shift;

  push @{$self->{sequences}}, $val;
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
  my $result = substr($rest, $-[0], $+[0]-$-[0]);
  $self->_consume($result);
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
  $self->{used} -= $len;
  return 1;
}

sub __shallowCopy {
  my $h = shift;
  my $result = {};
  for (keys %$h) { $result->{$_} = $h->{$_} };
  return $result;
}

1;
