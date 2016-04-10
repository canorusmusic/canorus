Hiter začetek
=============

Glavno okno
-----------

Ob zagonu se pojavi glavno okno aplikacije. Izgledati bi moralo
(odvisno od operacijskega sistema in jezika) približno takole:

.. image:: ../../images/mainwin.sl.png
   :width: 700px

Glavno okno je zgrajeno iz menija na vrhu, orodjarne za vstavljanje
na levi, splošne lastnosti elementov na vrhu in pogleda črtovja.

Vsak dokument vsebuje en ali več listov - vsak list predstavlja zavihek
na dnu (izgled je podoben aplikacijam za delo s preglednicami). Več
listov se ponavadi uporablja za različne stavke skladbe, transpozicije,
variacije ali pa samo za začasen prostor, kamor želite na hitro zapisati
glasbeno idejo.

Pisanje not
-----------

Za dodajanje not kliknite na črtovje, izberite glas v vrhnji orodjarni
in kliknite na ``Note in pavze`` ikono |INSERT_PLAYABLE| v levi orodjarni.
Na vrhu se sedaj pojavijo lastnosti, kot so dolžina
note/pavze, vidljivost, triole itd.

Za vstavljanje izbrane note kliknite na črtovje z levim miškinim klikom.
Za pavze uporabite desni klik. Za dodajanje predznakov uporabite tipki
+/-. Za popravljanje višine že vstavljene note uporabite tipki gor/dol.
Za vstavljanje pike uporabite tipko.

.. |INSERT_PLAYABLE| image:: ../../images/insertplayable.svg
   :height: 20px

Pisanje drugih elementov
------------------------

Za vstavljanje ključa, tonalitete, taktovskega načina, taktnice in
ostalih elementov uporabite gumbe v levi orodjarni, nastavite želene
lastnosti na vrhu in kliknite na črtovje. Leva orodjarna ves čas prikazuje
elemente, ki se lahko vstavijo v trenutno izbrano črtovje.

Za dodajanje lokov najprej izberite dve ali več not in nato izberite
lok.

Za vstavljanje navadne taktnice takoj za izbrano noto lahko uporabite
tudi tipko b.


Spreminjanje elementov
----------------------

Če želite spremeniti lastnosti že vstavljenih elementov, morate biti
v načinu za urejanje. Pritisnite tipko Esc ali kliknite na
puščico |ARROW| v levi orodjarni.
Ko izberete določen element v črtovju, se v zgornji orodjarni pojavijo
njegove lastnosti, ki jih lahko po želji spreminjate.

.. |ARROW| image:: ../../images/arrow.svg
   :height: 20px

\section{Novo črtovje, glas, besedilo}

Za vstavljanje novega črtovja kliknite na ikono ``Vstavi sled``
|STAFF_NEW| v levi orodjarni, izberite tip (črtovje, besedilo or funkcijske oznake) in kliknite
na pogled črtovja.

.. |STAFF_NEW| image:: ../../images/staffnew.svg
   :height: 20px

Za dodajanje novega glasu izberite črtovje in kliknite na ikono ``Nov
glas`` |VOICE_NEW| v vrhnji orodjarni.

.. |VOICE_NEW| image:: ../../images/voicenew.svg
   :height: 20px

Predvajanje
-----------

Za predvajanje od trenutne izbire dalje pritisnite preslednico ali
kliknite na zeleno ikono ``Predvajaj`` |PLAY| v vrhnji orodjarni.
Za ustavitev pritisnite še enkrat.

.. |PLAY| image:: ../../images/play.svg
   :height: 20px

Tiskanje
--------

Canorus uporablja program LilyPond za tiskanje.

Žal je potrebno LilyPond namestiti ločeno na vaš računalnik. Če LilyPonda
še niste namestili, obiščite http://www.lilypond.org. Če uporabljate
operacijski sitem Linux, lahko uporabite izbrani upravitelj paketov za
namestitev.

Za predogled tiska je potreben PDF pregledovalnik. Uporabite meni
Datoteka->Predogled tiska |PRINT_PREVIEW|.

.. |PRINT_PREVIEW| image:: ../../images/fileprintpreview.png
   :height: 20px

Dokument lahko natisnete neposredno s klikom na meni Datoteka->Tiskanje
ali pa posredno preko vašega PDF pregledovalnika (priporočeno).
