from CanorusPython import *

def newDefaultDocument(document):
	sheet1 = document.addSheetByName( tr('Sheet%1').replace('%1',str(1)) )
	
	staff1 = sheet1.addStaff()
	staff1.addVoice( Voice( tr('Staff%1').replace('%1',str(1)) + tr('Voice%1').replace('%1',str(1)), staff1, Note.StemUp) )
	staff1.addVoice( Voice( tr('Staff%1').replace('%1',str(1)) + tr('Voice%1').replace('%1',str(2)), staff1, Note.StemDown) )
	staff1.insertSign( Clef( Clef.Treble, staff1, 0 ) )
	staff1.insertSign( TimeSignature( 4, 4, staff1, 0 ) )
	
	staff2 = sheet1.addStaff()
	staff2.addVoice( Voice( tr('Staff%1').replace('%1',str(2)) + tr('Voice%1').replace('%1',str(1)), staff2, Note.StemUp) )
	staff2.addVoice( Voice( tr('Staff%1').replace('%1',str(2)) + tr('Voice%1').replace('%1',str(2)), staff2, Note.StemDown) )
	staff2.insertSign( Clef( Clef.Bass, staff2, 0 ) )
	staff2.insertSign( TimeSignature( 4, 4, staff2, 0 ) )
