/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011 Kshatra Corp <kip@thevertigo.com>.
    
    Public discussion on IRC available at #avaneya (irc.freenode.net) 
    or on the mailing list <avaneya@lists.avaneya.com>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Includes...
#include "Console.h"
#include "VikingExtractor.h"
#include "VicarImageAssembler.h"
#include "VicarImageBand.h"
#include <cassert>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <getopt.h>
#include <unistd.h>

// Using the standard namespace...
using namespace std;

// Show help...
void ShowHelp()
{
    cout << "Usage: VikingExtractor [options] input [output]"                                   << endl
         << "Options:"                                                                          << endl
         << "  -f, --diode-filter[=type]   Extract from matching supported diode filter"        << endl
         << "                              classes which are any (default), colour, infrared,"  << endl
         << "                              sun, or survey."                                     << endl
         << "      --dry-run               Don't write anything"                                << endl
         << "      --help                  Show this help"                                      << endl
         << "      --ignore-bad-files      Don't stop on corrupt or problematic input file,"    << endl
         << "                              but continue extraction of other files."             << endl
         << "      --interlace             Encode output with Adam7 interlacing"                << endl
         << "  -j, --jobs[=threads]        Number of threads to run parallelized. Only one"     << endl
         << "                              if -j is not provided, or auto if threads"           << endl
         << "                              argument not specified."                             << endl
         << "      --lander-filter=#       Extract from specific lander only which are"         << endl
         << "                              either (0, the default), 1, or 2."                   << endl
         << "      --no-colours            Disable VT/100 ANSI coloured terminal output."       << endl
         << "      --no-rotate             Don't apply automatic 90 degree counter"             << endl
         << "                              clockwise rotation."                                 << endl
         << "  -r, --recursive             Scan subfolders as well if input is a directory."    << endl
         << "      --save-record-labels    Save VICAR record labels as text file"               << endl
         << "      --sol-directorize       Put reconstructed images into subdirectories"        << endl
         << "                              numbered by camera event solar day."                 << endl
         << "      --summary-only          No warnings or errors displayed, summary only."      << endl
         << "  -V, --verbose               Be verbose"                                          << endl
         << "  -v, --version               Show version information"                            << endl << endl

         << "Converts 1970s Viking Lander era VICAR colour images to PNGs. The value of"        << endl
         << "'input' can be either a single VICAR file or a directory containing VICAR files"   << endl
         << "to attempt reconstruction into the provided output directory."                     << endl << endl

         << "Report bugs to <https://bugs.launchpad.net/avaneya>" << endl;
}

