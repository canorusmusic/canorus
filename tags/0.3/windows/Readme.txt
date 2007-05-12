Canorus Windows builds:
1) Configure Canorus with cmake and set CMAKE_INSTALL_PREFIX to nsis/canorus directory.
2) Compile the project with make and install it (this will install it to nsis/canorus).
3) Install Nullsoft installer (http://nsis.sf.net)
4) Install FontName plugin (needed for font installation): http://nsis.sourceforge.net/FontName_plug-in
5) Create an executable .exe installation with Nullsoft installer by using nsis/setup.nsi configuration script.


Matevž Jekovec <matevz.jekovec@gmail.com>
Canorus development team
http://www.canorus.org