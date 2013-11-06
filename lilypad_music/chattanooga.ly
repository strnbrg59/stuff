\header {
    title = "Chattanooga Choo-Choo"
}
\include "paper26.ly"
% Choices are 11, 13, 16, 19, 20, 23 and 26
\score {
    \notes {
    < \context Staff = staffA {
        \time 4/4
        \clef treble
        r1
        r1
        r1
        r1

        e'8 f'8 fis'8 g'8( )g'2( )g'8 g'8 a'8 b'8 c''8 d''8 c''8 e'8 a'8 g'8( )g'2.(
        % Pardon me boy-------------- Is that the Chat - ta - noo-ga Choo-Choo?---
        
        )g'2 a'8 g'8 e'8 d'8( )d'1
        %--- Track twenty-nine

        f'8 a'8 a'8 a'8 a'8 g'8 e'8 c'8( )c'1
        % Boy, you can gimme a shine--------
        }
      \context Staff = staffB {
        \clef bass
        c8 g8 a8 g8 c8 g8 a8 g8
        c8 g8 a8 g8 c8 g8 a8 g8
        c8 g8 a8 g8 c8 g8 a8 g8
        c8 g8 a8 g8 c8 g8 a8 g8

        c8 g8 a8 g8 c8 g8 a8 g8 
        c8 g8 a8 g8 c8 g8 a8 g8
        c8 g8 a8 g8 c8 g8 a8 g8
        % Pardon me boy.  Is that the Chattanooga Choo-Choo?

        c8 g8 a8 g8 c8 g8 a8 g8
        d8 g8 a8 g8 d8 g8 a8 g8
        % Track twenty-nine

        d8 g8 a8 g8 d8 g8 a8 g8
        c8 g8 a8 g8 c8 g8 a8 g8
        % Boy, you can gimme a shine--------
        }
    >
    }
}



