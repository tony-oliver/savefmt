TARGET =				test-savefmt
BUILDDIR = 				build

.PHONY:					all verbose cmake clean run view install uninstall ${BUILDDIR}/install_manifest.txt help

all: ${BUILDDIR};		@cd ${BUILDDIR} && make -j4

verbose: ${BUILDDIR};	@cd ${BUILDDIR} && make -j4 VERBOSE=1

${BUILDDIR}:;			@cmake -B ${BUILDDIR}

cmake:;					@cmake -B ${BUILDDIR}

clean:;					@rm -rvf ${BUILDDIR}

run: all;				@${BUILDDIR}/test/${TARGET}

view: all;				@firefox build/docs/html/index.html &			

install: all;			@sudo cmake --install ${BUILDDIR}

uninstall:				${BUILDDIR}/install_manifest.txt
						@[[ -f $< ]] && sudo xargs -r rm -fv < $< && sudo rm -fv $< || true

help:					@echo 'make [all] - compile the test suite'
						@echo 'make verbose - as above, reporting commands executed'
						@echo 'make clean - remove non-codebase artifacts'
						@echo 'make run - execute the test suite'
						@echo 'make view - open the documentation in firefox'
						@echo 'make install - copy the header(s) to system location'
						@echo 'make uninstall - remove the header(s) from system location'
						