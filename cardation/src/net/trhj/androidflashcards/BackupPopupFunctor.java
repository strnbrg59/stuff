package net.trhj.androidflashcards;

import android.app.Activity;

public class BackupPopupFunctor implements PopupFunctor {
    public void doit(Activity activity, Object extra) {
        CardDb.backup(activity);
    }
}
