# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# ============================================================================
#                   Invoke the common automatic test driver.
#
#                    Use 'perl auto_test.pl -?' to get help.

# ============================================================================
# NOTE: Always execute this program using "perl auto_test.pl". Do NOT put a
#       '#!/usr/bin/perl' in the first line. Perl *MAY* not be there!
# ============================================================================

# Build the argument list and check for the presence of the -x argument.

use feature ':5.10';
use ENV qw(@PATH);

$auto_test = "auto_test_misc.pl";
@arguments = ( "perl", $auto_test );
$found_x = 0;
foreach $arg (@ARGV)
{
    if ($arg =~ /^-x/) { $found_x = 1; }
    push @arguments, $arg;
}

# If the -x argument was not specified, use a default.

if ($found_x == 0)
{
    if ($^O =~ /MSWin/)
    {
        @exes = ("..\\vs2010\\Release_dll\\afnes_ide_main.exe",
                 "..\\vs2010\\Debug_dll\\afnes_ide_main.exe",
                 "..\\vs2008\\Release_dll\\afnes_ide_main.exe",
                 "..\\vs2008\\Debug_dll\\afnes_ide_main.exe",
                 "..\\vs2005\\Release_dll\\afnes_ide_main.exe",
                 "..\\vs2005\\Debug_dll\\afnes_ide_main.exe");
        $test_program = "";
        $index = 0;
        while (($index < @exes) && (! -e $test_program))
        {
           $test_program = @exes[$index];
           $index++;
        }
    }
    else
    {
        $system_type = `../tools/system_type.sh`;
        $system_type =~ s/\s//;
        $test_program = "../sage/bin/afnes_ide_main_" . $system_type . $ENV{"MODEL"} . ".exe";
        unless (-e $test_program)
        {
           $test_program = "../sage/bin/afnes_ide_main_d_" . $system_type . $ENV{"MODEL"} . ".exe";
        }
        unless (-e $test_program)
        {
           $test_program = "../sage/bin/afnes_ide_main_" . $system_type . $ENV{"MODEL"} . "_x.exe";
        }
        unless (-e $test_program)
        {
           $test_program = "../sage/bin/afnes_ide_main_d_" . $system_type . $ENV{"MODEL"} . "_x.exe";
        }
    }
    push @arguments, ( "-x", $test_program );
}

say @arguments;
system(@arguments);

$error_level = $?;
if ($error_level == -1)
{
    print "Failed to execute $!\n";
}
elsif ($error_level & 127)
{
    printf "Child died with signal %d\n", ($error_level & 127);
}
elsif ($error_level != 0)
{
    printf "Child exited with status %d\n", $error_level >> 8;
}

exit $error_level >> 8;
