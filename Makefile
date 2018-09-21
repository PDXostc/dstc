#
# Doodling
#


.PHONY: all clean

# all: $(TARGET) examples
all:
	(cd examples/; make)

clean:
	(cd examples; make clean)
	rm -f $(OBJ) $(TARGET) *~
