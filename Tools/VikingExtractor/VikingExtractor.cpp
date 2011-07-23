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
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// Using the standard namespace...
using namespace std;

// Show help...
void ShowHelp()
{
    cout << "Usage: VikingExtractor [options] input [output]"                                   << endl
         << "Options:"                                                                          << endl
         << "  -f, --diode-filter <type>   Extract from matching supported diode filter"        << endl
         << "                              classes which are any (default), colour, infrared,"  << endl
         << "                              sun, or survey (assembly mode only)."                << endl
         << "  -y, --dry-run               Don't write anything"                                << endl
         << "  -h, --help                  Show this help"                                      << endl
         << "  -b, --ignore-bad-files      Don't stop on corrupt or problematic input file,"    << endl
         << "                              but continue extraction of other files (assembly"    << endl
         << "                              mode only)."                                         << endl
         << "  -i, --interlace             Encode output with Adam7 interlacing"                << endl
         << "  -r, --no-colours            Disable VT/100 ANSI coloured terminal output."       << endl
         << "  -n, --lander-filter <#>     Extract from specific lander only which are any"     << endl
         << "                              (default), 1, or 2 (assembly mode only)."            << endl
         << "  -l, --save-record-labels    Save VICAR record labels as text file"               << endl
         << "  -V, --verbose               Be verbose"                                          << endl
         << "  -v, --version               Show version information"                            << endl << endl

         << "Converts 1970s Viking Lander era VICAR colour images to PNGs. If 'input' is a"     << endl
         << "directory, extract / assemble separate colour bands into output directory"         << endl
         << "(assembly mode). Otherwise operate on single file."                                << endl << endl
         
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
    int         OptionCharacter     = '\x0';
    int         OptionIndex         = 0;
    struct stat FileAttributes;
    bool        AssemblyMode        = false;
    string      InputFile;
    string      OutputFile;
    
    // User switches and defaults...
    string      DiodeFilterClass;
    bool        DryRun              = false;
    bool        IgnoreBadFiles      = false;
    bool        UseColours          = true;
    bool        VerboseConsole      = false;
    bool        Interlace           = false;
    string      LanderFilter;
    bool        SaveLabels          = false;

    // Command line option structure...
    option CommandLineOptions[] =
    {
        {"ignore-bad-files",    no_argument,        NULL,   'b'},
        {"diode-filter",        required_argument,  NULL,   'f'},
        {"dry-run",             no_argument,        NULL,   'y'},
        {"help",                no_argument,        NULL,   'h'},
        {"interlace",           no_argument,        NULL,   'i'},
        {"lander-filter",       required_argument,  NULL,   'n'},
        {"no-colours",          no_argument,        NULL,   'r'},
        {"save-record-labels",  no_argument,        NULL,   'l'},
        {"verbose",             no_argument,        NULL,   'V'},
        {"version",             no_argument,        NULL,   'v'},
        
        // End of array marker...
        {0, 0, 0, 0}
    };

    // Keep processing each option until there are none left...
    while((OptionCharacter = getopt_long(
        ArgumentCount, Arguments, "bf:yhin:rlVv", CommandLineOptions, &OptionIndex)) != -1)
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

            // Save labels...
            case 'l': { SaveLabels = true; break; }

            // Set lander filter...
            case 'n': { assert(optarg); LanderFilter = optarg; break; }

            // Set no terminal colour...
            case 'r': { UseColours = false; break; }

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

    // Set console no colours and verbosity flags...
    Console::GetInstance().SetUseColours(UseColours);
    Console::GetInstance().SetChannelEnabled(Console::Verbose, VerboseConsole);

    // We need at least one additional parameter, the input...
    
        // Fetch...
        if(optind + 1 <= ArgumentCount)
        {
            // Extract...
            InputFile = Arguments[optind++];
            
            // Is this a file or a directory?
            if(stat(InputFile.c_str(), &FileAttributes) != 0)
            {
                // Couldn't stat file...
                Message(Console::Error) << "could not stat input " << InputFile << endl;
                exit(1);
            }
            
            // Directory...
            if(S_ISDIR(FileAttributes.st_mode))
                AssemblyMode = true;
            
            // File...
            else
                AssemblyMode = false;
        }

        // Wasn't provided...
        else
        {
            // Show help...
            ShowHelp();
            exit(1);
        }

    // The output file is optional...

        // Output file was explicitly provided...
        if(optind + 1 <= ArgumentCount)
        {
            // Extract...
            OutputFile = Arguments[optind++];

            // If we are in assembly mode, make sure it is a directory...
            if(AssemblyMode)
            {
                // Failed to attributes...
                if(stat(OutputFile.c_str(), &FileAttributes) != 0)
                {
                    // Doesn't exist, create...
                    if(errno == ENOENT)
                        mkdir(OutputFile.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

                    // Some other error...
                    else
                    {
                        Message(Console::Error) << "could not stat output directory " << OutputFile << endl;
                        exit(1);
                    }
                }

                // Get output directory attributes...
                if(stat(OutputFile.c_str(), &FileAttributes) != 0)
                {
                    Message(Console::Error) << "could not stat output directory " << OutputFile << endl;
                    exit(1);
                }

                // Still doesn't exist...
                if(!S_ISDIR(FileAttributes.st_mode))
                {
                    Message(Console::Error) << "could not stat output directory " << OutputFile << endl;
                    exit(1);
                }

                // Done...
                AssemblyMode = true;
            }
            
            // Not in assembly mode, generating single output file...
            else
            {
                // Append .png suffix to output, if it doesn't exist already...
                if(OutputFile.find(".png") == string::npos)
                    OutputFile += string(".png");
            }
        }

        // Output file was not provided, create...
        else
        {
            // We are in assembly mode, use current working directory...
            if(AssemblyMode)
            {
                // Fetch...
                char Temp[1024];
                OutputFile = getcwd(Temp, 1024);
            }

            // Not in assembly mode, so create implicitly...
            else
            {
                // Start with the input file's name with image suffix...
                OutputFile = InputFile + string(".png");
                
                // Strip the path so only file name remains...
                const size_t PathIndex = OutputFile.find_last_of("\\/");
                if(PathIndex != string::npos)
                    OutputFile.erase(0, PathIndex + 1);
            }
        }

    // If in assembly mode (the output is a directory), make sure it 
    //  ends with a path separator...
    if(AssemblyMode)
    {
        // Search for the last path separator...
        size_t Index = string::npos;
        Index = OutputFile.find_last_of("/\\");

        // If found and it's not the last character, append one...
        if(Index != string::npos && (Index != OutputFile.length() - 1))
            OutputFile += '/';
    }

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
        if(AssemblyMode)
        {
            // Create image assembler...
            VicarImageAssembler Assembler(InputFile);
            
            // Set usage switches...
            Assembler.SetDiodeFilterClass(DiodeFilterClass);
            Assembler.SetIgnoreBadFiles(IgnoreBadFiles);
            Assembler.SetLanderFilter(LanderFilter);
            
            // Index the input directory...
            Assembler.Index();
            
            /*for(VicarImageAssembler::const_iterator Index = Assembler.FirstImage();
                Index !=*/
        }
    }

        // Failed...
        catch(const std::string &ErrorMessage)
        {
            // Alert...
            Message(Console::Error) << ErrorMessage << endl;
            
            // Terminate...
            exit(1);
        }

    // Extract from a single image...
    try
    {
        if(!AssemblyMode)
        {
            // Diode filter class should not be set...
            if(!DiodeFilterClass.empty())
                throw std::string("diode filter class available in assembly mode only");

            // Ignore bad files should not be set...
            if(IgnoreBadFiles)
                throw std::string("ignore bad files available in assembly mode only");
            
            // Lander filter should not be set...
            if(!LanderFilter.empty())
                throw std::string("lander filter available in assembly mode only");
            
            // Construct a VICAR colour image object...
            VicarImageBand Image(InputFile);
            
            // Set user flags
            Image.SetInterlace(Interlace);
            Image.SetSaveLabels(SaveLabels);
            
            // Load the image...
            Image.Load();
            
            // Check for an error...
            if(Image.IsError())
                throw Image.GetErrorMessage();
            
            // Write out the image, if not in dry mode...
            if(!DryRun)
                Image.Extract(OutputFile);
        }
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

