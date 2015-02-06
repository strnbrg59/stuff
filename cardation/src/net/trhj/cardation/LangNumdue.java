package net.trhj.cardation;

// A pair of strings -- a language name, and the number of words due.
public class LangNumdue {
    String lang_;
    int numdue_;
    LangNumdue(String lang, int numdue) {
        lang_ = lang;
        numdue_ = numdue;
    }

    @Override
    public String toString() {
        String result = new String(lang_);
        if (numdue_ > 0) {
            result += " (" + numdue_ + " due)";
        }
        return result;
    }
}
