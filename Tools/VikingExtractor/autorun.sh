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

    # Disc name and version...
    Title="Avaneya: Viking Lander Remastered DVD"
    Version="0.1"
    
    # Icon to use in windows...
    Icon=XDG/Avaneya.png

    # Distro name and codename...
    Distro=""
    DistroCodeName=""

    # VT100 terminal constants...
    VT100_RESET=$'\033[0m'
    VT100_BOLD=$'\033[1m'
    VT100_COLOUR_RED=$'\033[31m'
    VT100_COLOUR_GREEN=$'\033[32m'
    VT100_COLOUR_BLUE=$'\033[34m'
    
    # Status constants...
    STATUS_OK="${VT100_BOLD}${VT100_COLOUR_GREEN}✓${VT100_RESET}"
    STATUS_FAIL="${VT100_BOLD}${VT100_COLOUR_RED}✗${VT100_RESET}"
    
    # Local path from disc root to navigation menu...
    PYTHON_NAVMENU_MAIN=./Navigator/Main.py

#Zenity process ID...
ZenityPID=0

# Array of packages the user is missing that we need...
declare -a PackagesMissing

# Trap an interrupt... (e.g. ctrl-c)
TrapInterrupt()
{
    # Alert user...
    echo "Trap detected. Cleaning up..."
    
    # Kill any instance of Zenity that might be still executing...
    KillZenity
}

# Kill Zenity...
KillZenity()
{
    # Kill any Zenity GUI if still open...
    if [ ZenityPID != 0 ]; then
        kill $ZenityPID
        wait $ZenityPID 2> /dev/null
    fi
    ZenityPID=0
}

# Print banner...
PrintBanner()
{
    echo -e "${VT100_BOLD}${VT100_COLOUR_BLUE}${Title}, ${Version}${VT100_RESET}\n"
    echo -e "  Copyright (C) 2010, 2011, 2012 Cartesian Theatre. This is free"
    echo -e "  software; see Copying for copying conditions. There is NO"
    echo -e "  warranty; not even for MERCHANTABILITY or FITNESS FOR A"
    echo -e "  PARTICULAR PURPOSE.\n"
}

# Prepare a Debian based system, such as Ubuntu...
PrepareDebianBased()
{
    # Array for a list of packages that are always needed...
    declare -a local PackagesRequired;

    # Calculate which packages we need...
    case "$DistroCodeName" in

        # Ubuntu precise...
        precise)
            PackagesRequired=("python3" "python-gi")
        ;;
        
        # Debian squeeze...
        squeeze)
            PackagesRequired=("python3" "python-gtk2")
        ;;

        # Unknown distro...
        *)
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="I'm sorry, but your distribution is not supported:"
            exit 1
            ;;
    esac


    # Number of packages that are always needed...
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
    KillZenity

    # Some packages still need to be installed...
    if [ ${#PackagesMissing[*]} -gt 0 ]; then
        
        # Alert user of missing packages...
        echo "Packages which need to be installed... ${PackagesMissing[*]}"
        zenity \
            --info \
            --title="$Title" \
            --window-icon=$Icon \
            --text="Welcome! You need to install the following software from your operating system's package manager before you can use this disc. Once the software is installed, just restart the application.\n\n\t${PackagesMissing[*]}" \
            --ok-label="Ok"

        # Eject the disc, first try by CDROM eject method, then by SCSI method...
        echo -n "Eject disc... "
        eject -r -s 2> /dev/null
        if [ $? == 0 ]; then
            echo -e $STATUS_OK
        else
            echo -e $STATUS_FAIL
        fi

    # User has everything that they need...
    else
        echo "No packages needed to be installed..."
    fi
    
    # TODO: Implement the launching of the navigation menu GUI here...
}

# Takes a debian package name as a single parameter and returns true if it is
#  installed. This should work on all Debian based distros, including Ubuntu...
CheckDebInstalled()
{
    # Get name of package to check as only argument...
    local Package=$1

    # Check if the package is installed...
    echo -n "Checking if $Package is installed... "
    test_installed=( `apt-cache policy $Package | grep "Installed:" ` )

    # Installed...
    if [[ (-n "${test_installed}") && ("${test_installed[1]}" != "(none)")]]
    then
        echo -e $STATUS_OK
        return 1

    # Found in the package database, but not installed, or not even found in 
    #  the package data, and therefore not installed...
    else
        echo -e $SYMBOL_FAIL
        PackagesMissing=("${PackagesMissing[*]}" $Package)
        return 0
    fi
}

# Entry point...
Main()
{
    # Print banner...
    PrintBanner

    # Zenity has come on every Ubuntu distro since at least 10.04...
    echo -n "Checking for zenity..."
    if [ ! -x "`which zenity`" ]; then
	    echo $STATUS_FAIL
	    exit 1
	else
	    echo $STATUS_OK
        echo "Using zenity `zenity --version` ..."
    fi

    # Check for lsb_release...
    echo -n "Checking for lsb_release... "
    if [ ! -x "`which lsb_release`" ]; then
        echo $STATUS_FAIL
        zenity --error \
            --title="Error" \
            --window-icon=$Icon \
            --text="You need to install lsb_release before you can use this software. Use your distribution's package manager."
	    exit 1
	else
	    echo $STATUS_OK
    fi

    # Setup traps...
    trap TrapInterrupt SIGINT

    # Prepare for a given distro...
    Distro=`lsb_release --id --short`
    DistroCodeName=`lsb_release --short --codename`
    case "$Distro" in

        # Ubuntu, Debian, or some derivative...
        [Uu]buntu) ;&
        [Dd]ebian)
            echo "Ubuntu or Debian distribution... (${Distro} ${DistroCodeName})"
            PrepareDebianBased
        ;;
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
                --text="I'm sorry, but your distribution ${Distro} ${DistroCodeName} is not supported."
	        exit 1
            ;;
    esac

    # Run the navigation menu...
    echo -n "Finding best available Python runtime... "
    
        # First try with Python 3...
        if [ -x "`which python3`" ]; then
            echo $STATUS_OK
            /usr/bin/env python3 ${PYTHON_NAVMENU_MAIN}
        
        # ...if that doesn't work, try what's probably an alias for Python 2...
        elif [ -x "`which python`" ]; then
            echo $STATUS_OK
            /usr/bin/env python ${PYTHON_NAVMENU_MAIN}
        
        # ...and if that still doesn't work, then we're out of luck...
        else
            echo $STATUS_FAIL
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="I'm sorry, but I couldn't detect your Python runtime."
            exit 1
        fi

    # Done...
    exit 0
}

# Begin execution in Main...
Main;

