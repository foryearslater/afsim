#!/usr/bin/perl -w
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

$pgm_name = "make_doc.pl";
$version = "Version: 05/22/2009";

# Determine Operating System and mode (Interactive (Windows) or
#  Command-line)
# NOTE: If Windows O/S not detected assume that system is "Linux"
$os = $^O; # Get operating system (Windows = "MsWin..")

print "\n$pgm_name $version";

print "\n\nCreate documentation files for \".pl\" files and";
print "\n \".pod\" files in this folder";

while (1) {
    print "\n\nEnter converison type ...";
    print "\nEnter \"h\" for HTML format,";
    print "\nEnter \"t\" for TEXT format,";
    print "\nPress ENER key for default HTML: ";
    chomp($doc_type = <STDIN>); # Enter type of conversion

    if (length($doc_type) == 0 or
	$doc_type =~ /^h/i       ) {
	$doc_type = "HTML";
	last; # Exit while-loop
    }
    elsif ($doc_type =~ /^t/i) {
	$doc_type = "TEXT";
	last; # Exit while-loop
    }
    else {
	print "\n*ERROR - Unknown document type: $doc_type";
	print "\nPlease try again ...";
    }
}

print"\n\"$doc_type\" mode selected";

print "\n\nTo process all \".pl\" and \".pod\" files,";
print "\n press ENTER (or enter CONTROL-C to Abort).";
print "\n\nOtherwise, enter a single file name to process: ";
chomp($filename = <STDIN>);

if ($filename =~ /\.pl$/i  or
    $filename =~ /\.pod$/i)  {
    push(@file_list, $filename); # process this Perl file

    if ($doc_type =~ /^html$/i) {
	&pod2html(\@file_list); # Convert to HTML documentation
    }
    else {
	&pod2text(\@file_list); # Convert to TEXT documentation
    }
}
elsif (length($filename) == 0) {
    # Process all .pl and .pod files

    # Build file list of .pl and .pod files in current directory:
    @file_list = glob "*.pl";       # Get .pl files
    push(@file_list, glob "*.pod"); # Add .pod files

    if ($doc_type =~ /^html$/i) {
	&pod2html(\@file_list); # Convert to HTML documentation
    }
    else {
	&pod2text(\@file_list); # Convert to TEXT documentation
    }
}
else {
    print "\nNo \".pl\" or \".pod\" file names entered - Aborting ...";
}

# Remove any temporary files used in document conversion:
if (-f "pod2htmd.x~~") {
    unlink "pod2htmd.x~~";
}
if (-f "pod2htmi.x~~") {
    unlink "pod2htmi.x~~";
}
if (-f "pod2htmd.tmp") {
    unlink "pod2htmd.tmp";
}
if (-f "pod2htmi.tmp") {
    unlink "pod2htmi.tmp";
}

print "\n----";
print "\nDONE";
print "\n----\n";

if ($os =~ /^MSWin/) {
    # Assume running this script in Windows interactive mode
    print "\nPress ENTER key to Quit";
    <STDIN>;
}

###########################################################
# SUBROUTINES #############################################
###########################################################

# SUBROUTINE pod2html(<Perl/pod file list>) - Process .pl/.pod files to HTML format
sub pod2html {
    my @files = @{$_[0]}; # Convert reference back to array
    my ($file, $root_name, $cmd, $outfile); # Declare local variables

    foreach $file (@files) {
	if ($file =~ /^(.+)\.pl$/i or
	    $file =~ /^(.+)\.pod$/i  ) {
	    $root_name = $1;

	    # Create output file name depending on whether
	    #  document is a Perl script or a POD file:
	    if ($file =~ /\.pl$/i) {
		$outfile = $root_name . "_doc.html";
	    }
	    else {
		$outfile = $root_name . ".html";
	    }
	}
	else {
	    # This code should never execute:
	    print "\n*ERROR: Name not recognized as a \".pl\" or \".pod\" file";
	    print "\n        File name = $file";
	    print "\nPlease report this BUG!\n";

	    if ($os =~ /^mswin/i) {
		# Pause if running under Windows (assume running interactively)
		print "\nPress ENTER key to Abort ... ";
		<STDIN>;
	    }
	    die("\nTerminating execution ... \n");
	}
	$cmd = "pod2html --infile=";
	$cmd .= $file . " --outfile=";
	$cmd .= "$outfile";

	print "\nCreating HTML documentation for file: $file";
	print "\nWriting document to file:             $outfile\n";

	system($cmd); # Execute system command
    }
}

