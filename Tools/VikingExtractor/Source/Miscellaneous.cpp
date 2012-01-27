/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010, 2011, 2012 Kshatra Corp <kip@thevertigo.com>.
    
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
#include "Miscellaneous.h"
#include <cstdio>
#include <cmath>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/stat.h>

// Using the standard name space...
using namespace std;

// Create a directory and all of its parents, if necessary...
bool CreateDirectoryRecursively(const std::string &Path)
{
    // Make a copy of the user's string, since we need to change it...
    string PathCopy = Path;

    // Remove trailing path separator...
    if(PathCopy.at(PathCopy.length() - 1) == '/')
        PathCopy.erase(PathCopy.length() - 1);

    // Create the directories from the parent to child order...
    for(string::const_iterator Iterator = PathCopy.begin(); Iterator != PathCopy.end(); ++Iterator)
    {
        // Found the next partial path, create...
        if(*Iterator == '/')
        {
            // Get path up to this point...
            const string PartialPath(PathCopy, 0, Iterator - PathCopy.begin());

            // If it doesn't already exist, create it...
            if(access(PartialPath.c_str(), F_OK) != 0)
                mkdir(PartialPath.c_str(), S_IRWXU);
        }
    }

    // Check if final nested directory exists, if not, create...
    if(access(PathCopy.c_str(), F_OK) != 0)
        return (mkdir(PathCopy.c_str(), S_IRWXU) == 0);
    
    // Otherwise path already exists...
    else
        return true;
}

// Convert a given Martian solar day in the range [1 .. n] to Ls angle...
float SolarDayToLs(const size_t SolarDay)
{
    // Bounds check...
    assert(SolarDay >= 1);

    // Variables...
    float Ls                        = 0.0f;
    float EccentricAnomaly          = 0.0f;
    float EccentricAnomalyDelta     = 0.0f;

    // Constants...
    const float SolsInYear          = 668.6f;
    const float PerihelionDay       = 485.35f;
    const float PerihelionLs        = 250.99f;
    const float Ecentricity         = 0.09340f;
    const float RadiansToDegrees    = 180.0f / M_PI;
    const float TimePerihelion      = 2 * M_PI * (1 - PerihelionLs / 360.0f);

    // Calculate mean anomaly...
    const float zz = (SolarDay - PerihelionDay) / SolsInYear;
    const float SignedMeanAnomaly = 2.0f * M_PI * (zz - round(zz));
    const float MeanAnomaly = abs(SignedMeanAnomaly);

    // Solve Kepler equation MeanAnomaly = EccentricAnomaly - ε * sin(EccentricAnomaly)
    // using Newton iterations...
    EccentricAnomaly = MeanAnomaly + Ecentricity * sin(MeanAnomaly);
    do
    {
        EccentricAnomalyDelta = -(EccentricAnomaly - Ecentricity * sin(EccentricAnomaly) - MeanAnomaly) / 
               (1.0f - Ecentricity * cos(EccentricAnomaly));
        EccentricAnomaly = EccentricAnomaly + EccentricAnomalyDelta;
    }
    while(EccentricAnomalyDelta > 1.0e-6f);
    
    if(SignedMeanAnomaly < 0.0f)
        EccentricAnomaly = -EccentricAnomaly;

    // Compute true anomaly, now that eccentric anomaly is known...
    const float TrueAnomaly = 
        2.0f * atan(sqrt((1.0f + Ecentricity) / 
        (1.0f - Ecentricity)) * tan(EccentricAnomaly / 2.0f));

    // Compute Ls...
    Ls = TrueAnomaly - TimePerihelion;
    if(Ls < 0.0f)
        Ls += 2.0f * M_PI;
    if(Ls > 2.0f * M_PI)
        Ls -= 2.0f * M_PI;

    // Convert Ls from radians into degrees and return...
    return (RadiansToDegrees * Ls);
}

