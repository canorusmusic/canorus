from CanorusPython import *

def newDefaultDocument(document):
	sheet1 = document.addSheetByName( 'Sheet1' )
	
	staff1 = sheet1.addStaff()
	staff1.addVoice( Voice( 'Staff1Voice1', staff1, Note.StemUp) )
	staff1.addVoice( Voice( 'Staff1Voice2', staff1, Note.StemDown) )
	staff1.insertSign( Clef( Clef.Treble, staff1, 0 ) )
	staff1.insertSign( TimeSignature( 4, 4, staff1, 0 ) )
	
	staff2 = sheet1.addStaff()
	staff2.addVoice( Voice( 'Staff2Voice1', staff2, Note.StemUp) )
	staff2.addVoice( Voice( 'Staff2Voice2', staff2, Note.StemDown) )
	staff2.insertSign( Clef( Clef.Bass, staff2, 0 ) )
	staff2.insertSign( TimeSignature( 4, 4, staff2, 0 ) )
