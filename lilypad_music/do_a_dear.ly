\header {
    title = "Do Re Mi"
}
\include "paper26.ly"
% Choices are 11, 13, 16, 19, 20, 23 and 26
\score {
    \notes {
    < \context Staff = staffA {
        \time 2/4
        \clef treble
        c'4. d'8 e'4. c'8 e'4 c'4 e'2
        d'4. e'8 f'8 f'8 e'8 d'8 f'2( )f'2
        e'4. f'8 g'4. e'8 g'4 e'4 g'2
        f'4. g'8 a'8 a'8 g'8 f'8 a'2( )a'2
        g'4. c'8 d'8 e'8 f'8 g'8 a'2( )a'2
        a'4. d'8 e'8 fis'8 g'8 a'8 b'2( )b'2
        b'4. e'8 fis'8 gis'8 a'8 b'8 c''2( )c''4 b'8 bes'8
        a'4 f'4 b'4 g'4 c''4 g'4 e'4 d'4 c'4 d'8 e'8 f'8 g'8 a'8 b'8 c''2
        }
      \context Staff = staffB {
        \clef bass
        c4 <e4 g4> c4 <e4 g4> c4 <e4 g4> c4 <e4 g4>
        d4 <f4 g4> d4 <f4 g4> d4 <f4 g4> d4 <f4 g4>
        c4 <e4 g4> c4 <e4 g4> c4 <e4 g4> c4 <e4 g4>
        d4 <f4 g4> d4 <f4 g4> d4 <f4 g4> d4 <f4 g4>
        e4 <g4 bes4> e4 <g4 bes4> f4 <a4 c'4> f4 <a4 c'4>
        fis4 <a4 c'4> fis4 <a4 c'4> g4 <b4 d'4> g4 <b4 d'4>
        gis4 <b4 d'4> gis4 <b4 d'4> a4 <c'4 e'4> a4 r4
        f2 d2 c2( )c2 c4 d8 e8 f8 g8 a8 b8 c'2
        }
    >
    }
}
