#! /bin/bash

##
# An experimental shell script to compile and install my fork of the Saxon/C library. My fork's only change from the
# upstream is that it puts the Saxon PHP classes in a "Saxon" namespace so they don't conflict with the standard PHP
# XSLT classes.  Saxon will resolve this issue in their next release (https://saxonica.plan.io/issues/2380), but I
# need an immediate solution so have created this temporary fork to be used until the next official Saxon/C release.
#
# This script has not been tested across a lot of different platforms yet... just with Ubuntu 14.04 (LTS) and 15.04.
##

# Installation directories (a library is also installed to /usr/lib but that's a standard location)
SAXON_INSTALL_DIR="/opt/saxon-c"
JET_INSTALL_DIR="/opt/jet10.5-eval-amd64"
ENV_VARS="/etc/apache2/envvars"

# Check if we're running as root and if not run things using sudo
if (( $EUID != 0 )); then
  SUDO='sudo -E'
else
  SUDO=''
fi

# Excelsior JET evaluation download information
JET_BIN="/tmp/jet-1050-eval-en-linux-amd64-reg.bin"
JET_LINK="download/release/10.5/eval/linux/.*amd64-reg.bin"

# General bootstrap stuff for Saxon/C install -- only supports JDK 7 for now
$SUDO apt-get install -y -qq wget gcc-multilib re2c openjdk-7-jdk

# We need to set JAVA_HOME but at this time don't want to assume we're using the default Java
#   JAVA_HOME=$(readlink -f /usr/bin/java | sed "s:bin/java::")
#
# Instead, we'll hard code JDK 7 until there is JDK 8 support in Excelsior JET
if [ -d "/usr/lib/jvm/java-7-openjdk-amd64" ]; then
  export JAVA_HOME="/usr/lib/jvm/java-7-openjdk-amd64"
else
  echo "JAVA_HOME not set as expected. Investigate!"
  exit 1
fi

# Clean out all artifacts and system changes from previous Saxon/C builds
$SUDO rm -rf $SAXON_INSTALL_DIR $JET_INSTALL_DIR
$SUDO rm -f "/usr/lib/libsaxon.so" "/usr/lib/php5/*/saxon.so" "/lib64/libjvm.so"
$SUDO rm -f "/etc/php5/apache2/conf.d/20-saxon.ini" "/etc/ld.so.conf.d/jetvm.conf"
$SUDO sed -i "s|export JAVA_HOME=$JAVA_HOME||" $ENV_VARS
$SUDO sed -i "s|$JET_INSTALL_DIR/bin||" $ENV_VARS

if [ "$1" = "clean" ]; then
  echo "Saxon/C has been uninstalled"
  exit 0
fi

echo "Installing a new version of Saxon/C"

# Download the Excelsior JET program that's needed to compile Saxon/C into native code
BIN=$(curl -s http://www.excelsiorjet.com/evaluate | grep -Po $JET_LINK)
wget -q -T 5 -t 1 --inet4-only -O - http://www.excelsiorjet.com/$BIN > $JET_BIN

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
PHP_EXTENSIONS=$(grep -Po "Installing shared extensions:.*" /tmp/saxon-c-install.log | rev | cut -d " " -f1 | rev)

# Create and modify the Apache config files needed for Saxon/C's PHP to run
echo "extension=saxon.so" | $SUDO tee /etc/php5/apache2/conf.d/20-saxon.ini > /dev/null
echo "export JAVA_HOME=$JAVA_HOME" | $SUDO tee -a $ENV_VARS > /dev/null

if [ $(grep -c "^export PATH=" $ENV_VARS) = "0" ]; then
  echo "export PATH=\$PATH:$JET_INSTALL_DIR/bin" | $SUDO tee -a $ENV_VARS > /dev/null
else
  $SUDO sed -i "s|export PATH=|export PATH=$JET_INSTALL_DIR/bin:|" $ENV_VARS
fi

if [ $(grep -c "^export LD_LIBRARY_PATH=" $ENV_VARS) = "0" ]; then
  echo "export LD_LIBRARY_PATH=$PHP_EXTENSIONS:$JAVA_HOME/jre/lib:/lib64" | $SUDO tee -a $ENV_VARS > /dev/null
else
  $SUDO sed -i "s|$PHP_EXTENSIONS||" $ENV_VARS
  $SUDO sed -i "s|$JAVA_HOME/jre/lib||" $ENV_VARS
  $SUDO sed -i "s|/lib64||" $ENV_VARS
  $SUDO sed -i "s|export LD_LIBRARY_PATH=|export LD_LIBRARY_PATH=$PHP_EXTENSIONS:$JAVA_HOME/jre/lib:/lib64:|" $ENV_VARS
  # Lastly, let's do a little cleanup of search and replace cruft
  $SUDO sed -i "s|:\+|:|g" $ENV_VARS
fi

# Do a little formatting cleanup on the Apache config file just to keep it nice and clean
$SUDO sed -i '$!N; /^\(.*\)\n\1$/!P; D' $ENV_VARS

# Restart Apache to pick up our changes
$SUDO service apache2 restart

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
