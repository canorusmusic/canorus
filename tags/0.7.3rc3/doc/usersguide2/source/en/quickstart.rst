Quickstart
==========

Main window
-----------

The first thing you'll notice when starting the application is the
main window. It should look something like this:

.. image:: ../../images/mainwin.en.png
   :width: 350px

The main window consists of a menu at the top, insert toolbar on the
left, general and music properties on the top and a tabbed score view.

Each document can have one or more sheets - a sheet is represented
as a tab (similar to spreadsheet applications). Multiple sheets can
be used for different score parts, transpositions, variations or just
as a temporary place to write your ideas down.


Inserting notes
---------------

To add notes and rests to your score, click on the staff, select the
voice at the top and the click the ``Insert playable`` icon |INSERT_PLAYABLE|
in the left toolbar. The toolbar on the top now shows you the note/rest
length, visibility, tuplets etc.

.. |INSERT_PLAYABLE| image:: ../../images/insertplayable.svg
   :height: 20px

To add a note to a score, click on the staff using the left mouse
button. To add a rest, use the right mouse button. For raising/lowering
a note for half a step, use +/- keys. To fix the just added note,
use up/down keys. To toggle a dotted note, use the . key.


Inserting other music elements
------------------------------

To insert clefs, key signatures, time signatures, barlines and other
music elements, use the buttons located in the left toolbar, set their
properties at the top and click on the score. The left toolbar shows
you the elements you can add to the currently selected context.

For slurs, first select two or more notes and add a slur.

A special keyboard shortcut b manually inserts a single barline at
the current location (eg. for pickup measures).

Editing elements
----------------

If you want to change any music elements properties once they were
already inserted, you have to be in the default Edit mode by hitting
Esc or the arrow icon in the left toolbar |ARROW|.

When selecting one or more elements, their properties appear in the
toolbar at the top.

.. |ARROW| image:: ../../images/arrow.svg
   :height: 20px

New staff, voice, lyrics
------------------------

To insert a new staff, click on the ``Insert context`` dropdown
icon in the left toolbar |STAFF_NEW| select a context (staff, lyrics or
function marks) and click on the score.

.. |STAFF_NEW| image:: ../../images/staffnew.svg
   :height: 20px

To insert a new voice, select a staff and click on the ``New voice``
icon |VOICE_NEW|.

.. |VOICE_NEW| image:: ../../images/voicenew.svg
   :height: 20px

Playback
--------

To play the music you inserted from the current selection hit the
spacebar key or click on the green playback icon |PLAY|.

.. |PLAY| image:: ../../images/play.svg
   :height: 20px

To stop the playback, click on the icon again.

Printing
--------

Canorus uses LilyPond application for printing. 

Unfortunately, LilyPond is not bundled with Canorus. The first step
is to install LilyPond - visit http://www.lilypond.org and
download it. You can also use your distribution package manager to
install it on Linux.

For print preview, you should have a working PDF viewer installed
on your system. Use File->Print preview |PRINT_PREVIEW|.

.. |PRINT_PREVIEW| image:: ../../images/fileprintpreview.png
   :height: 20px

You can print the document directly using the File->Print or you can
print it indirectly from your PDF viewer application (preferred).
