#! /bin/bash

##
# An experimental shell script to compile and install my fork of the Saxon/C library. My fork's only change from the
# upstream is that it puts the Saxon PHP classes in a "Saxon" namespace so they don't conflict with the standard PHP
# XSLT classes.  Saxon will resolve this issue in their next release (https://saxonica.plan.io/issues/2380), but I
# need an immediate solution so have created this temporary fork to be used until the next official Saxon/C release.
#
# This script has not been tested across a lot of different platforms yet... just with Ubuntu 15.04, 14.04 (LTS) and
# CentOS/RHEL 6.6.
##

# Project specific installation directories
SAXON_INSTALL_DIR="/opt/saxon-c"
JET_INSTALL_DIR="/opt/jet10.5-eval-amd64"

# Check if we're running as root and if not run things using sudo
if (( $EUID != 0 )); then
  SUDO='sudo -E'
else
  SUDO=''
fi

# Check which type of installer our system has and set some variables (and do some extra work) based on that
! hash apt-get 2>/dev/null || {
  # We're probably using an Ubuntu/Debian-based image -- for this, we just need to set some paths/variables
  INSTALLER="apt-get"; QUIET_FLAG="-qq"
  SYSTEM_JAVA_HOME="/usr/lib/jvm/java-7-openjdk-amd64"
  ENV_VARS="/etc/apache2/envvars"; APACHE="apache2"
  SAXON_INI="/etc/php5/apache2/conf.d/20-saxon.ini"
  SYSTEM_PKGS="openjdk-7-jdk $APACHE gcc-multilib php5 php5-dev"

  $SUDO apt-get update -y
}
! hash yum 2>/dev/null || {
  # We're probably using a CentOS/RHEL-based image -- for this, we need to upgrade some packages + set paths
  INSTALLER="yum"; QUIET_FLAG="-q"
  MULTILIB="gcc gcc-c++ $(yum list 'compat-gcc-*-c++' | tail -n1 | cut -d ' ' -f 1)"
  DEPLIBS="libX11.i686 libXext.i686 libXrender.i686 libXi.i686 libXtst.i686 libgcc.i686"
  SYSTEM_JAVA_HOME="/usr/lib/jvm/java-1.7.0"
  ENV_VARS="/etc/sysconfig/httpd"; APACHE="httpd"
  SAXON_INI="/etc/php.d/saxon.ini"

  # We need to use the EPEL repository to pull in some additional packages, so we install and configure that
  $SUDO yum install -y epel-release
  MAJOR_VERSION=$(cat /etc/redhat-release | grep -Po "\d.\d" | cut -d '.' -f 1)

  # If we're using an older version of RHEL/CentOS, we need to upgrade our PHP version to 5.5
  if [ "$MAJOR_VERSION" -lt "7" ]; then
    $SUDO rpm -Uvh "http://mirror.webtatic.com/yum/el${MAJOR_VERSION}/latest.rpm"

    # Get a list of all installed PHP packages and uninstall them before configuring the new PHP package repo
    yum list installed | grep php | while read -r LINE; do echo $LINE | cut -d ' ' -f 1; done > /tmp/php-list.txt

    if [ -s "/tmp/php-list.txt" ]; then
      $SUDO yum remove -y $(cat /tmp/php-list.txt)
      # Then reinstall the packages that were previously uninstalled by this script
      $SUDO yum install -y $(cat /tmp/php-list.txt | sed -e "s|55w||" | sed -e "s|php|php55w|")
    fi

    PHP_DEVEL_VERSION="php55w php55w-devel"
  else
    PHP_DEVEL_VERSION="php php-devel"
  fi

  # These are the packages we'll need to install
  SYSTEM_PKGS="--enablerepo=epel java-1.7.0-openjdk-devel $APACHE $PHP_DEVEL_VERSION $MULTILIB $DEPLIBS"
}

