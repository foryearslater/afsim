#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Set DEBUG:
# - Level 0 = No DEBUG
# - Level 1 = TEST Mode: Do not actually rename files

$DEBUG = 0;

$pgm_name = "rename_output.pl";
$version = "Version: 11/20/2009";

print "\n$pgm_name $version\n";

# Determine Operating System and mode (Interactive (Windows) or
#  Command-line)
# NOTE: If Windows O/S not detected assume that system is "Linux"
$os = $^O; # Get operating system (Windows = "MsWin..")

$cmd_line_mode = 1; # Default is Linux-style Command-line mode

if ($os =~ /^mswin/i) {
    $cmd_line_mode = 0; # Assume running Windows Interactive mode
}

print "\nRename files in \"output\" sub-directory to";
print "\n include date-time-stamps";

print "\n\n - Only .csv, .evt, .log, and .rep files will be processed\n";

@d = localtime(time);

$yr = $d[5] + 1900;
$mo_day = sprintf("%02d%02d", $d[4]+1, $d[3]);
$hr_min_sec = sprintf("%02d%02d%02d", $d[2], $d[1], $d[0]);

# Define timestamp YEAR_<MonthDay>_<HourMinuteSecond>:
$timestamp = $yr . '_' . $mo_day . '_' . $hr_min_sec;

print "\nUsing Time-stamp: $timestamp\n";
&pause(0);

&file_rename(".csv");
&file_rename(".evt");
&file_rename(".log");
&file_rename(".rep");

print "\n\nDONE";

if (not $cmd_line_mode) {
    print "\nPress ENTER key to Quit: ";
    <STDIN>;
}

print "\n";

############################ SUBROUTINES ############################

# SUBROUTINE pause(<DEBUG Level>) - Pause for DEBUG level >= <DEBUG Level>
sub pause {
    if ($DEBUG >= $_[0]) {
	print "\nPress ENTER key to continue ... ";
	<STDIN>;
    }
}

# SUBROUTINE file_rename("<File extension>") - Rename files with
#  specified file extension in output directory
sub file_rename {
    my $file_ext = $_[0]; # Get file extension

    my ($old_file, $new_file); # Declare local variables

    my @files = glob("output/*" . $file_ext);
    foreach $old_file (@files) {
	# Do not modify files that already have time-stamps:
	if ($old_file =~ /\d\d\d\d_\d\d\d\d_\d\d\d\d\d\d/) {
	    next; # Skip this file
	}
	# Build new file name from old file name root and extension:
	$new_file = $old_file;
	$new_file =~ s/$file_ext//i;
	$new_file .= '_' . $timestamp . $file_ext;

	print "\nRenaming file \"$old_file\" to:";
	print "\n              \"$new_file\"";

	if ($DEBUG == 0) {
	    rename($old_file,$new_file);
	}
	else {
	    print "\n - IN TEST MODE: No file names changed";
	}
    }
}

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - rename_output.pl

=head2 SYNOPSIS

rename_output.pl - Rename .evt, .log, .rep and .csv files in the
"output" subdirectory to include a year_month_day_hour_minute_second
time-stamp.


=head2 DESCRIPTION

This is a simple Perl script that renames files written in a
subdirectory named "output" to include a date-hour-minute-second
time-stamp. This script needs to be executed from a directory
immediately above the "output" subdirectory. Once files are renamed
with their characteristic time-stamp (ex: xxxx_2009_1120_081243.evt),
this script will ignore them.

This script can be modified to include file names with other
extensions, but the current extensions being supported are:

 .csv
 .evt
 .log
 .rep

=head2 TROUBLESHOOTING PROBLEMS WITH THIS SCRIPT

See the comments in this script's source code to help understand what
each block of code accomplishes. Setting the $DEBUG variable to a
number higher than zero will cause intermediate processing to be
displayed in the terminal window. If $DEBUG is set to one,
intermediate processing will stream up the terminal window until the
script terminates normally or if a fatal execution error is
encountered. Debug output includes the line number of the input file
to help in determining the cause of any errors. For more in-depth
inspection of script processing, run the script on the command-line
using the Perl debugger:

 perl -d rename_output.pl

For more information on the Perl debugger, access the Windows
ActivePerl Documentation, Perl Core Document "perldebug", or, use the
following command in a Linux (or Windows) terminal window:

 perldoc perldebug

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F rename_output.pl > rename_output_doc.txt

 In a Linux environment:
   perldoc -otext -F rename_output.pl > rename_output_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=rename_output.pl --outfile=rename_output_doc.html

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
