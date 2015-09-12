DOXYGEN = doxygen

all:
	@echo "Nothing here. The main makefile is artx.mk."
	@echo "See example/Makefile for how to use artx.mk."
	@echo "Try 'make test' to run tests."

dox:
	@$(DOXYGEN)

test:
	@cd t && python test.py
