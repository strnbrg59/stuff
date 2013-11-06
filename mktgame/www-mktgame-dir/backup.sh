find . -newer touch.me ! -type d -print | grep -v '\.act$' | grep -v '\.o$'  | grep -v "~"
find /local/lib/httpd/strnbrg -newer touch.me -print | grep html | grep pro | grep -v "~"
