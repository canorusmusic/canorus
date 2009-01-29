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
Is written in Lyx. To edit or translate User's guide open
your_language.lyx in Lyx and export it to plain Latex
(generates your_language.tex which is needed).

You can then compile it to:
 - PDF (for printing)
 - HTML (for viewing on-line)
 - QtHelp (for viewing inside Canorus)

For generating PDF files latex, ps2pdf and friends are required.
For generating HTML files htlatex is required.
For generating QtHelp (qch and qhc) files, first HTML files are
required and then compressed into QtHelp format using Qt development
tool qcollectiongenerator.


Matevz Jekovec
Canorus development team
<matevz@jekovec.net>
