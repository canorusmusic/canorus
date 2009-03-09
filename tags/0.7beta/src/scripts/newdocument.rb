include CanorusRuby

def newDefaultDocument(document)
	sheet1 = document.addSheetByName("Sheet 1")
	
	staff1 = sheet1.addStaff()
	staff1voice1 = CAVoice.new(staff1, "Staff1Voice1", 1, CANote.StemUp)
	staff1.addVoice(staff1voice1)
	staff1voice2 = CAVoice.new(staff1, "Staff1Voice2", 2, CANote.StemDown)
	staff1.addVoice(staff1voice2)
	staff1.insertSign(CAClef.new(CAClef::Treble, staff1, 0))
	staff1.insertSign(CATimeSignature.new(4, 4, staff1, 0))
	
	staff2 = sheet1.addStaff()
	staff2voice1 = CAVoice.new(staff2, "Staff2Voice1", 1, CANote.StemUp)
	staff2.addVoice(staff2voice1)
	staff2voice2 = CAVoice.new(staff2, "Staff2Voice2", 1, CANote.StemDown)
	staff2.addVoice(staff2voice2)
	staff2.insertSign(CAClef.new(CAClef::Bass, staff2, 0))
	staff2.insertSign(CATimeSignature.new(4, 4, staff2, 0))
end
