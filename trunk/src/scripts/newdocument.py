from CanorusPython import *

def newDefaultDocument(document):
	sheet1 = document.addSheet()
	
	staff1 = sheet1.addStaff() # adds one voice automatically
	staff1.addVoice()          # add another voice
	staff1.voiceAt(0).setStemDirection( CANote.StemUp )
	staff1.voiceAt(1).setStemDirection( CANote.StemDown )
	staff1.voiceAt(0).append( CAClef( CAClef.Treble, staff1, 0 ) )
	staff1.voiceAt(0).append( CATimeSignature( 4, 4, staff1, 0 ) )
	
	staff2 = sheet1.addStaff() # adds one voice automatically
	staff2.addVoice()          # add another voice
	staff2.voiceAt(0).setStemDirection( CANote.StemUp )
	staff2.voiceAt(1).setStemDirection( CANote.StemDown )
	staff2.voiceAt(0).append( CAClef( CAClef.Bass, staff2, 0 ) )
	staff2.voiceAt(0).append( CATimeSignature( 4, 4, staff2, 0 ) )
	
	staff1.synchronizeVoices()
	staff2.synchronizeVoices()
