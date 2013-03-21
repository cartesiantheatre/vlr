#!/usr/bin/env bash
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <kip@thevertigo.com>.
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
#   * Zenity (zenity)
#   * Python 3 (python3)
#   * Python GObject bindings (python-gi >= 3.0), which, correct me if wrong,
#      includes or should pull the distro's Gtk3+ runtimes on Ubuntu Precise
#   * Python D-Bus interface
#   # C D-Bus runtime library
#

# Useful constants...

    # Disc name and version...
    Title="Avaneya: Viking Lander Remastered DVD"
    Version="0.1"
    
    # Preserve command line arguments because $@ is clobbered after calling any
    #  functions...
    Arguments=( "$@" )

    # Icon to use in windows...
    Icon=XDG/Avaneya.png

    # Distro name and codename...
    Distro=""
    DistroCodeName=""
    DistroPackageManager=""

    # VT100 terminal constants...
    VT100_RESET=$'\033[0m'
    VT100_BOLD=$'\033[1m'
    VT100_COLOUR_RED=$'\033[31m'
    VT100_COLOUR_GREEN=$'\033[32m'
    VT100_COLOUR_BLUE=$'\033[34m'
    
    # Status symbol constants...
    SYMBOL_STATUS_OK="${VT100_BOLD}${VT100_COLOUR_GREEN}✓${VT100_RESET}"
    SYMBOL_STATUS_FAIL="${VT100_BOLD}${VT100_COLOUR_RED}✗${VT100_RESET}"
    
    # Complete path to launcher entry point......
    PYTHON_LAUNCHER_MAIN=""

#Zenity process ID...
ZenityPID=0

# Array of packages the user is missing that we need...
declare -a PackagesMissing

# Find the complete path to the script containing the launcher's entry point...
FindLauncherMain()
{
    # Alert user...
    echo -n "Looking for launcher... "

    # Get the complete path to the directory containing this script...
    AUTORUN_SCRIPT_DIRECTORY="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

    # Check for Source/Main.py first...
    if [ -f "$AUTORUN_SCRIPT_DIRECTORY/Source/Main.py" ] ; then
        echo "Source/Main.py " $SYMBOL_STATUS_OK
        PYTHON_LAUNCHER_MAIN=$AUTORUN_SCRIPT_DIRECTORY/Source/Main.py
    
    # Nope. Check in the same directory...
    elif [ -f "$AUTORUN_SCRIPT_DIRECTORY/Main.py" ] ; then
        echo "Main.py " $SYMBOL_STATUS_OK
        PYTHON_LAUNCHER_MAIN=$AUTORUN_SCRIPT_DIRECTORY/Main.py

    # Couldn't find it anywhere...
    else
	    echo $SYMBOL_STATUS_FAIL
        zenity --error \
            --title="Error" \
            --window-icon=$Icon \
            --text="Unable to find the Viking Lander Remastered application."
	    exit 1
    fi
}

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

# Identify distro name and release code... $Distro and $DistroCodeName
IdentifyDistro()
{
    # Alert user...
    echo -n "Identifying distribution... "

	# Get the operating system family name...
	local OS=`uname`
	
	# Not running GNU...
	if [ "${OS}" != "Linux" ] ; then
	    echo $SYMBOL_STATUS_FAIL
		echo "This autorun script is only supported on GNU/Linux..."
		exit 1
	fi

    # If lsb_release is present, this is easy to get distro and code name...
    if [ -x "`which lsb_release`" ]; then
        Distro=`lsb_release --id --short`
        DistroCodeName=`lsb_release --short --codename`
	    DistroPackageManager="apt"

    # Otherwise fallback to tedious method of checking for distro specific signatures in /etc...
	else

	    echo $SYMBOL_STATUS_FAIL
		echo -n "Falling back to checking for distro specific signature in /etc... "

        # RedHat... (e.g. Fedora / CentOS)
	    if [ -f /etc/redhat-release ] ; then
		    Distro=`cat /etc/redhat-release |sed s/\ release.*//`
		    DistroCodeName=`cat /etc/redhat-release | sed s/.*\(// | sed s/\)//`
		    DistroPackageManager="yum"
	
	    # SuSe...
	    elif [ -f /etc/SuSE-release ] ; then
		    Distro='SuSe'
		    DistroCodeName=`cat /etc/SuSE-release | tr "\n" ' '| sed s/VERSION.*//`
		    DistroPackageManager="zypp"

	    # Mandrake...
	    elif [ -f /etc/mandrake-release ] ; then
		    Distro='Mandrake'
		    DistroCodeName=`cat /etc/mandrake-release | sed s/.*\(// | sed s/\)//`
		    DistroPackageManager="yum"

        # Debian...
	    elif [ -f /etc/debian_version ] ; then
		    Distro=`cat /etc/lsb-release | grep '^DISTRIB_ID' | awk -F=  '{ print $2 }'`
		    DistroCodeName=`cat /etc/lsb-release | grep '^DISTRIB_CODENAME' | awk -F=  '{ print $2 }'`
		    DistroPackageManager="apt"
		
        # Unknown...
		else
		    echo $SYMBOL_STATUS_FAIL
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="You need to install lsb_release before you can use this software. Use your distribution's package manager."
		    exit 1
	    fi
    fi

    # Lock distro identifiers...
 	readonly Distro
 	readonly DistroCodeName
 	readonly DistroPackageManager

    # Alert user of what we found...
    echo $SYMBOL_STATUS_OK
    echo "User is running ${Distro} / ${DistroCodeName}..."
    echo "System's packages are managed by ${DistroPackageManager}..."
}

