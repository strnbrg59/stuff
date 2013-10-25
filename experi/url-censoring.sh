filter() {
    sed -e 's/^.*@\(.*\)$/\1/' -e 's/\(.*\)?.*$/\1/'
}

for str in "foo@https://bar.com?password=girlfriend_name" \
           "ftp://gnu.ai.mit.edu/emacs.tar" \
           "http://google.com/~brin/search_engine_src.tbz?password=billion" \
           "http://releng/teton-i386-flamebox-latest/image.img"
do
    echo $str | filter
done
