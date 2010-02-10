/*
 * ReactOS log2lines
 * Written by Jan Roeloffzen
 *
 * - Help text and functions
 */
#include <stdio.h>

#include "version.h"
#include "compat.h"
#include "config.h"
#include "options.h"
#include "cmd.h"
#include "help.h"

char *verboseUsage =
"\n"
"Description:\n"
"  When <exefile> <offset> are given, log2lines works like raddr2line:\n"
"      - The <exefile> <offset> combination can be repeated\n"
"      - Also, <offset> can be repeated for each <exefile>\n"
"      - NOTE: Some of the options below will have no effect in this form.\n"
"  Otherwise it reads stdin and tries to translate lines of the form:\n"
"      <IMAGENAME:ADDRESS>\n"
"  The result is written to stdout.\n\n"
"  <offset> or <ADDRESS> can be absolute or relative with the restrictions:\n"
"  - An image with base < 0x400000 MUST be relocated to a > 0x400000 address.\n"
"  - The offset of a relocated image MUST be relative.\n\n"
"  log2lines uses a cache in order to avoid a directory scan at each\n"
"  image lookup, greatly increasing performance. Only image path and its\n"
"  base address are cached.\n\n"
"Options:\n"
"  -b   Use this combined with '-l'. Enable buffering on logFile.\n"
"       This may solve loosing output on real hardware (ymmv).\n\n"
"  -c   Console mode. Outputs text per character instead of per line.\n"
"       This is slightly slower but enables to see what you type.\n\n"
"  -d <directory>|<ISO image>\n"
"       <directory>: Directory to scan for images. (Do not append a '" PATH_STR "')\n"
"       <ISO image>: This option also takes an ISO image as argument:\n"
"       - The image is recognized by the '.iso' or '.7z' extension.\n"
"       - NOTE: The '.7z' and extracted '.iso' basenames must be identical,\n"
"         which is normally true for ReactOS trunk builds.\n"
"       - The image will be unpacked to a directory with the same name.\n"
"       - The embedded reactos.cab file will also be unpacked.\n"
"       - Combined with -f the file will be re-unpacked.\n"
"       - NOTE: this ISO unpack feature needs 7z to be in the PATH.\n"
"       Default: " DEF_OPT_DIR "\n\n"
"  -f   Force creating new cache.\n\n"
"  -F   As -f but exits immediately after creating cache.\n\n"
"  -h   This text.\n\n"
"  -l <logFile>\n"
"       <logFile>: Append copy to specified logFile.\n"
"       Default: no logFile\n\n"
"  -m   Prefix (mark) each translated line with '* '.\n\n"
"  -M   Prefix (mark) each NOT translated line with '? '.\n"
"       ( Only for lines of the form: <IMAGENAME:ADDRESS> )\n\n"
"  -P <cmd line>\n"
"       Pipeline command line. Spawn <cmd line> and pipeline its output to\n"
"       log2lines (as stdin). This is for shells lacking support of (one of):\n"
"       - Input file redirection.\n"
"       - Pipelining byte streams, needed for the -c option.\n\n"
"  -r   Raw output without translation.\n\n"
"  -R <cmd>\n"
"       Revision commands interfacing with SVN. <cmd> is one of:\n"
"       - check:\n"
"         To be combined with -S. Check each source file in the log and issue\n"
"         a warning if its revision is higher than that of the tested build.\n"
"         Also when the revison of the source tree is lower than that of the\n"
"         tested build (for every source file).\n"
"         In both cases the source file's -S output would be unreliable.\n"
"       - update:\n"
"         Updates the SVN log file. Currently only generates the index file\n"
"         The SVN log file itself must be generated by hand in the sources\n"
"         directory like this (-v is mandatory here):\n"
"             svn log -v > svndb.log ('svn log' accepts also a range)\n"
"         'svndb.log' and its index are needed for '-R regscan'\n"
"       - regscan[,<range>]:\n"
"         Scan for regression candidates. Essentially it tries to find\n"
"         matches between the SVN log entries and the sources hit by\n"
"         a backtrace (bt) command.\n"
"         <range> is the amount of revisions to look back from the build\n"
"         revision (default 500)\n"
"         The output of '-R regscan' is printed after EOF. The 'Changed path'\n"
"         lists will contain only matched files.\n"
"         Limitations:\n"
"         - The bug should really be a regression.\n"
"         - Expect a number of false positives.\n"
"         - The offending change must be in the sources hit by the backtrace.\n"
"           This mostly excludes changes in headerfiles for example.\n"
"         - Must be combined with -S.\n"
"       Can be combined with -tTS.\n\n"
"  -s   Statistics. A summary with the following info is printed after EOF:\n"
"       *** LOG2LINES SUMMARY ***\n"
"       - Translated:      Translated lines.\n"
"       - Reverted:        Lines translated back. See -u option\n"
"       - Retranslated:    Lines retranslated. See -U option\n"
"       - Skipped:         Lines not translated.\n"
"       - Differ:          Lines where (addr-1) info differs. See -tT options\n"
"       - Differ(func/src):Lines where also function or source info differ.\n"
"       - Rev conflicts:   Source files conflicting with build. See '-R check'\n"
"       - Reg candidates:  Regression candidates. See '-R regscan'\n"
"       - Offset error:    Image exists, but error retrieving offset info.\n"
"       - Total:           Total number of lines attempted to translate.\n"
"       Also some version info is displayed.\n\n"
"  -S <context>[+<add>][,<sources>]\n"
"       Source line options:\n"
"       <context>: Source lines. Display up to <context> lines until linenumber.\n"
"       <add>    : Optional. Display additional <add> lines after linenumber.\n"
"       <sources>: Optional. Specify alternate source tree.\n"
"       The environment variable " SOURCES_ENV " should be correctly set\n"
"       or specify <sources>. Use double quotes if the path contains spaces.\n"
"       For a reliable result, these sources should be up to date with\n"
"       the tested revision (or try '-R check').\n"
"       Can be combined with -tTR.\n"
"       Implies -U (For retrieving source info).\n\n"
"  -t   Translate twice. The address itself and for (address-1).\n"
"       Show extra filename, func and linenumber between [..] if they differ\n"
"       So if only the linenumbers differ, then only show the extra\n"
"       linenumber.\n\n"
"  -T   As -t, but show only filename+func+linenumber for (address-1)\n\n"
"  -u   Undo translations.\n"
"       Lines are translated back (reverted) to the form <IMAGENAME:ADDRESS>\n"
"       Also removes all lines previously added by this tool (e.g. see -S)\n\n"
"  -U   Undo and reprocess.\n"
"       Reverted to the form <IMAGENAME:ADDRESS>, and then retranslated\n"
"       Implies -u.\n\n"
"  -v   Show detailed errors and tracing.\n"
"       Repeating this option adds more verbosity.\n"
"       Default: only (major) errors\n\n"
"  -z <path to 7z>\n"
"       <path to 7z>: Specify path to 7z. See also option -d.\n"
"       Default: '7z'\n"
"\n"
"CLI escape commands:\n"
"  It is possible to change options and perform actions from the 'kdb:>' prompt\n"
"  By prepending the `(backquote) character to the option.\n"
"  Example: 'kdb:> `l new.log' changes the current logfile to 'new.log'.\n"
"  Flag options like 'b' are given a numeric value of 0 (off) or 1 (on).\n"
"  Options accepting a string as argument can be cleared by the value '" KDBG_ESC_OFF "'.\n"
"  Some ClI commands are read only or not (entirely) implemented.\n"
"  If no value is provided, the current one is printed.\n"
"  There are only a few extra ClI commands or with different behaviour:\n"
"  - `h : shows this helptext (without exiting)\n"
"  - `q : quits log2lines\n"
"  - `R regscan : the output is printed immediately (do a 'bt' first)\n"
"  - `R regclear : clears previous regscan matches\n"
"  - `s : the output is printed immediately\n"
"  - `s clear : clears all statistics.\n"
"  - `S : only <context> and <add> can be set.\n"
"  - `v <level> : sets the current debug loglevel\n"
"\n"
"Option Examples:\n"
"  Setup: A VMware machine with its serial port set to: '\\\\.\\pipe\\kdbg'.\n\n"
"  Just recreate cache after a svn update or a new module has been added:\n"
"       log2lines -F\n\n"
"  Use kdbg debugger via console (interactive):\n"
"       log2lines -c < \\\\.\\pipe\\kdbg\n\n"
"  Use kdbg debugger via console, and append copy to logFile:\n"
"       log2lines -c -l dbg.log < \\\\.\\pipe\\kdbg\n\n"
"  Same as above, but for PowerShell:\n"
"       log2lines -c -l dbg.log -P \"piperead -c \\\\.\\pipe\\kdbg\"\n\n"
"  Use kdbg debugger to send output to logfile:\n"
"       log2lines < \\\\.\\pipe\\kdbg > dbg.log\n\n"
"  Re-translate a debug log:\n"
"       log2lines -U -d bootcd-38701-dbg.iso < bugxxxx.log\n\n"
"  Re-translate a debug log. Specify a 7z file, which wil be decompressed.\n"
"  Also check for (address) - (address-1) differences:\n"
"       log2lines -U -t -d bootcd-38701-dbg.7z < bugxxxx.log\n"
"  Output:\n"
"       <ntdll.dll:60f1 (dll/ntdll/ldr/utils.c:337[331] (LdrPEStartup))>\n\n"
"  The following commands are equivalent:\n"
"       log2lines msi.dll 2e35d msi.dll 2235 msiexec.exe 30a8 msiexec.exe 2e89\n"
"       log2lines msi.dll 2e35d 2235 msiexec.exe 30a8 2e89\n\n"
"  Generate source lines from backtrace ('bt') output. Show 2 lines of context:\n"
"       log2lines -S 2 -d bootcd-38701-dbg.7z < bugxxxx.log\n"
"  Output:\n"
"       <msiexec.exe:2e89 (lib/3rdparty/mingw/crtexe.c:259 (__tmainCRTStartup))>\n"
"       | 0258  #else\n"
"       | 0259      mainret = main (\n"
"       <msiexec.exe:2fad (lib/3rdparty/mingw/crtexe.c:160 (WinMainCRTStartup))>\n"
"       | 0159    return __tmainCRTStartup ();\n"
"       | 0160  }\n\n"
"  Generate source lines. Show 2 lines of context plus 1 additional line and\n"
"  specify an alternate source tree:\n"
"       log2lines -S 2+1,\"c:\\ros trees\\r44000\" -d bootcd-44000-dbg < dbg.log\n"
"  Output:\n"
"       <msi.dll:2e35d (dll/win32/msi/msiquery.c:189 (MSI_IterateRecords))>\n"
"       | 0188      {\n"
"       | 0189          r = MSI_ViewFetch( view, &rec );\n"
"       | ----\n"
"       | 0190          if( r != ERROR_SUCCESS )\n\n"
"  Use '-R check' to show that action.c has been changed after the build:\n"
"       log2lines -s -d bootcd-43850-dbg.iso -R check -S 2  < dbg.log\n"
"  Output:\n"
"       <msi.dll:35821 (dll/win32/msi/registry.c:781 (MSIREG_OpenUserDataKey))>\n"
"       | 0780      if (create)\n"
"       | 0781          rc = RegCreateKeyW(HKEY_LOCAL_MACHINE, keypath, key);\n"
"       <msi.dll:5262 (dll/win32/msi/action.c:2665 (ACTION_ProcessComponents))>\n"
"       | R--- Conflict : source(44191) > build(43850)\n"
"       | 2664              else\n"
"       | 2665                  rc = MSIREG_OpenUserDataKey(comp->ComponentId,\n\n"
"CLI Examples: (some are based on the option examples above)\n"
"  Use '`R check' to show that action.c has been changed after the build:\n"
"       kdb:> `R check\n"
"       | L2L- -R is \"check\" (changed)\n"
"       kdb:> bt\n"
"       <msi.dll:35821 (dll/win32/msi/registry.c:781 (MSIREG_OpenUserDataKey))>\n"
"       | 0780      if (create)\n"
"       | 0781          rc = RegCreateKeyW(HKEY_LOCAL_MACHINE, keypath, key);\n"
"       <msi.dll:5262 (dll/win32/msi/action.c:2665 (ACTION_ProcessComponents))>\n"
"       | R--- Conflict : source(44191) > build(43850)\n"
"       | 2664              else\n"
"       | 2665                  rc = MSIREG_OpenUserDataKey(comp->ComponentId,\n\n"
"       kdb:>\n\n"
"  Generate source lines. Show 2 lines of context plus 1 additional line.\n"
"  The -Uu options are dependent on -S:\n"
"       kdb:> `S 2+1\n"
"       | L2L- -u Undo is 1 (changed)\n"
"       | L2L- -U Undo and reprocess is 1 (changed)\n"
"       | L2L- -S Sources option is 2+1,\"C:\\ROS\\reactos\\\" \n"
"       | L2L- (Setting source tree not implemented)\n"
"       kdb:> bt\n"
"       <msi.dll:2e35d (dll/win32/msi/msiquery.c:189 (MSI_IterateRecords))>\n"
"       | 0188      {\n"
"       | 0189          r = MSI_ViewFetch( view, &rec );\n"
"       | ----\n"
"       | 0190          if( r != ERROR_SUCCESS )\n"
"       kdb:>\n\n"
"  Change logfile:\n"
"       kdb:> `l\n"
"       | L2L- -l logfile is "" (unchanged)\n"
"       kdb:> `l new.log\n"
"       | L2L- -l logfile is \"new.log\" (changed)\n"
"       kdb:> `l off\n"
"       | L2L- -l logfile is "" (changed)\n"
"       kdb:>\n"
"\n";

void
usage(int verbose)
{
    fprintf(stderr, "log2lines " LOG2LINES_VERSION "\n\n");
    fprintf(stderr, "Usage: log2lines -%s {<exefile> <offset> {<offset>}}\n", optchars);
    if (verbose)
        fprintf(stderr, "%s", verboseUsage);
    else
        fprintf(stderr, "Try log2lines -h\n");
}
