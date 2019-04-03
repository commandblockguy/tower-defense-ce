# ----------------------------
# Set NAME to the program name
# Set ICON to the png icon file name
# Set DESCRIPTION to display within a compatible shell
# Set COMPRESSED to "YES" to create a compressed program
# ----------------------------

NAME        ?= TDEFENSE
COMPRESSED  ?= NO
ICON        ?= iconc.png
DESCRIPTION ?= "Reverse Tower Defense"

# ----------------------------

include $(CEDEV)/include/.makefile
