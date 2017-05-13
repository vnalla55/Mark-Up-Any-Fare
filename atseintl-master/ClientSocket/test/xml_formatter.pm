use strict;
use XML::Parser;

my $indent = 0;
my $document = "";
my %args = ();
my $skipping = 0;
my $leg = 0;
my $sop = 0;

sub output_attr
{
	while(@_) {
		my $name = shift @_;
		my $value = shift @_;
		$document .= " $name=\"$value\"";
	}
}

sub indentation { "  " x $indent; }

sub output_element
{
	my $element = shift @_;
	$document .= &indentation() . "<$element";
	&output_attr(@_);
	$document .= ">\n";
	++$indent;
}

sub output_close_element
{
	--$indent;
	my $element = shift @_;
	$document .= &indentation . "</$element>\n";
}

sub output_text
{
	my $text = shift @_;
	$document .= $text;
}

sub output_and_close_element
{
	my $element = shift @_;
	$document .= &indentation() . "<$element";
	output_attr(@_);
	$document .= "/>\n";
}

my $expect_sop = 0;

sub handle_start
{
	my ($info,$element,@attr) = @_;

	my %attr = @attr;

	if($skipping or ($args{'nodiag'} or $args{'diag'}) and $element eq 'DIA') {
		++$skipping;
		return;
	}

	if($element eq 'LEG') {
		$sop = 0;
		++$leg;

		if($args{'legs'} and $leg > $args{'legs'}) {
			++$skipping;
			return;
		}
	}

	if($element eq 'SOP') {
		++$sop;
		if($args{'sops'} and $sop > $args{'sops'}) {
			++$skipping;
			return;
		}
	}

	if($element eq 'ShoppingRequest' and $args{'request_type'}) {
		$attr{'N06'} = $args{'request_type'};
	}

	if($element eq 'ShoppingRequest' and $args{'timeout'}) {
		$attr{'D70'} = $args{'timeout'};
	}

	if($element eq 'PRO' and $args{'solutions'}) {
		$attr{'Q0S'} = $args{'solutions'};
	}
	
	&output_element($element,%attr);

	if($element eq 'ShoppingRequest' and $args{'diag'}) {
		&output_element('DIA',('Q0A' => $args{'diag'}));
		my $args = $args{'diagargs'};
		if($args) {
			my %args = %$args;
			foreach my $key (keys %args) {
				&output_and_close_element('ARG',('NAM' => $key, 'VAL' => $args{$key}));
			}
		}

		&output_close_element('DIA');
	}
}

sub handle_end
{
	if($skipping) {
		--$skipping;
		return;
	}
	
	my ($info,$element) = @_;
	&output_close_element($element);
}

sub handle_text
{
	my ($info,$text) = @_;
	$document .= $text;
}

sub format_xml
{
	$document = "";
	$indent = 0;
	my $xml = shift @_;
	%args = @_;

	my %handlers = (Start => \&handle_start, End => \&handle_end);
	$handlers{'Char'} = \&handle_text if $args{'output_text'};

	my $parser = new XML::Parser(Handlers => \%handlers);
	$parser->parse($xml);
	return $document;
}

1;
