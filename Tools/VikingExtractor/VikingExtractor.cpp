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
         << "  -y, --dry-run               Don't write anything"                                << endl
         << "  -h, --help                  Show this help"                                      << endl
         << "  -b, --ignore-bad-files      Don't stop on corrupt or problematic input file,"    << endl
         << "                              but continue extraction of other files."             << endl
         << "  -i, --interlace             Encode output with Adam7 interlacing"                << endl
         << "  -j, --jobs[=threads]        Number of threads to run parallelized. One if"       << endl
         << "                              -j not provided or auto if threads argument not"             << endl
         << "                              specified."                                          << endl
         << "  -l, --save-record-labels    Save VICAR record labels as text file"               << endl
         << "  -n, --lander-filter <#>     Extract from specific lander only which are any"     << endl
         << "                              (default), 1, or 2."                                 << endl
         << "  -a, --summary-only          No warnings or errors displayed, summary only."       << endl
         << "  -r, --no-colours            Disable VT/100 ANSI coloured terminal output."       << endl
         << "  -R, --no-rotate             Don't apply automatic 90 degree counter"             << endl
         << "                              clockwise rotation."                                 << endl
         << "  -s, --sol-directorize       Put reconstructed images into subdirectories"        << endl
         << "                              numbered by camera event solar day."                 << endl
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
    string      DiodeFilterClass;
    bool        DryRun                  = false;
    bool        IgnoreBadFiles          = false;
    bool        UseColours              = true;
    bool        AutoRotate              = true;
    bool        VerboseConsole          = false;
    bool        Interlace               = false;
    size_t      Jobs                    = 1;
    string      LanderFilter;
    bool        SaveLabels              = false;
    bool        SolDirectorize          = false;
    bool        SummarizeOnly           = false;

    // Command line option structure...
    option CommandLineOptions[] =
    {
        {"ignore-bad-files",    no_argument,        NULL,   'b'},
        {"diode-filter",        required_argument,  NULL,   'f'},
        {"dry-run",             no_argument,        NULL,   'y'},
        {"help",                no_argument,        NULL,   'h'},
        {"interlace",           no_argument,        NULL,   'i'},
        {"jobs",                optional_argument,  NULL,   'j'},
        {"lander-filter",       required_argument,  NULL,   'n'},
        {"summarize-only",      no_argument,        NULL,   'a'},
        {"no-colours",          no_argument,        NULL,   'r'},
        {"no-rotate",           no_argument,        NULL,   'R'},
        {"save-record-labels",  no_argument,        NULL,   'l'},
        {"sol-directorize",     no_argument,        NULL,   's'},
        {"verbose",             no_argument,        NULL,   'V'},
        {"version",             no_argument,        NULL,   'v'},
        
        // End of array marker...
        {0, 0, 0, 0}
    };

    // Keep processing each option until there are none left...
    while((OptionCharacter = getopt_long(
        ArgumentCount, Arguments, "bf:yhij::n:arRslVv", CommandLineOptions, &OptionIndex)) != -1)
    {
        // Which option?
        switch(OptionCharacter)
        {
            // Special case to store a value...
            case 0:
            {
                /* If this option set a flag, do nothing else now. */
                if(CommandLineOptions[OptionIndex].flag != false)
                    break;

                cout << "option " << CommandLineOptions[OptionIndex].name;
                
                if(optarg)
                    cout << " with arg " << optarg;

                // Done...
                cout << endl;
                break;
            }

            // Summarize only...
            case 'a': { SummarizeOnly = true; break; }
            
            // Ignore bad files...
            case 'b': { IgnoreBadFiles = true; break; }
           
            // Diode filter class...
            case 'f': { assert(optarg); DiodeFilterClass = optarg; break; }

            // Help...
            case 'h':
            {
                // Show help...
                ShowHelp();

                // Exit...
                exit(0);
            }

            // Interlacing...
            case 'i': { Interlace = true; break; }
            
            // Jobs...
            case 'j': 
            {
/*
    TODO: Seems to ignore threads parameter if provided.
*/

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

            // Save labels...
            case 'l': { SaveLabels = true; break; }

            // Set lander filter...
            case 'n': { assert(optarg); LanderFilter = optarg; break; }

            // Set no terminal colour...
            case 'r': { UseColours = false; break; }
            
            // Set no automatic image rotation correction...
            case 'R': { AutoRotate = false; break; }
            
            // Set solar directory mode...
            case 's': { SolDirectorize = true; break; }

            // Version...
            case 'v':
            {
                // Dump version information...
                ShowVersion();

                // Exit...
                exit(0);
            }

            // Verbose...
            case 'V': { VerboseConsole = true; break; }

            // Dry run...
            case 'y': { DryRun = true; break; }

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

clog << "jobs " << Jobs << endl;
exit(0);

    // We need at least one additional parameter, the input...
    
        // Fetch...
        if(optind + 1 <= ArgumentCount)
            InputFileOrRootDirectory = Arguments[optind++];

        // Wasn't provided...
        else
        {
            // Show help...
            ShowHelp();
            exit(1);
        }

    // The output directory is optional, but check if explicitly provided...
    if(optind + 1 <= ArgumentCount)
        OutputRootDirectory = Arguments[optind++];

    // Check for extraneous arguments...
    if(optind + 1 <= ArgumentCount)
    {
        // Alert, abort...
        Message(Console::Error) << "unknown parameter " << optind + 1 << "/" << ArgumentCount << endl;
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

