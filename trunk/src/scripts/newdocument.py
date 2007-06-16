import _CanorusPython

def newDefaultDocument(document):
	sheet1 = _CanorusPython.Document_addSheetByName(document, 'Sheet1')
	
	staff1 = _CanorusPython.Sheet_addStaff(sheet1)
#	staff1 = _CanorusPython.new_Staff(sheet1, 'Staff1', 5)
#	_CanorusPython.Sheet_addContext(sheet1, staff1)
	staff1voice1 = _CanorusPython.new_Voice(staff1, 'Staff1Voice1', 1, _CanorusPython.Note_StemUp)
	_CanorusPython.Staff_addVoice(staff1, staff1voice1)
	staff1voice2 = _CanorusPython.new_Voice(staff1, 'Staff1Voice2', 2, _CanorusPython.Note_StemDown)
	_CanorusPython.Staff_addVoice(staff1, staff1voice2)
	_CanorusPython.Staff_insertSign(staff1, _CanorusPython.new_Clef(_CanorusPython.Clef_Treble, staff1, 0))
	_CanorusPython.Staff_insertSign(staff1, _CanorusPython.new_TimeSignature(4, 4, staff1, 0))
	
	staff2 = _CanorusPython.Sheet_addStaff(sheet1)
	staff2voice1 = _CanorusPython.new_Voice(staff2, 'Staff2Voice1', 1, _CanorusPython.Note_StemUp)
	_CanorusPython.Staff_addVoice(staff2, staff2voice1)
	staff2voice2 = _CanorusPython.new_Voice(staff2, 'Staff2Voice2', 2, _CanorusPython.Note_StemDown)
	_CanorusPython.Staff_addVoice(staff2, staff2voice2)
	_CanorusPython.Staff_insertSign(staff2, _CanorusPython.new_Clef(_CanorusPython.Clef_Bass, staff2, 0))
	_CanorusPython.Staff_insertSign(staff2, _CanorusPython.new_TimeSignature(4, 4, staff2, 0))
