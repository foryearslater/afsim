#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# file name: regex_util.pl
# Version: 07/29/2009

print "\n------------------------------------------------------------";
print "\nregex_util.pl - Experiment with Perl regular expressions";
print "\nVer 1.4, 05/13/2009";
print "\n";
print "\n- Enter STRINGS without beginning or ending double-quotes (\").";
print "\n  - Use \"\\n\" to create an embedded end-of-line character";
print "\n     in a STRING.";
print "\n";
print "\n- Enter REGULAR EXPRESSIONS without the beginning or";
print "\n   ending \"/\" characters.";
print "\n";
print "\n- To re-use a previously-entered STRING or REGULAR EXPRESION,";
print "\n   just press the ENTER key without typing any characters";
print "\n";
print "\n- To Quit, enter CONTROL-C.";
print "\n------------------------------------------------------------";

while (1) {
    print "\n\nEnter STRING: ";
    chomp($instr = <STDIN>);

    if (length($instr) > 0) {
	$str = $instr;  # Update string
	$str =~ s/\\n/\n/g; # Convert \n to Newline(s)
	$str =~ s/\\t/\t/g; # Convert \t to Tab(s)
    }
    else {
	print "STRING = $str\n";
    }

    print "\nEnter REGULAR EXPRESSION: ";
    chomp($instr = <STDIN>);

    if (length($instr) > 0) {
	$pat = $instr;  # Update pattern
    }
    else {
	print "PATTERN = $pat\n";
    }

    print "\nEnter MODIFIER (ENTER = NONE, i, m, s, sm): ";
    chomp($instr = <STDIN>);

    if ($instr =~ /^(i|s|m|sm)$/) {
	$mod = $instr;
    }
    else {
	$mod = "";
    }
    # No modifier:
    if (($mod eq "") && ($str =~ /$pat/)) {
	&pr_match(); # Print the match condition
    }
    # Case insensitive modifier:
    elsif (($mod eq "i") && ($str =~ /$pat/i)) {
	&pr_match(); # Print the match condition
    }
    # String as set of multiple lines;
    #  "." does not match Newline
    #  "^"/"$" matches at start/end of any new line:
    elsif (($mod eq "m") && ($str =~ /$pat/m)) {
	&pr_match(); # Print the match condition
    }
    # String as a single long line;
    #  "." even matches Newline;
    #  "^"/"$" only matches at start/end of string:
    elsif (($mod eq "s") && ($str =~ /$pat/s)) {
	&pr_match(); # Print the match condition
    }
    # String as a single long line;
    #  "." even matches Newline;
    #  "^"/"$" matches at start/end of any new line:
    elsif (($mod eq "sm") && ($str =~ /$pat/sm)) {
	&pr_match(); # Print the match condition
    }
    else {
	print "\n\n\"$str\" does NOT MATCH \/$pat\/$mod";
    }
}

# ------------ SUBROUTINES --------------

# Print any matches found plus extracted variables:
sub pr_match {
    print "\n\n\"$str\" MATCHES \/$pat\/$mod";

    # Display sub-substring before and after match
    print "\n";
    if (defined $`) {
	print "\nString preceding match \$\` = $`";
    }
    else {
	print "\nString preceding match \$\` is UNDEFINED";
    }

    if (defined $&) {
	print "\nString matched \$\& = $&";
    }
    else {
	print "\nString matched \$\& is UNDEFINED";
    }

    if (defined $') { # '
	print "\nString following match \$\' = $'";
    }
    else {
	print "\nString following match \$\' is UNDEFINED";
    }

    if (defined $+) {
	print "\nLast bracket matched \$\+ = $+";
    }
    else {
	print "\nLast bracket matched \$\+ is UNDEFINED";
    }

    # NOTE: the following may not be compatible with all versions of Perl:

    # Init possible extracted read-only variables $1, $2, ... $+
    #   which are set by matches within: (<group 1>)(<group 2>) ...
    # Positions for start of matches in "@-[n]" array
    # Positions for end   of matches in "@+[n]" array
    # Position for start of entire match in $+[0]
    # Position for end   of entire match in $-[0]
    print "\n";
    foreach $gp_nr (1..$#-) {
	print "\nMatch \$$gp_nr: '${$gp_nr}'\n\t\tat indices ($-[$gp_nr],$+[$gp_nr])";
    }
    print "\n(START,END) for entire match = ($-[0],$+[0])";
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - regex_util.pl

=head2 SYNOPSIS

regex_util.pl - Test and experiment with Perl regular expression patterns

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

Perl "regular expressions" provide a powerful capability for
manipulating and processing data files, but creating regular
expressions has its challenges, even for those familiar with Perl and
how to use regular expressions. For those new to regular expressions,
this script can be used as a learning tool to experiment with how
different regular expression features can extract data from character
strings. However, it can also be used to help in debugging a Perl
script that uses regular expressions, but the results are not as
expected.

For those new to regular expressions, a good starting place is the
"perldoc" document "perlrequick". This can be accessed in a Linux
operating environment by entering:

 perldoc perlrequick

For Boeing-standard Windows operating system installations, click on
"perlrequick" on the left-hand frame under "Perl Core Documentation".

Next, continue with the Perldoc "prelretut" to gain a deeper
understanding into how regular expressions work.

It takes a little time to learn the features available with regular
expressions, but using regular expressions in your Perl code will
often result in scripts that are shorter, simpler, easier to debug and
modify.

=head2 USING THIS UTILITY

Instructions for using this utility script are displayed when the
script is first started:

 - Enter STRINGS without beginning or ending double-quotes (").
   - Use "\n" to create an embedded end-of-line character in a STRING.

 - Enter REGULAR EXPRESSIONS without the beginning or ending "/" characters.

 - To re-use a previously-entered STRING or REGULAR EXPRESION, just press
   the ENTER key without typing any characters.

 - To Quit, enter CONTROL-C.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F regex_util.pl > regex_util_doc.txt

 In a Linux environment:
   perldoc -otext -F regex_util.pl > regex_util_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=regex_util.pl --outfile=regex_util_doc.html

The resulting HTML file can be viewed with any web browser.  The
"perldoc" and "pod2html" programs are included in the Boeing-standard
Perl installation for the Windows operating system (available for
download from "Software Express") as well as Boeing-standard Linux
operating system installations. (NOTE: The "pod2html" program will
create two small temporary files that have names starting with
"pod2htmd" and "pod2htmi". Once the "pod2html" program has finished,
these two temporary files can be deleted.)

=head2 AUTHOR

 Gary Richardson
 Strategic Projects & Analysis
 Boeing Research & Technology

=head2 BUGS AND TECHNICAL SUPPORT

If you think you have found a bug in one of our existing data
processing/analysis tools, have ideas on how to improve these
utilities, or simply want help in getting started using the Perl
programming language in your modeling/simulation efforts (including
models other than those developed using AFSIM), please contact us.

 Gary Richardson, Associate Technical Fellow
 Strategic Projects & Analysis
 Boeing Research & Technology
 Tel: 314-232-5451
 E-mail: gary.a.richardson@boeing.com

=head2 COPYRIGHT

Copyright - The Boeing Company, 2009

=cut
