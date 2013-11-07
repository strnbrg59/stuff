#!/usr/bin/env python

############################################################################
# Automatic generation of the considerable verbiage that has to go into
# frame1.html.
############################################################################


#
# Each element of the items tuple becomes a purple heading that leads to
# an html file.  The file name is the same as the first element of each pair,
# but with spaces converted to underscores, and .html appended.
#
items = (
    ('Introduction', 'Introduction to ChomboVis'),
    ('Release notes', 'The latest news'),
    ('FAQ', 'Frequently asked questions'),
    ('Command line', 'Command line options'),
    ('Menus', 'Guide to the ChomboVis menu hierarchy'),
    ('Installation', 'How to build ChomboVis and other software it depends on'),
    ('Visualization', 'Various ways to look at your data'),
    ('Data browser', 'In earlier ChomboVis versions, known as spreadsheet'),
    ('API', 'Programmatic and command-line control over ChomboVis'),
    ('Other features', 'State saving/restoring, hardcopy, etc'),
    ('File format', 'The Chombo HDF5 file format'),
    ('Architecture', 'The design of ChomboVis'),
    ('Contact info', 'Web links and email for the latest ChomboVis information')
    )

#
# No user-editable stuff below here.  Ne touchez pas.
# 


import string

def itemname2filename( str ):
    """
    Replace spaces with '_'.  Suffix .html shouldn't be printed (Javascript
    adds it for us.
    """

    new_str = list(str)
    while ' ' in new_str:
        space_pos = new_str.index(' ')
        new_str[space_pos] = '_'
    new_str = string.join(new_str,'')
    return new_str

header_text = """
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<html>
  <head>
    <title>Frame1</title>
<script language="JavaScript">
      var emptyFrame = '<html></html>';
      var mode = 'doc';
      var currentbase = 'Introduction';
      function chooseURL(basename){
        currentbase = basename;
        return currentbase + '.html';
      }
      function chooseMode(modename){
        mode = modename;
        return mode + '.' + currentbase + '.html';
      }
      function drawDot(){
        return 'images/ll.gif';
      }
      
</script>
<style>
      a{text-decoration: none;
      }
      a:hover {color: yellow;
        text-decoration: underline;
      }
</style>

  </head>

  <body bgcolor="black" text="#FFFFAA" link="#AFE3EA" vlink="#D6B5E0"
    alink="#FFAD00">

<center>
<P>&nbsp;</P>
<a href="http://seesar.lbl.gov/anag/chombo" target="_parent"><img border=0 src="images/chombologo.s.gif"></a>
</center>

<!--<hr> -->
<table border=0 cellspacing=0 cellpadding=0 bordercolor="#0067CA" bgcolor="0067CA">
    """

item_preamble = """
    <tr> <td width=10 height=10 valign=top><IMG alt="" height=10
    src="images/ul.gif" width=10></td>
    <td rowspan=2 colspan=2 valign=center>
    """

item_postamble = """
    </td>
    <td width=10 height=10 bgcolor="#0067CA" valign=top><IMG alt="" height=10 src="images/ur.gif" width=10></td>
    </tr>
    <tr>
    <td width=10 height=10 bgcolor="#0067CA" valign=bottom><IMG alt="" height=10 src="images/ll.gif" width=10></td>
    <td width=10 height=10 bgcolor="#0067CA" valign=bottom><IMG alt="" height=10 src="images/lr.gif" width=10></td>
    </tr>
    """

# Print header.
print header_text

# Go through the items.
for item in items:
    print item_preamble
    str = ( '<a target="Frame2" name="' + item[0] + '" href=""'
        + 'onMouseOver="window.status=\'' + item[1] + '\'"'
        + 'onClick="this.href=chooseURL(\'' + itemname2filename(item[0]) + '\')">'
        + '<font face="geneva, arial, helvetica" size=3>' + item[0] + '</font></a>'
        )
    print str
    print item_postamble
    
# Print footer.
print "</TABLE>\n</BODY>\n</HTML>"

####### end of file #######
