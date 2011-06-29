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
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <getopt.h>

// Using the standard namespace...
using namespace std;

// Verbose flag...
static bool VerboseFlag = false;

// Entry point...
int main(int ArgumentCount, char *Arguments[])
{
    // Variables...
    int OptionCharacter = '\x0';
    int OptionIndex     = 0;

    // Command line option structure...
    struct option CommandLineOptions[] =
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
                VerboseFlag = true;
                
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
    
    // After any optional switches, there should be exactly two parameters
    //  with one for the input and the other for the output...
    if(optind + 2 != ArgumentCount)
    {
        // Alert, abort...
        cerr << "Expected input and output files." << endl;
        exit(1);
    }
    
    // Store the input and output file names...
    const string InputFile  = Arguments[optind++];
          string OutputFile = Arguments[optind++];

    // Append png suffix to output, if it doesn't exist already...
    if(OutputFile.find(".png") == string::npos)
        OutputFile += string(".png");



    // Done...
    return 0;
}

// Show help...
void ShowHelp()
{
    cout << "Usage: VikingExtractor [options] input output" << endl
         << "Options:" << endl
         << "  -h, --help               Show this help" << endl
         << "  -V, --verbose            Be verbose" << endl
         << "  -v, --version            Show version information" << endl;
}

// Show version information...
void ShowVersion()
{
    cout << "VikingExtractor " VIKING_EXTRACTOR_VERSION << endl
         << "Copyright (C) 2010, 2011 Kshatra Corp." << endl
         << "This is free software; see the source for copying conditions.  There is NO" << endl
         << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

