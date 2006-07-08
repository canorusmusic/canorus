#ifndef NOTE_H
#define NOTE

#include <QPainter>

#include "drawable.h"

class CANote : public CADrawable {
	public:
		CANote(int length, int x, int y);
		void draw(QPainter *p, int x, int y, float z);

	private:
		int length_;
};
#endif

