/*
    Sol2Ls, to translate a Martian absolute solar day in numerical form into a 
    Martian month, day, and solar longitute.
    Copyright (C) 2010, 2012 Kshatra Corp <kip@thevertigo.com>.
    
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
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <iostream>

// Using the standard name space...
using namespace std;

// Get the day of the month this solar day falls within, starting from one...
int GetDayOfMonth(const int SolarDay)
{
    const float MartianSolsPerYear  = 668.5991f;
    
    const int ClampedSolarDay = 
        static_cast<int>(fmodf(SolarDay, MartianSolsPerYear));

    if(ClampedSolarDay < 62)        return ClampedSolarDay;
    else if(ClampedSolarDay < 127)  return ClampedSolarDay - 62 + 1;
    else if(ClampedSolarDay < 193)  return ClampedSolarDay - 127 + 1;
    else if(ClampedSolarDay < 258)  return ClampedSolarDay - 193 + 1;
    else if(ClampedSolarDay < 318)  return ClampedSolarDay - 258 + 1;
    else if(ClampedSolarDay < 372)  return ClampedSolarDay - 318 + 1;
    else if(ClampedSolarDay < 422)  return ClampedSolarDay - 372 + 1;
    else if(ClampedSolarDay < 469)  return ClampedSolarDay - 422 + 1;
    else if(ClampedSolarDay < 515)  return ClampedSolarDay - 469 + 1;
    else if(ClampedSolarDay < 563)  return ClampedSolarDay - 515 + 1;
    else if(ClampedSolarDay < 614)  return ClampedSolarDay - 563 + 1;
    else
        return ClampedSolarDay - 614 + 1;
}

// Get the Martian month from the Ls angle...
std::string GetMonth(const float Ls)
{
    // Convert Ls to month...
    if(Ls <= 30.0f)                         return string("Gemini");
    else if(30.0f < Ls && Ls <= 60.0f)      return string("Cancer");
    else if(60.0f < Ls && Ls <= 90.0f)      return string("Leo");
    else if(90.0f < Ls && Ls <= 120.0f)     return string("Virgo");
    else if(120.0f < Ls && Ls <= 150.0f)    return string("Libra");
    else if(150.0f < Ls && Ls <= 180.0f)    return string("Scorpius");
    else if(180.0f < Ls && Ls <= 210.0f)    return string("Sagittarius");
    else if(210.0f < Ls && Ls <= 240.0f)    return string("Capricorn");
    else if(240.0f < Ls && Ls <= 270.0f)    return string("Aquarius");
    else if(270.0f < Ls && Ls <= 300.0f)    return string("Pisces");
    else if(300.0f < Ls && Ls <= 330.0f)    return string("Aries");
    else                                    return string("Taurus");
}

// Convert a given Martian solar day in the range [1 .. n] to angle of 
//  solar longitute...
float SolarDayToLs(const size_t SolarDay)
{
    // Bounds check...
    assert(SolarDay >= 1);

    // Variables...
    float Ls                        = 0.0f;
    float EccentricAnomaly          = 0.0f;
    float EccentricAnomalyDelta     = 0.0f;

    // Constants...
    const float SolsInYear          = 668.5991f;
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

int main(const int ArgumentCount, const char *Arguments[])
{
    assert(ArgumentCount == 2);

    const int SolarDay = atoi(Arguments[1]);
    const float Ls = SolarDayToLs(SolarDay);

/*
    ./sol2ls 193 gives wrong answer. Gives 1, Leo (Ls = 89.8705), 
    but should be Virgo 1. Problem possibly in solving Kepler equation.
*/

    std::cout 
        << GetDayOfMonth(SolarDay)
        << ", " << GetMonth(Ls) 
        << " (Ls = " << Ls << ")" << endl;

    return 0;
}

