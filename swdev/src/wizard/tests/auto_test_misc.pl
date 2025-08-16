# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# =============================================================================
#            This perl script runs a sequence of automated tests.
#
# Directories starting with "auto_" contain test cases. Text files within the
# directory that start with "test_" are consider the individual tests.
# =============================================================================

# ============================================================================
# NOTE: Always execute this program using "perl auto_test.pl". Do NOT put a
#       '#!/usr/bin/perl' in the first line. Perl *MAY* not be there!
# ============================================================================

use Cwd;
use Cwd 'abs_path';
use File::Basename;
use File::Compare;
use File::Spec;
use File::stat;
use Getopt::Std;
use Time::localtime;
use feature ':5.10';
use ENV qw(@PATH);

$onWindows = 0;
$onUnix    = 1;
if ($^O =~ /MSWin/)
{
    $onWindows = 1;
    $onUnix    = 0;
}

# Assign default values to the options and parse the options.

$Getopt::Std::STANDARD_HELP_VERSION = true;      # see Perl documentation

$options{"v"} = 0;
$options{"p"} = 0;
$options{"d"} = "auto_*";
$options{"f"} = "test_*";
$options{"e"} = ".js";

if (! getopts('h?vpd:f:e:x:', \%options))
{
    HELP_MESSAGE(\*STDERR);
    die "-FAIL- Invalid arguments\n";
}
if ($options{"h"} || $options{"?"})
{
    HELP_MESSAGE(\*STDERR);
    exit 0;
}
if ($#ARGV >= 0)
{
    HELP_MESSAGE(\*STDERR);
    exit 0;
}

$verbose      = $options{"v"};
$show_passes  = $options{"p"};
$dir_pattern  = $options{"d"};
$file_pattern = $options{"f"};
$ext          = $options{"e"};

die "-FAIL- -x <prog> is a required argument\n" if ($options{"x"} eq "");
$test_program = File::Spec->rel2abs($options{"x"});
$test_program_arg = $test_program;
# The following is used only on Unix because it fails on windows.
# It makes the path name look pretty by eliminating the . and ...
$test_program = abs_path($test_program) if ($onUnix);

die "-FAIL- Test program $test_program_arg does exist\n" if (! -x $test_program);
say "-TEST- Using $test_program";

# Locate the directory containing this script and form the name
# of the execution interface scripts needed to execute the tests.
#
# Absolute path names are required because we 'cd' around.

$proc_fullname = File::Spec->rel2abs($0);
( $proc_vol, $proc_dir, $proc_file) = File::Spec->splitpath($proc_fullname);
if ($onWindows)
{
    $auto_execute = File::Spec->catpath($proc_vol, $proc_dir, "auto_execute.bat");
}
else
{
    $auto_execute = File::Spec->catpath($proc_vol, $proc_dir, "auto_execute.sh");
}

# test script loaded with every test to actually run test
$auto_test_script_fullname = abs_path('auto_test.js');
$auto_test_namespace_fullname = abs_path('test_namespace.js');

die "-FAIL- Unable to locate script $auto_execute\n" if (! -f $auto_execute);

$totalErrorCount = 0;
foreach $dir (glob($dir_pattern))
{
    next if (! -d $dir);

    say "-TEST- Directory $dir" if $verbose;
    $cwd = getcwd();
    chdir $dir or die "-FAIL- Unable to change directory to $dir\n";
    foreach $file (glob($file_pattern . $ext))
    {
        next if (! -f $file);
        say "-TEST- $dir/$file starting" if $verbose;

        # Prepare the list file.

        $baseFile = basename($file, $ext);
        $listFile = $baseFile . ".lis";
        unlink($listFile);
        if (! open(LIST, ">", $listFile))
        {
            say "-FAIL- Unable to open list file $listFile";
            next
        }
        say LIST;
        close LIST;

        # Execute the test...

        # store test execution time for comparison of output file times later
        $epoch_timestamp = (stat($file))[9];
        $mtime = localtime($epoch_timestamp);
        # $mtime = ctime(stat($file)->mtime);   #### cleaner syntax but doesn't work

        $status = system($auto_execute, $test_program, "'$baseFile$ext'", $listFile, $auto_test_script_fullname, $auto_test_namespace_fullname);
        if ($? != 0)
        {
            open(LIST, ">>", $listFile)
                or die "-FAIL- Could not open list file $listFile to record error\n";
            if ($? == -1)
            {
                say LIST "-FAIL- Failed to execute $!";
            }
            elsif ($? & 127)
            {
                printf LIST "-FAIL- Died with signal %d\n", ($? & 127);
            }
            else
            {
                printf LIST "-FAIL- Exited with status %d\n", $? >> 8;
            }
            close LIST;
        }

        # Comparison check standard file outputs with the output just created

        foreach $standardFile (glob($baseFile . "*_standard.*"))
        {
           $epoch_timestamp = (stat($standardFile))[9];
           $outmtime = localtime($epoch_timestamp);
           if ($mtime > $outmtime)
           {
              say "-FAIL- Test \"$dir/$file\" Standard ouput Comparison: output file $baseFile$ftype was not modified during test";
           }
           else
           {
              my(undef, undef, $ftype) = fileparse($standardFile, qr/\..*/);
              if (!compare($baseFile . $ftype, $standardFile) == 0)
              {
                 say "-FAIL- Test \"$dir/$file\" standard file $standardFile does not match out file $baseFile$ftype";
              }
           }
        }

        # Filter the resulting output into a form that will show the errors
        # in the daily build summary (lines prefixed with '=====')

        if (open(LIST, "<", $listFile))
        {
            my $errorCount = 0;
            while (<LIST>)
            {
                if ($_ =~ /^-FAIL-/)
                {
                    ++$errorCount;
                    ++$totalErrorCount;
                    say "-FAIL- FAILURES in \"$dir/$file\":" if ($errorCount == 1);
                    print "$_";
                }
                elsif ($_ =~ /^-PASS-/)
                {
                    print "$_" if ($show_passes);
                }
            }
            close LIST;
            # say "-FAIL- Test \"$aTestFile\" had $errorCount failure(s)" if ($errorCount > 0);
        }

        say "-TEST- $dir/$file complete" if $verbose;
    }
    chdir $cwd or die "-FAIL- Unable to change directory to $cwd\n";
}

if ($totalErrorCount == 0)
{
    say "-PASS- No errors detected";
    exit 0;
}
else
{
    printf "-FAIL- %d total error(s) detected\n", $totalErrorCount;
    exit 1;
}

# Display help message (also called implicitly with --help option)
sub HELP_MESSAGE
{
    my $OUTFH = shift;

    say $OUTFH;
    say $OUTFH "Usage: perl auto_test.pl -v -p -d <dir-pat> -f <file-pat> -x <prog> -e <file-ext>";
    say $OUTFH;
    say $OUTFH "  -v            Display a message as each directory or file is processed.";
    say $OUTFH;
    say $OUTFH "  -p            Display pass messages as well as fail messages.";
    say $OUTFH;
    say $OUTFH "  -d <dir-pat>  The pattern indicating the directories to be processed.";
    say $OUTFH "                This value should be quoted if it contains wild cards.";
    say $OUTFH "                (Default = auto_*)";
    say $OUTFH;
    say $OUTFH "  -f <file-pat> The pattern indicating the files to be processed (without the";
    say $OUTFH "                file extension. This value should be quoted if it contains";
    say $OUTFH "                wild cards. (Default = test_*).";
    say $OUTFH;
    say $OUTFH "  -e <file-ext> The test file extension.";
    say $OUTFH;
    say $OUTFH "  -x <prog>     The name of the executable to be tested.";
    say $OUTFH;
}