# Convert sole argument to all lowercase...
LowerCase()
{
    echo "$1" | sed "y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/"
}

# Print banner...
PrintBanner()
{
    echo -e "${VT100_BOLD}${VT100_COLOUR_BLUE}${Title}, ${Version}${VT100_RESET}\n"
    echo -e "  Copyright (C) 2010-2013 Cartesian Theatre. This is free"
    echo -e "  software; see Copying for copying conditions. There is NO"
    echo -e "  warranty; not even for MERCHANTABILITY or FITNESS FOR A"
    echo -e "  PARTICULAR PURPOSE.\n"
}

# Prepare a Debian based system, such as Ubuntu...
PrepareDebianBased()
{
    # Array for a list of packages that's always needed...
    declare -a local PackagesRequired;

    # Calculate which packages we need...
    case "$DistroCodeName" in

        # Ubuntu...

            # Ubuntu precise...
            "precise")
                PackagesRequired=("python3-gi")
            ;;

        # Fedora...
            
            # Beefy Miracle...
            "Beefy Miracle")
                PackagesRequired=("python3-gobject")
            ;;

        # Debian...
        
            # Debian wheezy. Squeeze doesn't have the latter two needed
            #  packages...
            "wheezy")
                PackagesRequired=("python3-gi" "python3-dbus")
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

    # Check that each required package is installed...
    zenity --progress \
            --title="$Title" \
            --window-icon=$Icon \
            --text="Please wait while checking for required software..." \
            --pulsate \
            --auto-close \
            --no-cancel &
    ZenityPID=$!
    for (( Index=0; Index<$PackagesRequiredCount; Index++)); do
        CheckPackageInstalled ${PackagesRequired[${Index}]}
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
            echo -e $SYMBOL_STATUS_OK
        else
            echo -e $SYMBOL_STATUS_FAIL
        fi
        
        # Exit with error...
        exit 1

    # User has everything that they need...
    else
        echo "No packages needed to be installed..."
    fi
}

# Takes a package name as a single parameter and returns true if it is
#  installed. Also adds the name of the package if missing to PackagesMissing
#  global array...
CheckPackageInstalled()
{
    case "$DistroPackageManager" in

        # Apt based...
        apt)
            CheckDebInstalled $1
        ;;
        
        # Yum based...
        yum)
            CheckRPMInstalled $1
        ;;

        # Unknown...
        *)
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="I'm sorry, but your package manager is not supported..."
            exit 1
            ;;
    esac
}

# Takes a debian package name as a single parameter and returns true if it is
#  installed. Also adds the name of the package if missing to PackagesMissing
#  global array...
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
        echo -e $SYMBOL_STATUS_OK
        return 1

    # Found in the package database, but not installed, or not even found in 
    #  the package data, and therefore not installed...
    else
        echo -e $SYMBOL_STATUS_FAIL
        PackagesMissing=("${PackagesMissing[*]}" $Package)
        return 0
    fi
}

# Takes a RPM package name as a single parameter and returns true if it is
#  installed. Also adds the name of the package if missing to PackagesMissing
#  global array...
CheckRPMInstalled()
{
    # Get name of package to check as only argument...
    local Package=$1

    # Check if the package is installed...
    echo -n "Checking if $Package is installed... "
    yum list installed $Package &> /dev/null

    # Installed...
    if [ $? == 0 ]
    then
        echo -e $SYMBOL_STATUS_OK
        return 1

    # Not installed...
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

    # Setup traps...
    trap TrapInterrupt SIGINT

    # Zenity has come on most GNU distros...
    echo -n "Checking for zenity... "
    if [ ! -x "`which zenity`" ]; then
	    echo $SYMBOL_STATUS_FAIL
	    exit 1
	else
	    echo $SYMBOL_STATUS_OK
        echo "Using zenity `zenity --version`... "
    fi

    # Check for lsb_release...
    IdentifyDistro

    case "$Distro" in

        # Ubuntu, Debian, or some derivative...
        [Uu]buntu) ;&
        [Dd]ebian)
            PrepareDebianBased
        ;;

        # Fedora...
        [Ff]edora)
            PrepareDebianBased
        ;;
        
        # Unknown distro...
        *)
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="I'm sorry, but your distribution ${Distro} ${DistroCodeName} is not supported."
	        exit 1
            ;;
    esac

    # Find the launcher...
    FindLauncherMain

    # Run the launcher...
    echo -n "Launching GUI using... "

        # First try with Python 3...
        if [ -x "`which python3`" ]; then
            echo "python3 $SYMBOL_STATUS_OK"
            /usr/bin/env python3 "${PYTHON_LAUNCHER_MAIN}" "${Arguments[@]}"
        
        # ...if that doesn't work, try what's probably an alias for Python 2...
        elif [ -x "`which python`" ]; then
            echo "python $SYMBOL_STATUS_OK"
            /usr/bin/env python "${PYTHON_LAUNCHER_MAIN}" "${Arguments[@]}"
        
        # ...and if that still doesn't work, then we're out of luck...
        else
            echo $SYMBOL_STATUS_FAIL
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


