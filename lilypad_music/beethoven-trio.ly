\header {
    title = "Trio in Es-dur op. 1,1"
    composer = "Ludwig van Beethoven (1770-1827)"
    poet = "Violoncello"
    meter = "Adagio cantabile"
    arranger = "Herausgegeben von Theodor Sternberg"
}
\version "2.12.2"
{
    \clef bass
    \time 3/4
    \key as \major
        ees8-4 r8 des8 r8 r4
        c8 r8 aes,8-4 r8 r4
        f8-4 r8 aes8-1 r8 bes8-3 r8
        ees8-3 r8 ees,8-1 r8 r4
        aes2-4 ( ges4
% bar 5
       
        f4 des4-4 ) r4
        des4 ( c4 g4-3 )
        aes4 ( ees4 ) r4
        aes4-1 ( bes4-2 c'4-4 )
        des'4-4 des4-1 r8 des8-2
% bar 10

        c8 ( des8 ees4 ) ees,4-1
        aes,4-4 ( aes8-2 ) r8 g8. ( aes16 )
        bes4. ( ees'8-2 g8-1 bes8
        ees8-4 g8 bes,8-4 ) r8 r4
        aes4. ( c'8 f8-1 aes8
% bar 15

        d8-0 f8-1 bes,8-1 ) r8 r4
        r2.
        bes4.-2 ( c'16 bes16 aes16 g16-4 f16 ees16 )
        c'8.-1 ( aes16 f4 ) r4
        bes4.-2 ( c'16 bes16 ) bes16 ( aes16 g16-3 f16 )
% bar 20

        g8 r8 r8 c'16-4 ( bes16 ) bes16 ( aes16 g16-3 f16 )
        g8 r8 r4 r4
        \set Score.skipBars = ##t R1*3
}
