package Term::Cap;

# Since the debugger uses Term::ReadLine which uses Term::Cap, we want
# to load as few modules as possible.  This includes Carp.pm.
sub carp
{
    require Carp;
    goto &Carp::carp;
}

sub croak
{
    require Carp;
    goto &Carp::croak;
}

use strict;

use vars qw($VERSION $VMS_TERMCAP);

$VERSION = '1.18';

# TODO:
# support Berkeley DB termcaps
# force $FH into callers package?
# keep $FH in object at Tgetent time?

=head1 NAME

Term::Cap - Perl termcap interface

=head1 SYNOPSIS

    require Term::Cap;
    $terminal = Term::Cap->Tgetent({ TERM => undef, OSPEED => $ospeed });
    $terminal->Trequire(qw/ce ku kd/);
    $terminal->Tgoto('cm', $col, $row, $FH);
    $terminal->Tputs('dl', $count, $FH);
    $terminal->Tpad($string, $count, $FH);

=head1 DESCRIPTION

These are low-level functions to extract and use capabilities from
a terminal capability (termcap) database.

More information on the terminal capabilities will be found in the
termcap manpage on most Unix-like systems.

=head2 METHODS

The output strings for B<Tputs> are cached for counts of 1 for performance.
B<Tgoto> and B<Tpad> do not cache.  C<$self-E<gt>{_xx}> is the raw termcap
data and C<$self-E<gt>{xx}> is the cached version.

    print $terminal->Tpad($self->{_xx}, 1);

B<Tgoto>, B<Tputs>, and B<Tpad> return the string and will also
output the string to $FH if specified.


=cut

# Preload the default VMS termcap.
# If a different termcap is required then the text of one can be supplied
# in $Term::Cap::VMS_TERMCAP before Tgetent is called.

if ( $^O eq 'VMS' )
{
    chomp( my @entry = <DATA> );
    $VMS_TERMCAP = join '', @entry;
}

# Returns a list of termcap files to check.

sub termcap_path
{    ## private
    my @termcap_path;

    # $TERMCAP, if it's a filespec
    push( @termcap_path, $ENV{TERMCAP} )
      if (
        ( exists $ENV{TERMCAP} )
        && (
            ( $^O eq 'os2' || $^O eq 'MSWin32' || $^O eq 'dos' )
            ? $ENV{TERMCAP} =~ /^[a-z]:[\\\/]/is
            : $ENV{TERMCAP} =~ /^\//s
        )
      );
    if ( ( exists $ENV{TERMPATH} ) && ( $ENV{TERMPATH} ) )
    {

        # Add the users $TERMPATH
        push( @termcap_path, split( /:|\s+/, $ENV{TERMPATH} ) );
    }
    else
    {

        # Defaults
        push( @termcap_path,
            exists $ENV{'HOME'} ? $ENV{'HOME'} . '/.termcap' : undef,
            '/etc/termcap', '/usr/share/misc/termcap', );
    }

    # return the list of those termcaps that exist
    return grep { defined $_ && -f $_ } @termcap_path;
}

=over 4

=item B<Tgetent>

Returns a blessed object reference which the user can
then use to send the control strings to the terminal using B<Tputs>
and B<Tgoto>.

The function extracts the entry of the specified terminal
type I<TERM> (defaults to the environment variable I<TERM>) from the
database.

It will look in the environment for a I<TERMCAP> variable.  If
found, and the value does not begin with a slash, and the terminal
type name is the same as the environment string I<TERM>, the
I<TERMCAP> string is used instead of reading a termcap file.  If
it does begin with a slash, the string is used as a path name of
the termcap file to search.  If I<TERMCAP> does not begin with a
slash and name is different from I<TERM>, B<Tgetent> searches the
files F<$HOME/.termcap>, F</etc/termcap>, and F</usr/share/misc/termcap>,
in that order, unless the environment variable I<TERMPATH> exists,
in which case it specifies a list of file pathnames (separated by
spaces or colons) to be searched B<instead>.  Whenever multiple
files are searched and a tc field occurs in the requested entry,
the entry it names must be found in the same file or one of the
succeeding files.  If there is a C<:tc=...:> in the I<TERMCAP>
environment variable string it will continue the search in the
files as above.

