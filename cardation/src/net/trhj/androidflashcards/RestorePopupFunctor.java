package net.trhj.androidflashcards;

import android.app.Activity;

public class RestorePopupFunctor implements PopupFunctor {
    public void doit(Activity activity, Object extra) {
        CardDb.restore(activity);
    }
}
