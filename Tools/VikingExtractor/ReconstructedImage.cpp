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
#include "ReconstructedImage.h"
#include "Console.h"
#include <cassert>
#include <iostream>


// Using the standard namespace...
using namespace std;

// Helpful macro...
#define SetErrorAndReturn(Message)      { SetErrorMessage((Message)); return; }
#define SetErrorAndReturnFalse(Message) { SetErrorMessage((Message)); return false; }

// Constructor...
ReconstructableImage::ReconstructableImage(const std::string &CameraEventIdentifier)
{

}

// Add an image band...
void ReconstructableImage::AddImageBand(const VicarImageBand &ImageBand)
{

}

// Get a suggested file name for the reconstructed image...
std::string ReconstructableImage::GetSuggestedFileName() const
{

}

// Extract the image out as a PNG, or return false if failed...
bool ReconstructableImage::Reconstruct(const string &OutputFile) const
{

}