The extracted termcap entry is available in the object
as C<$self-E<gt>{TERMCAP}>.

It takes a hash reference as an argument with two optional keys:

=over 2

=item OSPEED

The terminal output bit rate (often mistakenly called the baud rate)
for this terminal - if not set a warning will be generated
and it will be defaulted to 9600.  I<OSPEED> can be specified as
either a POSIX termios/SYSV termio speeds (where 9600 equals 9600) or
an old DSD-style speed ( where 13 equals 9600).


=item TERM

The terminal type whose termcap entry will be used - if not supplied it will
default to $ENV{TERM}: if that is not set then B<Tgetent> will croak.

=back

It calls C<croak> on failure.

=cut

sub Tgetent
{    ## public -- static method
    my $class = shift;
    my ($self) = @_;

    $self = {} unless defined $self;
    bless $self, $class;

    my ( $term, $cap, $search, $field, $tmp_term, $TERMCAP );
    my ( $state, $first, $entry );
    local $_;

    # Compute PADDING factor from OSPEED (to be used by Tpad)
    if ( !$self->{OSPEED} )
    {
        if ($^W)
        {
            carp "OSPEED was not set, defaulting to 9600";
        }
        $self->{OSPEED} = 9600;
    }
    if ( $self->{OSPEED} < 16 )
    {

        # delays for old style speeds
        my @pad = (
            0,    200, 133.3, 90.9, 74.3, 66.7, 50, 33.3,
            16.7, 8.3, 5.5,   4.1,  2,    1,    .5, .2
        );
        $self->{PADDING} = $pad[ $self->{OSPEED} ];
    }
    else
    {
        $self->{PADDING} = 10000 / $self->{OSPEED};
    }

    unless ( $self->{TERM} )
    {
       if ( $ENV{TERM} )
       {
         $self->{TERM} =  $ENV{TERM} ;
       }
       else
       {
          if ( $^O eq 'MSWin32' )
          {
             $self->{TERM} =  'dumb';
          }
          else
          {
             croak "TERM not set";
          }
       }
    }

    $term = $self->{TERM};    # $term is the term type we are looking for

    # $tmp_term is always the next term (possibly :tc=...:) we are looking for
    $tmp_term = $self->{TERM};
    my $seen = {};

    if (exists $ENV{TERMCAP}) {
    	local $_ = $ENV{TERMCAP};
	if ( !m:^/:s && m/(^|\|)\Q$tmp_term\E[:|]/s ) {
	    $entry = $_;
	    $seen->{$tmp_term} = 1;
	}
    }

    my @termcap_path = termcap_path();

    $state = 1;    # 0 == finished
                   # 1 == next file
                   # 2 == search again
		   # 3 == try infocmp

    $first = 0;    # first entry (keeps term name)

    if ($entry)
    {

        # ok, we're starting with $TERMCAP
        $first++;    # we're the first entry
                     # do we need to continue?
        if ( $entry =~ s/:tc=([^:]+):/:/ )
        {
            $tmp_term = $1;
        }
        else
        {
            $state = 0;    # we're already finished
        }
    }


    while ( $state != 0 )
    {
        if ( $state == 1 ) {
            # get the next TERMCAP
            $TERMCAP = shift @termcap_path or $state = 3;
	} elsif ($state == 3) {
	    croak "failed termcap lookup on $tmp_term";
        } else {
            # do the same file again
            $state = 1;    # ok, maybe do a new file next time
        }

	my ($fh, $child);
	if ($state == 3) {
	    # need to do a proper fork, so that we can pass tmp_term
	    # without having to quote it.
	    $child = open($fh, "-|");
	    warn "cannot run infocmp: $!" if !defined $child;
	    if (!$child) {
	    	open(STDERR, ">", "/dev/null");
		exec('infocmp', '-CTrx', '--', $tmp_term);
		exit(1);
	    }
	} else {
	    open($fh, '<', $TERMCAP) || croak "open $TERMCAP: $!";
	}
	while (<$fh>) {
	    next if /^\t/ || /^#/;
	    if (m/(^|\|)\Q$tmp_term\E[:|]/) {
		chomp;
		s/^[^:]*:// if $first++;
		$state = 0;
		$seen->{$tmp_term} = 1;
		while (s/\\$//) {
		    defined(my $x = <$fh>) or last;
		    $_ .= $x; chomp;
		}
		if (defined $entry) {
		    $entry .= $_;
		} else {
		    $entry = $_;
		}
		last;
	    }
	}
        close $fh;
	waitpid($child, 0) if defined $child;

	next if $state != 0;

        # If :tc=...: found then search this file again
	while ($entry =~ s/:tc=([^:]+):/:/) {
	    $tmp_term = $1;
	    next if $seen->{$tmp_term};
	    $state = 2;
	    last;
	}
    }

    if ( !defined $entry ) {
        if ( $^O eq 'VMS' ) {
            $entry = $VMS_TERMCAP;
       # this is getting desperate now
        } elsif ( $self->{TERM} eq 'dumb' ){
	  $entry = 'dumb|80-column dumb tty::am::co#80::bl=^G:cr=^M:do=^J:sf=^J:';
	}
    }

    croak "Can't find $term" if !defined $entry;
    $entry =~ s/:+\s*:+/:/g;    # cleanup $entry
    $entry =~ s/:+/:/g;         # cleanup $entry
    $self->{TERMCAP} = $entry;  # save it
                                # print STDERR "DEBUG: $entry = ", $entry, "\n";

    # Precompile $entry into the object
    $entry =~ s/^[^:]*://;
    foreach $field ( split( /:[\s:\\]*/, $entry ) )
    {
        if ( defined $field && $field =~ /^(\w{2,})$/ )
        {
            $self->{ '_' . $field } = 1 unless defined $self->{ '_' . $1 };

            # print STDERR "DEBUG: flag $1\n";
        }
        elsif ( defined $field && $field =~ /^(\w{2,})\@/ )
        {
            $self->{ '_' . $1 } = "";

            # print STDERR "DEBUG: unset $1\n";
        }
        elsif ( defined $field && $field =~ /^(\w{2,})#(.*)/ )
        {
            $self->{ '_' . $1 } = $2 unless defined $self->{ '_' . $1 };

            # print STDERR "DEBUG: numeric $1 = $2\n";
        }
        elsif ( defined $field && $field =~ /^(\w{2,})=(.*)/ )
        {

            # print STDERR "DEBUG: string $1 = $2\n";
            next if defined $self->{ '_' . ( $cap = $1 ) };
            $_ = $2;
            if ( ord('A') == 193 )
            {
               s/\\E/\047/g;
               s/\\(\d\d\d)/pack('c',oct($1) & 0177)/eg;
               s/\\n/\n/g;
               s/\\r/\r/g;
               s/\\t/\t/g;
               s/\\b/\b/g;
               s/\\f/\f/g;
               s/\\\^/\337/g;
               s/\^\?/\007/g;
               s/\^(.)/pack('c',ord($1) & 31)/eg;
               s/\\(.)/$1/g;
               s/\337/^/g;
            }
            else
            {
               s/\\E/\033/g;
               s/\\(\d\d\d)/pack('c',oct($1) & 0177)/eg;
               s/\\n/\n/g;
               s/\\r/\r/g;
               s/\\t/\t/g;
               s/\\b/\b/g;
               s/\\f/\f/g;
               s/\\\^/\377/g;
               s/\^\?/\177/g;
               s/\^(.)/pack('c',ord($1) & 31)/eg;
               s/\\(.)/$1/g;
               s/\377/^/g;
            }
            $self->{ '_' . $cap } = $_;
        }

        # else { carp "junk in $term ignored: $field"; }
    }
    $self->{'_pc'} = "\0" unless defined $self->{'_pc'};
    $self->{'_bc'} = "\b" unless defined $self->{'_bc'};
    $self;
}

# $terminal->Tpad($string, $cnt, $FH);

=item B<Tpad>

Outputs a literal string with appropriate padding for the current terminal.

It takes three arguments:

=over 2

=item B<$string>

The literal string to be output.  If it starts with a number and an optional
'*' then the padding will be increased by an amount relative to this number,
if the '*' is present then this amount will be multiplied by $cnt.  This part
of $string is removed before output/

=item B<$cnt>

Will be used to modify the padding applied to string as described above.

=item B<$FH>

An optional filehandle (or IO::Handle ) that output will be printed to.

=back

The padded $string is returned.

=cut

sub Tpad
{    ## public
    my $self = shift;
    my ( $string, $cnt, $FH ) = @_;
    my ( $decr, $ms );

    if ( defined $string && $string =~ /(^[\d.]+)(\*?)(.*)$/ )
    {
        $ms = $1;
        $ms *= $cnt if $2;
        $string = $3;
        $decr   = $self->{PADDING};
        if ( $decr > .1 )
        {
            $ms += $decr / 2;
            $string .= $self->{'_pc'} x ( $ms / $decr );
        }
    }
    print $FH $string if $FH;
    $string;
}

# $terminal->Tputs($cap, $cnt, $FH);

=item B<Tputs>

Output the string for the given capability padded as appropriate without
any parameter substitution.

It takes three arguments:

=over 2

=item B<$cap>

The capability whose string is to be output.

=item B<$cnt>

A count passed to Tpad to modify the padding applied to the output string.
If $cnt is zero or one then the resulting string will be cached.

=item B<$FH>

An optional filehandle (or IO::Handle ) that output will be printed to.

=back

The appropriate string for the capability will be returned.

=cut

sub Tputs
{    ## public
    my $self = shift;
    my ( $cap, $cnt, $FH ) = @_;
    my $string;

    $cnt = 0 unless $cnt;

    if ( $cnt > 1 )
    {
        $string = Tpad( $self, $self->{ '_' . $cap }, $cnt );
    }
    else
    {

        # cache result because Tpad can be slow
        unless ( exists $self->{$cap} )
        {
            $self->{$cap} =
              exists $self->{"_$cap"}
              ? Tpad( $self, $self->{"_$cap"}, 1 )
              : undef;
        }
        $string = $self->{$cap};
    }
    print $FH $string if $FH;
    $string;
}

# $terminal->Tgoto($cap, $col, $row, $FH);

=item B<Tgoto>

B<Tgoto> decodes a cursor addressing string with the given parameters.

There are four arguments:

=over 2

=item B<$cap>

The name of the capability to be output.

=item B<$col>

The first value to be substituted in the output string ( usually the column
in a cursor addressing capability )

=item B<$row>

The second value to be substituted in the output string (usually the row
in cursor addressing capabilities)

=item B<$FH>

An optional filehandle (or IO::Handle ) to which the output string will be
printed.

=back

Substitutions are made with $col and $row in the output string with the
following sprintf() line formats:

 %%   output `%'
 %d   output value as in printf %d
 %2   output value as in printf %2d
 %3   output value as in printf %3d
 %.   output value as in printf %c
 %+x  add x to value, then do %.

 %>xy if value > x then add y, no output
 %r   reverse order of two parameters, no output
 %i   increment by one, no output
 %B   BCD (16*(value/10)) + (value%10), no output

 %n   exclusive-or all parameters with 0140 (Datamedia 2500)
 %D   Reverse coding (value - 2*(value%16)), no output (Delta Data)

The output string will be returned.

=cut

sub Tgoto
{    ## public
    my $self = shift;
    my ( $cap, $code, $tmp, $FH ) = @_;
    my $string = $self->{ '_' . $cap };
    my $result = '';
    my $after  = '';
    my $online = 0;
    my @tmp    = ( $tmp, $code );
    my $cnt    = $code;

    while ( $string =~ /^([^%]*)%(.)(.*)/ )
    {
        $result .= $1;
        $code   = $2;
        $string = $3;
        if ( $code eq 'd' )
        {
            $result .= sprintf( "%d", shift(@tmp) );
        }
        elsif ( $code eq '.' )
        {
            $tmp = shift(@tmp);
            if ( $tmp == 0 || $tmp == 4 || $tmp == 10 )
            {
                if ($online)
                {
                    ++$tmp, $after .= $self->{'_up'} if $self->{'_up'};
                }
                else
                {
                    ++$tmp, $after .= $self->{'_bc'};
                }
            }
            $result .= sprintf( "%c", $tmp );
            $online = !$online;
        }
        elsif ( $code eq '+' )
        {
            $result .= sprintf( "%c", shift(@tmp) + ord($string) );
            $string = substr( $string, 1, 99 );
            $online = !$online;
        }
        elsif ( $code eq 'r' )
        {
            ( $code, $tmp ) = @tmp;
            @tmp = ( $tmp, $code );
            $online = !$online;
        }
        elsif ( $code eq '>' )
        {
            ( $code, $tmp, $string ) = unpack( "CCa99", $string );
            if ( $tmp[0] > $code )
            {
                $tmp[0] += $tmp;
            }
        }
        elsif ( $code eq '2' )
        {
            $result .= sprintf( "%02d", shift(@tmp) );
            $online = !$online;
        }
        elsif ( $code eq '3' )
        {
            $result .= sprintf( "%03d", shift(@tmp) );
            $online = !$online;
        }
        elsif ( $code eq 'i' )
        {
            ( $code, $tmp ) = @tmp;
            @tmp = ( $code + 1, $tmp + 1 );
        }
        else
        {
            return "OOPS";
        }
    }
    $string = Tpad( $self, $result . $string . $after, $cnt );
    print $FH $string if $FH;
    $string;
}

# $terminal->Trequire(qw/ce ku kd/);

=item B<Trequire>

Takes a list of capabilities as an argument and will croak if one is not
found.

=cut

sub Trequire
{    ## public
    my $self = shift;
    my ( $cap, @undefined );
    foreach $cap (@_)
    {
        push( @undefined, $cap )
          unless defined $self->{ '_' . $cap } && $self->{ '_' . $cap };
    }
    croak "Terminal does not support: (@undefined)" if @undefined;
}

=back

=head1 EXAMPLES

    use Term::Cap;

    # Get terminal output speed
    require POSIX;
    my $termios = POSIX::Termios->new;
    $termios->getattr;
    my $ospeed = $termios->getospeed;

    # Old-style ioctl code to get ospeed:
    #     require 'ioctl.pl';
    #     ioctl(TTY,$TIOCGETP,$sgtty);
    #     ($ispeed,$ospeed) = unpack('cc',$sgtty);

    # allocate and initialize a terminal structure
    my $terminal = Term::Cap->Tgetent({ TERM => undef, OSPEED => $ospeed });

    # require certain capabilities to be available
    $terminal->Trequire(qw/ce ku kd/);

    # Output Routines, if $FH is undefined these just return the string

    # Tgoto does the % expansion stuff with the given args
    $terminal->Tgoto('cm', $col, $row, $FH);

    # Tputs doesn't do any % expansion.
    $terminal->Tputs('dl', $count = 1, $FH);

=head1 COPYRIGHT AND LICENSE

Copyright 1995-2015 (c) perl5 porters.

This software is free software and can be modified and distributed under
the same terms as Perl itself.

Please see the file README in the Perl source distribution for details of
the Perl license.

=head1 AUTHOR

This module is part of the core Perl distribution and is also maintained
for CPAN by Jonathan Stowe <jns@gellyfish.co.uk>.

The code is hosted on Github: https://github.com/jonathanstowe/Term-Cap
please feel free to fork, submit patches etc, etc there.

=head1 SEE ALSO

termcap(5)

=cut

# Below is a default entry for systems where there are terminals but no
# termcap
1;
__DATA__
vt220|vt200|DEC VT220 in vt100 emulation mode:
am:mi:xn:xo:
co#80:li#24:
RA=\E[?7l:SA=\E[?7h:
ac=kkllmmjjnnwwqquuttvvxx:ae=\E(B:al=\E[L:as=\E(0:
bl=^G:cd=\E[J:ce=\E[K:cl=\E[H\E[2J:cm=\E[%i%d;%dH:
cr=^M:cs=\E[%i%d;%dr:dc=\E[P:dl=\E[M:do=\E[B:
ei=\E[4l:ho=\E[H:im=\E[4h:
is=\E[1;24r\E[24;1H:
nd=\E[C:
kd=\E[B::kl=\E[D:kr=\E[C:ku=\E[A:le=^H:
mb=\E[5m:md=\E[1m:me=\E[m:mr=\E[7m:
kb=\0177:
r2=\E>\E[24;1H\E[?3l\E[?4l\E[?5l\E[?7h\E[?8h\E=:rc=\E8:
sc=\E7:se=\E[27m:sf=\ED:so=\E[7m:sr=\EM:ta=^I:
ue=\E[24m:up=\E[A:us=\E[4m:ve=\E[?25h:vi=\E[?25l:

