all: pdf html qthelp

pdf:
	pdflatex *.tex

html:
	htlatex *.tex

qthelp:
	qcollectiongenerator *.qhcp

clean: clean-html clean-pdf clean-qthelp

clean-pdf:
	rm -f *.log *.aux *.pdf

clean-html:
	rm -f *.4ct *.4tc *.aux *.css *.dvi *.html *.idv *.lg *.log *.tmp *.xref *.png *.toc

clean-qthelp:
	rm -f *.qhc *.qch
