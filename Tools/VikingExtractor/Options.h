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

// Multiple include protection...
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

// Includes...
#include "VicarImageBand.h"
#include <string>
#include <set>

// Options singleton class...
class Options
{
    // Public types...
    public:

        // A set of photosensor array diode band types...
        typedef std::set< VicarImageBand::PSADiode >  FilterDiodeBandSet;

    // Public methods...
    public:

        // Get the singleton instance...
        static Options &GetInstance();

        // Get options...
        bool            GetAutoRotate() { return m_AutoRotate; }
        bool            GetDryRun() { return m_DryRun; }
        std::string    &GetFilterCameraEvent() { return m_FilterCameraEvent; }
        const FilterDiodeBandSet &
                        GetFilterDiodeBandSet() { return m_FilterDiodeBandSet; }
        size_t          GetFilterLander() { return m_FilterLander; }
        size_t          GetFilterSolarDay() { return m_FilterSolarDay; }
        bool            GetIgnoreBadFiles() { return m_IgnoreBadFiles; }
        bool            GetInterlace() { return m_Interlace; }
        size_t          GetJobs() { return m_Jobs; }
        bool            GetNoReconstruct() { return m_NoReconstruct; };
        bool            GetOverwrite() { return m_Overwrite; }
        bool            GetRecursive() { return m_Recursive; }
        bool            GetSaveMetadata() { return m_SaveMetadata; }
        bool            GetSolDirectorize() { return m_SolDirectorize; }
        bool            GetSummarizeOnly() { return m_SummarizeOnly; }

        // Set options...
        void            SetAutoRotate(const bool AutoRotate = true) { m_AutoRotate = AutoRotate; }
        void            SetDryRun(const bool DryRun = true) { m_DryRun = DryRun; }
        void            SetFilterCameraEvent(const std::string &CameraEvent);
        void            SetFilterDiodeClass(const std::string &DiodeClass);
        void            SetFilterLander(const size_t Lander);
        void            SetFilterSolarDay(const size_t SolarDay) { m_FilterSolarDay = SolarDay; }
        void            SetIgnoreBadFiles(const bool IgnoreBadFiles = true) { m_IgnoreBadFiles = IgnoreBadFiles; }
        void            SetInterlace(const bool Interlace = true) { m_Interlace = Interlace; }
        void            SetJobs(const size_t Jobs) { m_Jobs = Jobs; }
        void            SetNoReconstruct(const bool NoReconstruct = true) { m_NoReconstruct = NoReconstruct; }
        void            SetOverwrite(const bool Overwrite = true) { m_Overwrite = Overwrite; }
        void            SetRecursive(const bool Recursive = true) { m_Recursive = Recursive; }
        void            SetSaveMetadata(const bool SaveMetadata = true) { m_SaveMetadata = SaveMetadata; }
        void            SetSolDirectorize(const bool SolDirectorize = true) { m_SolDirectorize = SolDirectorize; }
        void            SetSummarizeOnly(const bool SummarizeOnly = true) { m_SummarizeOnly = SummarizeOnly; }
        
    // Protected methods...
    protected:

        // Default constructor...
        Options();
        
        // Deconstructor...
       ~Options();

    // Protected data...
    protected:

        // Use OCR to try and figure out correct image orientation...
        bool                m_AutoRotate;

        // Don't actually write out any files...
        bool                m_DryRun;

        // Filter based on matching camera event ID, such as 22A158...
        std::string         m_FilterCameraEvent;
            
        // Acceptable diode band filter set...
        FilterDiodeBandSet  m_FilterDiodeBandSet;

        // Filter by lander number, 0 being either...
        size_t              m_FilterLander;

        // Filter by solar day...
        size_t              m_FilterSolarDay;

        // Don't stop processing files when you hit a bad one...
        bool                m_IgnoreBadFiles;
        
        // Use Adam7 interlacing...
        bool                m_Interlace;

        // Number of threads to use...
        size_t              m_Jobs;

        // Don't attempt to reconstruct camera events, just dump all 
        //  available band data as separate images...
        bool                m_NoReconstruct;

        // Overwrite output files...
        bool                m_Overwrite;

        // Recursively scan subdirectories if the input is a directory...
        bool                m_Recursive;
        
        // Save metadata...
        bool                m_SaveMetadata;

        // Place reconstructed images in a subdirectory on of the solar 
        //  day they were taken on...
        bool                m_SolDirectorize;
        
        // Mute all console output channels, except the summary channel...
        bool                m_SummarizeOnly;

    // Private data...
    private:

        // Singleton instance...
        static Options     *m_SingletonInstance;
};

// Multiple include protection...
#endif

