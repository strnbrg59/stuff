package net.trhj.androidflashcards;

import android.app.Activity;

public class DeletePopupFunctor implements PopupFunctor {
    LanguageSpinner spinner_;

    DeletePopupFunctor(LanguageSpinner spinner) {
        super();
        spinner_ = spinner;
    }

    public void doit(Activity activity, Object extra) {
        CardDb.deleteLanguage(activity, spinner_);
    }
}
