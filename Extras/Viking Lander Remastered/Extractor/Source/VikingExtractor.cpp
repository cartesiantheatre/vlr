/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
    
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

    // Provided by Autoconf...
    #include <config.h>
    
    // Our headers...
    #include "Console.h"
    #include "VikingExtractor.h"
    #include "VicarImageAssembler.h"
    #include "VicarImageBand.h"
    
    // Standard C / POSIX system headers...
    #include <cassert>
    #include <iostream>
    #include <string>
    #include <cstring>
    #include <cstdlib>
    #include <getopt.h>
    #include <unistd.h>
    
    // GNU OCRAD...
    #include <ocradlib.h>

// Using the standard namespace...
using namespace std;

// Show help...
void ShowHelp()
{
    cout << "Usage: viking-extractor [options] input [output]"                                  << endl
         << "Options:"                                                                          << endl
         << "      --directorize-band-class"                                                    << endl
         << "                              Put reconstructed images into subdirectories"        << endl
         << "                              named after band type class (e.g. Colour)."          << endl
         << "      --directorize-location  Put reconstructed images into subdirectories"        << endl
         << "                              named after location taken from."                    << endl
         << "      --directorize-month     Put reconstructed images into subdirectories"        << endl
         << "                              named after Martian month taken on."                 << endl
         << "      --directorize-sol       Put reconstructed images into subdirectories"        << endl
         << "                              numbered by camera event solar day."                 << endl
         << "      --dry-run               Don't write anything"                                << endl
         << "      --help                  Show this help"                                      << endl
         << "      --ignore-bad-files      Don't stop on corrupt or problematic input file,"    << endl
         << "                              but continue extraction of other files."             << endl
         << "      --interlace             Encode output with Adam7 interlacing"                << endl
         << "  -j, --jobs[=threads]        Number of threads to run parallelized. Only one"     << endl
         << "                              if -j is not provided, or auto if threads"           << endl
         << "                              argument not specified."                             << endl
         << "      --filter-camera-event[=id]"                                                  << endl
         << "                              Look only for the specified matching camera event"   << endl
         << "                              ID, such as 22A158."                                 << endl
         << "      --filter-diode[=type]   Extract from matching supported diode filter"        << endl
         << "                              classes which are any (default), colour, infrared,"  << endl
         << "                              sun, or survey."                                     << endl
         << "      --filter-lander=#       Extract from specific lander only which are"         << endl
         << "                              either (0, the default), 1, or 2."                   << endl
         << "      --filter-solar-day[=#]  Look only for camera events taken on the specified"  << endl
         << "                              solar day."                                          << endl
         << "      --generate-metadata     Whenever a colour image is recovered, machine"       << endl
         << "                              generate a text file containing various metadata."   << endl
         << "      --no-ansi-colours       Disable VT/100 ANSI coloured terminal output."       << endl
         << "      --no-auto-rotate        Don't automatically rotate image as needed."         << endl
         << "      --no-reconstruct        Don't attempt to reconstruct camera events, just"    << endl
         << "                              dump all available band data as separate images."    << endl
         << "      --overwrite             Overwrite any existing output files."                << endl
         << "  -r, --recursive             Scan subfolders as well if input is a directory."    << endl
         << "      --summarize-only        No warnings or errors displayed, summarize only."    << endl
         << "  -V, --verbose               Be verbose"                                          << endl
         << "  -v, --version               Show version information"                            << endl << endl

         << "Converts 1970s Viking Lander era VICAR colour images to PNGs. The value of"        << endl
         << "'input' can be either a single VICAR file or a directory containing VICAR files"   << endl
         << "to attempt reconstruction into the provided output directory."                     << endl << endl

         << "Report bugs to " VIKING_EXTRACTOR_BUGREPORT << endl;
}

// Show version information...
void ShowVersion()
{
    cout << "VikingExtractor " VIKING_EXTRACTOR_VERSION << " (GNU Ocrad " << OCRAD_version() << ")" << endl
         << "Copyright (C) 2010, 2011, 2012 Cartesian Theatre." << endl
         << "This is free software; see Copying for copying conditions. There is NO" << endl
         << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl << endl;

    cout << "Configured with: " VIKING_EXTRACTOR_CONFIG_FLAGS << endl;
}