// Show version information...
void ShowVersion()
{
    cout << "VikingExtractor " VIKING_EXTRACTOR_VERSION << endl
         << "Copyright (C) 2010, 2011 Kshatra Corp." << endl
         << "This is free software; see the source for copying conditions. There is NO" << endl
         << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

// Entry point...
int main(int ArgumentCount, char *Arguments[])
{
    // Variables...
    int         OptionCharacter         = '\x0';
    int         OptionIndex             = 0;
    string      InputFileOrRootDirectory;
    string      OutputRootDirectory;
    
    // User switches and defaults...
    bool        AutoRotate              = true;
    string      DiodeFilterClass;
    bool        DryRun                  = false;
    bool        IgnoreBadFiles          = false;
    bool        Interlace               = false;
    size_t      Jobs                    = 1;
    string      LanderFilter;
    bool        Recursive               = false;
    bool        SaveLabels              = false;
    bool        SolDirectorize          = false;
    bool        SummarizeOnly           = false;
    bool        UseColours              = true;
    bool        VerboseConsole          = false;

    // Enumerator of long command line option identifiers...
    enum option_long_enum
    {
        option_long_diode_filter = 256, /* To ensure no clashes with short option char identifiers */
        option_long_dry_run,
        option_long_help,
        option_long_ignore_bad_files,
        option_long_interlace,
        option_long_jobs,
        option_long_lander_filter,
        option_long_no_colours,
        option_long_no_rotate,
        option_long_recursive,
        option_long_save_record_labels,
        option_long_sol_directorize,
        option_long_summarize_only,
        option_long_verbose,
        option_long_version
    };

    // Command line option structure...
    option CommandLineLongOptions[] =
    {
        {"diode-filter",        required_argument,  NULL,   option_long_diode_filter},
        {"dry-run",             no_argument,        NULL,   option_long_dry_run},
        {"help",                no_argument,        NULL,   option_long_help},
        {"ignore-bad-files",    no_argument,        NULL,   option_long_ignore_bad_files},
        {"interlace",           no_argument,        NULL,   option_long_interlace},
        {"jobs",                optional_argument,  NULL,   option_long_jobs},
        {"lander-filter",       required_argument,  NULL,   option_long_lander_filter},
        {"no-colours",          no_argument,        NULL,   option_long_no_colours},
        {"no-rotate",           no_argument,        NULL,   option_long_no_rotate},
        {"recursive",           no_argument,        NULL,   option_long_recursive},
        {"save-record-labels",  no_argument,        NULL,   option_long_save_record_labels},
        {"sol-directorize",     no_argument,        NULL,   option_long_sol_directorize},
        {"summarize-only",      no_argument,        NULL,   option_long_summarize_only},
        {"verbose",             no_argument,        NULL,   option_long_verbose},
        {"version",             no_argument,        NULL,   option_long_version},
        
        // End of array marker...
        {0, 0, 0, 0}
    };

    // Keep processing each option until there are none left...
    while((OptionCharacter = getopt_long(
        ArgumentCount, Arguments, "f:j::rVv", CommandLineLongOptions, &OptionIndex)) != -1)
    {
        // Which option?
        switch(OptionCharacter)
        {
            // Special case to set a flag, which we don't use...
            case 0:
            {
                /* If this option set a flag, do nothing else now. */
                if(CommandLineLongOptions[OptionIndex].flag != false)
                    break;

                cout << "option " << CommandLineLongOptions[OptionIndex].name;
                
                if(optarg)
                    cout << " with arg " << optarg;

                // Done...
                cout << endl;
                break;
            }

            // Diode filter class...
            case 'f':
            case option_long_diode_filter: { assert(optarg); DiodeFilterClass = optarg; break; }

            // Dry run...
            case option_long_dry_run: { DryRun = true; break; }
           
            // Help...
            case option_long_help:
            {
                // Show help...
                ShowHelp();

                // Exit...
                exit(0);
            }

            // Ignore bad files...
            case option_long_ignore_bad_files: { IgnoreBadFiles = true; break; }

            // Interlace with Adam7...
            case option_long_interlace: { Interlace = true; break; }
            
            // Jobs...
            case 'j':
            case option_long_jobs: 
            {
                // Number of threads provided...
                if(optarg)
                    Jobs = atoi(optarg);
                
                // Number of threads not provided...
                else
                    Jobs = 0;

                // Automatic number of jobs explicitly or implicitly 
                //  requested, query number of cpus online...
                if(Jobs == 0)
                    Jobs = sysconf(_SC_NPROCESSORS_ONLN);

                // Done...
                break;
            }

            // Lander filter...
            case option_long_lander_filter: { assert(optarg); LanderFilter = optarg; break; }

            // No automatic image rotation correction...
            case option_long_no_rotate: { AutoRotate = false; break; }

            // No terminal colour...
            case option_long_no_colours: { UseColours = false; break; }

            // Recursive scan of subfolders if input is a directory...
            case 'r':
            case option_long_recursive: { Recursive = true; break; }

            // Save record labels...
            case option_long_save_record_labels: { SaveLabels = true; break; }

            // Sol directorize...
            case option_long_sol_directorize: { SolDirectorize = true; break; }

            // Summarize only...
            case option_long_summarize_only: { SummarizeOnly = true; break; }

            // Verbose...
            case 'V':
            case option_long_verbose: { VerboseConsole = true; break; }

            // Version...
            case 'v':
            case option_long_version:
            {
                // Dump version information...
                ShowVersion();

                // Exit...
                exit(0);
            }

            // Unknown option...
            case '?':
            {
                // get_opt_long already dumped an error message...
                exit(1);
            }
            
            // Unknown option...
            default:
            {
                //cout << "Exiting on option " << (int) OptionCharacter << endl;
                
                // Exit...
                exit(1);
            }
        }
    }

    // Toggle console colours and verbosity to user preference...
    Console::GetInstance().SetUseColours(UseColours);
    Console::GetInstance().SetChannelEnabled(Console::Verbose, VerboseConsole);

    // We need at least one additional parameter, the input...
    
        // Fetch...
        if(optind + 1 <= ArgumentCount)
            InputFileOrRootDirectory = Arguments[optind++];

        // Wasn't provided...
        else
        {
            // Alert, abort...
            Message(Console::Error) << "need input file or directory, see --help" << endl;
            exit(1);
        }

    // The output directory is optional, but check if explicitly provided...
    if(optind + 1 <= ArgumentCount)
        OutputRootDirectory = Arguments[optind++];

    // Check for extraneous arguments...
    if(optind + 1 <= ArgumentCount)
    {
        // Alert, abort...
        Message(Console::Error) 
            << "unknown parameter " 
            << optind + 1 << "/" << ArgumentCount 
            << " ("
            << Arguments[optind] << "), see --help" << endl;
        exit(1);
    }

    // Extract from a set of VICAR images...
    try
    {
        // Create image assembler where input and output "files" are just
        //  the input directory and the root output directory respectively...
        VicarImageAssembler Assembler(InputFileOrRootDirectory, OutputRootDirectory);
        
        // Set usage switches...
        Assembler.SetAutoRotate(AutoRotate);
        Assembler.SetDiodeFilterClass(DiodeFilterClass);
        Assembler.SetIgnoreBadFiles(IgnoreBadFiles);
        Assembler.SetInterlace(Interlace);
        Assembler.SetLanderFilter(LanderFilter);
        Assembler.SetRecursive(Recursive);
        Assembler.SetSolDirectorize(SolDirectorize);
        Assembler.SetSummarizeOnly(SummarizeOnly);
        
        // Reconstruct all possible images found of either the input 
        //  file or a directory into the output directory...
        Assembler.Reconstruct();
    }

        // Failed...
        catch(const std::string &ErrorMessage)
        {
            // Alert...
            Message(Console::Error) << ErrorMessage << endl;
            
            // Terminate...
            exit(1);
        }

    // Done...
    return 0;
}