# Just confirm we found one of the two supported package managers
if [ -z "$INSTALLER" ]; then
  echo "  I didn't find 'yum' or 'apt-get' installed on this machine."
  echo "  It seems this script doesn't yet support your system. Sorry!"
  exit 1
fi

# Excelsior JET evaluation download information
JET_BIN="/tmp/jet-1050-eval-en-linux-amd64-reg.bin"
JET_LINK="download/release/10.5/eval/linux/.*amd64-reg.bin"

# Change level of output depending on what we're doing
if [ "$1" = "clean" ]; then
  INSTALLER_ARGS="-y $QUIET_FLAG"
else
  INSTALLER_ARGS="-y"
fi

# General bootstrap stuff for Saxon/C install
$SUDO $INSTALLER install $INSTALLER_ARGS $SYSTEM_PKGS wget re2c

# We need to set JAVA_HOME but cannot yet assume we are using the default Java
#   JAVA_HOME=$(readlink -f /usr/bin/java | sed "s:bin/java::")
#
# Instead, we'll hard code JDK 7 until there is JDK 8 support in Excelsior JET
if [ -d "$SYSTEM_JAVA_HOME" ]; then
  export JAVA_HOME="$SYSTEM_JAVA_HOME"
else
  echo "JAVA_HOME not set as expected. Investigate!"
  exit 1
fi

# Clean out all artifacts and system changes from previous Saxon/C builds
SAXON_SO=$($SUDO find /usr/lib* -name "saxon.so")
$SUDO rm -rf $SAXON_INSTALL_DIR $JET_INSTALL_DIR
$SUDO rm -f "/usr/lib/libsaxon.so" "/lib64/libjvm.so" $SAXON_SO
$SUDO rm -f "$SAXON_INI" "/etc/ld.so.conf.d/jetvm.conf"
$SUDO sed -i "s|export JAVA_HOME=$JAVA_HOME||" $ENV_VARS
$SUDO sed -i "s|$JET_INSTALL_DIR/bin||" $ENV_VARS

# If we've supplied the "clean" argument, we are just doing an uninstall and can stop here
if [ "$1" = "clean" ]; then
  echo "Saxon/C has been uninstalled"
  exit 0
fi

# If we didn't find the "clean" argument we are doing a new build/installation of Saxon/C
echo "Installing Saxon/C"