// Entry point...
int main(int ArgumentCount, char *Arguments[])
{
    // Variables...
    int         OptionCharacter         = '\x0';
    int         OptionIndex             = 0;
    string      InputFileOrRootDirectory;
    string      OutputRootDirectory;
    
    // Some user options...
    bool        VerboseConsole          = false;

    // Enumerator of long command line option identifiers...
    enum option_long_enum
    {
        option_long_directorize_band_class = 256, /* To ensure no clashes with short option char identifiers */
        option_long_directorize_location,
        option_long_directorize_month,
        option_long_directorize_sol,
        option_long_dry_run,
        option_long_filter_camera_event,
        option_long_filter_diode_class,
        option_long_filter_lander,
        option_long_filter_solar_day,
        option_long_generate_metadata,
        option_long_help,
        option_long_ignore_bad_files,
        option_long_interlace,
        option_long_jobs,
        option_long_no_ansi_colours,
        option_long_no_auto_rotate,
        option_long_no_reconstruct,
        option_long_overwrite,
        option_long_recursive,
        option_long_summarize_only,
        option_long_verbose,
        option_long_version
    };

    // Command line option structure...
    option CommandLineLongOptions[] =
    {
        {"directorize-band-class",  no_argument,        NULL,   option_long_directorize_band_class},
        {"directorize-location",    no_argument,        NULL,   option_long_directorize_location},
        {"directorize-month",       no_argument,        NULL,   option_long_directorize_month},
        {"directorize-sol",         no_argument,        NULL,   option_long_directorize_sol},
        {"dry-run",                 no_argument,        NULL,   option_long_dry_run},
        {"filter-camera-event",     required_argument,  NULL,   option_long_filter_camera_event},
        {"filter-diode",            required_argument,  NULL,   option_long_filter_diode_class},
        {"filter-lander",           required_argument,  NULL,   option_long_filter_lander},
        {"filter-solar-day",        required_argument,  NULL,   option_long_filter_solar_day},
        {"generate-metadata",       no_argument,        NULL,   option_long_generate_metadata},
        {"help",                    no_argument,        NULL,   option_long_help},
        {"ignore-bad-files",        no_argument,        NULL,   option_long_ignore_bad_files},
        {"interlace",               no_argument,        NULL,   option_long_interlace},
        {"jobs",                    optional_argument,  NULL,   option_long_jobs},
        {"no-ansi-colours",         no_argument,        NULL,   option_long_no_ansi_colours},
        {"no-auto-rotate",          no_argument,        NULL,   option_long_no_auto_rotate},
        {"no-reconstruct",          no_argument,        NULL,   option_long_no_reconstruct},
        {"overwrite",               no_argument,        NULL,   option_long_overwrite},
        {"recursive",               no_argument,        NULL,   option_long_recursive},
        {"summarize-only",          no_argument,        NULL,   option_long_summarize_only},
        {"verbose",                 no_argument,        NULL,   option_long_verbose},
        {"version",                 no_argument,        NULL,   option_long_version},
        
        // End of array marker...
        {0, 0, 0, 0}
    };

    // Keep processing each option until there are none left...
    while((OptionCharacter = getopt_long(
        ArgumentCount, Arguments, "j::rVv", CommandLineLongOptions, &OptionIndex)) != -1)
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

            // Directorize by band type class...
            case option_long_directorize_band_class: { Options::GetInstance().SetDirectorizeBandTypeClass(); break; }

            // Directorize by location...
            case option_long_directorize_location: { Options::GetInstance().SetDirectorizeLocation(); break; }

            // Directorize by month...
            case option_long_directorize_month: { Options::GetInstance().SetDirectorizeMonth(); break; }

            // Directorize by solar day...
            case option_long_directorize_sol: { Options::GetInstance().SetDirectorizeSol(); break; }

            // Dry run...
            case option_long_dry_run: { Options::GetInstance().SetDryRun(); break; }

            // Filter by camera event ID...
            case option_long_filter_camera_event:
            { assert(optarg); Options::GetInstance().SetFilterCameraEvent(optarg); break; }
            
            // Filter by diode class...
            case option_long_filter_diode_class:
            { assert(optarg); Options::GetInstance().SetFilterDiodeClass(optarg); break; }

            // Filter by lander number...
            case option_long_filter_lander: 
            { assert(optarg); Options::GetInstance().SetFilterLander(atoi(optarg)); break; }

            // Filter by solar day...
            case option_long_filter_solar_day:
            { assert(optarg); Options::GetInstance().SetFilterSolarDay(atoi(optarg)); break; }

            // Generate metadata...
            case option_long_generate_metadata: { Options::GetInstance().SetGenerateMetadata(); break; }

            // Help...
            case option_long_help:
            {
                // Show help...
                ShowHelp();

                // Exit...
                exit(EXIT_SUCCESS);
            }

            // Ignore bad files...
            case option_long_ignore_bad_files: { Options::GetInstance().SetIgnoreBadFiles(); break; }

            // Interlace with Adam7...
            case option_long_interlace: { Options::GetInstance().SetInterlace(); break; }
            
            // Jobs...
            case 'j':
            case option_long_jobs: 
            {
                // Issue a non-implemented warning...
                Message(Console::Warning) 
                    << "parallelization is not implemented yet, using single thread" 
                    << endl;
                
                size_t Jobs = 0;
                
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
                Options::GetInstance().SetJobs(Jobs);
                break;
            }

            // No ANSI VT/100 terminal colour...
            case option_long_no_ansi_colours: { Console::GetInstance().SetUseColours(false); break; }

            // No automatic image rotation correction...
            case option_long_no_auto_rotate: { Options::GetInstance().SetAutoRotate(false); break; }

            // No reconstruct...
            case option_long_no_reconstruct: { Options::GetInstance().SetNoReconstruct(); break; }

            // Overwrite output files...
            case option_long_overwrite: { Options::GetInstance().SetOverwrite(); break; }

            // Recursive scan of subfolders if input is a directory...
            case 'r':
            case option_long_recursive: { Options::GetInstance().SetRecursive(); break; }

            // Summarize only...
            case option_long_summarize_only: { Options::GetInstance().SetSummarizeOnly(); break; }

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
                exit(EXIT_SUCCESS);
            }

            // Unknown option...
            case '?':
            {
                // get_opt_long already dumped an error message...
                exit(EXIT_FAILURE);
            }
            
            // Unknown option...
            default:
            {
                //cout << "Exiting on option " << (int) OptionCharacter << endl;
                
                // Exit...
                exit(EXIT_FAILURE);
            }
        }
    }

    // Toggle verbose console...
    Console::GetInstance().SetChannelEnabled(Console::Verbose, VerboseConsole);

    // Check OCRAD...

        // Verify linked against compatible library...
        if(OCRAD_version()[0] != OCRAD_version_string[0])
        {
            // Alert, abort...
            Message(Console::Error) 
                << "GNU Ocrad " << OCRAD_version_string << " linked against incompatible GNU Ocrad version " 
                << OCRAD_version() 
                << endl;
            exit(EXIT_FAILURE);
        }

        // Too old...
        if(OCRAD_version()[2] < '2')
        {
            // Alert, abort...
            Message(Console::Error) 
                << "GNU Ocrad needs to be at least 0.21, but got " 
                << OCRAD_version() 
                << endl;
            exit(EXIT_FAILURE);        
        }

    // We need at least one additional parameter, the input...
    
        // Fetch...
        if(optind + 1 <= ArgumentCount)
            InputFileOrRootDirectory = Arguments[optind++];

        // Wasn't provided...
        else
        {
            // Alert, abort...
            Message(Console::Error) << "need input file or directory, see --help" << endl;
            exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    // Extract from a set of VICAR images...
    try
    {
        // Create image assembler where input and output "files" are just
        //  the input directory and the root output directory respectively...
        VicarImageAssembler Assembler(InputFileOrRootDirectory, OutputRootDirectory);
        
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
            exit(EXIT_FAILURE);
        }

    // Done...
    return 0;
}