# SUBROUTINE pod2text(<file list>) - Process .pl/.pod files to TEXT format
sub pod2text {
    my @files = @{$_[0]}; # Convert reference back to array
    my ($file, $root_name, $cmd, $outfile); # Declare local variables

    # Set appropriate perldoc "-o" option based on operating system
    #  in use:
    if ($os =~/^mswin/i) {
	$o_option = ""; # Blank for Windows
    }
    else {
	$o_option = "-otext"; # Set output to TEXT for Linux
    }
    foreach $file (@files) {
	if ($file =~ /^(.+)\.pl$/i or
	    $file =~ /^(.+)\.pod$/i  ) {
	    $root_name = $1;

	    # Create output file name depending on whether
	    #  document is a Perl script or a POD file:
	    if ($file =~ /\.pl$/i) {
		$outfile = $root_name . "_doc.txt";
	    }
	    else {
		$outfile = $root_name . ".txt";
	    }
	}
	else {
	    # This code should never execute:
	    print "\n*ERROR: Name not recognized as a \".pl\" or \".pod\" file";
	    print "\n        File name = $file";
	    print "\nPlease report this BUG!\n";

	    if ($os =~ /^mswin/i) {
		# Pause if running under Windows (assume running interactively)
		print "\nPress ENTER key to Abort ... ";
		<STDIN>;
	    }
	    die("\nTerminating execution ... \n");
	}
	$cmd = "perldoc $o_option -F";
	$cmd .= " $file > $outfile";

	print "\nCreating TEXT documentation for file: $file";
	print "\nWriting document to file:             $outfile\n";

	system($cmd);
    }
}

# SUBROUTINE pod_text(

# End of Perl source code -- Begin this script's documentation using
#                            "Plain Old Documentation" format

# See text below for details on converting this script's embedded
#  documentation into user-readable document formats.

__END__

=pod

=head1 NAME - make_doc.pl

=head2 SYNOPSIS

make_doc.pl - Create HTML documentation from Plain Old Documentation
embedded in Perl scripts (with file extensions ending in ".pl") and
text documents (with file extensions ending in ".pod". Perl script
output documents will be named <script_root_name>_doc.html or
<script_root_name>_doc.txt, and re-formatted ".pod" output files will
be named <pod_root_name>.html or <pod_root_name>.txt.

=head2 DESCRIPTION

(NOTE: Individuals using this script to develop other analysis tools
are encouraged to add a short description of their new script's
functions in this section of the embedded documentation and to share
newly-developed scripts with AFSIM developers and users.)

This script automates the extraction of Plain Old Documentation (POD)
formatted documentation in Perl scripts (ending with ".pl" file name
extensions) or POD document files (ending with ".pod" file name
extensions). When executed, the user is first prompted to enter the
desired output format: either "HTML" or "TEXT". These entries are
case-insensitive and, in fact, only the first letter ("h" or "t")
needs to be entered. However, since the default output format is
"HTML", just pressing the ENTER key will result in documentation files
being generated in HTML format.

Next, the user is prompted to enter the filename of a Perl script
(with a ".pl" file extension) or a POD file (with a ".pod" file
extension). If the user wants all Perl scripts and POD files in the
current directory to be processed, just press the ENTER key.

=head2 EXTRACTING DOCUMENTATION FROM THIS PERL SCRIPT

The documentation for this Perl script is included at the bottom of
the Perl source code.  It has been written using a mark-up language
called "Plain Old Documentation" (POD). Using a text editor, any
improvements to this Perl script and/or its documentation can be
described in this section of the file.

To create this documentation as a text file from the command line,
enter:

 In a Windows environment:
   perldoc -F make_doc.pl > make_doc_doc.txt

 In a Linux environment:
   perldoc -otext -F make_doc.pl > make_doc_doc.txt

Entering "text" for the "-o" option in a Linux environment will
insure that ASCII text documentation will be created, but this option
is not recognized in Windows.

To convert POD documentation to HTML format, enter:

 pod2html --infile=make_doc.pl --outfile=make_doc_doc.html

The resulting HTML file can be viewed with any web browser.  The
"perldoc" and "pod2html" programs are included in the Boeing-standard
Perl installation for the Windows operating system (available for
download from "Software Express") as well as Boeing-standard Linux
operating system installations. (NOTE: The "pod2html" program will
create two small temporary files that have names starting with
"pod2htmd" and "pod2htmi". Once the "pod2html" program has finished,
these two temporary files can be deleted. However, when using the
"make_doc.pl" Perl script, these temporary files will be automatically
deleted.)

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
