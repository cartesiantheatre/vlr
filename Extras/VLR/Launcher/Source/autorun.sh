#!/usr/bin/env bash
# VikingExtractor, to recover images from Viking Lander operations.
# Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
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
#
#
#    Maintainer Notes
#    ================
#
#    This script is automatically run when the volume is mounted, if enabled, as
#    per XDG's Desktop Application Autostart Specification. The goal is to try
#    to bootstrap the actual target application by ensuring the user has a sane
#    execution environment with all of the necessary hard dependencies
#    (packages) pre-installed. What they don't have, they need to be informed of
#    in a way that is meaningful on their specific distro to make their life
#    simple (e.g. Ubuntu Software Centre). These are the runtimes that are
#    needed:
#
#        Hard Dependencies
#        =================
#
#        * Zenity (zenity)
#
#        * Python 3 (python3)
#
#        * Python GObject bindings (python-gi >= 3.0), which should pull the 
#          distro's Gtk3+ runtimes on Ubuntu Precise and GDBus bindings...
#
#        Soft Dependencies
#        =================
#
#        * GStreamer 1.0 base plugins as a soft dependency since at time of 
#          authoring gir1.2-gst-plugins-base-1.0 did not ship pre-installed on 
#          Ubuntu, up to and including Precise (12.04)
#

# Useful constants...

    # Disc name and version...
    Title="Avaneya: Viking Lander Remastered DVD"
    
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
    VT100Reset=$'\033[0m'
    VT100Bold=$'\033[1m'
    VT100ColourRed=$'\033[31m'
    VT100ColourGreen=$'\033[32m'
    VT100ColourBlue=$'\033[34m'
    
    # Status symbol constants...
    SymbolStatusOk="${VT100Bold}${VT100ColourGreen}✓${VT100Reset}"
    SymbolStatusFail="${VT100Bold}${VT100ColourRed}✗${VT100Reset}"

    # The lock file containing the process ID... (FHS 5.9.1)
    ProcessIDFile="/var/lock/vlr-lock"

    # Complete path to the directory containing this script...
    #AutostartScriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    AutostartScriptDirectory="$(dirname -- "$(readlink -f -- "$0")")"

    # Complete path to launcher entry point......
    PythonLauncherMain=""
    
    # True if we were the only instance of this script and managed to set a 
    #  lock...
    OwnLock=false
    
    # Set to true if EXIT trap should attempt to eject removable media...
    EjectRequested=false

    # Zenity process ID...
    ZenityProcessID=0

# Array of packages the user is missing that we need...
declare -a PackagesMissing

# Find the complete path to the script containing the launcher's entry point...
FindLauncherMain()
{
    # Alert user...
    echo -n "Looking for launcher... "

    # Check for Source/Main.py first...
    if [ -f "$AutostartScriptDirectory/Source/Main.py" ] ; then
        echo "Source/Main.py " $SymbolStatusOk
        PythonLauncherMain=$AutostartScriptDirectory/Source/Main.py
    
    # Nope. Check in the same directory...
    elif [ -f "$AutostartScriptDirectory/Main.py" ] ; then
        echo "Main.py " $SymbolStatusOk
        PythonLauncherMain=$AutostartScriptDirectory/Main.py

    # Couldn't find it anywhere...
    else
	    echo $SymbolStatusFail
        zenity --error \
            --title="Error" \
            --window-icon=$Icon \
            --text="Unable to find the Viking Lander Remastered application."
	    exit 1
    fi
}

# Set the VikingExtractor's locale directory environment variable to point to 
#  the DVD if found...
SetVikingExtractorLocaleDirectory()
{
    # Alert user...
    echo -n "Checking for local VikingExtractor l10n catalogue... "

    # Check within local media...
    if [ -f "$AutostartScriptDirectory/Extractor/share/locale/en_CA/LC_MESSAGES/viking-extractor.mo" ] ; then
        VE_LOCALE_DIR="$AutostartScriptDirectory/Extractor/share/locale"
        echo "$VE_LOCALE_DIR " $SymbolStatusOk

    # Guess systems...
    else
	    echo $SymbolStatusFail
    fi
}

# Signal trap to always run on exit... (e.g. ctrl-c)
OnExit()
{
    # Remove process ID file so no stale lock remains if we were the ones that
    #  locked it...
    if $OwnLock ; then
        rm -f "$ProcessIDFile"
    fi

    # Kill any instance of Zenity that might be still executing...
    KillZenity
    
    # If anything requested we eject the removable media on exit, do so now...
    if $EjectRequested ; then
        EjectRemovableMedia
    fi
}

