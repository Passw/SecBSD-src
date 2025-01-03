# $OpenBSD: Getopt.pm,v 1.14 2023/07/06 08:29:26 espie Exp $

# Copyright (c) 2012 Marc Espie <espie@openbsd.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

use v5.36;

package Option;
sub factory($class, $o)
{
	if ($o =~ m/^(.)$/) {
		return Option::Short->new($1);
	} elsif ($o =~ m/^(.)\:$/) {
		return Option::ShortArg->new($1);
	} elsif ($o =~ m/^(\-?.)(?:\:\!|\!\:)$/) {
		return Option::LongArg0->new($1);
	} elsif ($o =~ m/^(\-?.)\!$/) {
		return Option::Long->new($1);
	} elsif ($o =~ m/^(\-?.*)\=$/) {
		return Option::LongArg->new($1);
	} elsif ($o =~ m/^(\-?.*)\:$/) {
		return Option::LongArg0->new($1);
	} elsif ($o =~ m/^(\-?.*)$/) {
		return Option::Long->new($1);
	}
}

sub new($class, $v)
{
	bless \$v, $class;
}

sub setup($self, $opts, $isarray)
{
	$opts->add_option_accessor($$self, $isarray);
	return $self;
}

package Option::Short;
our @ISA = qw(Option);

sub match($self, $arg, $opts, $canonical, $code)
{
	if ($arg =~ m/^\-\Q$$self\E$/) {
		&$code($opts, $canonical, 1, $arg);
		return 1;
	}
	if ($arg =~ m/^(\-\Q$$self\E)(.*)$/) {
		unshift(@main::ARGV, "-$2");
		&$code($opts, $canonical, 1, $1);
		return 1;
	}
	return 0;
}

package Option::ShortArg;
our @ISA = qw(Option::Short);

sub match($self, $arg, $opts, $canonical, $code)
{
	if ($arg =~ m/^\-\Q$$self\E$/) {
		&$code($opts, $canonical, (shift @main::ARGV), $arg);
		return 1;
	}
	if ($arg =~ m/^(\-\Q$$self\E)(.*)$/) {
		&$code($opts, $canonical, $2, $1);
		return 1;
	}
	return 0;
}

package Option::Long;
our @ISA = qw(Option);

sub match($self, $arg, $opts, $canonical, $code)
{
	if ($arg =~ m/^\-\Q$$self\E$/) {
		&$code($opts, $canonical, 1, $arg);
		return 1;
	}
	return 0;
}

package Option::LongArg0;
our @ISA = qw(Option::Long);
sub match($self, $arg, $opts, $canonical, $code)
{
	if ($arg =~ m/^\-\Q$$self\E$/) {
		if (@main::ARGV > 0) {
			&$code($opts, $canonical, (shift @main::ARGV), $arg);
			return 1;
		} else {
			die "Missing argument  for option -$$self\n";
		}
	}
	return 0;
}

package Option::LongArg;
our @ISA = qw(Option::LongArg0);

sub match($self, $arg, $opts, $canonical, $code)
{
	if ($self->SUPER::match($arg, $opts, $canonical, $code)) {
		return 1;
	}
	if ($arg =~ m/^(-\Q$$self\E)\=(.*)$/) {
		&$code($opts, $canonical, $2, $1);
		return 1;
	}
	return 0;
}

package Option::Regexp;
sub new($class, $re, $code)
{
	bless {re => $re, code => $code}, $class;
}

sub setup($self, $, $)
{
	return $self;
}

sub match($self, $arg, $opts)
{
	if (my @l = ($arg =~ m/^$self->{re}$/)) {
		&{$self->{code}}(@l);
		return 1;
	} else {
		return 0;
	}
}

package Options;

sub new($class, $string, $code)
{
	if (ref($string) eq 'Regexp') {
		return Option::Regexp->new($string, $code);
	}
	my @alternates = split(/\|/, $string);

	bless {
		alt => [map { Option->factory($_); } @alternates],
		code => $code
	}, $class;
}

sub setup($self, $allopts, $isarray)
{
	$self->{alt}[0]->setup($allopts, $isarray);
	return $self;
}

sub match($self, $arg, $opts)
{

	my $canonical = ${$self->{alt}[0]};
	for my $s (@{$self->{alt}}) {
		if ($s->match($arg, $opts, $canonical, $self->{code})) {
			return 1;
		}
	}
	return 0;
}

# seems I spend my life rewriting option handlers, not surprisingly...
package LT::Getopt;
use LT::Util;


# parsing an option 'all-static' will automatically add an
# accessor $self->all_static   that maps to the option.

sub add_option_accessor($self, $option, $isarray)
{
	my $access = $option;
	$access =~ s/^\-//;
	$access =~ s/-/_/g;
	my $actual = $isarray ?
		sub($self) {
		    $self->{opt}{$option} //= [];
		    if (wantarray) {
			    return @{$self->{opt}{$option}};
		    } else {
			    return scalar @{$self->{opt}{$option}};
		    }
		} : sub($self) {
		    return $self->{opt}{$option};
		};
	my $callpkg = ref($self);
	unless ($self->can($access)) {
		no strict 'refs';
		*{$callpkg."::$access"} = $actual;
	}
}

sub create_options($self, @l)
{
	my @options = ();
	# first pass creates accessors
	push(@l, '-tag=', sub { $self->add_tag($_[2]); });
	while (my $opt = shift @l) {
		my $isarray = ($opt =~ s/\@$//);
		# default code or not
		my $code;
		if (@l > 0 && ref($l[0]) eq 'CODE') {
			$code = shift @l;
		} else {
			if ($isarray) {
				$code = sub {
				    my ($object, $canonical, $value) = @_;
				    push(@{$object->{opt}{$canonical}}, $value);
				};
			} else {
				$code = sub {
				    my ($object, $canonical, $value) = @_;
				    $object->{opt}{$canonical} = $value;
				};
			}
		}
		push(@options,
		    Options->new($opt, $code)->setup($self, $isarray));
	}
	return @options;
}

sub handle_options($self, @l)
{
	my @options = $self->create_options(@l);

MAINLOOP:
	while (@main::ARGV > 0) {
		my $arg = shift @main::ARGV;
		if ($arg =~ m/^\-\-$/) {
			last;
		}
		if ($arg =~ m/^\-/) {
			for my $opt (@options) {
				if ($opt->match($arg, $self)) {
					next MAINLOOP;
				}
			}
			shortdie "Unknown option $arg\n";
		} else {
			unshift(@main::ARGV, $arg);
			last;
		}
	}
}

sub handle_permuted_options($self, @l)
{
	my @options = $self->create_options(@l);

	$self->{kept} = [];

MAINLOOP2:
	while (@main::ARGV > 0) {
		my $arg = shift @main::ARGV;
		if ($arg =~ m/^\-\-$/) {
			next;   # XXX ?
		}
		if ($arg =~ m/^\-/) {
			for my $opt (@options) {
				if ($opt->match($arg, $self)) {
					next MAINLOOP2;
				}
			}
		}
		$self->keep_for_later($arg);
	}
	@main::ARGV = @{$self->{kept}};
}

sub keep_for_later($self, @args)
{
	push(@{$self->{kept}}, @args);
}

sub new($class)
{
	bless {}, $class;
}

1;
