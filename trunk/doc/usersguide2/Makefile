languages = en sl
SPHINX_BUILD_COMMAND = sphinx-build
RM_COMMAND = rm -rf

all: singlehtml

singlehtml: ; $(foreach l,$(languages),( $(SPHINX_BUILD_COMMAND) -b singlehtml source/$(l) build/$(l) ) &&) :

clean: clean-singlehtml

clean-singlehtml: ; $(foreach l,$(languages),( $(RM_COMMAND) build/$(l) ) &&) :
