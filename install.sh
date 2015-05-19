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
JET_INSTALL_DIR="/opt/jet10.5-eval-amd64/"

# Excelsior JET evaluation download information
JET_BIN="/tmp/jet-1050-eval-en-linux-amd64-reg.bin"
JET_LINK="download/release/10.5/eval/linux/.*amd64-reg.bin"

# Make sure we have JAVA_HOME set before running this script
if ! test $JAVA_HOME; then echo "Set JAVA_HOME before running this script"; exit 1; fi

if [ "$1" = "clean" ]; then
	echo "Cleaning previous build"
	sudo rm -rf $SAXON_INSTALL_DIR $JET_INSTALL_DIR /usr/lib/libsaxon.so
fi

echo "Installing Saxon/C"

# General bootstrap stuff for Saxon/C install
sudo apt-get install -y wget gcc-multilib re2c

# Download the JET program that's needed to compile Saxon/C into native code
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
sudo chmod 700 $JET_BIN
sudo $JET_BIN -batch -dest $JET_INSTALL_DIR

sudo mkdir $SAXON_INSTALL_DIR
sudo cp -r . $SAXON_INSTALL_DIR
cd $SAXON_INSTALL_DIR

# Compile our libsaxon.so and put it in /usr/lib
sudo chmod 700 build_libsaxon-HEC.sh
sudo PATH=$JET_INSTALL_DIR/bin/:$PATH ./build_libsaxon-HEC.sh
sudo cp libsaxon.so /usr/lib/libsaxon.so

# Then we configure our system so it can find the JET shared libraries
sudo tee /etc/ld.so.conf.d/jetvm.conf > /dev/null << JETVM_CONFIG_EOF

$JET_INSTALL_DIR/lib/x86/shared
$SAXON_INSTALL_DIR/Saxon-C-API/modules

JETVM_CONFIG_EOF

# Run `ldconfig` (see the note at the end of this file)
sudo ldconfig

# Link some files that seem to be needed but in some cases can't be found
if [ ! -e /lib64/libjvm.so ]; then
  sudo ln -s $JAVA_HOME/jre/lib/amd64/server/libjvm.so /lib64/libjvm.so
fi
if [ ! -e $SAXON_INSTALL_DIR/Saxon-C-API/jni.h ]; then
  sudo ln -s $JAVA_HOME/include/jni.h $SAXON_INSTALL_DIR/Saxon-C-API/jni.h
fi
if [ ! -e $SAXON_INSTALL_DIR/Saxon-C-API/jni_md.h ]; then
  sudo ln -s $JAVA_HOME/include/linux/jni_md.h $SAXON_INSTALL_DIR/Saxon-C-API/jni_md.h
fi

# Change into the Saxon PHP API directory
cd $SAXON_INSTALL_DIR/Saxon-C-API

# Uncomment out the necessary Saxon PHP header before compiling
sudo sed -i 's/\/\/\#include \"php_saxon\.h\"/\#include \"php_saxon\.h\"/' xsltProcessor.cc

# Now compile Saxon for PHP
sudo phpize
sudo ./configure --enable-saxon
sudo make install

# Create the Apache config file for Saxon/C
sudo tee /etc/php5/apache2/conf.d/20-saxon.ini > /dev/null << SAXON_CONFIG_EOF

extension=saxon.so

SAXON_CONFIG_EOF

# Restart Apache to pick up our changes
sudo service apache2 restart

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