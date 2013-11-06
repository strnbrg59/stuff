#!/home/strnbrg/software/sparc/bin/tclsh8.3

# Don't try this...
# the next line restarts using tclsh \
# exec tclsh "$0" "$@"
# ...because /usr/local/bin might not be on user nobody's path.

########################################################################
# From a list of grammars entered into a textarea (HTML form element), #
# produces another form suitable for entering transition probabilities.#
########################################################################
# Ted Sternberg #
# June 2000     #
#################

##########################################################
# function parseTextArea()                               #
# Parse POST input that describes a text area, into      #
# its individual rows (grammar names).                   #
#                                                        #
# $1 is the POST input, obtained from stdin.             #
##########################################################
proc parseTextArea post_input {

    puts "Content-type: text/html"
    puts ""
    
    #
    # Break up post_input at the carriage-returns ie grammar by grammar.  
    # Don't try too hard to deal with other special characters 
    # (%[0-9a-f][0-9a-f]).  But we'll allow the following: '/', '.', '_'.
    #
    set split_post_input [split $post_input "="]
    set parsed [lindex $split_post_input 1]
    regsub -nocase -all %0D%0A $parsed       "@" parsed ; #Netscape & IE
    regsub -nocase -all %0A    $parsed       "@" parsed ; #Linux Lynx
    regsub -nocase -all %2F    $parsed       "/" parsed
    regsub -nocase -all {%[0-9a-f][0-9a-f]}  $parsed  "" parsed
    regsub -nocase -all {[^a-z0-9/_\.@]}     $parsed  "" parsed
    regsub         -all @+                   $parsed  "@" parsed

    set result [split [string trim $parsed " @"] "@"]
    return $result
}
    
########################################################################
# function printTransitionMatrix()                                     #
#                                                                      #
# Print a new form -- a transition probability matrix for the user to  #
# fill in.  The html describes a table whose entries are form inputs.  #
#                                                                      #
# $1 is the list of grammars, separated by spaces.                     #
########################################################################
proc printTransitionMatrix grammar_names {

    puts {<HTML>}
    puts {<TITLE>Transition Probabilities</TITLE>}
    puts {<CENTER><H2>Transition Probabilities</H2></CENTER>}
    puts {<form action="/~strnbrg/cgi-bin/dimensioner/callmodel-find-counts.sh" \
          method="POST"><BR>}

    puts "Enter the transition probabilities: the number in"
    puts "row i, column j should be the probability that, if the current"
    puts "utterance is supposed to be from grammar i, "
    puts "then the next utterance is supposed to be from grammar j."
    puts "<P>"
    puts "First row is for the initial grammar"
    puts "<P>"
    puts "Be sure all rows add up to less than 1.0"

    puts "<table border=\"1\" cellpadding=\"0\">"

    # grammar names (headings) across the top:
    puts "<tr>"
    puts -nonewline "<td></td> "
    foreach g $grammar_names {
        puts -nonewline "<td>$g</td> "
    }
    puts "</tr>"
    
    puts "<BR>"

    # one row for each grammar
    set i 0
    foreach g $grammar_names {
        puts "<tr><td>$g</td>"
        set j 0
        foreach h $grammar_names {
            puts "<td><input type=\"text\" name=\"transprob_${i}_${j}\" \
                  size=5></td>"
            set j [expr $j + 1]
        }
        puts "</tr>"
        puts "<BR>"
        set i [expr $i + 1]
    }
    puts "</table>"

    puts {<input type="submit" value="submit"><BR>}

    # grammar names in hidden fields (for next cgi program to know what
    # they are:
    set i 0
    foreach g $grammar_names {
        puts "<input type=\"hidden\" name=\"grammar_name_${i}\" \
              value=$g>"
        set i [expr $i + 1]
    }

    puts "</form>"
}

##################
## main function #
##################
set post_data [read stdin $env(CONTENT_LENGTH)]
set grammar_names [parseTextArea $post_data]
printTransitionMatrix $grammar_names
