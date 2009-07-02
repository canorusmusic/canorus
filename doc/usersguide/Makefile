all: pdf qthelp

tex:
	lyx -e latex *.lyx

pdf: tex
	for i in *.tex; do pdflatex $$i && pdflatex $$i; done # Running twice for TOC

html: tex
	for i in *.tex; do htlatex $$i; done

qthelpFiles: html
	# Recreate qthelp project files (.qhp and .qhcp) - requires bash
	if [ -x /bin/bash ]; then \
		for i in *.lyx; do \
			/bin/bash createQtHelp.sh $$(echo $$i | sed 's/.lyx$$//'); \
		done \
	fi

qthelp: qthelpFiles
	for i in $$(ls *.qhcp | grep -v template.qhcp); do qcollectiongenerator $$i; done

clean: clean-tex clean-html clean-pdf clean-qthelp

clean-tex:
	rm -f *.tex images/*.eps

clean-pdf:
	rm -f *.log *.aux *.pdf

clean-html:
	rm -f *.4ct *.4tc *.aux *.css *.dvi *.html *.idv *.lg *.log *.tmp *.xref *.png *.toc

clean-qthelp:
	rm -f *.qhc *.qch
