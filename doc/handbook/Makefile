# $Id$

PDFLATEX?= pdflatex
EPSTOPDF?= epstopdf
PDFVIEWER?= okular

all:
	cd work; for l in *.eps; do ${EPSTOPDF} $$l; done
	${PDFLATEX} index.tex
	${PDFLATEX} index.tex
	${PDFLATEX} index.tex

run:
	${PDFVIEWER} index.pdf

public:
	scp index.pdf wieczyk@trzask.codepainters.com:www/handbook.pdf

clean:
	rm -f *.pdf *.aux *.log *.toc *.ps *.dvi work/*.pdf

