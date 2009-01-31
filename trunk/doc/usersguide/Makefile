all: pdf html qthelp

tex:
	lyx -e latex *.lyx

pdf: tex
	pdflatex *.tex
	pdflatex *.tex

html: tex
	htlatex *.tex

qthelp: html
	qcollectiongenerator *.qhcp

clean: clean-tex clean-html clean-pdf clean-qthelp

clean-tex:
	rm -f *.tex images/*.eps

clean-pdf:
	rm -f *.log *.aux *.pdf

clean-html:
	rm -f *.4ct *.4tc *.aux *.css *.dvi *.html *.idv *.lg *.log *.tmp *.xref *.png *.toc

clean-qthelp:
	rm -f *.qhc *.qch
