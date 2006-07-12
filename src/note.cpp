#include "note.h"

CANote::CANote(CANoteLength length) : CAMusElement() {
	_noteLength = length;
}

CANote *CANote::clone() {
	CANote *d = new CANote(_noteLength);
	
	return d;
}
