Canorus documentation is split to two parts:
 - Developer's guide
 - User's guide

*nix Makefiles are provided for each guide.

Requirements
============
Developer's guide:
Is generated out from Canorus source code using doxygen.
Doxygen generates html files.

User's guide:
Is generated using Sphinx, a python-based documentation generator. To edit or
translate User's guide go to source/your_language/ directory and edit .rst
files, one file per chapter.

Usually you want to compile User's guide to:
 - HTML (for viewing on-line, also used by Canorus built-in web engine)
 - PDF (for printing)

Sphinx-build supports generating HTML out of the box.
For generating PDF files latex, rst2pdf builder for sphinx is required (pip install rst2pdf).


Matevž Jekovec
Canorus development team
<matevz.jekovec@gmail.com>