# Kill Zenity...
KillZenity()
{
    # Kill any Zenity GUI if still open...
    if [ $ZenityProcessID != 0 ]; then
        
        # Send the terminate signal...
        kill $ZenityProcessID
        
        # Wait gracefully for it to exit...
        wait $ZenityProcessID 2> /dev/null
    fi
    ZenityProcessID=0
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
	    echo $SymbolStatusFail
		echo "This autostart script is only supported on GNU/Linux..."
		exit 1
	fi

    # If lsb_release is present, this is easy to get distro and code name...
    if [ -x "`which lsb_release`" ]; then
        Distro=`lsb_release --id --short`
        DistroCodeName=`lsb_release --short --codename`
	    DistroPackageManager="apt"

    # Otherwise fallback to tedious method of checking for distro specific 
    #  signatures in /etc...
	else

	    echo $SymbolStatusFail
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
		    echo $SymbolStatusFail
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
    echo "${Distro} ${DistroCodeName} $SymbolStatusOk"
    echo "Native package manager is ${DistroPackageManager}..."
}

# Lock the script to ensure no other instances are running, otherwise silently
#  exit...
LockScript()
{
    # Alert user...
    echo -n "Checking for unique instance... "

    # The noclobber option ensures the redirect will fail if the file the stream
    #  is being redirected to already exists. This is safe because it is atomic.
    #  If successful, write our process ID out into our newly created lock...
    if ( set -o noclobber; echo "$$" > "$ProcessIDFile") 2> /dev/null; then
	    echo $SymbolStatusOk
	    OwnLock=true

    # Lock already exists. Check if still alive...
    else
        
        # Get the existing process ID...
        OtherProcessID="$(cat "${ProcessIDFile}")"

        # Cat couldn't read the process ID file so other process may be just
        #  about to remove it...
        if [ $? != 0 ]; then
    	    echo $SymbolStatusFail
            exit 1
        fi        

        # Check if the other process is still alive by seeing if it could
        #  receive signals without actually sending any...
        if ! kill -0 $OtherProcessID &>/dev/null; then

            # Process is gone so assume lock is stale...
            rm -rf "${ProcessIDFile}"
    	    echo $SymbolStatusFail
            echo "Removed stale lock..."

            # Try setting the lock again...
            LockScript

        # The lock is valid because the other process is active...
        else
    	    echo $SymbolStatusFail
    	    echo "Process" $OtherProcessID "is existing instance..."
            exit 1
        fi
    fi
}

# Convert sole argument to all lowercase...
LowerCase()
{
    echo "$1" | sed "y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/"
}

