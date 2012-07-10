#!/usr/bin/env bash
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010, 2011, 2012 Cartesian Theatre <kip@thevertigo.com>.
#
# Public discussion on IRC available at #avaneya (irc.freenode.net) or
# on the mailing list <avaneya@lists.avaneya.com>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or 
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# This script is automatically run when the volume is mounted, if enabled, as 
# per XDG's Desktop Application Autostart Specification. The goal is to ensure 
# that the user has a sane execution environment that has the necessary 
# preinstalled packages. What they don't have, they need to be informed of in a 
# way that makes their life simple. These are the runtimes that are needed:
#
# * Zenity (zenity)
# * Python 3 (python3)
# * Python GObject bindings (python-gi >= 3.0)
#

# Useful constants...
Title="Avaneya: Viking Lander Remastered"
Icon=XDG/Avaneya.png

# Array of packages the user is missing that we need...
declare -a PackagesMissing

# Prepare an Ubuntu system...
function PrepareUbuntu
{
    # List of packages that are needed...
    declare -a local PackagesRequired=("python" "python-gi" "trousers");
    local PackagesRequiredCount=${#PackagesRequired[*]}

    # Use either gksudo or kdesudo to run a command as root, whichever is 
    #  available...
    if [ -x "`which gksudo`" ]; then
        SudoGui=gksudo
    elif [ -x "`which kdesudo`" ]; then
        SudoGui=kdesudo
    else
        zenity \
            --error \
            --title="Error" \
            --window-icon=$Icon \
            --text="I couldn't detect how to ask you for your password to install any needed software." \
        exit 1
    fi  
    echo "Will use $SudoGui as sudo frontend..."

    # Check that each required package is installed...
    zenity --progress \
            --title="$Title" \
            --window-icon=$Icon \
            --text="Checking for required software. Please wait..." \
            --pulsate \
            --auto-close \
            --no-cancel &
    ZenityPID=$!
    for (( Index=0; Index<$PackagesRequiredCount; Index++)); do
        CheckDebInstalled ${PackagesRequired[${Index}]}
    done
    kill $ZenityPID

    # Some packages still need to be installed...
    if [ ${#PackagesMissing[*]} -gt 0 ]; then
        
        # Alert user and ask them if they want to install them...
        echo "Packages which need to be installed... ${PackagesMissing[*]}"
        zenity \
            --question \
            --title="$Title" \
            --window-icon=$Icon \
            --text="I need to install the following software before you can use me. You will need a working internet connection:\n\n\t${PackagesMissing[*]}" \
            --ok-label="Install"

        # User cancelled, so exit...
        if [ $? != 0 ]; then
            echo "User cancelled..."
            exit 1
        
        # Otherwise install the packages...
        else
            zenity --progress \
                --title="$Title" \
                --text="Updating software database, please wait..." \
                --pulsate \
                --no-cancel \
                --auto-close &
            ZenityPID=$!

            # Graphical password prompt and dialog as update executes...
            $SudoGui "apt-get update"
            kill $ZenityPID

            zenity --progress \
                --title="$Title" \
                --text="Installing required software, please wait..." \
                --pulsate \
                --no-cancel \
                --auto-close &
            ZenityPID=$!

            $SudoGui "apt-get -y install ${PackagesMissing[*]}"
            kill $ZenityPID
        fi

    # User has everything that they need...
    else
        echo "No packages need to be installed..."
    fi
    
    # TODO: Implement the launching of the navigation menu GUI here...
}

# Takes a debian package name as a single parameter and returns true if it is
#  installed. This should work on all Debian based distros, including Ubuntu...
function CheckDebInstalled
{
    # Get name of package to check as only argument...
    local Package=$1

    # Check if the package is installed...
    echo -n "Checking if $Package is installed... "
    test_installed=( `apt-cache policy $Package | grep "Installed:" ` )

    # Installed...
    if [[ (-n "${test_installed}") && ("${test_installed[1]}" != "(none)")]]
    then
        echo "✓"
        return 1

    # Found in the package database, but not installed, or not even found in 
    #  the package data, and therefore not installed...
    else
        echo "✗"
        PackagesMissing=("${PackagesMissing[*]}" $Package)
        return 0
    fi
}

# Entry point...
function Main
{
    # Zenity has come on every Ubuntu distro since at least 10.04...
    if [ ! -x "`which zenity`" ]; then
	    echo "You need to install zenity before you can use this software."
	    exit 1
	else
        echo "Zenity `zenity --version` detected..."
    fi

    # Check for lsb_release...
    if [ ! -x "`which lsb_release`" ]; then
        zenity --error \
            --title="Error" \
            --window-icon=$Icon \
            --text="You need to install lsb_release before you can use this software. Use your distribution's package manager."
	    exit 1
    fi

    # Prepare for a given distro...
    local Distro=`lsb_release -is`
    case "$Distro" in

        # Ubuntu or derivative...
        [Uu]buntu)
            echo "Ubuntu distribution detected..."
            PrepareUbuntu;
            ;;
        
#        # Debian...
#        [Dd]ebian)
#            echo "Debian distribution..."
#        ;;
#        
#        # Fedora...
#        [Ff]edora)
#            echo "Fedora distribution..."
#        ;;
        
        # Unknown distro...
        *)
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="I'm sorry, but your distribution is not supported:"
	        exit 1
            ;;
    esac
    
    # Done...
    exit 0
}

# Being executing in Main...
Main;

