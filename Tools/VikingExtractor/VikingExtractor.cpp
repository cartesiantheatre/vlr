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
#include "VicarColourImage.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <getopt.h>

// Using the standard namespace...
using namespace std;

// Show help...
void ShowHelp()
{
    cout << "Usage: VikingExtractor [options] input [output]" << endl
         << "Options:" << endl
         << "  -h, --help               Show this help" << endl
         << "  -V, --verbose            Be verbose" << endl
         << "  -v, --version            Show version information" << endl << endl
         << "Converts 1970s Viking Lander era VICAR colour images to PNGs." << endl;
}

// Show version information...
void ShowVersion()
{
    cout << "VikingExtractor " VIKING_EXTRACTOR_VERSION << endl
         << "Copyright (C) 2010, 2011 Kshatra Corp." << endl
         << "This is free software; see the source for copying conditions.  There is NO" << endl
         << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

// Entry point...
int main(int ArgumentCount, char *Arguments[])
{
    // Variables...
    int     OptionCharacter = '\x0';
    int     OptionIndex     = 0;
    bool    Verbose         = false;
    string  InputFile;
    string  OutputFile;

    // Command line option structure...
    option CommandLineOptions[] =
    {
        /* These options set a flag. */
        {"help",    no_argument,    NULL,   'h'},
        {"verbose", no_argument,    NULL,   'V'},
        {"version", no_argument,    NULL,   'v'},
        
        // End of array marker...
        {0, 0, 0, 0}
    };

    // Keep processing each option until there are none left...
    while((OptionCharacter = getopt_long(
        ArgumentCount, Arguments, "hVv", CommandLineOptions, &OptionIndex)) != -1)
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

            // Help...
            case 'h':
            {
                // Show help...
                ShowHelp();

                // Exit...
                exit(0);
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
            InputFile = Arguments[optind++];
        
        // Wasn't provided...
        else
        {
            // Alert, abort...
            cerr << "Error: Input expected..." << endl;
            exit(1);
        }

    // The output file is optional...
    
        // Fetch...
        if(optind + 1 <= ArgumentCount)
            OutputFile = Arguments[optind++];
        
        // Wasn't explicitly provided...
        else
        {
            // Use same file name as input for output, but with .png extension...
            OutputFile = InputFile + string(".png");
        }

    // Append .png suffix to output, if it doesn't exist already...
    if(OutputFile.find(".png") == string::npos)
        OutputFile += string(".png");

    // Check for extraneous arguments...
    if(optind + 1 <= ArgumentCount)
    {
        // Alert, abort...
        cerr << "Error: Unknown parameter " << optind + 1 << "/" << ArgumentCount << endl;
        exit(1);
    }

    // Allocate a VICAR colour image object and read the header...
    VicarColourImage    Image(InputFile, Verbose);

    // Check for failed load, with error message already visible...
    if(!Image.IsOk())
        exit(1);

    // Done...
    return 0;
}

