from CanorusPython import *

def newDefaultDocument(document):
	sheet1 = document.addSheetByName( 'Sheet1' )
	
	staff1 = sheet1.addStaff()
	staff1.addVoice( Voice(staff1, 'Staff1Voice1', 1, Note.StemUp) )
	staff1.addVoice( Voice(staff1, 'Staff1Voice2', 2, Note.StemUp) )
	staff1.insertSign( Clef( Clef.Treble, staff1, 0 ) )
	staff1.insertSign( TimeSignature( 4, 4, staff1, 0 ) )

	staff2 = sheet1.addStaff()
	staff2.addVoice( Voice(staff2, 'Staff2Voice1', 1, Note.StemUp) )
	staff2.addVoice( Voice(staff2, 'Staff2Voice2', 2, Note.StemUp) )
	staff2.insertSign( Clef( Clef.Treble, staff2, 0 ) )
	staff2.insertSign( TimeSignature( 4, 4, staff2, 0 ) )
