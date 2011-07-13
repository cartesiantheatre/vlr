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
#include "VikingExtractor.h"
#include "VicarImageBand.h"
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
    cout << "Usage: VikingExtractor [options] input [output]" << endl
         << "Options:" << endl
         << "  -d, --dry-run            Don't write anything" << endl
         << "  -h, --help               Show this help" << endl
         << "  -i, --interlace          Encode output with Adam7 interlacing" << endl
         << "  -l, --save-record-labels Save VICAR record labels as text file" << endl
         << "  -V, --verbose            Be verbose" << endl
         << "  -v, --version            Show version information" << endl << endl

         << "Converts 1970s Viking Lander era VICAR colour images to PNGs." << endl
         << "If 'input' is a directory, extract / assemble separate colour" << endl
         << "bands into output directory. Otherwise operate on single file." << endl;
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
    int         OptionCharacter = '\x0';
    int         OptionIndex     = 0;
    bool        DryRun          = false;
    bool        Verbose         = false;
    bool        Interlace       = false;
    bool        SaveLabels      = false;
    struct stat FileAttributes;
    bool        AssembleMode    = false;
    string      InputFile;
    string      OutputFile;

    // Command line option structure...
    option CommandLineOptions[] =
    {
        {"dry-run",             no_argument,    NULL,   'd'},
        {"help",                no_argument,    NULL,   'h'},
        {"interlace",           no_argument,    NULL,   'i'},
        {"save-record-labels",  no_argument,    NULL,   'l'},
        {"verbose",             no_argument,    NULL,   'V'},
        {"version",             no_argument,    NULL,   'v'},
        
        // End of array marker...
        {0, 0, 0, 0}
    };

    // Keep processing each option until there are none left...
    while((OptionCharacter = getopt_long(
        ArgumentCount, Arguments, "dhilVv", CommandLineOptions, &OptionIndex)) != -1)
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

            // Dry run...
            case 'd':
            {
                // Set dry run flag...
                DryRun = true;

                // Done...
                break;
            }

            // Help...
            case 'h':
            {
                // Show help...
                ShowHelp();

                // Exit...
                exit(0);
            }

            // Interlacing...
            case 'i':
            {
                // Set interlace flag...
                Interlace = true;
                
                // Done...
                break;
            }

            // Save labels...
            case 'l':
            {
                // Set save labels flag...
                SaveLabels = true;

                // Done...
                break;
            }

            // Version...
            case 'v':
            {
                // Dump version information...
                ShowVersion();

                // Exit...
                exit(0);
            }

            // Verbose...
            case 'V':
            {
                // Set verbose flag...
                Verbose = true;
                
                // Done...
                break;
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
                cerr << "error: could not stat input " << InputFile << endl;
                exit(1);
            }
            
            // Directory...
            if(S_ISDIR(FileAttributes.st_mode))
                AssembleMode = true;
            
            // File...
            else
                AssembleMode = false;
        }

        // Wasn't provided...
        else
        {
            // Show help...
            ShowHelp();
            exit(1);
        }

    // The output file is optional...

        // Output file was provided...
        if(optind + 1 <= ArgumentCount)
        {
            // Extract...
            OutputFile = Arguments[optind++];

            // If we are in assemble mode, make sure it is a directory...
            if(AssembleMode)
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
                        cerr << "error: could not stat output directory " << OutputFile << endl;
                        exit(1);
                    }
                }

                // Get output directory attributes...
                if(stat(OutputFile.c_str(), &FileAttributes) != 0)
                {
                    cerr << "error: could not stat output directory " << OutputFile << endl;
                    exit(1);
                }

                // Still doesn't exist...
                if(!S_ISDIR(FileAttributes.st_mode))
                {
                    cerr << "error: could not stat output directory " << OutputFile << endl;
                    exit(1);
                }

                // Done...
                AssembleMode = true;
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
            if(AssembleMode)
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

    // Check for extraneous arguments...
    if(optind + 1 <= ArgumentCount)
    {
        // Alert, abort...
        cerr << "error: unknown parameter " << optind + 1 << "/" << ArgumentCount << endl;
        exit(1);
    }

    // Extract out the image or images...
    try
    {
        // Assemble mode...
        if(AssembleMode)
        {
            VicarImageAssembler Assembler(InputFile, Verbose);
            
            for(VicarImageAssembler::const_iterator Index = Assembler.FirstImage();
                Index !=
        }
        
        // Just extract from a single file...
        else
        {
            // Try to load a VICAR colour image object and read the header...
            VicarImageBand Image(InputFile, Verbose);
            
            // Set user the save label flag, if user selected...
            Image.SetSaveLabels(SaveLabels);
            
            // Write out the image, if not in dry mode...
            if(!DryRun)
                Image.Extract(OutputFile, Interlace);
        }
    }

        // Failed...
        catch(const std::string &ErrorMessage)
        {
            // Alert...
            cerr << InputFile << std::string(": error: ") << ErrorMessage << endl;
            
            // Terminate...
            exit(1);
        }

    // Done...
    return 0;
}