# Download the Excelsior JET program that's needed to compile Saxon/C into native code
BIN=$(curl -s http://www.excelsiorjet.com/evaluate | grep -Po $JET_LINK)
wget -q --inet4-only -O - http://www.excelsiorjet.com/$BIN > $JET_BIN

# Confirm that we have successfully downloaded JET
if [ -s "$JET_BIN" ]; then
  echo "JET successfully downloaded"
else
  echo "Failed to download JET"
  exit 1
fi

# Install Excelsior JET
$SUDO chmod 700 $JET_BIN
$SUDO $JET_BIN -batch -dest $JET_INSTALL_DIR

# Create our working directory
$SUDO mkdir -p $SAXON_INSTALL_DIR
$SUDO cp -r . $SAXON_INSTALL_DIR
cd $SAXON_INSTALL_DIR

# Compile our libsaxon.so and put it in /usr/lib
$SUDO chmod 700 build_libsaxon-HEC.sh
$SUDO PATH=$JET_INSTALL_DIR/bin/:$PATH ./build_libsaxon-HEC.sh
$SUDO cp libsaxon.so /usr/lib/libsaxon.so

# Then we configure our system so it can find the JET shared libraries
$SUDO tee /etc/ld.so.conf.d/jetvm.conf > /dev/null << JETVM_CONFIG_EOF

$JET_INSTALL_DIR/lib/x86/shared
$SAXON_INSTALL_DIR/Saxon-C-API/modules

JETVM_CONFIG_EOF

# Run `ldconfig` (see the note at the end of this file)
$SUDO ldconfig

# Link some files that seem to be needed but in some cases can't be found
if [ ! -e /lib64/libjvm.so ]; then
  $SUDO ln -s $JAVA_HOME/jre/lib/amd64/server/libjvm.so /lib64/libjvm.so
fi
if [ ! -e $SAXON_INSTALL_DIR/Saxon-C-API/jni.h ]; then
  $SUDO ln -s $JAVA_HOME/include/jni.h $SAXON_INSTALL_DIR/Saxon-C-API/jni.h
fi
if [ ! -e $SAXON_INSTALL_DIR/Saxon-C-API/jni_md.h ]; then
  $SUDO ln -s $JAVA_HOME/include/linux/jni_md.h $SAXON_INSTALL_DIR/Saxon-C-API/jni_md.h
fi

# Change into the Saxon PHP API directory
cd $SAXON_INSTALL_DIR/Saxon-C-API

# Uncomment out the necessary Saxon PHP header before compiling
$SUDO sed -i 's/\/\/\#include \"php_saxon\.h\"/\#include \"php_saxon\.h\"/' xsltProcessor.cc

# Now compile Saxon for PHP
$SUDO phpize
$SUDO ./configure --enable-saxon
$SUDO make install | tee /tmp/saxon-c-install.log
PHP_EXTENSIONS=$(grep -Po "Installing shared extensions:.*" /tmp/saxon-c-install.log | rev | cut -d " " -f 1 | rev)

# Create and modify the Apache config files needed for Saxon/C's PHP to run
echo "extension=saxon.so" | $SUDO tee $SAXON_INI > /dev/null
echo "export JAVA_HOME=$JAVA_HOME" | $SUDO tee -a $ENV_VARS > /dev/null

CONFIGS=("$ENV_VARS" "/etc/profile")
for CONFIG in ${CONFIGS[@]}; do
  if [ $(grep -c "^export PATH=" $CONFIG) = "0" ]; then
    echo "export PATH=\$PATH:$JET_INSTALL_DIR/bin" | $SUDO tee -a $CONFIG > /dev/null
  else
    $SUDO sed -i "s|export PATH=|export PATH=$JET_INSTALL_DIR/bin:|" $CONFIG
  fi

  if [ $(grep -c "^export LD_LIBRARY_PATH=" $CONFIG) = "0" ]; then
    echo "export LD_LIBRARY_PATH=$PHP_EXTENSIONS:$JAVA_HOME/jre/lib:/lib64:/usr/lib" | $SUDO tee -a $CONFIG > /dev/null
  else
    $SUDO sed -i "s|$PHP_EXTENSIONS||" $CONFIG
    $SUDO sed -i "s|$JAVA_HOME/jre/lib||" $CONFIG
    $SUDO sed -i "s|/lib64||" $CONFIG
    $SUDO sed -i "s|export LD_LIBRARY_PATH=|export LD_LIBRARY_PATH=$PHP_EXTENSIONS:$JAVA_HOME/jre/lib:/lib64:|" $CONFIG
    # Lastly, let's do a little cleanup of search and replace cruft
    $SUDO sed -i "s|:\+|:|g" $CONFIG
  fi

  # Do a little formatting cleanup on the config file just to keep it nice and clean
  $SUDO sed -i '$!N; /^\(.*\)\n\1$/!P; D' $CONFIG
done

# Restart Apache to pick up our changes
if [ -z $(pidof -s $APACHE) ]; then $SUDO service $APACHE start; else $SUDO service $APACHE restart; fi

#
# Post-Install Notes:
#
# There is some weirdness with running:
#   vagrant@islandora:/opt/saxon_c/Saxon-C-API$ sudo ldconfig
#
# Output:
#   /sbin/ldconfig.real: file /usr/lib/libsaxon.so is truncated
#
# The Saxon developers on the mailing list seem to think this is okay, though, and it still does work...
#   http://sourceforge.net/p/saxon/mailman/message/33490428/
#
