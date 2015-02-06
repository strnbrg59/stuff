package net.trhj.cardation;

import android.app.Activity;

public class DeletePopupFunctor implements PopupFunctor {
    LanguageSpinner spinner_;

    DeletePopupFunctor(LanguageSpinner spinner) {
        super();
        spinner_ = spinner;
    }

    public void onPositiveButton(Activity activity, Object extra) {
        CardDb.deleteCurrentLanguage(activity, spinner_);
    }

    public void onNegativeButton(Activity activity) {
    }
}
