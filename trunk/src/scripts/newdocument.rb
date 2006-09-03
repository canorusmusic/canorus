include CanorusRuby

def newDefaultDocument(document)
	sheet1 = document.addSheetByName("Sheet 1")
	staff1 = sheet1.addStaff()
	staff1voice1 = Voice.new(staff1, "Staff1Voice1")
	staff1.addVoice(staff1voice1)
	staff1voice1.insertMusElement(Clef.new(Clef::Treble, staff1, 0))
	staff1voice1.insertMusElement(TimeSignature.new(4, 4, staff1, 0))

	staff2 = sheet1.addStaff()
	staff2voice1 = Voice.new(staff2, "Staff2Voice1")
	staff2.addVoice(staff2voice1)
	staff2voice1.insertMusElement(Clef.new(Clef::Bass, staff2, 0))
	staff2voice1.insertMusElement(TimeSignature.new(4, 4, staff2, 0))
end
