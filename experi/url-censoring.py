import string

def foo():
    print censor("http://trhj.homeunix.net")

def censor(url) :
    slashslash = '://'

    slashslash_pos = url.find(slashslash)
    ampersand_pos = url.find('@')
    if (slashslash_pos == -1) or (ampersand_pos == -1):
        return url

    result = ''
    slashslash_pieces = url.split(slashslash)
    result += slashslash_pieces[0] + slashslash

    ampersand_pieces = slashslash_pieces[1].split('@')
    result += '***@' + ampersand_pieces[1]

    return result

if __name__ == '__main__':
    foo()
    for url in ('http://www.google.com', 'foobar', 'ted@gmail.com',
                'scp://username:password@10.1.35.75/image.img',
                ' http://bob@riverbed.com'):
        print censor(url)
