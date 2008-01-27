from CanorusPython import *

def newDefaultDocument(document):
	sheet1 = document.addSheetByName( tr('Sheet%1').replace('%1',str(1)) )
	
	staff1 = sheet1.addStaff()
	staff1.addVoice( CAVoice( tr('Staff%1').replace('%1',str(1)) + tr('Voice%1').replace('%1',str(1)), staff1, CANote.StemUp) )
	staff1.addVoice( CAVoice( tr('Staff%1').replace('%1',str(1)) + tr('Voice%1').replace('%1',str(2)), staff1, CANote.StemDown) )
	staff1.voiceAt(0).append( CAClef( CAClef.Treble, staff1, 0 ) )
	staff1.voiceAt(0).append( CATimeSignature( 4, 4, staff1, 0 ) )
	
	staff2 = sheet1.addStaff()
	staff2.addVoice( CAVoice( tr('Staff%1').replace('%1',str(2)) + tr('Voice%1').replace('%1',str(1)), staff2, CANote.StemUp) )
	staff2.addVoice( CAVoice( tr('Staff%1').replace('%1',str(2)) + tr('Voice%1').replace('%1',str(2)), staff2, CANote.StemDown) )
	staff2.voiceAt(0).append( CAClef( CAClef.Bass, staff2, 0 ) )
	staff2.voiceAt(0).append( CATimeSignature( 4, 4, staff2, 0 ) )
	
	staff1.synchronizeVoices()
	staff2.synchronizeVoices()
