# Saxon-C

This is a copy of the Saxon/C project from Saxonica.  It's licensed under an MPL 2.0 license by Saxonica.  If you are interested in this code, I'd recommend you take a look at [Saxon/C](http://www.saxonica.com/saxon-c/index.xml) on the Saxonica site, rather than this repository.  There is also an [activity log](https://saxonica.plan.io/projects/saxon-c/activity) and an [issue queue](https://saxonica.plan.io/projects/saxon-c/issues) on the Saxonica site.

### What's Different?

So, why does this project even exist?  Right now, the PHP Saxon XsltProcessor has a name clash with the original PHP XSLTProcessor (PHP being case insensitive).  This isn't a problem unless you want to be able to support both options in a single code base.  This project solves the problem by putting the Saxon XSLT processor in a "Saxon" namespace.  There is [a ticket](https://saxonica.plan.io/issues/2380) in at Saxonica to fix this issue with the next release, but until that's resolved I needed a version of the code that had the namespace.

The other reason this repository exists is that I'd like to script the installation of Saxon/C.  Since it's still in beta, I wasn't entirely sure about referencing the code from an install script that I wanted to continue working time after time (in testing, etc.)  So, until there are stable releases at a stable location, I thought I'd just put the code here.

### Installation

The idea is that you should be able to check out this repository and run the project's `./install.sh` script to compile and install the project on your machine.  Right now, the script hasn't had a great deal of testing, but it works for my particular use.  So far, I've tested it with Ubuntu 15.04, 14.04 (LTS) and CentOS/RHEL 6.6.

To check out the project and install it, type:

    git clone https://github.com/ksclarke/saxon-c.git
    cd saxon-c
    ./install.sh

To uninstall the project, type:

    ./install.sh clean

If you're not running the script as root you will be prompted by `sudo` for a password.

I think that's about it. If you test on a different Linux distro (or version) and discover issues with the build, feel free to create an issue in this project's [issue queue](https://github.com/ksclarke/saxon-c/issues).

### Contact

If you have any questions about this repository, feel free to contact me: Kevin S. Clarke <[ksclarke@gmail.com](mailto:ksclarke@gmail.com)>

If you have improvements to the install script, feel free to send me pull requests!