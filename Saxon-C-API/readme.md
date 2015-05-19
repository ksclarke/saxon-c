Saxon/C is an alpha release of Saxon-HE on the C/C++ programming platform. APIs are offered currently to run XSLT 2.0 from C/C++ or PHP applications.

Platforms supported: Intel

## Installation: ##

#### Saxon-HEC: ####
To install the Saxon-HEC alpha release, unzip the the file libsaxon-HEX-setup.zip and execute the command './libsaxon-HEX-setup'
First step is to select the destination of where the product files will be installed.
The product files are unpacked in the directory 'Saxon-HEC'

You need to setup the environment for the jet jvm. The jvm is in the directory JET-home=Saxonica-HEC/rt
The directory JET-home/lib/i386 must be listed in the LD_LIBRARY_PATH environment variable. For instance, if you
are using bash or Bourne shell, use the following commands:

    export LD_LIBRARY_PATH=/usr/lib/rt/lib/i386:$LD_LIBRARY_PATH

We assume that the 'rt' directory is in the location /usr/lib.
The Saxon-HEC API assumes the library is installed as follows: '/usr/lib/libsaxon.so'


#### PHP extension: ####
To build the php extension follow the steps below:

* uncomment the following code in the file xsltProcessor: #include "php_saxon.h" 

Run the commands:

* phpize
* ./configure --enable-saxon
* make
* sudo make install

Update the php.ini file (if using ubuntu it is usually in the location '/etc/php5/apache2/') to contain the php extension: insert the following in the Dynamic Extensions section: extension=saxon.so

* sudo service apache2 restart

## Getting started: ##

To get started please browse the Saxon/C API starting with the class [SaxonProcessor](classSaxonProcessor.html) class which acts as a factory class for generating the processors.

### C/C++ ###

<pre><code>
	SaxonProcessor *processor = new SaxonProcessor();
	XsltProcessor * xslt = processor->newTransformer();
	cout<<"Hello World"<<endl;
	cout<<"Test output: "<<xslt->xsltApplyStylesheet("cat.xml","test.xsl")<<endl;
</code></pre>

See the file xsltProcessor.cc

To compile the sample C++ program remove the line '#include 'php_saxon.h' in the file xsltProcessor.cc then execute the command: 

g++ -m32 -o xsltProcessor xsltProcessor.cc -ldl -L .  libsaxon.so -lstdc++



### PHP ###

Example code:
<pre><code>
	<?php 
		$saxon_version = $xslt->version();
		$xslt = new XsltProcessor();
		echo $xslt->xsltApplyStylesheet("cat.xml", "test.xsl");
		exit('PATH update'.getenv("PATH"));
		echo $xslt->xsltApplyStylesheet("cat.xml", "test.xsl");
		echo "This link is using Html: "
	?>
</code></pre>

Also look at the file xslt30TestSuite.php

## Technical Information: ##

Saxon/C is built by cross compiling the Java code of Saxon-HE 9.5 using the [Excelsior Jet tool](http://www.excelsior-usa.com/).
This generates platform specific machine code, which we interface with C/C++ using the Java Native Interace (JNI).

The PHP interface is in the form of a C/C++ PHP extension to Saxon/C created using the Zend module API.

The XML parser used is the one supplied by the Excelsior JET runtime. There are currently no links to libxml.

## Limitations: ##

The following limitations apply to the alpha release, but we expect to remove them in later releases:

* No support of XQuery or free standing XPath in the API.
* Currently Saxon-HE only
* No supports for XSLT extension functions

### Feedback/Comments: ###

Please use the help forums and bug trackers at [saxonica.plan.io](href="https://saxonica.plan.io/projects/saxon-c) if you need help or advice.



