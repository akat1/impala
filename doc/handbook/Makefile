# $Id$

PDFLATEX?= pdflatex
EPSTOPDF?= epstopdf

all:
	cd work; for l in *.eps; do ${EPSTOPDF} $$l; done
	${PDFLATEX} index.tex
	${PDFLATEX} index.tex
	${PDFLATEX} index.tex

clean:
	rm -f *.pdf *.aux *.log *.toc *.ps *.dvi