# Print banner...
PrintBanner()
{
    echo ""
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
    case "$Distro" in

        # Note: Do not place commas between packages! Bash treats them as part
        #       of the array element...

        # Ubuntu based...
        [Uu]buntu)

            # Which release?
            case "$DistroCodeName" in

                # Precise...
                "precise")
                    PackagesRequired=("python3-gi")
                ;;
                
                # Quantal...
                "quantal")
                    PackagesRequired=("python3-gi")
                ;;

                # Raring...
                "raring")
                    PackagesRequired=("python3-gi")
                ;;

                # Unknown release...
                *)
                    # Try to guess...
                    PackagesRequired=("python3-gi")
                    echo "Warning: Unknown $Distro release ($DistroCodeName)." \
                        "Guessing dependencies..."
                ;;

            esac
            ;;

        # Fedora based...
        [Ff]edora)

            # Which release?
            case "$DistroCodeName" in

                # Beefy Miracle...
                "Beefy Miracle")
                    PackagesRequired=("python3-gobject")
                ;;

                # Unknown release...
                *)
                    # Try to guess...
                    PackagesRequired=("python3-gobject")
                    echo "Warning: Unknown $Distro release ($DistroCodeName)." \
                        "Guessing dependencies..."
                ;;

            esac
            ;;

        # Debian...
        [Dd]ebian)

            # Which release?
            case "$DistroCodeName" in
                
                # Debian Wheezy. Squeeze doesn't have the latter two needed
                #  packages...
                "wheezy")
                    PackagesRequired=("python3-gi")
                ;;

                # Unknown release...
                *)
                    # Try to guess...
                    PackagesRequired=("python3-gi")
                    echo "Warning: Unknown $Distro release ($DistroCodeName)." \
                        "Guessing dependencies..."
                ;;

            esac
            ;;

        # Unknown distro...
        *)
            zenity --error \
                --title="Avaneya: Viking Lander Remastered" \
                --window-icon=$Icon \
                --text="I'm sorry, but your distribution is not supported."
            exit 1
            ;;
    esac

    # Number of packages that are always needed...
    local PackagesRequiredCount=${#PackagesRequired[*]}

    # Check that each required package is installed...
    for (( Index=0; Index<$PackagesRequiredCount; Index++)); do
        CheckPackageInstalled ${PackagesRequired[${Index}]}
    done
    KillZenity

    # Some packages still need to be installed...
    if [ ${#PackagesMissing[*]} -gt 0 ]; then

        # Print to console missing packages...
        echo "Packages which need to be installed... ${PackagesMissing[*]}"

        # Take appropriate resolution for distro...
        case "$Distro" in
        
            # Ubuntu...
            [Uu]buntu)
                
                # Prompt user for available options...
                zenity \
                    --question \
                    --title="$Title" \
                    --window-icon=$Icon \
                    --text="Welcome! This application needs the following additional software. Would you like to install it now?\n\n\t${PackagesMissing[*]}" \
                    --ok-label="Yes" \
                    --cancel-label="No"

                # User opted to install the packages...
                if [ $? == 0 ]; then
                    software-center ${PackagesMissing[*]}
                fi
            ;;

            # Some other distro...
            *)
                zenity \
                    --info \
                    --title="$Title" \
                    --window-icon=$Icon \
                    --text="Welcome! This application needs the following additional software you can install with your operating system's package manager. Once it is installed, just restart the application.\n\n\t${PackagesMissing[*]}" \
                    --ok-label="Ok"
            ;;
            
        esac

        # Eject the media only if we are running off of removable media...
        EjectRequested=1

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

# Takes a Debian package name as a single parameter and returns true if it is
#  installed. Also adds the name of the package if missing to PackagesMissing
#  global array...
CheckDebInstalled()
{
    # Get name of package to check as only argument...
    local Package=$1

    # Check if the package is installed...
    echo -n "Checking if $Package is installed... "
    TestInstalled=( `LANGUAGE="C" apt-cache policy $Package | grep "Installed:" ` )

    # Installed...
    if [[ (-n "${TestInstalled}") && ("${TestInstalled[1]}" != "(none)")]]
    then
        echo -e $SymbolStatusOk
        return 1

    # Found in the package database, but not installed, or not even found in 
    #  the package data, and therefore not installed...
    else
        echo -e $SymbolStatusFail
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
    $OldLanguage=$LANGUAGE
    LANGUAGE="C" yum list installed $Package &> /dev/null
    $LANGUAGE=$OldLanguage

    # Installed...
    if [ $? == 0 ]
    then
        echo -e $SymbolStatusOk
        return 1

    # Not installed...
    else
        echo -e $SymbolStatusFail
        PackagesMissing=("${PackagesMissing[*]}" $Package)
        return 0
    fi
}

# Eject the media only if we are running off of removable media...
EjectRemovableMedia()
{
    # Find the mount point it is located on...
    MountPoint=`df "$AutostartScriptDirectory" | awk 'NR==2{print $NF}'`

    # Alert user of our intentions...
    echo -n "Checking if" $MountPoint "is removable media..."

    # Eject if it is removable media. That is, under /media/* according to sect
    #  3.11.1 of the FHS...
    if [[ $MountPoint == /media/* ]]; then

        # Attempt to eject the media...
        echo -e $SymbolStatusOk
        echo "Attempting eject..."
        exec eject $MountPoint 2> /dev/null

        # Old approach: eject the disc by first trying CDROM eject method, then
        #  by SCSI method...
        # eject -r -s 2> /dev/null

        ## Report whether it was successful or not...
        #if [ $? == 0 ]; then
        #    echo -e $SymbolStatusOk
        #else
        #    echo -e $SymbolStatusFail
        #fi

    # Not running off of removable media, so don't try to eject...
    else
        echo -e $SymbolStatusFail
    fi
}

# Entry point...
Main()
{
    # Print banner...
    PrintBanner

    # Lock the script to ensure no other instances are running...
    LockScript

    # Zenity has come on most GNU distros...
    echo -n "Checking for zenity... "
    if [ ! -x "`which zenity`" ]; then
	    echo $SymbolStatusFail
	    exit 1
	else
	    echo $SymbolStatusOk
        echo "Using zenity `zenity --version`... "
    fi

    # Give user visual feedback as soon as possible that something is 
    #  happening...
    zenity --progress \
            --title="$Title" \
            --window-icon=$Icon \
            --text="Please wait a moment while the software loads..." \
            --pulsate \
            --auto-close \
            --no-cancel &
    ZenityProcessID=$!

    # Display user's locale...
    echo "User's LANG is... ${LANG}"
    echo "User's LANGUAGE is... ${LANGUAGE}"

    # Identify the user's distro...
    IdentifyDistro

    # Prepare the runtime environment as specific to user's distro...
    case "$Distro" in

        # Ubuntu, Debian, or some derivative...
        [Uu]buntu) ;&
        [Dd]ebian)
            PrepareDebianBased
        ;;

        # Fedora...
        # [Ff]edora)
        #    PrepareDebianBased
        #;;
        
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
    
    # Set the VikingExtractor locale directory environment if running off of
    #  disc and not in standard FHS..
    SetVikingExtractorLocaleDirectory

    # Run the launcher...
    echo -n "Launching GUI using... "

        # First try with Python 3...
        if [ -x "`which python3`" ]; then
            echo "python3 $SymbolStatusOk"
            /usr/bin/env python3 "${PythonLauncherMain}" "${Arguments[@]}"
        
        # ...and if that still doesn't work, then we're out of luck...
        else
            echo $SymbolStatusFail
            zenity --error \
                --title="Error" \
                --window-icon=$Icon \
                --text="I'm sorry, but I couldn't find your Python 3 runtime."
            exit 1
        fi

    # Done...
    exit 0
}

# Setup traps...
trap OnExit EXIT SIGINT SIGQUIT SIGSTOP

# Begin execution in Main...
Main;

